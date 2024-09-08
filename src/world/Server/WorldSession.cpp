/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
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
#include "ThreadSafeQueue.hpp"
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

OpcodeHandler WorldPacketHandlers[NUM_OPCODES];

WorldSession::WorldSession(uint32 id, std::string name, WorldSocket* sock) :
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
    m_muted(0)
{
#if VERSION_STRING >= Cata
    isAddonMessageFiltered = false;
#endif

    for (uint8 x = 0; x < 8; x++)
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

    std::shared_ptr<WorldPacket> packet;

    while ((packet = _recvQueue.pop()) != nullptr)
    {
    }

    for (uint32 x = 0; x < 8; x++)
    {
        delete[]sAccountData[x].data;
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

    if (InstanceID != instanceId)
    {
        // We're being updated by the wrong thread.
        // "Remove us!" - 2
        return 2;
    }

    // Socket disconnection.
    if (!_socket)
    {
        // Check if the player is in the process of being moved. We can't
        // delete him
        // if we are.
        if (_player && _player->m_beingPushed)
        {
            // Abort..
            return 0;
        }

        if (!_logoutTime)
            _logoutTime = m_currMsTime + PLAYER_LOGOUT_DELAY;

        /*
           if (_player && _player->m_duelPlayer)
           _player->endDuel(DUEL_WINNER_RETREAT);

           bDeleted = true; LogoutPlayer(true); // 1 - Delete session
           completely. return 1; */

    }

    std::shared_ptr<WorldPacket> packet;

    while ((packet = _recvQueue.pop()) != nullptr)
    {
        if (packet != nullptr)
        {
            uint16_t rawOpcode = packet->GetOpcode();
            uint32_t internalId = sOpcodeTables.getInternalIdForHex(rawOpcode);
            std::string opcodeName = sOpcodeTables.getNameForOpcode(rawOpcode);
            int currentVersion = sOpcodeTables.getVersionIdForAEVersion();

            // Step 1: Try handling with the new system
            bool handledByNewSystem = OpcodeHandlerRegistry::instance().handleOpcode(*this, *packet);
            if (handledByNewSystem)
                sLogger.info("{} : handled by opcode registry :-)", opcodeName);

            // Step 2: If not handled by the new system, fall back to the old system
            if (!handledByNewSystem)
            {
                if (internalId >= NUM_OPCODES)
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "[Session] Received out of range packet with opcode 0x{:04X}", rawOpcode);
                }
                else
                {
                    OpcodeHandler* handler = &WorldPacketHandlers[internalId];
                    if (handler->status == STATUS_LOGGEDIN && !_player && handler->handler != 0)
                    {
                        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "[Session] Received unexpected/wrong state packet with opcode {} (0x{:04X})",
                            opcodeName, rawOpcode);
                    }
                    else
                    {
                        // Valid Packet :>
                        if (handler->handler == 0)
                        {
                            sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "[Session] Received unhandled packet with opcode {} (0x{:04X})",
                                opcodeName, rawOpcode);
                        }
                        else
                        {
                            (this->*handler->handler)(*packet);
                        }
                    }
                }
            }

            packet = nullptr;

            if (InstanceID != instanceId)
            {
                // If we hit this -> means a packet has changed our map.
                return 2;
            }

            if (bDeleted)
            {
                return 1;
            }
        }
    }

    if (InstanceID != instanceId)
    {
        // If we hit this -> means a packet has changed our map.
        return 2;
    }

    if (_logoutTime && (m_currMsTime >= _logoutTime) && instanceId == InstanceID)
    {
        // Check if the player is in the process of being moved. We can't
        // delete him
        // if we are.
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

    if (m_lastPing + WORLDSOCKET_TIMEOUT < static_cast<uint32>(UNIXTIME))
    {
        // Check if the player is in the process of being moved. We can't
        // delete him
        // if we are.
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

        m_lastPing = static_cast<uint32>(UNIXTIME); // Prevent calling this code over and
        // over.
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


        _player->getSummonInterface()->removeAllSummons();

        _player->dismissActivePets();

        // _player->SaveAuras();

        if (Save)
            _player->saveToDB(false);

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
            for (uint32 ui = 0; ui < 8; ui++)
            {
                if (sAccountData[ui].bIsDirty)
                {
                    if (dirty)
                        ss << ",";
                    ss << "uiconfig" << ui << "=\"";

                    if (sAccountData[ui].data)
                    {
                        CharacterDatabase.EscapeLongString(sAccountData[ui].data, sAccountData[ui].sz, ss);
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

AccountDataEntry* WorldSession::GetAccountData(uint32 index)
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

void WorldSession::InitPacketHandlerTable()
{
    // Nullify Everything, default to STATUS_LOGGEDIN
    for (auto& WorldPacketHandler : WorldPacketHandlers)
    {
        WorldPacketHandler.status = STATUS_LOGGEDIN;
        WorldPacketHandler.handler = nullptr;
    }

    registerOpcodeHandler();
    loadHandlers();
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

void WorldSession::SendChatPacket(WorldPacket* data, uint32 langpos, int32 lang, WorldSession* originator)
{
    if (lang == -1)
        *reinterpret_cast<uint32*>(& data->contents()[langpos]) = lang;
    else
    {
        if (CanUseCommand('c') || (originator && originator->CanUseCommand('c')))
            *reinterpret_cast<uint32*>(& data->contents()[langpos]) = LANG_UNIVERSAL;
        else
            *reinterpret_cast<uint32*>(& data->contents()[langpos]) = lang;
    }

    SendPacket(data);
}

void WorldSession::Delete()
{
    delete this;
}

/*
   2008/10/04 MultiLanguages on each player session. LocalizedWorldSrv
   translating core message from sql. LocalizedMapName translating MAP Title
   from sql. LocalizedBroadCast translating new broadcast system from sql.
   Full merged from p2wow 's branch (p2branch). cebernic@gmail.com */

char szError[64];


// Returns a gossip menu option indexed by id
// These strings can be found in gossip_menu_option tables in the database
const char* WorldSession::LocalizedGossipOption(uint32 id)
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
const char* WorldSession::LocalizedWorldSrv(uint32 id)
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

const char* WorldSession::LocalizedMapName(uint32 id)
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

const char* WorldSession::LocalizedBroadCast(uint32 id)
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

void WorldSession::OutPacket(uint16 opcode)
{
    if (_socket && _socket->IsConnected())
    {
        _socket->OutPacket(opcode, 0, nullptr);
    }
}

void WorldSession::OutPacket(uint16 opcode, uint16 len, const void* data)
{
    if (_socket && _socket->IsConnected())
    {
        _socket->OutPacket(opcode, len, data);
    }
}

void WorldSession::QueuePacket(std::shared_ptr<WorldPacket> packet)
{
    m_lastPing = static_cast<uint32>(UNIXTIME);
    _recvQueue.push(packet);
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
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_ENUM_CHARACTERS, &WorldSession::handleCharEnumOpcode, true, true, true, true, false);
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_CHAR_CREATE, &WorldSession::handleCharCreateOpcode, true, true, true, true, false);
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_CHAR_DELETE, &WorldSession::handleCharDeleteOpcode, true, true, true, true, false);
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_CHAR_RENAME, &WorldSession::handleCharRenameOpcode, true, true, true, true, false);
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_CHAR_CUSTOMIZE, &WorldSession::handleCharCustomizeLooksOpcode, false, false, true, true, false);
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_CHAR_FACTION_CHANGE, &WorldSession::handleCharFactionOrRaceChange, false, false, true, false, false);
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_CHAR_RACE_CHANGE, &WorldSession::handleCharFactionOrRaceChange, false, false, true, false, false);

    // declined names (Cyrillic client)
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_SET_PLAYER_DECLINED_NAMES, &WorldSession::handleSetPlayerDeclinedNamesOpcode, false, true, true, true, false);

    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_PLAYER_LOGIN, &WorldSession::handlePlayerLoginOpcode, true, true, true, true, false);

    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_REALM_SPLIT, &WorldSession::handleRealmSplitOpcode, true, true, true, true, false);

    // Queries
    registry.registerOpcode(MSG_CORPSE_QUERY, &WorldSession::handleCorpseQueryOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_NAME_QUERY, &WorldSession::handleNameQueryOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUERY_TIME, &WorldSession::handleQueryTimeOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_CREATURE_QUERY, &WorldSession::handleCreatureQueryOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_GAMEOBJECT_QUERY, &WorldSession::handleGameObjectQueryOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_PAGE_TEXT_QUERY, &WorldSession::handlePageTextQueryOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_ITEM_NAME_QUERY, &WorldSession::handleItemNameQueryOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_QUERY_INSPECT_ACHIEVEMENTS, &WorldSession::handleAchievmentQueryOpcode, false, false, true, false, false);

    // Movement
    registry.registerOpcode(MSG_MOVE_HEARTBEAT, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_WORLDPORT_ACK, &WorldSession::handleMoveWorldportAckOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_JUMP, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_ASCEND, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_STOP_ASCEND, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_FORWARD, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_BACKWARD, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_SET_FACING, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_STRAFE_LEFT, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_STRAFE_RIGHT, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_STOP_STRAFE, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_TURN_LEFT, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_TURN_RIGHT, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_STOP_TURN, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_PITCH_UP, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_PITCH_DOWN, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_STOP_PITCH, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_SET_RUN_MODE, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_SET_WALK_MODE, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_SET_PITCH, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_SWIM, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_STOP_SWIM, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_FALL_LAND, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_STOP, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_SET_FLY, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_STOP_ASCEND, &WorldSession::handleMovementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_NOT_ACTIVE_MOVER, &WorldSession::handleMoveNotActiveMoverOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_ACTIVE_MOVER, &WorldSession::handleSetActiveMoverOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_CHNG_TRANSPORT, &WorldSession::handleMovementOpcodes, true, true, true, false, false);

    // ACK
    registry.registerOpcode(MSG_MOVE_TELEPORT_ACK, &WorldSession::handleMoveTeleportAckOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_FEATHER_FALL_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_WATER_WALK_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);   
    registry.registerOpcode(CMSG_FORCE_MOVE_ROOT_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_FORCE_MOVE_UNROOT_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_KNOCK_BACK_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_HOVER_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_SET_CAN_FLY_ACK, &WorldSession::handleAcknowledgementOpcodes, true, true, true, false, false);
    registry.registerOpcode(MSG_MOVE_START_DESCEND, &WorldSession::handleMovementOpcodes, true, true, true, false, false);

    // Force Speed Change
    registry.registerOpcode(CMSG_FORCE_RUN_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, false, false);
    registry.registerOpcode(CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, false, false);
    registry.registerOpcode(CMSG_FORCE_SWIM_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, false, false);
    registry.registerOpcode(CMSG_FORCE_WALK_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, false, false);
    registry.registerOpcode(CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, false, false);
    registry.registerOpcode(CMSG_FORCE_TURN_RATE_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, true, true, false, false);
    registry.registerOpcode(CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, false, true, false, false);
    registry.registerOpcode(CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK, &WorldSession::handleForceSpeedChangeAck, true, false, true, false, false);

    // Action Buttons
    registry.registerOpcode(CMSG_SET_ACTION_BUTTON, &WorldSession::handleSetActionButtonOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_REPOP_REQUEST, &WorldSession::handleRepopRequestOpcode, true, true, true, false, false);

    // Loot
    registry.registerOpcode(CMSG_AUTOSTORE_LOOT_ITEM, &WorldSession::handleAutostoreLootItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LOOT_MONEY, &WorldSession::handleLootMoneyOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LOOT, &WorldSession::handleLootOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LOOT_RELEASE, &WorldSession::handleLootReleaseOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LOOT_ROLL, &WorldSession::handleLootRollOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LOOT_MASTER_GIVE, &WorldSession::handleLootMasterGiveOpcode, true, true, true, false, false);

    // Player Interaction
    registry.registerOpcode(CMSG_WHO, &WorldSession::handleWhoOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_WHOIS, &WorldSession::handleWhoIsOpcode, true, false, true, false, false);
    registry.registerOpcode(CMSG_LOGOUT_REQUEST, &WorldSession::handleLogoutRequestOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_PLAYER_LOGOUT, &WorldSession::handlePlayerLogoutOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LOGOUT_CANCEL, &WorldSession::handleLogoutCancelOpcode, true, true, true, false, false);
    // registry.registerOpcode(CMSG_LOGOUT_CANCEL].status = STATUS_LOGGEDIN_RECENTLY_LOGGOUT, false, false, true, false, false);

    registry.registerOpcode(CMSG_ZONEUPDATE, &WorldSession::handleZoneupdate, true, true, true, false, false);
    // registry.registerOpcode(CMSG_SET_TARGET_OBSOLETE, &WorldSession::HandleSetTargetOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_SET_SELECTION, &WorldSession::handleSetSelectionOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_STANDSTATECHANGE, &WorldSession::handleStandStateChangeOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_CANCEL_MOUNT_AURA, &WorldSession::handleDismountOpcode, true, true, true, false, false);

    // Friends
    registry.registerOpcode(CMSG_CONTACT_LIST, &WorldSession::handleFriendListOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ADD_FRIEND, &WorldSession::handleAddFriendOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_DEL_FRIEND, &WorldSession::handleDelFriendOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ADD_IGNORE, &WorldSession::handleAddIgnoreOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_DEL_IGNORE, &WorldSession::handleDelIgnoreOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUG, &WorldSession::handleBugOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_CONTACT_NOTES, &WorldSession::handleSetFriendNote, false, true, true, false, false);

    // Areatrigger
    registry.registerOpcode(CMSG_AREATRIGGER, &WorldSession::handleAreaTriggerOpcode, true, true, true, false, false);

    // Account Data
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_UPDATE_ACCOUNT_DATA, &WorldSession::handleUpdateAccountData, true, true, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_ACCOUNT_DATA, &WorldSession::handleRequestAccountData, true, true, true, false, false);
    registry.registerOpcode(CMSG_TOGGLE_PVP, &WorldSession::handleTogglePVPOpcode, true, true, true, false, false);

    // Faction / Reputation
    registry.registerOpcode(CMSG_SET_FACTION_ATWAR, &WorldSession::handleSetFactionAtWarOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_WATCHED_FACTION, &WorldSession::handleSetWatchedFactionIndexOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_FACTION_INACTIVE, &WorldSession::handleSetFactionInactiveOpcode, true, true, true, false, false);

    // Player Interaction
    registry.registerOpcode(CMSG_GAMEOBJ_USE, &WorldSession::handleGameObjectUse, true, true, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_PLAYED_TIME, &WorldSession::handlePlayedTimeOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SETSHEATHED, &WorldSession::handleSetSheathedOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MESSAGECHAT, &WorldSession::handleMessageChatOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_EMOTE, &WorldSession::handleEmoteOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_TEXT_EMOTE, &WorldSession::handleTextEmoteOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_INSPECT, &WorldSession::handleInspectOpcode, true, true, true, false, false);
    // clearly wrong naming!
    //registry.registerOpcode(SMSG_BARBER_SHOP_RESULT, &WorldSession::handleBarberShopResult, false, false, true, false, false);

    // Channels
    registry.registerOpcode(CMSG_JOIN_CHANNEL, &WorldSession::handleChannelJoin, false, true, true, false, false);
    registry.registerOpcode(CMSG_LEAVE_CHANNEL, &WorldSession::handleChannelLeave, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_LIST, &WorldSession::handleChannelList, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_PASSWORD, &WorldSession::handleChannelPassword, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_SET_OWNER, &WorldSession::handleChannelSetOwner, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_OWNER, &WorldSession::handleChannelOwner, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_MODERATOR, &WorldSession::handleChannelModerator, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_UNMODERATOR, &WorldSession::handleChannelUnmoderator, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_MUTE, &WorldSession::handleChannelMute, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_UNMUTE, &WorldSession::handleChannelUnmute, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_INVITE, &WorldSession::handleChannelInvite, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_KICK, &WorldSession::handleChannelKick, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_BAN, &WorldSession::handleChannelBan, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_UNBAN, &WorldSession::handleChannelUnban, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_ANNOUNCEMENTS, &WorldSession::handleChannelAnnounce, true, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_MODERATE, &WorldSession::handleChannelModerate, true, true, true, false, false);
    registry.registerOpcode(CMSG_GET_CHANNEL_MEMBER_COUNT, &WorldSession::handleGetChannelMemberCount, false, true, true, false, false);
    registry.registerOpcode(CMSG_CHANNEL_DISPLAY_LIST, &WorldSession::handleChannelRosterQuery, true, true, true, false, false);

    // Groups / Raids
    registry.registerOpcode(CMSG_GROUP_INVITE, &WorldSession::handleGroupInviteOpcode, true, true, true, false, false);
    //registry.registerOpcode(CMSG_GROUP_CANCEL, &WorldSession::HandleGroupCancelOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_GROUP_ACCEPT, &WorldSession::handleGroupAcceptOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_DECLINE, &WorldSession::handleGroupDeclineOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_UNINVITE, &WorldSession::handleGroupUninviteOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_UNINVITE_GUID, &WorldSession::handleGroupUninviteGuidOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_SET_LEADER, &WorldSession::handleGroupSetLeaderOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_DISBAND, &WorldSession::handleGroupDisbandOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LOOT_METHOD, &WorldSession::handleLootMethodOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_MINIMAP_PING, &WorldSession::handleMinimapPingOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_RAID_CONVERT, &WorldSession::handleConvertGroupToRaidOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_CHANGE_SUB_GROUP, &WorldSession::handleGroupChangeSubGroup, true, true, true, false, false);
    registry.registerOpcode(CMSG_GROUP_ASSISTANT_LEADER, &WorldSession::handleGroupAssistantLeader, true, true, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_RAID_INFO, &WorldSession::handleRequestRaidInfoOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_RAID_READY_CHECK, &WorldSession::handleReadyCheckOpcode, true, false, true, false, false);
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
    registry.registerOpcode(CMSG_ENABLETAXI, &WorldSession::handleEnabletaxiOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_TAXINODE_STATUS_QUERY, &WorldSession::handleTaxiNodeStatusQueryOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_TAXIQUERYAVAILABLENODES, &WorldSession::handleTaxiQueryAvaibleNodesOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ACTIVATE_TAXI, &WorldSession::handleActivateTaxiOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_TABARDVENDOR_ACTIVATE, &WorldSession::handleTabardVendorActivateOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BANKER_ACTIVATE, &WorldSession::handleBankerActivateOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUY_BANK_SLOT, &WorldSession::handleBuyBankSlotOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_TRAINER_LIST, &WorldSession::handleTrainerListOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_TRAINER_BUY_SPELL, &WorldSession::handleTrainerBuySpellOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_PETITION_SHOWLIST, &WorldSession::handleCharterShowListOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_AUCTION_HELLO, &WorldSession::handleAuctionHelloOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GOSSIP_HELLO, &WorldSession::handleGossipHelloOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_GOSSIP_SELECT_OPTION, &WorldSession::handleGossipSelectOptionOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SPIRIT_HEALER_ACTIVATE, &WorldSession::handleSpiritHealerActivateOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_NPC_TEXT_QUERY, &WorldSession::handleNpcTextQueryOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BINDER_ACTIVATE, &WorldSession::handleBinderActivateOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ACTIVATE_TAXI_EXPRESS, &WorldSession::handleMultipleActivateTaxiOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MOVE_SPLINE_DONE, &WorldSession::handleMoveSplineDoneOpcode, true, true, true, false, false);
    // Item / Vendors
    registry.registerOpcode(CMSG_SWAP_INV_ITEM, &WorldSession::handleSwapInvItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SWAP_ITEM, &WorldSession::handleSwapItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_DESTROY_ITEM, &WorldSession::handleDestroyItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUTOEQUIP_ITEM, &WorldSession::handleAutoEquipItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUTOEQUIP_ITEM_SLOT, &WorldSession::handleAutoEquipItemSlotOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ITEM_QUERY_SINGLE, &WorldSession::handleItemQuerySingleOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SELL_ITEM, &WorldSession::handleSellItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUY_ITEM_IN_SLOT, &WorldSession::handleBuyItemInSlotOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUY_ITEM, &WorldSession::handleBuyItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LIST_INVENTORY, &WorldSession::handleListInventoryOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUTOSTORE_BAG_ITEM, &WorldSession::handleAutoStoreBagItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_AMMO, &WorldSession::handleAmmoSetOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUY_BACK_ITEM, &WorldSession::handleBuyBackOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SPLIT_ITEM, &WorldSession::handleSplitOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_READ_ITEM, &WorldSession::handleReadItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_REPAIR_ITEM, &WorldSession::handleRepairItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUTOBANK_ITEM, &WorldSession::handleAutoBankItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUTOSTORE_BANK_ITEM, &WorldSession::handleAutoStoreBankItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_CANCEL_TEMP_ENCHANTMENT, &WorldSession::handleCancelTemporaryEnchantmentOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SOCKET_GEMS, &WorldSession::handleInsertGemOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_WRAP_ITEM, &WorldSession::handleWrapItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ITEMREFUNDINFO, &WorldSession::handleItemRefundInfoOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_ITEMREFUNDREQUEST, &WorldSession::handleItemRefundRequestOpcode, false, false, true, false, false);

    registry.registerOpcode(CMSG_EQUIPMENT_SET_SAVE, &WorldSession::handleEquipmentSetSave, false, false, true, false, false);
    registry.registerOpcode(CMSG_EQUIPMENT_SET_USE, &WorldSession::handleEquipmentSetUse, false, false, true, false, false);
    registry.registerOpcode(CMSG_EQUIPMENT_SET_DELETE, &WorldSession::handleEquipmentSetDelete, false, false, true, false, false);

    // Spell System / Talent System
    registry.registerOpcode(CMSG_USE_ITEM, &WorldSession::handleUseItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_CAST_SPELL, &WorldSession::handleCastSpellOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SPELL_CLICK, &WorldSession::handleSpellClick, true, false, true, false, false);
    registry.registerOpcode(CMSG_CANCEL_CAST, &WorldSession::handleCancelCastOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_CANCEL_AURA, &WorldSession::handleCancelAuraOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_CANCEL_CHANNELLING, &WorldSession::handleCancelChannellingOpcode, true, false, true, false, false);
    registry.registerOpcode(CMSG_CANCEL_AUTO_REPEAT_SPELL, &WorldSession::handleCancelAutoRepeatSpellOpcode, true, false, true, false, false);
    registry.registerOpcode(CMSG_TOTEM_DESTROYED, &WorldSession::handleCancelTotem, true, true, true, false, false);
    registry.registerOpcode(CMSG_LEARN_TALENT, &WorldSession::handleLearnTalentOpcode, true, false, true, false, false);
    registry.registerOpcode(CMSG_LEARN_TALENTS_MULTIPLE, &WorldSession::handleLearnMultipleTalentsOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_UNLEARN_TALENTS, &WorldSession::handleUnlearnTalents, true, false, true, false, false);
    registry.registerOpcode(MSG_TALENT_WIPE_CONFIRM, &WorldSession::handleUnlearnTalents, true, false, true, false, false);
    registry.registerOpcode(CMSG_UPDATE_PROJECTILE_POSITION, &WorldSession::handleUpdateProjectilePosition, false, false, true, false, false);
    // Combat / Duel
    registry.registerOpcode(CMSG_ATTACKSWING, &WorldSession::handleAttackSwingOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ATTACK_STOP, &WorldSession::handleAttackStopOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_DUEL_ACCEPTED, &WorldSession::handleDuelAccepted, true, true, true, false, false);
    registry.registerOpcode(CMSG_DUEL_CANCELLED, &WorldSession::handleDuelCancelled, true, true, true, false, false);

    // Trade
    registry.registerOpcode(CMSG_INITIATE_TRADE, &WorldSession::handleInitiateTradeOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BEGIN_TRADE, &WorldSession::handleBeginTradeOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUSY_TRADE, &WorldSession::handleBusyTrade, true, true, true, false, false);
    registry.registerOpcode(CMSG_IGNORE_TRADE, &WorldSession::handleIgnoreTrade, true, true, true, false, false);
    registry.registerOpcode(CMSG_ACCEPT_TRADE, &WorldSession::handleAcceptTrade, true, true, true, false, false);
    registry.registerOpcode(CMSG_UNACCEPT_TRADE, &WorldSession::handleUnacceptTrade, true, true, true, false, false);
    registry.registerOpcode(CMSG_CANCEL_TRADE, &WorldSession::handleCancelTrade, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_TRADE_ITEM, &WorldSession::handleSetTradeItem, true, true, true, false, false);
    registry.registerOpcode(CMSG_CLEAR_TRADE_ITEM, &WorldSession::handleClearTradeItem, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_TRADE_GOLD, &WorldSession::handleSetTradeGold, true, true, true, false, false);

    // Quest System
    registry.registerOpcode(CMSG_QUEST_GIVER_STATUS_MULTIPLE_QUERY, &WorldSession::handleInrangeQuestgiverQuery, false, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTGIVER_STATUS_QUERY, &WorldSession::handleQuestgiverStatusQueryOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTGIVER_HELLO, &WorldSession::handleQuestgiverHelloOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTGIVER_ACCEPT_QUEST, &WorldSession::handleQuestgiverAcceptQuestOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTGIVER_CANCEL, &WorldSession::handleQuestgiverCancelOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTGIVER_CHOOSE_REWARD, &WorldSession::handleQuestgiverChooseRewardOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTGIVER_REQUEST_REWARD, &WorldSession::handleQuestgiverRequestRewardOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUEST_QUERY, &WorldSession::handleQuestQueryOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTGIVER_QUERY_QUEST, &WorldSession::handleQuestGiverQueryQuestOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTGIVER_COMPLETE_QUEST, &WorldSession::handleQuestgiverCompleteQuestOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUESTLOG_REMOVE_QUEST, &WorldSession::handleQuestlogRemoveQuestOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_RECLAIM_CORPSE, &WorldSession::handleCorpseReclaimOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_RESURRECT_RESPONSE, &WorldSession::handleResurrectResponse, true, true, true, false, false);
    registry.registerOpcode(CMSG_PUSHQUESTTOPARTY, &WorldSession::handlePushQuestToPartyOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_QUEST_PUSH_RESULT, &WorldSession::handleQuestPushResultOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_QUEST_POI_QUERY, &WorldSession::handleQuestPOIQueryOpcode, false, false, true, false, false);

    // Auction System
    registry.registerOpcode(CMSG_AUCTION_LIST_ITEMS, &WorldSession::handleAuctionListItems, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUCTION_LIST_BIDDER_ITEMS, &WorldSession::handleAuctionListBidderItems, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUCTION_SELL_ITEM, &WorldSession::handleAuctionSellItem, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUCTION_LIST_OWNER_ITEMS, &WorldSession::handleAuctionListOwnerItems, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUCTION_PLACE_BID, &WorldSession::handleAuctionPlaceBid, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUCTION_REMOVE_ITEM, &WorldSession::handleCancelAuction, true, true, true, false, false);
    registry.registerOpcode(CMSG_AUCTION_LIST_PENDING_SALES, &WorldSession::handleAuctionListPendingSales, false, true, true, false, false);

    // Mail System
    registry.registerOpcode(CMSG_GET_MAIL_LIST, &WorldSession::handleGetMailOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_ITEM_TEXT_QUERY, &WorldSession::handleItemTextQueryOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SEND_MAIL, &WorldSession::handleSendMailOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MAIL_TAKE_MONEY, &WorldSession::handleTakeMoneyOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MAIL_TAKE_ITEM, &WorldSession::handleTakeItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MAIL_MARK_AS_READ, &WorldSession::handleMarkAsReadOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MAIL_RETURN_TO_SENDER, &WorldSession::handleReturnToSenderOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MAIL_DELETE, &WorldSession::handleMailDeleteOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_QUERY_NEXT_MAIL_TIME, &WorldSession::handleMailTimeOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_MAIL_CREATE_TEXT_ITEM, &WorldSession::handleMailCreateTextItemOpcode, true, true, true, false, false);

    // Guild Query (called when not logged in sometimes)
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_GUILD_QUERY, &WorldSession::handleGuildQuery, true, true, true, false, false);

    // Guild System
    //registry.registerOpcode(CMSG_GUILD_CREATE, &WorldSession::HandleCreateGuild, false, false, true, false, false);
    registry.registerOpcode(CMSG_GUILD_INVITE, &WorldSession::handleInviteToGuild, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_ACCEPT, &WorldSession::handleGuildAccept, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_DECLINE, &WorldSession::handleGuildDecline, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_INFO, &WorldSession::handleGuildInfo, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_ROSTER, &WorldSession::handleGuildRoster, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_PROMOTE, &WorldSession::handleGuildPromote, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_DEMOTE, &WorldSession::handleGuildDemote, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_LEAVE, &WorldSession::handleGuildLeave, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_REMOVE, &WorldSession::handleGuildRemove, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_DISBAND, &WorldSession::handleGuildDisband, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_LEADER, &WorldSession::handleGuildLeader, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_MOTD, &WorldSession::handleGuildMotd, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_SET_RANK, &WorldSession::handleGuildSetRank, true, false, true, false, false);
    registry.registerOpcode(CMSG_GUILD_ADD_RANK, &WorldSession::handleGuildAddRank, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_DEL_RANK, &WorldSession::handleGuildDelRank, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_SET_PUBLIC_NOTE, &WorldSession::handleGuildSetPublicNote, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_SET_OFFICER_NOTE, &WorldSession::handleGuildSetOfficerNote, true, true, true, false, false);
    registry.registerOpcode(CMSG_PETITION_BUY, &WorldSession::handleCharterBuy, true, true, true, false, false);
    registry.registerOpcode(CMSG_PETITION_SHOW_SIGNATURES, &WorldSession::handleCharterShowSignatures, true, true, true, false, false);
    registry.registerOpcode(CMSG_TURN_IN_PETITION, &WorldSession::handleCharterTurnInCharter, true, true, true, false, false);
    registry.registerOpcode(CMSG_PETITION_QUERY, &WorldSession::handleCharterQuery, true, true, true, false, false);
    registry.registerOpcode(CMSG_OFFER_PETITION, &WorldSession::handleCharterOffer, true, true, true, false, false);
    registry.registerOpcode(CMSG_PETITION_SIGN, &WorldSession::handleCharterSign, true, true, true, false, false);
    registry.registerOpcode(MSG_PETITION_DECLINE, &WorldSession::handleCharterDecline, true, true, true, false, false);
    registry.registerOpcode(MSG_PETITION_RENAME, &WorldSession::handleCharterRename, true, true, true, false, false);
    registry.registerOpcode(MSG_SAVE_GUILD_EMBLEM, &WorldSession::handleSaveGuildEmblem, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_INFO_TEXT, &WorldSession::handleSetGuildInfo, true, true, true, false, false);
    registry.registerOpcode(MSG_QUERY_GUILD_BANK_TEXT, &WorldSession::handleGuildBankQueryText, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_GUILD_BANK_TEXT, &WorldSession::handleSetGuildBankText, true, true, true, false, false);
    registry.registerOpcode(MSG_GUILD_EVENT_LOG_QUERY, &WorldSession::handleGuildLog, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANKER_ACTIVATE, &WorldSession::handleGuildBankerActivate, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANK_BUY_TAB, &WorldSession::handleGuildBankBuyTab, true, true, true, false, false);
    registry.registerOpcode(MSG_GUILD_BANK_MONEY_WITHDRAWN, &WorldSession::handleGuildBankMoneyWithdrawn, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANK_UPDATE_TAB, &WorldSession::handleGuildBankUpdateTab, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANK_SWAP_ITEMS, &WorldSession::handleGuildBankSwapItems, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANK_WITHDRAW_MONEY, &WorldSession::handleGuildBankWithdrawMoney, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANK_DEPOSIT_MONEY, &WorldSession::handleGuildBankDepositMoney, true, true, true, false, false);
    registry.registerOpcode(CMSG_GUILD_BANK_QUERY_TAB, &WorldSession::handleGuildBankQueryTab, true, true, true, false, false);
    registry.registerOpcode(MSG_GUILD_BANK_LOG_QUERY, &WorldSession::handleGuildBankLogQuery, true, true, true, false, false);
    registry.registerOpcode(MSG_GUILD_PERMISSIONS, &WorldSession::handleGuildPermissions, true, true, true, false, false);

    // Tutorials
    registry.registerOpcode(CMSG_TUTORIAL_FLAG, &WorldSession::handleTutorialFlag, true, true, true, false, false);
    registry.registerOpcode(CMSG_TUTORIAL_CLEAR, &WorldSession::handleTutorialClear, true, true, true, false, false);
    registry.registerOpcode(CMSG_TUTORIAL_RESET, &WorldSession::handleTutorialReset, true, true, true, false, false);

    // Pets
    registry.registerOpcode(MSG_LIST_STABLED_PETS, &WorldSession::handleStabledPetList, true, true, true, false, false);

    registry.registerOpcode(CMSG_PET_ACTION, &WorldSession::handlePetAction, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_NAME_QUERY, &WorldSession::handlePetNameQuery, true, true, true, false, false);
    registry.registerOpcode(CMSG_BUY_STABLE_SLOT, &WorldSession::handleBuyStableSlot, true, true, true, false, false);
    registry.registerOpcode(CMSG_STABLE_PET, &WorldSession::handleStablePet, true, true, true, false, false);
    registry.registerOpcode(CMSG_UNSTABLE_PET, &WorldSession::handleUnstablePet, true, true, true, false, false);
    registry.registerOpcode(CMSG_STABLE_SWAP_PET, &WorldSession::handleStableSwapPet, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_SET_ACTION, &WorldSession::handlePetSetActionOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_RENAME, &WorldSession::handlePetRename, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_ABANDON, &WorldSession::handlePetAbandon, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_UNLEARN, &WorldSession::handlePetUnlearn, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_SPELL_AUTOCAST, &WorldSession::handlePetSpellAutocast, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_CANCEL_AURA, &WorldSession::handlePetCancelAura, true, true, true, false, false);
    registry.registerOpcode(CMSG_PET_LEARN_TALENT, &WorldSession::handlePetLearnTalent, false, false, true, false, false);
    registry.registerOpcode(CMSG_DISMISS_CRITTER, &WorldSession::handleDismissCritter, false, false, true, false, false);

    // Battlegrounds
    registry.registerOpcode(CMSG_BATTLEFIELD_PORT, &WorldSession::handleBattlefieldPortOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BATTLEFIELD_STATUS, &WorldSession::handleBattlefieldStatusOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BATTLEFIELD_LIST, &WorldSession::handleBattlefieldListOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BATTLEMASTER_HELLO, &WorldSession::handleBattleMasterHelloOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BATTLEMASTER_JOIN_ARENA, &WorldSession::handleArenaJoinOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_BATTLEMASTER_JOIN, &WorldSession::handleBattleMasterJoinOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_LEAVE_BATTLEFIELD, &WorldSession::handleLeaveBattlefieldOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_AREA_SPIRIT_HEALER_QUERY, &WorldSession::handleAreaSpiritHealerQueryOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_AREA_SPIRIT_HEALER_QUEUE, &WorldSession::handleAreaSpiritHealerQueueOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_BATTLEGROUND_PLAYER_POSITIONS, &WorldSession::handleBattlegroundPlayerPositionsOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_PVP_LOG_DATA, &WorldSession::handlePVPLogDataOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_INSPECT_HONOR_STATS, &WorldSession::handleInspectHonorStatsOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_ACTIONBAR_TOGGLES, &WorldSession::handleSetActionBarTogglesOpcode, true, true, true, false, false);
    //registry.registerOpcode(CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE, &WorldSession::HandleBgInviteResponse, false, false, true, false, false);

    // GM Ticket System
    registry.registerOpcode(CMSG_GMTICKET_CREATE, &WorldSession::handleGMTicketCreateOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_GMTICKET_UPDATETEXT, &WorldSession::handleGMTicketUpdateOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_GMTICKET_DELETETICKET, &WorldSession::handleGMTicketDeleteOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_GMTICKET_GETTICKET, &WorldSession::handleGMTicketGetTicketOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_GMTICKET_SYSTEMSTATUS, &WorldSession::handleGMTicketSystemStatusOpcode, false, true, true, false, false);
    registry.registerOpcode(CMSG_GMTICKETSYSTEM_TOGGLE, &WorldSession::handleGMTicketToggleSystemStatusOpcode, false, true, true, false, false);

    // Lag report
    registry.registerOpcode(CMSG_GM_REPORT_LAG, &WorldSession::handleReportLag, false, false, true, false, false);
    registry.registerOpcode(CMSG_GMSURVEY_SUBMIT, &WorldSession::handleGMSurveySubmitOpcode, false, false, true, false, false);

    // Meeting Stone / Instances
    registry.registerOpcode(CMSG_SUMMON_RESPONSE, &WorldSession::handleSummonResponseOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_RESET_INSTANCES, &WorldSession::handleResetInstanceOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SELF_RES, &WorldSession::handleSelfResurrect, true, true, true, false, false);
    registry.registerOpcode(MSG_RANDOM_ROLL, &WorldSession::handleRandomRollOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_SET_DUNGEON_DIFFICULTY, &WorldSession::handleDungeonDifficultyOpcode, true, true, true, false, false);
    registry.registerOpcode(MSG_SET_RAID_DIFFICULTY, &WorldSession::handleRaidDifficultyOpcode, false, false, true, false, false);
    registry.registerOpcode(CMSG_INSTANCE_LOCK_RESPONSE, &WorldSession::handleInstanceLockResponse, false, false, true, false, false);
    
    // Misc
    registry.registerOpcode(CMSG_OPEN_ITEM, &WorldSession::handleOpenItemOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_COMPLETE_CINEMATIC, &WorldSession::handleCompleteCinematic, true, true, true, false, false);
    registry.registerOpcode(CMSG_NEXT_CINEMATIC_CAMERA, &WorldSession::handleNextCinematic, true, false, true, false, false);
    registry.registerOpcode(CMSG_MOUNTSPECIAL_ANIM, &WorldSession::handleMountSpecialAnimOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_TOGGLE_CLOAK, &WorldSession::handleToggleCloakOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_TOGGLE_HELM, &WorldSession::handleToggleHelmOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_SET_TITLE, &WorldSession::handleSetTitle, true, true, true, false, false);
    registry.registerOpcode(CMSG_COMPLAIN, &WorldSession::handleReportSpamOpcode, true, false, true, false, false);
    registry.registerOpcode(CMSG_GAMEOBJ_REPORT_USE, &WorldSession::handleGameobjReportUseOpCode, false, false, true, false, false);
    registry.registerOpcode(CMSG_PET_CAST_SPELL, &WorldSession::handlePetCastSpell, true, true, true, false, false);
    registry.registerOpcode(CMSG_WORLD_STATE_UI_TIMER_UPDATE, &WorldSession::handleWorldStateUITimerUpdate, false, false, true, false, false);
    registry.registerOpcode(CMSG_SET_TAXI_BENCHMARK_MODE, &WorldSession::handleSetTaxiBenchmarkOpcode, true, false, true, false, false);
    registry.registerOpcode(CMSG_UNLEARN_SKILL, &WorldSession::handleUnlearnSkillOpcode, true, true, true, false, false);

    // Chat
    registry.registerOpcode(CMSG_CHAT_IGNORED, &WorldSession::handleChatIgnoredOpcode, true, false, true, false, false);
    registry.registerOpcode(CMSG_SET_CHANNEL_WATCH, &WorldSession::handleChatChannelWatchOpcode, true, false, true, false, false);

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
    registry.registerOpcode(CMSG_DISMISS_CONTROLLED_VEHICLE, &WorldSession::handleDismissVehicle, false, false, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_VEHICLE_EXIT, &WorldSession::handleLeaveVehicle, false, false, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_VEHICLE_PREV_SEAT, &WorldSession::handleRequestVehiclePreviousSeat, false, false, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_VEHICLE_NEXT_SEAT, &WorldSession::handleRequestVehicleNextSeat, false, false, true, false, false);
    registry.registerOpcode(CMSG_REQUEST_VEHICLE_SWITCH_SEAT, &WorldSession::handleRequestVehicleSwitchSeat, false, false, true, false, false);
    registry.registerOpcode(CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, &WorldSession::handleChangeSeatsOnControlledVehicle, false, false, true, false, false);
    registry.registerOpcode(CMSG_PLAYER_VEHICLE_ENTER, &WorldSession::handleEnterVehicle, false, false, true, false, false);
    registry.registerOpcode(CMSG_EJECT_PASSENGER, &WorldSession::handleRemoveVehiclePassenger, false, false, true, false, false);

    // Unsorted
    //registry.registerOpcode(CMSG_TIME_SYNC_RESPONSE, &WorldSession::HandleTimeSyncResp, false, false, true, false, false);
    registry.registerOpcode<SSTATUS_AUTHED>(CMSG_READY_FOR_ACCOUNT_DATA_TIMES, &WorldSession::handleReadyForAccountDataTimes, false, false, true, false, false);

    registry.registerOpcode(CMSG_OPT_OUT_OF_LOOT, &WorldSession::handleSetAutoLootPassOpcode, true, true, true, false, false);
    registry.registerOpcode(CMSG_REMOVE_GLYPH, &WorldSession::handleRemoveGlyph, false, false, true, false, false);
    registry.registerOpcode(CMSG_ALTER_APPEARANCE, &WorldSession::handleBarberShopResult, false, false, true, false, false);
    registry.registerOpcode(CMSG_GET_MIRRORIMAGE_DATA, &WorldSession::HandleMirrorImageOpcode, true, false, true, false, false);

    // Calendar - Unhandled
    registry.registerOpcode(CMSG_CALENDAR_GET_CALENDAR, &WorldSession::handleCalendarGetCalendar, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_COMPLAIN, &WorldSession::handleCalendarComplain, false, false, true, false, false);
    registry.registerOpcode(CMSG_CALENDAR_GET_NUM_PENDING, &WorldSession::handleCalendarGetNumPending, false, false, true, false, false);
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
    registry.registerOpcode(CMSG_FAR_SIGHT, &WorldSession::Unhandled, false, false, true, false, false);
    registry.registerOpcode(CMSG_LFG_GET_STATUS, &WorldSession::Unhandled, false, false, true, false, false);
    registry.registerOpcode(CMSG_VOICE_SESSION_ENABLE, &WorldSession::Unhandled, true, false, true, false, false);
    registry.registerOpcode(CMSG_SET_ACTIVE_VOICE_CHANNEL, &WorldSession::Unhandled, true, false, true, false, false);
}

