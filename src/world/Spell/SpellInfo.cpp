/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Skill.h"
#include "../../scripts/Battlegrounds/AlteracValley.h"
#include "Definitions/SpellEffectTarget.h"
#include "Spell/SpellEffects.h"
#include "Spell/SpellAuras.h"

SpellInfo::SpellInfo()
{
#if VERSION_STRING != Cata
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
    RequiredShapeShift = 0;
    ShapeshiftExclude = 0;
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

    CustomFlags = 0;

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        EffectCustomFlag[i] = 0;

    SpellFactoryFunc = NULL;
    AuraFactoryFunc = NULL;
    custom_proc_interval = 0;
    custom_BGR_one_buff_on_target = 0;
    custom_c_is_flags = 0;
    custom_RankNumber = 0;
    custom_NameHash = 0;
    custom_ThreatForSpell = 0;
    custom_ThreatForSpellCoef = 0;
    custom_spell_coef_flags = 0;
    custom_base_range_or_radius_sqr = 0;
    cone_width = 0;
    casttime_coef = 0;
    fixed_dddhcoef = 0;
    fixed_hotdotcoef = 0;
    Dspell_coef_override = 0;
    OTspell_coef_override = 0;
    ai_target_type = 0;
    custom_self_cast_only = false;
    custom_apply_on_shapeshift_change = false;
    custom_is_melee_spell = false;
    custom_is_ranged_spell = false;

    custom_SchoolMask = 0;
    SpellVisual = 0;
    field114 = 0;
    spellIconID = 0;
    activeIconID = 0;
    spellPriority = 0;
    Name = "";
    Rank = "";
    Description = "";
    BuffDescription = "";
    ManaCostPercentage = 0;
    StartRecoveryCategory = 0;
    StartRecoveryTime = 0;
    MaxTargetLevel = 0;
    SpellFamilyName = 0;
    MaxTargets = 0;
    Spell_Dmg_Type = 0;
    PreventionType = 0;
    StanceBarOrder = 0;
    MinFactionID = 0;
    MinReputation = 0;
    RequiredAuraVision = 0;
    RequiresAreaId = 0;
    School = 0;
    RuneCostID = 0;
    SpellDifficultyID = 0;
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
    modalNextSpell = 0;
    maxstack = 0;
    EquippedItemClass = 0;
    EquippedItemSubClass = 0;
    RequiredItemFlags = 0;
#else
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
    CastingTimeIndex = 0;
    DurationIndex = 0;
    powerType = 0;
    rangeIndex = 0;
    speed = 0;
    SpellVisual = 0;
    field114 = 0;
    spellIconID = 0;
    activeIconID = 0;
    Name = "";
    Rank = "";
    Description = "";
    BuffDescription = "";
    School = 0;
    RuneCostID = 0;
    SpellDifficultyID = 0;

    //dbc links
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

    // data from SpellScaling.dbc
    // data from SpellAuraOptions.dbc
    maxstack = 0;
    procChance = 0;
    procCharges = 0;
    procFlags = 0;

    // data from SpellAuraRestrictions.dbc
    CasterAuraState = 0;
    TargetAuraState = 0;
    CasterAuraStateNot = 0;
    TargetAuraStateNot = 0;
    casterAuraSpell = 0;
    targetAuraSpell = 0;
    casterAuraSpellNot = 0;
    targetAuraSpellNot = 0;

    // data from SpellCastingRequirements.dbc
    FacingCasterFlags = 0;
    RequiresAreaId = 0;
    RequiresSpellFocus = 0;

    // data from SpellCategories.dbc
    Category = 0;
    DispelType = 0;
    Spell_Dmg_Type = 0;
    MechanicsType = 0;
    PreventionType = 0;
    StartRecoveryCategory = 0;

    // data from SpellClassOptions.dbc
    SpellFamilyName = 0;
    for (uint8_t i = 0; i < 3; ++i)
        SpellGroupType[i] = 0;

    // data from SpellCooldowns.dbc
    CategoryRecoveryTime = 0;
    RecoveryTime = 0;
    StartRecoveryTime = 0;

    // data from SpellEquippedItems.dbc
    EquippedItemClass = 0;
    EquippedItemInventoryTypeMask = 0;
    EquippedItemSubClass = 0;

    // data from SpellInterrupts.dbc
    AuraInterruptFlags = 0;
    ChannelInterruptFlags = 0;
    InterruptFlags = 0;

    // data from SpellLevels.dbc
    baseLevel = 0;
    maxLevel = 0;
    spellLevel = 0;

    // data from SpellPower.dbc
    manaCost = 0;
    manaCostPerlevel = 0;
    ManaCostPercentage = 0;
    manaPerSecond = 0;
    manaPerSecondPerLevel = 0;

    // data from SpellReagents.dbc
    for (uint8_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
    {
        Reagent[i] = 0;
        ReagentCount[i] = 0;
    }

    // data from SpellShapeshift.dbc
    RequiredShapeShift = 0;
    ShapeshiftExclude = 0;

    // data from SpellTargetRestrictions.dbc
    MaxTargets = 0;
    MaxTargetLevel = 0;
    TargetCreatureType = 0;
    Targets = 0;

    // data from SpellTotems.dbc
    for (uint8_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
    {
        TotemCategory[i] = 0;
        Totem[i] = 0;
    }

    // data from SpellEffect.dbc
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        Effect[i] = 0;
        EffectMultipleValue[i] = 0;
        EffectApplyAuraName[i] = 0;
        EffectAmplitude[i] = 0;
        EffectBasePoints[i] = 0;
        EffectBonusMultiplier[i] = 0;
        dmg_multiplier[i] = 0;
        EffectChainTarget[i] = 0;
        EffectDieSides[i] = 0;
        EffectItemType[i] = 0;
        EffectMechanic[i] = 0;
        EffectMiscValue[i] = 0;
        EffectMiscValueB[i] = 0;
        EffectPointsPerComboPoint[i] = 0;
        EffectRadiusIndex[i] = 0;
        EffectRadiusMaxIndex[i] = 0;
        EffectRealPointsPerLevel[i] = 0;
        EffectSpellClassMask[i] = 0;
        EffectTriggerSpell[i] = 0;
        EffectImplicitTargetA[i] = 0;
        EffectImplicitTargetB[i] = 0;
        EffectSpellId[i] = 0;
        EffectIndex[i] = 0;
    }

    // custom values
    custom_proc_interval = 0;
    custom_BGR_one_buff_on_target = 0;
    custom_c_is_flags = 0;
    custom_RankNumber = 0;
    custom_NameHash = 0;
    custom_ThreatForSpell = 0;
    custom_ThreatForSpellCoef = 0;
    custom_spell_coef_flags = 0;
    custom_base_range_or_radius_sqr = 0;
    cone_width = 0;
    casttime_coef = 0;
    fixed_dddhcoef = 0;
    fixed_hotdotcoef = 0;
    Dspell_coef_override = 0;
    OTspell_coef_override = 0;
    ai_target_type = 0;

    custom_self_cast_only = false;
    custom_apply_on_shapeshift_change = false;
    custom_is_melee_spell = false;
    custom_is_ranged_spell = false;
    custom_SchoolMask = 0;
    CustomFlags = 0;

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        EffectCustomFlag[i] = 0;
    }

    SpellFactoryFunc = nullptr;
    AuraFactoryFunc = nullptr;
#endif
}

