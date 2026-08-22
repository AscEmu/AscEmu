/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgRandomRoll : public ManagedPacket
    {
    public:
        uint32_t min;
        uint32_t max;
        uint32_t roll;
        uint64_t guid;

        MsgRandomRoll() : MsgRandomRoll(0, 0, 0, 0)
        {
        }

        MsgRandomRoll(uint32_t min, uint32_t max, uint32_t roll, uint64_t guid) :
            ManagedPacket(MSG_RANDOM_ROLL, 8),
            min(min),
            max(max),
            roll(roll),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return static_cast<size_t>(4 + 4 + 4 + 8); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid rollGuid = guid;

                packet << uint32_t(roll);
                packet << uint32_t(min);
                packet << uint32_t(max);

                packet.writeBit(rollGuid[0]);
                packet.writeBit(rollGuid[6]);
                packet.writeBit(rollGuid[7]);
                packet.writeBit(rollGuid[1]);
                packet.writeBit(rollGuid[4]);
                packet.writeBit(rollGuid[5]);
                packet.writeBit(rollGuid[2]);
                packet.writeBit(rollGuid[3]);

                packet.flushBits();

                packet.writeByteSeq(rollGuid[5]);
                packet.writeByteSeq(rollGuid[4]);
                packet.writeByteSeq(rollGuid[2]);
                packet.writeByteSeq(rollGuid[0]);
                packet.writeByteSeq(rollGuid[3]);
                packet.writeByteSeq(rollGuid[1]);
                packet.writeByteSeq(rollGuid[6]);
                packet.writeByteSeq(rollGuid[7]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << min << max << roll << guid;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet >> max >> min;
                packet.readSkip<uint8_t>();
                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> min >> max;
                return true;
            }

            return false;
        }
    };
}
