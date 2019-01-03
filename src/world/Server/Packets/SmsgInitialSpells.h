/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgInitialSpells : public ManagedPacket
    {
    public:
        struct SpellCooldown
        {
            uint16_t spell_id;
            uint16_t item_id;
            uint16_t spell_category;
            uint32_t spell_remaining_cooldown_ms;
            uint32_t category_remaining_cooldown_ms;
        };

        uint8_t unk1;
        std::vector<uint16_t> spell_ids;
        std::vector<SpellCooldown> spell_cooldowns;

        SmsgInitialSpells() : SmsgInitialSpells(std::vector<uint16_t>(), std::vector<SpellCooldown>())
        {
        }

        SmsgInitialSpells(std::vector<uint16_t> spell_ids, std::vector<SpellCooldown> spell_cooldowns) :
            ManagedPacket(SMSG_INITIAL_SPELLS, 0),
            unk1(0),
            spell_ids(move(spell_ids)),
            spell_cooldowns(move(spell_cooldowns))
        {
        }

        void addSpellCooldown(uint16_t spell_id, uint16_t item_id, uint16_t spell_category,
            uint32_t spell_remaining_cooldown_ms, uint32_t category_remaining_cooldown_ms)
        {
            SpellCooldown cd{};
            cd.spell_id = spell_id;
            cd.item_id = item_id;
            cd.spell_category = spell_category;
            cd.spell_remaining_cooldown_ms = spell_remaining_cooldown_ms;
            cd.category_remaining_cooldown_ms = category_remaining_cooldown_ms;
            spell_cooldowns.push_back(cd);
        }

    protected:
        size_t expectedSize() const override
        {
            size_t size = 0;
            size += sizeof(uint8_t); // unk1
            size += sizeof(uint16_t); // spell id count
            size += sizeof(uint32_t) * spell_ids.size(); // spell data
            size += sizeof(uint16_t); // spell cooldowns size
            size += sizeof(SpellCooldown) * spell_cooldowns.size(); // spell cooldowns
            return size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            // All versions share same implementation
            packet << unk1;
            packet << uint16_t(spell_ids.size());
            for (auto spell_id : spell_ids)
            {
                ///\todo check out when we should send 0x0 and when we should send 0xeeee this is not slot, values is always eeee or 0, seems to be cooldown
                packet << spell_id << uint16_t(0);
            }

            packet << uint16_t(spell_cooldowns.size());

            for (auto cd : spell_cooldowns)
            {
                packet << cd.spell_id << cd.item_id << cd.spell_category
                        << cd.spell_remaining_cooldown_ms << cd.category_remaining_cooldown_ms;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket&) override
        {
            return false;
        }
    };
}}