SpellInfo::~SpellInfo() {}


bool SpellInfo::HasEffect(uint32_t effect) const
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == effect)
        {
            return true;
        }
    }

    return false;
}

bool SpellInfo::HasEffectApplyAuraName(uint32_t aura_name)
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effect[i] == SPELL_EFFECT_APPLY_AURA && EffectApplyAuraName[i] == aura_name)
            return true;

    return false;
}

bool SpellInfo::HasCustomFlagForEffect(uint32_t effect, uint32_t flag)
{
    if (effect >= MAX_SPELL_EFFECTS)
        return false;

    if ((EffectCustomFlag[effect] & flag) != 0)
        return true;
    else
        return false;
}

bool SpellInfo::isDamagingSpell() const
{
    if (HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE)          ||
        HasEffect(SPELL_EFFECT_ENVIRONMENTAL_DAMAGE)   ||
        HasEffect(SPELL_EFFECT_HEALTH_LEECH)           ||
        HasEffect(SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL) ||
        HasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS)      ||
        HasEffect(SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)  ||
        HasEffect(SPELL_EFFECT_POWER_BURN)             ||
        HasEffect(SPELL_EFFECT_ATTACK))
        return true;

    if (appliesAreaAura(SPELL_AURA_PERIODIC_DAMAGE)         ||
        appliesAreaAura(SPELL_AURA_PROC_TRIGGER_DAMAGE)     ||
        appliesAreaAura(SPELL_AURA_PERIODIC_DAMAGE_PERCENT) ||
        appliesAreaAura(SPELL_AURA_POWER_BURN))
        return true;

    return false;
}

