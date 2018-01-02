/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Spell/Definitions/PowerType.h"

#if VERSION_STRING != Cata
#include "Management/Guild.h"
#endif

OpcodeHandler WorldPacketHandlers[NUM_MSG_TYPES];

WorldSession::WorldSession(uint32 id, std::string Name, WorldSocket* sock) :
    m_loggingInPlayer(NULL),
    m_currMsTime(Util::getMSTime()),
    bDeleted(false),
    m_moveDelayTime(0),
    m_clientTimeDelay(0),
    m_bIsWLevelSet(false),
    _player(NULL),
    _socket(sock),
    _accountId(id),
    _accountName(Name),
    has_level_55_char(false),
    _side(-1),
    _logoutTime(0),
    permissions(NULL),
    permissioncount(0),
    _loggingOut(false),
    LoggingOut(false),
    instanceId(0),
    _updatecount(0),
    floodLines(0),
    floodTime(UNIXTIME),
    language(0),
    m_lastPing(0),
    m_wLevel(0),
    _accountFlags(0),
    has_dk(false),
    _latency(0),
    client_build(0),
    m_MoverGuid(0),
    m_muted(0)
{
    memset(movement_packet, 0, sizeof(movement_packet));

#if VERSION_STRING != Cata
    movement_info.redirectVelocity = 0;
#endif

    for (uint8 x = 0; x < 8; x++)
        sAccountData[x].data = NULL;
}

WorldSession::~WorldSession()
{
    deleteMutex.Acquire();

    if (_player)
    {
        LOG_ERROR("warning: logged out player in worldsession destructor");
        LogoutPlayer(true);
    }

    if (permissions)
        delete[]permissions;

    WorldPacket* packet;

    while ((packet = _recvQueue.Pop()) != 0)
        delete packet;

    for (uint32 x = 0; x < 8; x++)
    {
        if (sAccountData[x].data)
            delete[]sAccountData[x].data;
    }

    if (_socket)
        _socket->SetSession(0);

    if (m_loggingInPlayer)
        m_loggingInPlayer->SetSession(NULL);

    deleteMutex.Release();
}

uint8 WorldSession::Update(uint32 InstanceID)
{
    m_currMsTime = Util::getMSTime();

    if (!((++_updatecount) % 2) && _socket)
        _socket->UpdateQueuedPackets();

    WorldPacket* packet;
    OpcodeHandler* Handler;

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

    while ((packet = _recvQueue.Pop()) != 0)
    {
        ARCEMU_ASSERT(packet != NULL);

        if (packet->GetOpcode() >= NUM_MSG_TYPES)
        {
            LogDebugFlag(LF_OPCODE, "[Session] Received out of range packet with opcode 0x%.4X", packet->GetOpcode());
        }
        else
        {
            Handler = &WorldPacketHandlers[packet->GetOpcode()];
            if (Handler->status == STATUS_LOGGEDIN && !_player && Handler->handler != 0)
            {
                LogDebugFlag(LF_OPCODE, "[Session] Received unexpected/wrong state packet with opcode %s (0x%.4X)", getOpcodeName(packet->GetOpcode()).c_str(), packet->GetOpcode());
            }
            else
            {
                // Valid Packet :>
                if (Handler->handler == 0)
                {
                    LogDebugFlag(LF_OPCODE, "[Session] Received unhandled packet with opcode %s (0x%.4X)", getOpcodeName(packet->GetOpcode()).c_str(), packet->GetOpcode());
                }
                else
                {
                    (this->*Handler->handler)(*packet);
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

        if (_socket == NULL)
        {
            bDeleted = true;
            LogoutPlayer(true);
            return 1;
        }
        else
            LogoutPlayer(true);
    }

    if (m_lastPing + WORLDSOCKET_TIMEOUT < (uint32)UNIXTIME)
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
        if (_socket != NULL)
        {
            Disconnect();
            _socket = NULL;
        }

        m_lastPing = (uint32)UNIXTIME;	// Prevent calling this code over and
        // over.
        if (!_logoutTime)
            _logoutTime = m_currMsTime + PLAYER_LOGOUT_DELAY;
    }

    return 0;
}

void WorldSession::LogoutPlayer(bool Save)
{
    Player* pPlayer = GetPlayer();

    if (_loggingOut)
        return;

    _loggingOut = true;

    if (_player != NULL)
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
            if (obj != NULL)
            {
                switch (obj->GetTypeId())
                {
                    case TYPEID_UNIT:
                        static_cast <Creature*>(obj)->loot.looters.erase(_player->GetLowGUID());
                        break;
                    case TYPEID_GAMEOBJECT:
                        GameObject* go = static_cast<GameObject*>(obj);

                        if (!go->IsLootable())
                            break;

                        GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(go);
                        pLGO->loot.looters.erase(_player->GetLowGUID());

                        break;
                }
            }
        }

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
        GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(_player->GetGUID());
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
        sWorld.decrementPlayerCount(_player->GetTeam());

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
        else if (_player->IsDead() && _player->getDeathState() == JUST_DIED)
            _player->RepopRequestedPlayer();

        // Issue a message telling all guild members that this player signed
        // off
#if VERSION_STRING != Cata
        if (_player->IsInGuild())
        {
            Guild* pGuild = _player->m_playerInfo->guild;
            if (pGuild != NULL)
                pGuild->LogGuildEvent(GE_SIGNED_OFF, 1, _player->GetName());
        }
#endif

        _player->GetItemInterface()->EmptyBuyBack();
        _player->GetItemInterface()->removeLootableItems();

        // Save HP/Mana
        _player->load_health = _player->getUInt32Value(UNIT_FIELD_HEALTH);
        _player->load_mana = _player->GetPower(POWER_TYPE_MANA);


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

        _player->m_playerInfo->m_loggedInPlayer = NULL;

        if (_player->m_playerInfo->m_Group != NULL)
            _player->m_playerInfo->m_Group->Update();

        // Remove the "player locked" flag, to allow movement on next login
        GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

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
        _player = NULL;

        OutPacket(SMSG_LOGOUT_COMPLETE, 0, NULL);
        LOG_DEBUG("SESSION: Sent SMSG_LOGOUT_COMPLETE Message");
    }
    _loggingOut = false;

    SetLogoutTimer(0);
}

