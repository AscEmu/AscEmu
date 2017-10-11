/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "SpellDefines.hpp"
#include "WorldConf.h"
#include "CommonTypes.hpp"
#include "../Server/WUtil.h"
#include <string>

class Player;
class Unit;

class SERVER_DECL SpellInfo
{
public:
    SpellInfo();
    ~SpellInfo();

    // helper functions
    bool HasEffect(uint32 effect) const;
    bool HasEffectApplyAuraName(uint32_t aura_name);
    bool HasCustomFlagForEffect(uint32 effect, uint32 flag);

    bool isDamagingSpell() const;
    bool isHealingSpell() const;
    int firstBeneficialEffect() const;

    uint32_t getSpellDuration(Unit* caster) const;

    bool hasTargetType(uint32_t type) const;
    int aiTargetType() const;
    bool isTargetingStealthed() const;
    bool isRequireCooldownSpell() const;

    bool IsPassive();
    bool IsProfession();
    bool IsPrimaryProfession();
    bool IsPrimaryProfessionSkill(uint32 skill_id);

    bool isDeathPersistent() const;

    bool appliesAreaAura(uint32 aura) const;
    uint32 GetAreaAuraEffectId();

    uint32 getId() const { return Id; }
    void setId(uint32_t value) { Id = value; }

    uint32 getCategory() const { return Category; }

    uint32 getDispelType() const { return DispelType; }
    void setDispelType(uint32_t value) { DispelType = value; }              // used in HackFixes.cpp

    uint32 getMechanicsType() const { return MechanicsType; }
    void setMechanicsType(uint32_t value) { MechanicsType = value; }        // used in HackFixes.cpp

    uint32 getAttributes() const { return Attributes; }
    void setAttributes(uint32_t value) { Attributes = value; }              // used in HackFixes.cpp / SpellCustomizations.cpp
    void addAttributes(uint32_t value) { Attributes |= value; }             // used in HackFixes.cpp
    void removeAttributes(uint32_t value) { Attributes &= ~value; }         // used in HackFixes.cpp

    uint32 getAttributesEx() const { return AttributesEx; }
    void setAttributesEx(uint32_t value) { AttributesEx = value; }          // used in HackFixes.cpp / SpellCustomizations.cpp
    void addAttributesEx(uint32_t value) { AttributesEx |= value; }         // used in HackFixes.cpp

    uint32 getAttributesExB() const { return AttributesExB; }
    void setAttributesExB(uint32_t value) { AttributesExB = value; }        // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getAttributesExC() const { return AttributesExC; }
    void setAttributesExC(uint32_t value) { AttributesExC = value; }        // used in SpellCustomizations.cpp
    void addAttributesExC(uint32_t value) { AttributesExC |= value; }       // used in HackFixes.cpp

    uint32 getAttributesExD() const { return AttributesExD; }
    void setAttributesExD(uint32_t value) { AttributesExD = value; }        // used in SpellCustomizations.cpp

    uint32 getAttributesExE() const { return AttributesExE; }
    void setAttributesExE(uint32_t value) { AttributesExE = value; }        // used in SpellCustomizations.cpp

    uint32 getAttributesExF() const { return AttributesExF; }
    void setAttributesExF(uint32_t value) { AttributesExF = value; }        // used in SpellCustomizations.cpp

    uint32 getAttributesExG() const { return AttributesExG; }
    void setAttributesExG(uint32_t value) { AttributesExG = value; }        // used in SpellCustomizations.cpp

    uint32 getRequiredShapeShift() const { return RequiredShapeShift; }
    void setRequiredShapeShift(uint32_t value) { RequiredShapeShift = value; } // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getShapeshiftExclude() const { return ShapeshiftExclude; }
    void setShapeshiftExclude(uint32_t value) { ShapeshiftExclude = value; } // used in SpellCustomizations.cpp

    uint32 getTargets() const { return Targets; }                           // not used!
    void setTargets(uint32_t value) { Targets = value; }                    // used in SpellCustomizations.cpp

    uint32 getTargetCreatureType() const { return TargetCreatureType; }
    void setTargetCreatureType(uint32_t value) { TargetCreatureType = value; } // used in SpellCustomizations.cpp

    uint32 getRequiresSpellFocus() const { return RequiresSpellFocus; }
    void setRequiresSpellFocus(uint32_t value) { RequiresSpellFocus = value; } // used in SpellCustomizations.cpp

