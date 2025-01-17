/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum AuraRemoveMode : uint8_t
{
    AURA_REMOVE_BY_SERVER   = 0, // Internal stuff
    AURA_REMOVE_ON_EXPIRE   = 1,
    AURA_REMOVE_ON_DISPEL   = 2,
    AURA_REMOVE_ON_STEAL    = 3  // Spell steal
};