void WorldSession::SendBuyFailed(uint64 guid, uint32 itemid, uint8 error)
{
    WorldPacket data(13);
    data.SetOpcode(SMSG_BUY_FAILED);
    data << guid;
    data << itemid;
    data << error;
    SendPacket(&data);
}

void WorldSession::SendSellItem(uint64 vendorguid, uint64 itemid, uint8 error)
{
    WorldPacket data(17);
    data.SetOpcode(SMSG_SELL_ITEM);
    data << vendorguid;
    data << itemid;
    data << error;
    SendPacket(&data);
}

Player* WorldSession::GetPlayerOrThrow()
{
    Player* player = this->GetPlayer();
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
        c = (char)tolower(c);
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
    permissioncount = (uint32)tmp.size();
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
        WorldPacketHandlers[i].handler = 0;
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
            (unsigned int)session->GetAccountId(),
            session->GetAccountName().c_str(),
            session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "NOIP",
            session->GetPlayer() ? session->GetPlayer()->GetName() : "nologin");

        lenght = strlen(out);
        vsnprintf(&out[lenght], 32768 - lenght, format, ap);

        fprintf(mSessionLogFile, "%s\n", out);
        fflush(mSessionLogFile);
        va_end(ap);
    }
}

void WorldSession::SystemMessage(const char* format, ...)
{
    WorldPacket* data;
    char buffer[1024];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, 1024, format, ap);
    va_end(ap);

    data = sChatHandler.FillSystemMessageData(buffer);
    SendPacket(data);
    delete data;
}

