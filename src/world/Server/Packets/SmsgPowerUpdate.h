/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPowerUpdate : public ManagedPacket
    {
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
            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet.writeBit(guid[4]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[1]);

                packet.writeBits(1, 21);

                packet.flushBits();

                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[4]);

                packet << powerType;
                packet << power;

                packet.writeByteSeq(guid[6]);
                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << guid;

                if (m_protocol.expansion == WoW::Expansion::_Cata)
                    packet << uint32_t(1);

                packet << powerType << power;
                return true;
            }
            else // Classic and TBC
            {
                return false;
            }
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