void WorldSession::loadHandlers() // old system
{
#if VERSION_STRING == Cata
    WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].handler = &WorldSession::handleObjectUpdateFailedOpcode;
    WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_LOADING_SCREEN_NOTIFY].handler = &WorldSession::handleLoadScreenOpcode;
    WorldPacketHandlers[CMSG_LOADING_SCREEN_NOTIFY].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::handleReadyForAccountDataTimes;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::handleUITimeRequestOpcode;
    WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_TIME_SYNC_RESPONSE].handler = &WorldSession::handleTimeSyncRespOpcode;
    WorldPacketHandlers[CMSG_TIME_SYNC_RESPONSE].status = STATUS_AUTHED;

    // Queries
    WorldPacketHandlers[MSG_CORPSE_QUERY].handler = &WorldSession::handleCorpseQueryOpcode;
    WorldPacketHandlers[CMSG_NAME_QUERY].handler = &WorldSession::handleNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_TIME].handler = &WorldSession::handleQueryTimeOpcode;
    WorldPacketHandlers[CMSG_CREATURE_QUERY].handler = &WorldSession::handleCreatureQueryOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler = &WorldSession::handleGameObjectQueryOpcode;
    //WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler = &WorldSession::HandlePageTextQueryOpcode;
    //WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler = &WorldSession::HandleItemNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler = &WorldSession::handleAchievmentQueryOpcode;

    // Movement
    WorldPacketHandlers[MSG_MOVE_HEARTBEAT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_WORLDPORT_ACK].handler = &WorldSession::handleMoveWorldportAckOpcode;
    WorldPacketHandlers[MSG_MOVE_JUMP].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_FORWARD].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_BACKWARD].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_FACING].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_STRAFE_LEFT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_STRAFE_RIGHT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_STRAFE].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_TURN_LEFT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_TURN_RIGHT].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_TURN].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_PITCH_UP].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_PITCH_DOWN].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_PITCH].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_RUN_MODE].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_WALK_MODE].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_PITCH].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_SWIM].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_SWIM].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_FALL_LAND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_NOT_ACTIVE_MOVER].handler = &WorldSession::handleMoveNotActiveMoverOpcode;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_MOVER].handler = &WorldSession::HandleSetActiveMoverOpcode;
    WorldPacketHandlers[CMSG_MOVE_CHNG_TRANSPORT].handler = &WorldSession::handleMovementOpcodes;

    // ACK
    WorldPacketHandlers[MSG_MOVE_TELEPORT_ACK].handler = &WorldSession::handleMoveTeleportAckOpcode;
    //WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_PITCH_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;

    // Force Speed Change
    WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;

    // Action Buttons
    WorldPacketHandlers[CMSG_SET_ACTION_BUTTON].handler = &WorldSession::handleSetActionButtonOpcode;
    WorldPacketHandlers[CMSG_REPOP_REQUEST].handler = &WorldSession::handleRepopRequestOpcode;

    // Loot
    WorldPacketHandlers[CMSG_AUTOSTORE_LOOT_ITEM].handler = &WorldSession::handleAutostoreLootItemOpcode;
    WorldPacketHandlers[CMSG_LOOT_MONEY].handler = &WorldSession::handleLootMoneyOpcode;
    WorldPacketHandlers[CMSG_LOOT].handler = &WorldSession::handleLootOpcode;
    WorldPacketHandlers[CMSG_LOOT_RELEASE].handler = &WorldSession::handleLootReleaseOpcode;
    WorldPacketHandlers[CMSG_LOOT_ROLL].handler = &WorldSession::handleLootRollOpcode;
    WorldPacketHandlers[CMSG_LOOT_MASTER_GIVE].handler = &WorldSession::handleLootMasterGiveOpcode;

    // Player Interaction
    WorldPacketHandlers[CMSG_WHO].handler = &WorldSession::handleWhoOpcode;
    WorldPacketHandlers[CMSG_WHOIS].handler = &WorldSession::handleWhoIsOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_REQUEST].handler = &WorldSession::handleLogoutRequestOpcode;
    //WorldPacketHandlers[CMSG_PLAYER_LOGOUT].handler = &WorldSession::handlePlayerLogoutOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_CANCEL].handler = &WorldSession::handleLogoutCancelOpcode;
    // //WorldPacketHandlers[CMSG_LOGOUT_CANCEL].status = STATUS_LOGGEDIN_RECENTLY_LOGGOUT;

    WorldPacketHandlers[CMSG_ZONEUPDATE].handler = &WorldSession::handleZoneupdate;
    // //WorldPacketHandlers[CMSG_SET_TARGET_OBSOLETE].handler = &WorldSession::HandleSetTargetOpcode;
    WorldPacketHandlers[CMSG_SET_SELECTION].handler = &WorldSession::handleSetSelectionOpcode;
    WorldPacketHandlers[CMSG_STANDSTATECHANGE].handler = &WorldSession::handleStandStateChangeOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_MOUNT_AURA].handler = &WorldSession::HandleDismountOpcode;

    // Friends
    WorldPacketHandlers[CMSG_CONTACT_LIST].handler = &WorldSession::handleFriendListOpcode;
    //WorldPacketHandlers[CMSG_ADD_FRIEND].handler = &WorldSession::handleAddFriendOpcode;
    //WorldPacketHandlers[CMSG_DEL_FRIEND].handler = &WorldSession::handleDelFriendOpcode;
    //WorldPacketHandlers[CMSG_ADD_IGNORE].handler = &WorldSession::handleAddIgnoreOpcode;
    //WorldPacketHandlers[CMSG_DEL_IGNORE].handler = &WorldSession::handleDelIgnoreOpcode;
    // WorldPacketHandlers[CMSG_SET_CONTACT_NOTES].handler = &WorldSession::handleSetContactNotes;

    // Areatrigger
    WorldPacketHandlers[CMSG_AREATRIGGER].handler = &WorldSession::handleAreaTriggerOpcode;

    // Account Data
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].handler = &WorldSession::handleUpdateAccountData;
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].status = STATUS_AUTHED;
    //WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].handler = &WorldSession::handleRequestAccountData;
    WorldPacketHandlers[CMSG_TOGGLE_PVP].handler = &WorldSession::handleTogglePVPOpcode;

    // Faction / Reputation
    //WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler = &WorldSession::HandleSetAtWarOpcode;
    //WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler = &WorldSession::HandleSetWatchedFactionIndexOpcode;
    //WorldPacketHandlers[CMSG_SET_FACTION_INACTIVE].handler = &WorldSession::HandleSetFactionInactiveOpcode;

    // Player Interaction
    WorldPacketHandlers[CMSG_GAMEOBJ_USE].handler = &WorldSession::handleGameObjectUse;
    WorldPacketHandlers[CMSG_REQUEST_PLAYED_TIME].handler = &WorldSession::handlePlayedTimeOpcode;
    WorldPacketHandlers[CMSG_SETSHEATHED].handler = &WorldSession::handleSetSheathedOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT].handler = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_EMOTE].handler = &WorldSession::handleEmoteOpcode;
    WorldPacketHandlers[CMSG_TEXT_EMOTE].handler = &WorldSession::handleTextEmoteOpcode;
    WorldPacketHandlers[CMSG_INSPECT].handler = &WorldSession::handleInspectOpcode;
    WorldPacketHandlers[SMSG_BARBER_SHOP_RESULT].handler = &WorldSession::handleBarberShopResult;

    WorldPacketHandlers[CMSG_MESSAGECHAT_SAY].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_YELL].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_CHANNEL].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_WHISPER].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_GUILD].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_OFFICER].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_AFK].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_DND].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_EMOTE].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_PARTY].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_RAID].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_RAID_WARNING].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT_BATTLEGROUND].handler = &WorldSession::handleMessageChatOpcode;

    // Channels
    WorldPacketHandlers[CMSG_JOIN_CHANNEL].handler = &WorldSession::handleChannelJoin;
    WorldPacketHandlers[CMSG_LEAVE_CHANNEL].handler = &WorldSession::handleChannelLeave;
    WorldPacketHandlers[CMSG_CHANNEL_LIST].handler = &WorldSession::handleChannelList;
    WorldPacketHandlers[CMSG_CHANNEL_PASSWORD].handler = &WorldSession::handleChannelPassword;
    WorldPacketHandlers[CMSG_CHANNEL_SET_OWNER].handler = &WorldSession::handleChannelSetOwner;
    WorldPacketHandlers[CMSG_CHANNEL_OWNER].handler = &WorldSession::handleChannelOwner;
    WorldPacketHandlers[CMSG_CHANNEL_MODERATOR].handler = &WorldSession::handleChannelModerator;
    WorldPacketHandlers[CMSG_CHANNEL_UNMODERATOR].handler = &WorldSession::handleChannelUnmoderator;
    WorldPacketHandlers[CMSG_CHANNEL_MUTE].handler = &WorldSession::handleChannelMute;
    WorldPacketHandlers[CMSG_CHANNEL_UNMUTE].handler = &WorldSession::handleChannelUnmute;
    WorldPacketHandlers[CMSG_CHANNEL_INVITE].handler = &WorldSession::handleChannelInvite;
    WorldPacketHandlers[CMSG_CHANNEL_KICK].handler = &WorldSession::handleChannelKick;
    WorldPacketHandlers[CMSG_CHANNEL_BAN].handler = &WorldSession::handleChannelBan;
    WorldPacketHandlers[CMSG_CHANNEL_UNBAN].handler = &WorldSession::handleChannelUnban;
    WorldPacketHandlers[CMSG_CHANNEL_ANNOUNCEMENTS].handler = &WorldSession::handleChannelAnnounce;
    WorldPacketHandlers[CMSG_CHANNEL_MODERATE].handler = &WorldSession::handleChannelModerate;
    WorldPacketHandlers[CMSG_GET_CHANNEL_MEMBER_COUNT].handler = &WorldSession::handleGetChannelMemberCount;
    WorldPacketHandlers[CMSG_CHANNEL_DISPLAY_LIST].handler = &WorldSession::handleChannelRosterQuery;

    // Groups / Raids
    WorldPacketHandlers[CMSG_GROUP_INVITE].handler = &WorldSession::handleGroupInviteOpcode;
    WorldPacketHandlers[CMSG_GROUP_INVITE_RESPONSE].handler = &WorldSession::handleGroupInviteResponseOpcode;
    WorldPacketHandlers[CMSG_GROUP_SET_ROLES].handler = &WorldSession::handleGroupSetRolesOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CANCEL].handler = &WorldSession::HandleGroupCancelOpcode;
    //WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler = &WorldSession::HandleGroupAcceptOpcode;
    //WorldPacketHandlers[CMSG_GROUP_DECLINE].handler = &WorldSession::HandleGroupDeclineOpcode;
    //WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler = &WorldSession::HandleGroupUninviteOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler = &WorldSession::handleGroupUninviteGuidOpcode;
    WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler = &WorldSession::handleGroupSetLeaderOpcode;
    WorldPacketHandlers[CMSG_GROUP_DISBAND].handler = &WorldSession::handleGroupDisbandOpcode;
    WorldPacketHandlers[CMSG_LOOT_METHOD].handler = &WorldSession::handleLootMethodOpcode;
    //WorldPacketHandlers[MSG_MINIMAP_PING].handler = &WorldSession::HandleMinimapPingOpcode;
    WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler = &WorldSession::handleConvertGroupToRaidOpcode;
    WorldPacketHandlers[CMSG_GROUP_REQUEST_JOIN_UPDATES].handler = &WorldSession::handleGroupRequestJoinUpdatesOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler = &WorldSession::HandleGroupChangeSubGroup;
    //WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler = &WorldSession::HandleGroupAssistantLeader;
    WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler = &WorldSession::handleRequestRaidInfoOpcode;
    WorldPacketHandlers[MSG_RAID_READY_CHECK].handler = &WorldSession::handleReadyCheckOpcode;
    //WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler = &WorldSession::HandleSetPlayerIconOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler = &WorldSession::HandlePartyMemberStatsOpcode;
    //WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler = &WorldSession::HandleGroupPromote;
    WorldPacketHandlers[CMSG_ROLE_CHECK_BEGIN].handler = &WorldSession::handleGroupRoleCheckBeginOpcode;
    WorldPacketHandlers[CMSG_MAKE_EVERYONE_ASSISTANT].handler = &WorldSession::nothingToHandle;
    WorldPacketHandlers[MSG_RAID_READY_CHECK_FINISHED].handler = &WorldSession::nothingToHandle;

    // LFG System
    WorldPacketHandlers[CMSG_LFG_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgLockInfoOpcode;
    //WorldPacketHandlers[CMSG_SET_LFG_COMMENT].handler = &WorldSession::handleLfgSetCommentOpcode;
    //WorldPacketHandlers[CMSG_LFG_JOIN].handler = &WorldSession::handleLfgJoinOpcode;
    //WorldPacketHandlers[CMSG_LFG_LEAVE].handler = &WorldSession::handleLfgLeaveOpcode;
    //WorldPacketHandlers[CMSG_SEARCH_LFG_JOIN].handler = &WorldSession::handleLfgSearchOpcode;
    //WorldPacketHandlers[CMSG_SEARCH_LFG_LEAVE].handler = &WorldSession::handleLfgSearchLeaveOpcode;
    //WorldPacketHandlers[CMSG_LFG_PROPOSAL_RESULT].handler = &WorldSession::handleLfgProposalResultOpcode;
    //WorldPacketHandlers[CMSG_LFG_SET_ROLES].handler = &WorldSession::handleLfgSetRolesOpcode;
    //WorldPacketHandlers[CMSG_LFG_SET_BOOT_VOTE].handler = &WorldSession::handleLfgSetBootVoteOpcode;
    //WorldPacketHandlers[CMSG_LFD_PLAYER_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPlayerLockInfoRequestOpcode;
    //WorldPacketHandlers[CMSG_LFG_TELEPORT].handler = &WorldSession::handleLfgTeleportOpcode;
    //WorldPacketHandlers[CMSG_LFD_PARTY_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPartyLockInfoRequestOpcode;

    // Taxi / NPC Interaction
    WorldPacketHandlers[CMSG_ENABLETAXI].handler = &WorldSession::handleEnabletaxiOpcode;
    WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler = &WorldSession::handleTaxiNodeStatusQueryOpcode;
    WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler = &WorldSession::handleTaxiQueryAvaibleNodesOpcode;
    WorldPacketHandlers[CMSG_ACTIVATE_TAXI].handler = &WorldSession::handleActivateTaxiOpcode;
    WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler = &WorldSession::handleTabardVendorActivateOpcode;
    WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler = &WorldSession::handleBankerActivateOpcode;
    WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler = &WorldSession::handleBuyBankSlotOpcode;
    WorldPacketHandlers[CMSG_TRAINER_LIST].handler = &WorldSession::handleTrainerListOpcode;
    WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler = &WorldSession::handleTrainerBuySpellOpcode;
    WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler = &WorldSession::handleCharterShowListOpcode;
    WorldPacketHandlers[MSG_AUCTION_HELLO].handler = &WorldSession::handleAuctionHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler = &WorldSession::handleGossipHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler = &WorldSession::handleGossipSelectOptionOpcode;
    WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler = &WorldSession::handleSpiritHealerActivateOpcode;
    WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler = &WorldSession::handleNpcTextQueryOpcode;
    WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler = &WorldSession::handleBinderActivateOpcode;
    WorldPacketHandlers[CMSG_ACTIVATE_TAXI_EXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;
    WorldPacketHandlers[CMSG_MOVE_SPLINE_DONE].handler = &WorldSession::handleMoveSplineDoneOpcode;

    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROY_ITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    //WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    //WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    WorldPacketHandlers[CMSG_BUY_BACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
    WorldPacketHandlers[CMSG_SPLIT_ITEM].handler = &WorldSession::handleSplitOpcode;
    //WorldPacketHandlers[CMSG_READ_ITEM].handler = &WorldSession::handleReadItemOpcode;
    WorldPacketHandlers[CMSG_REPAIR_ITEM].handler = &WorldSession::handleRepairItemOpcode;
    WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler = &WorldSession::handleAutoBankItemOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler = &WorldSession::handleAutoStoreBankItemOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler = &WorldSession::handleCancelTemporaryEnchantmentOpcode;
    WorldPacketHandlers[CMSG_SOCKET_GEMS].handler = &WorldSession::handleInsertGemOpcode;
    //WorldPacketHandlers[CMSG_WRAP_ITEM].handler = &WorldSession::handleWrapItemOpcode;
    WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler = &WorldSession::handleItemRefundInfoOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler = &WorldSession::handleItemRefundRequestOpcode;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler = &WorldSession::handleEquipmentSetSave;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler = &WorldSession::handleEquipmentSetUse;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler = &WorldSession::handleEquipmentSetDelete;
    WorldPacketHandlers[CMSG_TRANSMOGRIFY_ITEMS].handler = &WorldSession::handleTransmogrifyItems;
    WorldPacketHandlers[CMSG_REFORGE_ITEM].handler = &WorldSession::handleReforgeItemOpcode;
    WorldPacketHandlers[CMSG_VOID_STORAGE_QUERY].handler = &WorldSession::handleVoidStorageQuery;
    WorldPacketHandlers[CMSG_VOID_STORAGE_TRANSFER].handler = &WorldSession::handleVoidStorageTransfer;
    WorldPacketHandlers[CMSG_VOID_STORAGE_UNLOCK].handler = &WorldSession::handleVoidStorageUnlock;
    WorldPacketHandlers[CMSG_VOID_SWAP_ITEM].handler = &WorldSession::handleVoidSwapItem;

    // Spell System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    WorldPacketHandlers[CMSG_SPELL_CLICK].handler = &WorldSession::handleSpellClick;
    WorldPacketHandlers[CMSG_CANCEL_CAST].handler = &WorldSession::handleCancelCastOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AURA].handler = &WorldSession::handleCancelAuraOpcode;
    WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler = &WorldSession::handleCancelChannellingOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler = &WorldSession::handleCancelAutoRepeatSpellOpcode;
    WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler = &WorldSession::handleCancelTotem;
    WorldPacketHandlers[CMSG_UPDATE_PROJECTILE_POSITION].handler = &WorldSession::handleUpdateProjectilePosition;
    
    // Talent System
    WorldPacketHandlers[CMSG_LEARN_TALENT].handler = &WorldSession::handleLearnTalentOpcode;
    WorldPacketHandlers[CMSG_LEARN_PREVIEW_TALENTS].handler = &WorldSession::handleLearnPreviewTalentsOpcode;
    //WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler = &WorldSession::handleLearnMultipleTalentsOpcode;
    //WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler = &WorldSession::handleUnlearnTalents;
    WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler = &WorldSession::handleUnlearnTalents;

    // Combat / Duel
    WorldPacketHandlers[CMSG_ATTACKSWING].handler = &WorldSession::handleAttackSwingOpcode;
    WorldPacketHandlers[CMSG_ATTACK_STOP].handler = &WorldSession::handleAttackStopOpcode;
    WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler = &WorldSession::handleDuelAccepted;
    WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler = &WorldSession::handleDuelCancelled;

    // Trade
    WorldPacketHandlers[CMSG_INITIATE_TRADE].handler = &WorldSession::handleInitiateTradeOpcode;
    WorldPacketHandlers[CMSG_BEGIN_TRADE].handler = &WorldSession::handleBeginTradeOpcode;
    //WorldPacketHandlers[CMSG_BUSY_TRADE].handler = &WorldSession::handleBusyTrade;
    //WorldPacketHandlers[CMSG_IGNORE_TRADE].handler = &WorldSession::handleIgnoreTrade;
    WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler = &WorldSession::handleAcceptTrade;
    //WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler = &WorldSession::handleUnacceptTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].handler = &WorldSession::handleCancelTrade;
    WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler = &WorldSession::handleSetTradeItem;
    WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler = &WorldSession::handleClearTradeItem;
    WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler = &WorldSession::handleSetTradeGold;
    // Quest System
    WorldPacketHandlers[CMSG_QUEST_GIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_QUERY].handler = &WorldSession::handleQuestgiverStatusQueryOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_HELLO].handler = &WorldSession::handleQuestgiverHelloOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_ACCEPT_QUEST].handler = &WorldSession::handleQuestgiverAcceptQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_CANCEL].handler = &WorldSession::handleQuestgiverCancelOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_CHOOSE_REWARD].handler = &WorldSession::handleQuestgiverChooseRewardOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_REQUEST_REWARD].handler = &WorldSession::handleQuestgiverRequestRewardOpcode;
    WorldPacketHandlers[CMSG_QUEST_QUERY].handler = &WorldSession::handleQuestQueryOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_QUERY_QUEST].handler = &WorldSession::handleQuestGiverQueryQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_COMPLETE_QUEST].handler = &WorldSession::handleQuestgiverCompleteQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTLOG_REMOVE_QUEST].handler = &WorldSession::handleQuestlogRemoveQuestOpcode;
    WorldPacketHandlers[CMSG_RECLAIM_CORPSE].handler = &WorldSession::handleCorpseReclaimOpcode;
    //WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler = &WorldSession::HandleResurrectResponseOpcode;
    WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler = &WorldSession::handlePushQuestToPartyOpcode;
    //WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler = &WorldSession::handleQuestPushResultOpcode;
    WorldPacketHandlers[CMSG_QUEST_POI_QUERY].handler = &WorldSession::handleQuestPOIQueryOpcode;

    // Auction System
    WorldPacketHandlers[CMSG_AUCTION_LIST_ITEMS].handler = &WorldSession::handleAuctionListItems;
    WorldPacketHandlers[CMSG_AUCTION_LIST_BIDDER_ITEMS].handler = &WorldSession::handleAuctionListBidderItems;
    WorldPacketHandlers[CMSG_AUCTION_SELL_ITEM].handler = &WorldSession::handleAuctionSellItem;
    WorldPacketHandlers[CMSG_AUCTION_LIST_OWNER_ITEMS].handler = &WorldSession::handleAuctionListOwnerItems;
    WorldPacketHandlers[CMSG_AUCTION_PLACE_BID].handler = &WorldSession::handleAuctionPlaceBid;
    WorldPacketHandlers[CMSG_AUCTION_REMOVE_ITEM].handler = &WorldSession::handleCancelAuction;
    WorldPacketHandlers[CMSG_AUCTION_LIST_PENDING_SALES].handler = &WorldSession::handleAuctionListPendingSales;

    // Mail System
    WorldPacketHandlers[CMSG_GET_MAIL_LIST].handler = &WorldSession::handleGetMailOpcode;
    WorldPacketHandlers[CMSG_ITEM_TEXT_QUERY].handler = &WorldSession::handleItemTextQueryOpcode;
    WorldPacketHandlers[CMSG_SEND_MAIL].handler = &WorldSession::handleSendMailOpcode;
    WorldPacketHandlers[CMSG_MAIL_TAKE_MONEY].handler = &WorldSession::handleTakeMoneyOpcode;
    WorldPacketHandlers[CMSG_MAIL_TAKE_ITEM].handler = &WorldSession::handleTakeItemOpcode;
    WorldPacketHandlers[CMSG_MAIL_MARK_AS_READ].handler = &WorldSession::handleMarkAsReadOpcode;
    WorldPacketHandlers[CMSG_MAIL_RETURN_TO_SENDER].handler = &WorldSession::handleReturnToSenderOpcode;
    WorldPacketHandlers[CMSG_MAIL_DELETE].handler = &WorldSession::handleMailDeleteOpcode;
    WorldPacketHandlers[MSG_QUERY_NEXT_MAIL_TIME].handler = &WorldSession::handleMailTimeOpcode;
    WorldPacketHandlers[CMSG_MAIL_CREATE_TEXT_ITEM].handler = &WorldSession::handleMailCreateTextItemOpcode;

    // Guild 
    WorldPacketHandlers[CMSG_GUILD_QUERY].handler = &WorldSession::handleGuildQuery;
    WorldPacketHandlers[CMSG_GUILD_QUERY].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_GUILD_INVITE].handler = &WorldSession::handleInviteToGuild;
    WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler = &WorldSession::handleGuildAccept;
    WorldPacketHandlers[CMSG_GUILD_DECLINE].handler = &WorldSession::handleGuildDecline;
    WorldPacketHandlers[CMSG_GUILD_ROSTER].handler = &WorldSession::handleGuildRoster;
    WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler = &WorldSession::handleGuildPromote;
    WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler = &WorldSession::handleGuildDemote;
    WorldPacketHandlers[CMSG_GUILD_LEAVE].handler = &WorldSession::handleGuildLeave;
    WorldPacketHandlers[CMSG_GUILD_REMOVE].handler = &WorldSession::handleGuildRemove;
    WorldPacketHandlers[CMSG_GUILD_DISBAND].handler = &WorldSession::handleGuildDisband;
    WorldPacketHandlers[CMSG_GUILD_LEADER].handler = &WorldSession::handleGuildLeader;
    WorldPacketHandlers[CMSG_GUILD_MOTD].handler = &WorldSession::handleGuildMotd;
    WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler = &WorldSession::handleGuildAddRank;
    WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler = &WorldSession::handleGuildDelRank;
    WorldPacketHandlers[CMSG_PETITION_BUY].handler = &WorldSession::handleCharterBuy;
    WorldPacketHandlers[CMSG_PETITION_SHOW_SIGNATURES].handler = &WorldSession::handleCharterShowSignatures;
    WorldPacketHandlers[CMSG_TURN_IN_PETITION].handler = &WorldSession::handleCharterTurnInCharter;
    WorldPacketHandlers[CMSG_PETITION_QUERY].handler = &WorldSession::handleCharterQuery;
    WorldPacketHandlers[CMSG_OFFER_PETITION].handler = &WorldSession::handleCharterOffer;
    WorldPacketHandlers[CMSG_PETITION_SIGN].handler = &WorldSession::handleCharterSign;
    WorldPacketHandlers[MSG_PETITION_DECLINE].handler = &WorldSession::handleCharterDecline;
    WorldPacketHandlers[MSG_PETITION_RENAME].handler = &WorldSession::handleCharterRename;
    WorldPacketHandlers[MSG_SAVE_GUILD_EMBLEM].handler = &WorldSession::handleSaveGuildEmblem;
    WorldPacketHandlers[CMSG_GUILD_INFO_TEXT].handler = &WorldSession::handleSetGuildInfo;
    WorldPacketHandlers[CMSG_GUILD_BANK_MONEY_WITHDRAWN_QUERY].handler = &WorldSession::handleGuildBankMoneyWithdrawn;
    WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler = &WorldSession::handleGuildBankerActivate;
    WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler = &WorldSession::handleGuildBankQueryTab;
    WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler = &WorldSession::handleGuildBankSwapItems;
    WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler = &WorldSession::handleGuildBankBuyTab;
    WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler = &WorldSession::handleGuildBankUpdateTab;
    WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler = &WorldSession::handleGuildBankLogQuery;
    WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TEXT].handler = &WorldSession::handleQueryGuildBankTabText;
    WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler = &WorldSession::handleGuildBankWithdrawMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler = &WorldSession::handleGuildBankDepositMoney;
    WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler = &WorldSession::handleSetGuildBankText;
    WorldPacketHandlers[CMSG_QUERY_GUILD_XP].handler = &WorldSession::handleGuildQueryXPOpcode;
    WorldPacketHandlers[CMSG_GUILD_SET_RANK].handler = &WorldSession::handleGuildSetRank;
    WorldPacketHandlers[CMSG_GUILD_SET_NOTE].handler = &WorldSession::handleGuildSetNoteOpcode;
    WorldPacketHandlers[CMSG_QUERY_GUILD_REWARDS].handler = &WorldSession::handleGuildRewardsQueryOpcode;
    WorldPacketHandlers[CMSG_GUILD_QUERY_RANKS].handler = &WorldSession::handleGuildQueryRanksOpcode;
    WorldPacketHandlers[CMSG_GUILD_ASSIGN_MEMBER_RANK].handler = &WorldSession::handleGuildAssignRankOpcode;
    WorldPacketHandlers[CMSG_GUILD_EVENT_LOG_QUERY].handler = &WorldSession::handleGuildLog;
    WorldPacketHandlers[CMSG_GUILD_REQUEST_CHALLENGE_UPDATE].handler = &WorldSession::handleGuildRequestChallengeUpdate;
    WorldPacketHandlers[CMSG_GUILD_REQUEST_MAX_DAILY_XP].handler = &WorldSession::handleGuildRequestMaxDailyXP;
    WorldPacketHandlers[CMSG_GUILD_QUERY_NEWS].handler = &WorldSession::handleGuildQueryNewsOpcode;
    WorldPacketHandlers[CMSG_GUILD_NEWS_UPDATE_STICKY].handler = &WorldSession::handleGuildNewsUpdateStickyOpcode;
    WorldPacketHandlers[CMSG_GUILD_PERMISSIONS].handler = &WorldSession::handleGuildPermissions;

    // Guild Finder
    WorldPacketHandlers[CMSG_LF_GUILD_GET_RECRUITS].handler = &WorldSession::handleGuildFinderGetRecruits;
    WorldPacketHandlers[CMSG_LF_GUILD_ADD_RECRUIT].handler = &WorldSession::handleGuildFinderAddRecruit;
    WorldPacketHandlers[CMSG_LF_GUILD_BROWSE].handler = &WorldSession::handleGuildFinderBrowse;
    WorldPacketHandlers[CMSG_LF_GUILD_DECLINE_RECRUIT].handler = &WorldSession::handleGuildFinderDeclineRecruit;
    WorldPacketHandlers[CMSG_LF_GUILD_GET_APPLICATIONS].handler = &WorldSession::handleGuildFinderGetApplications;
    WorldPacketHandlers[CMSG_LF_GUILD_POST_REQUEST].handler = &WorldSession::handleGuildFinderPostRequest;
    WorldPacketHandlers[CMSG_LF_GUILD_REMOVE_RECRUIT].handler = &WorldSession::handleGuildFinderRemoveRecruit;
    WorldPacketHandlers[CMSG_LF_GUILD_SET_GUILD_POST].handler = &WorldSession::handleGuildFinderSetGuildPost;


    // Tutorials
    WorldPacketHandlers[CMSG_TUTORIAL_FLAG].handler = &WorldSession::handleTutorialFlag;
    WorldPacketHandlers[CMSG_TUTORIAL_CLEAR].handler = &WorldSession::handleTutorialClear;
    WorldPacketHandlers[CMSG_TUTORIAL_RESET].handler = &WorldSession::handleTutorialReset;

    // Pets
    //WorldPacketHandlers[CMSG_PET_ACTION].handler = &WorldSession::HandlePetAction;
    //WorldPacketHandlers[CMSG_REQUEST_PET_INFO].handler = &WorldSession::HandlePetInfo;
    //WorldPacketHandlers[CMSG_PET_NAME_QUERY].handler = &WorldSession::HandlePetNameQuery;
    //WorldPacketHandlers[CMSG_BUY_STABLE_SLOT].handler = &WorldSession::HandleBuyStableSlot;
    //WorldPacketHandlers[CMSG_STABLE_PET].handler = &WorldSession::HandleStablePet;
    //WorldPacketHandlers[CMSG_UNSTABLE_PET].handler = &WorldSession::HandleUnstablePet;
    //WorldPacketHandlers[CMSG_STABLE_SWAP_PET].handler = &WorldSession::HandleStableSwapPet;
    //WorldPacketHandlers[MSG_LIST_STABLED_PETS].handler = &WorldSession::HandleStabledPetList;
    //WorldPacketHandlers[CMSG_PET_SET_ACTION].handler = &WorldSession::HandlePetSetActionOpcode;
    //WorldPacketHandlers[CMSG_PET_RENAME].handler = &WorldSession::HandlePetRename;
    //WorldPacketHandlers[CMSG_PET_ABANDON].handler = &WorldSession::HandlePetAbandon;
    //WorldPacketHandlers[CMSG_PET_UNLEARN].handler = &WorldSession::HandlePetUnlearn;
    WorldPacketHandlers[CMSG_PET_SPELL_AUTOCAST].handler = &WorldSession::handlePetSpellAutocast;
    //WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler = &WorldSession::HandlePetCancelAura;
    WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler = &WorldSession::handlePetLearnTalent;
    //WorldPacketHandlers[CMSG_DISMISS_CRITTER].handler = &WorldSession::HandleDismissCritter;

    // Battlegrounds
    //WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler = &WorldSession::HandleBattlefieldPortOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler = &WorldSession::handleBattlefieldStatusOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler = &WorldSession::handleBattlefieldListOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler = &WorldSession::handleBattleMasterHelloOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler = &WorldSession::handleArenaJoinOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler = &WorldSession::handleBattleMasterJoinOpcode;
    //WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler = &WorldSession::HandleLeaveBattlefieldOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler = &WorldSession::handleAreaSpiritHealerQueryOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler = &WorldSession::handleAreaSpiritHealerQueueOpcode;
    //WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler = &WorldSession::HandleBattlegroundPlayerPositionsOpcode;
    //WorldPacketHandlers[MSG_PVP_LOG_DATA].handler = &WorldSession::handlePVPLogDataOpcode;
    WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler = &WorldSession::handleInspectHonorStatsOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler = &WorldSession::handleSetActionBarTogglesOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE].handler = &WorldSession::HandleBgInviteResponse;
    WorldPacketHandlers[CMSG_REQUEST_RATED_BG_INFO].handler = &WorldSession::handleRequestRatedBgInfoOpcode;
    WorldPacketHandlers[CMSG_REQUEST_RATED_BG_STATS].handler = &WorldSession::handleRequestRatedBgStatsOpcode;
    WorldPacketHandlers[CMSG_REQUEST_PVP_REWARDS].handler = &WorldSession::handleRequestPvPRewardsOpcode;
    WorldPacketHandlers[CMSG_REQUEST_PVP_OPTIONS_ENABLED].handler = &WorldSession::handleRequestPvpOptionsOpcode;

    // GM Ticket System
    WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler = &WorldSession::handleGMTicketCreateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler = &WorldSession::handleGMTicketUpdateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler = &WorldSession::handleGMTicketDeleteOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketGetTicketOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler = &WorldSession::handleGMTicketSystemStatusOpcode;
    //WorldPacketHandlers[CMSG_GMTICKETSYSTEM_TOGGLE].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;
    WorldPacketHandlers[SMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;

    // Reports
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].handler = &WorldSession::handleReportLag;
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].handler = &WorldSession::handleGMSurveySubmitOpcode;
    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_REPORT].handler = &WorldSession::handleReportOpcode;
    WorldPacketHandlers[CMSG_REPORT_PLAYER].handler = &WorldSession::handleReportPlayerOpcode;

    // Meeting Stone / Instances
    //WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler = &WorldSession::handleSummonResponseOpcode;
    WorldPacketHandlers[CMSG_RESET_INSTANCES].handler = &WorldSession::handleResetInstanceOpcode;
    //WorldPacketHandlers[CMSG_SELF_RES].handler = &WorldSession::HandleSelfResurrectOpcode;
    //WorldPacketHandlers[MSG_RANDOM_ROLL].handler = &WorldSession::HandleRandomRollOpcode;
    WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler = &WorldSession::handleDungeonDifficultyOpcode;
    WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler = &WorldSession::handleRaidDifficultyOpcode;

    // Misc
    WorldPacketHandlers[CMSG_OPEN_ITEM].handler = &WorldSession::handleOpenItemOpcode;
    WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler = &WorldSession::handleCompleteCinematic;
    WorldPacketHandlers[CMSG_NEXT_CINEMATIC_CAMERA].handler = &WorldSession::handleNextCinematic;
    //WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler = &WorldSession::handleMountSpecialAnimOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler = &WorldSession::HandleToggleCloakOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_HELM].handler = &WorldSession::HandleToggleHelmOpcode;
    //WorldPacketHandlers[CMSG_SET_TITLE].handler = &WorldSession::HandleSetVisibleRankOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::handleGameobjReportUseOpCode;
    WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler = &WorldSession::handlePetCastSpell;
    WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::handleWorldStateUITimerUpdate;
    WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler = &WorldSession::handleSetTaxiBenchmarkOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler = &WorldSession::handleUnlearnSkillOpcode;
    WorldPacketHandlers[CMSG_REQUEST_CEMETERY_LIST].handler = &WorldSession::handleRequestCemeteryListOpcode;
    WorldPacketHandlers[CMSG_REQUEST_HOTFIX].handler = &WorldSession::handleRequestHotfix;
    WorldPacketHandlers[CMSG_RETURN_TO_GRAVEYARD].handler = &WorldSession::handleReturnToGraveyardOpcode;
    WorldPacketHandlers[CMSG_BUG].handler = &WorldSession::handleBugOpcode;
    WorldPacketHandlers[CMSG_SUGGESTION_SUBMIT].handler = &WorldSession::handleSuggestionOpcode;
    WorldPacketHandlers[CMSG_LOG_DISCONNECT].handler = &WorldSession::handleLogDisconnectOpcode;

    // Chat
    WorldPacketHandlers[CMSG_CHAT_IGNORED].handler = &WorldSession::handleChatIgnoredOpcode;
    WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler = &WorldSession::handleChatChannelWatchOpcode;

    // Arenas
    //WorldPacketHandlers[CMSG_ARENA_TEAM_QUERY].handler = &WorldSession::handleArenaTeamQueryOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_ROSTER].handler = &WorldSession::handleArenaTeamRosterOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_INVITE].handler = &WorldSession::handleArenaTeamAddMemberOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_ACCEPT].handler = &WorldSession::handleArenaTeamInviteAcceptOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_DECLINE].handler = &WorldSession::handleArenaTeamInviteDenyOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_LEAVE].handler = &WorldSession::handleArenaTeamLeaveOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_REMOVE].handler = &WorldSession::handleArenaTeamRemoveMemberOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_DISBAND].handler = &WorldSession::handleArenaTeamDisbandOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_LEADER].handler = &WorldSession::handleArenaTeamPromoteOpcode;
    //WorldPacketHandlers[MSG_INSPECT_ARENA_TEAMS].handler = &WorldSession::handleInspectArenaStatsOpcode;

    // cheat/gm commands?
    //WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::handleWorldTeleportOpcode;

    // Vehicle
    WorldPacketHandlers[CMSG_DISMISS_CONTROLLED_VEHICLE].handler = &WorldSession::handleDismissVehicle;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_EXIT].handler = &WorldSession::handleLeaveVehicle;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_PREV_SEAT].handler = &WorldSession::handleRequestVehiclePreviousSeat;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_NEXT_SEAT].handler = &WorldSession::handleRequestVehicleNextSeat;
    WorldPacketHandlers[CMSG_REQUEST_VEHICLE_SWITCH_SEAT].handler = &WorldSession::handleRequestVehicleSwitchSeat;
    WorldPacketHandlers[CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE].handler = &WorldSession::handleChangeSeatsOnControlledVehicle;
    WorldPacketHandlers[CMSG_PLAYER_VEHICLE_ENTER].handler = &WorldSession::handleEnterVehicle;
    WorldPacketHandlers[CMSG_EJECT_PASSENGER].handler = &WorldSession::handleRemoveVehiclePassenger;

    // Unsorted
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESPONSE].handler = &WorldSession::HandleTimeSyncResOp; // MiscHandler.cpp
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::HandleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::HandleSetAutoLootPassOpcode;
    //WorldPacketHandlers[CMSG_QUEST_GIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
    //WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::handleRemoveGlyph;
    WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::handleBarberShopResult;
    //WorldPacketHandlers[CMSG_GET_MIRRORIMAGE_DATA].handler = &WorldSession::HandleMirrorImageOpcode;

    // Calendar - Unhandled
    WorldPacketHandlers[CMSG_CALENDAR_GET_CALENDAR].handler = &WorldSession::handleCalendarGetCalendar;
    //WorldPacketHandlers[CMSG_CALENDAR_COMPLAIN].handler = &WorldSession::handleCalendarComplain;
    WorldPacketHandlers[CMSG_CALENDAR_GET_NUM_PENDING].handler = &WorldSession::handleCalendarGetNumPending;
    //WorldPacketHandlers[CMSG_CALENDAR_ADD_EVENT].handler = &WorldSession::handleCalendarAddEvent;

    //WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].handler = &WorldSession::handleCalendarGetEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].handler = &WorldSession::handleCalendarGuildFilter;
    //WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].handler = &WorldSession::handleCalendarArenaTeam;
    //WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].handler = &WorldSession::handleCalendarUpdateEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].handler = &WorldSession::handleCalendarRemoveEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].handler = &WorldSession::handleCalendarCopyEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].handler = &WorldSession::handleCalendarEventInvite;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].handler = &WorldSession::handleCalendarEventRsvp;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].handler = &WorldSession::handleCalendarEventRemoveInvite;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].handler = &WorldSession::handleCalendarEventStatus;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].handler = &WorldSession::handleCalendarEventModeratorStatus;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].status = STATUS_LOGGEDIN;

    //Misc - wanted to be unhandled
    WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_PET_LEVEL_CHEAT].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_QUERY_BATTLEFIELD_STATE].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;

    WorldPacketHandlers[CMSG_ADDON_REGISTERED_PREFIXES].handler = &WorldSession::handleAddonRegisteredPrefixesOpcode;
    WorldPacketHandlers[CMSG_UNREGISTER_ALL_ADDON_PREFIXES].handler = &WorldSession::handleUnregisterAddonPrefixesOpcode;

