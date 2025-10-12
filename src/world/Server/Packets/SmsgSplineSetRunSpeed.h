/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSplineSetRunSpeed : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float rate;

        SmsgSplineSetRunSpeed() : SmsgSplineSetRunSpeed(WoWGuid(), 0)
        {
        }

        SmsgSplineSetRunSpeed(WoWGuid guid, float rate) :
            ManagedPacket(SMSG_SPLINE_SET_RUN_SPEED, 0),
            rate(rate)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << guid << rate;
#elif VERSION_STRING == Cata
            packet.writeBit(guid[4]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[2]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[4]);
            packet << float(rate);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[1]);
#else // TODO: Mop
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
