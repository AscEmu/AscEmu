/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPvpOptionsEnabled : public ManagedPacket
    {
    public:
        bool wargamesEnabled;
        bool ratedBgEnabled;
        bool ratedArenasEnabled;

        SmsgPvpOptionsEnabled() : SmsgPvpOptionsEnabled(true, true, true)
        {
        }

        SmsgPvpOptionsEnabled(bool wargamesEnabled, bool ratedBgEnabled, bool ratedArenasEnabled) :
            ManagedPacket(SMSG_PVP_OPTIONS_ENABLED, 1),
            wargamesEnabled(wargamesEnabled),
            ratedBgEnabled(ratedBgEnabled),
            ratedArenasEnabled(ratedArenasEnabled)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                const auto expansion = m_protocol.expansion;
                if (expansion == WoW::Expansion::_WoD)
                {
                    // 6.x: five flags
                    for (uint8_t i = 0; i < 5; ++i)
                        packet.writeBit(true);
                }
                else if (expansion == WoW::Expansion::_Legion)
                {
                    // 7.x: rated arenas, skirmish, random battlegrounds, war game battlegrounds, war game arenas, rated battlegrounds
                    packet.writeBit(ratedArenasEnabled);
                    packet.writeBit(true);
                    packet.writeBit(true);
                    packet.writeBit(wargamesEnabled);
                    packet.writeBit(wargamesEnabled);
                    packet.writeBit(ratedBgEnabled);
                }
                else
                {
                    // 8.x+: rated battlegrounds, random battlegrounds, war game battlegrounds, war game arenas, rated arenas, skirmish
                    packet.writeBit(ratedBgEnabled);
                    packet.writeBit(true);
                    packet.writeBit(wargamesEnabled);
                    packet.writeBit(wargamesEnabled);
                    packet.writeBit(ratedArenasEnabled);
                    packet.writeBit(true);
                    // 10.x+: solo shuffle, rated solo shuffle, blitz, rated blitz
                    if (expansion >= WoW::Expansion::_Dragonflight)
                    {
                        for (uint8_t i = 0; i < 4; ++i)
                            packet.writeBit(false);
                    }
                }
                packet.flushBits();
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                return false;

            packet.writeBit(1);
            packet.writeBit(wargamesEnabled);
            packet.writeBit(1);
            packet.writeBit(ratedBgEnabled);
            packet.writeBit(ratedArenasEnabled);

            packet.flushBits();

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
