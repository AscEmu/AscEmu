/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAreaSpiritHealerTime : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t restTime;

        SmsgAreaSpiritHealerTime() : SmsgAreaSpiritHealerTime(0, 0)
        {
        }

        SmsgAreaSpiritHealerTime(uint64_t guid, uint32_t restTime) :
            ManagedPacket(SMSG_AREA_SPIRIT_HEALER_TIME, 12),
            guid(guid),
            restTime(restTime)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid healerGuid = guid;

                packet.writeBit(healerGuid[5]);
                packet.writeBit(healerGuid[2]);
                packet.writeBit(healerGuid[7]);
                packet.writeBit(healerGuid[6]);
                packet.writeBit(healerGuid[1]);
                packet.writeBit(healerGuid[0]);
                packet.writeBit(healerGuid[3]);
                packet.writeBit(healerGuid[4]);

                packet.flushBits();

                packet.writeByteSeq(healerGuid[2]);
                packet.writeByteSeq(healerGuid[3]);
                packet.writeByteSeq(healerGuid[5]);
                packet.writeByteSeq(healerGuid[4]);
                packet.writeByteSeq(healerGuid[6]);
                packet << restTime;
                packet.writeByteSeq(healerGuid[7]);
                packet.writeByteSeq(healerGuid[0]);
                packet.writeByteSeq(healerGuid[1]);
            }
            else
            {
                packet << guid << restTime;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
