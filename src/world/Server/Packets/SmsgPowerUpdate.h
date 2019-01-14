/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
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
            ObjectGuid powerGuid = guid;

            packet.writeBit(powerGuid[4]);
            packet.writeBit(powerGuid[6]);
            packet.writeBit(powerGuid[7]);
            packet.writeBit(powerGuid[5]);
            packet.writeBit(powerGuid[2]);
            packet.writeBit(powerGuid[3]);
            packet.writeBit(powerGuid[0]);
            packet.writeBit(powerGuid[1]);

            packet.writeBits(1, 21);

            packet.WriteByteSeq(powerGuid[7]);
            packet.WriteByteSeq(powerGuid[0]);
            packet.WriteByteSeq(powerGuid[5]);
            packet.WriteByteSeq(powerGuid[3]);
            packet.WriteByteSeq(powerGuid[1]);
            packet.WriteByteSeq(powerGuid[2]);
            packet.WriteByteSeq(powerGuid[4]);

            packet << uint8_t(powerType);
            packet << uint32_t(power);

            packet.WriteByteSeq(powerGuid[6]);
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
}}
