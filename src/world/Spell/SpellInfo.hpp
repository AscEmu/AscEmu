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
    uint32 Attributes;
    uint32 AttributesEx;
    uint32 AttributesExB;
    uint32 AttributesExC;
    uint32 AttributesExD;
    uint32 AttributesExE;
    uint32 AttributesExF;
    uint32 AttributesExG;
    uint32 AttributesExH;
    uint32 AttributesExI;
    uint32 AttributesExJ;
    uint32 CastingTimeIndex;
    uint32 DurationIndex;
    uint32 powerType;
    uint32 rangeIndex;
    float speed;
    uint32 SpellVisual;
    uint32 field114;
    uint32 spellIconID;
    uint32 activeIconID;
    std::string Name;
    std::string Rank;
    std::string Description;
    std::string BuffDescription;
    uint32 School;
    uint32 RuneCostID;
    //uint32 SpellMissileID;
    //uint32 spellDescriptionVariableID;
    uint32 SpellDifficultyID;

    //dbc links
    uint32 SpellScalingId;                              // SpellScaling.dbc
    uint32 SpellAuraOptionsId;                          // SpellAuraOptions.dbc
    uint32 SpellAuraRestrictionsId;                     // SpellAuraRestrictions.dbc
    uint32 SpellCastingRequirementsId;                  // SpellCastingRequirements.dbc
    uint32 SpellCategoriesId;                           // SpellCategories.dbc
    uint32 SpellClassOptionsId;                         // SpellClassOptions.dbc
    uint32 SpellCooldownsId;                            // SpellCooldowns.dbc
    uint32 SpellEquippedItemsId;                        // SpellEquippedItems.dbc
    uint32 SpellInterruptsId;                           // SpellInterrupts.dbc
    uint32 SpellLevelsId;                               // SpellLevels.dbc
    uint32 SpellPowerId;                                // SpellPower.dbc
    uint32 SpellReagentsId;                             // SpellReagents.dbc
    uint32 SpellShapeshiftId;                           // SpellShapeshift.dbc
    uint32 SpellTargetRestrictionsId;                   // SpellTargetRestrictions.dbc
    uint32 SpellTotemsId;                               // SpellTotems.dbc

    // data from SpellScaling.dbc
    // data from SpellAuraOptions.dbc
    uint32 maxstack;
    uint32 procChance;
    uint32 procCharges;
    uint32 procFlags;

    // data from SpellAuraRestrictions.dbc
    uint32 CasterAuraState;
    uint32 TargetAuraState;
    uint32 CasterAuraStateNot;
    uint32 TargetAuraStateNot;
    uint32 casterAuraSpell;
    uint32 targetAuraSpell;
    uint32 casterAuraSpellNot;
    uint32 targetAuraSpellNot;

    // data from SpellCastingRequirements.dbc
    uint32 FacingCasterFlags;
    int32 RequiresAreaId;
    uint32 RequiresSpellFocus;

    // data from SpellCategories.dbc
    uint32 Category;
    uint32 DispelType;
    uint32 Spell_Dmg_Type;
    uint32 MechanicsType;
    uint32 PreventionType;
    uint32 StartRecoveryCategory;

    // data from SpellClassOptions.dbc
    uint32 SpellGroupType[3];
    uint32 SpellFamilyName;

    // data from SpellCooldowns.dbc
    uint32 CategoryRecoveryTime;
    uint32 RecoveryTime;
    uint32 StartRecoveryTime;

    // data from SpellEquippedItems.dbc
    int32 EquippedItemClass;
    int32 EquippedItemInventoryTypeMask;
    int32 EquippedItemSubClass;

    // data from SpellInterrupts.dbc
    uint32 AuraInterruptFlags;
    uint32 ChannelInterruptFlags;
    uint32 InterruptFlags;

    // data from SpellLevels.dbc
    uint32 baseLevel;
    uint32 maxLevel;
    uint32 spellLevel;

    // data from SpellPower.dbc
    uint32 manaCost;
    uint32 manaCostPerlevel;
    uint32 ManaCostPercentage;
    uint32 manaPerSecond;
    uint32 manaPerSecondPerLevel;

    // data from SpellReagents.dbc
    uint32 Reagent[MAX_SPELL_REAGENTS];
    uint32 ReagentCount[MAX_SPELL_REAGENTS];

    // data from SpellShapeshift.dbc
    uint32 RequiredShapeShift;
    uint32 ShapeshiftExclude;

    // data from SpellTargetRestrictions.dbc
    uint32 MaxTargets;
    uint32 MaxTargetLevel;
    uint32 TargetCreatureType;
    uint32 Targets;

    // data from SpellTotems.dbc
    uint32 TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];
    uint32 Totem[MAX_SPELL_TOTEMS];
    
    // data from SpellEffect.dbc
    uint32 Effect[MAX_SPELL_EFFECTS];
    float EffectMultipleValue[MAX_SPELL_EFFECTS];
    uint32 EffectApplyAuraName[MAX_SPELL_EFFECTS];
    uint32 EffectAmplitude[MAX_SPELL_EFFECTS];
    int32 EffectBasePoints[MAX_SPELL_EFFECTS];
    float EffectBonusMultiplier[MAX_SPELL_EFFECTS];
    float dmg_multiplier[MAX_SPELL_EFFECTS];
    uint32 EffectChainTarget[MAX_SPELL_EFFECTS];
    int32 EffectDieSides[MAX_SPELL_EFFECTS];
    uint32 EffectItemType[MAX_SPELL_EFFECTS];
    uint32 EffectMechanic[MAX_SPELL_EFFECTS];
    int32 EffectMiscValue[MAX_SPELL_EFFECTS];
    int32 EffectMiscValueB[MAX_SPELL_EFFECTS];
    float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS];
    uint32 EffectRadiusIndex[MAX_SPELL_EFFECTS];
    uint32 EffectRadiusMaxIndex[MAX_SPELL_EFFECTS];
    float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];
    uint32 EffectSpellClassMask[MAX_SPELL_EFFECTS];
    uint32 EffectTriggerSpell[MAX_SPELL_EFFECTS];
    uint32 EffectImplicitTargetA[MAX_SPELL_EFFECTS];
    uint32 EffectImplicitTargetB[MAX_SPELL_EFFECTS];
    uint32 EffectSpellId[MAX_SPELL_EFFECTS];
    uint32 EffectIndex[MAX_SPELL_EFFECTS];

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

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
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
        for (uint8 i = 0; i < 3; ++i)
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
        for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
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
        for (uint8 i = 0; i < MAX_SPELL_TOTEMS; ++i)
        {
            TotemCategory[i] = 0;
            Totem[i] = 0;
        }

        // data from SpellEffect.dbc
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
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
        custom_DiminishStatus = 0;
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
        CustomFlags = 0;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            custom_ProcOnNameHash[i] = 0;
            EffectCustomFlag[i] = 0;
        }

        SpellFactoryFunc = nullptr;
        AuraFactoryFunc = nullptr;
    }
};

#endif  //_SPELL_INFO_HPP
