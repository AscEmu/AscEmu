/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgSetPctSpellModifier : public ManagedPacket
    {
    public:
        uint8_t spellGroup = 0;
        uint8_t spellType = 0;
        uint32_t modifier = 0;
        uint32_t unknown = 1;
        std::vector<std::pair<uint8_t, float>> modifierValues;

        SmsgSetPctSpellModifier() :
            ManagedPacket(SMSG_SET_PCT_SPELL_MODIFIER, 0)
        {
        }

        SmsgSetPctSpellModifier(uint8_t spellGroup, uint8_t spellType, uint32_t modifier) :
            ManagedPacket(SMSG_SET_PCT_SPELL_MODIFIER, 0),
            spellGroup(spellGroup),
            spellType(spellType),
            modifier(modifier)
        {
        }

        SmsgSetPctSpellModifier(uint8_t spellType, std::vector<std::pair<uint8_t, float>> modifierValues) :
            ManagedPacket(SMSG_SET_PCT_SPELL_MODIFIER, 0),
            spellType(spellType),
            modifierValues(std::move(modifierValues))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
                return 6;

            return 9 + (5 * modifierValues.size());
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << spellGroup << spellType << modifier;
            }
            else
            {
                packet << unknown << static_cast<uint32_t>(modifierValues.size()) << spellType;

                for (const auto& modifierValue : modifierValues)
                    packet << modifierValue.first << modifierValue.second;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
