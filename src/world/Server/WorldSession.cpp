/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "WorldSession.h"

#include "DatabaseDefinition.hpp"
#include "Threading/ThreadSafeQueue.hpp"
#include "WorldPacket.h"
#include "Objects/Item.hpp"
#include "Exceptions/PlayerExceptions.hpp"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/Management/MapMgr.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "WorldSocket.h"
#include "Packets/SmsgNotification.h"
#include "Packets/SmsgLogoutComplete.h"
#include "OpcodeTable.hpp"
#include "World.h"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Management/Battleground/BattlegroundMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Packets/SmsgMessageChat.h"
#include "Objects/Transporter.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Script/HookInterface.hpp"
#include <cstdarg>
#include "OpcodeHandlerRegistry.hpp"

using namespace AscEmu::Packets;

WorldSession::WorldSession(uint32_t id, std::string name, WorldSocket* sock) :
    m_loggingInPlayer(nullptr),
    m_currMsTime(Util::getMSTime()),
    m_lastPing(0),
    bDeleted(false),
    m_moveDelayTime(0),
    m_clientTimeDelay(0),
    m_wLevel(0),
    m_bIsWLevelSet(false),
    _player(nullptr),
    _socket(sock),
    _accountId(id),
    _accountFlags(0),
    _accountName(name),
#if VERSION_STRING > TBC
    has_level_55_char(false),
    has_dk(false),
#endif
    _side(255),
    _logoutTime(0),
    _loggingOut(false),
    LoggingOut(false),
    _latency(0),
    client_build(0),
    instanceId(0),
    _updatecount(0),
    floodLines(0),
    floodTime(UNIXTIME),
    language(0),
    m_muted(0),
    m_loginTime(0)
{
#if VERSION_STRING >= Cata
    isAddonMessageFiltered = false;
#endif

    for (uint8_t x = 0; x < 8; x++)
        sAccountData[x].data = nullptr;
}

WorldSession::~WorldSession()
{
    std::lock_guard guard(deleteMutex);

    if (_player)
    {
        sLogger.failure("warning: logged out player in worldsession destructor");
        LogoutPlayer(true);
    }

    while (auto packet = _recvQueue.pop())
    {
    }

    if (_socket)
        _socket->SetSession(nullptr);

    if (m_loggingInPlayer)
        m_loggingInPlayer->setSession(nullptr);
}

uint8_t WorldSession::Update(uint32_t InstanceID)
{
    m_currMsTime = Util::getMSTime();

    if (!((++_updatecount) % 2) && _socket)
        _socket->UpdateQueuedPackets();

    if (m_loginTime == 0)
        m_loginTime = Util::getMSTime();

    if (InstanceID != instanceId)
    {
        // We're being updated by the wrong thread.
        // "Remove us!" - 2
        return 2;
    }

    // Socket disconnection.
    if (!_socket)
    {
        // Check if the player is in the process of being moved. We can't delete him if we are.
        if (_player && _player->m_beingPushed)
        {
            // Abort..
            return 0;
        }

        if (!_logoutTime)
            _logoutTime = m_currMsTime + PLAYER_LOGOUT_DELAY;
    }

    while (auto packet = _recvQueue.pop())
    {
        if (packet.value() != nullptr)
        {
            // handling opcode
            OpcodeHandlerRegistry::instance().handleOpcode(*this, *packet.value());

            // set pointer to nullptr since it was processed
            packet.value() = nullptr;

            // If we hit this -> means a packet has changed our map.
            if (InstanceID != instanceId)
                return 2;

            if (bDeleted)
                return 1;
        }
    }

    // If we hit this -> means a packet has changed our map.
    if (InstanceID != instanceId)
        return 2;

    if (_logoutTime && (m_currMsTime >= _logoutTime) && instanceId == InstanceID)
    {
        // Check if the player is in the process of being moved. We can't delete him if we are.
        if (_player && _player->m_beingPushed)
        {
            // Abort..
            return 0;
        }

        if (_socket == nullptr)
        {
            bDeleted = true;
            LogoutPlayer(true);
            return 1;
        }
        else
            LogoutPlayer(true);
    }

    if (m_lastPing + WORLDSOCKET_TIMEOUT < static_cast<uint32_t>(UNIXTIME))
    {
        // Check if the player is in the process of being moved. We can't delete him if we are.
        if (_player && _player->m_beingPushed)
        {
            // Abort..
            return 0;
        }

        // ping timeout!
        if (_socket != nullptr)
        {
            Disconnect();
            _socket = nullptr;
        }

        m_lastPing = static_cast<uint32_t>(UNIXTIME); // Prevent calling this code over and over.

        if (!_logoutTime)
            _logoutTime = m_currMsTime + PLAYER_LOGOUT_DELAY;
    }

    return 0;
}

void WorldSession::LogoutPlayer(bool Save)
{
    Player* pPlayer = _player;

    if (_loggingOut)
        return;

    _loggingOut = true;
    m_loginTime = 0;

    if (_player != nullptr)
    {
        _player->setFaction(_player->getInitialFactionId());

        sObjectMgr.removePlayer(_player);
        _player->m_isReadyToBeRemoved = true;

        sHookInterface.OnLogout(pPlayer);
        if (_player->m_duelPlayer)
            _player->m_duelPlayer->endDuel(DUEL_WINNER_RETREAT);

        if (_player->m_currentLoot && _player->IsInWorld())
        {
            Object* obj = _player->getWorldMap()->getObject(_player->m_currentLoot);
            if (obj != nullptr)
            {
                switch (obj->getObjectTypeId())
                {
                    case TYPEID_UNIT:
                    {
                        if (const auto creature = dynamic_cast<Creature*>(obj))
                            creature->loot.removeLooter(_player->getGuidLow());
                    } break;
                    case TYPEID_GAMEOBJECT:
                    {
                        if (const auto go = dynamic_cast<GameObject*>(obj))
                        {
                            if (!go->IsLootable())
                                break;

                            if (const auto pLGO = dynamic_cast<GameObject_Lootable*>(go))
                                pLGO->loot.removeLooter(_player->getGuidLow());
                        }
                    } break;
                }
            }
        }

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
        GM_Ticket* ticket = sTicketMgr.getGMTicketByPlayer(_player->getGuid());
        if (ticket != NULL)
        {
            // Send status change to gm_sync_channel
            Channel* chn = sChannelMgr.getChannel(sWorld.getGmClientChannel(), _player);
            if (chn)
            {
                std::stringstream ss;
                ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_ONLINESTATE;
                ss << ":" << ticket->guid;
                ss << ":0";
                chn->Say(_player, ss.str().c_str(), NULL, true);
            }
        }
#endif

        // part channels
        _player->removeAllChannels();

        auto transport = _player->GetTransport();
        if (transport != nullptr)
        {
            transport->RemovePassenger(_player);
        }

        // cancel current spell
        for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
        {
            if (_player->getCurrentSpell(CurrentSpellType(i)) != nullptr)
                _player->interruptSpellWithSpellType(CurrentSpellType(i));
        }

        _player->sendFriendStatus(false);

        // Decrement the global player count
        sWorld.decrementPlayerCount(_player->getTeam());

        if (_player->m_isQueuedForBg)
            sBattlegroundManager.removePlayerFromQueues(_player);

        // Repop or Resurrect and remove from battlegrounds
        if (_player->m_bg)
        {
            if (pPlayer->getDeathState() == JUST_DIED)
                pPlayer->setResurrect();
            if (pPlayer->getDeathState() != ALIVE)
                pPlayer->resurrect();
            _player->m_bg->removePlayer(_player, true);
        }
        else if (_player->isDead() && _player->getDeathState() == JUST_DIED)
            _player->repopRequest();

        // Issue a message telling all guild members that this player signed
        // off

        _player->getItemInterface()->EmptyBuyBack();
        _player->getItemInterface()->removeLootableItems();

        // Save HP/Mana
        _player->m_loadHealth = _player->getHealth();
        _player->m_loadMana = _player->getPower(POWER_TYPE_MANA);

        // _player->SaveAuras();

        if (Save)
            _player->saveToDB(false);

        // Remove pet/summons after save so current pet is properly saved
        // Keep pet active so it will be summoned again when player logs in
        _player->unSummonPetTemporarily();
        _player->getSummonInterface()->removeAllSummons();

        // Dismounting with removeAllAuras may in certain cases add a player
        // aura,
        // which can result in a nice crash during shutdown. Therefore let's
        // dismount on logout.
        // Ofc if the player was mounted on login they will be still mounted
        // ;)
        _player->dismount();

        _player->cleanupAfterTaxiFlight();

        _player->removeAllAuras();
        if (_player->IsInWorld())
            _player->removeFromWorld();

        if (_player->m_playerInfo->m_Group != nullptr)
            _player->m_playerInfo->m_Group->Update();

        // Remove the "player locked" flag, to allow movement on next login
        _player->removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);

        // Save Honor Points
        // _player->SaveHonorFields();

        // Update any dirty account_data fields.
        bool dirty = false;

        if (worldConfig.server.useAccountData)
        {
            std::stringstream ss;
            ss << "UPDATE account_data SET ";
            for (uint32_t ui = 0; ui < 8; ui++)
            {
                if (sAccountData[ui].bIsDirty)
                {
                    if (dirty)
                        ss << ",";
                    ss << "uiconfig" << ui << "=\"";

                    if (sAccountData[ui].data)
                    {
                        CharacterDatabase.EscapeLongString(sAccountData[ui].data.get(), sAccountData[ui].sz, ss);
                        // ss.write(sAccountData[ui].data,sAccountData[ui].sz);
                    }
                    ss << "\"";
                    dirty = true;
                    sAccountData[ui].bIsDirty = false;
                }
            }
            if (dirty)
            {
                ss << " WHERE acct=" << _accountId << ";";
                CharacterDatabase.ExecuteNA(ss.str().c_str());
            }
        }

        delete _player;
        _player = nullptr;

        SendPacket(SmsgLogoutComplete().serialise().get());
        sLogger.debug("SESSION: Sent SMSG_LOGOUT_COMPLETE Message");
    }
    _loggingOut = false;
    LoggingOut = false;

    SetLogoutTimer(0);
}

