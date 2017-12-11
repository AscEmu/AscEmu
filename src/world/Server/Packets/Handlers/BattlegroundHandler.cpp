/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
#include "Management/Battleground/Battleground.h"
#include "Management/ArenaTeam.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/MapMgr.h"

#if VERSION_STRING != Cata
void WorldSession::HandleBattlefieldPortOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint16 mapinfo;
    uint16 unk;
    uint8 action;
    uint32 bgtype;

    recv_data >> unk;
    recv_data >> bgtype;
    recv_data >> mapinfo;
    recv_data >> action;

    if (action == 0)
    {
        BattlegroundManager.RemovePlayerFromQueues(_player);
    }
    else
    {
        /**********************************************************************************
         * Usually the fields in the packet would've been used to check what instance we're porting into, however since we're not
         * doing "queue multiple battleground types at once" we can just use our cached pointer in the player class. - Burlex
         **********************************************************************************/

        if (_player->m_pendingBattleground)
            _player->m_pendingBattleground->PortPlayer(_player);
    }
}

void WorldSession::HandleBattlefieldStatusOpcode(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

        /**********************************************************************************
        * This is done based on whether we are queued, inside, or not in a battleground.
        ***********************************************************************************/
        if (_player->m_pendingBattleground)        // Ready to port
            BattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_READY, _player->m_pendingBattleground->GetType(), _player->m_pendingBattleground->GetId(), 120000, 0, _player->m_pendingBattleground->Rated());
        else if (_player->m_bg)                    // Inside a bg
            BattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_TIME, _player->m_bg->GetType(), _player->m_bg->GetId(), (uint32)UNIXTIME - _player->m_bg->GetStartTime(), _player->GetMapId(), _player->m_bg->Rated());
        else                                    // None
            BattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
}

void WorldSession::HandleBattlefieldListOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint32 BGType;
    recv_data >> BGType;
    uint8 from;
    recv_data >> from; // 0 - battlemaster, 1 - whatever

    // weeeeeeeeeeeeeeeeeeeee
    CHECK_INWORLD_ASSERT;

    BattlegroundManager.HandleBattlegroundListPacket(this, BGType, from);
}

void WorldSession::SendBattlegroundList(Creature* pCreature, uint32 mapid)
{
    if (!pCreature)
        return;

    /**********************************************************************************
    * uint32 t = BattleGroundType
    **********************************************************************************/
    uint32 t = BATTLEGROUND_WARSONG_GULCH;
    if (mapid == 0)
    {
        if (pCreature->GetCreatureProperties()->SubName.compare("Arena") != 0)
        {
            t = BATTLEGROUND_ARENA_2V2;
        }
        else
        {
            MySQLStructure::Battlemasters const* battlemaster = sMySQLStore.getBattleMaster(pCreature->GetCreatureProperties()->Id);
            if (battlemaster != NULL)
            {
                t = battlemaster->battlegroundId;
            }
        }
    }
    else
    {
        t = mapid;
    }

    BattlegroundManager.HandleBattlegroundListPacket(this, t);
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleBattleMasterHelloOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    CHECK_INWORLD_RETURN;

    uint64 guid;
    recv_data >> guid;
    LOG_DEBUG("Received CMSG_BATTLEMASTER_HELLO from " I64FMT, guid);

    Creature* bm = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));

    if (!bm)
        return;

    if (!bm->isBattleMaster())        // Not a Battlemaster
        return;

    SendBattlegroundList(bm, 0);
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandleLeaveBattlefieldOpcode(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

        if (_player->m_bg && _player->IsInWorld())
            _player->m_bg->RemovePlayer(_player, false);
}

void WorldSession::HandleReadyForAccountDataTimes(WorldPacket& /*recvData*/)
{
    LogDebugFlag(LF_OPCODE, "WORLD: CMSG_READY_FOR_ACCOUNT_DATA_TIMES");

    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::HandleAreaSpiritHealerQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        if (!_player->m_bg)
            return;

    uint64 guid;
    recv_data >> guid;

    Creature* psg = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (psg == NULL)
        return;

    uint32 restime = _player->m_bg->GetLastResurrect() + 30;
    if ((uint32)UNIXTIME > restime)
        restime = 1000;
    else
        restime = (restime - (uint32)UNIXTIME) * 1000;

    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    data << guid;
    data << restime;
    SendPacket(&data);
}

void WorldSession::HandleAreaSpiritHealerQueueOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        if (!_player->m_bg)
            return;

    uint64 guid;
    recv_data >> guid;
    Creature* psg = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (psg == NULL)
        return;

    _player->m_bg->QueuePlayerForResurrect(_player, psg);
    _player->CastSpell(_player, 2584, true);
}

void WorldSession::HandleBattlegroundPlayerPositionsOpcode(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

        // empty opcode
        CBattleground* bg = _player->m_bg;
    if (!bg)
        return;

    uint32 count1 = 0;
    uint32 count2 = 0;

    Player* ap = objmgr.GetPlayer(static_cast<uint32>(bg->GetFlagHolderGUID(TEAM_ALLIANCE)));
    if (ap != NULL)
        count2++;

    Player* hp = objmgr.GetPlayer(static_cast<uint32>(bg->GetFlagHolderGUID(TEAM_HORDE)));

    // If the two are the same, then it's from a Bg that only has 1 flag like EOTS
    if ((ap != NULL) &&
        (hp != NULL) &&
        (ap->GetGUID() == hp->GetGUID()))
        hp = NULL;

    if (hp != NULL)
        count2++;

    WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4 + 4 + 16 * count1 + 16 * count2));
    data << uint32(count1);
    data << uint32(count2);

    if (ap != NULL)
    {
        data << uint64(ap->GetGUID());
        data << float(ap->GetPositionX());
        data << float(ap->GetPositionY());
    }

    if (hp != NULL)
    {
        data << uint64(hp->GetGUID());
        data << float(hp->GetPositionX());
        data << float(hp->GetPositionY());
    }

    SendPacket(&data);
}

void WorldSession::HandleBattleMasterJoinOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN;

    if (_player->HasAura(BG_DESERTER))
    {
        WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
        data << (uint32)0xFFFFFFFE;
        _player->GetSession()->SendPacket(&data);
        return;
    }

    if (_player->GetGroup() && _player->GetGroup()->m_isqueued)
    {
        SystemMessage("You are in a group that is already queued for a battleground or inside a battleground. Leave this first.");
        return;
    }

    /* are we already in a queue? */
    if (_player->m_bgIsQueued)
        BattlegroundManager.RemovePlayerFromQueues(_player);

    if (_player->IsInWorld())
        BattlegroundManager.HandleBattlegroundJoin(this, recv_data);
}

void WorldSession::HandleArenaJoinOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN;

    if (_player->GetGroup() && _player->GetGroup()->m_isqueued)
    {
        SystemMessage("You are in a group that is already queued for a battleground or inside a battleground. Leave this first.");
        return;
    }

    /* are we already in a queue? */
    if (_player->m_bgIsQueued)
        BattlegroundManager.RemovePlayerFromQueues(_player);

    uint32 bgtype = 0;
    uint64 guid;
    uint8 arenacategory;
    uint8 as_group;
    uint8 rated_match;

    recv_data >> guid;
    recv_data >> arenacategory;
    recv_data >> as_group;
    recv_data >> rated_match;

    switch (arenacategory)
    {
        case 0:        // 2v2
            bgtype = BATTLEGROUND_ARENA_2V2;
            break;

        case 1:        // 3v3
            bgtype = BATTLEGROUND_ARENA_3V3;
            break;

        case 2:        // 5v5
            bgtype = BATTLEGROUND_ARENA_5V5;
            break;
    }

    if (bgtype != 0)
        BattlegroundManager.HandleArenaJoin(this, bgtype, as_group, rated_match);
}

void WorldSession::HandleInspectHonorStatsOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);
    CHECK_INWORLD_RETURN;

    uint64 guid;
    recv_data >> guid;

    if (_player->GetMapMgr() == NULL)
    {
        LOG_ERROR("HandleInspectHonorStatsOpcode : _player map mgr was null");
        return;
    }

    if (_player->GetMapMgr()->GetPlayer((uint32)guid) == NULL)
    {
        LOG_ERROR("HandleInspectHonorStatsOpcode : guid was null");
        return;
    }

    Player* player = _player->GetMapMgr()->GetPlayer((uint32)guid);

    WorldPacket data(MSG_INSPECT_HONOR_STATS, 13);

    data << player->GetGUID();
    data << uint8(player->GetHonorCurrency());
#if VERSION_STRING != Classic
    data << player->getUInt32Value(PLAYER_FIELD_KILLS);
#if VERSION_STRING != Cata
    data << player->getUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION);
    data << player->getUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION);
#endif
#endif
    data << player->getUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);

    SendPacket(&data);
}

void WorldSession::HandleInspectArenaStatsOpcode(WorldPacket& recv_data)
{
#if VERSION_STRING != Classic
    CHECK_PACKET_SIZE(recv_data, 8);
    CHECK_INWORLD_RETURN;

    uint64 guid;
    recv_data >> guid;

    Player* player = _player->GetMapMgr()->GetPlayer((uint32)guid);
    if (player == nullptr)
    {
        LOG_ERROR("HandleInspectHonorStatsOpcode : guid was null");
        return;
    }

    for (uint8 i = 0; i < 3; i++)
    {
        uint32 id = player->getUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (i * 7));
        if (id > 0)
        {
            ArenaTeam* team = objmgr.GetArenaTeamById(id);
            if (team != nullptr)
            {
                WorldPacket data(MSG_INSPECT_ARENA_TEAMS, 8 + 1 + 4 * 5);
                data << player->GetGUID();
                data << team->m_type;
                data << team->m_id;
                data << team->m_stat_rating;
                data << team->m_stat_gamesplayedweek;
                data << team->m_stat_gameswonweek;
                data << team->m_stat_gamesplayedseason;
                SendPacket(&data);

            }
        }
    }
#endif
}

void WorldSession::HandlePVPLogDataOpcode(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN;
    if (_player->m_bg)
        _player->m_bg->SendPVPData(_player);
}

void WorldSession::SendNotInArenaTeamPacket(uint8 type)
{
    WorldPacket data(SMSG_ARENA_ERROR, 4 + 1); // 886 - You are not in a %uv%u arena team
    data << uint32(0);                       // E_ERR_ARENA_NO_TEAM_II (1 = E_ERR_ARENA_EXPIRED_CAIS)
    data << uint8(type);                     // team type (2=2v2,3=3v3,5=5v5), can be used for custom types...
    SendPacket(&data);
}

void WorldSession::HandleBgInviteResponse(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "Recieved unknown packet: CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE");

    // uint32 ?
    // uint8  ?
}
#endif
