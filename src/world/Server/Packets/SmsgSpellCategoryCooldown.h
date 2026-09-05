/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    struct SpellCategoryCooldownEntry
    {
        uint32_t category = 0;
        int32_t modCooldown = 0;
    };

    class SmsgSpellCategoryCooldown : public ManagedPacket
    {
    public:
        std::vector<SpellCategoryCooldownEntry> entries;

        SmsgSpellCategoryCooldown() : SmsgSpellCategoryCooldown(std::vector<SpellCategoryCooldownEntry>{})
        {
        }

        explicit SmsgSpellCategoryCooldown(std::vector<SpellCategoryCooldownEntry> categoryEntries) :
            ManagedPacket(SMSG_SPELL_CATEGORY_COOLDOWN, 4 + static_cast<size_t>(categoryEntries.size()) * 8),
            entries(std::move(categoryEntries))
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + entries.size() * 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBits(static_cast<uint32_t>(entries.size()), 23);
                packet.flushBits();

                for (auto const& entry : entries)
                {
                    packet << uint32_t(entry.category);
                    packet << int32_t(entry.modCooldown);
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(static_cast<uint32_t>(entries.size()), 21);
                packet.flushBits();

                for (auto const& entry : entries)
                {
                    packet << int32_t(entry.modCooldown);
                    packet << uint32_t(entry.category);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
