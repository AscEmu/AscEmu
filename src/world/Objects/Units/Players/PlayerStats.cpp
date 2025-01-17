/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Player.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Units/Stats.h"
#include "Server/World.h"
#include "Storage/WDB/WDBStructures.hpp"

//\ brief: This file includes all (rewritten and confirmed) stat calculations for players
void Player::updateManaRegeneration([[maybe_unused]]bool initialUpdate/* = false*/)
{
    // Used by all versions
    const auto spirit = getStat(STAT_SPIRIT);

#if VERSION_STRING == Classic
    // Calculations are based on Nostalrius' work
    float_t regenerateValue = 0.0f;
    switch (getClass())
    {
        case PALADIN:
        case HUNTER:
        case WARLOCK:
        case DRUID:
            regenerateValue = spirit / 5 + 15.0f;
            break;
        case PRIEST:
        case MAGE:
            regenerateValue = spirit / 4 + 12.5f;
            break;
        case SHAMAN:
            regenerateValue = spirit / 5 + 17.0f;
            break;
        default:
            break;
    }

    regenerateValue = (regenerateValue / 2) * getTotalPctMultiplierForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_TYPE_MANA);

    // This is per 5 seconds
    float_t mp5 = getTotalFloatDamageForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_TYPE_MANA) / 5.0f;
    for (uint8_t i = 0; i < STAT_COUNT; ++i)
        mp5 += (m_modManaRegenFromStat[i] * getStat(i)) / 500.0f;

    auto manaWhileCastingPct = m_modInterrManaRegenPct;
    // Cap at 100%
    if (manaWhileCastingPct > 100)
        manaWhileCastingPct = 100;

    // Make sure mana regeneration isn't negative
    if (regenerateValue < 0)
        regenerateValue = 0.0f;
    if (mp5 < 0)
        mp5 = 0.0f;

    const auto manaRegen = std::ceil((mp5 + regenerateValue) * worldConfig.getFloatRate(RATE_POWER1));
    const auto manaRegenCast = std::ceil((mp5 + (regenerateValue * manaWhileCastingPct / 100.0f)) * worldConfig.getFloatRate(RATE_POWER1));
    setPowerRegeneration(POWER_TYPE_MANA, manaRegen);
    setPowerRegenerationWhileInterrupted(POWER_TYPE_MANA, manaRegenCast);
#else
    // Patch 2.2.0: "Any effect which triggers a change in your rate of power regeneration (Mana, Rage, Energy, Focus)
    // will now cause an immediate reward of some power at the old rate of increase, and then begin new "ticks" of power at the new rate approximately 2 seconds later.
    // This was done to improve functionality of abilities such as Evocation and Innervate so that they did not have wasted "ticks"."
    if (!initialUpdate && isAlive())
        _regeneratePowersAtRegenUpdate(POWER_TYPE_MANA);

    auto playerLevel = getLevel();
    if (playerLevel > DBC_STAT_LEVEL_CAP)
        playerLevel = DBC_STAT_LEVEL_CAP;

    // Get base mana regeneration value from DBC
    const auto index = (getClass() - 1) * DBC_STAT_LEVEL_CAP + playerLevel - 1;
    const auto manaPerSpiritDBC = sGtRegenMPPerSptStore.lookupEntry(index);
    //\ todo: gtOCTRegenMP.dbc seems to have base mana regen values for all classes (i.e 0.25 for paladin 1-80lvl on wotlk)
    //\ how is it used in calculations?
    //const auto baseManaRegen = sGtOCTRegenMPStore.lookupEntry(index);

    float_t baseRegen = 0.0f;
    if (manaPerSpiritDBC != nullptr)
        baseRegen = manaPerSpiritDBC->ratio;

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
    const auto intellect = getStat(STAT_INTELLECT);
    // From wowwiki: MP5 = 5 * (0.001 + std::sqrt(Int) * Spirit * Base_Regen) * 0.60 rounded up
    float_t regenerateValue = 0.001f + std::sqrt(static_cast<float_t>(intellect)) * spirit * baseRegen;
    regenerateValue *= getTotalPctMultiplierForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_TYPE_MANA);

    // This is per 5 seconds
    float_t mp5 = getTotalFloatDamageForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_TYPE_MANA) / 5.0f;
    for (uint8_t i = 0; i < STAT_COUNT; ++i)
        mp5 += (m_modManaRegenFromStat[i] * getStat(i)) / 500.0f;

    auto manaWhileCastingPct = m_modInterrManaRegenPct;
    // Cap at 100%
    if (manaWhileCastingPct > 100)
        manaWhileCastingPct = 100;

    // Make sure mana regeneration isn't negative
    if (regenerateValue < 0)
        regenerateValue = 0.0f;
    if (mp5 < 0)
        mp5 = 0.0f;

    const auto manaRegen = std::ceil((mp5 + regenerateValue) * worldConfig.getFloatRate(RATE_POWER1));
    const auto manaRegenCast = std::ceil((mp5 + (regenerateValue * manaWhileCastingPct / 100.0f)) * worldConfig.getFloatRate(RATE_POWER1));
    setPowerRegeneration(POWER_TYPE_MANA, manaRegen);
    setPowerRegenerationWhileInterrupted(POWER_TYPE_MANA, manaRegenCast);