    uint32 getFacingCasterFlags() const { return FacingCasterFlags; }
    void setFacingCasterFlags(uint32_t value) { FacingCasterFlags = value; } // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getCasterAuraState() const { return CasterAuraState; }
    void setCasterAuraState(uint32_t value) { CasterAuraState = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getTargetAuraState() const { return TargetAuraState; }
    void setTargetAuraState(uint32_t value) { TargetAuraState = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getCasterAuraStateNot() const { return CasterAuraStateNot; }
    void setCasterAuraStateNot(uint32_t value) { CasterAuraStateNot = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getTargetAuraStateNot() const { return TargetAuraStateNot; }
    void setTargetAuraStateNot(uint32_t value) { TargetAuraStateNot = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getCasterAuraSpell() const { return casterAuraSpell; }
    void setCasterAuraSpell(uint32_t value) { casterAuraSpell = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getTargetAuraSpell() const { return targetAuraSpell; }
    void setTargetAuraSpell(uint32_t value) { targetAuraSpell = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getCasterAuraSpellNot() const { return casterAuraSpellNot; }
    void setCasterAuraSpellNot(uint32_t value) { casterAuraSpellNot = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getTargetAuraSpellNot() const { return targetAuraSpellNot; }
    void setTargetAuraSpellNot(uint32_t value) { targetAuraSpellNot = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getCastingTimeIndex() const { return CastingTimeIndex; }
    void setCastingTimeIndex(uint32_t value) { CastingTimeIndex = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32 getRecoveryTime() const { return RecoveryTime; }
    uint32 getCategoryRecoveryTime() const { return CategoryRecoveryTime; }
    uint32 getInterruptFlags() const { return InterruptFlags; }
    uint32 getAuraInterruptFlags() const { return AuraInterruptFlags; }
    uint32 getChannelInterruptFlags() const { return ChannelInterruptFlags; }
    uint32 getProcFlags() const { return procFlags; }
    uint32 getProcChance() const { return procChance; }
    uint32 getProcCharges() const { return procCharges; }
    uint32 getMaxLevel() const { return maxLevel; }
    uint32 getBaseLevel() const { return baseLevel; }
    uint32 getSpellLevel() const { return spellLevel; }
    uint32 getDurationIndex() const { return DurationIndex; }
    int32 getPowerType() const { return powerType; }
    uint32 getManaCost() const { return manaCost; }
    uint32 getManaCostPerlevel() const { return manaCostPerlevel; }
    uint32 getManaPerSecond() const { return manaPerSecond; }
    uint32 getManaPerSecondPerLevel() const { return manaPerSecondPerLevel; }
    uint32 getRangeIndex() const { return rangeIndex; }
    float getSpeed() const { return speed; }
    uint32 getMaxstack() const { return maxstack; }

    uint32 getTotem(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEMS && idx > 0);
        return Totem[idx];
    }

    uint32 getReagent(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS && idx > 0);
        return Reagent[idx];
    }

    uint32 getReagentCount(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS && idx > 0);
        return ReagentCount[idx];
    }

    int32 getEquippedItemClass() const { return EquippedItemClass; }
    int32 getEquippedItemSubClass() const { return EquippedItemSubClass; }

    uint32 getEffect(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return Effect[idx];
    }

    int32 getEffectDieSides(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectDieSides[idx];
    }

    float getEffectRealPointsPerLevel(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectRealPointsPerLevel[idx];
    }

    int32 getEffectBasePoints(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectBasePoints[idx];
    }

    int32 getEffectMechanic(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectMechanic[idx];
    }

    uint32 getEffectImplicitTargetA(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectImplicitTargetA[idx];
    }

    uint32 getEffectImplicitTargetB(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectImplicitTargetB[idx];
    }

    uint32 getEffectRadiusIndex(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectRadiusIndex[idx];
    }

    uint32 getEffectApplyAuraName(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectApplyAuraName[idx];
    }

    uint32 getEffectAmplitude(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectAmplitude[idx];
    }

    float getEffectMultipleValue(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectMultipleValue[idx];
    }

    uint32 getEffectChainTarget(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectChainTarget[idx];
    }

    uint32 getEffectItemType(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectItemType[idx];
    }

    int32 getEffectMiscValue(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectMiscValue[idx];
    }

    int32 getEffectMiscValueB(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectMiscValueB[idx];
    }

    uint32 getEffectTriggerSpell(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectTriggerSpell[idx];
    }

    float getEffectPointsPerComboPoint(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectPointsPerComboPoint[idx];
    }


    uint32 getSpellVisual() const { return SpellVisual; }
    uint32 getField114() const { return field114; }
    uint32 getSpellIconID() const { return spellIconID; }
    uint32 getActiveIconID() const { return activeIconID; }
    std::string getName() const { return Name; }
    std::string getRank() const { return Rank; }
    std::string getDescription() const { return Description; }
    std::string getBuffDescription() const { return BuffDescription; }
    uint32 getManaCostPercentage() const { return ManaCostPercentage; }
    uint32 getStartRecoveryCategory() const { return StartRecoveryCategory; }
    uint32 getStartRecoveryTime() const { return StartRecoveryTime; }
    uint32 getMaxTargetLevel() const { return MaxTargetLevel; }
    uint32 getSpellFamilyName() const { return SpellFamilyName; }

    uint32 getSpellGroupType(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return SpellGroupType[idx];
    }

    uint32 getMaxTargets() const { return MaxTargets; }
    uint32 getSpell_Dmg_Type() const { return Spell_Dmg_Type; }
    uint32 getPreventionType() const { return PreventionType; }

    float getDmg_multiplier(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return dmg_multiplier[idx];
    }


    uint32 getTotemCategory(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEM_CATEGORIES && idx > 0);
        return TotemCategory[idx];
    }

    int32 getRequiresAreaId() const { return RequiresAreaId; }
    uint32 getSchool() const { return School; }
    uint32 getRuneCostID() const { return RuneCostID; }
    uint32 getSpellDifficultyID() const { return SpellDifficultyID; }
    uint32 getCustom_DiminishStatus() const { return custom_DiminishStatus; }
    uint32 getCustom_proc_interval() const { return custom_proc_interval; }
    uint32 getCustom_BGR_one_buff_on_target() const { return custom_BGR_one_buff_on_target; }
    uint32 getCustom_BGR_one_buff_from_caster_on_self() const { return custom_BGR_one_buff_from_caster_on_self; }
    uint32 getCustom_c_is_flags() const { return custom_c_is_flags; }
    uint32 getCustom_RankNumber() const { return custom_RankNumber; }
    uint32 getCustom_ThreatForSpell() const { return custom_ThreatForSpell; }
    float getCustom_ThreatForSpellCoef() const { return custom_ThreatForSpellCoef; }

    uint32 getCustom_spell_coef_flags() const { return custom_spell_coef_flags; }
    float getCustom_base_range_or_radius_sqr() const { return custom_base_range_or_radius_sqr; }
    float getCone_width() const { return cone_width; }
    float getCasttime_coef() const { return casttime_coef; }
    float getFixed_dddhcoef() const { return fixed_dddhcoef; }
    float getFixed_hotdotcoef() const { return fixed_hotdotcoef; }
    float getDspell_coef_override() const { return Dspell_coef_override; }
    float getOTspell_coef_override() const { return OTspell_coef_override; }
    int getAi_target_type() const { return ai_target_type; }
    bool getCustom_self_cast_only() const { return custom_self_cast_only; }
    bool getCustom_apply_on_shapeshift_change() const { return custom_apply_on_shapeshift_change; }
    bool getCustom_always_apply() const { return custom_always_apply; }
    bool getCustom_is_melee_spell() const { return custom_is_melee_spell; }
    bool getCustom_is_ranged_spell() const { return custom_is_ranged_spell; }
    uint32 getCustom_SchoolMask() const { return custom_SchoolMask; }
    uint32 getCustomFlags() const { return CustomFlags; }

    uint32 getEffectCustomFlag(int idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS && idx > 0);
        return EffectCustomFlag[idx];
    }


#if VERSION_STRING != Cata
    uint32 getModalNextSpell() const { return modalNextSpell; }
    uint32 getRequiredItemFlags() const { return RequiredItemFlags; }
    uint32_t getEffectSpellClassMask(int idx1, int idx2) const
    {
        ARCEMU_ASSERT(idx1 < 3 && idx1 > 0 && idx2 < 3 && idx2 > 0);
        return EffectSpellClassMask[idx1][idx2];
    }
    uint32 getSpellPriority() const { return spellPriority; }
    int32 getStanceBarOrder() const { return StanceBarOrder; }
    uint32 getMinFactionID() const { return MinFactionID; }
    uint32 getMinReputation() const { return MinReputation; }
    uint32 getRequiredAuraVision() const { return RequiredAuraVision; }
        //////////////////////////////////////////////////////////////////////////////////////////
        // Applied
private:
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
    uint32 Targets;                     // not used!
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

public:

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

        //////////////////////////////////////////////////////////////////////////////////////////
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
#else

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
        int32 powerType;            // uint32  error: case value evaluates to -2, which cannot be narrowed to type 'uint32' (aka 'unsigned int') [-Wc++11-narrowing]
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
#endif
        void* (*SpellFactoryFunc);
        void* (*AuraFactoryFunc);
};
