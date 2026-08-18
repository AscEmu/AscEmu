/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MovementPacketBuilder.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgFlightSplineSync : public ManagedPacket
    {
    public:
        MovementMgr::MoveSpline* moveSpline;
        WoWGuid guid;

        SmsgFlightSplineSync() : SmsgFlightSplineSync(nullptr, WoWGuid())
        {
        }

        SmsgFlightSplineSync(MovementMgr::MoveSpline* moveSpline, WoWGuid guid) :
            ManagedPacket(SMSG_FLIGHT_SPLINE_SYNC, 12),
            moveSpline(moveSpline), guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 12; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (moveSpline == nullptr)
                return false;

            if (m_protocol.expansion >= WoW::Expansion::_Mop)
                return false;

            ByteBuffer packedGuid;
            packedGuid.appendPackGuid(guid);

            MovementMgr::PacketBuilder::WriteSplineSync(*moveSpline, packet);
            packet.append(packedGuid);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
