/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellInfo.hpp"

#include "Definitions/School.hpp"
#include "Definitions/SpellEffects.hpp"
#include "Definitions/SpellEffectTarget.hpp"
#include "Definitions/SpellFamily.hpp"
#include "Definitions/SpellIsFlags.hpp"
#include "Definitions/SpellCastTargetFlags.hpp"
#include "SpellAura.hpp"
#include "SpellMgr.hpp"
#include "SpellTarget.h"
#include "Logging/Logger.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Management/Skill.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"

SpellInfo const* SpellRankInfo::getPreviousSpell() const { return previousSpell; }
SpellInfo const* SpellRankInfo::getNextSpell() const { return nextSpell; }
SpellInfo const* SpellRankInfo::getFirstSpell() const { return firstSpell; }
SpellInfo const* SpellRankInfo::getLastSpell() const { return lastSpell; }
uint8_t SpellRankInfo::getRank() const { return rank; }

SpellInfo const* SpellRankInfo::getSpellWithRank(uint8_t spellRank) const
{
    if (spellRank == 0)
        return nullptr;

    const auto* spellInfo = getFirstSpell();
    do
    {
        if (spellInfo->getRankInfo()->getRank() == spellRank)
            return spellInfo;

        spellInfo = spellInfo->getRankInfo()->getNextSpell();
    } while (spellInfo != nullptr);

    return nullptr;
}

bool SpellRankInfo::isSpellPartOfThisSpellRankChain(uint32_t spellId) const
{
    if (spellId == 0)
        return false;

    return isSpellPartOfThisSpellRankChain(sSpellMgr.getSpellInfo(spellId));
}

bool SpellRankInfo::isSpellPartOfThisSpellRankChain(SpellInfo const* providedSpellInfo) const
{
    if (providedSpellInfo == nullptr || !providedSpellInfo->hasSpellRanks())
        return false;

    const auto* const rankInfo = providedSpellInfo->getRankInfo();
    return getFirstSpell()->getId() == rankInfo->getFirstSpell()->getId() && getLastSpell()->getId() == rankInfo->getLastSpell()->getId();
}

SpellInfo::SpellInfo()
{
    for (uint8_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
        Totem[i] = 0;

    for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
    {
        Reagent[i] = 0;
        ReagentCount[i] = 0;
    }

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        Effect[i] = 0;
        EffectDieSides[i] = 0;
        EffectRealPointsPerLevel[i] = 0.0f;
        EffectBasePoints[i] = 0;
        EffectMechanic[i] = 0;
        EffectImplicitTargetA[i] = 0;
        EffectImplicitTargetB[i] = 0;
        EffectRadiusIndex[i] = 0;
        EffectApplyAuraName[i] = 0;
        EffectAmplitude[i] = 0;
        EffectMultipleValue[i] = 0.0f;
        EffectChainTarget[i] = 0;
        EffectItemType[i] = 0;
        EffectMiscValue[i] = 0;
        EffectMiscValueB[i] = 0;
        EffectTriggerSpell[i] = 0;
        EffectPointsPerComboPoint[i] = 0.0f;
        for (uint8_t u = 0; u < MAX_SPELL_EFFECTS; ++u)
            EffectSpellClassMask[i][u] = 0;
#if VERSION_STRING >= Cata
        EffectRadiusMaxIndex[i] = 0;
        EffectSpellId[i] = 0;
        EffectIndex[i] = 0;
#endif
        SpellFamilyFlags[i] = 0;
        EffectDamageMultiplier[i] = 0.0f;
        EffectBonusMultiplier[i] = 0.0f;
    }

    for (uint8_t i = 0; i < 2; ++i)
        SpellVisual[i] = 0;

#if VERSION_STRING > Classic
    for (uint8_t i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
        TotemCategory[i] = 0;
#endif
    
    // Custom values
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        EffectCustomFlag[i] = 0;
}

bool SpellInfo::hasEffect(uint32_t effect) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == effect)
            return true;
    }

    return false;
}

bool SpellInfo::hasEffectApplyAuraName(uint32_t auraType) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] != SPELL_EFFECT_APPLY_AURA && Effect[i] != SPELL_EFFECT_PERSISTENT_AREA_AURA && Effect[i] != SPELL_EFFECT_APPLY_ENEMY_AREA_AURA &&
            Effect[i] != SPELL_EFFECT_APPLY_FRIEND_AREA_AURA && Effect[i] != SPELL_EFFECT_APPLY_GROUP_AREA_AURA &&
#if VERSION_STRING >= TBC
            Effect[i] != SPELL_EFFECT_APPLY_OWNER_AREA_AURA &&
#endif
            Effect[i] != SPELL_EFFECT_APPLY_PET_AREA_AURA && Effect[i] != SPELL_EFFECT_APPLY_RAID_AREA_AURA)
            continue;

        if (EffectApplyAuraName[i] == auraType)
            return true;
    }

    return false;
}

bool SpellInfo::hasCustomFlagForEffect(uint32_t effectIndex, uint32_t flag) const
{
    if (effectIndex >= MAX_SPELL_EFFECTS)
        return false;

    if ((EffectCustomFlag[effectIndex] & flag) != 0)
        return true;

    return false;
}

bool SpellInfo::isDamagingSpell() const
{
    if (hasDamagingEffect())
        return true;

    if (hasEffect(SPELL_EFFECT_HEALTH_LEECH)           ||
        hasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS)      ||
        hasEffect(SPELL_EFFECT_ATTACK))
        return true;

    if (hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE)         ||
        hasEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_DAMAGE)     ||
        hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE_PERCENT) ||
        hasEffectApplyAuraName(SPELL_AURA_PERIODIC_POWER_BURN))
        return true;

    return false;
}

bool SpellInfo::isHealingSpell() const
{
    if (hasHealingEffect())
        return true;

    if (firstBeneficialEffect() != -1)
        return true;

    return false;
}