bool SpellInfo::isHealingSpell() const
{
    if (firstBeneficialEffect() != -1)
    {
        return true;
    }

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
        case 13952:
        case 15493:
        case 25263:
        case 25292: // Holy Light Rank 9
        case 27135: // Holy Light Rank 10
        case 27136: // Holy Light Rank 11
        case 29383:
        case 29427:
        case 29562:
        case 31713:
        case 32769:
        case 37979:
        case 43451:
        case 44479:
        case 46029:
        case 48781: // Holy Light Rank 12
        case 48782: // Holy Light Rank 13
        case 52444:
        case 56539: // Holy Light Rank 13
        case 58053: // Holy Light Rank 13
        case 66112: // Holy Light Rank 13
        case 68011: // Holy Light Rank 13
        case 68012: // Holy Light Rank 13
        case 68013: // Holy Light Rank 13
        case 19750: // Flash of Light Rank 1
        case 19939: // Flash of Light Rank 2
        case 19940: // Flash of Light Rank 3
        case 19941: // Flash of Light Rank 4
        case 19942: // Flash of Light Rank 5
        case 19943: // Flash of Light Rank 6
        case 25514:
        case 27137: // Flash of Light Rank 7
        case 33641:
        case 37249:
        case 37254:
        case 37257:
        case 48784: // Flash of Light Rank 8
        case 48785: // Flash of Light Rank 9
        case 57766:
        case 59997:
        case 66113: // Flash of Light Rank 9
        case 66922:
        case 68008: // Flash of Light Rank 9
        case 68009: // Flash of Light Rank 9
        case 68010: // Flash of Light Rank 9
        case 71930:
        {
            return true;
        }
        default:
            break;
    }

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

uint32_t SpellInfo::getSpellDuration(Unit* caster) const
{
    auto spell_duration = sSpellDurationStore.LookupEntry(DurationIndex);
    if (spell_duration == nullptr)
    {
        return 0;
    }

    if (caster == nullptr)
    {
        return spell_duration->Duration1;
    }

    auto ret = spell_duration->Duration1 + (spell_duration->Duration2 * caster->getLevel());
    if (ret > spell_duration->Duration3)
    {
        return spell_duration->Duration3;
    }
    return ret;
}

bool SpellInfo::hasTargetType(uint32_t type) const
{
    for (auto i = 0; i < 3; ++i)
    {
        if (EffectImplicitTargetA[i] == type ||
            EffectImplicitTargetB[i] == type)
        {
            return true;
        }
    }

    return false;
}

