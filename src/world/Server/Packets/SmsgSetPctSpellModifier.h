/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgSetPctSpellModifier : public ManagedPacket
    {
    public:
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

    protected:
        size_t expectedSize() const override
        {
            return 1 + 1 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << spellGroup << spellType << modifier;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
