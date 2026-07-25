/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLearnedSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;
        
        SmsgLearnedSpell() : SmsgLearnedSpell(0)
        {
        }

        SmsgLearnedSpell(uint32_t spellId) :
            ManagedPacket(SMSG_LEARNED_SPELL, 6),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override { return 6; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet.writeBits(1, 22);
                packet.writeBit(0);
                packet << spellId;
            }
            else
            {
                packet << spellId;

                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    packet << uint16_t(0);  //unknown
                }
                else
                {
                    packet << uint32_t(0);  //unknown
                }
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