int SpellInfo::aiTargetType() const
{
    /*  this is not good as one spell effect can target self and other one an enemy,
    maybe we should make it for each spell effect or use as flags */
    if (
        hasTargetType(EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS) ||
        hasTargetType(EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS)       ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA)                              ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT)                      ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED)                    ||
        hasTargetType(EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME)
    )
    {
        return TTYPE_DESTINATION;
    }

    if (
        hasTargetType(EFF_TARGET_LOCATION_TO_SUMMON)      ||
        hasTargetType(EFF_TARGET_IN_FRONT_OF_CASTER)      ||
        hasTargetType(EFF_TARGET_ALL_FRIENDLY_IN_AREA)    ||
        hasTargetType(EFF_TARGET_PET_SUMMON_LOCATION)     ||
        hasTargetType(EFF_TARGET_LOCATION_INFRONT_CASTER) ||
        hasTargetType(EFF_TARGET_CONE_IN_FRONT)
    )
    {
        return TTYPE_SOURCE;
    }
    if (
        hasTargetType(EFF_TARGET_SINGLE_ENEMY)                      ||
        hasTargetType(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER)         ||
        hasTargetType(EFF_TARGET_DUEL)                              ||
        hasTargetType(EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET)         ||
        hasTargetType(EFF_TARGET_CHAIN)                             ||
        hasTargetType(EFF_TARGET_CURRENT_SELECTION)                 ||
        hasTargetType(EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER)   ||
        hasTargetType(EFF_TARGET_MULTIPLE_GUARDIAN_SUMMON_LOCATION) ||
        hasTargetType(EFF_TARGET_SELECTED_ENEMY_CHANNELED)
    )
    {
        return TTYPE_SINGLETARGET;
    }

    if (
        hasTargetType(EFF_TARGET_ALL_PARTY_AROUND_CASTER)     ||
        hasTargetType(EFF_TARGET_SINGLE_FRIEND)               ||
        hasTargetType(EFF_TARGET_PET_MASTER)                  ||
        hasTargetType(EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED) ||
        hasTargetType(EFF_TARGET_ALL_PARTY_IN_AREA)           ||
        hasTargetType(EFF_TARGET_SINGLE_PARTY)                ||
        hasTargetType(EFF_TARGET_ALL_PARTY)                   ||
        hasTargetType(EFF_TARGET_ALL_RAID)                    ||
        hasTargetType(EFF_TARGET_PARTY_MEMBER)                ||
        hasTargetType(EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS)
    )
    {
        return TTYPE_OWNER;
    }

    if (
        hasTargetType(EFF_TARGET_SELF) ||
        hasTargetType(4) ||
        hasTargetType(EFF_TARGET_PET) ||
        hasTargetType(EFF_TARGET_MINION)
    )
    {
        return TTYPE_CASTER;
    }

    return TTYPE_NULL;
}

bool SpellInfo::isTargetingStealthed() const
{
    if (hasTargetType(EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS) ||
        hasTargetType(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER) ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED) ||
        hasTargetType(EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT))
    {
        return true;
    }

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
        {
            return true;
        }
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

bool SpellInfo::IsPassive()
{
    return (Attributes & ATTRIBUTES_PASSIVE) != 0;
}

bool SpellInfo::IsProfession()
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            uint32_t skill = EffectMiscValue[i];

            //Profession skill
            if (skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID)
                return true;

            if (IsPrimaryProfessionSkill(skill))
                return true;
        }
    }
    return false;
}

bool SpellInfo::IsPrimaryProfession()
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            uint32_t skill = EffectMiscValue[i];

            if (IsPrimaryProfessionSkill(skill))
                return true;
        }
    }
    return false;
}

bool SpellInfo::IsPrimaryProfessionSkill(uint32_t skill_id)
{
    if (DBC::Structures::SkillLineEntry const* skill_line = sSkillLineStore.LookupEntry(skill_id))
        if (skill_line && skill_line->type == SKILL_TYPE_PROFESSION)
            return true;

    return false;
}

bool SpellInfo::isDeathPersistent() const
{
    return (AttributesExC & ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD) != 0;
}

bool SpellInfo::appliesAreaAura(uint32_t aura) const
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
                if (EffectApplyAuraName[i] == aura)
                {
                    return true;
                }
                break;
            default:
                break;
        }
    }

    return false;
}

uint32_t SpellInfo::GetAreaAuraEffectId()
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
