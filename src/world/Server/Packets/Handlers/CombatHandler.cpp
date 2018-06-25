/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgAttackSwing.h"

using namespace AscEmu::Packets;

void WorldSession::handleAttackSwingOpcode(WorldPacket& recvPacket)
{
    CmsgAttackSwing recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_ATTACKSWING: %u (guidLow)", recv_packet.guid.getGuidLow());

    if (GetPlayer()->IsFeared() || GetPlayer()->IsStunned() || GetPlayer()->IsPacified() || GetPlayer()->IsDead())
        return;

    const auto unitTarget = GetPlayer()->GetMapMgr()->GetUnit(recv_packet.guid.GetOldGuid());
    if (unitTarget == nullptr)
        return;

    if (!isAttackable(GetPlayer(), unitTarget, false) || unitTarget->IsDead())
        return;

    GetPlayer()->smsg_AttackStart(unitTarget);
    GetPlayer()->EventAttackStart();
}

void WorldSession::handleAttackStopOpcode(WorldPacket& /*recvPacket*/)
{
    const auto unitTarget = GetPlayer()->GetMapMgr()->GetUnit(GetPlayer()->GetSelection());
    if (unitTarget == nullptr)
        return;

    GetPlayer()->EventAttackStop();
    GetPlayer()->smsg_AttackStop(unitTarget);
}
