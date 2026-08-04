/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

static constexpr uint8_t MAX_SPELL_EFFECTS = 3;
static constexpr uint8_t MAX_SPELL_TOTEMS = 2;
static constexpr uint8_t MAX_SPELL_REAGENTS = 8;
static constexpr uint8_t MAX_SPELL_TOTEM_CATEGORIES = 2;

namespace WDB::Structures
{
    enum FactionTemplateFlags : uint32_t
    {
        FACTION_TEMPLATE_FLAG_PVP = 0x00000800, // flagged for PvP
        FACTION_TEMPLATE_FLAG_CONTESTED_GUARD = 0x00001000, // faction will attack players that were involved in PvP combats
        FACTION_TEMPLATE_FLAG_HOSTILE_BY_DEFAULT = 0x00002000
    };

    enum FactionMasks : uint32_t
    {
        FACTION_MASK_PLAYER = 1, // any player
        FACTION_MASK_ALLIANCE = 2, // player or creature from alliance team
        FACTION_MASK_HORDE = 4, // player or creature from horde team
        FACTION_MASK_MONSTER = 8 // aggressive creature from monster team
        // if none flags set then non-aggressive creature
    };
}
