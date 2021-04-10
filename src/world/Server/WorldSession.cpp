/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

#include "StdAfx.h"
#include "FastQueue.h"
#include "Threading/Mutex.h"
#include "WorldPacket.h"
#include "Management/Item.h"
#include "Exceptions/PlayerExceptions.hpp"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Spell/Definitions/PowerType.h"
#include "CharacterErrors.h"
#include "WorldSocket.h"
#include "Auth/MD5.h"
#include "Packets/SmsgNotification.h"
#include "Packets/SmsgLogoutComplete.h"
#include "OpcodeTable.hpp"

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
    has_level_55_char(false),
    has_dk(false),
    _side(-1),
    m_MoverGuid(0),
    _logoutTime(0),
    permissions(nullptr),
    permissioncount(0),
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

    memset(movement_packet, 0, sizeof(movement_packet));

    for (uint8 x = 0; x < 8; x++)
        sAccountData[x].data = nullptr;
}

WorldSession::~WorldSession()
{
    deleteMutex.Acquire();

    if (_player)
    {
        LOG_ERROR("warning: logged out player in worldsession destructor");
        LogoutPlayer(true);
    }

    delete[]permissions;

    WorldPacket* packet;

    while ((packet = _recvQueue.Pop()) != nullptr)
        delete packet;

    for (uint32 x = 0; x < 8; x++)
    {
        delete[]sAccountData[x].data;
    }

    if (_socket)
        _socket->SetSession(nullptr);

    if (m_loggingInPlayer)
        m_loggingInPlayer->SetSession(nullptr);

    deleteMutex.Release();
}

