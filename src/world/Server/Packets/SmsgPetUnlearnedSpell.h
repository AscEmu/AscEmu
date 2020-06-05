/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPetUnlearnedSpell : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t spellId;

        SmsgPetUnlearnedSpell() : SmsgPetUnlearnedSpell(0)
        {
        }

        SmsgPetUnlearnedSpell(uint32_t spellId) :
            ManagedPacket(SMSG_PET_UNLEARNED_SPELL, 4),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << spellId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