int SpellInfo::firstBeneficialEffect() const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (Effect[i])
        {
            case SPELL_EFFECT_HEALTH_LEECH:
            case SPELL_EFFECT_HEAL:
            case SPELL_EFFECT_HEAL_MAX_HEALTH:
                return i;
            case SPELL_EFFECT_APPLY_AURA:
            case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
            case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
            {
                switch (EffectApplyAuraName[i])
                {
                    case SPELL_AURA_PERIODIC_HEAL:
                    case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                        return i;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    return -1;
}

uint8_t SpellInfo::getFirstSchoolFromSchoolMask() const
{
    for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
    {
        if (getSchoolMask() & (1 << i))
            return i;
    }

    // This should not happen
    return SCHOOL_NORMAL;
}

bool SpellInfo::isDamagingEffect(uint8_t effIndex) const
{
    if (effIndex < MAX_SPELL_EFFECTS)
    {
        if (getEffect(effIndex) == SPELL_EFFECT_SCHOOL_DAMAGE ||
            getEffect(effIndex) == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE ||
            getEffect(effIndex) == SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL ||
            getEffect(effIndex) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE ||
            getEffect(effIndex) == SPELL_EFFECT_WEAPON_DAMAGE ||
            getEffect(effIndex) == SPELL_EFFECT_POWER_BURN)
            return true;
        return false;
    }
    else
    {
        sLogger.failure("SpellInfo::isDamagingEffect called with invalid effIndex {}", static_cast<uint32_t>(effIndex));
        return false;
    }
}

bool SpellInfo::isHealingEffect(uint8_t effIndex) const
{
    if (effIndex < MAX_SPELL_EFFECTS)
    {
        if (getEffect(effIndex) == SPELL_EFFECT_HEAL ||
            getEffect(effIndex) == SPELL_EFFECT_HEAL_MAX_HEALTH ||
            getEffect(effIndex) == SPELL_EFFECT_HEAL_MECHANICAL)
            return true;

#if VERSION_STRING == Classic
        // In classic these spells have SPELL_EFFECT_SCRIPT_EFFECT instead of heal effect
        switch (Id)
        {
        case 635:   // Holy Light Rank 1
        case 639:   // Holy Light Rank 2
        case 647:   // Holy Light Rank 3
        case 1026:  // Holy Light Rank 4
        case 1042:  // Holy Light Rank 5
        case 3472:  // Holy Light Rank 6
        case 10328: // Holy Light Rank 7
        case 10329: // Holy Light Rank 8
        case 25292: // Holy Light Rank 9
        case 19750: // Flash of Light Rank 1
        case 19939: // Flash of Light Rank 2
        case 19940: // Flash of Light Rank 3
        case 19941: // Flash of Light Rank 4
        case 19942: // Flash of Light Rank 5
        case 19943: // Flash of Light Rank 6
            return true;
        default:
            break;
        }
#endif
        return false;
    }
    else
    {
        sLogger.failure("SpellInfo::isHealingEffect called with invalid effIndex {}", static_cast<uint32_t>(effIndex));
        return false;
    }
}

bool SpellInfo::hasDamagingEffect() const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (isDamagingEffect(i))
            return true;
    }
    return false;
}

bool SpellInfo::hasHealingEffect() const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (isHealingEffect(i))
            return true;
    }
    return false;
}

bool SpellInfo::isAffectingSpell(SpellInfo const* spellInfo) const
{
    if (spellInfo == nullptr)
        return false;

    if (spellInfo->SpellFamilyName != SpellFamilyName)
        return false;

    // TODO: this always returns false on classic and TBC since EffectSpellClassMask field does not exist there
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (isEffectIndexAffectingSpell(i, spellInfo))
            return true;
    }

    return false;
}

bool SpellInfo::isEffectIndexAffectingSpell(uint8_t effIndex, SpellInfo const* spellInfo) const
{
    // It's not spell effect count, it's spell mask field count
    for (uint8_t i = 0; i < 3; ++i)
    {
        // If any of the indexes contain same mask, the spells affect each other
        if (EffectSpellClassMask[effIndex][i] > 0 && (EffectSpellClassMask[effIndex][i] & spellInfo->SpellFamilyFlags[i]))
            return true;
    }

    return false;
}

bool SpellInfo::isAuraEffectAffectingSpell(AuraEffect auraEffect, SpellInfo const* spellInfo) const
{
    if (spellInfo->SpellFamilyName != SpellFamilyName)
        return false;

    uint8_t effIndex = 255;

    // Find effect index for the aura effect
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getEffectApplyAuraName(i) == auraEffect)
        {
            effIndex = i;
            break;
        }
    }

    // Aura did not have this aura effect
    if (effIndex == 255)
        return false;

    return isEffectIndexAffectingSpell(effIndex, spellInfo);
}

bool SpellInfo::hasValidPowerType() const
{
    return getPowerType() < TOTAL_PLAYER_POWER_TYPES;
}

int32_t SpellInfo::getBasePowerCost(Unit* caster) const
{
    if (caster == nullptr)
        return 0;

    int32_t powerCost = 0;
    if (getAttributesEx() & ATTRIBUTESEX_DRAIN_WHOLE_POWER)
    {
        if (!hasValidPowerType())
        {
            sLogger.failure("SpellInfo::getBasePowerCost : Unknown power type {} for spell id {}", getPowerType(), getId());
            return 0;
        }

        if (getPowerType() == POWER_TYPE_HEALTH)
            powerCost = static_cast<int32_t>(caster->getHealth());
        else
            powerCost = static_cast<int32_t>(caster->getPower(getPowerType()));
    }
    else
    {
        powerCost = getManaCost();
        // Check if spell costs percentage of caster's power
        if (getManaCostPercentage() > 0)
        {
            switch (getPowerType())
            {
                case POWER_TYPE_HEALTH:
                    powerCost += static_cast<int32_t>(caster->getBaseHealth() * getManaCostPercentage() / 100);
                    break;
                case POWER_TYPE_MANA:
                    powerCost += static_cast<int32_t>(caster->getBaseMana() * getManaCostPercentage() / 100);
                    break;
                case POWER_TYPE_RAGE:
                case POWER_TYPE_FOCUS:
                case POWER_TYPE_ENERGY:
#if VERSION_STRING < Cata
                case POWER_TYPE_HAPPINESS:
#endif
                    powerCost += static_cast<int32_t>(caster->getMaxPower(getPowerType()) * getManaCostPercentage() / 100);
                    break;
#if VERSION_STRING >= WotLK
                case POWER_TYPE_RUNES:
                case POWER_TYPE_RUNIC_POWER:
                    // In 3.3.5a only obsolete spells use these and have a non-null getManaCostPercentage
                    break;
#endif
                default:
                    sLogger.failure("SpellInfo::getBasePowerCost() : Unknown power type {} for spell id {}", getPowerType(), getId());
                    return 0;
            }
        }
    }

    return powerCost;
}

