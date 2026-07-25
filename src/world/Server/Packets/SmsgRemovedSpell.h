/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgRemovedSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;

        SmsgRemovedSpell() : SmsgRemovedSpell(0)
        {
        }

        explicit SmsgRemovedSpell(uint32_t spellId) :
            ManagedPacket(SMSG_REMOVED_SPELL, 0),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.expansion < WoW::Expansion::_WotLK ? 2 : 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_WotLK)
                packet << static_cast<uint16_t>(spellId);
            else
                packet << spellId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
