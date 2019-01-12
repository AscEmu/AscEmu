/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

using namespace AscEmu::Packets;

OpcodeHandler WorldPacketHandlers[NUM_MSG_TYPES];

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

#if VERSION_STRING < Cata
    movement_info.redirect_velocity = 0;
#endif

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

        if (packet->GetOpcode() >= NUM_MSG_TYPES)
        {
            LogDebugFlag(LF_OPCODE, "[Session] Received out of range packet with opcode 0x%.4X", packet->GetOpcode());
        }
        else
        {
            OpcodeHandler* handler = &WorldPacketHandlers[packet->GetOpcode()];
            if (handler->status == STATUS_LOGGEDIN && !_player && handler->handler != 0)
            {
                LogDebugFlag(LF_OPCODE, "[Session] Received unexpected/wrong state packet with opcode %s (0x%.4X)", getOpcodeName(packet->GetOpcode()).c_str(), packet->GetOpcode());
            }
            else
            {
                // Valid Packet :>
                if (handler->handler == 0)
                {
                    LogDebugFlag(LF_OPCODE, "[Session] Received unhandled packet with opcode %s (0x%.4X)", getOpcodeName(packet->GetOpcode()).c_str(), packet->GetOpcode());
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

        m_lastPing = static_cast<uint32>(UNIXTIME);	// Prevent calling this code over and
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

        objmgr.RemovePlayer(_player);
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
                        dynamic_cast<Creature*>(obj)->loot.looters.erase(_player->getGuidLow());
                        break;
                    case TYPEID_GAMEOBJECT:
                        GameObject* go = dynamic_cast<GameObject*>(obj);

                        if (!go->IsLootable())
                            break;

                        GameObject_Lootable* pLGO = dynamic_cast<GameObject_Lootable*>(go);
                        pLGO->loot.looters.erase(_player->getGuidLow());

                        break;
                }
            }
        }

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
        GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(_player->getGuid());
        if (ticket != NULL)
        {
            // Send status change to gm_sync_channel
            Channel* chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), _player);
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

        _player->Social_TellFriendsOffline();

        // Decrement the global player count
        sWorld.decrementPlayerCount(_player->getTeam());

        if (_player->m_bgIsQueued)
            BattlegroundManager.RemovePlayerFromQueues(_player);

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


        _player->summonhandler.RemoveAllSummons();

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

        OutPacket(SMSG_LOGOUT_COMPLETE, 0, nullptr);
        LOG_DEBUG("SESSION: Sent SMSG_LOGOUT_COMPLETE Message");
    }
    _loggingOut = false;

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
            c = 'a';			// for the lazy people

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
    if (permissions[0] == 'a' && cmdstr != 'z')	// all
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

    WorldPacket data(SMSG_NOTIFICATION, strlen(msg1) + 1);
    data << msg1;
    SendPacket(&data);
}

void WorldSession::InitPacketHandlerTable()
{
    // Nullify Everything, default to STATUS_LOGGEDIN
    for (uint32 i = 0; i < NUM_MSG_TYPES; ++i)
    {
        WorldPacketHandlers[i].status = STATUS_LOGGEDIN;
        WorldPacketHandlers[i].handler = nullptr;
    }

    loadSpecificHandlers();
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
        LogDebugFlag(LF_OPCODE, "Opcode %s (0x%.4X) received. Apply nothingToHandle handler but size is %u!",
            getOpcodeName(recv_data.GetOpcode()).c_str(), recv_data.GetOpcode(), recv_data.size());
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

void WorldSession::SendPacket(StackBufferBase* packet)
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
