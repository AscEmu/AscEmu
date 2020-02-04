/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Player.h"
#include "PlayerDefines.hpp"

#include "Units/Stats.h"

//\ brief: This file includes all (rewritten and confirmed) stat calculations for players
void Player::updateManaRegeneration()
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

    regenerateValue = (regenerateValue / 2) * PctPowerRegenModifier[POWER_TYPE_MANA];

    // This is per 5 seconds
    float_t mp5 = m_ModInterrMRegen / 5.0f;
    for (uint8_t i = 0; i < STAT_COUNT; ++i)
        mp5 += (m_modManaRegenFromStat[i] * getStat(i)) / 500.0f;

    auto manaWhileCastingPct = m_ModInterrMRegenPCT;
    // Cap at 100%
    if (manaWhileCastingPct > 100)
        manaWhileCastingPct = 100;

    // Make sure mana regeneration isn't negative
    if (regenerateValue < 0)
        regenerateValue = 0.0f;
    if (mp5 < 0)
        mp5 = 0.0f;

    setManaRegeneration(mp5 + regenerateValue);
    setManaRegenerationWhileCasting(mp5 + (regenerateValue * manaWhileCastingPct / 100.0f));
#elif VERSION_STRING == TBC
    // Patch 2.2.0: Any effect which triggers a change in your rate of power regeneration (Mana, Rage, Energy, Focus)
    // will now cause an immediate reward of some power at the old rate of increase, and then begin new "ticks" of power at the new rate approximately 2 seconds later.
    // This was done to improve functionality of abilities such as Evocation and Innervate so that they did not have wasted "ticks".

    // Appled note: No need to do in wotlk anymore since mana regens every 100ms and those mentioned spells work differently
    // i.e Evocation used to increase spirit for the full duration but in wotlk it energizes % of player's mana on every aura tick instead

    if (isAlive())
    {
        m_manaEnergyRegenerateTimer = REGENERATION_INTERVAL_MANA_ENERGY;
        regeneratePowers(0);
    }

    auto playerLevel = getLevel();
    if (playerLevel > DBC_STAT_LEVEL_CAP)
        playerLevel = DBC_STAT_LEVEL_CAP;

    // Get base mana regeneration value from DBC
    const auto index = (getClass() - 1) * DBC_STAT_LEVEL_CAP + playerLevel - 1;
    const auto manaPerSpiritDBC = sGtRegenMPPerSptStore.LookupEntry(index);
    //\ todo: gtOCTRegenMP.dbc seems to have base mana regen values for all classes (i.e 0.25 for paladin 1-80lvl on wotlk)
    //\ how is it used in calculations?
    //const auto baseManaRegen = sGtOCTRegenMPStore.LookupEntry(index);

    float_t baseRegen = 0.0f;
    if (manaPerSpiritDBC != nullptr)
        baseRegen = manaPerSpiritDBC->ratio;

    const auto intellect = getStat(STAT_INTELLECT);

    // From wowwiki: MP5 = 5 * (0.001 + sqrt(Int) * Spirit * Base_Regen) * 0.60 rounded up
    float_t regenerateValue = 0.001f + sqrt(static_cast<float_t>(intellect)) * spirit * baseRegen * PctPowerRegenModifier[POWER_TYPE_MANA];

    // This is per 5 seconds
    float_t mp5 = m_ModInterrMRegen / 5.0f;
    for (uint8_t i = 0; i < STAT_COUNT; ++i)
        mp5 += (m_modManaRegenFromStat[i] * getStat(i)) / 500.0f;

    auto manaWhileCastingPct = m_ModInterrMRegenPCT;
    // Cap at 100%
    if (manaWhileCastingPct > 100)
        manaWhileCastingPct = 100;

    // Make sure mana regeneration isn't negative
    if (regenerateValue < 0)
        regenerateValue = 0.0f;
    if (mp5 < 0)
        mp5 = 0.0f;

    setManaRegeneration(mp5 + regenerateValue);
    setManaRegenerationWhileCasting(mp5 + (regenerateValue * manaWhileCastingPct / 100.0f));