void WorldSession::SendChatPacket(WorldPacket* data, uint32 langpos, int32 lang, WorldSession* originator)
{
    if (lang == -1)
        *(uint32*)& data->contents()[langpos] = lang;
    else
    {
        if (CanUseCommand('c') || (originator && originator->CanUseCommand('c')))
            *(uint32*)& data->contents()[langpos] = LANG_UNIVERSAL;
        else
            *(uint32*)& data->contents()[langpos] = lang;
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

#if VERSION_STRING == WotLK
void WorldSession::SendRefundInfo(uint64 GUID)
{
    if (!_player || !_player->IsInWorld())
        return;

    auto item = _player->GetItemInterface()->GetItemByGUID(GUID);
    if (item == nullptr)
        return;

    if (item->IsEligibleForRefund())
    {
        std::pair <time_t, uint32> RefundEntry;

        RefundEntry = _player->GetItemInterface()->LookupRefundable(GUID);

        if (RefundEntry.first == 0 || RefundEntry.second == 0)
            return;

        auto item_extended_cost = sItemExtendedCostStore.LookupEntry(RefundEntry.second);
        if (item_extended_cost == nullptr)
            return;

        ItemProperties const* proto = item->GetItemProperties();

        item->SetFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_REFUNDABLE);
        // ////////////////////////////////////////////////////////////////////////////////////////
        // As of 3.2.0a the server sends this packet to provide refund info on
        // an item
        //
        // {SERVER} Packet: (0x04B2) UNKNOWN PacketSize = 68 TimeStamp =
        // 265984265
        // E6 EE 09 18 02 00 00 42 00 00 00 00 4B 25 00 00 00 00 00 00 50 50
        // 00 00 0A 00 00 00 00
        // 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
        // 00 00 00 00 00 00 00
        // 00 00 00 00 00 00 D3 12 12 00
        //
        // Structure:
        // uint64 GUID
        // uint32 price (in copper)
        // uint32 honor
        // uint32 arena
        // uint32 item1
        // uint32 item1cnt
        // uint32 item2
        // uint32 item2cnt
        // uint32 item3
        // uint32 item3cnt
        // uint32 item4
        // uint32 item4cnt
        // uint32 item5
        // uint32 item5cnt
        // uint32 unknown - always seems 0
        // uint32 buytime - buytime in total playedtime seconds
        //
        //
        // Remainingtime:
        // Seems to be in playedtime format
        //
        //
        // ////////////////////////////////////////////////////////////////////////////////////////


        WorldPacket packet(SMSG_ITEMREFUNDINFO, 68);
        packet << uint64(GUID);
        packet << uint32(proto->BuyPrice);
        packet << uint32(item_extended_cost->honor_points);
        packet << uint32(item_extended_cost->arena_points);

        for (uint8 i = 0; i < 5; ++i)
        {
            packet << uint32(item_extended_cost->item[i]);
            packet << uint32(item_extended_cost->count[i]);
        }

        packet << uint32(0);	// always seems to be 0

        uint32* played = _player->GetPlayedtime();

        if (played[1] >(RefundEntry.first + 60 * 60 * 2))
            packet << uint32(0);
        else
            packet << uint32(RefundEntry.first);

        this->SendPacket(&packet);

        LOG_DEBUG("Sent SMSG_ITEMREFUNDINFO.");
    }
}
#endif

void WorldSession::SendAccountDataTimes(uint32 mask)
{
#if VERSION_STRING == TBC
    StackWorldPacket<128> data(SMSG_ACCOUNT_DATA_TIMES);

    MD5Hash md5hash;
    for (int i = 0; i < 8; ++i)
    {
        AccountDataEntry* acct_data = GetAccountData(i);

        if (!acct_data->data)
        {
            data << uint64(0) << uint64(0);
            continue;
        }
        md5hash.Initialize();
        md5hash.UpdateData((const uint8*)acct_data->data, acct_data->sz);
        md5hash.Finalize();

        data.Write(md5hash.GetDigest(), MD5_DIGEST_LENGTH);
    }
#else
    WorldPacket data(SMSG_ACCOUNT_DATA_TIMES, 4 + 1 + 4 + 8 * 4);	// changed in WotLK
    data << uint32(UNIXTIME);	// unix time of something
    data << uint8(1);
    data << uint32(mask);		// type mask
    for (uint8 i = 0; i < NUM_ACCOUNT_DATA_TYPES; ++i)
    {
        if (mask & (1 << i))
        {
            // data << uint32(GetAccountData(AccountDataType(i))->Time);
            // also unix time
            data << uint32(0);
        }
    }
#endif
    SendPacket(&data);
}

#if VERSION_STRING != Cata
void WorldSession::HandleLearnTalentOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint32 talent_id;
    uint32 requested_rank;
    uint32 unk;

    recv_data >> talent_id;
    recv_data >> requested_rank;
    recv_data >> unk;

    _player->LearnTalent(talent_id, requested_rank);
}

