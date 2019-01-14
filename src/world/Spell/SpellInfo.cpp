/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Skill.h"
#include "../../scripts/Battlegrounds/AlteracValley.h"
#include "Definitions/SpellEffectTarget.h"
#include "Spell/Definitions/SpellEffects.h"
#include "Spell/SpellAuras.h"

SpellInfo::SpellInfo()
{
    Id = 0;
    Category = 0;
    DispelType = 0;
    MechanicsType = 0;
    Attributes = 0;
    AttributesEx = 0;
    AttributesExB = 0;
    AttributesExC = 0;
    AttributesExD = 0;
    AttributesExE = 0;
    AttributesExF = 0;
    AttributesExG = 0;
    AttributesExH = 0;
    AttributesExI = 0;
    AttributesExJ = 0;
    Shapeshifts = 0;
    ShapeshiftsExcluded = 0;
    Targets = 0;
    TargetCreatureType = 0;
    RequiresSpellFocus = 0;
    FacingCasterFlags = 0;
    CasterAuraState = 0;
    TargetAuraState = 0;
    CasterAuraStateNot = 0;
    TargetAuraStateNot = 0;
    casterAuraSpell = 0;
    targetAuraSpell = 0;
    casterAuraSpellNot = 0;
    targetAuraSpellNot = 0;
    CastingTimeIndex = 0;
    RecoveryTime = 0;
    CategoryRecoveryTime = 0;
    InterruptFlags = 0;
    AuraInterruptFlags = 0;
    ChannelInterruptFlags = 0;
    procFlags = 0;
    procChance = 0;
    procCharges = 0;
    maxLevel = 0;
    baseLevel = 0;
    spellLevel = 0;
    DurationIndex = 0;
    powerType = 0;
    manaCost = 0;
    manaCostPerlevel = 0;
    manaPerSecond = 0;
    manaPerSecondPerLevel = 0;
    rangeIndex = 0;
    speed = 0;
    MaxStackAmount = 0;
    for (auto i = 0; i < MAX_SPELL_TOTEMS; ++i)
        Totem[i] = 0;
    for (auto i = 0; i < MAX_SPELL_REAGENTS; ++i)
    {
        Reagent[i] = 0;
        ReagentCount[i] = 0;
    }
    EquippedItemClass = 0;
    EquippedItemSubClass = 0;
    EquippedItemInventoryTypeMask = 0;
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        Effect[i] = 0;
        EffectDieSides[i] = 0;
        EffectRealPointsPerLevel[i] = 0;
        EffectBasePoints[i] = 0;
        EffectMechanic[i] = 0;
        EffectImplicitTargetA[i] = 0;
        EffectImplicitTargetB[i] = 0;
        EffectRadiusIndex[i] = 0;
        EffectApplyAuraName[i] = 0;
        EffectAmplitude[i] = 0;
        EffectMultipleValue[i] = 0;
        EffectChainTarget[i] = 0;
        EffectItemType[i] = 0;
        EffectMiscValue[i] = 0;
        EffectMiscValueB[i] = 0;
        EffectTriggerSpell[i] = 0;
        EffectPointsPerComboPoint[i] = 0;
        for (auto u = 0; u < MAX_SPELL_EFFECTS; ++u)
            EffectSpellClassMask[i][u] = 0;
#if VERSION_STRING >= Cata
        EffectRadiusMaxIndex[i] = 0;
        EffectSpellId[i] = 0;
        EffectIndex[i] = 0;
#endif
        SpellFamilyFlags[i] = 0;
        EffectDamageMultiplier[i] = 0;
        EffectBonusMultiplier[i] = 0;
    }
    SpellVisual = 0;
    spellIconID = 0;
    activeIconID = 0;
    spellPriority = 0;
    Name = "";
    Rank = "";
    ManaCostPercentage = 0;
    StartRecoveryCategory = 0;
    StartRecoveryTime = 0;
    MaxTargetLevel = 0;
    SpellFamilyName = 0;
    MaxTargets = 0;
    DmgClass = 0;
    PreventionType = 0;
    for (auto i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
        TotemCategory[i] = 0;
    AreaGroupId = 0;
    School = 0;
    RuneCostID = 0;
    SpellDifficultyId = 0;
    
#if VERSION_STRING >= Cata
    // DBC links
    SpellScalingId = 0;
    SpellAuraOptionsId = 0;
    SpellAuraRestrictionsId = 0;
    SpellCastingRequirementsId = 0;
    SpellCategoriesId = 0;
    SpellClassOptionsId = 0;
    SpellCooldownsId = 0;
    SpellEquippedItemsId = 0;
    SpellInterruptsId = 0;
    SpellLevelsId = 0;
    SpellPowerId = 0;
    SpellReagentsId = 0;
    SpellShapeshiftId = 0;
    SpellTargetRestrictionsId = 0;
    SpellTotemsId = 0;
#endif
    // Coefficient values
    spell_coeff_direct = -1;
    spell_coeff_overtime = -1;
    spell_coeff_direct_override = -1;
    spell_coeff_overtime_override = -1;

    // Custom values
    custom_proc_interval = 0;
    custom_BGR_one_buff_on_target = 0;
    custom_c_is_flags = 0;
    custom_RankNumber = 0;
    custom_NameHash = 0;
    custom_ThreatForSpell = 0;
    custom_ThreatForSpellCoef = 0;
    custom_base_range_or_radius_sqr = 0;
    cone_width = 0;
    ai_target_type = 0;

    custom_self_cast_only = false;
    custom_apply_on_shapeshift_change = false;
    custom_is_melee_spell = false;
    custom_is_ranged_spell = false;
    custom_SchoolMask = 0;

    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
        EffectCustomFlag[i] = 0;

    // Script linkers
    spellScriptLink = nullptr;
    auraScriptLink = nullptr;
}

SpellInfo::~SpellInfo() {}


bool SpellInfo::hasEffect(uint32_t effect) const
{
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == effect)
            return true;
    }

    return false;
}

bool SpellInfo::hasEffectApplyAuraName(uint32_t auraType) const
{
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if ((Effect[i] == SPELL_EFFECT_APPLY_AURA || Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA) && EffectApplyAuraName[i] == auraType)
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
        hasEffectApplyAuraName(SPELL_AURA_POWER_BURN))
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
    for (auto i = 0; i < 3; ++i)
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
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        for (auto u = 0; u < MAX_SPELL_EFFECTS; ++u)
        {
            // todo: test on Cata
            if (EffectSpellClassMask[u][i] && (EffectSpellClassMask[u][i] & spellInfo->SpellFamilyFlags[i]))
                return true;
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
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
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
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            auto skill = EffectMiscValue[i];

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
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            auto skill = EffectMiscValue[i];
            if (isPrimaryProfessionSkill(skill))
                return true;
        }
    }
    return false;
}

bool SpellInfo::isPrimaryProfessionSkill(uint32_t skill_id) const
{
    if (auto skill_line = sSkillLineStore.LookupEntry(skill_id))
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

bool SpellInfo::appliesAreaAura(uint32_t auraType) const
{
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
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
    for (auto i = 0; i < MAX_SPELL_EFFECTS; ++i)
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