#elif VERSION_STRING == WotLK
    auto playerLevel = getLevel();
    if (playerLevel > DBC_STAT_LEVEL_CAP)
        playerLevel = DBC_STAT_LEVEL_CAP;

    // Get base mana regeneration value from DBC
    const auto index = (getClass() - 1) * DBC_STAT_LEVEL_CAP + playerLevel - 1;
    const auto manaPerSpiritDBC = sGtRegenMPPerSptStore.LookupEntry(index);
    //\ todo: gtOCTRegenMP.dbc seems to have base mana regen values for all classes (i.e 0.25 for paladin 1-80lvl on wotlk)
    //\ how is it used in calculations?
    //const auto baseManaRegen = sGtOCTRegenMPStore.LookupEntry(index);

    float_t baseRegen = 0.0f;
    if (manaPerSpiritDBC != nullptr)
        baseRegen = manaPerSpiritDBC->ratio;

    const auto intellect = getStat(STAT_INTELLECT);

    // From wowwiki: MP5 = 5 * (0.001 + sqrt(Int) * Spirit * Base_Regen) * 0.60 rounded up
    float_t regenerateValue = 0.001f + sqrt(static_cast<float_t>(intellect)) * spirit * baseRegen * PctPowerRegenModifier[POWER_TYPE_MANA];

    // This is per 5 seconds
    float_t mp5 = m_ModInterrMRegen / 5.0f;
    for (uint8_t i = 0; i < STAT_COUNT; ++i)
        mp5 += (m_modManaRegenFromStat[i] * getStat(i)) / 500.0f;

    auto manaWhileCastingPct = m_ModInterrMRegenPCT;
    // Cap at 100%
    if (manaWhileCastingPct > 100)
        manaWhileCastingPct = 100;

    // Make sure mana regeneration isn't negative
    if (regenerateValue < 0)
        regenerateValue = 0.0f;
    if (mp5 < 0)
        mp5 = 0.0f;

    setManaRegeneration(mp5 + regenerateValue);
    setManaRegenerationWhileCasting(mp5 + (regenerateValue * manaWhileCastingPct / 100.0f));
#elif VERSION_STRING == Cata
    auto playerLevel = getLevel();
    if (playerLevel > DBC_STAT_LEVEL_CAP)
        playerLevel = DBC_STAT_LEVEL_CAP;

    // Get base mana regeneration value from DBC
    const auto index = (getClass() - 1) * DBC_STAT_LEVEL_CAP + playerLevel - 1;
    const auto manaPerSpiritDBC = sGtRegenMPPerSptStore.LookupEntry(index);
    //\ todo: gtOCTRegenMP.dbc seems to have base mana regen values for all classes (i.e 0.25 for paladin 1-80lvl on wotlk)
    //\ how is it used in calculations?
    //const auto baseManaRegen = sGtOCTRegenMPStore.LookupEntry(index);

    float_t baseRegen = 0.0f;
    if (manaPerSpiritDBC != nullptr)
        baseRegen = manaPerSpiritDBC->ratio;

    const auto intellect = getStat(STAT_INTELLECT);

    // From wowwiki: MP5 = 5 * (0.001 + sqrt(Int) * Spirit * Base_Regen) * 0.60 rounded up
    float_t regenerateValue = 0.001f + sqrt(static_cast<float_t>(intellect)) * spirit * baseRegen * PctPowerRegenModifier[POWER_TYPE_MANA];

    // In cata 5 second rule no longer exists
    // Instead you regen 5% of your base mana without modifiers while in combat
    float_t baseCombatRegen = getBaseMana() * 0.05f;
    baseCombatRegen = (baseCombatRegen + m_ModInterrMRegen) / 5.0f;

    auto manaWhileInCombatPct = m_ModInterrMRegenPCT;
    // Cap at 100%
    if (manaWhileInCombatPct > 100)
        manaWhileInCombatPct = 100;

    setManaRegeneration(regenerateValue + baseCombatRegen);
    setManaRegenerationWhileCasting(baseCombatRegen + (regenerateValue * manaWhileInCombatPct / 100.0f));
#elif VERSION_STRING == Mop
    auto playerLevel = getLevel();
    if (playerLevel > DBC_STAT_LEVEL_CAP)
        playerLevel = DBC_STAT_LEVEL_CAP;

    // Get base mana regeneration value from DBC
    const auto index = (getClass() - 1) * DBC_STAT_LEVEL_CAP + playerLevel - 1;
    const auto manaPerSpiritDBC = sGtRegenMPPerSptStore.LookupEntry(index);
    //\ todo: gtOCTRegenMP.dbc seems to have base mana regen values for all classes (i.e 0.25 for paladin 1-80lvl on wotlk)
    //\ how is it used in calculations?
    //const auto baseManaRegen = sGtOCTRegenMPStore.LookupEntry(index);

    float_t baseRegen = 0.0f;
    if (manaPerSpiritDBC != nullptr)
        baseRegen = manaPerSpiritDBC->ratio;

    // In MOP base combat mana regen is 2% of player's total mana
    float_t baseCombatRegen = getMaxPower(POWER_TYPE_MANA) * 0.02f;

    // Combat Regen = Total Mana * 0.02 + (1.1287 * SPI * Meditation%)
    float_t regenerateValue = baseRegen * spirit * PctPowerRegenModifier[POWER_TYPE_MANA];
    regenerateValue = (regenerateValue + m_ModInterrMRegen) / 5.0f;

    auto manaWhileInCombatPct = m_ModInterrMRegenPCT;
    // Cap at 100%
    if (manaWhileInCombatPct > 100)
        manaWhileInCombatPct = 100;

    setManaRegeneration(regenerateValue + baseCombatRegen);
    setManaRegenerationWhileCasting(baseCombatRegen + (regenerateValue * manaWhileInCombatPct / 100.0f));
#endif
}
