/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSupercededSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;
        uint32_t supercedeSpellId;
        
        SmsgSupercededSpell() : SmsgSupercededSpell(0, 0)
        {
        }

        SmsgSupercededSpell(uint32_t spellId, uint32_t supercedeSpellId) :
            ManagedPacket(SMSG_SUPERCEDED_SPELL, 8),
            spellId(spellId),
            supercedeSpellId(supercedeSpellId)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << spellId << supercedeSpellId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
