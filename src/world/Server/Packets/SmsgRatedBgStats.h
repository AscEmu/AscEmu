/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgRatedBgStats : public ManagedPacket
    {
#if VERSION_STRING > WotLK
    public:
        uint8_t unk;

        SmsgRatedBgStats() : SmsgRatedBgStats(0)
        {
        }

        SmsgRatedBgStats(uint8_t unk) :
            ManagedPacket(SMSG_RATED_BG_STATS, 29),
            unk(unk)
        {
        }

    protected:
        size_t expectedSize() const override { return 29; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uint32_t(0);    // unknown
            packet << unk;            // unknown - always 3?... type?
            packet << uint32_t(0);    // unknown
            packet << uint32_t(0);    // unknown
            packet << uint32_t(0);    // unknown
            packet << uint32_t(0);    // unknown
            packet << uint32_t(0);    // unknown
            packet << uint32_t(0);    // unknown

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}
