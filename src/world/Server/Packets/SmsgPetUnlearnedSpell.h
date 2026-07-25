/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPetUnlearnedSpell : public ManagedPacket
    {
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
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet << spellId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