void WorldSession::HandleUnlearnTalents(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN
        uint32 price = GetPlayer()->CalcTalentResetCost(GetPlayer()->GetTalentResetTimes());
    if (!GetPlayer()->HasGold(price))
        return;

    GetPlayer()->SetTalentResetTimes(GetPlayer()->GetTalentResetTimes() + 1);
    GetPlayer()->ModGold(-(int32)price);
    GetPlayer()->Reset_Talents();
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint32 skill_line_id;
    uint32 points_remaining = _player->GetPrimaryProfessionPoints();
    recv_data >> skill_line_id;

    // Remove any spells within that line that the player has
    _player->RemoveSpellsFromLine(skill_line_id);

    // Finally, remove the skill line.
    _player->_RemoveSkillLine(skill_line_id);

    // added by Zack : This is probably wrong or already made elsewhere :
    // restore skill learnability
    if (points_remaining == _player->GetPrimaryProfessionPoints())
    {
        // we unlearned a skill so we enable a new one to be learned
        auto skill_line = sSkillLineStore.LookupEntry(skill_line_id);
        if (!skill_line)
            return;

        if (skill_line->type == SKILL_TYPE_PROFESSION && points_remaining < 2)
            _player->SetPrimaryProfessionPoints(points_remaining + 1);
    }
}

void WorldSession::HandleLearnMultipleTalentsOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN uint32 talentcount;
    uint32 talentid;
    uint32 rank;

    LOG_DEBUG("Recieved packet CMSG_LEARN_TALENTS_MULTIPLE.");

    // //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 0x04C1 CMSG_LEARN_TALENTS_MULTIPLE
    // As of 3.2.2.10550 the client sends this packet when clicking "learn" on
    // the talent interface (in preview talents mode)
    // This packet tells the server which talents to learn
    //
    // Structure:
    //
    // struct talentdata{
    // uint32 talentid; - unique numeric identifier of the talent (index of
    // talent.dbc)
    // uint32 talentrank; - rank of the talent
    // };
    //
    // uint32 talentcount; - number of talentid-rank pairs in the packet
    // talentdata[ talentcount ];
    //
    //
    // //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    recvPacket >> talentcount;

    for (uint32 i = 0; i < talentcount; ++i)
    {
        recvPacket >> talentid;
        recvPacket >> rank;

        _player->LearnTalent(talentid, rank, true);
    }
}
#endif

void WorldSession::SendMOTD()
{
    WorldPacket data(SMSG_MOTD, 50);
    data << uint32(0);
    uint32 linecount = 0;
    std::string str_motd = worldConfig.getMessageOfTheDay();
    std::string::size_type pos, nextpos;

    pos = 0;
    while ((nextpos = str_motd.find('@', pos)) != std::string::npos)
    {
        if (nextpos != pos)
        {
            data << str_motd.substr(pos, nextpos - pos);
            ++linecount;
        }
        pos = nextpos + 1;
    }
        if (pos < str_motd.length())
        {
            data << str_motd.substr(pos);
            ++linecount;
        }

        data.put(0, linecount);

        SendPacket(&data);
}

#if VERSION_STRING > TBC
void WorldSession::HandleEquipmentSetUse(WorldPacket& data)
{
    CHECK_INWORLD_RETURN LOG_DEBUG("Received CMSG_EQUIPMENT_SET_USE");

    WoWGuid GUID;
    int8 SrcBagID;
    uint8 SrcSlotID;
    uint8 result = 0;

    for (int8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        uint64 ItemGUID = 0;

        GUID.Clear();

        data >> GUID;
        data >> SrcBagID;
        data >> SrcSlotID;

        ItemGUID = GUID.GetOldGuid();

        // Let's see if we even have this item
        auto item = _player->GetItemInterface()->GetItemByGUID(ItemGUID);
        if (item == nullptr)
        {
            // Nope we don't probably WPE hack :/
            result = 1;
            continue;
        }

        int8 dstslot = i;
        int8 dstbag = static_cast<int8>(INVALID_BACKPACK_SLOT);

        // This is the best case, we already have the item equipped
        if ((SrcBagID == dstbag) && (SrcSlotID == dstslot))
            continue;

        // Let's see if we have an item in the destination slot
        auto dstslotitem = _player->GetItemInterface()->GetInventoryItem(dstslot);

        if (dstslotitem == nullptr)
        {
            // we have no item equipped in the slot, so let's equip
            AddItemResult additemresult;
            int8 EquipError = _player->GetItemInterface()->CanEquipItemInSlot(dstbag, dstslot, item->GetItemProperties(), false, false);
            if (EquipError == INV_ERR_OK)
            {
                dstslotitem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(SrcBagID, SrcSlotID, false);
                additemresult = _player->GetItemInterface()->SafeAddItem(item, dstbag, dstslot);

                if (additemresult != ADD_ITEM_RESULT_OK)
                {
                    // We failed for w/e reason, so let's revert
                    auto check = _player->GetItemInterface()->SafeAddItem(item, SrcBagID, SrcSlotID);
                    if (!check)
                    {
                        LOG_ERROR("HandleEquipmentSetUse", "Error while adding item %u to player %s twice", item->GetEntry(), _player->GetNameString());
                        result = 0;
                    }
                    else
                        result = 1;
                }
            }
            else
            {
                result = 1;
            }

        }
        else
        {
            // There is something equipped so we need to swap
            if (!_player->GetItemInterface()->SwapItems(INVALID_BACKPACK_SLOT, dstslot, SrcBagID, SrcSlotID))
                result = 1;
        }

    }

    _player->SendEquipmentSetUseResult(result);
}

