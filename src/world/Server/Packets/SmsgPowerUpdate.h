/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPowerUpdate : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;
        uint8_t powerType;
        uint32_t power;

        SmsgPowerUpdate() : SmsgPowerUpdate(WoWGuid(), 0, 0)
        {
        }

        SmsgPowerUpdate(WoWGuid guid, uint8_t powerType, uint32_t power) :
            ManagedPacket(SMSG_POWER_UPDATE, 0),
            guid(guid),
            powerType(powerType),
            power(power)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 1 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            packet.writeBit(guid[4]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[1]);

            packet.writeBits(1, 21);

            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[4]);

            packet << powerType;
            packet << power;

            packet.WriteByteSeq(guid[6]);
#elif VERSION_STRING != Mop
            packet << guid;

#if VERSION_STRING == Cata
            packet << uint32_t(1);
#endif
            packet << powerType << power;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
