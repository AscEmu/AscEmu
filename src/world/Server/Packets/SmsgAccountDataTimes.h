/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAccountDataTimes : public ManagedPacket
    {
    public:
        uint32_t time;
        uint8_t unknown1;
        uint32_t mask;
        uint8_t dataCount;

        SmsgAccountDataTimes() : SmsgAccountDataTimes(0, 0, 0, 0)
        {
        }

        SmsgAccountDataTimes(uint32_t time, uint8_t unknown1, uint32_t mask, uint8_t dataCount) :
            ManagedPacket(SMSG_ACCOUNT_DATA_TIMES, dataCount > 8 ? 32 * 4 : 4 + 1 + 4 + dataCount * 4),
            time(time),
            unknown1(unknown1),
            mask(mask),
            dataCount(dataCount)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= TBC

            for (auto i = 0; i < dataCount; ++i)
                packet << uint32_t(0);

#elif VERSION_STRING <= Cata

            packet << time << unknown1 << mask;
            for (auto i = 0; i < dataCount; ++i)
                if (mask & (1 << i))
                    packet << uint32_t(0);

#else

            packet.writeBit(1);
            packet.flushBits();

            for (uint8_t i = 0; i < 8; ++i)
                    packet << uint32_t(0);

            packet << mask << time;
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
