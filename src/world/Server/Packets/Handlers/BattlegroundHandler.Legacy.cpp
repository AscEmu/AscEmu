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
#include "Management/Battleground/Battleground.h"
#include "Management/ArenaTeam.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/MapMgr.h"


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
