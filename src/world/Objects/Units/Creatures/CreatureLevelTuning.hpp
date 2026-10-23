/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"

#include <cstdint>

class Creature;
class Unit;
struct CreatureProperties;

// a creature with a level range meets every unit on that unit's level inside the range; health and damage
// follow the two ends of the range, the creature itself carries the values of the upper level
class SERVER_DECL CreatureLevelTuning
{
public:
    static bool isScaling(CreatureProperties const* properties);
    static bool isScaling(Creature const* creature);

    static uint32_t levelForTarget(Creature const* creature, Unit const* target);

    static float healthAtLevel(CreatureProperties const* properties, uint32_t level);
    static float damageAtLevel(CreatureProperties const* properties, uint32_t level);

    // the ratio between the target's level and the creature's own level
    static float healthMultiplierForTarget(Creature const* creature, Unit const* target);
    static float damageMultiplierForTarget(Creature const* creature, Unit const* target);

    // the upper damage of a row holds the maximum roll, the lower one the minimum roll
    static constexpr float MAX_ROLL_FACTOR = 1.5f;
};