void WorldSession::HandleEquipmentSetSave(WorldPacket& data)
{
    CHECK_INWORLD_RETURN LOG_DEBUG("Received CMSG_EQUIPMENT_SET_SAVE");

    WoWGuid GUID;
    uint32 setGUID;

    data >> GUID;

    setGUID = Arcemu::Util::GUID_LOPART(GUID.GetOldGuid());

    if (setGUID == 0)
        setGUID = objmgr.GenerateEquipmentSetID();

    Arcemu::EquipmentSet* set = new Arcemu::EquipmentSet();

    set->SetGUID = setGUID;

    data >> set->SetID;
    data >> set->SetName;
    data >> set->IconName;

    for (uint32 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        GUID.Clear();
        data >> GUID;
        set->ItemGUID[i] = Arcemu::Util::GUID_LOPART(GUID.GetOldGuid());
    }

    bool success;
    success = _player->GetItemInterface()->m_EquipmentSets.AddEquipmentSet(set->SetGUID, set);

    if (success)
    {
        LOG_DEBUG("Player %u successfully stored equipment set %u at slot %u ", _player->GetLowGUID(), set->SetGUID, set->SetID);
        _player->SendEquipmentSetSaved(set->SetID, set->SetGUID);
    }
    else
    {
        LOG_DEBUG("Player %u couldn't store equipment set %u at slot %u ", _player->GetLowGUID(), set->SetGUID, set->SetID);
    }
}

void WorldSession::HandleEquipmentSetDelete(WorldPacket& data)
{
    CHECK_INWORLD_RETURN LOG_DEBUG("Received CMSG_EQUIPMENT_SET_DELETE");

    WoWGuid setGUID;
    bool success;

    data >> setGUID;

    uint32 GUID = Arcemu::Util::GUID_LOPART(setGUID.GetOldGuid());

    success = _player->GetItemInterface()->m_EquipmentSets.DeleteEquipmentSet(GUID);

    if (success)
    {
        LOG_DEBUG("Equipmentset with GUID %u was successfully deleted.", GUID);
    }
    else
    {
        LOG_DEBUG("Equipmentset with GUID %u couldn't be deleted.", GUID);
    }

}
#endif

#if VERSION_STRING == WotLK
void WorldSession::HandleQuestPOIQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN LOG_DEBUG("Received CMSG_QUEST_POI_QUERY");

    uint32 count = 0;
    recv_data >> count;

    if (count > MAX_QUEST_LOG_SIZE)
    {
        LOG_DEBUG
            ("Client sent Quest POI query for more than MAX_QUEST_LOG_SIZE quests.");

        count = MAX_QUEST_LOG_SIZE;
    }

    WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4 + (4 + 4) * count);

    data << uint32(count);

    for (uint32 i = 0; i < count; i++)
    {
        uint32 questId;
        recv_data >> questId;

        sQuestMgr.BuildQuestPOIResponse(data, questId);
    }

    SendPacket(&data);

    LOG_DEBUG("Sent SMSG_QUEST_POI_QUERY_RESPONSE");
}
#endif

