/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

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

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
        EffectCustomFlag[i] = 0;

    SpellFactoryFunc = NULL;
    AuraFactoryFunc = NULL;
    custom_proc_interval = 0;
    custom_BGR_one_buff_on_target = 0;
    custom_BGR_one_buff_from_caster_on_self = 0;
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
    custom_always_apply = false;
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
    custom_DiminishStatus = 0;
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
}

SpellInfo::~SpellInfo() {}


bool SpellInfo::HasEffect(uint32 effect)
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (Effect[i] == effect)
            return true;

    return false;
}

bool SpellInfo::HasCustomFlagForEffect(uint32 effect, uint32 flag)
{
    if (effect >= MAX_SPELL_EFFECTS)
        return false;

    if ((EffectCustomFlag[effect] & flag) != 0)
        return true;
    else
        return false;
}

bool SpellInfo::IsPassive()
{
    return (Attributes & ATTRIBUTES_PASSIVE) != 0;
}

bool SpellInfo::IsProfession()
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            uint32 skill = EffectMiscValue[i];

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
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (Effect[i] == SPELL_EFFECT_SKILL)
        {
            uint32 skill = EffectMiscValue[i];

            if (IsPrimaryProfessionSkill(skill))
                return true;
        }
    }
    return false;
}

bool SpellInfo::IsPrimaryProfessionSkill(uint32 skill_id)
{
    if (DBC::Structures::SkillLineEntry const* skill_line = sSkillLineStore.LookupEntry(skill_id))
        if (skill_line && skill_line->type == SKILL_TYPE_PROFESSION)
            return true;

    return false;
}

bool SpellInfo::IsDeathPersistent()
{
    return (AttributesExC & ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD) != 0;
}

bool SpellInfo::AppliesAreaAura(uint32 aura)
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if ((Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA ||
             Effect[i] == SPELL_EFFECT_APPLY_GROUP_AREA_AURA ||
             Effect[i] == SPELL_EFFECT_APPLY_RAID_AREA_AURA ||
             Effect[i] == SPELL_EFFECT_APPLY_PET_AREA_AURA ||
             Effect[i] == SPELL_EFFECT_APPLY_FRIEND_AREA_AURA ||
             Effect[i] == SPELL_EFFECT_APPLY_ENEMY_AREA_AURA ||
             Effect[i] == SPELL_EFFECT_APPLY_OWNER_AREA_AURA) &&
            EffectApplyAuraName[i] == aura)
            return true;
    }

    return false;
}

uint32 SpellInfo::GetAreaAuraEffectId()
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
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
