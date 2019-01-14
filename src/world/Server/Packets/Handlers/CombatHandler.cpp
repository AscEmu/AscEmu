/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgAttackSwing.h"
#include "Server/WorldSession.h"
#include "Units/Players/Player.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"

using namespace AscEmu::Packets;

void WorldSession::handleAttackSwingOpcode(WorldPacket& recvPacket)
{
    CmsgAttackSwing srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_ATTACKSWING: %u (guidLow)", srlPacket.guid.getGuidLow());

    if (_player->IsFeared() || _player->IsStunned() || _player->IsPacified() || _player->isDead())
        return;

    const auto unitTarget = _player->GetMapMgr()->GetUnit(srlPacket.guid.GetOldGuid());
    if (unitTarget == nullptr)
        return;

    if (!isAttackable(_player, unitTarget, false) || unitTarget->isDead())
        return;

    _player->smsg_AttackStart(unitTarget);
    _player->EventAttackStart();
}

void WorldSession::handleAttackStopOpcode(WorldPacket& /*recvPacket*/)
{
    const auto unitTarget = _player->GetMapMgr()->GetUnit(_player->GetSelection());
    if (unitTarget == nullptr)
        return;

    _player->EventAttackStop();
    _player->smsg_AttackStop(unitTarget);
}