Player* WorldSession::GetPlayerOrThrow()
{
    Player* player = this->_player;
    if (player == nullptr)
        throw AscEmu::Exception::PlayerNotFoundException();

    return player;
}

void WorldSession::LoadSecurity(std::string securitystring)
{
    permissions = securitystring;

    sLogger.debug("Loaded permissions for {}. [{}]", this->GetAccountId(), permissions);
}

std::unique_ptr<char[]> WorldSession::GetPermissions() const
{
    auto charPtr = std::make_unique<char[]>(permissions.size() + 1);
    std::strcpy(charPtr.get(), permissions.c_str());
    return charPtr;
}

bool WorldSession::hasPermissions() const
{
    return permissions.size() > 1 ? true : false;
}

bool WorldSession::hasPermission(const char* requiredPermission) const
{
    if (requiredPermission == nullptr || permissions.empty())
        return false;

    //sLogger.info("Your permission string is \"{}\"", permissions);

    if (permissions.find(requiredPermission) != std::string::npos)
        return true;
    return false;
}

bool WorldSession::HasGMPermissions() const
{
    return hasPermission("a");
}

bool WorldSession::CanUseCommand(char cmdstr)
{
    std::string commandPermission;
    commandPermission.append(1, cmdstr);
    return canUseCommand(commandPermission);
}

bool WorldSession::canUseCommand(const std::string& cmdstr) const
{
    return hasPermission(cmdstr.c_str());
}

AccountDataEntry* WorldSession::GetAccountData(uint32_t index)
{
    if (index < 8)
    {
        return &sAccountData[index];
    }

    sLogger.failure("GetAccountData tried to get invalid index {}", index);
    return nullptr;
}

void WorldSession::SendNotification(const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    SendPacket(SmsgNotification(msg1).serialise().get());
}

void SessionLog::writefromsession(WorldSession* session, const char* format, ...)
{
    if (isSessionLogOpen())
    {
        va_list ap;
        va_start(ap, format);
        char out[32768];

        std::string current_time = "[" + Util::GetCurrentDateTimeString() + "] ";
        snprintf(out, 32768, current_time.c_str());
        size_t lenght = strlen(out);

        snprintf(&out[lenght], 32768 - lenght, "Account %u [%s], IP %s, Player %s :: ",
            session->GetAccountId(),
            session->GetAccountName().c_str(),
            session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "NOIP",
            session->GetPlayer() ? session->GetPlayer()->getName().c_str() : "nologin");

        lenght = strlen(out);
        vsnprintf(&out[lenght], 32768 - lenght, format, ap);

        fprintf(mSessionLogFile, "%s\n", out);
        fflush(mSessionLogFile);
        va_end(ap);
    }
}

void SessionLog::write(WorldSession* session, const char* format, ...)
{
    if (isSessionLogOpen())
    {
        va_list ap;
        va_start(ap, format);

        // Get current time
        std::string current_time = "[" + Util::GetCurrentDateTimeString() + "] ";

        // Format the fixed part of the log entry using AscEmu::StringFormat
        std::string logEntry = AscEmu::StringFormat("{}Account {} [{}], IP {}, Player {} :: ",
            current_time,
            session->GetAccountId(),
            session->GetAccountName(),
            session->GetSocket() ? session->GetSocket()->GetRemoteIP() : "NOIP",
            session->GetPlayer() ? session->GetPlayer()->getName() : "nologin");

        // Use a buffer to format the variable arguments (like vsnprintf)
        char messageBuffer[1024];
        vsnprintf(messageBuffer, sizeof(messageBuffer), format, ap);
        va_end(ap);

        // Append the formatted message to the log entry
        logEntry += messageBuffer;

        // Write to the log file
        fprintf(mSessionLogFile, "%s\n", logEntry.c_str());
        fflush(mSessionLogFile);
    }
}

void WorldSession::SystemMessage(const char* format, ...)
{
    char buffer[1024];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, 1024, format, ap);
    va_end(ap);

    SendPacket(SmsgMessageChat(SystemMessagePacket(buffer)).serialise().get());
}

void WorldSession::sendSystemMessagePacket(std::string& _message)
{
    SendPacket(SmsgMessageChat(SystemMessagePacket(_message)).serialise().get());
}

void WorldSession::SendChatPacket(WorldPacket* data, uint32_t langpos, int32_t lang, WorldSession* originator)
{
    if (lang == -1)
        *reinterpret_cast<uint32_t*>(& data->contents()[langpos]) = lang;
    else
    {
        if (CanUseCommand('c') || (originator && originator->CanUseCommand('c')))
            *reinterpret_cast<uint32_t*>(& data->contents()[langpos]) = LANG_UNIVERSAL;
        else
            *reinterpret_cast<uint32_t*>(& data->contents()[langpos]) = lang;
    }

    SendPacket(data);
}

/*
   2008/10/04 MultiLanguages on each player session. LocalizedWorldSrv
   translating core message from sql. LocalizedMapName translating MAP Title
   from sql. LocalizedBroadCast translating new broadcast system from sql.
   Full merged from p2wow 's branch (p2branch). cebernic@gmail.com */

char szError[64];


// Returns a gossip menu option indexed by id
// These strings can be found in gossip_menu_option tables in the database
const char* WorldSession::LocalizedGossipOption(uint32_t id)
{
    MySQLStructure::GossipMenuOption const* wst = sMySQLStore.getGossipMenuOption(id);
    if (!wst)
    {
        memset(szError, 0, 64);
        sprintf(szError, "ID:%u is a bad GossipMenuOption TEXT!", id);
        return szError;
    }

    MySQLStructure::LocalesGossipMenuOption const* lpi = (language > 0) ? sMySQLStore.getLocalizedGossipMenuOption(id, language) : nullptr;
    if (lpi != nullptr)
    {
        return lpi->name;
    }
    else
    {
        return wst->text.c_str();
    }
}

// Returns a worldstring indexed by id
// These strings can be found in the worldstring tables in the database
const char* WorldSession::LocalizedWorldSrv(uint32_t id)
{
    MySQLStructure::WorldStringTable const* wst = sMySQLStore.getWorldString(id);
    if (!wst)
    {
        memset(szError, 0, 64);
        sprintf(szError, "ID:%u is a bad WorldString TEXT!", id);
        return szError;
    }

    MySQLStructure::LocalesWorldStringTable const* lpi = (language > 0) ? sMySQLStore.getLocalizedWorldStringTable(id, language) : nullptr;
    if (lpi != nullptr)
    {
        return lpi->text;
    }
    else
    {
        return wst->text.c_str();
    }
}

const char* WorldSession::LocalizedMapName(uint32_t id)
{
    MySQLStructure::MapInfo const* mi = sMySQLStore.getWorldMapInfo(id);
    if (!mi)
    {
        memset(szError, 0, 64);
        sprintf(szError, "ID:%u still have no map title yet!", id);
        return szError;
    }

    MySQLStructure::LocalesWorldmapInfo const* lpi = (language > 0) ? sMySQLStore.getLocalizedWorldmapInfo(id, language) : nullptr;
    if (lpi != nullptr)
    {
        return lpi->text;
    }
    else
    {
        return mi->name.c_str();
    }
}