bool SpellInfo::isNegativeAura() const
{
    if (custom_c_is_flags & SPELL_FLAG_IS_FORCEDDEBUFF)
        return true;

    if (custom_c_is_flags & SPELL_FLAG_IS_FORCEDBUFF)
        return false;

    if (getAttributes() & ATTRIBUTES_NEGATIVE)
        return true;

    // Custom checks based on spell family name to override default result from below
    switch (getSpellFamilyName())
    {
        case SPELLFAMILY_WARRIOR:
        {
#if VERSION_STRING >= TBC
            // Death Wish was made positive in late TBC
            if (getSpellFamilyFlags(0) == 0x100000)
                return false;
#endif
            // Recklessness should be positive
            if (getSpellFamilyFlags(0) == 0x10)
                return false;
        } break;
        case SPELLFAMILY_PALADIN:
        {
            // Divine Shield should be positive
            if (getSpellFamilyFlags(0) == 0x400000)
                return false;

            // Judgement effects should be negative
            if (getSpellFamilyFlags(0) == 0x80000)
                return true;
        } break;
        default:
            break;
    }

    // Custom checks based on spell id to override default result from below
    // Use id only if spell has no family flags
    switch (getId())
    {
        // Deathbringer Saurfang - Mark of the Fallen Champion
        case 72293:
        // Deathbringer Saurfang - Rune of Blood
        case 72410:
        // Lady Deathwhisper trash - Darkreckoning
        case 69483:
        // Trial Of Champion - Dreadscale and Acidmaw - Burning Bile
        case 66869:
            // These should be negative
            return true;
        default:
            break;
    }

    // Check each effect
    // TODO: missing cata and mop effects
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getEffectApplyAuraName(i) == SPELL_AURA_NONE)
            continue;

        // Using just spell base points could give false results
        const auto effValue = calculateEffectValue(i);
        switch (getEffectApplyAuraName(i))
        {
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_MOD_CONFUSE:
            case SPELL_AURA_MOD_CHARM:
            case SPELL_AURA_MOD_FEAR:
            case SPELL_AURA_MOD_STUN:
            case SPELL_AURA_MOD_PACIFY:
            case SPELL_AURA_MOD_ROOT:
            case SPELL_AURA_MOD_SILENCE:
            case SPELL_AURA_PERIODIC_LEECH:
            case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
            case SPELL_AURA_PERIODIC_MANA_LEECH:
            case SPELL_AURA_MOD_DISARM:
            case SPELL_AURA_MOD_STALKED:
            case SPELL_AURA_CHANNEL_DEATH_ITEM:
            case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            case SPELL_AURA_PREVENTS_FLEEING:
            case SPELL_AURA_GHOST:
            case SPELL_AURA_PERIODIC_POWER_BURN:
            case SPELL_AURA_AREA_CHARM:
#if VERSION_STRING >= TBC
            case SPELL_AURA_MOD_DISARM_OFFHAND:
#endif
#if VERSION_STRING >= WotLK
            case SPELL_AURA_MOD_DISARM_RANGED:
            case SPELL_AURA_298:
            case SPELL_AURA_301:
            case SPELL_AURA_PREVENT_RESURRECTION:
#endif
                // No need to do other checks, definitely negative
                return true;
            case SPELL_AURA_MOD_ATTACKSPEED:
            case SPELL_AURA_MOD_DAMAGE_DONE:
            case SPELL_AURA_MOD_RESISTANCE:
            case SPELL_AURA_MOD_STAT:
            case SPELL_AURA_MOD_SKILL:
            case SPELL_AURA_MOD_INCREASE_SPEED:
            case SPELL_AURA_MOD_DECREASE_SPEED:
            case SPELL_AURA_MOD_INCREASE_HEALTH:
            case SPELL_AURA_MOD_INCREASE_ENERGY:
            case SPELL_AURA_MOD_PARRY_PERCENT:
            case SPELL_AURA_MOD_DODGE_PERCENT:
            case SPELL_AURA_MOD_BLOCK_PERCENT:
            case SPELL_AURA_MOD_CRIT_PERCENT:
            case SPELL_AURA_MOD_HIT_CHANCE:
            case SPELL_AURA_MOD_SPELL_HIT_CHANCE:
            case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
            case SPELL_AURA_MOD_PACIFY_SILENCE:
            case SPELL_AURA_MOD_CASTING_SPEED:
            case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
            case SPELL_AURA_MOD_PERCENT_STAT:
            case SPELL_AURA_MOD_ATTACK_POWER:
            case SPELL_AURA_MOD_RESISTANCE_PCT:
            case SPELL_AURA_MOD_HEALING:
            case SPELL_AURA_MOD_HEALING_PCT:
            case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
            case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
            case SPELL_AURA_MOD_HEALING_DONE_PERCENT:
            case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
            case SPELL_AURA_MOD_HASTE:
            case SPELL_AURA_MOD_RANGED_HASTE:
            case SPELL_AURA_MOD_ATTACK_POWER_PCT:
#if VERSION_STRING >= TBC
            case SPELL_AURA_MELEE_SLOW_PCT:
            case SPELL_AURA_INCREASE_TIME_BETWEEN_ATTACKS:
            case SPELL_AURA_INCREASE_CASTING_TIME_PCT:
            case SPELL_AURA_252:
            case SPELL_AURA_259:
#endif
                // Negative if effect value is negative
                if (effValue < 0)
                    return true;
                break;
            case SPELL_AURA_MOD_DAMAGE_TAKEN:
            case SPELL_AURA_MOD_POWER_COST:
            case SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN:
#if VERSION_STRING >= TBC
            case SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT:
#endif
#if VERSION_STRING >= WotLK
            case SPELL_AURA_INCREASE_SPELL_DOT_DAMAGE_PCT:
#endif
                // Negative if effect value is positive
                if (effValue > 0)
                    return true;
                break;
            case SPELL_AURA_MOD_STEALTH:
            case SPELL_AURA_MOD_INVISIBILITY:
            case SPELL_AURA_EFFECT_IMMUNITY:
            case SPELL_AURA_STATE_IMMUNITY:
            case SPELL_AURA_SCHOOL_IMMUNITY:
            case SPELL_AURA_DAMAGE_IMMUNITY:
            case SPELL_AURA_DISPEL_IMMUNITY:
            case SPELL_AURA_SCHOOL_ABSORB:
            case SPELL_AURA_FAR_SIGHT:
            case SPELL_AURA_MECHANIC_IMMUNITY:
            case SPELL_AURA_MOUNTED:
                // No need to do other checks, definitely positive
                return false;
            default:
                break;
        }
    }

    return false;
}

uint32_t SpellInfo::getSpellDefaultDuration(Unit const* caster) const
{
    const auto spell_duration = sSpellDurationStore.lookupEntry(DurationIndex);
    if (spell_duration == nullptr)
        return 0;

    if (caster == nullptr)
        return spell_duration->Duration1;

    const int32_t ret = spell_duration->Duration1 + (spell_duration->Duration2 * caster->getLevel());
    if (ret > spell_duration->Duration3)
        return spell_duration->Duration3;

    return ret;
}

bool SpellInfo::hasTargetType(uint32_t type) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_NULL)
            continue;

        if (EffectImplicitTargetA[i] == type ||
            EffectImplicitTargetB[i] == type)
            return true;
    }

    return false;
}