#elif VERSION_STRING == Cata
    const auto intellect = getStat(STAT_INTELLECT);
    // From wowwiki: MP5 = 5 * (0.001 + std::sqrt(Int) * Spirit * Base_Regen) * 0.60 rounded up
    float_t regenerateValue = 0.001f + std::sqrt(static_cast<float_t>(intellect)) * spirit * baseRegen;
    regenerateValue *= getTotalPctMultiplierForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_TYPE_MANA);

    // In cata 5 second rule no longer exists
    // Instead you regen 5% of your base mana without modifiers while in combat
    float_t baseCombatRegen = (getBaseMana() * 0.05f) + getTotalFloatDamageForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_TYPE_MANA);
    baseCombatRegen /= 5.0f;

    auto manaWhileInCombatPct = m_modInterrManaRegenPct;
    // Cap at 100%
    if (manaWhileInCombatPct > 100)
        manaWhileInCombatPct = 100;

    const auto manaRegen = std::ceil((regenerateValue + baseCombatRegen) * worldConfig.getFloatRate(RATE_POWER1));
    const auto manaRegenCombat = std::ceil((baseCombatRegen + (regenerateValue * manaWhileInCombatPct / 100.0f)) * worldConfig.getFloatRate(RATE_POWER1));
    setPowerRegeneration(POWER_TYPE_MANA, manaRegen);
    setPowerRegenerationWhileInterrupted(POWER_TYPE_MANA, manaRegenCombat);
#elif VERSION_STRING == Mop
    // In MOP base combat mana regen is 2% of player's total mana
    float_t baseCombatRegen = getMaxPower(POWER_TYPE_MANA) * 0.02f;

    // Combat Regen = Total Mana * 0.02 + (1.1287 * SPI * Meditation%)
    float_t regenerateValue = baseRegen * spirit * getTotalPctMultiplierForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_TYPE_MANA);
    regenerateValue = (regenerateValue + getTotalFloatDamageForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_TYPE_MANA)) / 5.0f;

    auto manaWhileInCombatPct = m_modInterrManaRegenPct;
    // Cap at 100%
    if (manaWhileInCombatPct > 100)
        manaWhileInCombatPct = 100;

    const auto manaRegen = std::ceil((regenerateValue + baseCombatRegen) * worldConfig.getFloatRate(RATE_POWER1));
    const auto manaRegenCombat = std::ceil((baseCombatRegen + (regenerateValue * manaWhileInCombatPct / 100.0f)) * worldConfig.getFloatRate(RATE_POWER1));
    setPowerRegeneration(POWER_TYPE_MANA, manaRegen);
    setPowerRegenerationWhileInterrupted(POWER_TYPE_MANA, manaRegenCombat);
#endif
#endif
}

void Player::updateRageRegeneration([[maybe_unused]]bool initialUpdate/* = false*/)
{
#if VERSION_STRING >= TBC
    // Patch 2.2.0: "Any effect which triggers a change in your rate of power regeneration (Mana, Rage, Energy, Focus)
    // will now cause an immediate reward of some power at the old rate of increase, and then begin new "ticks" of power at the new rate approximately 2 seconds later.
    // This was done to improve functionality of abilities such as Evocation and Innervate so that they did not have wasted "ticks"."
    if (!initialUpdate && isAlive())
        _regeneratePowersAtRegenUpdate(POWER_TYPE_RAGE);
#endif

    // Base out of combat decay is 1.25 rage per second
    float_t outCombat = -12.5f;
    float_t inCombat = 0.0f;

    // Flat values are per 5 seconds
    const auto flatAmount = getTotalFloatDamageForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_TYPE_RAGE) / 5.0f;
    outCombat += flatAmount;
    inCombat += flatAmount;
    // There dont seem to be any rage percent modifers
    // Apply config rate to combat regeneration only
    inCombat *= worldConfig.getFloatRate(RATE_POWER2);
    // Remove base out of combat decay
    outCombat -= -12.5f;

    setPowerRegeneration(POWER_TYPE_RAGE, outCombat);
    setPowerRegenerationWhileInterrupted(POWER_TYPE_RAGE, inCombat);
}

