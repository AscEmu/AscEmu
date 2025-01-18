/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "AEVersion.hpp"
#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSupercededSpell : public ManagedPacket
    {
    public:
#if VERSION_STRING < WotLK
        uint16_t spellId;
        uint16_t supercedeSpellId;
#else
        uint32_t spellId;
        uint32_t supercedeSpellId;
#endif
        
        SmsgSupercededSpell() : SmsgSupercededSpell(0, 0)
        {
        }

        SmsgSupercededSpell(uint32_t spellId, uint32_t supercedeSpellId) :
#if VERSION_STRING < WotLK
            ManagedPacket(SMSG_SUPERCEDED_SPELL, 4),
            spellId(static_cast<uint16_t>(spellId)),
            supercedeSpellId(static_cast<uint16_t>(supercedeSpellId))
#else
            ManagedPacket(SMSG_SUPERCEDED_SPELL, 8),
            spellId(spellId),
            supercedeSpellId(supercedeSpellId)
#endif
        {
        }

    protected:
        size_t expectedSize() const override
        {
#if VERSION_STRING < WotLK
            return 4;
#else
            return 8;
#endif
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << spellId << supercedeSpellId;
#else
            packet << supercedeSpellId << spellId;
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