uint32_t SpellInfo::getRequiredTargetMaskForEffectTarget(uint32_t implicitTarget, uint8_t effectIndex) const
{
    uint32_t targetMask = 0;
    switch (implicitTarget)
    {
        case EFF_TARGET_NONE:
            // If effect has no implicit target, set one based on effect type
            switch (Effect[effectIndex])
            {
                case SPELL_EFFECT_DODGE:
                case SPELL_EFFECT_EVADE:
                case SPELL_EFFECT_PARRY:
                case SPELL_EFFECT_BLOCK:
                case SPELL_EFFECT_WEAPON:
                case SPELL_EFFECT_DEFENSE:
                case SPELL_EFFECT_ENERGIZE:
                case SPELL_EFFECT_TRANSFORM_ITEM:
                case SPELL_EFFECT_SPELL_DEFENSE:
                case SPELL_EFFECT_LANGUAGE:
                case SPELL_EFFECT_SPAWN:
                case SPELL_EFFECT_TRADE_SKILL:
                case SPELL_EFFECT_STEALTH:
                case SPELL_EFFECT_DETECT:
                case SPELL_EFFECT_FORCE_CRITICAL_HIT:
                case SPELL_EFFECT_GUARANTEE_HIT:
                case SPELL_EFFECT_PROFICIENCY:
                case SPELL_EFFECT_USE_GLYPH:
                case SPELL_EFFECT_ATTACK:
                case SPELL_EFFECT_SANCTUARY:
                case SPELL_EFFECT_STUCK:
                case SPELL_EFFECT_SUMMON_PHANTASM:
                case SPELL_EFFECT_SELF_RESURRECT:
                case SPELL_EFFECT_SUMMON_MULTIPLE_TOTEMS:
                case SPELL_EFFECT_DESTROY_ALL_TOTEMS:
                case SPELL_EFFECT_SKILL:
#if VERSION_STRING >= TBC
                case SPELL_EFFECT_UNKNOWN_131:
                case SPELL_EFFECT_KILL_CREDIT:
#if VERSION_STRING >= WotLK
                case SPELL_EFFECT_DUAL_WIELD_2H:
#endif
#endif
                    targetMask = SPELL_TARGET_OBJECT_SELF;
                    break;
                // TODO: possibly wotlk only
                case SPELL_EFFECT_SUMMON_GUARDIAN:
#if VERSION_STRING >= Cata
                case SPELL_EFFECT_UNKNOWN_171:
                case SPELL_EFFECT_UNKNOWN_179:
#endif
                    targetMask = SPELL_TARGET_AREA;
                    break;
                default:
                    break;
            }
            break;
        case EFF_TARGET_SELF:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS:
            targetMask = SPELL_TARGET_REQUIRE_FRIENDLY;
            break;
        case 4:
            targetMask = SPELL_TARGET_AREA_SELF | SPELL_TARGET_REQUIRE_FRIENDLY;
            break;
        case EFF_TARGET_PET:
            targetMask = SPELL_TARGET_OBJECT_CURPET;
            break;
        case EFF_TARGET_SINGLE_ENEMY:
            targetMask = SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_SCRIPTED_TARGET:
            targetMask = SPELL_TARGET_OBJECT_SCRIPTED;
            break;
        case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS:
            targetMask = SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_ALL_ENEMY_IN_AREA:
            targetMask = SPELL_TARGET_AREA_SELF | SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT:
            targetMask = SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        // todo: confirm this
        /*case EFF_TARGET_TELEPORT_LOCATION:
            targetMask = SPELL_TARGET_AREA;
            break;*/
        case EFF_TARGET_LOCATION_TO_SUMMON:
            targetMask = SPELL_TARGET_NO_OBJECT;
            break;
        case EFF_TARGET_ALL_PARTY_AROUND_CASTER:
            targetMask = SPELL_TARGET_AREA_PARTY;
            break;
        case EFF_TARGET_SINGLE_FRIEND:
            targetMask = SPELL_TARGET_REQUIRE_FRIENDLY;
            break;
        case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
            targetMask = SPELL_TARGET_AREA_SELF;
            break;
        case EFF_TARGET_GAMEOBJECT:
            targetMask = SPELL_TARGET_REQUIRE_GAMEOBJECT;
            break;
        case EFF_TARGET_IN_FRONT_OF_CASTER:
            targetMask = SPELL_TARGET_AREA_CONE | SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_DUEL:
            targetMask = SPELL_TARGET_ANY_OBJECT;
            break;
        case EFF_TARGET_GAMEOBJECT_ITEM:
            targetMask = SPELL_TARGET_REQUIRE_GAMEOBJECT | SPELL_TARGET_REQUIRE_ITEM;
            break;
        case EFF_TARGET_PET_MASTER:
            targetMask = SPELL_TARGET_OBJECT_PETOWNER;
            break;
        case EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
            targetMask = SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED:
            targetMask = SPELL_TARGET_OBJECT_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_SELF;
            break;
        case EFF_TARGET_ALL_FRIENDLY_IN_AREA:
            targetMask = SPELL_TARGET_REQUIRE_FRIENDLY;
            break;
        case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME:
            targetMask = SPELL_TARGET_REQUIRE_FRIENDLY | SPELL_TARGET_AREA;
            break;
        // todo: confirm this
        /*case EFF_TARGET_MINION:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;*/
        case EFF_TARGET_ALL_PARTY_IN_AREA:
            targetMask = SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_PARTY;
            break;
        case EFF_TARGET_SINGLE_PARTY:
            targetMask = SPELL_TARGET_AREA_PARTY;
            break;
        case EFF_TARGET_PET_SUMMON_LOCATION:
            targetMask = SPELL_TARGET_OBJECT_SCRIPTED;
            break;
        case EFF_TARGET_ALL_PARTY:
            targetMask = SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_PARTY | SPELL_TARGET_AREA_RAID;
            break;
        case EFF_TARGET_SELF_FISHING:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_SCRIPTED_GAMEOBJECT:
            targetMask = SPELL_TARGET_OBJECT_SCRIPTED;
            break;
        case EFF_TARGET_TOTEM_EARTH:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_TOTEM_WATER:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_TOTEM_AIR:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_TOTEM_FIRE:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_CHAIN:
            targetMask = SPELL_TARGET_AREA_CHAIN | SPELL_TARGET_REQUIRE_FRIENDLY;
            break;
        case EFF_TARGET_SCIPTED_OBJECT_LOCATION:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_DYNAMIC_OBJECT:
            targetMask = SPELL_TARGET_NO_OBJECT; //dont fill target map for this (fucks up some spell visuals)
            break;
        case EFF_TARGET_MULTIPLE_SUMMON_LOCATION:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_MULTIPLE_SUMMON_PET_LOCATION:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_SUMMON_LOCATION:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_LOCATION_NEAR_CASTER:
            targetMask = SPELL_TARGET_AREA | SPELL_TARGET_REQUIRE_GAMEOBJECT | SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case 34:
            targetMask = SPELL_TARGET_NOT_IMPLEMENTED; //seige stuff
            break;
        case EFF_TARGET_CURRENT_SELECTION:
            targetMask = SPELL_TARGET_AREA_CURTARGET | SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER:
            targetMask = SPELL_TARGET_AREA_CONE | SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_ALL_RAID:
            targetMask = SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_RAID; //used by commanding shout, targets raid now
            break;
        case EFF_TARGET_PARTY_MEMBER:
            targetMask = SPELL_TARGET_REQUIRE_FRIENDLY | SPELL_TARGET_AREA_PARTY;
            break;
        case EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS:
            targetMask = SPELL_TARGET_AREA_SELF | SPELL_TARGET_AREA_RAID | SPELL_TARGET_OBJECT_TARCLASS | SPELL_TARGET_REQUIRE_FRIENDLY;
            break;
        case EFF_TARGET_NATURE_SUMMON_LOCATION:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case 64:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_BEHIND_TARGET_LOCATION:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case 66:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case 67:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case 69:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_MULTIPLE_GUARDIAN_SUMMON_LOCATION:
            targetMask = SPELL_TARGET_AREA_RANDOM;
            break;
        case EFF_TARGET_NETHETDRAKE_SUMMON_LOCATION:
            targetMask = SPELL_TARGET_OBJECT_SELF;
            break;
        case EFF_TARGET_ENEMIES_IN_AREA_CHANNELED_WITH_EXCEPTIONS:
            targetMask = SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_SELECTED_ENEMY_CHANNELED:
            targetMask = SPELL_TARGET_REQUIRE_ATTACKABLE;
            break;
        case EFF_TARGET_SELECTED_ENEMY_DEADLY_POISON:
            targetMask = SPELL_TARGET_AREA_RANDOM;
            break;
        case EFF_TARGET_AREA_DESTINATION:
            targetMask = SPELL_TARGET_AREA;
            break;
        case EFF_TARGET_NON_COMBAT_PET:
            targetMask = SPELL_TARGET_OBJECT_CURCRITTER;
            break;
        case EFF_TARGET_CONE_IN_FRONT:
            targetMask = SPELL_TARGET_REQUIRE_ATTACKABLE | SPELL_TARGET_AREA_CONE;
            break;
        default:
            break;
    }

    // Check if spell chains
    if (getEffectChainTarget(effectIndex) > 0)
        targetMask |= SPELL_TARGET_AREA_CHAIN;

    return targetMask;
}

