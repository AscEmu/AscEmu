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
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x+: one block per bracket, sent as rated info
                packet.setOpcode(SMSG_BATTLEFIELD_RATED_INFO);
                const auto expansion = m_protocol.expansion;
                uint8_t brackets = 6;
                uint8_t values = 8;                                             // 6.x
                if (expansion == WoW::Expansion::_Legion)
                    values = 11;
                else if (expansion == WoW::Expansion::_BfA)
                    values = 13;
                else if (expansion == WoW::Expansion::_Shadowlands)
                    values = 16;
                else if (expansion == WoW::Expansion::_Dragonflight)
                {
                    brackets = 9;
                    values = 19;
                }
                else if (expansion >= WoW::Expansion::_TheWarWithin)
                {
                    brackets = 9;
                    values = 18;
                }

                for (uint8_t bracket = 0; bracket < brackets; ++bracket)
                {
                    for (uint8_t i = 0; i < values; ++i)
                        packet << int32_t(0);
                    if (expansion >= WoW::Expansion::_TheWarWithin)
                        packet << uint8_t(0);                                   // best season tier
                    if (expansion >= WoW::Expansion::_Shadowlands)
                    {
                        packet.writeBit(false);                                 // disqualified
                        packet.flushBits();
                    }
                }
                return true;
            }

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
