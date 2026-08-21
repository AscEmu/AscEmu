/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgBattlePetJournal : public ManagedPacket
    {
    public:
        static constexpr uint8_t maxLoadoutSlots = 3;

        SmsgBattlePetJournal() : ManagedPacket(SMSG_BATTLE_PET_JOURNAL, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + maxLoadoutSlots; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet.writeBits(0, 19);

            packet.writeBit(1);
            packet.writeBits(maxLoadoutSlots, 25);

            for (uint8_t i = 0; i < maxLoadoutSlots; ++i)
            {
                packet.writeBit(1);
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(0);

                for (uint8_t b = 0; b < 8; ++b)
                    packet.writeBit(0);
            }

            packet.flushBits();

           
            for (uint8_t i = 0; i < maxLoadoutSlots; ++i)
                packet << i;

            packet << static_cast<uint16_t>(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
