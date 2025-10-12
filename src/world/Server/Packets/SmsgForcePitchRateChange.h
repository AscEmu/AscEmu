/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    class SmsgForcePitchRateChange : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float rate;
        MovementInfo mi;

        SmsgForcePitchRateChange() : SmsgForcePitchRateChange(WoWGuid(), 0, MovementInfo())
        {
        }

        SmsgForcePitchRateChange(WoWGuid guid, float rate, MovementInfo mi) :
//Zyres: Due to wrong understanding of these opcodes the logic gets turned around here
#if VERSION_STRING < Cata
            ManagedPacket(MSG_MOVE_SET_PITCH_RATE, 0),
#else
            ManagedPacket(SMSG_FORCE_PITCH_RATE_CHANGE, 0),
#endif
            guid(guid),
            rate(rate),
            mi(mi)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 4 + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << guid << uint32_t(0) << rate;
            ByteBuffer addition;
            mi.writeMovementInfo(addition, 0, false);
            packet.append(addition);
#elif VERSION_STRING == Cata // TODO: Cata
#else // TODO: Mop
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
