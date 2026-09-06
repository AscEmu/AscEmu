/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

//////////////////////////////////////////////////////////////////////////////////////////
/// Stable spawn type values shared by spawn groups, SpawnManager and respawn persistence.
/// Values 1/2 are persisted in respawn data and must not be renumbered.
//////////////////////////////////////////////////////////////////////////////////////////
enum SpawnObjectType : uint16_t
{
    SPAWN_TYPE_INVALID                      = 0,
    SPAWN_TYPE_CREATURE                     = 1,
    SPAWN_TYPE_GAMEOBJECT                   = 2
};