const char* WorldSession::LocalizedBroadCast(uint32_t id)
{
    MySQLStructure::WorldBroadCast const* wb = sMySQLStore.getWorldBroadcastById(id);
    if (!wb)
    {
        memset(szError, 0, 64);
        sprintf(szError, "ID:%u is a invalid WorldBroadCast message!", id);
        return szError;
    }

    MySQLStructure::LocalesWorldbroadcast const* lpi = (language > 0) ? sMySQLStore.getLocalizedWorldbroadcast(id, language) : nullptr;
    if (lpi)
    {
        return lpi->text;
    }
    else
    {
        return wb->text.c_str();
    }
}

void WorldSession::Unhandled(WorldPacket& recv_data)
{
    recv_data.rfinish();
}

void WorldSession::nothingToHandle(WorldPacket& recv_data)
{
    if (!recv_data.isEmpty())
    {
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Opcode {} [{}] (0x{:04X}) received. Apply nothingToHandle handler but size is {}!",
            sOpcodeTables.getNameForOpcode(recv_data.GetOpcode()), sOpcodeTables.getNameForAEVersion(), recv_data.GetOpcode(), recv_data.size());
    }
}

void WorldSession::SendPacket(WorldPacket* packet)
{
    if (packet->GetOpcode() == 0x0000)
    {
        sLogger.failure("Return, packet 0x0000 is not a valid packet!");
        return;
    }

    if (_socket && _socket->IsConnected())
    {
        _socket->SendPacket(packet);
    }
}

void WorldSession::OutPacket(uint16_t opcode)
{
    if (_socket && _socket->IsConnected())
    {
        _socket->OutPacket(opcode, 0, nullptr);
    }
}

void WorldSession::OutPacket(uint16_t opcode, uint16_t len, const void* data)
{
    if (_socket && _socket->IsConnected())
    {
        _socket->OutPacket(opcode, len, data);
    }
}

void WorldSession::QueuePacket(std::unique_ptr<WorldPacket> packet)
{
    m_lastPing = static_cast<uint32_t>(UNIXTIME);
    _recvQueue.push(std::move(packet));
}

void WorldSession::Disconnect()
{
    if (_socket && _socket->IsConnected())
    {
        _socket->Disconnect();
    }
}