uint32_t SpellInfo::getRequiredTargetMaskForEffect(uint8_t effectIndex, bool getExplicitMask/* = false*/) const
{
    auto targetMask = getRequiredTargetMaskForEffectTarget(getEffectImplicitTargetA(effectIndex), effectIndex);

    // Do not get target mask from B if it's not set
    if (getEffectImplicitTargetB(effectIndex) != EFF_TARGET_NONE)
        targetMask |= getRequiredTargetMaskForEffectTarget(getEffectImplicitTargetB(effectIndex), effectIndex);

    // Add spell cast target flags to mask
    if (Targets != 0)
    {
        if (Targets & TARGET_FLAG_UNIT)
            targetMask |= SPELL_TARGET_REQUIRE_ATTACKABLE;
        if (Targets & TARGET_FLAG_ITEM)
            targetMask |= SPELL_TARGET_REQUIRE_ITEM;
        if (Targets & TARGET_FLAG_SOURCE_LOCATION)
            targetMask |= SPELL_TARGET_AREA_SELF;
        if (Targets & TARGET_FLAG_DEST_LOCATION)
            targetMask |= SPELL_TARGET_AREA;
        // TODO: confirm this
        /*if (Targets & TARGET_FLAG_UNK8)
            targetMask |= SPELL_TARGET_REQUIRE_ATTACKABLE;*/
        if (Targets & TARGET_FLAG_UNIT_CASTER)
            targetMask |= SPELL_TARGET_OBJECT_SELF;
        if (Targets & TARGET_FLAG_CORPSE)
            targetMask |= SPELL_TARGET_REQUIRE_ATTACKABLE;
        if (Targets & TARGET_FLAG_UNIT_CORPSE)
            targetMask |= SPELL_TARGET_REQUIRE_ATTACKABLE;
        if (Targets & TARGET_FLAG_OBJECT)
            targetMask |= SPELL_TARGET_REQUIRE_GAMEOBJECT;
        if (Targets & TARGET_FLAG_OPEN_LOCK)
            targetMask |= SPELL_TARGET_REQUIRE_GAMEOBJECT;
        if (Targets & TARGET_FLAG_CORPSE2)
            targetMask |= SPELL_TARGET_REQUIRE_FRIENDLY;
    }

    // Remove explicit object target masks if spell has no max range
    if (getExplicitMask)
    {
        const auto rangeEntry = sSpellRangeStore.lookupEntry(getRangeIndex());
        if (rangeEntry != nullptr)
        {
#if VERSION_STRING >= WotLK
            if (rangeEntry->maxRangeFriendly == 0.0f && rangeEntry->maxRange == 0.0f)
#else
            if (rangeEntry->maxRange == 0.0f)
#endif
            {
                targetMask &= ~(SPELL_TARGET_REQUIRE_GAMEOBJECT | SPELL_TARGET_REQUIRE_UNIT);
            }
        }
    }

    return targetMask;
}

uint32_t SpellInfo::getRequiredTargetMask(bool getExplicitMask) const
{
    uint32_t fullMask = 0;
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getEffect(i) == SPELL_EFFECT_NULL)
            continue;

        fullMask |= getRequiredTargetMaskForEffect(i, getExplicitMask);
    }

    return fullMask;
}

int SpellInfo::aiTargetType() const
{
    /*  this is not good as one spell effect can target self and other one an enemy,
    maybe we should make it for each spell effect or use as flags */
    if (hasTargetType(EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS) ||
        hasTargetType(EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS)       ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA)                              ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT)                      ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED)                    ||
        hasTargetType(EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME))
        return TTYPE_DESTINATION;

    if (hasTargetType(EFF_TARGET_LOCATION_TO_SUMMON)      ||
        hasTargetType(EFF_TARGET_IN_FRONT_OF_CASTER)      ||
        hasTargetType(EFF_TARGET_ALL_FRIENDLY_IN_AREA)    ||
        hasTargetType(EFF_TARGET_PET_SUMMON_LOCATION)     ||
        hasTargetType(EFF_TARGET_LOCATION_INFRONT_CASTER) ||
        hasTargetType(EFF_TARGET_CONE_IN_FRONT))
        return TTYPE_SOURCE;

    if (hasTargetType(EFF_TARGET_SINGLE_ENEMY)                      ||
        hasTargetType(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER)         ||
        hasTargetType(EFF_TARGET_DUEL)                              ||
        hasTargetType(EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET)         ||
        hasTargetType(EFF_TARGET_CHAIN)                             ||
        hasTargetType(EFF_TARGET_CURRENT_SELECTION)                 ||
        hasTargetType(EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER)   ||
        hasTargetType(EFF_TARGET_MULTIPLE_GUARDIAN_SUMMON_LOCATION) ||
        hasTargetType(EFF_TARGET_SELECTED_ENEMY_CHANNELED))
        return TTYPE_SINGLETARGET;

    if (hasTargetType(EFF_TARGET_ALL_PARTY_AROUND_CASTER)     ||
        hasTargetType(EFF_TARGET_SINGLE_FRIEND)               ||
        hasTargetType(EFF_TARGET_PET_MASTER)                  ||
        hasTargetType(EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED) ||
        hasTargetType(EFF_TARGET_ALL_PARTY_IN_AREA)           ||
        hasTargetType(EFF_TARGET_SINGLE_PARTY)                ||
        hasTargetType(EFF_TARGET_ALL_PARTY)                   ||
        hasTargetType(EFF_TARGET_ALL_RAID)                    ||
        hasTargetType(EFF_TARGET_PARTY_MEMBER)                ||
        hasTargetType(EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS))
        return TTYPE_OWNER;

    if (hasTargetType(EFF_TARGET_SELF) ||
        hasTargetType(4) ||
        hasTargetType(EFF_TARGET_PET) ||
        hasTargetType(EFF_TARGET_MINION))
        return TTYPE_CASTER;

    return TTYPE_NULL;
}

