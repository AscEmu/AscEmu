/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum SummonType
{
    SUMMONTYPE_NONE                = 0,
    SUMMONTYPE_PET                 = 1,
    SUMMONTYPE_GUARDIAN            = 2,
    SUMMONTYPE_MINION              = 3,
    SUMMONTYPE_TOTEM               = 4,
    SUMMONTYPE_MINIPET             = 5,
    SUMMONTYPE_GUARDIAN2           = 6,
    SUMMONTYPE_WILD2               = 7,
    SUMMONTYPE_WILD3               = 8,    // Related to phases and DK prequest line (3.3.5a)
    SUMMONTYPE_VEHICLE             = 9,
    SUMMONTYPE_VEHICLE2            = 10,   // Oculus and Argent Tournament vehicles (3.3.5a)
    SUMMONTYPE_LIGHTWELL           = 11,
    SUMMONTYPE_JEEVES              = 12
};

enum SummonSlot : uint8_t
{
    SUMMON_SLOT_NONE                = 0,
    SUMMON_SLOT_TOTEM_FIRE          = 1,    // Not just totem, seems to be default slot for different summon types
    SUMMON_SLOT_TOTEM_EARTH         = 2,
    SUMMON_SLOT_TOTEM_WATER         = 3,
    SUMMON_SLOT_TOTEM_AIR           = 4,
    SUMMON_SLOT_MINIPET             = 5,
    SUMMON_SLOT_QUEST               = 6,

    MAX_SUMMON_SLOT
};

enum SummonCorpseDespawnTimers : uint16_t
{
    SUMMON_CORPSE_DESPAWN_DEFAULT_TIMER     = 400,
    SUMMON_CORPSE_DESPAWN_EXTENDED_TIMER    = 10000
};
