/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"

void WorldSession::HandleRequestRatedBgInfoOpcode(WorldPacket & recv_data)
{
    uint8_t unk_type;
    recv_data >> unk_type;

    LOG_DEBUG("CMSG_REQUEST_RATED_BG_INFO received with unk_type = %u", unk_type);

    WorldPacket data(SMSG_RATED_BG_INFO, 72);
    for (int i = 0; i < 18; ++i)
    {
        data << uint32_t(0);    // unknown
    }

    SendPacket(&data);
}

void WorldSession::HandleRequestRatedBgStatsOpcode(WorldPacket& /*recv_data*/)
{
    LOG_DEBUG("CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket data(SMSG_RATED_BG_STATS, 29);
    data << uint32_t(0);    // unknown
    data << uint8_t(3);     // unknown - always 3?... type?
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown

    SendPacket(&data);
}

void WorldSession::HandleRequestPvPRewardsOpcode(WorldPacket& /*recv_data*/)
{
    LOG_DEBUG("CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket packet(SMSG_REQUEST_PVP_REWARDS_RESPONSE, 24);
    packet << uint32_t(0);    // unknown currency week cap conquest points
    packet << uint32_t(0);    // unknown currency on week conquest points
    packet << uint32_t(0);    // unknown currency week cap conquest arena
    packet << uint32_t(0);    // unknown currency on week conquest random baattleground
    packet << uint32_t(0);    // unknown currency on week conquest arena
    packet << uint32_t(0);    // unknown currency week cap conquest points

    SendPacket(&packet);
}

void WorldSession::HandleRequestPvpOptionsOpcode(WorldPacket& /*recv_data*/)
{
    LOG_DEBUG("CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket data(SMSG_PVP_OPTIONS_ENABLED, 1);
    data.writeBit(1);       // unknown 
    data.writeBit(1);       // unknown wargames enabled
    data.writeBit(1);       // unknown 
    data.writeBit(1);       // unknown rated battlegrounds enabled
    data.writeBit(1);       // unknown rated arenas enabled

    data.flushBits();

    SendPacket(&data);
}

//\todo Rewrite for cata - after this all functions are copied from wotlk

void WorldSession::HandleBattlefieldPortOpcode(WorldPacket& recvData)
{
    uint16_t mapinfo;
    uint16_t unk;
    uint8_t action;
    uint32_t bgtype;

    recvData >> unk;
    recvData >> bgtype;
    recvData >> mapinfo;
    recvData >> action;

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

void WorldSession::HandleBattlefieldListOpcode(WorldPacket& recvData)
{
    uint32_t BGType;
    recvData >> BGType;
    uint8_t from;
    recvData >> from; // 0 - battlemaster, 1 - whatever

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
            if (battlemaster != nullptr)
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

void WorldSession::HandleLeaveBattlefieldOpcode(WorldPacket& /*recv_data*/)
{
    if (_player->m_bg && _player->IsInWorld())
        _player->m_bg->RemovePlayer(_player, false);
}

void WorldSession::HandleReadyForAccountDataTimes(WorldPacket& /*recvData*/)
{
    LogDebugFlag(LF_OPCODE, "WORLD: CMSG_READY_FOR_ACCOUNT_DATA_TIMES");

    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::HandleAreaSpiritHealerQueryOpcode(WorldPacket& recvData)
{
    if (!_player->m_bg)
        return;

    uint64_t guid;
    recvData >> guid;

    Creature* psg = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (psg == nullptr)
        return;

    uint32_t restime = _player->m_bg->GetLastResurrect() + 30;
    if ((uint32)UNIXTIME > restime)
        restime = 1000;
    else
        restime = (restime - static_cast<uint32_t>(UNIXTIME)) * 1000;

    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    data << guid;
    data << restime;
    SendPacket(&data);
}

void WorldSession::HandleAreaSpiritHealerQueueOpcode(WorldPacket& recvData)
{
    if (!_player->m_bg)
        return;

    uint64_t guid;
    recvData >> guid;
    Creature* psg = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (psg == nullptr)
        return;

    _player->m_bg->QueuePlayerForResurrect(_player, psg);
    _player->CastSpell(_player, 2584, true);
}

void WorldSession::HandleBattlegroundPlayerPositionsOpcode(WorldPacket& /*recv_data*/)
{
    // empty opcode
    CBattleground* bg = _player->m_bg;

    if (!bg)
        return;

    uint32_t count1 = 0;
    uint32_t count2 = 0;

    Player* ap = objmgr.GetPlayer(static_cast<uint32_t>(bg->GetFlagHolderGUID(TEAM_ALLIANCE)));
    if (ap != nullptr)
        count2++;

    Player* hp = objmgr.GetPlayer(static_cast<uint32_t>(bg->GetFlagHolderGUID(TEAM_HORDE)));

    // If the two are the same, then it's from a Bg that only has 1 flag like EOTS
    if ((ap != nullptr) &&
        (hp != nullptr) &&
        (ap->GetGUID() == hp->GetGUID()))
        hp = nullptr;

    if (hp != nullptr)
        count2++;

    WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4 + 4 + 16 * count1 + 16 * count2));
    data << uint32(count1);
    data << uint32(count2);

    if (ap != nullptr)
    {
        data << uint64(ap->GetGUID());
        data << float(ap->GetPositionX());
        data << float(ap->GetPositionY());
    }

    if (hp != nullptr)
    {
        data << uint64(hp->GetGUID());
        data << float(hp->GetPositionX());
        data << float(hp->GetPositionY());
    }

    SendPacket(&data);
}

void WorldSession::HandleBattleMasterJoinOpcode(WorldPacket& recvData)
{
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
        BattlegroundManager.HandleBattlegroundJoin(this, recvData);
}

void WorldSession::HandleArenaJoinOpcode(WorldPacket& recvData)
{
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

    recvData >> guid;
    recvData >> arenacategory;
    recvData >> as_group;
    recvData >> rated_match;

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

void WorldSession::HandleInspectHonorStatsOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 8);
    CHECK_INWORLD_RETURN;

    uint64_t guid;
    recvData >> guid;

    if (_player->GetMapMgr() == nullptr)
    {
        LOG_ERROR("HandleInspectHonorStatsOpcode : _player map mgr was null");
        return;
    }

    if (_player->GetMapMgr()->GetPlayer((uint32)guid) == nullptr)
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
#endif
    data << player->getUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);

    SendPacket(&data);
}

void WorldSession::HandleInspectArenaStatsOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 8);

    uint64_t guid;
    recvData >> guid;

    Player* player = _player->GetMapMgr()->GetPlayer(static_cast<uint32_t>(guid));
    if (player == nullptr)
    {
        LOG_ERROR("HandleInspectHonorStatsOpcode : guid was null");
        return;
    }

    for (uint8_t i = 0; i < 3; i++)
    {
        uint32_t id = player->getUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (i * 7));
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
}

void WorldSession::HandlePVPLogDataOpcode(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN;
    if (_player->m_bg)
        _player->m_bg->SendPVPData(_player);
}

void WorldSession::SendNotInArenaTeamPacket(uint8_t type)
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
