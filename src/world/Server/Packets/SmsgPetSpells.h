/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    static inline constexpr uint8_t MAX_ACTION_SLOT = 10;
    using SmsgPetSpellsVector = std::vector<uint32_t>;
    using SmsgPetActionsArray = std::array<uint32_t, MAX_ACTION_SLOT>;

    static inline constexpr uint32_t packPetActionButtonData(uint32_t spellId, uint8_t state)
    {
        return (static_cast<uint32_t>(state) << 24) | spellId;
    }

    class SmsgPetSpells : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint16_t family;
        uint32_t expireDuration;
        uint8_t reactState; // 0x0 = passive, 0x1 = defensive, 0x2 = aggressive
        uint8_t petAction; // 0x0 = stay, 0x1 = follow, 0x2 = attack
        uint16_t flags; // flags: 0xFF = disabled pet bar (eg. when pet stunned)
        SmsgPetActionsArray actions;
        SmsgPetSpellsVector spells;
        uint8_t cooldowns;

        bool resetSpells = false;

        SmsgPetSpells() : SmsgPetSpells(0, 0, 0, 0, 0, 0, SmsgPetActionsArray(), SmsgPetSpellsVector())
        {
        }

        SmsgPetSpells(uint64_t guid, uint16_t family, uint32_t expireDuration, uint8_t reactState, uint8_t petAction,
            uint16_t flags, SmsgPetActionsArray&& actions, SmsgPetSpellsVector&& spells) :
            ManagedPacket(SMSG_PET_SPELLS, 0),
            guid(guid),
            family(family),
            expireDuration(expireDuration),
            reactState(reactState),
            petAction(petAction),
            flags(flags),
            actions(std::move(actions)),
            spells(std::move(spells)),
            cooldowns(0)
        {
        }

        // Used with guid 0 to remove spells
        SmsgPetSpells(uint64_t guid) :
            ManagedPacket(SMSG_PET_SPELLS, 0),
            guid(guid),
            family(0),
            expireDuration(0),
            reactState(0),
            petAction(0),
            flags(0),
            actions(SmsgPetActionsArray()),
            spells(SmsgPetSpellsVector()),
            cooldowns(0),
            resetSpells(true)
        {
        }

    protected:
#if VERSION_STRING < WotLK
        size_t expectedSize() const override { return resetSpells ?
            8 :
            static_cast<size_t>(8 + 4 + 1 + 1 + 2 + (4 * MAX_ACTION_SLOT)) + 1 + (4 * spells.size()) + 1; }
#else
        size_t expectedSize() const override { return resetSpells ?
            8 :
            static_cast<size_t>(8 + 2 + 4 + 1 + 1 + 2 + (4 * MAX_ACTION_SLOT)) + 1 + (4 * spells.size()) + 1; }
#endif

        bool internalSerialise(WorldPacket& packet) override
        {
            if (resetSpells)
            {
                packet << guid;
                return true;
            }

            packet << guid;
#if VERSION_STRING >= WotLK
            packet << family;
#endif
            packet << expireDuration << reactState << petAction << flags;
            // Action bar
            for (const auto slot : actions)
                packet << slot;

            // Spell book
            packet << uint8_t(spells.size());
            if (!spells.empty())
            {
                for (const auto spell : spells)
                    packet << spell;
            }

            // TODO: cooldowns
            packet << cooldowns;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