uint8 WorldSession::Update(uint32 InstanceID)
{
    m_currMsTime = Util::getMSTime();

    if (!((++_updatecount) % 2) && _socket)
        _socket->UpdateQueuedPackets();

    WorldPacket* packet;

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
           if (_player && _player->DuelingWith)
           _player->EndDuel(DUEL_WINNER_RETREAT);

           bDeleted = true; LogoutPlayer(true); // 1 - Delete session
           completely. return 1; */

    }

    while ((packet = _recvQueue.Pop()) != nullptr)
    {
        ARCEMU_ASSERT(packet != NULL);

        if (sOpcodeTables.getInternalIdForHex(packet->GetOpcode()) >= NUM_OPCODES)
        {
            LogDebugFlag(LF_OPCODE, "[Session] Received out of range packet with opcode 0x%.4X", packet->GetOpcode());
        }
        else
        {
            OpcodeHandler* handler = &WorldPacketHandlers[sOpcodeTables.getInternalIdForHex(packet->GetOpcode())];
            if (handler->status == STATUS_LOGGEDIN && !_player && handler->handler != 0)
            {
                LogDebugFlag(LF_OPCODE, "[Session] Received unexpected/wrong state packet with opcode %s (0x%.4X)", 
                    sOpcodeTables.getNameForOpcode(packet->GetOpcode()).c_str(), packet->GetOpcode());
            }
            else
            {
                // Valid Packet :>
                if (handler->handler == 0)
                {
                    LogDebugFlag(LF_OPCODE, "[Session] Received unhandled packet with opcode %s (0x%.4X)",
                        sOpcodeTables.getNameForOpcode(packet->GetOpcode()).c_str(), packet->GetOpcode());
                }
                else
                {
                    (this->*handler->handler)(*packet);
                }
            }
        }

        delete packet;

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
        _player->SetFaction(_player->GetInitialFactionId());

        sObjectMgr.RemovePlayer(_player);
        _player->ok_to_remove = true;

        sHookInterface.OnLogout(pPlayer);
        if (_player->DuelingWith)
            _player->DuelingWith->EndDuel(DUEL_WINNER_RETREAT);

        if (_player->m_currentLoot && _player->IsInWorld())
        {
            Object* obj = _player->GetMapMgr()->_GetObject(_player->m_currentLoot);
            if (obj != nullptr)
            {
                switch (obj->getObjectTypeId())
                {
                    case TYPEID_UNIT:
                    {
                        if (const auto creature = dynamic_cast<Creature*>(obj))
                            creature->loot.looters.erase(_player->getGuidLow());
                    } break;
                    case TYPEID_GAMEOBJECT:
                    {
                        if (const auto go = dynamic_cast<GameObject*>(obj))
                        {
                            if (!go->IsLootable())
                                break;

                            if (const auto pLGO = dynamic_cast<GameObject_Lootable*>(go))
                                pLGO->loot.looters.erase(_player->getGuidLow());
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
        _player->CleanupChannels();

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

        if (_player->m_bgIsQueued)
            sBattlegroundManager.RemovePlayerFromQueues(_player);

        // Repop or Resurrect and remove from battlegrounds
        if (_player->m_bg)
        {
            if (pPlayer->getDeathState() == JUST_DIED)
                pPlayer->RemoteRevive();
            if (pPlayer->getDeathState() != ALIVE)
                pPlayer->ResurrectPlayer();
            _player->m_bg->RemovePlayer(_player, true);
        }
        else if (_player->isDead() && _player->getDeathState() == JUST_DIED)
            _player->RepopRequestedPlayer();

        // Issue a message telling all guild members that this player signed
        // off

        _player->getItemInterface()->EmptyBuyBack();
        _player->getItemInterface()->removeLootableItems();

        // Save HP/Mana
        _player->load_health = _player->getHealth();
        _player->load_mana = _player->getPower(POWER_TYPE_MANA);


        _player->getSummonInterface()->removeAllSummons();

        _player->DismissActivePets();

        // _player->SaveAuras();

        if (Save)
            _player->SaveToDB(false);

        // Dismounting with RemoveAllAuras may in certain cases add a player
        // aura,
        // which can result in a nice crash during shutdown. Therefore let's
        // dismount on logout.
        // Ofc if the player was mounted on login they will be still mounted
        // ;)
        _player->Dismount();

        _player->RemoveAllAuras();
        if (_player->IsInWorld())
            _player->RemoveFromWorld();

        _player->m_playerInfo->m_loggedInPlayer = nullptr;

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
        LOG_DEBUG("SESSION: Sent SMSG_LOGOUT_COMPLETE Message");
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
    std::list < char >tmp;
    bool hasa = false;
    for (uint32 i = 0; i < securitystring.length(); ++i)
    {
        char c = securitystring.at(i);
        c = static_cast<char>(tolower(c));
        if (c == '4' || c == '3')
            c = 'a'; // for the lazy people

        if (c == 'a')
        {
            // all permissions
            tmp.push_back('a');
            hasa = true;
        }
        else if (!hasa && (c == '0') && i == 0)
            break;
        else if (!hasa || (hasa && (c == 'z')))
        {
            tmp.push_back(c);
        }
    }

    permissions = new char[tmp.size() + 1];
    memset(permissions, 0, tmp.size() + 1);
    permissioncount = static_cast<uint32>(tmp.size());
    int k = 0;

    for (std::list <char>::iterator itr = tmp.begin(); itr != tmp.end(); ++itr)
        permissions[k++] = (*itr);

    if (permissions[tmp.size()] != 0)
        permissions[tmp.size()] = 0;

    LOG_DEBUG("Loaded permissions for %u. (%u) : [%s]", this->GetAccountId(), permissioncount, securitystring.c_str());
}

void WorldSession::SetSecurity(std::string securitystring)
{
    delete[]permissions;
    LoadSecurity(securitystring);
}

bool WorldSession::CanUseCommand(char cmdstr)
{
    if (permissioncount == 0)
        return false;
    if (cmdstr == 0)
        return true;
    if (permissions[0] == 'a' && cmdstr != 'z') // all
        return true;

    for (int i = 0; i < permissioncount; ++i)
        if (permissions[i] == cmdstr)
            return true;

    return false;
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
    for (uint32 i = 0; i < NUM_OPCODES; ++i)
    {
        WorldPacketHandlers[i].status = STATUS_LOGGEDIN;
        WorldPacketHandlers[i].handler = nullptr;
    }
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
            static_cast<unsigned int>(session->GetAccountId()),
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

void WorldSession::SystemMessage(const char* format, ...)
{
    char buffer[1024];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, 1024, format, ap);
    va_end(ap);

    WorldPacket* data = sChatHandler.FillSystemMessageData(buffer);
    SendPacket(data);
    delete data;
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
        LogDebugFlag(LF_OPCODE, "Opcode %s [%s] (0x%.4X) received. Apply nothingToHandle handler but size is %lu!",
            sOpcodeTables.getNameForOpcode(recv_data.GetOpcode()).c_str(), sOpcodeTables.getNameForAEVersion().c_str(), recv_data.GetOpcode(), recv_data.size());
    }
}

void WorldSession::SendPacket(WorldPacket* packet)
{
    if (packet->GetOpcode() == 0x0000)
    {
        LOG_ERROR("Return, packet 0x0000 is not a valid packet!");
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

void WorldSession::QueuePacket(WorldPacket* packet)
{
    m_lastPing = static_cast<uint32>(UNIXTIME);
    _recvQueue.Push(packet);
}

void WorldSession::Disconnect()
{
    if (_socket && _socket->IsConnected())
    {
        _socket->Disconnect();
    }
}

//MIT
#if VERSION_STRING == Classic
void WorldSession::loadHandlers()
{
    // Login
    WorldPacketHandlers[CMSG_CHAR_ENUM].handler = &WorldSession::handleCharEnumOpcode;
    WorldPacketHandlers[CMSG_CHAR_ENUM].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CREATE].handler = &WorldSession::handleCharCreateOpcode;
    WorldPacketHandlers[CMSG_CHAR_CREATE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_DELETE].handler = &WorldSession::handleCharDeleteOpcode;
    WorldPacketHandlers[CMSG_CHAR_DELETE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_RENAME].handler = &WorldSession::handleCharRenameOpcode;
    WorldPacketHandlers[CMSG_CHAR_RENAME].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].handler = &WorldSession::HandleCharCustomizeLooksOpcode;
    //WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].status = STATUS_AUTHED;

    // declined names (Cyrillic client)
    //WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].handler = &WorldSession::handleDeclinedPlayerNameOpcode;
    //WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler = &WorldSession::handlePlayerLoginOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGIN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_REALM_SPLIT].handler = &WorldSession::handleRealmSplitOpcode;
    WorldPacketHandlers[CMSG_REALM_SPLIT].status = STATUS_AUTHED;

    // Queries
    //WorldPacketHandlers[MSG_CORPSE_QUERY].handler = &WorldSession::HandleCorpseQueryOpcode;
    WorldPacketHandlers[CMSG_NAME_QUERY].handler = &WorldSession::handleNameQueryOpcode;
    //WorldPacketHandlers[CMSG_QUERY_TIME].handler = &WorldSession::HandleQueryTimeOpcode;
    //WorldPacketHandlers[CMSG_CREATURE_QUERY].handler = &WorldSession::HandleCreatureQueryOpcode;
    //WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler = &WorldSession::HandleGameObjectQueryOpcode;
    //WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler = &WorldSession::HandlePageTextQueryOpcode;
    //WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler = &WorldSession::HandleItemNameQueryOpcode;
    //WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler = &WorldSession::HandleAchievmentQueryOpcode;

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
    WorldPacketHandlers[CMSG_MOVE_SET_FLY].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_NOT_ACTIVE_MOVER].handler = &WorldSession::handleMoveNotActiveMoverOpcode;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_MOVER].handler = &WorldSession::HandleSetActiveMoverOpcode;
    WorldPacketHandlers[CMSG_MOVE_CHNG_TRANSPORT].handler = &WorldSession::handleMovementOpcodes;

    // ACK
    WorldPacketHandlers[MSG_MOVE_TELEPORT_ACK].handler = &WorldSession::handleMoveTeleportAckOpcode;
    WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;

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
    WorldPacketHandlers[CMSG_PLAYER_LOGOUT].handler = &WorldSession::handlePlayerLogoutOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_CANCEL].handler = &WorldSession::handleLogoutCancelOpcode;
    // WorldPacketHandlers[CMSG_LOGOUT_CANCEL].status = STATUS_LOGGEDIN_RECENTLY_LOGGOUT;

    WorldPacketHandlers[CMSG_ZONEUPDATE].handler = &WorldSession::handleZoneupdate;
    // WorldPacketHandlers[CMSG_SET_TARGET_OBSOLETE].handler = &WorldSession::HandleSetTargetOpcode;
    WorldPacketHandlers[CMSG_SET_SELECTION].handler = &WorldSession::handleSetSelectionOpcode;
    WorldPacketHandlers[CMSG_STANDSTATECHANGE].handler = &WorldSession::handleStandStateChangeOpcode;
    WorldPacketHandlers[CMSG_CANCEL_MOUNT_AURA].handler = &WorldSession::handleDismountOpcode;

    // Friends
    WorldPacketHandlers[CMSG_CONTACT_LIST].handler = &WorldSession::handleFriendListOpcode;
    WorldPacketHandlers[CMSG_ADD_FRIEND].handler = &WorldSession::handleAddFriendOpcode;
    WorldPacketHandlers[CMSG_DEL_FRIEND].handler = &WorldSession::handleDelFriendOpcode;
    WorldPacketHandlers[CMSG_ADD_IGNORE].handler = &WorldSession::handleAddIgnoreOpcode;
    WorldPacketHandlers[CMSG_DEL_IGNORE].handler = &WorldSession::handleDelIgnoreOpcode;
    WorldPacketHandlers[CMSG_BUG].handler = &WorldSession::handleBugOpcode;
    // WorldPacketHandlers[CMSG_SET_FRIEND_NOTE].handler = &WorldSession::HandleSetFriendNote;

    // Areatrigger
    WorldPacketHandlers[CMSG_AREATRIGGER].handler = &WorldSession::handleAreaTriggerOpcode;

    // Account Data
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].handler = &WorldSession::handleUpdateAccountData;
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].status = STATUS_AUTHED;
    WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].handler = &WorldSession::handleRequestAccountData;
    WorldPacketHandlers[CMSG_TOGGLE_PVP].handler = &WorldSession::handleTogglePVPOpcode;

    // Faction / Reputation
    WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler = &WorldSession::handleSetFactionAtWarOpcode;
    WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler = &WorldSession::handleSetWatchedFactionIndexOpcode;
    WorldPacketHandlers[CMSG_SET_FACTION_INACTIVE].handler = &WorldSession::handleSetFactionInactiveOpcode;

    // Player Interaction
    WorldPacketHandlers[CMSG_GAMEOBJ_USE].handler = &WorldSession::handleGameObjectUse;
    WorldPacketHandlers[CMSG_PLAYED_TIME].handler = &WorldSession::handlePlayedTimeOpcode;
    WorldPacketHandlers[CMSG_SETSHEATHED].handler = &WorldSession::handleSetSheathedOpcode;
    //WorldPacketHandlers[CMSG_MESSAGECHAT].handler = &WorldSession::HandleMessagechatOpcode;
    WorldPacketHandlers[CMSG_EMOTE].handler = &WorldSession::handleEmoteOpcode;
    WorldPacketHandlers[CMSG_TEXT_EMOTE].handler = &WorldSession::handleTextEmoteOpcode;
    WorldPacketHandlers[CMSG_INSPECT].handler = &WorldSession::handleInspectOpcode;
    //WorldPacketHandlers[SMSG_BARBER_SHOP_RESULT].handler = &WorldSession::handleBarberShopResult;

    // Channels
    //WorldPacketHandlers[CMSG_JOIN_CHANNEL].handler = &WorldSession::handleChannelJoin;
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
    //WorldPacketHandlers[CMSG_GET_CHANNEL_MEMBER_COUNT].handler = &WorldSession::handleChannelNumMembersQuery;
    WorldPacketHandlers[CMSG_CHANNEL_DISPLAY_LIST].handler = &WorldSession::handleChannelRosterQuery;

    // Groups / Raids
    WorldPacketHandlers[CMSG_GROUP_INVITE].handler = &WorldSession::handleGroupInviteOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CANCEL].handler = &WorldSession::handleGroupCancelOpcode;
    WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler = &WorldSession::handleGroupAcceptOpcode;
    WorldPacketHandlers[CMSG_GROUP_DECLINE].handler = &WorldSession::handleGroupDeclineOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler = &WorldSession::handleGroupUninviteOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler = &WorldSession::handleGroupUninviteGuidOpcode;
    WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler = &WorldSession::handleGroupSetLeaderOpcode;
    WorldPacketHandlers[CMSG_GROUP_DISBAND].handler = &WorldSession::handleGroupDisbandOpcode;
    WorldPacketHandlers[CMSG_LOOT_METHOD].handler = &WorldSession::handleLootMethodOpcode;
    WorldPacketHandlers[MSG_MINIMAP_PING].handler = &WorldSession::handleMinimapPingOpcode;
    WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler = &WorldSession::handleConvertGroupToRaidOpcode;
    WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler = &WorldSession:: handleGroupChangeSubGroup;
    WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler = &WorldSession::handleGroupAssistantLeader;
    WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler = &WorldSession::handleRequestRaidInfoOpcode;
    WorldPacketHandlers[MSG_RAID_READY_CHECK].handler = &WorldSession::handleReadyCheckOpcode;
    WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler = &WorldSession::handleSetPlayerIconOpcode;
    WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler = &WorldSession::handlePartyMemberStatsOpcode;
    WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler = &WorldSession::handleGroupPromote;

    // LFG System
    WorldPacketHandlers[CMSG_SET_LFG_COMMENT].handler = &WorldSession::handleLfgSetCommentOpcode;
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
    WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler = &WorldSession::handleTaxiNodeStatusQueryOpcode;
    WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler = &WorldSession::handleTaxiQueryAvaibleNodesOpcode;
    WorldPacketHandlers[CMSG_ACTIVATETAXI].handler = &WorldSession::handleActivateTaxiOpcode;
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
    WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;
    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
    WorldPacketHandlers[CMSG_SPLIT_ITEM].handler = &WorldSession::handleSplitOpcode;
    WorldPacketHandlers[CMSG_READ_ITEM].handler = &WorldSession::handleReadItemOpcode;
    WorldPacketHandlers[CMSG_REPAIR_ITEM].handler = &WorldSession::handleRepairItemOpcode;
    WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler = &WorldSession::handleAutoBankItemOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler = &WorldSession::handleAutoStoreBankItemOpcode;
    WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler = &WorldSession::handleCancelTemporaryEnchantmentOpcode;
    WorldPacketHandlers[CMSG_SOCKET_GEMS].handler = &WorldSession::handleInsertGemOpcode;
    WorldPacketHandlers[CMSG_WRAP_ITEM].handler = &WorldSession::handleWrapItemOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler = &WorldSession::handleItemRefundInfoOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler = &WorldSession::HandleItemRefundRequestOpcode;

    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler = &WorldSession::handleEquipmentSetSave;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler = &WorldSession::handleEquipmentSetUse;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler = &WorldSession::handleEquipmentSetDelete;

    // Spell System / Talent System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    WorldPacketHandlers[CMSG_SPELLCLICK].handler = &WorldSession::handleSpellClick;
    WorldPacketHandlers[CMSG_CANCEL_CAST].handler = &WorldSession::handleCancelCastOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AURA].handler = &WorldSession::handleCancelAuraOpcode;
    WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler = &WorldSession::handleCancelChannellingOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler = &WorldSession::handleCancelAutoRepeatSpellOpcode;
    WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler = &WorldSession::handleCancelTotem;
    WorldPacketHandlers[CMSG_LEARN_TALENT].handler = &WorldSession::handleLearnTalentOpcode;
    //WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler = &WorldSession::handleLearnMultipleTalentsOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler = &WorldSession::handleUnlearnTalents;
    WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler = &WorldSession::handleUnlearnTalents;
    //WorldPacketHandlers[CMSG_UPDATE_PROJECTILE_POSITION].handler = &WorldSession::handleUpdateProjectilePosition;

    // Combat / Duel
    WorldPacketHandlers[CMSG_ATTACKSWING].handler = &WorldSession::handleAttackSwingOpcode;
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler = &WorldSession::handleAttackStopOpcode;
    WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler = &WorldSession::handleDuelAccepted;
    WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler = &WorldSession::handleDuelCancelled;

    // Trade
    WorldPacketHandlers[CMSG_INITIATE_TRADE].handler = &WorldSession::handleInitiateTradeOpcode;
    WorldPacketHandlers[CMSG_BEGIN_TRADE].handler = &WorldSession::handleBeginTradeOpcode;
    WorldPacketHandlers[CMSG_BUSY_TRADE].handler = &WorldSession::handleBusyTrade;
    WorldPacketHandlers[CMSG_IGNORE_TRADE].handler = &WorldSession::handleIgnoreTrade;
    WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler = &WorldSession::handleAcceptTrade;
    WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler = &WorldSession::handleUnacceptTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].handler = &WorldSession::handleCancelTrade;
    WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler = &WorldSession::handleSetTradeItem;
    WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler = &WorldSession::handleClearTradeItem;
    WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler = &WorldSession::handleSetTradeGold;

    // Quest System
    //WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
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
    WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler = &WorldSession::handleResurrectResponse;
    WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler = &WorldSession::handlePushQuestToPartyOpcode;
    WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler = &WorldSession::handleQuestPushResultOpcode;
    //WorldPacketHandlers[CMSG_QUEST_POI_QUERY].handler = &WorldSession::handleQuestPOIQueryOpcode;

    // Auction System
    WorldPacketHandlers[CMSG_AUCTION_LIST_ITEMS].handler = &WorldSession::handleAuctionListItems;
    WorldPacketHandlers[CMSG_AUCTION_LIST_BIDDER_ITEMS].handler = &WorldSession::handleAuctionListBidderItems;
    WorldPacketHandlers[CMSG_AUCTION_SELL_ITEM].handler = &WorldSession::handleAuctionSellItem;
    WorldPacketHandlers[CMSG_AUCTION_LIST_OWNER_ITEMS].handler = &WorldSession::handleAuctionListOwnerItems;
    WorldPacketHandlers[CMSG_AUCTION_PLACE_BID].handler = &WorldSession::handleAuctionPlaceBid;
    WorldPacketHandlers[CMSG_AUCTION_REMOVE_ITEM].handler = &WorldSession::handleCancelAuction;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_PENDING_SALES].handler = &WorldSession::HandleAuctionListPendingSales;

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

    // Guild Query (called when not logged in sometimes)
    WorldPacketHandlers[CMSG_GUILD_QUERY].handler = &WorldSession::handleGuildQuery;
    WorldPacketHandlers[CMSG_GUILD_QUERY].status = STATUS_AUTHED;

    // Guild System
    //WorldPacketHandlers[CMSG_GUILD_CREATE].handler = &WorldSession::HandleCreateGuild;
    WorldPacketHandlers[CMSG_GUILD_INVITE].handler = &WorldSession::handleInviteToGuild;
    WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler = &WorldSession::handleGuildAccept;
    WorldPacketHandlers[CMSG_GUILD_DECLINE].handler = &WorldSession::handleGuildDecline;
    WorldPacketHandlers[CMSG_GUILD_INFO].handler = &WorldSession::handleGuildInfo;
    WorldPacketHandlers[CMSG_GUILD_ROSTER].handler = &WorldSession::handleGuildRoster;
    WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler = &WorldSession::handleGuildPromote;
    WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler = &WorldSession::handleGuildDemote;
    WorldPacketHandlers[CMSG_GUILD_LEAVE].handler = &WorldSession::handleGuildLeave;
    WorldPacketHandlers[CMSG_GUILD_REMOVE].handler = &WorldSession::handleGuildRemove;
    WorldPacketHandlers[CMSG_GUILD_DISBAND].handler = &WorldSession::handleGuildDisband;
    WorldPacketHandlers[CMSG_GUILD_LEADER].handler = &WorldSession::handleGuildLeader;
    WorldPacketHandlers[CMSG_GUILD_MOTD].handler = &WorldSession::handleGuildMotd;
    WorldPacketHandlers[CMSG_GUILD_SET_RANK].handler = &WorldSession::handleGuildSetRank;
    WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler = &WorldSession::handleGuildAddRank;
    WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler = &WorldSession::handleGuildDelRank;
    WorldPacketHandlers[CMSG_GUILD_SET_PUBLIC_NOTE].handler = &WorldSession::handleGuildSetPublicNote;
    WorldPacketHandlers[CMSG_GUILD_SET_OFFICER_NOTE].handler = &WorldSession::handleGuildSetOfficerNote;
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
    WorldPacketHandlers[MSG_QUERY_GUILD_BANK_TEXT].handler = &WorldSession::handleGuildBankQueryText;
    WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler = &WorldSession::handleSetGuildBankText;
    WorldPacketHandlers[MSG_GUILD_EVENT_LOG_QUERY].handler = &WorldSession::handleGuildLog;
    WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler = &WorldSession::handleGuildBankerActivate;
    WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler = &WorldSession::handleGuildBankBuyTab;
    WorldPacketHandlers[MSG_GUILD_BANK_MONEY_WITHDRAWN].handler = &WorldSession::handleGuildBankMoneyWithdrawn;
    WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler = &WorldSession::handleGuildBankUpdateTab;
    WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler = &WorldSession::handleGuildBankSwapItems;
    WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler = &WorldSession::handleGuildBankWithdrawMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler = &WorldSession::handleGuildBankDepositMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler = &WorldSession::handleGuildBankQueryTab;
    WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler = &WorldSession::handleGuildBankLogQuery;
    WorldPacketHandlers[MSG_GUILD_PERMISSIONS].handler = &WorldSession::handleGuildPermissions;

    // Tutorials
    WorldPacketHandlers[CMSG_TUTORIAL_FLAG].handler = &WorldSession::handleTutorialFlag;
    WorldPacketHandlers[CMSG_TUTORIAL_CLEAR].handler = &WorldSession::handleTutorialClear;
    WorldPacketHandlers[CMSG_TUTORIAL_RESET].handler = &WorldSession::handleTutorialReset;

    // Pets
    WorldPacketHandlers[CMSG_PET_ACTION].handler = &WorldSession::handlePetAction;
    //WorldPacketHandlers[CMSG_REQUEST_PET_INFO].handler = &WorldSession::handlePetInfo;
    WorldPacketHandlers[CMSG_PET_NAME_QUERY].handler = &WorldSession::handlePetNameQuery;
    WorldPacketHandlers[CMSG_BUY_STABLE_SLOT].handler = &WorldSession::handleBuyStableSlot;
    WorldPacketHandlers[CMSG_STABLE_PET].handler = &WorldSession::handleStablePet;
    WorldPacketHandlers[CMSG_UNSTABLE_PET].handler = &WorldSession::handleUnstablePet;
    WorldPacketHandlers[CMSG_STABLE_SWAP_PET].handler = &WorldSession::handleStableSwapPet;
    WorldPacketHandlers[MSG_LIST_STABLED_PETS].handler = &WorldSession::handleStabledPetList;
    WorldPacketHandlers[CMSG_PET_SET_ACTION].handler = &WorldSession::handlePetSetActionOpcode;
    WorldPacketHandlers[CMSG_PET_RENAME].handler = &WorldSession::handlePetRename;
    WorldPacketHandlers[CMSG_PET_ABANDON].handler = &WorldSession::handlePetAbandon;
    WorldPacketHandlers[CMSG_PET_UNLEARN].handler = &WorldSession::handlePetUnlearn;
    WorldPacketHandlers[CMSG_PET_SPELL_AUTOCAST].handler = &WorldSession::handlePetSpellAutocast;
    WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler = &WorldSession::handlePetCancelAura;
    //WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler = &WorldSession::HandlePetLearnTalent;
    //WorldPacketHandlers[CMSG_DISMISS_CRITTER].handler = &WorldSession::HandleDismissCritter;

    // Battlegrounds
    WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler = &WorldSession::handleBattlefieldPortOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler = &WorldSession::handleBattlefieldStatusOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler = &WorldSession::handleBattlefieldListOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler = &WorldSession::handleBattleMasterHelloOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler = &WorldSession::handleArenaJoinOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler = &WorldSession::handleBattleMasterJoinOpcode;
    WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler = &WorldSession::handleLeaveBattlefieldOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler = &WorldSession::handleAreaSpiritHealerQueryOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler = &WorldSession::handleAreaSpiritHealerQueueOpcode;
    WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler = &WorldSession::handleBattlegroundPlayerPositionsOpcode;
    WorldPacketHandlers[MSG_PVP_LOG_DATA].handler = &WorldSession::handlePVPLogDataOpcode;
    WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler = &WorldSession::handleInspectHonorStatsOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler = &WorldSession::handleSetActionBarTogglesOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE].handler = &WorldSession::HandleBgInviteResponse;

    // GM Ticket System
    /*WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler = &WorldSession::handleGMTicketCreateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler = &WorldSession::handleGMTicketUpdateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler = &WorldSession::handleGMTicketDeleteOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketGetTicketOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler = &WorldSession::handleGMTicketSystemStatusOpcode;
    WorldPacketHandlers[CMSG_GMTICKETSYSTEM_TOGGLE].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;*/

    // Lag report
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].handler = &WorldSession::handleReportLag;
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].status = STATUS_LOGGEDIN;
    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].handler = &WorldSession::handleGMSurveySubmitOpcode;
    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].status = STATUS_LOGGEDIN;

    // Meeting Stone / Instances
    WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler = &WorldSession::handleSummonResponseOpcode;
    WorldPacketHandlers[CMSG_RESET_INSTANCES].handler = &WorldSession::handleResetInstanceOpcode;
    WorldPacketHandlers[CMSG_SELF_RES].handler = &WorldSession::handleSelfResurrect;
    WorldPacketHandlers[MSG_RANDOM_ROLL].handler = &WorldSession::handleRandomRollOpcode;
    WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler = &WorldSession::handleDungeonDifficultyOpcode;
    //WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler = &WorldSession::HandleRaidDifficultyOpcode;

    // Misc
    WorldPacketHandlers[CMSG_OPEN_ITEM].handler = &WorldSession::handleOpenItemOpcode;
    WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler = &WorldSession::handleCompleteCinematic;
    WorldPacketHandlers[CMSG_NEXT_CINEMATIC_CAMERA].handler = &WorldSession::handleNextCinematic;
    WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler = &WorldSession::handleMountSpecialAnimOpcode;
    WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler = &WorldSession::handleToggleCloakOpcode;
    WorldPacketHandlers[CMSG_TOGGLE_HELM].handler = &WorldSession::handleToggleHelmOpcode;
    WorldPacketHandlers[CMSG_SET_TITLE].handler = &WorldSession::handleSetTitle;
    WorldPacketHandlers[CMSG_COMPLAIN].handler = &WorldSession::handleReportSpamOpcode;
    //WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::HandleGameobjReportUseOpCode;
    WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler = &WorldSession::handlePetCastSpell;
    //WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::HandleWorldStateUITimerUpdate;
    WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler = &WorldSession::handleSetTaxiBenchmarkOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler = &WorldSession::handleUnlearnSkillOpcode;

    // Chat
    WorldPacketHandlers[CMSG_CHAT_IGNORED].handler = &WorldSession::handleChatIgnoredOpcode;
    WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler = &WorldSession::handleChatChannelWatchOpcode;

    // Arenas
    WorldPacketHandlers[CMSG_ARENA_TEAM_QUERY].handler = &WorldSession::handleArenaTeamQueryOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_ROSTER].handler = &WorldSession::handleArenaTeamRosterOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_INVITE].handler = &WorldSession::handleArenaTeamAddMemberOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_ACCEPT].handler = &WorldSession::handleArenaTeamInviteAcceptOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_DECLINE].handler = &WorldSession::handleArenaTeamInviteDenyOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_LEAVE].handler = &WorldSession::handleArenaTeamLeaveOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_REMOVE].handler = &WorldSession::handleArenaTeamRemoveMemberOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_DISBAND].handler = &WorldSession::handleArenaTeamDisbandOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_LEADER].handler = &WorldSession::handleArenaTeamPromoteOpcode;
    WorldPacketHandlers[MSG_INSPECT_ARENA_TEAMS].handler = &WorldSession::handleInspectArenaStatsOpcode;

    // cheat/gm commands?
    WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::handleWorldTeleportOpcode;

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
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::HandleTimeSyncResp;       //MiscHandler.cpp
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::handleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::handleSetAutoLootPassOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
    //WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::handleRemoveGlyph;
    //WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::handleBarberShopResult;
    WorldPacketHandlers[CMSG_GET_MIRRORIMAGE_DATA].handler = &WorldSession::HandleMirrorImageOpcode;

    // Calendar - Unhandled
    //WorldPacketHandlers[CMSG_CALENDAR_GET_CALENDAR].handler = &WorldSession::HandleCalendarGetCalendar;
    //WorldPacketHandlers[CMSG_CALENDAR_COMPLAIN].handler = &WorldSession::HandleCalendarComplain;
    //WorldPacketHandlers[CMSG_CALENDAR_GET_NUM_PENDING].handler = &WorldSession::HandleCalendarGetNumPending;
    //WorldPacketHandlers[CMSG_CALENDAR_ADD_EVENT].handler = &WorldSession::HandleCalendarAddEvent;

    //WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].handler = &WorldSession::HandleCalendarGetEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].handler = &WorldSession::HandleCalendarGuildFilter;
    //WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].handler = &WorldSession::HandleCalendarArenaTeam;
    //WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].handler = &WorldSession::HandleCalendarUpdateEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].handler = &WorldSession::HandleCalendarRemoveEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].handler = &WorldSession::HandleCalendarCopyEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].handler = &WorldSession::HandleCalendarEventInvite;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].handler = &WorldSession::HandleCalendarEventRsvp;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].handler = &WorldSession::HandleCalendarEventRemoveInvite;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].handler = &WorldSession::HandleCalendarEventStatus;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].handler = &WorldSession::HandleCalendarEventModeratorStatus;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].status = STATUS_LOGGEDIN;

    //Misc - Unhandled
    //WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;
}
#elif VERSION_STRING == TBC
void WorldSession::loadHandlers()
{
    // Login
    WorldPacketHandlers[CMSG_CHAR_ENUM].handler = &WorldSession::handleCharEnumOpcode;
    WorldPacketHandlers[CMSG_CHAR_ENUM].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CREATE].handler = &WorldSession::handleCharCreateOpcode;
    WorldPacketHandlers[CMSG_CHAR_CREATE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_DELETE].handler = &WorldSession::handleCharDeleteOpcode;
    WorldPacketHandlers[CMSG_CHAR_DELETE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_RENAME].handler = &WorldSession::handleCharRenameOpcode;
    WorldPacketHandlers[CMSG_CHAR_RENAME].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].handler = &WorldSession::HandleCharCustomizeLooksOpcode;
    //WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].status = STATUS_AUTHED;

    // declined names (Cyrillic client)
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].handler = &WorldSession::handleDeclinedPlayerNameOpcode;
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler = &WorldSession::handlePlayerLoginOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGIN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_REALM_SPLIT].handler = &WorldSession::handleRealmSplitOpcode;
    WorldPacketHandlers[CMSG_REALM_SPLIT].status = STATUS_AUTHED;

    // Queries
    WorldPacketHandlers[MSG_CORPSE_QUERY].handler = &WorldSession::handleCorpseQueryOpcode;
    WorldPacketHandlers[CMSG_NAME_QUERY].handler = &WorldSession::handleNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_TIME].handler = &WorldSession::handleQueryTimeOpcode;
    WorldPacketHandlers[CMSG_CREATURE_QUERY].handler = &WorldSession::handleCreatureQueryOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler = &WorldSession::handleGameObjectQueryOpcode;
    //WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler = &WorldSession::HandlePageTextQueryOpcode;
    //WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler = &WorldSession::HandleItemNameQueryOpcode;
    //WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler = &WorldSession::HandleAchievmentQueryOpcode;

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
    WorldPacketHandlers[CMSG_MOVE_SET_FLY].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_NOT_ACTIVE_MOVER].handler = &WorldSession::handleMoveNotActiveMoverOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIVE_MOVER].handler = &WorldSession::handleSetActiveMoverOpcode;
    WorldPacketHandlers[CMSG_MOVE_CHNG_TRANSPORT].handler = &WorldSession::handleMovementOpcodes;

    // ACK
    WorldPacketHandlers[MSG_MOVE_TELEPORT_ACK].handler = &WorldSession::handleMoveTeleportAckOpcode;
    WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler = &WorldSession::handleMovementOpcodes;
    //WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;

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
    //WorldPacketHandlers[CMSG_WHOIS].handler = &WorldSession::handleWhoIsOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_REQUEST].handler = &WorldSession::handleLogoutRequestOpcode;
    //WorldPacketHandlers[CMSG_PLAYER_LOGOUT].handler = &WorldSession::handlePlayerLogoutOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_CANCEL].handler = &WorldSession::handleLogoutCancelOpcode;
    // WorldPacketHandlers[CMSG_LOGOUT_CANCEL].status = STATUS_LOGGEDIN_RECENTLY_LOGGOUT;

    WorldPacketHandlers[CMSG_ZONEUPDATE].handler = &WorldSession::handleZoneupdate;
    // WorldPacketHandlers[CMSG_SET_TARGET_OBSOLETE].handler = &WorldSession::HandleSetTargetOpcode;
    WorldPacketHandlers[CMSG_SET_SELECTION].handler = &WorldSession::handleSetSelectionOpcode;
    WorldPacketHandlers[CMSG_STANDSTATECHANGE].handler = &WorldSession::handleStandStateChangeOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_MOUNT_AURA].handler = &WorldSession::HandleDismountOpcode;

    // Friends
    WorldPacketHandlers[CMSG_CONTACT_LIST].handler = &WorldSession::handleFriendListOpcode;
    WorldPacketHandlers[CMSG_ADD_FRIEND].handler = &WorldSession::handleAddFriendOpcode;
    WorldPacketHandlers[CMSG_DEL_FRIEND].handler = &WorldSession::handleDelFriendOpcode;
    WorldPacketHandlers[CMSG_ADD_IGNORE].handler = &WorldSession::handleAddIgnoreOpcode;
    WorldPacketHandlers[CMSG_DEL_IGNORE].handler = &WorldSession::handleDelIgnoreOpcode;
    //WorldPacketHandlers[CMSG_BUG].handler = &WorldSession::handleBugOpcode;
    WorldPacketHandlers[CMSG_SET_CONTACT_NOTES].handler = &WorldSession::handleSetFriendNote;

    // Areatrigger
    //WorldPacketHandlers[CMSG_AREATRIGGER].handler = &WorldSession::HandleAreaTriggerOpcode;

    // Account Data
    //WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].handler = &WorldSession::handleUpdateAccountData;
    //WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].status = STATUS_AUTHED;
    //WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].handler = &WorldSession::handleRequestAccountData;
    WorldPacketHandlers[CMSG_TOGGLE_PVP].handler = &WorldSession::handleTogglePVPOpcode;

    // Faction / Reputation
    //WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler = &WorldSession::HandleSetAtWarOpcode;
    //WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler = &WorldSession::HandleSetWatchedFactionIndexOpcode;
    //WorldPacketHandlers[CMSG_SET_FACTION_INACTIVE].handler = &WorldSession::HandleSetFactionInactiveOpcode;

    // Player Interaction
    //WorldPacketHandlers[CMSG_GAMEOBJ_USE].handler = &WorldSession::handleGameObjectUse;
    WorldPacketHandlers[CMSG_PLAYED_TIME].handler = &WorldSession::handlePlayedTimeOpcode;
    WorldPacketHandlers[CMSG_SETSHEATHED].handler = &WorldSession::handleSetSheathedOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT].handler = &WorldSession::handleMessageChatOpcode;
    //WorldPacketHandlers[CMSG_EMOTE].handler = &WorldSession::handleEmoteOpcode;
    WorldPacketHandlers[CMSG_TEXT_EMOTE].handler = &WorldSession::handleTextEmoteOpcode;
    //WorldPacketHandlers[CMSG_INSPECT].handler = &WorldSession::handleInspectOpcode;
    //WorldPacketHandlers[SMSG_BARBER_SHOP_RESULT].handler = &WorldSession::handleBarberShopResult;

    // Channels
    WorldPacketHandlers[CMSG_JOIN_CHANNEL].handler = &WorldSession::handleChannelJoin;
    //WorldPacketHandlers[CMSG_LEAVE_CHANNEL].handler = &WorldSession::HandleChannelLeave;
    //WorldPacketHandlers[CMSG_CHANNEL_LIST].handler = &WorldSession::HandleChannelList;
    //WorldPacketHandlers[CMSG_CHANNEL_PASSWORD].handler = &WorldSession::HandleChannelPassword;
    //WorldPacketHandlers[CMSG_CHANNEL_SET_OWNER].handler = &WorldSession::HandleChannelSetOwner;
    //WorldPacketHandlers[CMSG_CHANNEL_OWNER].handler = &WorldSession::HandleChannelOwner;
    //WorldPacketHandlers[CMSG_CHANNEL_MODERATOR].handler = &WorldSession::HandleChannelModerator;
    //WorldPacketHandlers[CMSG_CHANNEL_UNMODERATOR].handler = &WorldSession::HandleChannelUnmoderator;
    //WorldPacketHandlers[CMSG_CHANNEL_MUTE].handler = &WorldSession::HandleChannelMute;
    //WorldPacketHandlers[CMSG_CHANNEL_UNMUTE].handler = &WorldSession::HandleChannelUnmute;
    //WorldPacketHandlers[CMSG_CHANNEL_INVITE].handler = &WorldSession::HandleChannelInvite;
    //WorldPacketHandlers[CMSG_CHANNEL_KICK].handler = &WorldSession::HandleChannelKick;
    //WorldPacketHandlers[CMSG_CHANNEL_BAN].handler = &WorldSession::HandleChannelBan;
    //WorldPacketHandlers[CMSG_CHANNEL_UNBAN].handler = &WorldSession::HandleChannelUnban;
    //WorldPacketHandlers[CMSG_CHANNEL_ANNOUNCEMENTS].handler = &WorldSession::HandleChannelAnnounce;
    //WorldPacketHandlers[CMSG_CHANNEL_MODERATE].handler = &WorldSession::HandleChannelModerate;
    WorldPacketHandlers[CMSG_GET_CHANNEL_MEMBER_COUNT].handler = &WorldSession::handleGetChannelMemberCount;
    //WorldPacketHandlers[CMSG_CHANNEL_DISPLAY_LIST].handler = &WorldSession::HandleChannelRosterQuery;

    // Groups / Raids
    WorldPacketHandlers[CMSG_GROUP_INVITE].handler = &WorldSession::handleGroupInviteOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CANCEL].handler = &WorldSession::handleGroupCancelOpcode;
    WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler = &WorldSession::handleGroupAcceptOpcode;
    WorldPacketHandlers[CMSG_GROUP_DECLINE].handler = &WorldSession::handleGroupDeclineOpcode;
    //WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler = &WorldSession::HandleGroupUninviteOpcode;
    //WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler = &WorldSession::HandleGroupUninviteGuidOpcode;
    //WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler = &WorldSession::HandleGroupSetLeaderOpcode;
    //WorldPacketHandlers[CMSG_GROUP_DISBAND].handler = &WorldSession::HandleGroupDisbandOpcode;
    //WorldPacketHandlers[CMSG_LOOT_METHOD].handler = &WorldSession::HandleLootMethodOpcode;
    //WorldPacketHandlers[MSG_MINIMAP_PING].handler = &WorldSession::HandleMinimapPingOpcode;
    //WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler = &WorldSession::HandleConvertGroupToRaidOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler = &WorldSession::HandleGroupChangeSubGroup;
    //WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler = &WorldSession::HandleGroupAssistantLeader;
    WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler = &WorldSession::handleRequestRaidInfoOpcode;
    //WorldPacketHandlers[MSG_RAID_READY_CHECK].handler = &WorldSession::HandleReadyCheckOpcode;
    //WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler = &WorldSession::HandleSetPlayerIconOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler = &WorldSession::HandlePartyMemberStatsOpcode;
    //WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler = &WorldSession::HandleGroupPromote;

    // LFG System
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
    //WorldPacketHandlers[CMSG_ACTIVATETAXI].handler = &WorldSession::handleActivateTaxiOpcode;
    //WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler = &WorldSession::HandleTabardVendorActivateOpcode;
    //WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler = &WorldSession::HandleBankerActivateOpcode;
    //WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler = &WorldSession::HandleBuyBankSlotOpcode;
    //WorldPacketHandlers[CMSG_TRAINER_LIST].handler = &WorldSession::HandleTrainerListOpcode;
    //WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler = &WorldSession::HandleTrainerBuySpellOpcode;
    //WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler = &WorldSession::HandleCharterShowListOpcode;
    //WorldPacketHandlers[MSG_AUCTION_HELLO].handler = &WorldSession::HandleAuctionHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler = &WorldSession::handleGossipHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler = &WorldSession::handleGossipSelectOptionOpcode;
    WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler = &WorldSession::handleSpiritHealerActivateOpcode;
    WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler = &WorldSession::handleNpcTextQueryOpcode;
    //WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler = &WorldSession::HandleBinderActivateOpcode;
    //WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;
    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    //WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    //WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    //WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    //WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
    //WorldPacketHandlers[CMSG_SPLIT_ITEM].handler = &WorldSession::handleSplitOpcode;
    //WorldPacketHandlers[CMSG_READ_ITEM].handler = &WorldSession::handleReadItemOpcode;
    //WorldPacketHandlers[CMSG_REPAIR_ITEM].handler = &WorldSession::handleRepairItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler = &WorldSession::handleAutoBankItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler = &WorldSession::handleAutoStoreBankItemOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler = &WorldSession::handleCancelTemporaryEnchantmentOpcode;
    //WorldPacketHandlers[CMSG_SOCKET_GEMS].handler = &WorldSession::handleInsertGemOpcode;
    //WorldPacketHandlers[CMSG_WRAP_ITEM].handler = &WorldSession::handleWrapItemOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler = &WorldSession::handleItemRefundInfoOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler = &WorldSession::HandleItemRefundRequestOpcode;

    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler = &WorldSession::handleEquipmentSetSave;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler = &WorldSession::handleEquipmentSetUse;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler = &WorldSession::handleEquipmentSetDelete;

    // Spell System / Talent System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    //WorldPacketHandlers[CMSG_SPELLCLICK].handler = &WorldSession::handleSpellClick;
    WorldPacketHandlers[CMSG_CANCEL_CAST].handler = &WorldSession::handleCancelCastOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AURA].handler = &WorldSession::handleCancelAuraOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler = &WorldSession::handleCancelChannellingOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler = &WorldSession::handleCancelAutoRepeatSpellOpcode;
    //WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler = &WorldSession::handleCancelTotem;
    //WorldPacketHandlers[CMSG_LEARN_TALENT].handler = &WorldSession::handleLearnTalentOpcode;
    //WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler = &WorldSession::handleLearnMultipleTalentsOpcode;
    //WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler = &WorldSession::handleUnlearnTalents;
    //WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler = &WorldSession::handleUnlearnTalents;
    //WorldPacketHandlers[CMSG_UPDATE_PROJECTILE_POSITION].handler = &WorldSession::handleUpdateProjectilePosition;

    // Combat / Duel
    WorldPacketHandlers[CMSG_ATTACKSWING].handler = &WorldSession::handleAttackSwingOpcode;
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler = &WorldSession::handleAttackStopOpcode;
    //WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler = &WorldSession::HandleDuelAccepted;
    //WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler = &WorldSession::HandleDuelCancelled;

    // Trade
    //WorldPacketHandlers[CMSG_INITIATE_TRADE].handler = &WorldSession::handleInitiateTradeOpcode;
    //WorldPacketHandlers[CMSG_BEGIN_TRADE].handler = &WorldSession::handleBeginTradeOpcode;
    //WorldPacketHandlers[CMSG_BUSY_TRADE].handler = &WorldSession::handleBusyTrade;
    //WorldPacketHandlers[CMSG_IGNORE_TRADE].handler = &WorldSession::handleIgnoreTrade;
    //WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler = &WorldSession::handleAcceptTrade;
    //WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler = &WorldSession::handleUnacceptTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].handler = &WorldSession::handleCancelTrade;
    //WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler = &WorldSession::handleSetTradeItem;
    //WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler = &WorldSession::handleClearTradeItem;
    //WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler = &WorldSession::handleSetTradeGold;

    // Quest System
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
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
    WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler = &WorldSession::handleResurrectResponse;
    //WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler = &WorldSession::handlePushQuestToPartyOpcode;
    WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler = &WorldSession::handleQuestPushResultOpcode;
    //WorldPacketHandlers[CMSG_QUEST_POI_QUERY].handler = &WorldSession::handleQuestPOIQueryOpcode;

    // Auction System
    //WorldPacketHandlers[CMSG_AUCTION_LIST_ITEMS].handler = &WorldSession::HandleAuctionListItems;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_BIDDER_ITEMS].handler = &WorldSession::HandleAuctionListBidderItems;
    //WorldPacketHandlers[CMSG_AUCTION_SELL_ITEM].handler = &WorldSession::HandleAuctionSellItem;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_OWNER_ITEMS].handler = &WorldSession::HandleAuctionListOwnerItems;
    //WorldPacketHandlers[CMSG_AUCTION_PLACE_BID].handler = &WorldSession::HandleAuctionPlaceBid;
    //WorldPacketHandlers[CMSG_AUCTION_REMOVE_ITEM].handler = &WorldSession::HandleCancelAuction;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_PENDING_SALES].handler = &WorldSession::HandleAuctionListPendingSales;

    // Mail System
    //WorldPacketHandlers[CMSG_GET_MAIL_LIST].handler = &WorldSession::handleGetMailOpcode;
    //WorldPacketHandlers[CMSG_ITEM_TEXT_QUERY].handler = &WorldSession::handleItemTextQueryOpcode;
    //WorldPacketHandlers[CMSG_SEND_MAIL].handler = &WorldSession::handleSendMailOpcode;
    //WorldPacketHandlers[CMSG_MAIL_TAKE_MONEY].handler = &WorldSession::handleTakeMoneyOpcode;
    //WorldPacketHandlers[CMSG_MAIL_TAKE_ITEM].handler = &WorldSession::handleTakeItemOpcode;
    //WorldPacketHandlers[CMSG_MAIL_MARK_AS_READ].handler = &WorldSession::handleMarkAsReadOpcode;
    //WorldPacketHandlers[CMSG_MAIL_RETURN_TO_SENDER].handler = &WorldSession::handleReturnToSenderOpcode;
    //WorldPacketHandlers[CMSG_MAIL_DELETE].handler = &WorldSession::handleMailDeleteOpcode;
    WorldPacketHandlers[MSG_QUERY_NEXT_MAIL_TIME].handler = &WorldSession::handleMailTimeOpcode;
    //WorldPacketHandlers[CMSG_MAIL_CREATE_TEXT_ITEM].handler = &WorldSession::handleMailCreateTextItemOpcode;

    // Guild Query (called when not logged in sometimes)
    //WorldPacketHandlers[CMSG_GUILD_QUERY].handler = &WorldSession::HandleGuildQuery;
    //WorldPacketHandlers[CMSG_GUILD_QUERY].status = STATUS_AUTHED;

    // Guild System
    //WorldPacketHandlers[CMSG_GUILD_CREATE].handler = &WorldSession::HandleCreateGuild;
    //WorldPacketHandlers[CMSG_GUILD_INVITE].handler = &WorldSession::HandleInviteToGuild;
    //WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler = &WorldSession::HandleGuildAccept;
    //WorldPacketHandlers[CMSG_GUILD_DECLINE].handler = &WorldSession::HandleGuildDecline;
    //WorldPacketHandlers[CMSG_GUILD_INFO].handler = &WorldSession::HandleGuildInfo;
    //WorldPacketHandlers[CMSG_GUILD_ROSTER].handler = &WorldSession::HandleGuildRoster;
    //WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler = &WorldSession::HandleGuildPromote;
    //WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler = &WorldSession::HandleGuildDemote;
    //WorldPacketHandlers[CMSG_GUILD_LEAVE].handler = &WorldSession::HandleGuildLeave;
    //WorldPacketHandlers[CMSG_GUILD_REMOVE].handler = &WorldSession::HandleGuildRemove;
    //WorldPacketHandlers[CMSG_GUILD_DISBAND].handler = &WorldSession::HandleGuildDisband;
    //WorldPacketHandlers[CMSG_GUILD_LEADER].handler = &WorldSession::HandleGuildLeader;
    //WorldPacketHandlers[CMSG_GUILD_MOTD].handler = &WorldSession::HandleGuildMotd;
    //WorldPacketHandlers[CMSG_GUILD_RANK].handler = &WorldSession::HandleGuildRank;
    //WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler = &WorldSession::HandleGuildAddRank;
    //WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler = &WorldSession::HandleGuildDelRank;
    //WorldPacketHandlers[CMSG_GUILD_SET_PUBLIC_NOTE].handler = &WorldSession::HandleGuildSetPublicNote;
    //WorldPacketHandlers[CMSG_GUILD_SET_OFFICER_NOTE].handler = &WorldSession::HandleGuildSetOfficerNote;
    //WorldPacketHandlers[CMSG_PETITION_BUY].handler = &WorldSession::HandleCharterBuy;
    //WorldPacketHandlers[CMSG_PETITION_SHOW_SIGNATURES].handler = &WorldSession::HandleCharterShowSignatures;
    //WorldPacketHandlers[CMSG_TURN_IN_PETITION].handler = &WorldSession::HandleCharterTurnInCharter;
    //WorldPacketHandlers[CMSG_PETITION_QUERY].handler = &WorldSession::HandleCharterQuery;
    //WorldPacketHandlers[CMSG_OFFER_PETITION].handler = &WorldSession::HandleCharterOffer;
    //WorldPacketHandlers[CMSG_PETITION_SIGN].handler = &WorldSession::HandleCharterSign;
    //WorldPacketHandlers[MSG_PETITION_DECLINE].handler = &WorldSession::HandleCharterDecline;
    //WorldPacketHandlers[MSG_PETITION_RENAME].handler = &WorldSession::HandleCharterRename;
    //WorldPacketHandlers[MSG_SAVE_GUILD_EMBLEM].handler = &WorldSession::HandleSaveGuildEmblem;
    //WorldPacketHandlers[CMSG_GUILD_INFO_TEXT].handler = &WorldSession::HandleSetGuildInformation;
    //WorldPacketHandlers[MSG_QUERY_GUILD_BANK_TEXT].handler = &WorldSession::HandleGuildBankQueryText;
    //WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler = &WorldSession::HandleSetGuildBankText;
    //WorldPacketHandlers[MSG_GUILD_EVENT_LOG_QUERY].handler = &WorldSession::HandleGuildLog;
    //WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler = &WorldSession::HandleGuildBankOpenVault;
    //WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler = &WorldSession::HandleGuildBankBuyTab;
    WorldPacketHandlers[MSG_GUILD_BANK_MONEY_WITHDRAWN].handler = &WorldSession::handleGuildBankMoneyWithdrawn;
    //WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler = &WorldSession::HandleGuildBankModifyTab;
    //WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler = &WorldSession::HandleGuildBankDepositItem;
    //WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler = &WorldSession::HandleGuildBankWithdrawMoney;
    //WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler = &WorldSession::HandleGuildBankDepositMoney;
    //WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler = &WorldSession::HandleGuildBankViewTab;
    //WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler = &WorldSession::HandleGuildBankViewLog;
    //WorldPacketHandlers[MSG_GUILD_PERMISSIONS].handler = &WorldSession::HandleGuildGetFullPermissions;

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
    //WorldPacketHandlers[CMSG_PET_SPELL_AUTOCAST].handler = &WorldSession::HandlePetSpellAutocast;
    //WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler = &WorldSession::HandlePetCancelAura;
    //WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler = &WorldSession::HandlePetLearnTalent;
    //WorldPacketHandlers[CMSG_DISMISS_CRITTER].handler = &WorldSession::HandleDismissCritter;

    // Battlegrounds
    //WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler = &WorldSession::HandleBattlefieldPortOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler = &WorldSession::handleBattlefieldStatusOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler = &WorldSession::HandleBattlefieldListOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler = &WorldSession::HandleBattleMasterHelloOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler = &WorldSession::HandleArenaJoinOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler = &WorldSession::HandleBattleMasterJoinOpcode;
    //WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler = &WorldSession::HandleLeaveBattlefieldOpcode;
    //WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler = &WorldSession::HandleAreaSpiritHealerQueryOpcode;
    //WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler = &WorldSession::HandleAreaSpiritHealerQueueOpcode;
    //WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler = &WorldSession::HandleBattlegroundPlayerPositionsOpcode;
    //WorldPacketHandlers[MSG_PVP_LOG_DATA].handler = &WorldSession::HandlePVPLogDataOpcode;
    //WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler = &WorldSession::HandleInspectHonorStatsOpcode;
    //WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler = &WorldSession::HandleSetActionBarTogglesOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE].handler = &WorldSession::HandleBgInviteResponse;

    // GM Ticket System
    WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler = &WorldSession::handleGMTicketCreateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler = &WorldSession::handleGMTicketUpdateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler = &WorldSession::handleGMTicketDeleteOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketGetTicketOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler = &WorldSession::handleGMTicketSystemStatusOpcode;
    WorldPacketHandlers[CMSG_GMTICKETSYSTEM_TOGGLE].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;

    // Lag report
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].handler = &WorldSession::handleReportLag;
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].status = STATUS_LOGGEDIN;
    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].handler = &WorldSession::handleGMSurveySubmitOpcode;
    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].status = STATUS_LOGGEDIN;

    // Meeting Stone / Instances
    //WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler = &WorldSession::handleSummonResponseOpcode;
    //WorldPacketHandlers[CMSG_RESET_INSTANCES].handler = &WorldSession::HandleResetInstanceOpcode;
    //WorldPacketHandlers[CMSG_SELF_RES].handler = &WorldSession::HandleSelfResurrectOpcode;
    //WorldPacketHandlers[MSG_RANDOM_ROLL].handler = &WorldSession::HandleRandomRollOpcode;
    //WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler = &WorldSession::HandleDungeonDifficultyOpcode;
    //WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler = &WorldSession::HandleRaidDifficultyOpcode;

    // Misc
    //WorldPacketHandlers[CMSG_OPEN_ITEM].handler = &WorldSession::HandleOpenItemOpcode;
    //WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler = &WorldSession::handleCompleteCinematic;
    //WorldPacketHandlers[CMSG_NEXT_CINEMATIC_CAMERA].handler = &WorldSession::handleNextCinematic;
    //WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler = &WorldSession::handleMountSpecialAnimOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler = &WorldSession::HandleToggleCloakOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_HELM].handler = &WorldSession::HandleToggleHelmOpcode;
    //WorldPacketHandlers[CMSG_SET_TITLE].handler = &WorldSession::HandleSetVisibleRankOpcode;
    //WorldPacketHandlers[CMSG_COMPLAIN].handler = &WorldSession::HandleReportSpamOpcode;
    //WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::HandleGameobjReportUseOpCode;
    //WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler = &WorldSession::handlePetCastSpell;
    //WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::HandleWorldStateUITimerUpdate;
    //WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler = &WorldSession::HandleSetTaxiBenchmarkOpcode;
    //WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler = &WorldSession::handleUnlearnSkillOpcode;

    // Chat
    //WorldPacketHandlers[CMSG_CHAT_IGNORED].handler = &WorldSession::HandleChatIgnoredOpcode;
    //WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler = &WorldSession::HandleChatChannelWatchOpcode;

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
    //WorldPacketHandlers[MSG_INSPECT_ARENA_TEAMS].handler = &WorldSession::HandleInspectArenaStatsOpcode;

    // cheat/gm commands?
    WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::handleWorldTeleportOpcode;

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
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::HandleTimeSyncResp;       //MiscHandler.cpp
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::HandleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::HandleSetAutoLootPassOpcode;
    //WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::handleRemoveGlyph;
    //WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::handleBarberShopResult;
    //WorldPacketHandlers[CMSG_GET_MIRRORIMAGE_DATA].handler = &WorldSession::HandleMirrorImageOpcode;

    // Calendar - Unhandled
    //WorldPacketHandlers[CMSG_CALENDAR_GET_CALENDAR].handler = &WorldSession::HandleCalendarGetCalendar;
    //WorldPacketHandlers[CMSG_CALENDAR_COMPLAIN].handler = &WorldSession::HandleCalendarComplain;
    //WorldPacketHandlers[CMSG_CALENDAR_GET_NUM_PENDING].handler = &WorldSession::HandleCalendarGetNumPending;
    //WorldPacketHandlers[CMSG_CALENDAR_ADD_EVENT].handler = &WorldSession::HandleCalendarAddEvent;

    //WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].handler = &WorldSession::HandleCalendarGetEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].handler = &WorldSession::HandleCalendarGuildFilter;
    //WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].handler = &WorldSession::HandleCalendarArenaTeam;
    //WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].handler = &WorldSession::HandleCalendarUpdateEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].handler = &WorldSession::HandleCalendarRemoveEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].handler = &WorldSession::HandleCalendarCopyEvent;
    //WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].handler = &WorldSession::HandleCalendarEventInvite;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].handler = &WorldSession::HandleCalendarEventRsvp;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].handler = &WorldSession::HandleCalendarEventRemoveInvite;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].handler = &WorldSession::HandleCalendarEventStatus;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].status = STATUS_LOGGEDIN;

    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].handler = &WorldSession::HandleCalendarEventModeratorStatus;
    //WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].status = STATUS_LOGGEDIN;

    //Misc - Unhandled
    //WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;
}
#elif VERSION_STRING == WotLK
void WorldSession::loadHandlers()
{
    // Login
    WorldPacketHandlers[CMSG_CHAR_ENUM].handler = &WorldSession::handleCharEnumOpcode;
    WorldPacketHandlers[CMSG_CHAR_ENUM].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CREATE].handler = &WorldSession::handleCharCreateOpcode;
    WorldPacketHandlers[CMSG_CHAR_CREATE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_DELETE].handler = &WorldSession::handleCharDeleteOpcode;
    WorldPacketHandlers[CMSG_CHAR_DELETE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_RENAME].handler = &WorldSession::handleCharRenameOpcode;
    WorldPacketHandlers[CMSG_CHAR_RENAME].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].handler = &WorldSession::handleCharCustomizeLooksOpcode;
    WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].handler = &WorldSession::handleCharFactionOrRaceChange;
    WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].handler = &WorldSession::handleCharFactionOrRaceChange;
    WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].status = STATUS_AUTHED;

    // declined names (Cyrillic client)
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].handler = &WorldSession::handleDeclinedPlayerNameOpcode;
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler = &WorldSession::handlePlayerLoginOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGIN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_REALM_SPLIT].handler = &WorldSession::handleRealmSplitOpcode;
    WorldPacketHandlers[CMSG_REALM_SPLIT].status = STATUS_AUTHED;

    // Queries
    WorldPacketHandlers[MSG_CORPSE_QUERY].handler = &WorldSession::handleCorpseQueryOpcode;
    WorldPacketHandlers[CMSG_NAME_QUERY].handler = &WorldSession::handleNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_TIME].handler = &WorldSession::handleQueryTimeOpcode;
    WorldPacketHandlers[CMSG_CREATURE_QUERY].handler = &WorldSession::handleCreatureQueryOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler = &WorldSession::handleGameObjectQueryOpcode;
    WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler = &WorldSession::handlePageTextQueryOpcode;
    WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler = &WorldSession::handleItemNameQueryOpcode;
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
    WorldPacketHandlers[CMSG_MOVE_SET_FLY].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_NOT_ACTIVE_MOVER].handler = &WorldSession::handleMoveNotActiveMoverOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIVE_MOVER].handler = &WorldSession::handleSetActiveMoverOpcode;
    WorldPacketHandlers[CMSG_MOVE_CHNG_TRANSPORT].handler = &WorldSession::handleMovementOpcodes;

    // ACK
    WorldPacketHandlers[MSG_MOVE_TELEPORT_ACK].handler = &WorldSession::handleMoveTeleportAckOpcode;
    WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;

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
    WorldPacketHandlers[CMSG_PLAYER_LOGOUT].handler = &WorldSession::handlePlayerLogoutOpcode;
    WorldPacketHandlers[CMSG_LOGOUT_CANCEL].handler = &WorldSession::handleLogoutCancelOpcode;
    // WorldPacketHandlers[CMSG_LOGOUT_CANCEL].status = STATUS_LOGGEDIN_RECENTLY_LOGGOUT;

    WorldPacketHandlers[CMSG_ZONEUPDATE].handler = &WorldSession::handleZoneupdate;
    // WorldPacketHandlers[CMSG_SET_TARGET_OBSOLETE].handler = &WorldSession::HandleSetTargetOpcode;
    WorldPacketHandlers[CMSG_SET_SELECTION].handler = &WorldSession::handleSetSelectionOpcode;
    WorldPacketHandlers[CMSG_STANDSTATECHANGE].handler = &WorldSession::handleStandStateChangeOpcode;
    WorldPacketHandlers[CMSG_CANCEL_MOUNT_AURA].handler = &WorldSession::handleDismountOpcode;

    // Friends
    WorldPacketHandlers[CMSG_CONTACT_LIST].handler = &WorldSession::handleFriendListOpcode;
    WorldPacketHandlers[CMSG_ADD_FRIEND].handler = &WorldSession::handleAddFriendOpcode;
    WorldPacketHandlers[CMSG_DEL_FRIEND].handler = &WorldSession::handleDelFriendOpcode;
    WorldPacketHandlers[CMSG_ADD_IGNORE].handler = &WorldSession::handleAddIgnoreOpcode;
    WorldPacketHandlers[CMSG_DEL_IGNORE].handler = &WorldSession::handleDelIgnoreOpcode;
    WorldPacketHandlers[CMSG_BUG].handler = &WorldSession::handleBugOpcode;
    WorldPacketHandlers[CMSG_SET_CONTACT_NOTES].handler = &WorldSession::handleSetFriendNote;

    // Areatrigger
    WorldPacketHandlers[CMSG_AREATRIGGER].handler = &WorldSession::handleAreaTriggerOpcode;

    // Account Data
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].handler = &WorldSession::handleUpdateAccountData;
    WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].status = STATUS_AUTHED;
    WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].handler = &WorldSession::handleRequestAccountData;
    WorldPacketHandlers[CMSG_TOGGLE_PVP].handler = &WorldSession::handleTogglePVPOpcode;

    // Faction / Reputation
    WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler = &WorldSession::handleSetFactionAtWarOpcode;
    WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler = &WorldSession::handleSetWatchedFactionIndexOpcode;
    WorldPacketHandlers[CMSG_SET_FACTION_INACTIVE].handler = &WorldSession::handleSetFactionInactiveOpcode;

    // Player Interaction
    WorldPacketHandlers[CMSG_GAMEOBJ_USE].handler = &WorldSession::handleGameObjectUse;
    WorldPacketHandlers[CMSG_PLAYED_TIME].handler = &WorldSession::handlePlayedTimeOpcode;
    WorldPacketHandlers[CMSG_SETSHEATHED].handler = &WorldSession::handleSetSheathedOpcode;
    WorldPacketHandlers[CMSG_MESSAGECHAT].handler = &WorldSession::handleMessageChatOpcode;
    WorldPacketHandlers[CMSG_EMOTE].handler = &WorldSession::handleEmoteOpcode;
    WorldPacketHandlers[CMSG_TEXT_EMOTE].handler = &WorldSession::handleTextEmoteOpcode;
    WorldPacketHandlers[CMSG_INSPECT].handler = &WorldSession::handleInspectOpcode;
    // clearly wrong naming!
    //WorldPacketHandlers[SMSG_BARBER_SHOP_RESULT].handler = &WorldSession::handleBarberShopResult;

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
    //WorldPacketHandlers[CMSG_GROUP_CANCEL].handler = &WorldSession::HandleGroupCancelOpcode;
    WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler = &WorldSession::handleGroupAcceptOpcode;
    WorldPacketHandlers[CMSG_GROUP_DECLINE].handler = &WorldSession::handleGroupDeclineOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler = &WorldSession::handleGroupUninviteOpcode;
    WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler = &WorldSession::handleGroupUninviteGuidOpcode;
    WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler = &WorldSession::handleGroupSetLeaderOpcode;
    WorldPacketHandlers[CMSG_GROUP_DISBAND].handler = &WorldSession::handleGroupDisbandOpcode;
    WorldPacketHandlers[CMSG_LOOT_METHOD].handler = &WorldSession::handleLootMethodOpcode;
    WorldPacketHandlers[MSG_MINIMAP_PING].handler = &WorldSession::handleMinimapPingOpcode;
    WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler = &WorldSession::handleConvertGroupToRaidOpcode;
    WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler = &WorldSession::handleGroupChangeSubGroup;
    WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler = &WorldSession::handleGroupAssistantLeader;
    WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler = &WorldSession::handleRequestRaidInfoOpcode;
    WorldPacketHandlers[MSG_RAID_READY_CHECK].handler = &WorldSession::handleReadyCheckOpcode;
    WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler = &WorldSession::handleSetPlayerIconOpcode;
    WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler = &WorldSession::handlePartyMemberStatsOpcode;
    WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler = &WorldSession::handleGroupPromote;

    // LFG System
    WorldPacketHandlers[CMSG_SET_LFG_COMMENT].handler = &WorldSession::handleLfgSetCommentOpcode;
    WorldPacketHandlers[CMSG_LFG_JOIN].handler = &WorldSession::handleLfgJoinOpcode;
    WorldPacketHandlers[CMSG_LFG_LEAVE].handler = &WorldSession::handleLfgLeaveOpcode;
    WorldPacketHandlers[CMSG_SEARCH_LFG_JOIN].handler = &WorldSession::handleLfgSearchOpcode;
    WorldPacketHandlers[CMSG_SEARCH_LFG_LEAVE].handler = &WorldSession::handleLfgSearchLeaveOpcode;
    WorldPacketHandlers[CMSG_LFG_PROPOSAL_RESULT].handler = &WorldSession::handleLfgProposalResultOpcode;
    WorldPacketHandlers[CMSG_LFG_SET_ROLES].handler = &WorldSession::handleLfgSetRolesOpcode;
    WorldPacketHandlers[CMSG_LFG_SET_BOOT_VOTE].handler = &WorldSession::handleLfgSetBootVoteOpcode;
    WorldPacketHandlers[CMSG_LFD_PLAYER_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPlayerLockInfoRequestOpcode;
    WorldPacketHandlers[CMSG_LFG_TELEPORT].handler = &WorldSession::handleLfgTeleportOpcode;
    WorldPacketHandlers[CMSG_LFD_PARTY_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPartyLockInfoRequestOpcode;

    // Taxi / NPC Interaction
    WorldPacketHandlers[CMSG_ENABLETAXI].handler = &WorldSession::handleEnabletaxiOpcode;
    WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler = &WorldSession::handleTaxiNodeStatusQueryOpcode;
    WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler = &WorldSession::handleTaxiQueryAvaibleNodesOpcode;
    WorldPacketHandlers[CMSG_ACTIVATETAXI].handler = &WorldSession::handleActivateTaxiOpcode;
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
    WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;
    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
    WorldPacketHandlers[CMSG_SPLIT_ITEM].handler = &WorldSession::handleSplitOpcode;
    WorldPacketHandlers[CMSG_READ_ITEM].handler = &WorldSession::handleReadItemOpcode;
    WorldPacketHandlers[CMSG_REPAIR_ITEM].handler = &WorldSession::handleRepairItemOpcode;
    WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler = &WorldSession::handleAutoBankItemOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler = &WorldSession::handleAutoStoreBankItemOpcode;
    WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler = &WorldSession::handleCancelTemporaryEnchantmentOpcode;
    WorldPacketHandlers[CMSG_SOCKET_GEMS].handler = &WorldSession::handleInsertGemOpcode;
    WorldPacketHandlers[CMSG_WRAP_ITEM].handler = &WorldSession::handleWrapItemOpcode;
    WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler = &WorldSession::handleItemRefundInfoOpcode;
    WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler = &WorldSession::handleItemRefundRequestOpcode;

    WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler = &WorldSession::handleEquipmentSetSave;
    WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler = &WorldSession::handleEquipmentSetUse;
    WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler = &WorldSession::handleEquipmentSetDelete;

    // Spell System / Talent System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    WorldPacketHandlers[CMSG_SPELLCLICK].handler = &WorldSession::handleSpellClick;
    WorldPacketHandlers[CMSG_CANCEL_CAST].handler = &WorldSession::handleCancelCastOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AURA].handler = &WorldSession::handleCancelAuraOpcode;
    WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler = &WorldSession::handleCancelChannellingOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler = &WorldSession::handleCancelAutoRepeatSpellOpcode;
    WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler = &WorldSession::handleCancelTotem;
    WorldPacketHandlers[CMSG_LEARN_TALENT].handler = &WorldSession::handleLearnTalentOpcode;
    WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler = &WorldSession::handleLearnMultipleTalentsOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler = &WorldSession::handleUnlearnTalents;
    WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler = &WorldSession::handleUnlearnTalents;
    WorldPacketHandlers[CMSG_UPDATE_PROJECTILE_POSITION].handler = &WorldSession::handleUpdateProjectilePosition;
    // Combat / Duel
    WorldPacketHandlers[CMSG_ATTACKSWING].handler = &WorldSession::handleAttackSwingOpcode;
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler = &WorldSession::handleAttackStopOpcode;
    WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler = &WorldSession::handleDuelAccepted;
    WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler = &WorldSession::handleDuelCancelled;

    // Trade
    WorldPacketHandlers[CMSG_INITIATE_TRADE].handler = &WorldSession::handleInitiateTradeOpcode;
    WorldPacketHandlers[CMSG_BEGIN_TRADE].handler = &WorldSession::handleBeginTradeOpcode;
    WorldPacketHandlers[CMSG_BUSY_TRADE].handler = &WorldSession::handleBusyTrade;
    WorldPacketHandlers[CMSG_IGNORE_TRADE].handler = &WorldSession::handleIgnoreTrade;
    WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler = &WorldSession::handleAcceptTrade;
    WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler = &WorldSession::handleUnacceptTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].handler = &WorldSession::handleCancelTrade;
    WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler = &WorldSession::handleSetTradeItem;
    WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler = &WorldSession::handleClearTradeItem;
    WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler = &WorldSession::handleSetTradeGold;

    // Quest System
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
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
    WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler = &WorldSession::handleResurrectResponse;
    WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler = &WorldSession::handlePushQuestToPartyOpcode;
    WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler = &WorldSession::handleQuestPushResultOpcode;
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

    // Guild Query (called when not logged in sometimes)
    WorldPacketHandlers[CMSG_GUILD_QUERY].handler = &WorldSession::handleGuildQuery;
    WorldPacketHandlers[CMSG_GUILD_QUERY].status = STATUS_AUTHED;

    // Guild System
    //WorldPacketHandlers[CMSG_GUILD_CREATE].handler = &WorldSession::HandleCreateGuild;
    WorldPacketHandlers[CMSG_GUILD_INVITE].handler = &WorldSession::handleInviteToGuild;
    WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler = &WorldSession::handleGuildAccept;
    WorldPacketHandlers[CMSG_GUILD_DECLINE].handler = &WorldSession::handleGuildDecline;
    WorldPacketHandlers[CMSG_GUILD_INFO].handler = &WorldSession::handleGuildInfo;
    WorldPacketHandlers[CMSG_GUILD_ROSTER].handler = &WorldSession::handleGuildRoster;
    WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler = &WorldSession::handleGuildPromote;
    WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler = &WorldSession::handleGuildDemote;
    WorldPacketHandlers[CMSG_GUILD_LEAVE].handler = &WorldSession::handleGuildLeave;
    WorldPacketHandlers[CMSG_GUILD_REMOVE].handler = &WorldSession::handleGuildRemove;
    WorldPacketHandlers[CMSG_GUILD_DISBAND].handler = &WorldSession::handleGuildDisband;
    WorldPacketHandlers[CMSG_GUILD_LEADER].handler = &WorldSession::handleGuildLeader;
    WorldPacketHandlers[CMSG_GUILD_MOTD].handler = &WorldSession::handleGuildMotd;
    WorldPacketHandlers[CMSG_GUILD_SET_RANK].handler = &WorldSession::handleGuildSetRank;
    WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler = &WorldSession::handleGuildAddRank;
    WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler = &WorldSession::handleGuildDelRank;
    WorldPacketHandlers[CMSG_GUILD_SET_PUBLIC_NOTE].handler = &WorldSession::handleGuildSetPublicNote;
    WorldPacketHandlers[CMSG_GUILD_SET_OFFICER_NOTE].handler = &WorldSession::handleGuildSetOfficerNote;
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
    WorldPacketHandlers[MSG_QUERY_GUILD_BANK_TEXT].handler = &WorldSession::handleGuildBankQueryText;
    WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler = &WorldSession::handleSetGuildBankText;
    WorldPacketHandlers[MSG_GUILD_EVENT_LOG_QUERY].handler = &WorldSession::handleGuildLog;
    WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler = &WorldSession::handleGuildBankerActivate;
    WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler = &WorldSession::handleGuildBankBuyTab;
    WorldPacketHandlers[MSG_GUILD_BANK_MONEY_WITHDRAWN].handler = &WorldSession::handleGuildBankMoneyWithdrawn;
    WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler = &WorldSession::handleGuildBankUpdateTab;
    WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler = &WorldSession::handleGuildBankSwapItems;
    WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler = &WorldSession::handleGuildBankWithdrawMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler = &WorldSession::handleGuildBankDepositMoney;
    WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler = &WorldSession::handleGuildBankQueryTab;
    WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler = &WorldSession::handleGuildBankLogQuery;
    WorldPacketHandlers[MSG_GUILD_PERMISSIONS].handler = &WorldSession::handleGuildPermissions;

    // Tutorials
    WorldPacketHandlers[CMSG_TUTORIAL_FLAG].handler = &WorldSession::handleTutorialFlag;
    WorldPacketHandlers[CMSG_TUTORIAL_CLEAR].handler = &WorldSession::handleTutorialClear;
    WorldPacketHandlers[CMSG_TUTORIAL_RESET].handler = &WorldSession::handleTutorialReset;

    // Pets
    WorldPacketHandlers[MSG_LIST_STABLED_PETS].handler = &WorldSession::handleStabledPetList;

    WorldPacketHandlers[CMSG_PET_ACTION].handler = &WorldSession::handlePetAction;
    WorldPacketHandlers[CMSG_PET_NAME_QUERY].handler = &WorldSession::handlePetNameQuery;
    WorldPacketHandlers[CMSG_BUY_STABLE_SLOT].handler = &WorldSession::handleBuyStableSlot;
    WorldPacketHandlers[CMSG_STABLE_PET].handler = &WorldSession::handleStablePet;
    WorldPacketHandlers[CMSG_UNSTABLE_PET].handler = &WorldSession::handleUnstablePet;
    WorldPacketHandlers[CMSG_STABLE_SWAP_PET].handler = &WorldSession::handleStableSwapPet;
    WorldPacketHandlers[CMSG_PET_SET_ACTION].handler = &WorldSession::handlePetSetActionOpcode;
    WorldPacketHandlers[CMSG_PET_RENAME].handler = &WorldSession::handlePetRename;
    WorldPacketHandlers[CMSG_PET_ABANDON].handler = &WorldSession::handlePetAbandon;
    WorldPacketHandlers[CMSG_PET_UNLEARN].handler = &WorldSession::handlePetUnlearn;
    WorldPacketHandlers[CMSG_PET_SPELL_AUTOCAST].handler = &WorldSession::handlePetSpellAutocast;
    WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler = &WorldSession::handlePetCancelAura;
    WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler = &WorldSession::handlePetLearnTalent;
    WorldPacketHandlers[CMSG_DISMISS_CRITTER].handler = &WorldSession::handleDismissCritter;

    // Battlegrounds
    WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler = &WorldSession::handleBattlefieldPortOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler = &WorldSession::handleBattlefieldStatusOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler = &WorldSession::handleBattlefieldListOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler = &WorldSession::handleBattleMasterHelloOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler = &WorldSession::handleArenaJoinOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler = &WorldSession::handleBattleMasterJoinOpcode;
    WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler = &WorldSession::handleLeaveBattlefieldOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler = &WorldSession::handleAreaSpiritHealerQueryOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler = &WorldSession::handleAreaSpiritHealerQueueOpcode;
    WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler = &WorldSession::handleBattlegroundPlayerPositionsOpcode;
    WorldPacketHandlers[MSG_PVP_LOG_DATA].handler = &WorldSession::handlePVPLogDataOpcode;
    WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler = &WorldSession::handleInspectHonorStatsOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler = &WorldSession::handleSetActionBarTogglesOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE].handler = &WorldSession::HandleBgInviteResponse;

    // GM Ticket System
    WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler = &WorldSession::handleGMTicketCreateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler = &WorldSession::handleGMTicketUpdateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler = &WorldSession::handleGMTicketDeleteOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketGetTicketOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler = &WorldSession::handleGMTicketSystemStatusOpcode;
    WorldPacketHandlers[CMSG_GMTICKETSYSTEM_TOGGLE].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;

    // Lag report
    WorldPacketHandlers[CMSG_GM_REPORT_LAG].handler = &WorldSession::handleReportLag;
    WorldPacketHandlers[CMSG_GM_REPORT_LAG].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].handler = &WorldSession::handleGMSurveySubmitOpcode;
    WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].status = STATUS_LOGGEDIN;

    // Meeting Stone / Instances
    WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler = &WorldSession::handleSummonResponseOpcode;
    WorldPacketHandlers[CMSG_RESET_INSTANCES].handler = &WorldSession::handleResetInstanceOpcode;
    WorldPacketHandlers[CMSG_SELF_RES].handler = &WorldSession::handleSelfResurrect;
    WorldPacketHandlers[MSG_RANDOM_ROLL].handler = &WorldSession::handleRandomRollOpcode;
    WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler = &WorldSession::handleDungeonDifficultyOpcode;
    WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler = &WorldSession::handleRaidDifficultyOpcode;
    
    // Misc
    WorldPacketHandlers[CMSG_OPEN_ITEM].handler = &WorldSession::handleOpenItemOpcode;
    WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler = &WorldSession::handleCompleteCinematic;
    WorldPacketHandlers[CMSG_NEXT_CINEMATIC_CAMERA].handler = &WorldSession::handleNextCinematic;
    WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler = &WorldSession::handleMountSpecialAnimOpcode;
    WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler = &WorldSession::handleToggleCloakOpcode;
    WorldPacketHandlers[CMSG_TOGGLE_HELM].handler = &WorldSession::handleToggleHelmOpcode;
    WorldPacketHandlers[CMSG_SET_TITLE].handler = &WorldSession::handleSetTitle;
    WorldPacketHandlers[CMSG_COMPLAIN].handler = &WorldSession::handleReportSpamOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::handleGameobjReportUseOpCode;
    WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler = &WorldSession::handlePetCastSpell;
    WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::handleWorldStateUITimerUpdate;
    WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler = &WorldSession::handleSetTaxiBenchmarkOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler = &WorldSession::handleUnlearnSkillOpcode;

    // Chat
    WorldPacketHandlers[CMSG_CHAT_IGNORED].handler = &WorldSession::handleChatIgnoredOpcode;
    WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler = &WorldSession::handleChatChannelWatchOpcode;

    // Arenas
    WorldPacketHandlers[CMSG_ARENA_TEAM_QUERY].handler = &WorldSession::handleArenaTeamQueryOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_ROSTER].handler = &WorldSession::handleArenaTeamRosterOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_INVITE].handler = &WorldSession::handleArenaTeamAddMemberOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_ACCEPT].handler = &WorldSession::handleArenaTeamInviteAcceptOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_DECLINE].handler = &WorldSession::handleArenaTeamInviteDenyOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_LEAVE].handler = &WorldSession::handleArenaTeamLeaveOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_REMOVE].handler = &WorldSession::handleArenaTeamRemoveMemberOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_DISBAND].handler = &WorldSession::handleArenaTeamDisbandOpcode;
    WorldPacketHandlers[CMSG_ARENA_TEAM_LEADER].handler = &WorldSession::handleArenaTeamPromoteOpcode;
    WorldPacketHandlers[MSG_INSPECT_ARENA_TEAMS].handler = &WorldSession::handleInspectArenaStatsOpcode;

    // cheat/gm commands?
    WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::handleWorldTeleportOpcode;

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
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::HandleTimeSyncResp;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::handleReadyForAccountDataTimes;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::handleSetAutoLootPassOpcode;
    WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::handleRemoveGlyph;
    WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::handleBarberShopResult;
    WorldPacketHandlers[CMSG_GET_MIRRORIMAGE_DATA].handler = &WorldSession::HandleMirrorImageOpcode;

    // Calendar - Unhandled
    WorldPacketHandlers[CMSG_CALENDAR_GET_CALENDAR].handler = &WorldSession::handleCalendarGetCalendar;
    WorldPacketHandlers[CMSG_CALENDAR_COMPLAIN].handler = &WorldSession::handleCalendarComplain;
    WorldPacketHandlers[CMSG_CALENDAR_GET_NUM_PENDING].handler = &WorldSession::handleCalendarGetNumPending;
    WorldPacketHandlers[CMSG_CALENDAR_ADD_EVENT].handler = &WorldSession::handleCalendarAddEvent;

    WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].handler = &WorldSession::handleCalendarGetEvent;
    WorldPacketHandlers[CMSG_CALENDAR_GET_EVENT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].handler = &WorldSession::handleCalendarGuildFilter;
    WorldPacketHandlers[CMSG_CALENDAR_GUILD_FILTER].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].handler = &WorldSession::handleCalendarArenaTeam;
    WorldPacketHandlers[CMSG_CALENDAR_ARENA_TEAM].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].handler = &WorldSession::handleCalendarUpdateEvent;
    WorldPacketHandlers[CMSG_CALENDAR_UPDATE_EVENT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].handler = &WorldSession::handleCalendarRemoveEvent;
    WorldPacketHandlers[CMSG_CALENDAR_REMOVE_EVENT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].handler = &WorldSession::handleCalendarCopyEvent;
    WorldPacketHandlers[CMSG_CALENDAR_COPY_EVENT].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].handler = &WorldSession::handleCalendarEventInvite;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_INVITE].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].handler = &WorldSession::handleCalendarEventRsvp;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_RSVP].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].handler = &WorldSession::handleCalendarEventRemoveInvite;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_REMOVE_INVITE].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].handler = &WorldSession::handleCalendarEventStatus;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_STATUS].status = STATUS_LOGGEDIN;

    WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].handler = &WorldSession::handleCalendarEventModeratorStatus;
    WorldPacketHandlers[CMSG_CALENDAR_EVENT_MODERATOR_STATUS].status = STATUS_LOGGEDIN;

    //Misc - Unhandled
    WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;
}
#elif VERSION_STRING == Cata
void WorldSession::loadHandlers()
{
    // Login
    WorldPacketHandlers[CMSG_CHAR_ENUM].handler = &WorldSession::handleCharEnumOpcode;
    WorldPacketHandlers[CMSG_CHAR_ENUM].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CREATE].handler = &WorldSession::handleCharCreateOpcode;
    WorldPacketHandlers[CMSG_CHAR_CREATE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_DELETE].handler = &WorldSession::handleCharDeleteOpcode;
    WorldPacketHandlers[CMSG_CHAR_DELETE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_RENAME].handler = &WorldSession::handleCharRenameOpcode;
    WorldPacketHandlers[CMSG_CHAR_RENAME].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].handler = &WorldSession::handleCharCustomizeLooksOpcode;
    WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].status = STATUS_AUTHED;

    // declined names (Cyrillic client)
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].handler = &WorldSession::handleDeclinedPlayerNameOpcode;
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler = &WorldSession::handlePlayerLoginOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGIN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_REALM_SPLIT].handler = &WorldSession::handleRealmSplitOpcode;
    WorldPacketHandlers[CMSG_REALM_SPLIT].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].handler = &WorldSession::handleObjectUpdateFailedOpcode;
    WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_LOAD_SCREEN].handler = &WorldSession::handleLoadScreenOpcode;
    WorldPacketHandlers[CMSG_LOAD_SCREEN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::handleReadyForAccountDataTimes;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_UI_TIME_REQUEST].handler = &WorldSession::handleUITimeRequestOpcode;
    WorldPacketHandlers[CMSG_UI_TIME_REQUEST].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::handleTimeSyncRespOpcode;
    WorldPacketHandlers[CMSG_TIME_SYNC_RESP].status = STATUS_AUTHED;

    // Queries
    WorldPacketHandlers[MSG_CORPSE_QUERY].handler = &WorldSession::handleCorpseQueryOpcode;
    WorldPacketHandlers[CMSG_NAME_QUERY].handler = &WorldSession::handleNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_TIME].handler = &WorldSession::handleQueryTimeOpcode;
    WorldPacketHandlers[CMSG_CREATURE_QUERY].handler = &WorldSession::handleCreatureQueryOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler = &WorldSession::handleGameObjectQueryOpcode;
    //WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler = &WorldSession::HandlePageTextQueryOpcode;
    //WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler = &WorldSession::HandleItemNameQueryOpcode;
    //WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler = &WorldSession::HandleAchievmentQueryOpcode;

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
    WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_PITCH_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;

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
    //WorldPacketHandlers[CMSG_AREATRIGGER].handler = &WorldSession::HandleAreaTriggerOpcode;

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
    WorldPacketHandlers[CMSG_PLAYED_TIME].handler = &WorldSession::handlePlayedTimeOpcode;
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
    WorldPacketHandlers[CMSG_ACTIVATETAXI].handler = &WorldSession::handleActivateTaxiOpcode;
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
    WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;

    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    //WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    //WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
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

    // Spell System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    WorldPacketHandlers[CMSG_SPELLCLICK].handler = &WorldSession::handleSpellClick;
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
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler = &WorldSession::handleAttackStopOpcode;
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
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
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
    WorldPacketHandlers[CMSG_SUGGESTION].handler = &WorldSession::handleSuggestionOpcode;
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
    //WorldPacketHandlers[CMSG_DISMISS_CONTROLLED_VEHICLE].handler = &WorldSession::HandleDismissVehicle;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_EXIT].handler = &WorldSession::HandleLeaveVehicle;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_PREV_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_NEXT_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_SWITCH_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_PLAYER_VEHICLE_ENTER].handler = &WorldSession::HandleEnterVehicle;
    //WorldPacketHandlers[CMSG_EJECT_PASSENGER].handler = &WorldSession::HandleRemoveVehiclePassenger;

    // Unsorted
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::HandleTimeSyncResOp;       //MiscHandler.cpp
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::HandleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::HandleSetAutoLootPassOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
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
}
#elif VERSION_STRING == Mop
void WorldSession::loadHandlers()
{
    // Login
    WorldPacketHandlers[CMSG_CHAR_ENUM].handler = &WorldSession::handleCharEnumOpcode;
    WorldPacketHandlers[CMSG_CHAR_ENUM].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CREATE].handler = &WorldSession::handleCharCreateOpcode;
    WorldPacketHandlers[CMSG_CHAR_CREATE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_DELETE].handler = &WorldSession::handleCharDeleteOpcode;
    WorldPacketHandlers[CMSG_CHAR_DELETE].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_RENAME].handler = &WorldSession::handleCharRenameOpcode;
    WorldPacketHandlers[CMSG_CHAR_RENAME].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].handler = &WorldSession::handleCharCustomizeLooksOpcode;
    WorldPacketHandlers[CMSG_CHAR_CUSTOMIZE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_FACTION_CHANGE].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].handler = &WorldSession::HandleCharFactionOrRaceChange;
    //WorldPacketHandlers[CMSG_CHAR_RACE_CHANGE].status = STATUS_AUTHED;

    // declined names (Cyrillic client)
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].handler = &WorldSession::handleDeclinedPlayerNameOpcode;
    WorldPacketHandlers[CMSG_SET_PLAYER_DECLINED_NAMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler = &WorldSession::handlePlayerLoginOpcode;
    WorldPacketHandlers[CMSG_PLAYER_LOGIN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_REALM_SPLIT].handler = &WorldSession::handleRealmSplitOpcode;
    WorldPacketHandlers[CMSG_REALM_SPLIT].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].handler = &WorldSession::handleObjectUpdateFailedOpcode;
    WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_LOAD_SCREEN].handler = &WorldSession::handleLoadScreenOpcode;
    WorldPacketHandlers[CMSG_LOAD_SCREEN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::handleReadyForAccountDataTimes;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_UI_TIME_REQUEST].handler = &WorldSession::handleUITimeRequestOpcode;
    WorldPacketHandlers[CMSG_UI_TIME_REQUEST].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::handleTimeSyncRespOpcode;
    WorldPacketHandlers[CMSG_TIME_SYNC_RESP].status = STATUS_AUTHED;

    // Queries
    WorldPacketHandlers[MSG_CORPSE_QUERY].handler = &WorldSession::handleCorpseQueryOpcode;
    WorldPacketHandlers[CMSG_NAME_QUERY].handler = &WorldSession::handleNameQueryOpcode;
    WorldPacketHandlers[CMSG_QUERY_TIME].handler = &WorldSession::handleQueryTimeOpcode;
    WorldPacketHandlers[CMSG_CREATURE_QUERY].handler = &WorldSession::handleCreatureQueryOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler = &WorldSession::handleGameObjectQueryOpcode;
    //WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler = &WorldSession::HandlePageTextQueryOpcode;
    //WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler = &WorldSession::HandleItemNameQueryOpcode;
    //WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler = &WorldSession::HandleAchievmentQueryOpcode;

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
    WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    //WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler = &WorldSession::handleMovementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_PITCH_RATE_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;
    WorldPacketHandlers[CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK].handler = &WorldSession::handleAcknowledgementOpcodes;

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
    //WorldPacketHandlers[CMSG_AREATRIGGER].handler = &WorldSession::HandleAreaTriggerOpcode;

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
    WorldPacketHandlers[CMSG_PLAYED_TIME].handler = &WorldSession::handlePlayedTimeOpcode;
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
    WorldPacketHandlers[CMSG_ACTIVATETAXI].handler = &WorldSession::handleActivateTaxiOpcode;
    //WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler = &WorldSession::HandleTabardVendorActivateOpcode;
    WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler = &WorldSession::handleBankerActivateOpcode;
    WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler = &WorldSession::handleBuyBankSlotOpcode;
    WorldPacketHandlers[CMSG_TRAINER_LIST].handler = &WorldSession::handleTrainerListOpcode;
    WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler = &WorldSession::handleTrainerBuySpellOpcode;
    //WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler = &WorldSession::HandleCharterShowListOpcode;
    WorldPacketHandlers[MSG_AUCTION_HELLO].handler = &WorldSession::handleAuctionHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler = &WorldSession::handleGossipHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler = &WorldSession::handleGossipSelectOptionOpcode;
    WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler = &WorldSession::handleSpiritHealerActivateOpcode;
    WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler = &WorldSession::handleNpcTextQueryOpcode;
    WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler = &WorldSession::handleBinderActivateOpcode;
    //WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;

    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    //WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    //WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
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

    // Spell System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    WorldPacketHandlers[CMSG_SPELLCLICK].handler = &WorldSession::handleSpellClick;
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
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler = &WorldSession::handleAttackStopOpcode;
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
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
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
    WorldPacketHandlers[CMSG_SUGGESTION].handler = &WorldSession::handleSuggestionOpcode;
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
    //WorldPacketHandlers[CMSG_DISMISS_CONTROLLED_VEHICLE].handler = &WorldSession::HandleDismissVehicle;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_EXIT].handler = &WorldSession::HandleLeaveVehicle;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_PREV_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_NEXT_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_REQUEST_VEHICLE_SWITCH_SEAT].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE].handler = &WorldSession::HandleChangeVehicleSeat;
    //WorldPacketHandlers[CMSG_PLAYER_VEHICLE_ENTER].handler = &WorldSession::HandleEnterVehicle;
    //WorldPacketHandlers[CMSG_EJECT_PASSENGER].handler = &WorldSession::HandleRemoveVehiclePassenger;

    // Unsorted
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::HandleTimeSyncResOp;       //MiscHandler.cpp
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::HandleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::HandleSetAutoLootPassOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
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

    //Misc - Unhandled
    WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;

    WorldPacketHandlers[CMSG_ADDON_REGISTERED_PREFIXES].handler = &WorldSession::handleAddonRegisteredPrefixesOpcode;
    WorldPacketHandlers[CMSG_UNREGISTER_ALL_ADDON_PREFIXES].handler = &WorldSession::handleUnregisterAddonPrefixesOpcode;
}
#endif
