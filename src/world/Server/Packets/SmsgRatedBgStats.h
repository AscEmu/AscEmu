/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgRatedBgStats : public ManagedPacket
    {
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
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                return false;

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
    };
}
