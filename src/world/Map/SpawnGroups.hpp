/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "Map/SpawnTypes.hpp"
#include <set>
#include <string>


enum SpawnObjectTypeMask
{
    // Masks are a separate bitset and intentionally keep the historical bit layout.
    SPAWN_TYPEMASK_CREATURE                 = 0x01,
    SPAWN_TYPEMASK_GAMEOBJECT               = 0x02,
    SPAWN_TYPEMASK_WITH_DATA                = SPAWN_TYPEMASK_CREATURE | SPAWN_TYPEMASK_GAMEOBJECT
};

enum SpawnFlags
{
    SPAWFLAG_FLAG_NONE                      = 0x00,             // Creatures always Respawns if no BossId is set
    SPAWFLAG_FLAG_FULLPACK                  = 0x01,             // Creatures are only available in the Full group example: Shattred Halls
    SPAWFLAG_FLAG_BOUNDTOBOSS               = 0x02,             // Creatures are required to Start the Encounter
    SPAWFLAG_FLAG_SCRIPT                    = 0x04,             // currently not used

    SPAWNFLAG_FLAGS_ALL = (SPAWFLAG_FLAG_NONE | SPAWFLAG_FLAG_FULLPACK | SPAWFLAG_FLAG_BOUNDTOBOSS | SPAWFLAG_FLAG_SCRIPT)
};

enum SpawnGroupFlags
{
    SPAWNGROUP_FLAG_NONE                    = 0x00,
    SPAWNGROUP_FLAG_SYSTEM                  = 0x01,
    SPAWNGROUP_FLAG_MANUAL_SPAWN            = 0x02,
    SPAWNGROUP_FLAG_DYNAMIC_SPAWN_RATE      = 0x04,             // currently not used
    SPAWNGROUP_FLAG_ESCORTQUESTNPC          = 0x08,             // currently not used

    SPAWNGROUP_FLAGS_ALL = (SPAWNGROUP_FLAG_SYSTEM | SPAWNGROUP_FLAG_MANUAL_SPAWN | SPAWNGROUP_FLAG_DYNAMIC_SPAWN_RATE | SPAWNGROUP_FLAG_ESCORTQUESTNPC)
};

struct SpawnGroupTemplateData
{
    uint32_t groupId;
    std::string name;
    uint32_t mapId;
    SpawnGroupFlags groupFlags;
    SpawnFlags spawnFlags;
    std::set<uint32_t> spawns;
    uint32_t bossId;
};
