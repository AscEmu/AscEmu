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
 *
 */

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Server/WorldSession.h"
#include "Log.hpp"
#include "Units/Players/Player.h"

#if VERSION_STRING != Cata
void WorldSession::HandleAttackSwingOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 8);
    uint64_t guid;
    recvData >> guid;

    if (!guid)
    {
        // does this mean cancel combat?
        HandleAttackStopOpcode(recvData);
        return;
    }

    // AttackSwing
    LogDebugFlag(LF_OPCODE, "Recvd CMSG_ATTACKSWING Message");

    if (GetPlayer()->IsPacified() || GetPlayer()->IsStunned() || GetPlayer()->IsFeared())
        return;

    Unit* pEnemy = _player->GetMapMgr()->GetUnit(guid);

    if (!pEnemy)
    {
        LOG_DEBUG("WORLD: " I64FMT " does not exist.", guid);
        return;
    }

    if (pEnemy->IsDead() || _player->IsDead() || !isAttackable(_player, pEnemy, false))        // haxors :(
        return;

    GetPlayer()->smsg_AttackStart(pEnemy);
    GetPlayer()->EventAttackStart();
}

void WorldSession::HandleAttackStopOpcode(WorldPacket& /*recv_data*/)
{
    uint64_t guid = GetPlayer()->GetSelection();

    if (guid)
    {
        Unit* pEnemy = _player->GetMapMgr()->GetUnit(guid);
        if (pEnemy != nullptr)
        {
            GetPlayer()->EventAttackStop();
            GetPlayer()->smsg_AttackStop(pEnemy);
        }
    }
}
#endif
