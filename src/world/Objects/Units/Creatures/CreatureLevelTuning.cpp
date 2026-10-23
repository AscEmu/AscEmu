/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreatureLevelTuning.hpp"

#include "AEVersion.hpp"
#include "Creature.h"
#include "CreatureDefines.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Unit.hpp"

#include <algorithm>
#include <cmath>

bool CreatureLevelTuning::isScaling(CreatureProperties const* properties)
{
#if VERSION_STRING >= Legion
    return properties != nullptr && properties->MinLevel < properties->MaxLevel;
#else
    (void)properties;
    return false;
#endif
}

bool CreatureLevelTuning::isScaling(Creature const* creature)
{
    return creature != nullptr && isScaling(creature->GetCreatureProperties());
}

uint32_t CreatureLevelTuning::levelForTarget(Creature const* creature, Unit const* target)
{
    const auto properties = creature->GetCreatureProperties();
    if (target == nullptr || !isScaling(properties))
        return creature->getLevel();

    // the player behind a pet decides
    Unit const* reference = target;
    if (Player const* owner = target->getPlayerOwnerOrSelf(); owner != nullptr)
        reference = owner;

    return std::clamp(reference->getLevel(), properties->MinLevel, properties->MaxLevel);
}

namespace
{
    float interpolate(uint32_t minLevel, uint32_t maxLevel, float low, float high, uint32_t level)
    {
        if (maxLevel <= minLevel || low <= 0.0f || high <= 0.0f)
            return high;

        const float t = static_cast<float>(std::clamp(level, minLevel, maxLevel) - minLevel) / static_cast<float>(maxLevel - minLevel);
        return low * std::pow(high / low, t);
    }
}

float CreatureLevelTuning::healthAtLevel(CreatureProperties const* properties, uint32_t level)
{
    return interpolate(properties->MinLevel, properties->MaxLevel, static_cast<float>(properties->MinHealth), static_cast<float>(properties->MaxHealth), level);
}

float CreatureLevelTuning::damageAtLevel(CreatureProperties const* properties, uint32_t level)
{
    return interpolate(properties->MinLevel, properties->MaxLevel, properties->MinDamage, properties->MaxDamage / MAX_ROLL_FACTOR, level);
}

float CreatureLevelTuning::healthMultiplierForTarget(Creature const* creature, Unit const* target)
{
    if (!isScaling(creature) || target == nullptr)
        return 1.0f;

    const auto properties = creature->GetCreatureProperties();
    const float own = healthAtLevel(properties, creature->getLevel());
    return own > 0.0f ? healthAtLevel(properties, levelForTarget(creature, target)) / own : 1.0f;
}

float CreatureLevelTuning::damageMultiplierForTarget(Creature const* creature, Unit const* target)
{
    if (!isScaling(creature) || target == nullptr)
        return 1.0f;

    const auto properties = creature->GetCreatureProperties();
    const float own = damageAtLevel(properties, creature->getLevel());
    return own > 0.0f ? damageAtLevel(properties, levelForTarget(creature, target)) / own : 1.0f;
}
