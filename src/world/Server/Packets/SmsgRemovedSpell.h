/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgRemovedSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;
        
        SmsgRemovedSpell() : SmsgRemovedSpell(0)
        {
        }

        SmsgRemovedSpell(uint32_t spellId) :
            ManagedPacket(SMSG_REMOVED_SPELL, 4),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << spellId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