bool SpellInfo::isTargetingStealthed() const
{
    if (hasTargetType(EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS) ||
        hasTargetType(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER) ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED) ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT))
        return true;

    switch (Id)
    {
                    // SPELL_HASH_MAGMA_TOTEM
        case 8187:  // Magma Totem Rank 1
        case 8190:  // Magma Totem Rank 1
        case 10579: // Magma Totem Rank 2
        case 10580: // Magma Totem Rank 3
        case 10581: // Magma Totem Rank 4
        case 10585: // Magma Totem Rank 2
        case 10586: // Magma Totem Rank 3
        case 10587: // Magma Totem Rank 4
        case 25550: // Magma Totem Rank 5
        case 25552: // Magma Totem Rank 5
        case 58731: // Magma Totem Rank 6
        case 58732: // Magma Totem Rank 6
        case 58734: // Magma Totem Rank 7
        case 58735: // Magma Totem Rank 7
            return true;
        default:
            break;
    }

    return false;
}

bool SpellInfo::isRequireCooldownSpell() const
{
    auto cond1 = Attributes & ATTRIBUTES_TRIGGER_COOLDOWN && AttributesEx & ATTRIBUTESEX_NOT_BREAK_STEALTH;
    auto cond2 = Attributes & ATTRIBUTES_TRIGGER_COOLDOWN && (!AttributesEx || AttributesEx & ATTRIBUTESEX_REMAIN_OOC);

    return cond1 || cond2;
}

bool SpellInfo::isPassive() const
{
    return (Attributes & ATTRIBUTES_PASSIVE) != 0;
}

bool SpellInfo::isProfession(bool checkRiding/* = false*/) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            const auto skill = static_cast<uint32_t>(EffectMiscValue[i]);

            //Profession skill
            if (skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID)
                return true;

            if (checkRiding && skill == SKILL_RIDING)
                return true;

            if (isPrimaryProfessionSkill(skill))
                return true;
        }
    }
    return false;
}

bool SpellInfo::isPrimaryProfession() const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            const auto skill = static_cast<uint32_t>(EffectMiscValue[i]);
            if (isPrimaryProfessionSkill(skill))
                return true;
        }
    }
    return false;
}

bool SpellInfo::isPrimaryProfessionSkill(uint32_t skill_id) const
{
    if (const auto skill_line = sSkillLineStore.lookupEntry(skill_id))
    {
        if (skill_line && skill_line->type == SKILL_TYPE_PROFESSION)
            return true;
    }

    return false;
}

bool SpellInfo::isTalent() const
{
    return m_isTalent;
}

bool SpellInfo::isPetTalent() const
{
    return m_isPetTalent;
}

bool SpellInfo::isAllowingDeadTarget() const
{
    return hasAttribute(ATTRIBUTESEXB_CAN_BE_CASTED_ON_DEAD_TARGET) || Targets & (SpellCastTargetFlags::TARGET_FLAG_CORPSE | SpellCastTargetFlags::TARGET_FLAG_CORPSE2 | SpellCastTargetFlags::TARGET_FLAG_UNIT_CORPSE);
}

bool SpellInfo::isDeathPersistent() const
{
    return (AttributesExC & ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD) != 0;
}

bool SpellInfo::isChanneled() const
{
    return (AttributesEx & (ATTRIBUTESEX_CHANNELED_1 | ATTRIBUTESEX_CHANNELED_2)) != 0;
}

bool SpellInfo::isRangedAutoRepeat() const
{
    return (AttributesExB & ATTRIBUTESEXB_AUTOREPEAT) != 0;
}

bool SpellInfo::isOnNextMeleeAttack() const
{
    return (Attributes & (ATTRIBUTES_ON_NEXT_ATTACK | ATTRIBUTES_ON_NEXT_SWING_2)) != 0;
}

bool SpellInfo::isStackableFromMultipleCasters() const
{
    return getMaxstack() > 1 && !isChanneled() && !(getAttributesExC() & ATTRIBUTESEXC_APPLY_OWN_STACK_FOR_EACH_CASTER);
}

bool SpellInfo::hasSpellRanks() const
{
    return m_spellRankInfo.has_value();
}

SpellRankInfo const* SpellInfo::getRankInfo() const
{
    if (!hasSpellRanks())
        return nullptr;

    return &m_spellRankInfo.value();
}

bool SpellInfo::canKnowOnlySingleRank() const
{
    // Passive spells or spells without mana cost should have only one rank known
    if (isPassive() || (getPowerType() != POWER_TYPE_MANA && getPowerType() != POWER_TYPE_HEALTH))
        return true;

    // Profession skills should have only one rank known
    if (isProfession(true))
        return true;

    const auto isSpellAutoLearnedFromSkill = [](uint32_t spellId) -> bool
    {
        const auto spellRange = sSpellMgr.getSkillEntryRangeForSpell(spellId);
        for (const auto& [_, skillEntry] : spellRange)
        {
            if (skillEntry->acquireMethod != 1)
                continue;
            if (skillEntry->minSkillLineRank > 0)
                return true;
        }

        return false;
    };

    // Spells that are auto learned from a skill with certain skill level should have only one rank known
    if (isSpellAutoLearnedFromSkill(getId()))
        return true;

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getEffect(i) == SPELL_EFFECT_NULL)
            continue;

        if (getSpellFamilyName() == SPELLFAMILY_DRUID)
        {
            // Druid shapeshift spells should have only one rank known
            if (getEffect(i) == SPELL_EFFECT_APPLY_AURA && getEffectApplyAuraName(i) == SPELL_AURA_MOD_SHAPESHIFT)
                return true;
        }
        else if (getSpellFamilyName() == SPELLFAMILY_PALADIN)
        {
            // Paladin auras should have only one rank known
            if (getEffect(i) == SPELL_EFFECT_APPLY_RAID_AREA_AURA ||
                getEffect(i) == SPELL_EFFECT_APPLY_GROUP_AREA_AURA)
                return true;
        }
    }

    return false;
}

int32_t SpellInfo::calculateEffectValue(uint8_t effIndex, Unit* unitCaster/* = nullptr*/, Item* itemCaster/* = nullptr*/, SpellForcedBasePoints forcedBasePoints/* = SpellForcedBasePoints()*/) const
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return 0;

    const float_t basePointsPerLevel = getEffectRealPointsPerLevel(effIndex);
    const auto randomPoints = getEffectDieSides(effIndex);
    int32_t basePoints = 0;

    // Random suffix value calculation
    if (itemCaster != nullptr && static_cast<int32_t>(itemCaster->getRandomPropertiesId()) < 0)
    {
        const auto randomSuffix = sItemRandomSuffixStore.lookupEntry(std::abs(static_cast<int32_t>(itemCaster->getRandomPropertiesId())));
        if (randomSuffix != nullptr)
        {
            auto faulty = false;
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (randomSuffix->enchantments[i] == 0)
                    continue;

                const auto spellItemEnchant = sSpellItemEnchantmentStore.lookupEntry(randomSuffix->enchantments[i]);
                if (spellItemEnchant == nullptr)
                    continue;

                for (uint8_t j = 0; j < 3; ++j)
                {
                    if (spellItemEnchant->spell[j] != getId())
                        continue;

                    if (randomSuffix->prefixes[j] == 0)
                    {
                        faulty = true;
                        break;
                    }

                    basePoints = RANDOM_SUFFIX_MAGIC_CALCULATION(randomSuffix->prefixes[i], itemCaster->getPropertySeed());
                    if (basePoints == 0)
                    {
                        faulty = true;
                        break;
                    }

                    // Value OK
                    return basePoints;
                }

                if (faulty)
                    break;
            }
        }
    }

