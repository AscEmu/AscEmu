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
    class SmsgRemovedSpell : public ManagedPacket
    {
    public:
#if VERSION_STRING < WotLK
        uint16_t spellId;
#else
        uint32_t spellId;
#endif
        
        SmsgRemovedSpell() : SmsgRemovedSpell(0)
        {
        }

        SmsgRemovedSpell(uint32_t spellId) :
#if VERSION_STRING < WotLK
            ManagedPacket(SMSG_REMOVED_SPELL, 2),
            spellId(static_cast<uint16_t>(spellId))
#else
            ManagedPacket(SMSG_REMOVED_SPELL, 4),
            spellId(spellId)
#endif
        {
        }

    protected:
        size_t expectedSize() const override
        {
#if VERSION_STRING < WotLK
            return 2;
#else
            return 4;
#endif
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << spellId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
