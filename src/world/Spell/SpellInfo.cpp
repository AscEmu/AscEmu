/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Definitions/School.h"
#include "Definitions/SpellEffects.h"
#include "Definitions/SpellEffectTarget.h"
#include "Definitions/SpellIsFlags.h"
#include "SpellAuras.h"
#include "SpellTarget.h"

#include "Management/Skill.h"
#include "Units/Creatures/AIInterface.h"
#include "Units/Players/Player.h"

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
            Effect[i] != SPELL_EFFECT_APPLY_FRIEND_AREA_AURA && Effect[i] != SPELL_EFFECT_APPLY_GROUP_AREA_AURA && Effect[i] != SPELL_EFFECT_APPLY_OWNER_AREA_AURA &&
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
    ARCEMU_ASSERT(effIndex < MAX_SPELL_EFFECTS);

    if (getEffect(effIndex) == SPELL_EFFECT_SCHOOL_DAMAGE ||
        getEffect(effIndex) == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE ||
        getEffect(effIndex) == SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL ||
        getEffect(effIndex) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE ||
        getEffect(effIndex) == SPELL_EFFECT_WEAPON_DAMAGE ||
        getEffect(effIndex) == SPELL_EFFECT_POWER_BURN)
        return true;
    return false;
}

bool SpellInfo::isHealingEffect(uint8_t effIndex) const
{
    ARCEMU_ASSERT(effIndex < MAX_SPELL_EFFECTS);

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
            sLogger.failure("SpellInfo::getBasePowerCost : Unknown power type %u for spell id %u", getPowerType(), getId());
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
                case POWER_TYPE_HAPPINESS:
                    powerCost += static_cast<int32_t>(caster->getMaxPower(getPowerType()) * getManaCostPercentage() / 100);
                    break;
#if VERSION_STRING >= WotLK
                case POWER_TYPE_RUNES:
                case POWER_TYPE_RUNIC_POWER:
                    // In 3.3.5a only obsolete spells use these and have a non-null getManaCostPercentage
                    break;
#endif
                default:
                    sLogger.failure("SpellInfo::getBasePowerCost() : Unknown power type %u for spell id %u", getPowerType(), getId());
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

    // Check each effect
    // If any effect contain one of the following aura effects, the aura is negative
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (getEffectApplyAuraName(i) == SPELL_AURA_NONE)
            continue;

        switch (getEffectApplyAuraName(i))
        {
            //\ todo: add more checks later
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_LEECH:
            case SPELL_AURA_PERIODIC_MANA_LEECH:
            case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            case SPELL_AURA_PERIODIC_POWER_BURN:
                // No need to do other checks, definitely negative
                return true;
            default:
                break;
        }
    }

    return false;
}

uint32_t SpellInfo::getSpellDefaultDuration(Unit const* caster) const
{
    auto spell_duration = sSpellDurationStore.LookupEntry(DurationIndex);
    if (spell_duration == nullptr)
        return 0;

    if (caster == nullptr)
        return spell_duration->Duration1;

    auto ret = spell_duration->Duration1 + (spell_duration->Duration2 * caster->getLevel());
    if (ret > spell_duration->Duration3)
        return spell_duration->Duration3;

    return ret;
}

bool SpellInfo::hasTargetType(uint32_t type) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
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
            targetMask = SPELL_TARGET_REQUIRE_ITEM | SPELL_TARGET_REQUIRE_GAMEOBJECT;
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
            targetMask = SPELL_TARGET_AREA_SELF | SPELL_TARGET_NO_OBJECT;
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
            targetMask = SPELL_TARGET_AREA_SELF | SPELL_TARGET_NO_OBJECT; //dont fill target map for this (fucks up some spell visuals)
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
        case 87:
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

uint32_t SpellInfo::getRequiredTargetMaskForEffect(uint8_t effectIndex) const
{
    auto targetMask = getRequiredTargetMaskForEffectTarget(getEffectImplicitTargetA(effectIndex), effectIndex);

    // Do not get target mask from B if it's not set
    if (getEffectImplicitTargetB(effectIndex) != EFF_TARGET_NONE)
        targetMask |= getRequiredTargetMaskForEffectTarget(getEffectImplicitTargetB(effectIndex), effectIndex);

    return targetMask;
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

bool SpellInfo::isProfession() const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            const auto skill = EffectMiscValue[i];

            //Profession skill
            if (skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID)
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
            const auto skill = EffectMiscValue[i];
            if (isPrimaryProfessionSkill(skill))
                return true;
        }
    }
    return false;
}