#if VERSION_STRING >= Cata
    basePoints = getEffectBasePoints(effIndex);
#else
    basePoints = getEffectBasePoints(effIndex) + 1;
#endif

    forcedBasePoints.get(effIndex, &basePoints);

    // Check if value increases with level
    if (unitCaster != nullptr)
    {
        auto diff = -static_cast<int32_t>(getBaseLevel());
        if (getMaxLevel() != 0 && unitCaster->getLevel() > getMaxLevel())
            diff += getMaxLevel();
        else
            diff += unitCaster->getLevel();

        diff = Util::float2int32(diff * basePointsPerLevel);
        // Should not happen but just in case do not make total value negative
        if (diff > 0)
            basePoints += diff;
    }

    if (randomPoints > 1)
        basePoints += Util::getRandomInt(randomPoints);

    // Check if value increases with combo points
    const auto comboDamage = getEffectPointsPerComboPoint(effIndex);
    if (comboDamage > 0.0f && unitCaster != nullptr && unitCaster->isPlayer())
    {
        const auto plrCaster = static_cast<Player*>(unitCaster);
        basePoints += static_cast<int32_t>(std::round(comboDamage * plrCaster->getComboPoints()));
    }

    return basePoints;
}

bool SpellInfo::doesEffectApplyAura(uint8_t effIndex) const
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return false;

    return Effect[effIndex] == SPELL_EFFECT_APPLY_AURA || isAreaAuraEffect(effIndex);
}

bool SpellInfo::isAreaAuraEffect(uint8_t effIndex) const
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return false;

    switch (Effect[effIndex])
    {
        case SPELL_EFFECT_PERSISTENT_AREA_AURA:
        case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
        case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
        case SPELL_EFFECT_APPLY_PET_AREA_AURA:
        case SPELL_EFFECT_APPLY_FRIEND_AREA_AURA:
        case SPELL_EFFECT_APPLY_ENEMY_AREA_AURA:
#if VERSION_STRING >= TBC
        case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
#endif
            return true;
        default:
            break;
    }

    return false;
}

bool SpellInfo::appliesAreaAura(uint32_t auraType) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (!isAreaAuraEffect(i))
            continue;

        if (EffectApplyAuraName[i] == auraType)
            return true;
    }

    return false;
}

uint32_t SpellInfo::getAreaAuraEffect() const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (isAreaAuraEffect(i))
            return Effect[i];
    }

    return 0;
}

bool SpellInfo::isTriggerSpellCastedByCaster(SpellInfo const* triggeringSpell) const
{
    const auto targetMask = getRequiredTargetMask(true);
    if (targetMask & SPELL_TARGET_REQUIRE_UNIT)
        return true;

    if (triggeringSpell != nullptr && triggeringSpell->isChanneled())
    {
        uint32_t mask = 0;
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (getEffect(i) == SPELL_EFFECT_NULL)
                continue;

            if (getEffectImplicitTargetA(i) == EFF_TARGET_SELF || getEffectImplicitTargetA(i) == EFF_TARGET_LOCATION_TO_SUMMON)
                continue;

            if (getEffectImplicitTargetB(i) == EFF_TARGET_SELF || getEffectImplicitTargetB(i) == EFF_TARGET_LOCATION_TO_SUMMON)
                continue;

            mask |= getRequiredTargetMaskForEffect(i);
        }

        if (mask & SPELL_TARGET_REQUIRE_UNIT)
            return true;
    }

    return false;
}

float_t SpellInfo::getMinRange([[maybe_unused]]bool friendly/* = false*/) const
{
    const auto* const rangeEntry = sSpellRangeStore.lookupEntry(getRangeIndex());
    if (rangeEntry == nullptr)
        return 0.0f;

#if VERSION_STRING > TBC
    if (friendly)
        return rangeEntry->minRangeFriendly;
#endif

    return rangeEntry->minRange;
}

float_t SpellInfo::getMaxRange([[maybe_unused]]bool friendly/* = false*/, Object* caster/* = nullptr*/, Spell* spell/* = nullptr*/) const
{
    const auto* const rangeEntry = sSpellRangeStore.lookupEntry(getRangeIndex());
    if (rangeEntry == nullptr)
        return 0.0f;

    float_t range = 0.0f;
#if VERSION_STRING > TBC
    if (friendly)
        range = rangeEntry->maxRangeFriendly;
    else
#endif
        range = rangeEntry->maxRange;

    if (caster != nullptr && caster->isCreatureOrPlayer())
        dynamic_cast<Unit*>(caster)->applySpellModifiers(SPELLMOD_RANGE, &range, this, spell);

    return range;
}

uint32_t SpellInfo::getTotem(uint8_t idx) const
{
    if (idx >= MAX_SPELL_TOTEMS)
    {
        sLogger.failure("Totem index id {} is invalid!", idx);
        return 0;
    }

    return Totem[idx];
}

int32_t SpellInfo::getReagent(uint8_t idx) const
{
    if (idx >= MAX_SPELL_REAGENTS)
    {
        sLogger.failure("Reagent index id {} is invalid!", idx);
        return 0;
    }

    return Reagent[idx];
}

uint32_t SpellInfo::getReagentCount(uint8_t idx) const
{
    if (idx >= MAX_SPELL_REAGENTS)
    {
        sLogger.failure("ReagentCount index id {} is invalid!", idx);
        return 0;
    }

    return ReagentCount[idx];
}

uint32_t SpellInfo::getEffect(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return Effect[idx];
}

int32_t SpellInfo::getEffectDieSides(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectDieSides[idx];
}

float SpellInfo::getEffectRealPointsPerLevel(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0.f;
    }

    return EffectRealPointsPerLevel[idx];
}

int32_t SpellInfo::getEffectBasePoints(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectBasePoints[idx];
}

uint32_t SpellInfo::getEffectMechanic(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectMechanic[idx];
}

uint32_t SpellInfo::getEffectImplicitTargetA(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectImplicitTargetA[idx];
}

uint32_t SpellInfo::getEffectImplicitTargetB(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectImplicitTargetB[idx];
}

uint32_t SpellInfo::getEffectRadiusIndex(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectRadiusIndex[idx];
}

uint32_t SpellInfo::getEffectApplyAuraName(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectApplyAuraName[idx];
}

uint32_t SpellInfo::getEffectAmplitude(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectAmplitude[idx];
}

float SpellInfo::getEffectMultipleValue(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectMultipleValue[idx];
}

uint32_t SpellInfo::getEffectChainTarget(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectChainTarget[idx];
}

uint32_t SpellInfo::getEffectItemType(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectItemType[idx];
}

