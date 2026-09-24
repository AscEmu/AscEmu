/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MovementPacketBuilder.h"
#include "Objects/Units/Unit.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    // The spline-driven counterpart to SmsgMonsterMove, used by MoveSplineInit::Launch()/Stop().
    // Unlike SmsgMonsterMove's simple single-point moves, the actual spline point data is
    // variable-length and written by MovementMgr::PacketBuilder, matching the pre-refactor code.
    class SmsgMonsterMoveSpline : public ManagedPacket
    {
    public:
        enum class Mode
        {
            Launch,
            Stop
        };

        Unit* unit;
        bool transport;
        Mode mode;
        MovementMgr::MoveSpline* moveSpline = nullptr;
        MovementMgr::Location stopLocation{};
        uint32_t splineId = 0;

        SmsgMonsterMoveSpline() : SmsgMonsterMoveSpline(nullptr, false, nullptr)
        {
        }

        // Launch variant
        SmsgMonsterMoveSpline(Unit* unit, bool transport, MovementMgr::MoveSpline* moveSpline) :
            ManagedPacket(SMSG_MONSTER_MOVE, 64),
            unit(unit), transport(transport), mode(Mode::Launch), moveSpline(moveSpline)
        {
        }

        // Stop variant
        SmsgMonsterMoveSpline(Unit* unit, bool transport, MovementMgr::Location stopLocation, uint32_t splineId) :
            ManagedPacket(SMSG_MONSTER_MOVE, 64),
            unit(unit), transport(transport), mode(Mode::Stop), stopLocation(stopLocation), splineId(splineId)
        {
        }

    protected:
        size_t expectedSize() const override { return 64; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (unit == nullptr)
                return false;

            // MoP and Forever serialize the mover guid inside their version-specific monster-move payload.
            // MoP 5.4.8 has no SMSG_MONSTER_MOVE_TRANSPORT opcode at all; Forever likewise uses its
            // modern SMSG_ON_MONSTER_MOVE body rather than the pre-MoP flat guid prefix. The payload is written by 
            // PacketBuilder::WriteMonsterMove/WriteStopMovement below - it is not
            // written up front as a flat/packed guid the way pre-Mop clients expect. So none of this
            // applies on MoP/Forever; leave it byte-for-byte for Classic/TBC/WotLK/Cata.
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << WoWGuid(unit->getGuid());

                if (transport)
                {
                    packet.setOpcode(SMSG_MONSTER_MOVE_TRANSPORT);
                    packet << WoWGuid(unit->getTransGuid());
#if VERSION_STRING >= WotLK
                    if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                        packet << int8_t(unit->GetTransSeat());
#endif
                }
            }

            if (mode == Mode::Launch)
            {
                if (moveSpline == nullptr)
                    return false;

                if (m_protocol.isMop() || m_protocol.isForever())
                    MovementMgr::PacketBuilder::WriteMonsterMove(*moveSpline, packet, unit);
                else
                    MovementMgr::PacketBuilder::WriteMonsterMove(*moveSpline, packet);
            }
            else
            {
                if (m_protocol.isMop() || m_protocol.isForever())
                    MovementMgr::PacketBuilder::WriteStopMovement(stopLocation, splineId, packet, unit);
                else
                    MovementMgr::PacketBuilder::WriteStopMovement(stopLocation, splineId, packet);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