bool SpellInfo::isPrimaryProfessionSkill(uint32_t skill_id) const
{
    if (const auto skill_line = sSkillLineStore.LookupEntry(skill_id))
    {
        if (skill_line && skill_line->type == SKILL_TYPE_PROFESSION)
            return true;
    }

    return false;
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

int32_t SpellInfo::calculateEffectValue(uint8_t effIndex, Unit* unitCaster/* = nullptr*/, Item* itemCaster/* = nullptr*/, uint32_t forcedBasePoints/* = 0*/) const
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return 0;

    const float_t basePointsPerLevel = getEffectRealPointsPerLevel(effIndex);
    const auto randomPoints = getEffectDieSides(effIndex);
    int32_t basePoints = 0;

    // Random suffix value calculation
    if (itemCaster != nullptr && static_cast<int32_t>(itemCaster->getRandomPropertiesId()) < 0)
    {
        const auto randomSuffix = sItemRandomSuffixStore.LookupEntry(std::abs(static_cast<int32_t>(itemCaster->getRandomPropertiesId())));
        if (randomSuffix != nullptr)
        {
            auto faulty = false;
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (randomSuffix->enchantments[i] == 0)
                    continue;

                const auto spellItemEnchant = sSpellItemEnchantmentStore.LookupEntry(randomSuffix->enchantments[i]);
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

    if (forcedBasePoints > 0)
    {
        basePoints = forcedBasePoints;
    }
    else
    {
#if VERSION_STRING >= Cata
        basePoints = getEffectBasePoints(effIndex);
#else
        basePoints = getEffectBasePoints(effIndex) + 1;
#endif
    }

    // Check if value increases with level
    if (unitCaster != nullptr)
    {
        auto diff = -static_cast<int32_t>(getBaseLevel());
        if (getMaxLevel() != 0 && unitCaster->getLevel() > getMaxLevel())
            diff += getMaxLevel();
        else
            diff += unitCaster->getLevel();

        basePoints += float2int32(diff * basePointsPerLevel);
    }

    if (randomPoints > 1)
        basePoints += Util::getRandomUInt(randomPoints);

    // Check if value increases with combo points
    const auto comboDamage = getEffectPointsPerComboPoint(effIndex);
    if (comboDamage > 0.0f && unitCaster != nullptr && unitCaster->isPlayer())
    {
        const auto plrCaster = static_cast<Player*>(unitCaster);
        basePoints += static_cast<int32_t>(std::round(comboDamage * plrCaster->m_comboPoints));
        // TODO: rewrite combo points, here's an old comment from legacy method:
        //this is ugly so i will explain the case maybe someone ha a better idea :
        // while casting a spell talent will trigger upon the spell prepare faze
        // the effect of the talent is to add 1 combo point but when triggering spell finishes it will clear the extra combo point
        plrCaster->m_spellcomboPoints = 0;
    }

    return basePoints;
}

bool SpellInfo::doesEffectApplyAura(uint8_t effIndex) const
{
    if (effIndex >= MAX_SPELL_EFFECTS)
        return false;

    switch (Effect[effIndex])
    {
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_PERSISTENT_AREA_AURA:
        case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
        case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
        case SPELL_EFFECT_APPLY_PET_AREA_AURA:
        case SPELL_EFFECT_APPLY_FRIEND_AREA_AURA:
        case SPELL_EFFECT_APPLY_ENEMY_AREA_AURA:
        case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
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
        switch (Effect[i])
        {
            case SPELL_EFFECT_PERSISTENT_AREA_AURA:
            case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
            case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
            case SPELL_EFFECT_APPLY_PET_AREA_AURA:
            case SPELL_EFFECT_APPLY_FRIEND_AREA_AURA:
            case SPELL_EFFECT_APPLY_ENEMY_AREA_AURA:
            case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
                if (EffectApplyAuraName[i] == auraType)
                    return true;
                break;
            default:
                break;
        }
    }

    return false;
}

uint32_t SpellInfo::getAreaAuraEffect() const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_APPLY_GROUP_AREA_AURA ||
            Effect[i] == SPELL_EFFECT_APPLY_RAID_AREA_AURA ||
            Effect[i] == SPELL_EFFECT_APPLY_PET_AREA_AURA ||
            Effect[i] == SPELL_EFFECT_APPLY_FRIEND_AREA_AURA ||
            Effect[i] == SPELL_EFFECT_APPLY_ENEMY_AREA_AURA ||
            Effect[i] == SPELL_EFFECT_APPLY_OWNER_AREA_AURA)
            return Effect[i];
    }

    return 0;
}