int32_t SpellInfo::getEffectMiscValue(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectMiscValue[idx];
}

int32_t SpellInfo::getEffectMiscValueB(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectMiscValueB[idx];
}

uint32_t SpellInfo::getEffectTriggerSpell(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectTriggerSpell[idx];
}

float SpellInfo::getEffectPointsPerComboPoint(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectPointsPerComboPoint[idx];
}

uint32_t SpellInfo::getEffectSpellClassMask(uint8_t idx1, uint8_t idx2) const
{
    if (idx1 >= MAX_SPELL_EFFECTS || idx2 >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Totem index id {} or effect index {} is invalid!", idx1, idx2);
        return 0;
    }

    return EffectSpellClassMask[idx1][idx2];
}

uint32_t const* SpellInfo::getEffectSpellClassMask(uint8_t idx1) const
{
    if (idx1 >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx1);
        return 0;
    }

    return EffectSpellClassMask[idx1];
}

uint32_t SpellInfo::getSpellFamilyFlags(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return SpellFamilyFlags[idx];
}

float SpellInfo::getEffectDamageMultiplier(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectDamageMultiplier[idx];
}

#if VERSION_STRING > Classic
uint32_t SpellInfo::getTotemCategory(uint8_t idx) const
{
    if (idx >= MAX_SPELL_TOTEM_CATEGORIES)
    {
        sLogger.failure("TotemCategory index id {} is invalid!", idx);
        return 0;
    }

    return TotemCategory[idx];
}
#endif

float SpellInfo::getEffectBonusMultiplier(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectBonusMultiplier[idx];
}

uint32_t SpellInfo::getEffectCustomFlag(uint8_t idx) const
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return 0;
    }

    return EffectCustomFlag[idx];
}

void SpellInfo::setTotem(uint32_t totemId, uint8_t idx)                                         // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_TOTEMS)
    {
        sLogger.failure("Totem index id {} is invalid!", idx);
        return;
    }

    Totem[idx] = totemId;
}

void SpellInfo::setReagent(int32_t reagentId, uint8_t idx)                                      // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_REAGENTS)
    {
        sLogger.failure("Spellreagents index id {} is invalid!", idx);
        return;
    }

    Reagent[idx] = reagentId;
}

void SpellInfo::setReagentCount(uint32_t reagentId, uint8_t idx)                                // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_REAGENTS)
    {
        sLogger.failure("Reagentcount index id {} is invalid!", idx);
        return;
    }

    ReagentCount[idx] = reagentId;
}

void SpellInfo::setEffect(uint32_t effectId, uint8_t idx)                                       // used in HackFixes.cpp / ObjectMgr.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    Effect[idx] = effectId;
}

void SpellInfo::setEffectDieSides(int32_t effecSide, uint8_t idx)                               // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectDieSides[idx] = effecSide;
}

void SpellInfo::setEffectRealPointsPerLevel(float pointsPerLevel, uint8_t idx)                  // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectRealPointsPerLevel[idx] = pointsPerLevel;
}

void SpellInfo::setEffectBasePoints(int32_t pointsPerLevel, uint8_t idx)                        // used in HackFixes.cpp / ObjectMgr.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectBasePoints[idx] = pointsPerLevel;
}

void SpellInfo::setEffectMechanic(uint32_t mechanic, uint8_t idx)                               // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectMechanic[idx] = mechanic;
}

void SpellInfo::setEffectImplicitTargetA(uint32_t targetA, uint8_t idx)                         // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectImplicitTargetA[idx] = targetA;
}

void SpellInfo::setEffectImplicitTargetB(uint32_t targetB, uint8_t idx)                         // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectImplicitTargetB[idx] = targetB;
}

void SpellInfo::setEffectRadiusIndex(uint32_t radiusIndex, uint8_t idx)                         // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectRadiusIndex[idx] = radiusIndex;
}

void SpellInfo::setEffectApplyAuraName(uint32_t auraName, uint8_t idx)                          // used in HackFixes.cpp / ObjectMgr.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectApplyAuraName[idx] = auraName;
}

void SpellInfo::setEffectAmplitude(uint32_t amplitude, uint8_t idx)                             // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectAmplitude[idx] = amplitude;
}

void SpellInfo::setEffectMultipleValue(float multiply, uint8_t idx)                             // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectMultipleValue[idx] = multiply;
}

void SpellInfo::setEffectChainTarget(uint32_t chainTarget, uint8_t idx)                         // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectChainTarget[idx] = chainTarget;
}

void SpellInfo::setEffectItemType(uint32_t itemEntryId, uint8_t idx)
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectItemType[idx] = itemEntryId;
}

void SpellInfo::setEffectMiscValue(int32_t misc, uint8_t idx)                                   // used in HackFixes.cpp / ObjectMgr.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectMiscValue[idx] = misc;
}

void SpellInfo::setEffectMiscValueB(int32_t miscB, uint8_t idx)
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectMiscValueB[idx] = miscB;
}

void SpellInfo::setEffectTriggerSpell(uint32_t spell, uint8_t idx)                              // used in ObjectMgr.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectTriggerSpell[idx] = spell;
}

void SpellInfo::setEffectPointsPerComboPoint(float effectPoints, uint8_t idx)                   // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectPointsPerComboPoint[idx] = effectPoints;
}

void SpellInfo::setEffectSpellClassMask(uint32_t spellClass, uint8_t idx1, uint8_t idx2)        // used in HackFixes.cpp
{
    if (idx1 >= MAX_SPELL_EFFECTS || idx2 >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id1 {} or id2 {} is invalid!", idx1, idx2);
        return;
    }

    EffectSpellClassMask[idx1][idx2] = spellClass;
}

void SpellInfo::setSpellFamilyFlags(uint32_t value, uint8_t idx)                                // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    SpellFamilyFlags[idx] = value;
}

void SpellInfo::setEffectDamageMultiplier(float dmgMultiplier, uint8_t idx)                     // used in HackFixes.cpp
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectDamageMultiplier[idx] = dmgMultiplier;
}

#if VERSION_STRING > Classic
void SpellInfo::setTotemCategory(uint32_t category, uint8_t idx)
{
    if (idx >= MAX_SPELL_TOTEM_CATEGORIES)
    {
        sLogger.failure("TotemCategory index id {} is invalid!", idx);
        return;
    }

    TotemCategory[idx] = category;
}
#endif

void SpellInfo::setEffectBonusMultiplier(float value, uint8_t idx)
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectBonusMultiplier[idx] = value;
}

#if VERSION_STRING >= Cata
void SpellInfo::setEffectRadiusMaxIndex(uint32_t value, uint8_t idx)
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectRadiusMaxIndex[idx] = value;
}

void SpellInfo::setEffectSpellId(uint32_t value, uint8_t idx)
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectSpellId[idx] = value;
}

void SpellInfo::setEffectIndex(uint32_t value, uint8_t idx)
{
    if (idx >= MAX_SPELL_EFFECTS)
    {
        sLogger.failure("Effect index id {} is invalid!", idx);
        return;
    }

    EffectIndex[idx] = value;
}
#endif
