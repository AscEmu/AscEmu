/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Unit.hpp"
#include "Server/World.h"

//\ brief: This file includes all (rewritten and confirmed) stat calculations for units
//\ player specific calculations should go to PlayerStats.cpp

static float_t getConfigRateFor(Unit const* unit, WorldConfigRates rate)
{
    if (!unit->isPlayer() && unit->isVehicle())
        return worldConfig.getFloatRate(RATE_VEHICLES_POWER_REGEN);
    else
        return worldConfig.getFloatRate(rate);
}

void Unit::updateEnergyRegeneration([[maybe_unused]]bool initialUpdate/* = false*/)
{
#if VERSION_STRING >= TBC
    // Patch 2.2.0: "Any effect which triggers a change in your rate of power regeneration (Mana, Rage, Energy, Focus)
    // will now cause an immediate reward of some power at the old rate of increase, and then begin new "ticks" of power at the new rate approximately 2 seconds later.
    // This was done to improve functionality of abilities such as Evocation and Innervate so that they did not have wasted "ticks"."
    if (!initialUpdate && isAlive())
        _regeneratePowersAtRegenUpdate(POWER_TYPE_ENERGY);
#endif

    // Base gain is 10 energy per second
    float_t amount = 10.0f;
    // Flat values are per 5 seconds
    amount += getTotalFloatDamageForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_TYPE_ENERGY) / 5.0f;
    // Percentage mods
    amount *= getTotalPctMultiplierForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_TYPE_ENERGY);
    // Config rates
    amount *= getConfigRateFor(this, RATE_POWER4);
    // Remove base gain
    amount -= 10.0f;

    setPowerRegeneration(POWER_TYPE_ENERGY, amount);
    setPowerRegenerationWhileInterrupted(POWER_TYPE_ENERGY, amount);
}

void Unit::updateFocusRegeneration([[maybe_unused]]bool initialUpdate/* = false*/)
{
#if VERSION_STRING >= TBC
    // Patch 2.2.0: "Any effect which triggers a change in your rate of power regeneration (Mana, Rage, Energy, Focus)
    // will now cause an immediate reward of some power at the old rate of increase, and then begin new "ticks" of power at the new rate approximately 2 seconds later.
    // This was done to improve functionality of abilities such as Evocation and Innervate so that they did not have wasted "ticks"."
    if (!initialUpdate && isAlive())
        _regeneratePowersAtRegenUpdate(POWER_TYPE_FOCUS);
#endif

#if VERSION_STRING < WotLK
    // Base gain is 6 focus per second
    const float_t base = 6.0f;
#else
    // Base gain is 5 focus per second
    const float_t base = 5.0f;
#endif
    auto amount = base;
    // Flat values are per 5 seconds
    amount += getTotalFloatDamageForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_TYPE_FOCUS) / 5.0f;
    // Percentage mods
    amount *= getTotalPctMultiplierForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_TYPE_FOCUS);
    // Config rates
    amount *= getConfigRateFor(this, RATE_POWER3);
    // Remove base gain
    amount -= base;

    setPowerRegeneration(POWER_TYPE_FOCUS, amount);
    setPowerRegenerationWhileInterrupted(POWER_TYPE_FOCUS, amount);
}
