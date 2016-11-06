/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _SPELL_INFO_HPP
#define _SPELL_INFO_HPP

#include "SpellDefines.hpp"

class Player;

struct SERVER_DECL SpellInfo
{
    // Applied
    uint32 Id;
    uint32 Category;
    uint32 DispelType;
    uint32 MechanicsType;
    uint32 Attributes;
    uint32 AttributesEx;
    uint32 AttributesExB;
    uint32 AttributesExC;
    uint32 AttributesExD;
    uint32 AttributesExE;
    uint32 AttributesExF;
    uint32 AttributesExG;
    uint32 RequiredShapeShift;          // (12-13 Stances[2])
    uint32 ShapeshiftExclude;           // (14-15 StancesExcluded[2])
    uint32 Targets;
    uint32 TargetCreatureType;
    uint32 RequiresSpellFocus;
    uint32 FacingCasterFlags;
    uint32 CasterAuraState;
    uint32 TargetAuraState;
    uint32 CasterAuraStateNot;
    uint32 TargetAuraStateNot;
    uint32 casterAuraSpell;
    uint32 targetAuraSpell;
    uint32 casterAuraSpellNot;
    uint32 targetAuraSpellNot;
    uint32 CastingTimeIndex;
    uint32 RecoveryTime;
    uint32 CategoryRecoveryTime;
    uint32 InterruptFlags;
    uint32 AuraInterruptFlags;
    uint32 ChannelInterruptFlags;
    uint32 procFlags;
    uint32 procChance;
    uint32 procCharges;
    uint32 maxLevel;
    uint32 baseLevel;
    uint32 spellLevel;
    uint32 DurationIndex;
    int32 powerType;
    uint32 manaCost;
    uint32 manaCostPerlevel;
    uint32 manaPerSecond;
    uint32 manaPerSecondPerLevel;
    uint32 rangeIndex;
    float speed;
    uint32 modalNextSpell;
    uint32 maxstack;
    uint32 Totem[MAX_SPELL_TOTEMS];
    uint32 Reagent[MAX_SPELL_REAGENTS];
    uint32 ReagentCount[MAX_SPELL_REAGENTS];
    int32 EquippedItemClass;
    int32 EquippedItemSubClass;
    uint32 RequiredItemFlags;
    uint32 Effect[MAX_SPELL_EFFECTS];
    int32 EffectDieSides[MAX_SPELL_EFFECTS];
    float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];
    int32 EffectBasePoints[MAX_SPELL_EFFECTS];
    int32 EffectMechanic[MAX_SPELL_EFFECTS];
    uint32 EffectImplicitTargetA[MAX_SPELL_EFFECTS];
    uint32 EffectImplicitTargetB[MAX_SPELL_EFFECTS];
    uint32 EffectRadiusIndex[MAX_SPELL_EFFECTS];
    uint32 EffectApplyAuraName[MAX_SPELL_EFFECTS];
    uint32 EffectAmplitude[MAX_SPELL_EFFECTS];
    float EffectMultipleValue[MAX_SPELL_EFFECTS];
    uint32 EffectChainTarget[MAX_SPELL_EFFECTS];
    uint32 EffectItemType[MAX_SPELL_EFFECTS];
    int32 EffectMiscValue[MAX_SPELL_EFFECTS];
    int32 EffectMiscValueB[MAX_SPELL_EFFECTS];
    uint32 EffectTriggerSpell[MAX_SPELL_EFFECTS];
    float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS]; 
    uint32 EffectSpellClassMask[3][3];
    uint32 SpellVisual;
    uint32 field114;                                          // (131-132 SpellVisual[2])
    uint32 spellIconID;
    uint32 activeIconID;
    uint32 spellPriority;
    std::string Name;
    std::string Rank;
    std::string Description;
    std::string BuffDescription;
    uint32 ManaCostPercentage;
    uint32 StartRecoveryCategory;
    uint32 StartRecoveryTime;
    uint32 MaxTargetLevel;
    uint32 SpellFamilyName;
    uint32 SpellGroupType[MAX_SPELL_EFFECTS];
    uint32 MaxTargets;
    uint32 Spell_Dmg_Type;
    uint32 PreventionType;
    int32 StanceBarOrder;
    float dmg_multiplier[MAX_SPELL_EFFECTS];
    uint32 MinFactionID;
    uint32 MinReputation;
    uint32 RequiredAuraVision;
    uint32 TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];
    int32 RequiresAreaId;
    uint32 School;
    uint32 RuneCostID;
    //uint32 SpellMissileID;
    //uint32 SpellDescriptionVariable;
    uint32 SpellDifficultyID;

    //custom values
    uint32 custom_DiminishStatus;
    uint32 custom_proc_interval;
    uint32 custom_BGR_one_buff_on_target;
    uint32 custom_BGR_one_buff_from_caster_on_self;
    uint32 custom_c_is_flags;
    uint32 custom_RankNumber;
    uint32 custom_NameHash;
    uint32 custom_ThreatForSpell;
    float custom_ThreatForSpellCoef;
    uint32 custom_ProcOnNameHash[MAX_SPELL_EFFECTS];
    uint32 custom_spell_coef_flags;
    float custom_base_range_or_radius_sqr;
    float cone_width;
    float casttime_coef;
    float fixed_dddhcoef;
    float fixed_hotdotcoef;
    float Dspell_coef_override;
    float OTspell_coef_override;
    int ai_target_type;
    bool custom_self_cast_only;
    bool custom_apply_on_shapeshift_change;
    bool custom_always_apply;
    bool custom_is_melee_spell;
    bool custom_is_ranged_spell;
	bool CheckLocation(uint32 map_id, uint32 zone_id, uint32 area_id, Player* player = NULL);
    uint32 custom_SchoolMask;
    uint32 CustomFlags;
    uint32 EffectCustomFlag[MAX_SPELL_EFFECTS];

    void* (*SpellFactoryFunc);
    void* (*AuraFactoryFunc);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool HasEffect   - Tells if the Spell has a certain effect
    bool HasEffect(uint32 effect)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (Effect[i] == effect)
                return true;

        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool HasCustomFlagForEffect - Tells if the Spell has this flag for this effect
    bool HasCustomFlagForEffect(uint32 effect, uint32 flag)
    {
        if (effect >= MAX_SPELL_EFFECTS)
            return false;

        if ((EffectCustomFlag[effect] & flag) != 0)
            return true;
        else
            return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool AppliesAura  - Tells if the Spell applies this Aura
    bool AppliesAura(uint32 aura)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {

            if ((Effect[i] == 6 ||        /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                    Effect[i] == 27 ||    /// SPELL_EFFECT_PERSISTENT_AREA_AURA
                    Effect[i] == 35 ||    /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                    Effect[i] == 65 ||    /// SPELL_EFFECT_APPLY_RAID_AREA_AURA
                    Effect[i] == 119 ||   /// SPELL_EFFECT_APPLY_PET_AREA_AURA
                    Effect[i] == 128 ||   /// SPELL_EFFECT_APPLY_FRIEND_AREA_AURA
                    Effect[i] == 129 ||   /// SPELL_EFFECT_APPLY_ENEMY_AREA_AURA
                    Effect[i] == 143) &&  /// SPELL_EFFECT_APPLY_OWNER_AREA_AURA
                    EffectApplyAuraName[i] == aura)
                return true;
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note uint32 GetAAEffectId()  - Returns the Effect Id of the Area Aura effect if the spell has one.
    uint32 GetAAEffectId()
    {

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
        {

            if (Effect[i] == 35 ||        /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                    Effect[i] == 65 ||    /// SPELL_EFFECT_APPLY_RAID_AREA_AURA
                    Effect[i] == 119 ||   /// SPELL_EFFECT_APPLY_PET_AREA_AURA
                    Effect[i] == 128 ||   /// SPELL_EFFECT_APPLY_FRIEND_AREA_AURA
                    Effect[i] == 129 ||   /// SPELL_EFFECT_APPLY_ENEMY_AREA_AURA
                    Effect[i] == 143)     /// SPELL_EFFECT_APPLY_OWNER_AREA_AURA
                return Effect[i];
        }

        return 0;
    }

    SpellInfo()
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
};

#endif  //_SPELL_INFO_HPP
