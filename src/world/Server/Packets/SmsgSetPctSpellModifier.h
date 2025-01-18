/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSetPctSpellModifier : public ManagedPacket
    {
    public:
#if VERSION_STRING < Cata
        uint8_t spellGroup;
        uint8_t spellType;
        uint32_t modifier;

        SmsgSetPctSpellModifier() : SmsgSetPctSpellModifier(0, 0, 0)
        {
        }

        SmsgSetPctSpellModifier(uint8_t spellGroup, uint8_t spellType, uint32_t modifier) :
            ManagedPacket(SMSG_SET_PCT_SPELL_MODIFIER, 0),
            spellGroup(spellGroup),
            spellType(spellType),
            modifier(modifier)
        {
        }
#else
        uint32_t unk; // probably count for different modifier types in packet
        uint32_t modCount;
        uint8_t spellType;
        std::vector<std::pair<uint8_t, float>> modValues;

        SmsgSetPctSpellModifier() : SmsgSetPctSpellModifier(0, {})
        {
        }

        SmsgSetPctSpellModifier(uint8_t spellType, std::vector<std::pair<uint8_t, float>> modValues) :
            ManagedPacket(SMSG_SET_PCT_SPELL_MODIFIER, 0),
            spellType(spellType),
            modValues(modValues),
            unk(1),
            modCount(static_cast<uint32_t>(modValues.size()))
        {
        }
#endif

    protected:
        size_t expectedSize() const override
        {
#if VERSION_STRING < Cata
            return 1 + 1 + 4;
#else
            return 4 + 4 + 1 + ((1 + 4) * modValues.size());
#endif
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << spellGroup << spellType << modifier;
#else
            packet << unk << modCount << spellType;

            for (const auto& mod : modValues)
            {
                packet << mod.first << mod.second;
            }
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
