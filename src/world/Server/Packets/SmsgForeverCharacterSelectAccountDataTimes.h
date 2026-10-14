/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgForeverCharacterSelectAccountDataTimes final : public ManagedPacket
    {
    public:
        uint16_t unknown0{0};
        uint32_t unixTime{0};
        std::array<uint32_t, 41> unknownValues{};

        explicit SmsgForeverCharacterSelectAccountDataTimes(uint32_t unixTime = 0) : ManagedPacket(SMSG_ACCOUNT_DATA_TIMES, 170), unixTime(unixTime) {}

    protected:
        size_t expectedSize() const override { return 170; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << unknown0;
            packet << unixTime;
            for (const uint32_t value : unknownValues)
                packet << value;

            return packet.size() == expectedSize();
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