//MIT
void WorldSession::registerOpcodeHandler()
{
    // Register opcodes using the new OpcodeHandlerRegistry
    OpcodeHandlerRegistry& registry = OpcodeHandlerRegistry::instance();

    // Login
    registry.registerOpcode<STATUS_AUTHED>(CMSG_ENUM_CHARACTERS, &WorldSession::handleCharEnumOpcode, true, true, true, true, true);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_CHAR_CREATE, &WorldSession::handleCharCreateOpcode, true, true, true, true, true);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_CHAR_DELETE, &WorldSession::handleCharDeleteOpcode, true, true, true, true, true);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_CHAR_RENAME, &WorldSession::handleCharRenameOpcode, true, true, true, true, false);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_CHAR_CUSTOMIZE, &WorldSession::handleCharCustomizeLooksOpcode, false, false, true, true, false);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_CHAR_FACTION_CHANGE, &WorldSession::handleCharFactionOrRaceChange, false, false, true, false, false);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_CHAR_RACE_CHANGE, &WorldSession::handleCharFactionOrRaceChange, false, false, true, false, false);

    // declined names (Cyrillic client)
    registry.registerOpcode<STATUS_AUTHED>(CMSG_SET_PLAYER_DECLINED_NAMES, &WorldSession::handleSetPlayerDeclinedNamesOpcode, false, true, true, true, true);

    registry.registerOpcode<STATUS_AUTHED>(CMSG_PLAYER_LOGIN, &WorldSession::handlePlayerLoginOpcode, true, true, true, true, true);

    registry.registerOpcode<STATUS_AUTHED>(CMSG_REALM_SPLIT, &WorldSession::handleRealmSplitOpcode, true, true, true, true, true);

    // Queries
    registry.registerOpcode(MSG_CORPSE_QUERY, &WorldSession::handleCorpseQueryOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_NAME_QUERY, &WorldSession::handleNameQueryOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUERY_TIME, &WorldSession::handleQueryTimeOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_CREATURE_QUERY, &WorldSession::handleCreatureQueryOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_GAMEOBJECT_QUERY, &WorldSession::handleGameObjectQueryOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_PAGE_TEXT_QUERY, &WorldSession::handlePageTextQueryOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ITEM_NAME_QUERY, &WorldSession::handleItemNameQueryOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_QUERY_INSPECT_ACHIEVEMENTS, &WorldSession::handleAchievmentQueryOpcode, false, false, true, true, false);

    // Movement
    registry.registerOpcode(MSG_MOVE_HEARTBEAT, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_WORLDPORT_ACK, &WorldSession::handleMoveWorldportAckOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_JUMP, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_ASCEND, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_STOP_ASCEND, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_FORWARD, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_BACKWARD, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_SET_FACING, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_STRAFE_LEFT, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_STRAFE_RIGHT, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_STOP_STRAFE, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_TURN_LEFT, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_TURN_RIGHT, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_STOP_TURN, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_PITCH_UP, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_PITCH_DOWN, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_STOP_PITCH, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_SET_RUN_MODE, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_SET_WALK_MODE, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_SET_PITCH, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_START_SWIM, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_STOP_SWIM, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_FALL_LAND, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(MSG_MOVE_STOP, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(CMSG_MOVE_SET_FLY, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_STOP_ASCEND, &WorldSession::handleMovementOpcodes, true, true, true, true, false);
    registry.registerOpcode(CMSG_MOVE_NOT_ACTIVE_MOVER, &WorldSession::handleMoveNotActiveMoverOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SET_ACTIVE_MOVER, &WorldSession::handleSetActiveMoverOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_CHNG_TRANSPORT, &WorldSession::handleMovementOpcodes, true, true, true, true, false);

    // ACK
    registry.registerOpcode(MSG_MOVE_TELEPORT_ACK, &WorldSession::handleMoveTeleportAckOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MOVE_FEATHER_FALL_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_WATER_WALK_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, true, false);
    registry.registerOpcode(CMSG_FORCE_MOVE_ROOT_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, true, false);
    registry.registerOpcode(CMSG_FORCE_MOVE_UNROOT_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, true, false);
    registry.registerOpcode(CMSG_MOVE_KNOCK_BACK_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_HOVER_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_SET_CAN_FLY_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_DESCEND, &WorldSession::handleMovementOpcodes, true, true, true, true, false);

    // Force Speed Change
    registry.registerOpcode(CMSG_FORCE_RUN_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, true, false);
    registry.registerOpcode(CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, true, false);
    registry.registerOpcode(CMSG_FORCE_SWIM_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, true, false);
    registry.registerOpcode(CMSG_FORCE_WALK_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, true, false);
    registry.registerOpcode(CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, true, false);
    registry.registerOpcode(CMSG_FORCE_TURN_RATE_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, true, false);
    registry.registerOpcode(CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, false, true, true, false);
    registry.registerOpcode(CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, false, true, true, false);

    // Action Buttons
    registry.registerOpcode(CMSG_SET_ACTION_BUTTON, &WorldSession::handleSetActionButtonOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_REPOP_REQUEST, &WorldSession::handleRepopRequestOpcode, true, true, true, true, false);

    // Loot
    registry.registerOpcode(CMSG_AUTOSTORE_LOOT_ITEM, &WorldSession::handleAutostoreLootItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LOOT_MONEY, &WorldSession::handleLootMoneyOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LOOT, &WorldSession::handleLootOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LOOT_RELEASE, &WorldSession::handleLootReleaseOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LOOT_ROLL, &WorldSession::handleLootRollOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LOOT_MASTER_GIVE, &WorldSession::handleLootMasterGiveOpcode, true, true, true, true, false);

    // Player Interaction
    registry.registerOpcode(CMSG_WHO, &WorldSession::handleWhoOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_WHOIS, &WorldSession::handleWhoIsOpcode, true, false, true, true, false);
    registry.registerOpcode(CMSG_LOGOUT_REQUEST, &WorldSession::handleLogoutRequestOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_PLAYER_LOGOUT, &WorldSession::handlePlayerLogoutOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LOGOUT_CANCEL, &WorldSession::handleLogoutCancelOpcode, true, true, true, true, false);
    // registry.registerOpcode(CMSG_LOGOUT_CANCEL].status = STATUS_LOGGEDIN_RECENTLY_LOGGOUT, false, false, true, false, false);

    registry.registerOpcode(CMSG_ZONEUPDATE, &WorldSession::handleZoneupdate, true, true, true, true, false);
    // registry.registerOpcode(CMSG_SET_TARGET_OBSOLETE, &WorldSession::HandleSetTargetOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_SET_SELECTION, &WorldSession::handleSetSelectionOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_STANDSTATECHANGE, &WorldSession::handleStandStateChangeOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_CANCEL_MOUNT_AURA, &WorldSession::handleDismountOpcode, true, true, true, false, false);

    // Friends
    registry.registerOpcode(CMSG_CONTACT_LIST, &WorldSession::handleFriendListOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_ADD_FRIEND, &WorldSession::handleAddFriendOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_DEL_FRIEND, &WorldSession::handleDelFriendOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ADD_IGNORE, &WorldSession::handleAddIgnoreOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_DEL_IGNORE, &WorldSession::handleDelIgnoreOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUG, &WorldSession::handleBugOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SET_CONTACT_NOTES, &WorldSession::handleSetFriendNote, false, true, true, false, false);

    // Areatrigger
    registry.registerOpcode(CMSG_AREATRIGGER, &WorldSession::handleAreaTriggerOpcode, true, true, true, true, false);

    // Account Data
    registry.registerOpcode<STATUS_AUTHED>(CMSG_UPDATE_ACCOUNT_DATA, &WorldSession::handleUpdateAccountData, true, true, true, true, false);
    registry.registerOpcode(CMSG_REQUEST_ACCOUNT_DATA, &WorldSession::handleRequestAccountData, true, true, true, false, false);
    registry.registerOpcode(CMSG_TOGGLE_PVP, &WorldSession::handleTogglePVPOpcode, true, true, true, true, false);

    // Faction / Reputation
    registry.registerOpcode(CMSG_SET_FACTION_ATWAR, &WorldSession::handleSetFactionAtWarOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_WATCHED_FACTION, &WorldSession::handleSetWatchedFactionIndexOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_FACTION_INACTIVE, &WorldSession::handleSetFactionInactiveOpcode, true, true, true, false, false);

    // Player Interaction
    registry.registerOpcode(CMSG_GAMEOBJ_USE, &WorldSession::handleGameObjectUse, true, true, true, true, false);
    registry.registerOpcode(CMSG_REQUEST_PLAYED_TIME, &WorldSession::handlePlayedTimeOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SETSHEATHED, &WorldSession::handleSetSheathedOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT, &WorldSession::handleMessageChatOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_EMOTE, &WorldSession::handleEmoteOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_TEXT_EMOTE, &WorldSession::handleTextEmoteOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_INSPECT, &WorldSession::handleInspectOpcode, true, true, true, true, false);
    // clearly wrong naming!
    //registry.registerOpcode(SMSG_BARBER_SHOP_RESULT, &WorldSession::handleBarberShopResult, false, false, true, false, false);

    // Channels
    registry.registerOpcode(CMSG_JOIN_CHANNEL, &WorldSession::handleChannelJoin, false, true, true, true, false);
    registry.registerOpcode(CMSG_LEAVE_CHANNEL, &WorldSession::handleChannelLeave, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_LIST, &WorldSession::handleChannelList, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_PASSWORD, &WorldSession::handleChannelPassword, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_SET_OWNER, &WorldSession::handleChannelSetOwner, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_OWNER, &WorldSession::handleChannelOwner, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_MODERATOR, &WorldSession::handleChannelModerator, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_UNMODERATOR, &WorldSession::handleChannelUnmoderator, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_MUTE, &WorldSession::handleChannelMute, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_UNMUTE, &WorldSession::handleChannelUnmute, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_INVITE, &WorldSession::handleChannelInvite, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_KICK, &WorldSession::handleChannelKick, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_BAN, &WorldSession::handleChannelBan, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_UNBAN, &WorldSession::handleChannelUnban, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_ANNOUNCEMENTS, &WorldSession::handleChannelAnnounce, true, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_MODERATE, &WorldSession::handleChannelModerate, true, true, true, true, false);
    registry.registerOpcode(CMSG_GET_CHANNEL_MEMBER_COUNT, &WorldSession::handleGetChannelMemberCount, false, true, true, true, false);
    registry.registerOpcode(CMSG_CHANNEL_DISPLAY_LIST, &WorldSession::handleChannelRosterQuery, true, true, true, true, false);

    // Groups / Raids
    registry.registerOpcode(CMSG_GROUP_INVITE, &WorldSession::handleGroupInviteOpcode, true, true, true, true, false);
    //registry.registerOpcode(CMSG_GROUP_CANCEL, &WorldSession::HandleGroupCancelOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_GROUP_ACCEPT, &WorldSession::handleGroupAcceptOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_DECLINE, &WorldSession::handleGroupDeclineOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_UNINVITE, &WorldSession::handleGroupUninviteOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_UNINVITE_GUID, &WorldSession::handleGroupUninviteGuidOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_GROUP_SET_LEADER, &WorldSession::handleGroupSetLeaderOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_GROUP_DISBAND, &WorldSession::handleGroupDisbandOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LOOT_METHOD, &WorldSession::handleLootMethodOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_MINIMAP_PING, &WorldSession::handleMinimapPingOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_RAID_CONVERT, &WorldSession::handleConvertGroupToRaidOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_GROUP_CHANGE_SUB_GROUP, &WorldSession::handleGroupChangeSubGroup, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_ASSISTANT_LEADER, &WorldSession::handleGroupAssistantLeader, true, true, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_RAID_INFO, &WorldSession::handleRequestRaidInfoOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_RAID_READY_CHECK, &WorldSession::handleReadyCheckOpcode, true, false, true, true, false);
    registry.registerOpcode(MSG_RAID_TARGET_UPDATE, &WorldSession::handleSetPlayerIconOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_PARTY_MEMBER_STATS, &WorldSession::handlePartyMemberStatsOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_PARTY_ASSIGNMENT, &WorldSession::handleGroupPromote, true, true, true, false, false);

    // LFG System
    registry.registerOpcode(CMSG_SET_LFG_COMMENT, &WorldSession::handleLfgSetCommentOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LFG_JOIN, &WorldSession::handleLfgJoinOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_LFG_LEAVE, &WorldSession::handleLfgLeaveOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_SEARCH_LFG_JOIN, &WorldSession::handleLfgSearchOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_SEARCH_LFG_LEAVE, &WorldSession::handleLfgSearchLeaveOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_LFG_PROPOSAL_RESULT, &WorldSession::handleLfgProposalResultOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_LFG_SET_ROLES, &WorldSession::handleLfgSetRolesOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_LFG_SET_BOOT_VOTE, &WorldSession::handleLfgSetBootVoteOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_LFD_PLAYER_LOCK_INFO_REQUEST, &WorldSession::handleLfgPlayerLockInfoRequestOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_LFG_TELEPORT, &WorldSession::handleLfgTeleportOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_LFD_PARTY_LOCK_INFO_REQUEST, &WorldSession::handleLfgPartyLockInfoRequestOpcode, false, false, true, false, false);

    // Taxi / NPC Interaction
    registry.registerOpcode(CMSG_ENABLETAXI, &WorldSession::handleEnabletaxiOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_TAXINODE_STATUS_QUERY, &WorldSession::handleTaxiNodeStatusQueryOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_TAXIQUERYAVAILABLENODES, &WorldSession::handleTaxiQueryAvaibleNodesOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_ACTIVATE_TAXI, &WorldSession::handleActivateTaxiOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_TABARDVENDOR_ACTIVATE, &WorldSession::handleTabardVendorActivateOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BANKER_ACTIVATE, &WorldSession::handleBankerActivateOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BUY_BANK_SLOT, &WorldSession::handleBuyBankSlotOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_TRAINER_LIST, &WorldSession::handleTrainerListOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_TRAINER_BUY_SPELL, &WorldSession::handleTrainerBuySpellOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_PETITION_SHOWLIST, &WorldSession::handleCharterShowListOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_AUCTION_HELLO, &WorldSession::handleAuctionHelloOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_GOSSIP_HELLO, &WorldSession::handleGossipHelloOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_GOSSIP_SELECT_OPTION, &WorldSession::handleGossipSelectOptionOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SPIRIT_HEALER_ACTIVATE, &WorldSession::handleSpiritHealerActivateOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_NPC_TEXT_QUERY, &WorldSession::handleNpcTextQueryOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BINDER_ACTIVATE, &WorldSession::handleBinderActivateOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_ACTIVATE_TAXI_EXPRESS, &WorldSession::handleMultipleActivateTaxiOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MOVE_SPLINE_DONE, &WorldSession::handleMoveSplineDoneOpcode, true, true, true, true, false);
    // Item / Vendors
    registry.registerOpcode(CMSG_SWAP_INV_ITEM, &WorldSession::handleSwapInvItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SWAP_ITEM, &WorldSession::handleSwapItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_DESTROY_ITEM, &WorldSession::handleDestroyItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUTOEQUIP_ITEM, &WorldSession::handleAutoEquipItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUTOEQUIP_ITEM_SLOT, &WorldSession::handleAutoEquipItemSlotOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_ITEM_QUERY_SINGLE, &WorldSession::handleItemQuerySingleOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SELL_ITEM, &WorldSession::handleSellItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BUY_ITEM_IN_SLOT, &WorldSession::handleBuyItemInSlotOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BUY_ITEM, &WorldSession::handleBuyItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LIST_INVENTORY, &WorldSession::handleListInventoryOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUTOSTORE_BAG_ITEM, &WorldSession::handleAutoStoreBagItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SET_AMMO, &WorldSession::handleAmmoSetOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUY_BACK_ITEM, &WorldSession::handleBuyBackOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SPLIT_ITEM, &WorldSession::handleSplitOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_READ_ITEM, &WorldSession::handleReadItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_REPAIR_ITEM, &WorldSession::handleRepairItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUTOBANK_ITEM, &WorldSession::handleAutoBankItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUTOSTORE_BANK_ITEM, &WorldSession::handleAutoStoreBankItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_CANCEL_TEMP_ENCHANTMENT, &WorldSession::handleCancelTemporaryEnchantmentOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SOCKET_GEMS, &WorldSession::handleInsertGemOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_WRAP_ITEM, &WorldSession::handleWrapItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ITEMREFUNDINFO, &WorldSession::handleItemRefundInfoOpcode, false, false, true, true, false);
    registry.registerOpcode(CMSG_ITEMREFUNDREQUEST, &WorldSession::handleItemRefundRequestOpcode, false, false, true, false, false);

    registry.registerOpcode(CMSG_EQUIPMENT_SET_SAVE, &WorldSession::handleEquipmentSetSave, false, false, true, false, false);
    registry.registerOpcode(CMSG_EQUIPMENT_SET_USE, &WorldSession::handleEquipmentSetUse, false, false, true, false, false);
    registry.registerOpcode(CMSG_EQUIPMENT_SET_DELETE, &WorldSession::handleEquipmentSetDelete, false, false, true, false, false);

    // Spell System / Talent System
    registry.registerOpcode(CMSG_USE_ITEM, &WorldSession::handleUseItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_CAST_SPELL, &WorldSession::handleCastSpellOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SPELL_CLICK, &WorldSession::handleSpellClick, true, false, true, true, false);
    registry.registerOpcode(CMSG_CANCEL_CAST, &WorldSession::handleCancelCastOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_CANCEL_AURA, &WorldSession::handleCancelAuraOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_CANCEL_CHANNELLING, &WorldSession::handleCancelChannellingOpcode, true, false, true, true, false);
    registry.registerOpcode(CMSG_CANCEL_AUTO_REPEAT_SPELL, &WorldSession::handleCancelAutoRepeatSpellOpcode, true, false, true, true, false);
    registry.registerOpcode(CMSG_TOTEM_DESTROYED, &WorldSession::handleCancelTotem, true, true, true, true, false);
    registry.registerOpcode(CMSG_LEARN_TALENT, &WorldSession::handleLearnTalentOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LEARN_TALENTS_MULTIPLE, &WorldSession::handleLearnMultipleTalentsOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_UNLEARN_TALENTS, &WorldSession::handleUnlearnTalents, true, false, true, false, false);
    registry.registerOpcode(MSG_TALENT_WIPE_CONFIRM, &WorldSession::handleUnlearnTalents, true, false, true, true, false);
    registry.registerOpcode(CMSG_UPDATE_PROJECTILE_POSITION, &WorldSession::handleUpdateProjectilePosition, false, false, true, true, false);
    // Combat / Duel
    registry.registerOpcode(CMSG_ATTACKSWING, &WorldSession::handleAttackSwingOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_ATTACK_STOP, &WorldSession::handleAttackStopOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_DUEL_ACCEPTED, &WorldSession::handleDuelAccepted, true, true, true, true, false);
    registry.registerOpcode(CMSG_DUEL_CANCELLED, &WorldSession::handleDuelCancelled, true, true, true, true, false);

    // Trade
    registry.registerOpcode(CMSG_INITIATE_TRADE, &WorldSession::handleInitiateTradeOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BEGIN_TRADE, &WorldSession::handleBeginTradeOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BUSY_TRADE, &WorldSession::handleBusyTrade, true, true, true, false, false);
    registry.registerOpcode(CMSG_IGNORE_TRADE, &WorldSession::handleIgnoreTrade, true, true, true, false, false);
    registry.registerOpcode(CMSG_ACCEPT_TRADE, &WorldSession::handleAcceptTrade, true, true, true, true, false);
    registry.registerOpcode(CMSG_UNACCEPT_TRADE, &WorldSession::handleUnacceptTrade, true, true, true, false, false);
    registry.registerOpcode(CMSG_CANCEL_TRADE, &WorldSession::handleCancelTrade, true, true, true, true, false);
    registry.registerOpcode(CMSG_SET_TRADE_ITEM, &WorldSession::handleSetTradeItem, true, true, true, true, false);
    registry.registerOpcode(CMSG_CLEAR_TRADE_ITEM, &WorldSession::handleClearTradeItem, true, true, true, true, false);
    registry.registerOpcode(CMSG_SET_TRADE_GOLD, &WorldSession::handleSetTradeGold, true, true, true, true, false);

    // Quest System
    registry.registerOpcode(CMSG_QUEST_GIVER_STATUS_MULTIPLE_QUERY, &WorldSession::handleInrangeQuestgiverQuery, false, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTGIVER_STATUS_QUERY, &WorldSession::handleQuestgiverStatusQueryOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTGIVER_HELLO, &WorldSession::handleQuestgiverHelloOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTGIVER_ACCEPT_QUEST, &WorldSession::handleQuestgiverAcceptQuestOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTGIVER_CANCEL, &WorldSession::handleQuestgiverCancelOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTGIVER_CHOOSE_REWARD, &WorldSession::handleQuestgiverChooseRewardOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTGIVER_REQUEST_REWARD, &WorldSession::handleQuestgiverRequestRewardOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUEST_QUERY, &WorldSession::handleQuestQueryOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTGIVER_QUERY_QUEST, &WorldSession::handleQuestGiverQueryQuestOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTGIVER_COMPLETE_QUEST, &WorldSession::handleQuestgiverCompleteQuestOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_QUESTLOG_REMOVE_QUEST, &WorldSession::handleQuestlogRemoveQuestOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_RECLAIM_CORPSE, &WorldSession::handleCorpseReclaimOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_RESURRECT_RESPONSE, &WorldSession::handleResurrectResponse, true, true, true, false, false);
    registry.registerOpcode(CMSG_PUSHQUESTTOPARTY, &WorldSession::handlePushQuestToPartyOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_QUEST_PUSH_RESULT, &WorldSession::handleQuestPushResultOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUEST_POI_QUERY, &WorldSession::handleQuestPOIQueryOpcode, false, false, true, true, false);

    // Auction System
    registry.registerOpcode(CMSG_AUCTION_LIST_ITEMS, &WorldSession::handleAuctionListItems, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUCTION_LIST_BIDDER_ITEMS, &WorldSession::handleAuctionListBidderItems, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUCTION_SELL_ITEM, &WorldSession::handleAuctionSellItem, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUCTION_LIST_OWNER_ITEMS, &WorldSession::handleAuctionListOwnerItems, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUCTION_PLACE_BID, &WorldSession::handleAuctionPlaceBid, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUCTION_REMOVE_ITEM, &WorldSession::handleCancelAuction, true, true, true, true, false);
    registry.registerOpcode(CMSG_AUCTION_LIST_PENDING_SALES, &WorldSession::handleAuctionListPendingSales, false, true, true, true, false);

    // Mail System
    registry.registerOpcode(CMSG_GET_MAIL_LIST, &WorldSession::handleGetMailOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_ITEM_TEXT_QUERY, &WorldSession::handleItemTextQueryOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SEND_MAIL, &WorldSession::handleSendMailOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MAIL_TAKE_MONEY, &WorldSession::handleTakeMoneyOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MAIL_TAKE_ITEM, &WorldSession::handleTakeItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MAIL_MARK_AS_READ, &WorldSession::handleMarkAsReadOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MAIL_RETURN_TO_SENDER, &WorldSession::handleReturnToSenderOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MAIL_DELETE, &WorldSession::handleMailDeleteOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_QUERY_NEXT_MAIL_TIME, &WorldSession::handleMailTimeOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_MAIL_CREATE_TEXT_ITEM, &WorldSession::handleMailCreateTextItemOpcode, true, true, true, true, false);

    // Guild Query (called when not logged in sometimes)
    registry.registerOpcode<STATUS_AUTHED>(CMSG_GUILD_QUERY, &WorldSession::handleGuildQuery, true, true, true, true, false);

    // Guild System
    //registry.registerOpcode(CMSG_GUILD_CREATE, &WorldSession::HandleCreateGuild, false, false, true, false, false);
    registry.registerOpcode(CMSG_GUILD_INVITE, &WorldSession::handleInviteToGuild, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_ACCEPT, &WorldSession::handleGuildAccept, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_DECLINE, &WorldSession::handleGuildDecline, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_INFO, &WorldSession::handleGuildInfo, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_ROSTER, &WorldSession::handleGuildRoster, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_PROMOTE, &WorldSession::handleGuildPromote, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_DEMOTE, &WorldSession::handleGuildDemote, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_LEAVE, &WorldSession::handleGuildLeave, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_REMOVE, &WorldSession::handleGuildRemove, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_DISBAND, &WorldSession::handleGuildDisband, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_LEADER, &WorldSession::handleGuildLeader, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_MOTD, &WorldSession::handleGuildMotd, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_SET_RANK, &WorldSession::handleGuildSetRank, true, false, true, true, false);
    registry.registerOpcode(CMSG_GUILD_ADD_RANK, &WorldSession::handleGuildAddRank, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_DEL_RANK, &WorldSession::handleGuildDelRank, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_SET_PUBLIC_NOTE, &WorldSession::handleGuildSetPublicNote, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_SET_OFFICER_NOTE, &WorldSession::handleGuildSetOfficerNote, true, true, true, false, false);
    registry.registerOpcode(CMSG_PETITION_BUY, &WorldSession::handleCharterBuy, true, true, true, true, false);
    registry.registerOpcode(CMSG_PETITION_SHOW_SIGNATURES, &WorldSession::handleCharterShowSignatures, true, true, true, true, false);
    registry.registerOpcode(CMSG_TURN_IN_PETITION, &WorldSession::handleCharterTurnInCharter, true, true, true, true, false);
    registry.registerOpcode(CMSG_PETITION_QUERY, &WorldSession::handleCharterQuery, true, true, true, true, false);
    registry.registerOpcode(CMSG_OFFER_PETITION, &WorldSession::handleCharterOffer, true, true, true, true, false);
    registry.registerOpcode(CMSG_PETITION_SIGN, &WorldSession::handleCharterSign, true, true, true, true, false);
    registry.registerOpcode(MSG_PETITION_DECLINE, &WorldSession::handleCharterDecline, true, true, true, true, false);
    registry.registerOpcode(MSG_PETITION_RENAME, &WorldSession::handleCharterRename, true, true, true, true, false);
    registry.registerOpcode(MSG_SAVE_GUILD_EMBLEM, &WorldSession::handleSaveGuildEmblem, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_INFO_TEXT, &WorldSession::handleSetGuildInfo, true, true, true, true, false);
    registry.registerOpcode(MSG_QUERY_GUILD_BANK_TEXT, &WorldSession::handleGuildBankQueryText, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_GUILD_BANK_TEXT, &WorldSession::handleSetGuildBankText, true, true, true, true, false);
    registry.registerOpcode(MSG_GUILD_EVENT_LOG_QUERY, &WorldSession::handleGuildLog, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANKER_ACTIVATE, &WorldSession::handleGuildBankerActivate, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_BANK_BUY_TAB, &WorldSession::handleGuildBankBuyTab, true, true, true, true, false);
    registry.registerOpcode(MSG_GUILD_BANK_MONEY_WITHDRAWN, &WorldSession::handleGuildBankMoneyWithdrawn, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANK_UPDATE_TAB, &WorldSession::handleGuildBankUpdateTab, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_BANK_SWAP_ITEMS, &WorldSession::handleGuildBankSwapItems, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_BANK_WITHDRAW_MONEY, &WorldSession::handleGuildBankWithdrawMoney, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_BANK_DEPOSIT_MONEY, &WorldSession::handleGuildBankDepositMoney, true, true, true, true, false);
    registry.registerOpcode(CMSG_GUILD_BANK_QUERY_TAB, &WorldSession::handleGuildBankQueryTab, true, true, true, true, false);
    registry.registerOpcode(MSG_GUILD_BANK_LOG_QUERY, &WorldSession::handleGuildBankLogQuery, true, true, true, true, false);
    registry.registerOpcode(MSG_GUILD_PERMISSIONS, &WorldSession::handleGuildPermissions, true, true, true, false, false);

    // Tutorials
    registry.registerOpcode(CMSG_TUTORIAL_FLAG, &WorldSession::handleTutorialFlag, true, true, true, true, false);
    registry.registerOpcode(CMSG_TUTORIAL_CLEAR, &WorldSession::handleTutorialClear, true, true, true, true, false);
    registry.registerOpcode(CMSG_TUTORIAL_RESET, &WorldSession::handleTutorialReset, true, true, true, true, false);

    // Pets
    registry.registerOpcode(MSG_LIST_STABLED_PETS, &WorldSession::handleStabledPetList, true, true, true, false, false);

    registry.registerOpcode(CMSG_PET_ACTION, &WorldSession::handlePetAction, true, true, true, true, false);
    registry.registerOpcode(CMSG_PET_NAME_QUERY, &WorldSession::handlePetNameQuery, true, true, true, true, false);
    registry.registerOpcode(CMSG_BUY_STABLE_SLOT, &WorldSession::handleBuyStableSlot, true, true, true, false, false);
    registry.registerOpcode(CMSG_STABLE_PET, &WorldSession::handleStablePet, true, true, true, false, false);
    registry.registerOpcode(CMSG_UNSTABLE_PET, &WorldSession::handleUnstablePet, true, true, true, false, false);
    registry.registerOpcode(CMSG_STABLE_SWAP_PET, &WorldSession::handleStableSwapPet, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_SET_ACTION, &WorldSession::handlePetSetActionOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_RENAME, &WorldSession::handlePetRename, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_ABANDON, &WorldSession::handlePetAbandon, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_UNLEARN, &WorldSession::handlePetUnlearn, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_SPELL_AUTOCAST, &WorldSession::handlePetSpellAutocast, true, true, true, true, false);
    registry.registerOpcode(CMSG_PET_CANCEL_AURA, &WorldSession::handlePetCancelAura, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_LEARN_TALENT, &WorldSession::handlePetLearnTalent, false, false, true, true, false);
    registry.registerOpcode(CMSG_DISMISS_CRITTER, &WorldSession::handleDismissCritter, false, false, true, false, false);

    // Battlegrounds
    registry.registerOpcode(CMSG_BATTLEFIELD_PORT, &WorldSession::handleBattlefieldPortOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BATTLEFIELD_STATUS, &WorldSession::handleBattlefieldStatusOpcode, true, true, true, true, false);
    registry.registerOpcode<STATUS_LOGGEDIN>(CMSG_BATTLEFIELD_LIST, &WorldSession::handleBattlefieldListOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BATTLEMASTER_HELLO, &WorldSession::handleBattleMasterHelloOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_BATTLEMASTER_JOIN_ARENA, &WorldSession::handleArenaJoinOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BATTLEMASTER_JOIN, &WorldSession::handleBattleMasterJoinOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_LEAVE_BATTLEFIELD, &WorldSession::handleLeaveBattlefieldOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_AREA_SPIRIT_HEALER_QUERY, &WorldSession::handleAreaSpiritHealerQueryOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_AREA_SPIRIT_HEALER_QUEUE, &WorldSession::handleAreaSpiritHealerQueueOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_BATTLEGROUND_PLAYER_POSITIONS, &WorldSession::handleBattlegroundPlayerPositionsOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_PVP_LOG_DATA, &WorldSession::handlePVPLogDataOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_INSPECT_HONOR_STATS, &WorldSession::handleInspectHonorStatsOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SET_ACTIONBAR_TOGGLES, &WorldSession::handleSetActionBarTogglesOpcode, true, true, true, true, false);
    //registry.registerOpcode(CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE, &WorldSession::HandleBgInviteResponse, false, false, true, false, false);

    // GM Ticket System
    registry.registerOpcode(CMSG_GMTICKET_CREATE, &WorldSession::handleGMTicketCreateOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_GMTICKET_UPDATETEXT, &WorldSession::handleGMTicketUpdateOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_GMTICKET_DELETETICKET, &WorldSession::handleGMTicketDeleteOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_GMTICKET_GETTICKET, &WorldSession::handleGMTicketGetTicketOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_GMTICKET_SYSTEMSTATUS, &WorldSession::handleGMTicketSystemStatusOpcode, false, true, true, true, false);
    registry.registerOpcode(CMSG_GMTICKETSYSTEM_TOGGLE, &WorldSession::handleGMTicketToggleSystemStatusOpcode, false, true, true, false, false);

    // Lag report
    registry.registerOpcode(CMSG_GM_REPORT_LAG, &WorldSession::handleReportLag, false, false, true, false, false);
    registry.registerOpcode(CMSG_GMSURVEY_SUBMIT, &WorldSession::handleGMSurveySubmitOpcode, false, false, true, false, false);

    // Meeting Stone / Instances
    registry.registerOpcode(CMSG_SUMMON_RESPONSE, &WorldSession::handleSummonResponseOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_RESET_INSTANCES, &WorldSession::handleResetInstanceOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_SELF_RES, &WorldSession::handleSelfResurrect, true, true, true, false, false);
    registry.registerOpcode(MSG_RANDOM_ROLL, &WorldSession::handleRandomRollOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_SET_DUNGEON_DIFFICULTY, &WorldSession::handleDungeonDifficultyOpcode, true, true, true, true, false);
    registry.registerOpcode(MSG_SET_RAID_DIFFICULTY, &WorldSession::handleRaidDifficultyOpcode, false, false, true, true, false);
    registry.registerOpcode(CMSG_INSTANCE_LOCK_RESPONSE, &WorldSession::handleInstanceLockResponse, false, false, true, false, false);
    registry.registerOpcode(CMSG_VIOLENCE_LEVEL, &WorldSession::handleViolenceLevel, false, false, false, true, false);

    // Misc
    registry.registerOpcode(CMSG_OPEN_ITEM, &WorldSession::handleOpenItemOpcode, true, true, true, true, false);
    registry.registerOpcode(CMSG_COMPLETE_CINEMATIC, &WorldSession::handleCompleteCinematic, true, true, true, true, false);
    registry.registerOpcode(CMSG_NEXT_CINEMATIC_CAMERA, &WorldSession::handleNextCinematic, true, false, true, true, false);
    registry.registerOpcode(CMSG_MOUNTSPECIAL_ANIM, &WorldSession::handleMountSpecialAnimOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_TOGGLE_CLOAK, &WorldSession::handleToggleCloakOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_TOGGLE_HELM, &WorldSession::handleToggleHelmOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_TITLE, &WorldSession::handleSetTitle, true, true, true, false, false);
    registry.registerOpcode(CMSG_COMPLAIN, &WorldSession::handleReportSpamOpcode, true, false, true, false, false);
    registry.registerOpcode(CMSG_GAMEOBJ_REPORT_USE, &WorldSession::handleGameobjReportUseOpCode, false, false, true, true, false);
    registry.registerOpcode(CMSG_PET_CAST_SPELL, &WorldSession::handlePetCastSpell, true, true, true, true, false);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_WORLD_STATE_UI_TIMER_UPDATE, &WorldSession::handleWorldStateUITimerUpdate, false, false, true, true, false);
    registry.registerOpcode(CMSG_SET_TAXI_BENCHMARK_MODE, &WorldSession::handleSetTaxiBenchmarkOpcode, true, false, true, true, false);
    registry.registerOpcode(CMSG_UNLEARN_SKILL, &WorldSession::handleUnlearnSkillOpcode, true, true, true, true, false);

    // Chat
    registry.registerOpcode(CMSG_CHAT_IGNORED, &WorldSession::handleChatIgnoredOpcode, true, false, true, true, false);
    registry.registerOpcode(CMSG_SET_CHANNEL_WATCH, &WorldSession::handleChatChannelWatchOpcode, true, false, true, true, false);

    // Arenas
    registry.registerOpcode(CMSG_ARENA_TEAM_QUERY, &WorldSession::handleArenaTeamQueryOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ARENA_TEAM_ROSTER, &WorldSession::handleArenaTeamRosterOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ARENA_TEAM_INVITE, &WorldSession::handleArenaTeamAddMemberOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ARENA_TEAM_ACCEPT, &WorldSession::handleArenaTeamInviteAcceptOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ARENA_TEAM_DECLINE, &WorldSession::handleArenaTeamInviteDenyOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ARENA_TEAM_LEAVE, &WorldSession::handleArenaTeamLeaveOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ARENA_TEAM_REMOVE, &WorldSession::handleArenaTeamRemoveMemberOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ARENA_TEAM_DISBAND, &WorldSession::handleArenaTeamDisbandOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ARENA_TEAM_LEADER, &WorldSession::handleArenaTeamPromoteOpcode, false, true, true, false, false);
    registry.registerOpcode(MSG_INSPECT_ARENA_TEAMS, &WorldSession::handleInspectArenaStatsOpcode, false, true, true, false, false);

    // cheat/gm commands?
    registry.registerOpcode(CMSG_WORLD_TELEPORT, &WorldSession::handleWorldTeleportOpcode, true, true, true, false, false);

    // Vehicle
    registry.registerOpcode(CMSG_DISMISS_CONTROLLED_VEHICLE, &WorldSession::handleDismissVehicle, false, false, true, true, false);
    registry.registerOpcode(CMSG_REQUEST_VEHICLE_EXIT, &WorldSession::handleLeaveVehicle, false, false, true, true, false);
    registry.registerOpcode(CMSG_REQUEST_VEHICLE_PREV_SEAT, &WorldSession::handleRequestVehiclePreviousSeat, false, false, true, true, false);
    registry.registerOpcode(CMSG_REQUEST_VEHICLE_NEXT_SEAT, &WorldSession::handleRequestVehicleNextSeat, false, false, true, true, false);
    registry.registerOpcode(CMSG_REQUEST_VEHICLE_SWITCH_SEAT, &WorldSession::handleRequestVehicleSwitchSeat, false, false, true, true, false);
    registry.registerOpcode(CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, &WorldSession::handleChangeSeatsOnControlledVehicle, false, false, true, true, false);
    registry.registerOpcode(CMSG_PLAYER_VEHICLE_ENTER, &WorldSession::handleEnterVehicle, false, false, true, true, false);
    registry.registerOpcode(CMSG_EJECT_PASSENGER, &WorldSession::handleRemoveVehiclePassenger, false, false, true, true, false);

    // Unsorted
    registry.registerOpcode<STATUS_AUTHED>(CMSG_READY_FOR_ACCOUNT_DATA_TIMES, &WorldSession::handleReadyForAccountDataTimes, false, false, true, true, false);

    registry.registerOpcode(CMSG_OPT_OUT_OF_LOOT, &WorldSession::handleSetAutoLootPassOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_REMOVE_GLYPH, &WorldSession::handleRemoveGlyph, false, false, true, false, false);
    registry.registerOpcode(CMSG_ALTER_APPEARANCE, &WorldSession::handleBarberShopResult, false, false, true, true, false);
    registry.registerOpcode(CMSG_GET_MIRRORIMAGE_DATA, &WorldSession::HandleMirrorImageOpcode, true, false, true, false, false);

    // Calendar - Unhandled
    registry.registerOpcode(CMSG_CALENDAR_GET_CALENDAR, &WorldSession::handleCalendarGetCalendar, false, false, true, true, false);
    registry.registerOpcode(CMSG_CALENDAR_COMPLAIN, &WorldSession::handleCalendarComplain, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_GET_NUM_PENDING, &WorldSession::handleCalendarGetNumPending, false, false, true, true, false);
    registry.registerOpcode(CMSG_CALENDAR_ADD_EVENT, &WorldSession::handleCalendarAddEvent, false, false, true, false, false);

    registry.registerOpcode(CMSG_CALENDAR_GET_EVENT, &WorldSession::handleCalendarGetEvent, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_GUILD_FILTER, &WorldSession::handleCalendarGuildFilter, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_ARENA_TEAM, &WorldSession::handleCalendarArenaTeam, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_UPDATE_EVENT, &WorldSession::handleCalendarUpdateEvent, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_REMOVE_EVENT, &WorldSession::handleCalendarRemoveEvent, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_COPY_EVENT, &WorldSession::handleCalendarCopyEvent, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_EVENT_INVITE, &WorldSession::handleCalendarEventInvite, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_EVENT_RSVP, &WorldSession::handleCalendarEventRsvp, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_EVENT_REMOVE_INVITE, &WorldSession::handleCalendarEventRemoveInvite, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_EVENT_STATUS, &WorldSession::handleCalendarEventStatus, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_EVENT_MODERATOR_STATUS, &WorldSession::handleCalendarEventModeratorStatus, false, false, true, false, false);

    //Misc - Unhandled
    registry.registerOpcode(CMSG_FAR_SIGHT, &WorldSession::Unhandled, false, false, true, true, false);
    registry.registerOpcode(CMSG_LFG_GET_STATUS, &WorldSession::Unhandled, false, false, true, true, false);
    registry.registerOpcode(CMSG_VOICE_SESSION_ENABLE, &WorldSession::Unhandled, true, false, true, true, false);
    registry.registerOpcode(CMSG_SET_ACTIVE_VOICE_CHANNEL, &WorldSession::Unhandled, true, false, true, false, false);

    // new since cata
    registry.registerOpcode<STATUS_AUTHED>(CMSG_OBJECT_UPDATE_FAILED, &WorldSession::handleObjectUpdateFailedOpcode, false, false, false, true, false);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_LOADING_SCREEN_NOTIFY, &WorldSession::handleLoadScreenOpcode, false, false, false, true, true);
    registry.registerOpcode<STATUS_AUTHED>(CMSG_TIME_SYNC_RESPONSE, &WorldSession::handleTimeSyncRespOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MOVE_SET_CAN_FLY, &WorldSession::handleMovementOpcodes, false, false, false, true, false);
    registry.registerOpcode(CMSG_FORCE_PITCH_RATE_CHANGE_ACK, &WorldSession::handleAcknowledgementOpcodes, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_SAY, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_YELL, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_CHANNEL, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_WHISPER, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_GUILD, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_OFFICER, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_AFK, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_DND, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_EMOTE, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_PARTY, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_RAID, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_RAID_WARNING, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MESSAGECHAT_BATTLEGROUND, &WorldSession::handleMessageChatOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GROUP_INVITE_RESPONSE, &WorldSession::handleGroupInviteResponseOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GROUP_SET_ROLES, &WorldSession::handleGroupSetRolesOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GROUP_REQUEST_JOIN_UPDATES, &WorldSession::handleGroupRequestJoinUpdatesOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_ROLE_CHECK_BEGIN, &WorldSession::handleGroupRoleCheckBeginOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_MAKE_EVERYONE_ASSISTANT, &WorldSession::nothingToHandle, false, false, false, true, false);
    registry.registerOpcode(MSG_RAID_READY_CHECK_FINISHED, &WorldSession::nothingToHandle, false, false, false, true, false);
    registry.registerOpcode(CMSG_LFG_LOCK_INFO_REQUEST, &WorldSession::handleLfgLockInfoOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_TRANSMOGRIFY_ITEMS, &WorldSession::handleTransmogrifyItems, false, false, false, true, false);
    registry.registerOpcode(CMSG_REFORGE_ITEM, &WorldSession::handleReforgeItemOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_VOID_STORAGE_QUERY, &WorldSession::handleVoidStorageQuery, false, false, false, true, false);
    registry.registerOpcode(CMSG_VOID_STORAGE_TRANSFER, &WorldSession::handleVoidStorageTransfer, false, false, false, true, false);
    registry.registerOpcode(CMSG_VOID_STORAGE_UNLOCK, &WorldSession::handleVoidStorageUnlock, false, false, false, true, false);
    registry.registerOpcode(CMSG_VOID_SWAP_ITEM, &WorldSession::handleVoidSwapItem, false, false, false, true, false);
    registry.registerOpcode(CMSG_LEARN_PREVIEW_TALENTS, &WorldSession::handleLearnPreviewTalentsOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_BANK_MONEY_WITHDRAWN_QUERY, &WorldSession::handleGuildBankMoneyWithdrawn, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_BANK_QUERY_TEXT, &WorldSession::handleQueryGuildBankTabText, false, false, false, true, false);
    registry.registerOpcode(CMSG_QUERY_GUILD_XP, &WorldSession::handleGuildQueryXPOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_SET_NOTE, &WorldSession::handleGuildSetNoteOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_QUERY_GUILD_REWARDS, &WorldSession::handleGuildRewardsQueryOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_QUERY_RANKS, &WorldSession::handleGuildQueryRanksOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_ASSIGN_MEMBER_RANK, &WorldSession::handleGuildAssignRankOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_EVENT_LOG_QUERY, &WorldSession::handleGuildLog, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_REQUEST_CHALLENGE_UPDATE, &WorldSession::handleGuildRequestChallengeUpdate, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_REQUEST_MAX_DAILY_XP, &WorldSession::handleGuildRequestMaxDailyXP, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_QUERY_NEWS, &WorldSession::handleGuildQueryNewsOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_NEWS_UPDATE_STICKY, &WorldSession::handleGuildNewsUpdateStickyOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_GUILD_PERMISSIONS, &WorldSession::handleGuildPermissions, false, false, false, true, false);
    registry.registerOpcode(CMSG_LF_GUILD_GET_RECRUITS, &WorldSession::handleGuildFinderGetRecruits, false, false, false, true, false);
    registry.registerOpcode(CMSG_LF_GUILD_ADD_RECRUIT, &WorldSession::handleGuildFinderAddRecruit, false, false, false, true, false);
    registry.registerOpcode(CMSG_LF_GUILD_BROWSE, &WorldSession::handleGuildFinderBrowse, false, false, false, true, false);
    registry.registerOpcode(CMSG_LF_GUILD_DECLINE_RECRUIT, &WorldSession::handleGuildFinderDeclineRecruit, false, false, false, true, false);
    registry.registerOpcode(CMSG_LF_GUILD_GET_APPLICATIONS, &WorldSession::handleGuildFinderGetApplications, false, false, false, true, false);
    registry.registerOpcode(CMSG_LF_GUILD_POST_REQUEST, &WorldSession::handleGuildFinderPostRequest, false, false, false, true, false);
    registry.registerOpcode(CMSG_LF_GUILD_REMOVE_RECRUIT, &WorldSession::handleGuildFinderRemoveRecruit, false, false, false, true, false);
    registry.registerOpcode(CMSG_LF_GUILD_SET_GUILD_POST, &WorldSession::handleGuildFinderSetGuildPost, false, false, false, true, false);
    registry.registerOpcode(CMSG_REQUEST_RATED_BG_INFO, &WorldSession::handleRequestRatedBgInfoOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_REQUEST_RATED_BG_STATS, &WorldSession::handleRequestRatedBgStatsOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_REQUEST_PVP_REWARDS, &WorldSession::handleRequestPvPRewardsOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_REQUEST_PVP_OPTIONS_ENABLED, &WorldSession::handleRequestPvpOptionsOpcode, false, false, false, true, false);
    registry.registerOpcode(SMSG_GMTICKET_GETTICKET, &WorldSession::handleGMTicketToggleSystemStatusOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_REPORT, &WorldSession::handleReportOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_REPORT_PLAYER, &WorldSession::handleReportPlayerOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_REQUEST_CEMETERY_LIST, &WorldSession::handleRequestCemeteryListOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_REQUEST_HOTFIX, &WorldSession::handleRequestHotfix, false, false, false, true, false);
    registry.registerOpcode(CMSG_RETURN_TO_GRAVEYARD, &WorldSession::handleReturnToGraveyardOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_SUGGESTION_SUBMIT, &WorldSession::handleSuggestionOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_LOG_DISCONNECT, &WorldSession::handleLogDisconnectOpcode, false, false, false, true, false);
    registry.registerOpcode(CMSG_PET_LEVEL_CHEAT, &WorldSession::Unhandled, false, false, false, true, false);
    registry.registerOpcode(CMSG_QUERY_BATTLEFIELD_STATE, &WorldSession::Unhandled, false, false, false, true, false);
    registry.registerOpcode(CMSG_ADDON_REGISTERED_PREFIXES, &WorldSession::handleAddonRegisteredPrefixesOpcode, false, false, false, true, true);
    registry.registerOpcode(CMSG_UNREGISTER_ALL_ADDON_PREFIXES, &WorldSession::handleUnregisterAddonPrefixesOpcode, false, false, false, true, true);
}