#if VERSION_STRING >= WotLK
void Player::updateRunicPowerRegeneration(bool initialUpdate/* = false*/)
{
    // Patch 2.2.0: "Any effect which triggers a change in your rate of power regeneration (Mana, Rage, Energy, Focus)
    // will now cause an immediate reward of some power at the old rate of increase, and then begin new "ticks" of power at the new rate approximately 2 seconds later.
    // This was done to improve functionality of abilities such as Evocation and Innervate so that they did not have wasted "ticks"."
    if (!initialUpdate && isAlive())
        _regeneratePowersAtRegenUpdate(POWER_TYPE_RUNIC_POWER);

    // There dont seem to be any out of combat modifiers
    float_t inCombat = 0.0f;
    // Flat Values are per 5 seconds (Butchery talent only increases combat regeneration)
    inCombat += getTotalFloatDamageForAuraEffectByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_TYPE_RUNIC_POWER) / 5.0f;
    // There dont seem to be any runic power percent modifers
    // Apply config rate to combat regeneration only
    inCombat *= worldConfig.getFloatRate(RATE_POWER7);

    setPowerRegeneration(POWER_TYPE_RUNIC_POWER, 0.0f);
    setPowerRegenerationWhileInterrupted(POWER_TYPE_RUNIC_POWER, inCombat);
}
#endif

float_t Player::calculateHealthRegenerationValue(bool inCombat) const
{
    float_t regenerateValue = 0.0f;
#if VERSION_STRING == Classic
    const auto spirit = getStat(STAT_SPIRIT);
    switch (getClass())
    {
        case DRUID:
        case MAGE:
            regenerateValue = spirit * 0.11f + 1.0f;
            break;
        case HUNTER:
            regenerateValue = spirit * 0.43f - 5.5f;
            break;
        case PALADIN:
            regenerateValue = spirit * 0.25f;
            break;
        case PRIEST:
            regenerateValue = spirit * 0.15f + 1.4f;
            break;
        case ROGUE:
            regenerateValue = spirit * 0.84f - 13.0f;
            break;
        case SHAMAN:
            regenerateValue = spirit * 0.28f - 3.6f;
            break;
        case WARLOCK:
            regenerateValue = spirit * 0.12f + 1.5f;
            break;
        case WARRIOR:
            regenerateValue = spirit * 1.26f - 22.6f;
            break;
        default:
            break;
    }
#elif VERSION_STRING == TBC || VERSION_STRING == WotLK
    auto playerLevel = getLevel();
    if (playerLevel > DBC_STAT_LEVEL_CAP)
        playerLevel = DBC_STAT_LEVEL_CAP;

    // Get base health regeneration value from DBC
    const auto index = (getClass() - 1) * DBC_STAT_LEVEL_CAP + playerLevel - 1;

    float_t baseRegen = 0.0f;
    if (const auto hpPerSpiritDBC = sGtOCTRegenHPStore.lookupEntry(index))
        baseRegen = hpPerSpiritDBC->ratio;

    float_t extraRegen = 0.0f;
    if (const auto hpPerSpiritDBC = sGtRegenHPPerSptStore.lookupEntry(index))
        extraRegen = hpPerSpiritDBC->ratio;

    auto spirit = getStat(STAT_SPIRIT);
    uint32_t extraSpirit = 0;
    if (spirit > 50)
    {
        extraSpirit = spirit - 50;
        spirit = 50;
    }

    regenerateValue = spirit * baseRegen + extraSpirit * extraRegen;
#elif VERSION_STRING >= Cata
    const auto maxHp = static_cast<float_t>(getMaxHealth());
    if (getLevel() < 15)
        regenerateValue = 0.2f * (maxHp / getLevel());
    else
        regenerateValue = 0.015f * maxHp;
#endif

    // Food buffs
    // The value is stored as per 5 seconds
    regenerateValue += getTotalFloatDamageForAuraEffect(SPELL_AURA_MOD_HEALTH_REGEN) / 5.0f * 2;
    // Percentage mods
    regenerateValue *= getTotalPctMultiplierForAuraEffect(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);
    // According to wowwiki health regeneration is increased by 33% while sitting
    if (m_isResting || isSitting())
        regenerateValue *= 1.33f;
    // Apply config rate
    regenerateValue *= worldConfig.getFloatRate(RATE_HEALTH);

    // Combat modifier
    if (inCombat)
        regenerateValue *= getTotalFloatDamageForAuraEffect(SPELL_AURA_MOD_HEALTH_REGEN_DURING_COMBAT) / 100.0f;

    // Auras like Demon Armor in pre wotlk work in combat as well
    // The value is stored as per 5 seconds
    regenerateValue += (getTotalFloatDamageForAuraEffect(SPELL_AURA_MOD_HEALTH_REGEN_ALWAYS) / 5.0f * 2) * worldConfig.getFloatRate(RATE_HEALTH);

    return regenerateValue;
}
