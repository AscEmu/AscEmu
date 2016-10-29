/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _SPELL_INFO_HPP
#define _SPELL_INFO_HPP

#include "SpellDefines.hpp"

class Player;

class SERVER_DECL SpellInfo
{
    public:

        SpellInfo();
        ~SpellInfo();

        // helper functions

        //////////////////////////////////////////////////////////////////////////////////////////
        // Applied values from DBC
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

        //////////////////////////////////////////////////////////////////////////////////////////
        // custom values
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

    
};

#endif  //_SPELL_INFO_HPP
