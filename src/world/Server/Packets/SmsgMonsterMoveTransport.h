/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgMonsterMoveTransport : public ManagedPacket
    {
    public:
        WoWGuid unitGuid;
        WoWGuid vehicleGuid;
        uint8_t seat;
        LocationVector currentPosition;

        SmsgMonsterMoveTransport() : SmsgMonsterMoveTransport(WoWGuid(), WoWGuid(), 0, {0, 0, 0, 0})
        {
        }

        SmsgMonsterMoveTransport(WoWGuid unitGuid, WoWGuid vehicleGuid, uint8_t seat, LocationVector currentPosition) :
            ManagedPacket(SMSG_MONSTER_MOVE_TRANSPORT, 0),
            unitGuid(unitGuid),
            vehicleGuid(vehicleGuid),
            seat(seat),
            currentPosition(currentPosition)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8 + 1 + 1 + 3 * 4 + 4 + 1 + 4 + 3 * 4 + 3 * 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unitGuid;
            packet << vehicleGuid;
            packet << seat;
            packet << uint8_t(unitGuid.isPlayer());

            packet << currentPosition.x << currentPosition.y << currentPosition.z;
            packet << Util::getMSTime();
            packet << uint8_t(4);           // splinetype facing_angle
            packet << currentPosition.o;    // facing angle
            packet << uint32_t(0x00800000); // splineflag transport
            packet << uint32_t(0);          // movetime
            packet << uint32_t(1);          // move point
            packet << float(0.0f) << float(0.0f) << float(0.0f);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
