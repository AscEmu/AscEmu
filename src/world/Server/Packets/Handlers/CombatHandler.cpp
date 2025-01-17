/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/Packets/CmsgAttackSwing.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Maps/WorldMap.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleAttackSwingOpcode(WorldPacket& recvPacket)
{
    CmsgAttackSwing srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_ATTACKSWING: {} (guidLow)", srlPacket.guid.getGuidLow());

    if (_player->isFeared() || _player->isStunned() || _player->isPacified() || _player->isDead())
        return;

    const auto unitTarget = _player->getWorldMap()->getUnit(srlPacket.guid.getRawGuid());
    if (unitTarget == nullptr)
        return;

    if (!_player->isValidTarget(unitTarget) || unitTarget->isDead())
        return;

    _player->smsg_AttackStart(unitTarget);
    _player->eventAttackStart();
}

void WorldSession::handleAttackStopOpcode(WorldPacket& /*recvPacket*/)
{
    const auto unitTarget = _player->getWorldMap()->getUnit(_player->getTargetGuid());
    if (unitTarget == nullptr)
        return;

    _player->eventAttackStop();
    _player->smsg_AttackStop(unitTarget);
}
