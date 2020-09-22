/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Definitions/School.h"
#include "Definitions/SpellEffects.h"
#include "Definitions/SpellEffectTarget.h"
#include "SpellAuras.h"

#include "Management/Skill.h"
#include "Units/Creatures/AIInterface.h"

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

    // If any of the effect indexes contain same mask, the spells affect each other
    // TODO: this always returns false on classic and TBC since EffectSpellClassMask field does not exist there
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        for (uint8_t u = 0; u < MAX_SPELL_EFFECTS; ++u)
        {
            if (EffectSpellClassMask[u][i] && (EffectSpellClassMask[u][i] & spellInfo->SpellFamilyFlags[i]))
                return true;
        }
    }
    return false;
}

bool SpellInfo::isAuraEffectAffectingSpell(AuraEffect auraEffect, SpellInfo const* spellInfo) const
{
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

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (EffectSpellClassMask[i][effIndex] && EffectSpellClassMask[i][effIndex] & spellInfo->SpellFamilyFlags[effIndex])
            return true;
    }

    return false;
}

bool SpellInfo::hasValidPowerType() const
{
    return getPowerType() < TOTAL_PLAYER_POWER_TYPES;
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