void WorldSession::HandleMirrorImageOpcode(WorldPacket& recv_data)
{
    if (!_player->IsInWorld())
        return;

    LOG_DEBUG("Received CMG_GET_MIRRORIMAGE_DATA");

    uint64 GUID;

    recv_data >> GUID;

    Unit* Image = _player->GetMapMgr()->GetUnit(GUID);
    if (Image == NULL)
        return;					// ups no unit found with that GUID on the map. Spoofed packet?

    if (Image->GetCreatedByGUID() == 0)
        return;

    uint64 CasterGUID = Image->GetCreatedByGUID();
    Unit* Caster = _player->GetMapMgr()->GetUnit(CasterGUID);

    if (Caster == NULL)
        return;					// apperantly this mirror image mirrors nothing, poor lonely soul :(Maybe it's the Caster's ghost called Casper

    WorldPacket data(SMSG_MIRRORIMAGE_DATA, 68);

    data << uint64(GUID);
    data << uint32(Caster->GetDisplayId());
    data << uint8(Caster->getRace());

    if (Caster->IsPlayer())
    {
        Player* pcaster = static_cast <Player*>(Caster);

        data << uint8(pcaster->getGender());
        data << uint8(pcaster->getClass());

        // facial features, like big nose, piercings, bonehead, etc
        data << uint8(pcaster->getByteValue(PLAYER_BYTES, 0));	// skin color
        data << uint8(pcaster->getByteValue(PLAYER_BYTES, 1));	// face
        data << uint8(pcaster->getByteValue(PLAYER_BYTES, 2));	// hair style
        data << uint8(pcaster->getByteValue(PLAYER_BYTES, 3));	// hair color
        data << uint8(pcaster->getByteValue(PLAYER_BYTES_2, 0));	// facial hair

#if VERSION_STRING != Cata
        if (pcaster->IsInGuild())
            data << uint32(pcaster->GetGuildId());
        else
            data << uint32(0);
#else
        if (pcaster->GetGuild())
            data << uint32(pcaster->GetGuildId());
        else
            data << uint32(0);
#endif

        static const uint32 imageitemslots[] =
        {
            EQUIPMENT_SLOT_HEAD,
            EQUIPMENT_SLOT_SHOULDERS,
            EQUIPMENT_SLOT_BODY,
            EQUIPMENT_SLOT_CHEST,
            EQUIPMENT_SLOT_WAIST,
            EQUIPMENT_SLOT_LEGS,
            EQUIPMENT_SLOT_FEET,
            EQUIPMENT_SLOT_WRISTS,
            EQUIPMENT_SLOT_HANDS,
            EQUIPMENT_SLOT_BACK,
            EQUIPMENT_SLOT_TABARD
        };

        for (uint8 i = 0; i < 11; ++i)
        {
            Item* item = pcaster->GetItemInterface()->GetInventoryItem(static_cast <int16> (imageitemslots[i]));
            if (item != nullptr)
                data << uint32(item->GetItemProperties()->DisplayInfoID);
            else
                data << uint32(0);
        }
    }
    else // do not send player data for creatures
    {
        data << uint8(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
    }

    SendPacket(&data);

    LOG_DEBUG("Sent: SMSG_MIRRORIMAGE_DATA");
}

void WorldSession::Unhandled(WorldPacket& recv_data)
{
    recv_data.rfinish();
}

void WorldSession::nothingToHandle(WorldPacket& recv_data)
{
    if (recv_data.isEmpty() == false)
    {
        LogDebugFlag(LF_OPCODE, "Opcode %s (0x%.4X) received. Apply nothingToHandle handler but size is %u!", getOpcodeName(recv_data.GetOpcode()).c_str(), recv_data.GetOpcode(), recv_data.size());
    }
}

void WorldSession::HandleDismissCritter(WorldPacket& recv_data)
{
    uint64 GUID;

    recv_data >> GUID;

    if (_player->GetSummonedCritterGUID() == 0)
    {
        LOG_ERROR("Player %u sent dismiss companion packet, but player has no companion", _player->GetLowGUID());
        return;
    }

    if (_player->GetSummonedCritterGUID() != GUID)
    {
        LOG_ERROR("Player %u sent dismiss companion packet, but it doesn't match player's companion", _player->GetLowGUID());
        return;
    }

    Unit* companion = _player->GetMapMgr()->GetUnit(GUID);

    if (companion != NULL)
    {
        companion->Delete();
    }

    _player->SetSummonedCritterGUID(0);
}

#if VERSION_STRING > TBC
void WorldSession::SendClientCacheVersion(uint32 version)
{
    WorldPacket data(SMSG_CLIENTCACHE_VERSION, 4);
    data << uint32(version);
    SendPacket(&data);
}
#endif

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
        _socket->OutPacket(opcode, 0, NULL);
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
    m_lastPing = (uint32)UNIXTIME;
    _recvQueue.Push(packet);
}

void WorldSession::Disconnect()
{
    if (_socket && _socket->IsConnected())
    {
        _socket->Disconnect();
    }
}