#elif VERSION_STRING == Mop
    // Login
    WorldPacketHandlers[CMSG_ENUM_CHARACTERS].handler = &WorldSession::handleCharEnumOpcode;
    WorldPacketHandlers[CMSG_ENUM_CHARACTERS].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CREATE].handler = &WorldSession::handleCharCreateOpcode;
    WorldPacketHandlers[CMSG_CHAR_CREATE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_DELETE].handler = &WorldSession::handleCharDeleteOpcode;
    WorldPacketHandlers[CMSG_CHAR_DELETE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_RENAME].handler = &WorldSession::handleCharRenameOpcode;
    //WorldPacketHandlers[CMSG_CHAR_RENAME].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].handler = &WorldSession::handleCharCustomizeLooksOpcode;
    //WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].status = STATUS_AUTHED;

    // declined names (Cyrillic client)
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].handler = &WorldSession::handleSetPlayerDeclinedNamesOpcode;
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler = &WorldSession::handlePlayerLoginOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGIN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_REALM_SPLIT].handler = &WorldSession::handleRealmSplitOpcode;
    WorldPacketHandlers[CMSG_REALM_SPLIT].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].handler = &WorldSession::handleObjectUpdateFailedOpcode;
    //WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_LOADING_SCREEN_NOTIFY].handler = &WorldSession::handleLoadScreenOpcode;
    WorldPacketHandlers[CMSG_LOADING_SCREEN_NOTIFY].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::handleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::handleUITimeRequestOpcode;
    //WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_TIME_SYNC_RESPONSE].handler = &WorldSession::handleTimeSyncRespOpcode;
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESPONSE].status = STATUS_AUTHED;

    // Queries
    //WorldPacketHandlers[MSG_CORPSE_QUERY].handler = &WorldSession::handleCorpseQueryOpcode;
    //WorldPacketHandlers[CMSG_NAME_QUERY].handler = &WorldSession::handleNameQueryOpcode;
    //WorldPacketHandlers[CMSG_QUERY_TIME].handler = &WorldSession::handleQueryTimeOpcode;
    //WorldPacketHandlers[CMSG_CREATURE_QUERY].handler = &WorldSession::handleCreatureQueryOpcode;
    //WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler = &WorldSession::handleGameObjectQueryOpcode;
    //WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler = &WorldSession::handlePageTextQueryOpcode;
    //WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler = &WorldSession::handleItemNameQueryOpcode;
    //WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler = &WorldSession::HandleAchievmentQueryOpcode;

    // Movement
    //WorldPacketHandlers[MSG_MOVE_HEARTBEAT].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_WORLDPORT_ACK].handler = &WorldSession::handleMoveWorldportAckOpcode;
    //WorldPacketHandlers[MSG_MOVE_JUMP].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_FORWARD].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_BACKWARD].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_SET_FACING].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_STRAFE_LEFT].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_STRAFE_RIGHT].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_STOP_STRAFE].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_TURN_LEFT].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_TURN_RIGHT].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_STOP_TURN].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_PITCH_UP].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_PITCH_DOWN].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_STOP_PITCH].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_SET_RUN_MODE].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_SET_WALK_MODE].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_SET_PITCH].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_SWIM].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_STOP_SWIM].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_FALL_LAND].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_STOP].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_SET_FLY].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_NOT_ACTIVE_MOVER].handler = &WorldSession::handleMoveNotActiveMoverOpcode;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_MOVER].handler = &WorldSession::handleSetActiveMoverOpcode;
    //WorldPacketHandlers[CMSG_MOVE_CHNG_TRANSPORT].handler = &WorldSession::handleMovementOpcodes;

    // ACK
    //WorldPacketHandlers[MSG_MOVE_TELEPORT_ACK].handler = &WorldSession::handleMoveTeleportAckOpcode;
    //WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[CMSG_FORCE_PITCH_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;

    // Force Speed Change
    //WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    //WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    //WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    //WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    //WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    //WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    //WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;
    //WorldPacketHandlers[CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleForceSpeedChangeAck;

    // Action Buttons
    //WorldPacketHandlers[CMSG_SET_ACTION_BUTTON].handler = &WorldSession::handleSetActionButtonOpcode;
    //WorldPacketHandlers[CMSG_REPOP_REQUEST].handler = &WorldSession::handleRepopRequestOpcode;

    // Loot
    //WorldPacketHandlers[CMSG_AUTOSTORE_LOOT_ITEM].handler = &WorldSession::handleAutostoreLootItemOpcode;
    //WorldPacketHandlers[CMSG_LOOT_MONEY].handler = &WorldSession::handleLootMoneyOpcode;
    //WorldPacketHandlers[CMSG_LOOT].handler = &WorldSession::handleLootOpcode;
    //WorldPacketHandlers[CMSG_LOOT_RELEASE].handler = &WorldSession::handleLootReleaseOpcode;
    //WorldPacketHandlers[CMSG_LOOT_ROLL].handler = &WorldSession::handleLootRollOpcode;
    //WorldPacketHandlers[CMSG_LOOT_MASTER_GIVE].handler = &WorldSession::handleLootMasterGiveOpcode;

    // Player Interaction
    //WorldPacketHandlers[CMSG_WHO].handler = &WorldSession::handleWhoOpcode;
    //WorldPacketHandlers[CMSG_WHOIS].handler = &WorldSession::handleWhoIsOpcode;
    //WorldPacketHandlers[CMSG_LOGOUT_REQUEST].handler = &WorldSession::handleLogoutRequestOpcode;
    //WorldPacketHandlers[CMSG_PLAYER_LOGOUT].handler = &WorldSession::handlePlayerLogoutOpcode;
    //WorldPacketHandlers[CMSG_LOGOUT_CANCEL].handler = &WorldSession::handleLogoutCancelOpcode;
    //WorldPacketHandlers[CMSG_LOGOUT_CANCEL].status = STATUS_LOGGEDIN_RECENTLY_LOGGOUT;

    //WorldPacketHandlers[CMSG_ZONEUPDATE].handler = &WorldSession::handleZoneupdate;
    //WorldPacketHandlers[CMSG_SET_TARGET_OBSOLETE].handler = &WorldSession::handleSetTargetOpcode;
    //WorldPacketHandlers[CMSG_SET_SELECTION].handler = &WorldSession::handleSetSelectionOpcode;
    //WorldPacketHandlers[CMSG_STANDSTATECHANGE].handler = &WorldSession::handleStandStateChangeOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_MOUNT_AURA].handler = &WorldSession::HandleDismountOpcode;

    // Friends
    //WorldPacketHandlers[CMSG_CONTACT_LIST].handler = &WorldSession::handleFriendListOpcode;
    //WorldPacketHandlers[CMSG_ADD_FRIEND].handler = &WorldSession::handleAddFriendOpcode;
    //WorldPacketHandlers[CMSG_DEL_FRIEND].handler = &WorldSession::handleDelFriendOpcode;
    //WorldPacketHandlers[CMSG_ADD_IGNORE].handler = &WorldSession::handleAddIgnoreOpcode;
    //WorldPacketHandlers[CMSG_DEL_IGNORE].handler = &WorldSession::handleDelIgnoreOpcode;
    // WorldPacketHandlers[CMSG_SET_CONTACT_NOTES].handler = &WorldSession::handleSetContactNotes;

    // Areatrigger
    //WorldPacketHandlers[CMSG_AREATRIGGER].handler = &WorldSession::HandleAreaTriggerOpcode;

    // Account Data
    //WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].handler = &WorldSession::handleUpdateAccountData;
    //WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].status = STATUS_AUTHED;
    //WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].handler = &WorldSession::handleRequestAccountData;
    //WorldPacketHandlers[CMSG_TOGGLE_PVP].handler = &WorldSession::handleTogglePVPOpcode;

    // Faction / Reputation
    //WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler = &WorldSession::HandleSetAtWarOpcode;
    //WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler = &WorldSession::HandleSetWatchedFactionIndexOpcode;
    //WorldPacketHandlers[CMSG_SET_FACTION_INACTIVE].handler = &WorldSession::HandleSetFactionInactiveOpcode;

    // Player Interaction
    //WorldPacketHandlers[CMSG_GAMEOBJ_USE].handler = &WorldSession::handleGameObjectUse;
    //WorldPacketHandlers[CMSG_REQUEST_PLAYED_TIME].handler = &WorldSession::handlePlayedTimeOpcode;
    //WorldPacketHandlers[CMSG_SETSHEATHED].handler = &WorldSession::handleSetSheathedOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT].handler = &WorldSession::HandleMessagechatOpcode;
    //WorldPacketHandlers[CMSG_EMOTE].handler = &WorldSession::handleEmoteOpcode;
    //WorldPacketHandlers[CMSG_TEXT_EMOTE].handler = &WorldSession::handleTextEmoteOpcode;
    //WorldPacketHandlers[CMSG_INSPECT].handler = &WorldSession::handleInspectOpcode;
    //WorldPacketHandlers[SMSG_BARBER_SHOP_RESULT].handler = &WorldSession::handleBarberShopResult;

    //WorldPacketHandlers[CMSG_MESSAGECHAT_SAY].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_YELL].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_CHANNEL].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_WHISPER].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_GUILD].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_OFFICER].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_AFK].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_DND].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_EMOTE].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_PARTY].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_RAID].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_RAID_WARNING].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT_BATTLEGROUND].handler = &WorldSession::handleMessageChatOpcode;

    // Channels
    //WorldPacketHandlers[CMSG_JOIN_CHANNEL].handler = &WorldSession::handleChannelJoin;
    //WorldPacketHandlers[CMSG_LEAVE_CHANNEL].handler = &WorldSession::handleChannelLeave;
    //WorldPacketHandlers[CMSG_CHANNEL_LIST].handler = &WorldSession::handleChannelList;
    //WorldPacketHandlers[CMSG_CHANNEL_PASSWORD].handler = &WorldSession::handleChannelPassword;
    //WorldPacketHandlers[CMSG_CHANNEL_SET_OWNER].handler = &WorldSession::handleChannelSetOwner;
    //WorldPacketHandlers[CMSG_CHANNEL_OWNER].handler = &WorldSession::handleChannelOwner;
    //WorldPacketHandlers[CMSG_CHANNEL_MODERATOR].handler = &WorldSession::handleChannelModerator;
    //WorldPacketHandlers[CMSG_CHANNEL_UNMODERATOR].handler = &WorldSession::handleChannelUnmoderator;
    //WorldPacketHandlers[CMSG_CHANNEL_MUTE].handler = &WorldSession::handleChannelMute;
    //WorldPacketHandlers[CMSG_CHANNEL_UNMUTE].handler = &WorldSession::handleChannelUnmute;
    //WorldPacketHandlers[CMSG_CHANNEL_INVITE].handler = &WorldSession::handleChannelInvite;
    //WorldPacketHandlers[CMSG_CHANNEL_KICK].handler = &WorldSession::handleChannelKick;
    //WorldPacketHandlers[CMSG_CHANNEL_BAN].handler = &WorldSession::handleChannelBan;
    //WorldPacketHandlers[CMSG_CHANNEL_UNBAN].handler = &WorldSession::handleChannelUnban;
    //WorldPacketHandlers[CMSG_CHANNEL_ANNOUNCEMENTS].handler = &WorldSession::handleChannelAnnounce;
    //WorldPacketHandlers[CMSG_CHANNEL_MODERATE].handler = &WorldSession::handleChannelModerate;
    //WorldPacketHandlers[CMSG_GET_CHANNEL_MEMBER_COUNT].handler = &WorldSession::handleGetChannelMemberCount;
    //WorldPacketHandlers[CMSG_CHANNEL_DISPLAY_LIST].handler = &WorldSession::handleChannelRosterQuery;

    // Groups / Raids
    //WorldPacketHandlers[CMSG_GROUP_INVITE].handler = &WorldSession::handleGroupInviteOpcode;
    //WorldPacketHandlers[CMSG_GROUP_INVITE_RESPONSE].handler = &WorldSession::handleGroupInviteResponseOpcode;
    //WorldPacketHandlers[CMSG_GROUP_SET_ROLES].handler = &WorldSession::handleGroupSetRolesOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CANCEL].handler = &WorldSession::HandleGroupCancelOpcode;
    //WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler = &WorldSession::HandleGroupAcceptOpcode;
    //WorldPacketHandlers[CMSG_GROUP_DECLINE].handler = &WorldSession::HandleGroupDeclineOpcode;
    //WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler = &WorldSession::HandleGroupUninviteOpcode;
    //WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler = &WorldSession::handleGroupUninviteGuidOpcode;
    //WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler = &WorldSession::handleGroupSetLeaderOpcode;
    //WorldPacketHandlers[CMSG_GROUP_DISBAND].handler = &WorldSession::handleGroupDisbandOpcode;
    //WorldPacketHandlers[CMSG_LOOT_METHOD].handler = &WorldSession::handleLootMethodOpcode;
    //WorldPacketHandlers[MSG_MINIMAP_PING].handler = &WorldSession::HandleMinimapPingOpcode;
    //WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler = &WorldSession::handleConvertGroupToRaidOpcode;
    //WorldPacketHandlers[CMSG_GROUP_REQUEST_JOIN_UPDATES].handler = &WorldSession::handleGroupRequestJoinUpdatesOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler = &WorldSession::HandleGroupChangeSubGroup;
    //WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler = &WorldSession::HandleGroupAssistantLeader;
    //WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler = &WorldSession::handleRequestRaidInfoOpcode;
    //WorldPacketHandlers[MSG_RAID_READY_CHECK].handler = &WorldSession::handleReadyCheckOpcode;
    //WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler = &WorldSession::HandleSetPlayerIconOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler = &WorldSession::HandlePartyMemberStatsOpcode;
    //WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler = &WorldSession::HandleGroupPromote;
    //WorldPacketHandlers[CMSG_ROLE_CHECK_BEGIN].handler = &WorldSession::handleGroupRoleCheckBeginOpcode;
    //WorldPacketHandlers[CMSG_MAKE_EVERYONE_ASSISTANT].handler = &WorldSession::nothingToHandle;
    //WorldPacketHandlers[MSG_RAID_READY_CHECK_FINISHED].handler = &WorldSession::nothingToHandle;

    // LFG System
    //WorldPacketHandlers[CMSG_LFG_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgLockInfoOpcode;
    //WorldPacketHandlers[CMSG_SET_LFG_COMMENT].handler = &WorldSession::handleLfgSetCommentOpcode;
    //WorldPacketHandlers[CMSG_LFG_JOIN].handler = &WorldSession::handleLfgJoinOpcode;
    //WorldPacketHandlers[CMSG_LFG_LEAVE].handler = &WorldSession::handleLfgLeaveOpcode;
    //WorldPacketHandlers[CMSG_SEARCH_LFG_JOIN].handler = &WorldSession::handleLfgSearchOpcode;
    //WorldPacketHandlers[CMSG_SEARCH_LFG_LEAVE].handler = &WorldSession::handleLfgSearchLeaveOpcode;
    //WorldPacketHandlers[CMSG_LFG_PROPOSAL_RESULT].handler = &WorldSession::handleLfgProposalResultOpcode;
    //WorldPacketHandlers[CMSG_LFG_SET_ROLES].handler = &WorldSession::handleLfgSetRolesOpcode;
    //WorldPacketHandlers[CMSG_LFG_SET_BOOT_VOTE].handler = &WorldSession::handleLfgSetBootVoteOpcode;
    //WorldPacketHandlers[CMSG_LFD_PLAYER_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPlayerLockInfoRequestOpcode;
    //WorldPacketHandlers[CMSG_LFG_TELEPORT].handler = &WorldSession::handleLfgTeleportOpcode;
    //WorldPacketHandlers[CMSG_LFD_PARTY_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPartyLockInfoRequestOpcode;

    // Taxi / NPC Interaction
    //WorldPacketHandlers[CMSG_ENABLETAXI].handler = &WorldSession::handleEnabletaxiOpcode;
    //WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler = &WorldSession::handleTaxiNodeStatusQueryOpcode;
    //WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler = &WorldSession::handleTaxiQueryAvaibleNodesOpcode;
    //WorldPacketHandlers[CMSG_ACTIVATE_TAXI].handler = &WorldSession::handleActivateTaxiOpcode;
    //WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler = &WorldSession::HandleTabardVendorActivateOpcode;
    //WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler = &WorldSession::handleBankerActivateOpcode;
    //WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler = &WorldSession::handleBuyBankSlotOpcode;
    //WorldPacketHandlers[CMSG_TRAINER_LIST].handler = &WorldSession::handleTrainerListOpcode;
    //WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler = &WorldSession::handleTrainerBuySpellOpcode;
    //WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler = &WorldSession::HandleCharterShowListOpcode;
    //WorldPacketHandlers[MSG_AUCTION_HELLO].handler = &WorldSession::handleAuctionHelloOpcode;
    //WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler = &WorldSession::handleGossipHelloOpcode;
    //WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler = &WorldSession::handleGossipSelectOptionOpcode;
    //WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler = &WorldSession::handleSpiritHealerActivateOpcode;
    //WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler = &WorldSession::handleNpcTextQueryOpcode;
    //WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler = &WorldSession::handleBinderActivateOpcode;
    //WorldPacketHandlers[CMSG_ACTIVATE_TAXI_EXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;
    //WorldPacketHandlers[CMSG_MOVE_SPLINE_DONE].handler = &WorldSession::handleMoveSplineDoneOpcode;

    // Item / Vendors
    //WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    //WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    //WorldPacketHandlers[CMSG_DESTROY_ITEM].handler = &WorldSession::handleDestroyItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    //WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    //WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    //WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    //WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    //WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    //WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    //WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    //WorldPacketHandlers[CMSG_BUY_BACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
    //WorldPacketHandlers[CMSG_SPLIT_ITEM].handler = &WorldSession::handleSplitOpcode;
    //WorldPacketHandlers[CMSG_READ_ITEM].handler = &WorldSession::handleReadItemOpcode;
    //WorldPacketHandlers[CMSG_REPAIR_ITEM].handler = &WorldSession::handleRepairItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler = &WorldSession::handleAutoBankItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler = &WorldSession::handleAutoStoreBankItemOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler = &WorldSession::handleCancelTemporaryEnchantmentOpcode;
    //WorldPacketHandlers[CMSG_SOCKET_GEMS].handler = &WorldSession::handleInsertGemOpcode;
    //WorldPacketHandlers[CMSG_WRAP_ITEM].handler = &WorldSession::handleWrapItemOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler = &WorldSession::handleItemRefundInfoOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler = &WorldSession::handleItemRefundRequestOpcode;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler = &WorldSession::handleEquipmentSetSave;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler = &WorldSession::handleEquipmentSetUse;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler = &WorldSession::handleEquipmentSetDelete;

    // Spell System
    //WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    //WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    //WorldPacketHandlers[CMSG_SPELL_CLICK].handler = &WorldSession::handleSpellClick;
    //WorldPacketHandlers[CMSG_CANCEL_CAST].handler = &WorldSession::handleCancelCastOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_AURA].handler = &WorldSession::handleCancelAuraOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler = &WorldSession::handleCancelChannellingOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler = &WorldSession::handleCancelAutoRepeatSpellOpcode;
    //WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler = &WorldSession::handleCancelTotem;
    //WorldPacketHandlers[CMSG_UPDATE_PROJECTILE_POSITION].handler = &WorldSession::handleUpdateProjectilePosition;
    
    // Talent System
    //WorldPacketHandlers[CMSG_LEARN_TALENT].handler = &WorldSession::handleLearnTalentOpcode;
    //WorldPacketHandlers[CMSG_LEARN_PREVIEW_TALENTS].handler = &WorldSession::handleLearnPreviewTalentsOpcode;
    //WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler = &WorldSession::handleLearnMultipleTalentsOpcode;
    //WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler = &WorldSession::handleUnlearnTalents;
    //WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler = &WorldSession::handleUnlearnTalents;

    // Combat / Duel
    //WorldPacketHandlers[CMSG_ATTACKSWING].handler = &WorldSession::handleAttackSwingOpcode;
    //WorldPacketHandlers[CMSG_ATTACK_STOP].handler = &WorldSession::handleAttackStopOpcode;
    //WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler = &WorldSession::handleDuelAccepted;
    //WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler = &WorldSession::handleDuelCancelled;

    // Trade
    //WorldPacketHandlers[CMSG_INITIATE_TRADE].handler = &WorldSession::handleInitiateTradeOpcode;
    //WorldPacketHandlers[CMSG_BEGIN_TRADE].handler = &WorldSession::handleBeginTradeOpcode;
    //WorldPacketHandlers[CMSG_BUSY_TRADE].handler = &WorldSession::handleBusyTrade;
    //WorldPacketHandlers[CMSG_IGNORE_TRADE].handler = &WorldSession::handleIgnoreTrade;
    //WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler = &WorldSession::handleAcceptTrade;
    //WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler = &WorldSession::handleUnacceptTrade;
    //WorldPacketHandlers[CMSG_CANCEL_TRADE].handler = &WorldSession::handleCancelTrade;
    //WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler = &WorldSession::handleSetTradeItem;
    //WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler = &WorldSession::handleClearTradeItem;
    //WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler = &WorldSession::handleSetTradeGold;
    // Quest System
    //WorldPacketHandlers[CMSG_QUEST_GIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
    //WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_QUERY].handler = &WorldSession::handleQuestgiverStatusQueryOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_HELLO].handler = &WorldSession::handleQuestgiverHelloOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_ACCEPT_QUEST].handler = &WorldSession::handleQuestgiverAcceptQuestOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_CANCEL].handler = &WorldSession::handleQuestgiverCancelOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_CHOOSE_REWARD].handler = &WorldSession::handleQuestgiverChooseRewardOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_REQUEST_REWARD].handler = &WorldSession::handleQuestgiverRequestRewardOpcode;
    //WorldPacketHandlers[CMSG_QUEST_QUERY].handler = &WorldSession::handleQuestQueryOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_QUERY_QUEST].handler = &WorldSession::handleQuestGiverQueryQuestOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_COMPLETE_QUEST].handler = &WorldSession::handleQuestgiverCompleteQuestOpcode;
    //WorldPacketHandlers[CMSG_QUESTLOG_REMOVE_QUEST].handler = &WorldSession::handleQuestlogRemoveQuestOpcode;
    //WorldPacketHandlers[CMSG_RECLAIM_CORPSE].handler = &WorldSession::handleCorpseReclaimOpcode;
    //WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler = &WorldSession::HandleResurrectResponseOpcode;
    //WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler = &WorldSession::handlePushQuestToPartyOpcode;
    //WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler = &WorldSession::handleQuestPushResultOpcode;
    //WorldPacketHandlers[CMSG_QUEST_POI_QUERY].handler = &WorldSession::handleQuestPOIQueryOpcode;

    // Auction System
    //WorldPacketHandlers[CMSG_AUCTION_LIST_ITEMS].handler = &WorldSession::handleAuctionListItems;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_BIDDER_ITEMS].handler = &WorldSession::handleAuctionListBidderItems;
    //WorldPacketHandlers[CMSG_AUCTION_SELL_ITEM].handler = &WorldSession::handleAuctionSellItem;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_OWNER_ITEMS].handler = &WorldSession::handleAuctionListOwnerItems;
    //WorldPacketHandlers[CMSG_AUCTION_PLACE_BID].handler = &WorldSession::handleAuctionPlaceBid;
    //WorldPacketHandlers[CMSG_AUCTION_REMOVE_ITEM].handler = &WorldSession::handleCancelAuction;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_PENDING_SALES].handler = &WorldSession::handleAuctionListPendingSales;

    // Mail System
    //WorldPacketHandlers[CMSG_GET_MAIL_LIST].handler = &WorldSession::handleGetMailOpcode;
    //WorldPacketHandlers[CMSG_ITEM_TEXT_QUERY].handler = &WorldSession::handleItemTextQueryOpcode;
    //WorldPacketHandlers[CMSG_SEND_MAIL].handler = &WorldSession::handleSendMailOpcode;
    //WorldPacketHandlers[CMSG_MAIL_TAKE_MONEY].handler = &WorldSession::handleTakeMoneyOpcode;
    //WorldPacketHandlers[CMSG_MAIL_TAKE_ITEM].handler = &WorldSession::handleTakeItemOpcode;
    //WorldPacketHandlers[CMSG_MAIL_MARK_AS_READ].handler = &WorldSession::handleMarkAsReadOpcode;
    //WorldPacketHandlers[CMSG_MAIL_RETURN_TO_SENDER].handler = &WorldSession::handleReturnToSenderOpcode;
    //WorldPacketHandlers[CMSG_MAIL_DELETE].handler = &WorldSession::handleMailDeleteOpcode;
    //WorldPacketHandlers[MSG_QUERY_NEXT_MAIL_TIME].handler = &WorldSession::handleMailTimeOpcode;
    //WorldPacketHandlers[CMSG_MAIL_CREATE_TEXT_ITEM].handler = &WorldSession::handleMailCreateTextItemOpcode;

    // Guild 
    //WorldPacketHandlers[CMSG_GUILD_QUERY].handler = &WorldSession::handleGuildQuery;
    //WorldPacketHandlers[CMSG_GUILD_QUERY].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_GUILD_INVITE].handler = &WorldSession::handleInviteToGuild;
    //WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler = &WorldSession::handleGuildAccept;
    //WorldPacketHandlers[CMSG_GUILD_DECLINE].handler = &WorldSession::handleGuildDecline;
    //WorldPacketHandlers[CMSG_GUILD_ROSTER].handler = &WorldSession::handleGuildRoster;
    //WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler = &WorldSession::handleGuildPromote;
    //WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler = &WorldSession::handleGuildDemote;
    //WorldPacketHandlers[CMSG_GUILD_LEAVE].handler = &WorldSession::handleGuildLeave;
    //WorldPacketHandlers[CMSG_GUILD_REMOVE].handler = &WorldSession::handleGuildRemove;
    //WorldPacketHandlers[CMSG_GUILD_DISBAND].handler = &WorldSession::handleGuildDisband;
    //WorldPacketHandlers[CMSG_GUILD_LEADER].handler = &WorldSession::handleGuildLeader;
    //WorldPacketHandlers[CMSG_GUILD_MOTD].handler = &WorldSession::handleGuildMotd;
    //WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler = &WorldSession::handleGuildAddRank;
    //WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler = &WorldSession::handleGuildDelRank;
    //WorldPacketHandlers[CMSG_PETITION_BUY].handler = &WorldSession::handleCharterBuy;
    //WorldPacketHandlers[CMSG_PETITION_SHOW_SIGNATURES].handler = &WorldSession::handleCharterShowSignatures;
    //WorldPacketHandlers[CMSG_TURN_IN_PETITION].handler = &WorldSession::handleCharterTurnInCharter;
    //WorldPacketHandlers[CMSG_PETITION_QUERY].handler = &WorldSession::handleCharterQuery;
    //WorldPacketHandlers[CMSG_OFFER_PETITION].handler = &WorldSession::handleCharterOffer;
    //WorldPacketHandlers[CMSG_PETITION_SIGN].handler = &WorldSession::handleCharterSign;
    //WorldPacketHandlers[MSG_PETITION_DECLINE].handler = &WorldSession::handleCharterDecline;
    //WorldPacketHandlers[MSG_PETITION_RENAME].handler = &WorldSession::handleCharterRename;
    //WorldPacketHandlers[MSG_SAVE_GUILD_EMBLEM].handler = &WorldSession::handleSaveGuildEmblem;
    //WorldPacketHandlers[CMSG_GUILD_INFO_TEXT].handler = &WorldSession::handleSetGuildInfo;
    //WorldPacketHandlers[CMSG_GUILD_BANK_MONEY_WITHDRAWN_QUERY].handler = &WorldSession::handleGuildBankMoneyWithdrawn;
    //WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler = &WorldSession::handleGuildBankerActivate;
    //WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler = &WorldSession::handleGuildBankQueryTab;
    //WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler = &WorldSession::handleGuildBankSwapItems;
    //WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler = &WorldSession::handleGuildBankBuyTab;
    //WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler = &WorldSession::handleGuildBankUpdateTab;
    //WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler = &WorldSession::handleGuildBankLogQuery;
    //WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TEXT].handler = &WorldSession::handleQueryGuildBankTabText;
    //WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler = &WorldSession::handleGuildBankWithdrawMoney;
    //WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler = &WorldSession::handleGuildBankDepositMoney;
    //WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler = &WorldSession::handleSetGuildBankText;
    //WorldPacketHandlers[CMSG_QUERY_GUILD_XP].handler = &WorldSession::handleGuildQueryXPOpcode;
    //WorldPacketHandlers[CMSG_GUILD_SET_RANK].handler = &WorldSession::handleGuildSetRank;
    //WorldPacketHandlers[CMSG_GUILD_SET_NOTE].handler = &WorldSession::handleGuildSetNoteOpcode;
    //WorldPacketHandlers[CMSG_QUERY_GUILD_REWARDS].handler = &WorldSession::handleGuildRewardsQueryOpcode;
    //WorldPacketHandlers[CMSG_GUILD_QUERY_RANKS].handler = &WorldSession::handleGuildQueryRanksOpcode;
    //WorldPacketHandlers[CMSG_GUILD_ASSIGN_MEMBER_RANK].handler = &WorldSession::handleGuildAssignRankOpcode;
    //WorldPacketHandlers[CMSG_GUILD_EVENT_LOG_QUERY].handler = &WorldSession::handleGuildLog;
    //WorldPacketHandlers[CMSG_GUILD_REQUEST_CHALLENGE_UPDATE].handler = &WorldSession::handleGuildRequestChallengeUpdate;
    //WorldPacketHandlers[CMSG_GUILD_REQUEST_MAX_DAILY_XP].handler = &WorldSession::handleGuildRequestMaxDailyXP;
    //WorldPacketHandlers[CMSG_GUILD_QUERY_NEWS].handler = &WorldSession::handleGuildQueryNewsOpcode;
    //WorldPacketHandlers[CMSG_GUILD_NEWS_UPDATE_STICKY].handler = &WorldSession::handleGuildNewsUpdateStickyOpcode;
    //WorldPacketHandlers[CMSG_GUILD_PERMISSIONS].handler = &WorldSession::handleGuildPermissions;

    // Guild Finder
    //WorldPacketHandlers[CMSG_LF_GUILD_GET_RECRUITS].handler = &WorldSession::handleGuildFinderGetRecruits;
    //WorldPacketHandlers[CMSG_LF_GUILD_ADD_RECRUIT].handler = &WorldSession::handleGuildFinderAddRecruit;
    //WorldPacketHandlers[CMSG_LF_GUILD_BROWSE].handler = &WorldSession::handleGuildFinderBrowse;
    //WorldPacketHandlers[CMSG_LF_GUILD_DECLINE_RECRUIT].handler = &WorldSession::handleGuildFinderDeclineRecruit;
    //WorldPacketHandlers[CMSG_LF_GUILD_GET_APPLICATIONS].handler = &WorldSession::handleGuildFinderGetApplications;
    //WorldPacketHandlers[CMSG_LF_GUILD_POST_REQUEST].handler = &WorldSession::handleGuildFinderPostRequest;
    //WorldPacketHandlers[CMSG_LF_GUILD_REMOVE_RECRUIT].handler = &WorldSession::handleGuildFinderRemoveRecruit;
    //WorldPacketHandlers[CMSG_LF_GUILD_SET_GUILD_POST].handler = &WorldSession::handleGuildFinderSetGuildPost;

    // Tutorials
    //WorldPacketHandlers[CMSG_TUTORIAL_FLAG].handler = &WorldSession::handleTutorialFlag;
    //WorldPacketHandlers[CMSG_TUTORIAL_CLEAR].handler = &WorldSession::handleTutorialClear;
    //WorldPacketHandlers[CMSG_TUTORIAL_RESET].handler = &WorldSession::handleTutorialReset;

    // Pets
    //WorldPacketHandlers[CMSG_PET_ACTION].handler = &WorldSession::HandlePetAction;
    //WorldPacketHandlers[CMSG_REQUEST_PET_INFO].handler = &WorldSession::HandlePetInfo;
    //WorldPacketHandlers[CMSG_PET_NAME_QUERY].handler = &WorldSession::HandlePetNameQuery;
    //WorldPacketHandlers[CMSG_BUY_STABLE_SLOT].handler = &WorldSession::HandleBuyStableSlot;
    //WorldPacketHandlers[CMSG_STABLE_PET].handler = &WorldSession::HandleStablePet;
    //WorldPacketHandlers[CMSG_UNSTABLE_PET].handler = &WorldSession::HandleUnstablePet;
    //WorldPacketHandlers[CMSG_STABLE_SWAP_PET].handler = &WorldSession::HandleStableSwapPet;
    //WorldPacketHandlers[MSG_LIST_STABLED_PETS].handler = &WorldSession::HandleStabledPetList;
    //WorldPacketHandlers[CMSG_PET_SET_ACTION].handler = &WorldSession::HandlePetSetActionOpcode;
    //WorldPacketHandlers[CMSG_PET_RENAME].handler = &WorldSession::HandlePetRename;
    //WorldPacketHandlers[CMSG_PET_ABANDON].handler = &WorldSession::HandlePetAbandon;
    //WorldPacketHandlers[CMSG_PET_UNLEARN].handler = &WorldSession::HandlePetUnlearn;
    //WorldPacketHandlers[CMSG_PET_SPELL_AUTOCAST].handler = &WorldSession::handlePetSpellAutocast;
    //WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler = &WorldSession::HandlePetCancelAura;
    //WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler = &WorldSession::handlePetLearnTalent;
    //WorldPacketHandlers[CMSG_DISMISS_CRITTER].handler = &WorldSession::HandleDismissCritter;

    // Battlegrounds
    //WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler = &WorldSession::HandleBattlefieldPortOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler = &WorldSession::handleBattlefieldStatusOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler = &WorldSession::handleBattlefieldListOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler = &WorldSession::handleBattleMasterHelloOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler = &WorldSession::handleArenaJoinOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler = &WorldSession::handleBattleMasterJoinOpcode;
    //WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler = &WorldSession::HandleLeaveBattlefieldOpcode;
    //WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler = &WorldSession::handleAreaSpiritHealerQueryOpcode;
    //WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler = &WorldSession::handleAreaSpiritHealerQueueOpcode;
    //WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler = &WorldSession::HandleBattlegroundPlayerPositionsOpcode;
    //WorldPacketHandlers[MSG_PVP_LOG_DATA].handler = &WorldSession::handlePVPLogDataOpcode;
    //WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler = &WorldSession::handleInspectHonorStatsOpcode;
    //WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler = &WorldSession::handleSetActionBarTogglesOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE].handler = &WorldSession::HandleBgInviteResponse;
    //WorldPacketHandlers[CMSG_REQUEST_RATED_BG_INFO].handler = &WorldSession::handleRequestRatedBgInfoOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_RATED_BG_STATS].handler = &WorldSession::handleRequestRatedBgStatsOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_PVP_REWARDS].handler = &WorldSession::handleRequestPvPRewardsOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_PVP_OPTIONS_ENABLED].handler = &WorldSession::handleRequestPvpOptionsOpcode;

    // GM Ticket System
    //WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler = &WorldSession::handleGMTicketCreateOpcode;
    //WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler = &WorldSession::handleGMTicketUpdateOpcode;
    //WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler = &WorldSession::handleGMTicketDeleteOpcode;
    //WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketGetTicketOpcode;
    //WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler = &WorldSession::handleGMTicketSystemStatusOpcode;
    //WorldPacketHandlers[CMSG_GMTICKETSYSTEM_TOGGLE].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;
    //WorldPacketHandlers[SMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;

    // Reports
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].handler = &WorldSession::handleReportLag;
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].handler = &WorldSession::handleGMSurveySubmitOpcode;
    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_REPORT].handler = &WorldSession::handleReportOpcode;
    //WorldPacketHandlers[CMSG_REPORT_PLAYER].handler = &WorldSession::handleReportPlayerOpcode;

    // Meeting Stone / Instances
    //WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler = &WorldSession::handleSummonResponseOpcode;
    //WorldPacketHandlers[CMSG_RESET_INSTANCES].handler = &WorldSession::handleResetInstanceOpcode;
    //WorldPacketHandlers[CMSG_SELF_RES].handler = &WorldSession::HandleSelfResurrectOpcode;
    //WorldPacketHandlers[MSG_RANDOM_ROLL].handler = &WorldSession::HandleRandomRollOpcode;
    //WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler = &WorldSession::handleDungeonDifficultyOpcode;
    //WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler = &WorldSession::handleRaidDifficultyOpcode;

    // Misc
    //WorldPacketHandlers[CMSG_OPEN_ITEM].handler = &WorldSession::handleOpenItemOpcode;
    //WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler = &WorldSession::handleCompleteCinematic;
    //WorldPacketHandlers[CMSG_NEXT_CINEMATIC_CAMERA].handler = &WorldSession::handleNextCinematic;
    //WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler = &WorldSession::handleMountSpecialAnimOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler = &WorldSession::HandleToggleCloakOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_HELM].handler = &WorldSession::HandleToggleHelmOpcode;
    //WorldPacketHandlers[CMSG_SET_TITLE].handler = &WorldSession::HandleSetVisibleRankOpcode;
    //WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::handleGameobjReportUseOpCode;
    //WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler = &WorldSession::handlePetCastSpell;
    //WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::handleWorldStateUITimerUpdate;
    //WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler = &WorldSession::handleSetTaxiBenchmarkOpcode;
    //WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler = &WorldSession::handleUnlearnSkillOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_CEMETERY_LIST].handler = &WorldSession::handleRequestCemeteryListOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_HOTFIX].handler = &WorldSession::handleRequestHotfix;
    //WorldPacketHandlers[CMSG_RETURN_TO_GRAVEYARD].handler = &WorldSession::handleReturnToGraveyardOpcode;
    //WorldPacketHandlers[CMSG_BUG].handler = &WorldSession::handleBugOpcode;
    //WorldPacketHandlers[CMSG_SUGGESTION_SUBMIT].handler = &WorldSession::handleSuggestionOpcode;
    //WorldPacketHandlers[CMSG_LOG_DISCONNECT].handler = &WorldSession::handleLogDisconnectOpcode;

    // Chat
    //WorldPacketHandlers[CMSG_CHAT_IGNORED].handler = &WorldSession::handleChatIgnoredOpcode;
    //WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler = &WorldSession::handleChatChannelWatchOpcode;

    // Arenas
    //WorldPacketHandlers[CMSG_ARENA_TEAM_QUERY].handler = &WorldSession::handleArenaTeamQueryOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_ROSTER].handler = &WorldSession::handleArenaTeamRosterOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_INVITE].handler = &WorldSession::handleArenaTeamAddMemberOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_ACCEPT].handler = &WorldSession::handleArenaTeamInviteAcceptOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_DECLINE].handler = &WorldSession::handleArenaTeamInviteDenyOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_LEAVE].handler = &WorldSession::handleArenaTeamLeaveOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_REMOVE].handler = &WorldSession::handleArenaTeamRemoveMemberOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_DISBAND].handler = &WorldSession::handleArenaTeamDisbandOpcode;
    //WorldPacketHandlers[CMSG_ARENA_TEAM_LEADER].handler = &WorldSession::handleArenaTeamPromoteOpcode;
    //WorldPacketHandlers[MSG_INSPECT_ARENA_TEAMS].handler = &WorldSession::handleInspectArenaStatsOpcode;

    // cheat/gm commands?
    //WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::handleWorldTeleportOpcode;

    // Vehicle
    //WorldPacketHandlers[CMSG_DISMISS_CONTROLLED_VEHICLE].handler = &WorldSession::HandleDismissVehicle;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_EXIT].handler = &WorldSession::HandleLeaveVehicle;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_PREV_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_NEXT_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_SWITCH_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_PLAYER_VEHICLE_ENTER].handler = &WorldSession::HandleEnterVehicle;
    //WorldPacketHandlers[CMSG_EJECT_PASSENGER].handler = &WorldSession::HandleRemoveVehiclePassenger;

    // Unsorted
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESPONSE].handler = &WorldSession::HandleTimeSyncResOp; // MiscHandler.cpp
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::HandleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::HandleSetAutoLootPassOpcode;
    //WorldPacketHandlers[CMSG_QUEST_GIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
    //WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::handleRemoveGlyph;
    //WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::handleBarberShopResult;
    //WorldPacketHandlers[CMSG_GET_MIRRORIMAGE_DATA].handler = &WorldSession::HandleMirrorImageOpcode;

    // Calendar - Unhandled
    //WorldPacketHandlers[CMSG_CALENDAR_GET_CALENDAR].handler = &WorldSession::handleCalendarGetCalendar;
    //WorldPacketHandlers[CMSG_CALENDAR_COMPLAIN].handler = &WorldSession::handleCalendarComplain;
    //WorldPacketHandlers[CMSG_CALENDAR_GET_NUM_PENDING].handler = &WorldSession::handleCalendarGetNumPending;
    //WorldPacketHandlers[CMSG_CALENDAR_ADD_EVENT].handler = &WorldSession::handleCalendarAddEvent;

    //WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].handler = &WorldSession::handleCalendarGetEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].handler = &WorldSession::handleCalendarGuildFilter;
    //WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].handler = &WorldSession::handleCalendarArenaTeam;
    //WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].handler = &WorldSession::handleCalendarUpdateEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].handler = &WorldSession::handleCalendarRemoveEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].handler = &WorldSession::handleCalendarCopyEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].handler = &WorldSession::handleCalendarEventInvite;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].handler = &WorldSession::handleCalendarEventRsvp;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].handler = &WorldSession::handleCalendarEventRemoveInvite;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].handler = &WorldSession::handleCalendarEventStatus;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].handler = &WorldSession::handleCalendarEventModeratorStatus;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].status = STATUS_LOGGEDIN;

    //Misc - Unhandled
    //WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;

    WorldPacketHandlers[CMSG_ADDON_REGISTERED_PREFIXES].handler = &WorldSession::handleAddonRegisteredPrefixesOpcode;
    WorldPacketHandlers[CMSG_UNREGISTER_ALL_ADDON_PREFIXES].handler = &WorldSession::handleUnregisterAddonPrefixesOpcode;
#endif
}
