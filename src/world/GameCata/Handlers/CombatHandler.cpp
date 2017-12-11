/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Server/WorldSession.h"
#include "Log.hpp"
#include "Units/Players/Player.h"

//\todo Rewrite for cata - after this all functions are copied from wotlk

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