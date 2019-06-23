/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "SpellDefines.hpp"
#include "WorldConf.h"
#include "CommonTypes.hpp"
#include <string>

class Player;
class Unit;

class SERVER_DECL SpellInfo
{
public:
    SpellInfo();
    ~SpellInfo();

    friend class SpellMgr;

    // helper functions
    bool hasEffect(uint32_t effect) const;
    bool hasEffectApplyAuraName(uint32_t auraType) const;
    bool hasCustomFlagForEffect(uint32_t effectIndex, uint32_t flag) const;

    bool isDamagingSpell() const;
    bool isHealingSpell() const;
    int firstBeneficialEffect() const;

    bool isDamagingEffect(uint8_t effIndex) const;
    bool isHealingEffect(uint8_t effIndex) const;
    bool hasDamagingEffect() const;
    bool hasHealingEffect() const;

    // Checks if spell (in most cases an aura) affects another spell, based on spell group mask
    bool isAffectingSpell(SpellInfo const* spellInfo) const;

    uint32_t getSpellDefaultDuration(Unit const* caster) const;

    bool hasTargetType(uint32_t type) const;
    int aiTargetType() const;
    bool isTargetingStealthed() const;
    bool isRequireCooldownSpell() const;

    bool isPassive() const;
    bool isProfession() const;
    bool isPrimaryProfession() const;
    bool isPrimaryProfessionSkill(uint32_t skill_id) const;

    bool isDeathPersistent() const;
    bool isChanneled() const;
    bool isRangedAutoRepeat() const;
    bool isOnNextMeleeAttack() const;

    bool appliesAreaAura(uint32_t auraType) const;
    uint32_t getAreaAuraEffect() const;

    // Getters for spell data
    uint32_t getId() const { return Id; }
    uint32_t getCategory() const { return Category; }
    uint32_t getDispelType() const { return DispelType; }
    uint32_t getMechanicsType() const { return MechanicsType; }
    uint32_t getAttributes() const { return Attributes; }
    uint32_t getAttributesEx() const { return AttributesEx; }
    uint32_t getAttributesExB() const { return AttributesExB; }
    uint32_t getAttributesExC() const { return AttributesExC; }
    uint32_t getAttributesExD() const { return AttributesExD; }
    uint32_t getAttributesExE() const { return AttributesExE; }
    uint32_t getAttributesExF() const { return AttributesExF; }
    uint32_t getAttributesExG() const { return AttributesExG; }
    uint32_t getAttributesExH() const { return AttributesExH; }
    uint32_t getAttributesExI() const { return AttributesExI; }
    uint32_t getAttributesExJ() const { return AttributesExJ; }
    uint32_t getRequiredShapeShift() const { return Shapeshifts; }
    uint32_t getShapeshiftExclude() const { return ShapeshiftsExcluded; }
    uint32_t getTargets() const { return Targets; } // not used!
    uint32_t getTargetCreatureType() const { return TargetCreatureType; }
    uint32_t getRequiresSpellFocus() const { return RequiresSpellFocus; }
    uint32_t getFacingCasterFlags() const { return FacingCasterFlags; }
    uint32_t getCasterAuraState() const { return CasterAuraState; }
    uint32_t getTargetAuraState() const { return TargetAuraState; }
    uint32_t getCasterAuraStateNot() const { return CasterAuraStateNot; }
    uint32_t getTargetAuraStateNot() const { return TargetAuraStateNot; }
    uint32_t getCasterAuraSpell() const { return casterAuraSpell; }
    uint32_t getTargetAuraSpell() const { return targetAuraSpell; }
    uint32_t getCasterAuraSpellNot() const { return casterAuraSpellNot; }
    uint32_t getTargetAuraSpellNot() const { return targetAuraSpellNot; }
    uint32_t getCastingTimeIndex() const { return CastingTimeIndex; }
    uint32_t getRecoveryTime() const { return RecoveryTime; }
    uint32_t getCategoryRecoveryTime() const { return CategoryRecoveryTime; }
    uint32_t getInterruptFlags() const { return InterruptFlags; }
    uint32_t getAuraInterruptFlags() const { return AuraInterruptFlags; }
    uint32_t getChannelInterruptFlags() const { return ChannelInterruptFlags; }
    uint32_t getProcFlags() const { return procFlags; }
    uint32_t getProcChance() const { return procChance; }
    uint32_t getProcCharges() const { return procCharges; }
    uint32_t getMaxLevel() const { return maxLevel; }
    uint32_t getBaseLevel() const { return baseLevel; }
    uint32_t getSpellLevel() const { return spellLevel; }
    uint32_t getDurationIndex() const { return DurationIndex; }
    int32_t getPowerType() const { return powerType; }
    uint32_t getManaCost() const { return manaCost; }
    uint32_t getManaCostPerlevel() const { return manaCostPerlevel; } // not used!
    uint32_t getManaPerSecond() const { return manaPerSecond; } // not used!
    uint32_t getManaPerSecondPerLevel() const { return manaPerSecondPerLevel; } // not used!
    uint32_t getRangeIndex() const { return rangeIndex; }
    float getSpeed() const { return speed; }
    uint32_t getMaxstack() const { return MaxStackAmount; }

    uint32_t getTotem(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEMS);
        return Totem[idx];
    }

    int32_t getReagent(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS);
        return Reagent[idx];
    }

    uint32_t getReagentCount(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS);
        return ReagentCount[idx];
    }

    int32_t getEquippedItemClass() const { return EquippedItemClass; }
    int32_t getEquippedItemSubClass() const { return EquippedItemSubClass; }
    int32_t getEquippedItemInventoryTypeMask() const { return EquippedItemInventoryTypeMask; }

    uint32_t getEffect(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return Effect[idx];
    }

    int32_t getEffectDieSides(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectDieSides[idx];
    }

    float getEffectRealPointsPerLevel(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectRealPointsPerLevel[idx];
    }

    int32_t getEffectBasePoints(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectBasePoints[idx];
    }

    uint32_t getEffectMechanic(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectMechanic[idx];
    }

    uint32_t getEffectImplicitTargetA(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectImplicitTargetA[idx];
    }

    uint32_t getEffectImplicitTargetB(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectImplicitTargetB[idx];
    }

    uint32_t getEffectRadiusIndex(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectRadiusIndex[idx];
    }

    uint32_t getEffectApplyAuraName(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectApplyAuraName[idx];
    }

    uint32_t getEffectAmplitude(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectAmplitude[idx];
    }

    float getEffectMultipleValue(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectMultipleValue[idx];
    }

    uint32_t getEffectChainTarget(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectChainTarget[idx];
    }

    uint32_t getEffectItemType(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectItemType[idx];
    }

    int32_t getEffectMiscValue(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectMiscValue[idx];
    }

    int32_t getEffectMiscValueB(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectMiscValueB[idx];
    }

    uint32_t getEffectTriggerSpell(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectTriggerSpell[idx];
    }

    float getEffectPointsPerComboPoint(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectPointsPerComboPoint[idx];
    }

    uint32_t getEffectSpellClassMask(uint8_t idx1, uint8_t idx2) const
    {
        ARCEMU_ASSERT(idx1 < MAX_SPELL_EFFECTS && idx2 < MAX_SPELL_EFFECTS);
        return EffectSpellClassMask[idx1][idx2];
    }

    uint32_t* getEffectSpellClassMask(uint8_t idx1)
    {
        ARCEMU_ASSERT(idx1 < MAX_SPELL_EFFECTS);
        return EffectSpellClassMask[idx1];
    }

    uint32_t getSpellVisual() const { return SpellVisual; }
    uint32_t getSpellIconID() const { return spellIconID; }
    uint32_t getActiveIconID() const { return activeIconID; }
    uint32_t getSpellPriority() const { return spellPriority; } // not used!
    std::string getName() const { return Name; }
    std::string getRank() const { return Rank; }
    uint32_t getManaCostPercentage() const { return ManaCostPercentage; }
    uint32_t getStartRecoveryCategory() const { return StartRecoveryCategory; }
    uint32_t getStartRecoveryTime() const { return StartRecoveryTime; }
    uint32_t getMaxTargetLevel() const { return MaxTargetLevel; }
    uint32_t getSpellFamilyName() const { return SpellFamilyName; }

    uint32_t getSpellFamilyFlags(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return SpellFamilyFlags[idx];
    }

    const uint32_t* getSpellFamilyFlags() const { return SpellFamilyFlags; }

    uint32_t getMaxTargets() const { return MaxTargets; }
    uint32_t getDmgClass() const { return DmgClass; }
    uint32_t getPreventionType() const { return PreventionType; }

    float getEffectDamageMultiplier(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectDamageMultiplier[idx];
    }

    uint32_t getTotemCategory(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEM_CATEGORIES);
        return TotemCategory[idx];
    }

    int32_t getRequiresAreaId() const { return AreaGroupId; }
    uint32_t getSchool() const { return School; }
    uint32_t getRuneCostID() const { return RuneCostID; }

    float getEffectBonusMultiplier(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectBonusMultiplier[idx];
    }

    float* getEffectBonusMultiplier()
    {
        return EffectBonusMultiplier;
    }

    uint32_t getSpellDifficultyID() const { return SpellDifficultyId; }

    //////////////////////////////////////////////////////////////////////////////////////////
    //custom values
    uint32_t getCustom_proc_interval() const { return custom_proc_interval; }
    uint32_t getCustom_BGR_one_buff_on_target() const { return custom_BGR_one_buff_on_target; }
    uint32_t getCustom_c_is_flags() const { return custom_c_is_flags; }
    uint32_t getCustom_RankNumber() const { return custom_RankNumber; }
    uint32_t getCustom_ThreatForSpell() const { return custom_ThreatForSpell; }
    float getCustom_ThreatForSpellCoef() const { return custom_ThreatForSpellCoef; }

    float getCustom_base_range_or_radius_sqr() const { return custom_base_range_or_radius_sqr; }
    float getCone_width() const { return cone_width; }
    int getAi_target_type() const { return ai_target_type; }
    bool getCustom_self_cast_only() const { return custom_self_cast_only; }
    bool getCustom_apply_on_shapeshift_change() const { return custom_apply_on_shapeshift_change; }
    bool getCustom_is_melee_spell() const { return custom_is_melee_spell; }
    bool getCustom_is_ranged_spell() const { return custom_is_ranged_spell; }
    uint32_t getCustom_SchoolMask() const { return custom_SchoolMask; }

    uint32_t getEffectCustomFlag(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectCustomFlag[idx];
    }
        
private:
    // Setters for spell data
    void setId(uint32_t value) { Id = value; }
    void setCategory(uint32_t value) { Category = value; }
    void setDispelType(uint32_t value) { DispelType = value; }              // used in HackFixes.cpp
    void setMechanicsType(uint32_t value) { MechanicsType = value; }        // used in HackFixes.cpp

    void setAttributes(uint32_t value) { Attributes = value; }              // used in HackFixes.cpp
    void addAttributes(uint32_t value) { Attributes |= value; }             // used in HackFixes.cpp
    void removeAttributes(uint32_t value) { Attributes &= ~value; }         // used in HackFixes.cpp

    void setAttributesEx(uint32_t value) { AttributesEx = value; }          // used in HackFixes.cpp
    void addAttributesEx(uint32_t value) { AttributesEx |= value; }         // used in HackFixes.cpp

    void setAttributesExB(uint32_t value) { AttributesExB = value; }        // used in HackFixes.cpp

    void setAttributesExC(uint32_t value) { AttributesExC = value; }
    void addAttributesExC(uint32_t value) { AttributesExC |= value; }       // used in HackFixes.cpp

    void setAttributesExD(uint32_t value) { AttributesExD = value; }
    void setAttributesExE(uint32_t value) { AttributesExE = value; }
    void setAttributesExF(uint32_t value) { AttributesExF = value; }
    void setAttributesExG(uint32_t value) { AttributesExG = value; }
    void setAttributesExH(uint32_t value) { AttributesExH = value; }
    void setAttributesExI(uint32_t value) { AttributesExI = value; }
    void setAttributesExJ(uint32_t value) { AttributesExJ = value; }
    void setRequiredShapeShift(uint32_t value) { Shapeshifts = value; } // used in HackFixes.cpp
    void setShapeshiftExclude(uint32_t value) { ShapeshiftsExcluded = value; }
    void setTargets(uint32_t value) { Targets = value; }
    void setTargetCreatureType(uint32_t value) { TargetCreatureType = value; }
    void setRequiresSpellFocus(uint32_t value) { RequiresSpellFocus = value; }
    void setFacingCasterFlags(uint32_t value) { FacingCasterFlags = value; } // used in HackFixes.cpp
    void setCasterAuraState(uint32_t value) { CasterAuraState = value; }    // used in HackFixes.cpp
    void setTargetAuraState(uint32_t value) { TargetAuraState = value; }    // used in HackFixes.cpp
    void setCasterAuraStateNot(uint32_t value) { CasterAuraStateNot = value; }    // used in HackFixes.cpp
    void setTargetAuraStateNot(uint32_t value) { TargetAuraStateNot = value; }    // used in HackFixes.cpp
    void setCasterAuraSpell(uint32_t value) { casterAuraSpell = value; }    // used in HackFixes.cpp
    void setTargetAuraSpell(uint32_t value) { targetAuraSpell = value; }    // used in HackFixes.cpp
    void setCasterAuraSpellNot(uint32_t value) { casterAuraSpellNot = value; }    // used in HackFixes.cpp
    void setTargetAuraSpellNot(uint32_t value) { targetAuraSpellNot = value; }    // used in HackFixes.cpp
    void setCastingTimeIndex(uint32_t value) { CastingTimeIndex = value; }    // used in HackFixes.cpp
    void setRecoveryTime(uint32_t value) { RecoveryTime = value; }    // used in HackFixes.cpp / Spell_ClassScripts.cpp
    void setCategoryRecoveryTime(uint32_t value) { CategoryRecoveryTime = value; }    // used in HackFixes.cpp

    void setInterruptFlags(uint32_t value) { InterruptFlags = value; }
    void removeInterruptFlags(uint32_t value) { InterruptFlags |= ~value; }    // used in HackFixes.cpp

    void addAuraInterruptFlags(uint32_t value) { AuraInterruptFlags |= value; }    // used in HackFixes.cpp
    void setAuraInterruptFlags(uint32_t value) { AuraInterruptFlags = value; }    // used in HackFixes.cpp

    void setChannelInterruptFlags(uint32_t value) { ChannelInterruptFlags = value; }    // used in HackFixes.cpp

    void setProcFlags(uint32_t value) { procFlags = value; }    // used in HackFixes.cpp
    void addProcFlags(uint32_t value) { procFlags |= value; }    // used in HackFixes.cpp

    void setProcChance(uint32_t value) { procChance = value; }    // used in HackFixes.cpp
    void setProcCharges(uint32_t value) { procCharges = value; }    // used in HackFixes.cpp
    void setMaxLevel(uint32_t value) { maxLevel = value; }
    void setBaseLevel(uint32_t value) { baseLevel = value; }
    void setSpellLevel(uint32_t value) { spellLevel = value; }    // used in HackFixes.cpp
    void setDurationIndex(uint32_t value) { DurationIndex = value; }    // used in HackFixes.cpp / SpellEffects.cpp
    void setPowerType(int32_t value) { powerType = value; }
    void setManaCost(uint32_t value) { manaCost = value; }
    void setManaCostPerlevel(uint32_t value) { manaCostPerlevel = value; }
    void setManaPerSecond(uint32_t value) { manaPerSecond = value; }
    void setManaPerSecondPerLevel(uint32_t value) { manaPerSecondPerLevel = value; }
    void setRangeIndex(uint32_t value) { rangeIndex = value; }    // used in HackFixes.cpp
    void setSpeed(float value) { speed = value; }    // used in HackFixes.cpp
    void setMaxstack(uint32_t value) { MaxStackAmount = value; }    // used in HackFixes.cpp

    void setTotem(uint32_t totemId, uint8_t idx)                      // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEMS);
        Totem[idx] = totemId;
    }

    void setReagent(int32_t reagentId, uint8_t idx)                      // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS);
        Reagent[idx] = reagentId;
    }

    void setReagentCount(uint32_t reagentId, uint8_t idx)                 // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS);
        ReagentCount[idx] = reagentId;
    }

    void setEquippedItemClass(int32_t value) { EquippedItemClass = value; }    // used in HackFixes.cpp
    void setEquippedItemSubClass(int32_t value) { EquippedItemSubClass = value; }
    void setEquippedItemInventoryTypeMask(int32_t value) { EquippedItemInventoryTypeMask = value; }

    void setEffect(uint32_t effectId, uint8_t idx)                          // used in HackFixes.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        Effect[idx] = effectId;
    }

    void setEffectDieSides(int32_t effecSide, uint8_t idx)                 // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectDieSides[idx] = effecSide;
    }

    void setEffectRealPointsPerLevel(float pointsPerLevel, uint8_t idx)   // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectRealPointsPerLevel[idx] = pointsPerLevel;
    }

    void setEffectBasePoints(int32_t pointsPerLevel, uint8_t idx)               // used in HackFixes.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectBasePoints[idx] = pointsPerLevel;
    }

    void setEffectMechanic(uint32_t mechanic, uint8_t idx)                       // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectMechanic[idx] = mechanic;
    }

    void setEffectImplicitTargetA(uint32_t targetA, uint8_t idx)                // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectImplicitTargetA[idx] = targetA;
    }

    void setEffectImplicitTargetB(uint32_t targetB, uint8_t idx)                // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectImplicitTargetB[idx] = targetB;
    }

    void setEffectRadiusIndex(uint32_t radiusIndex, uint8_t idx)                // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectRadiusIndex[idx] = radiusIndex;
    }

    void setEffectApplyAuraName(uint32_t auraName, uint8_t idx)                 // used in HackFixes.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectApplyAuraName[idx] = auraName;
    }

    void setEffectAmplitude(uint32_t amplitude, uint8_t idx)                    // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectAmplitude[idx] = amplitude;
    }

    void setEffectMultipleValue(float multiply, uint8_t idx)                   // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectMultipleValue[idx] = multiply;
    }

    void setEffectChainTarget(uint32_t chainTarget, uint8_t idx)                // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectChainTarget[idx] = chainTarget;
    }

    void setEffectItemType(uint32_t itemEntryId, uint8_t idx)
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectItemType[idx] = itemEntryId;
    }

    void setEffectMiscValue(int32_t misc, uint8_t idx)                          // used in HackFixes.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectMiscValue[idx] = misc;
    }

    void setEffectMiscValueB(int32_t miscB, uint8_t idx)
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectMiscValueB[idx] = miscB;
    }

    void setEffectTriggerSpell(uint32_t spell, uint8_t idx)                     // used in ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectTriggerSpell[idx] = spell;
    }

    void setEffectPointsPerComboPoint(float effectPoints, uint8_t idx)          // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectPointsPerComboPoint[idx] = effectPoints;
    }

    void setEffectSpellClassMask(uint32_t spellClass, uint8_t idx1, uint8_t idx2)           // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx1 < MAX_SPELL_EFFECTS && idx2 < MAX_SPELL_EFFECTS);
        EffectSpellClassMask[idx1][idx2] = spellClass;
    }

    void setSpellVisual(uint32_t value) { SpellVisual = value; }
    void setSpellIconID(uint32_t value) { spellIconID = value; }
    void setActiveIconID(uint32_t value) { activeIconID = value; }
    void setSpellPriority(uint32_t value) { spellPriority = value; }
    void setName(std::string value) { Name = value; }
    void setRank(std::string value) { Rank = value; }
    void setManaCostPercentage(uint32_t value) { ManaCostPercentage = value; }
    void setStartRecoveryCategory(uint32_t value) { StartRecoveryCategory = value; }
    void setStartRecoveryTime(uint32_t value) { StartRecoveryTime = value; }
    void setMaxTargetLevel(uint32_t value) { MaxTargetLevel = value; }
    void setSpellFamilyName(uint32_t value) { SpellFamilyName = value; }        // used in HackFixes.cpp

    void setSpellFamilyFlags(uint32_t value, uint8_t idx)                             // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        SpellFamilyFlags[idx] = value;
    }

    void setMaxTargets(uint32_t value) { MaxTargets = value; }        // used in HackFixes.cpp
    void setDmgClass(uint32_t value) { DmgClass = value; }        // used in HackFixes.cpp
    void setPreventionType(uint32_t value) { PreventionType = value; }

    void setEffectDamageMultiplier(float dmgMultiplier, uint8_t idx)                       // used in HackFixes.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectDamageMultiplier[idx] = dmgMultiplier;
    }

    void setTotemCategory(uint32_t category, uint8_t idx)
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEM_CATEGORIES);
        TotemCategory[idx] = category;
    }

    void setRequiresAreaId(int32_t value) { AreaGroupId = value; }
    void setSchool(uint32_t value) { School = value; }                  // used in HackFixes.cpp
    void setRuneCostID(uint32_t value) { RuneCostID = value; }

    void setEffectBonusMultiplier(float value, uint8_t idx)
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectBonusMultiplier[idx] = value;
    }

    void setSpellDifficultyID(uint32_t value) { SpellDifficultyId = value; }

#if VERSION_STRING >= Cata
    void setEffectRadiusMaxIndex(uint32_t value, uint8_t idx)
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectRadiusMaxIndex[idx] = value;
    }

    void setEffectSpellId(uint32_t value, uint8_t idx)
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectSpellId[idx] = value;
    }

    void setEffectIndex(uint32_t value, uint8_t idx)
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectIndex[idx] = value;
    }
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Applied values from DBC
    uint32_t Id;
    // Data from SpellCategories.dbc (in Cataclysm)
    uint32_t Category;
    uint32_t DispelType;
    uint32_t MechanicsType;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t Attributes;
    uint32_t AttributesEx;
    uint32_t AttributesExB;
    uint32_t AttributesExC;
    uint32_t AttributesExD;
    uint32_t AttributesExE;
    uint32_t AttributesExF;
    uint32_t AttributesExG;
    uint32_t AttributesExH;
    uint32_t AttributesExI;
    uint32_t AttributesExJ;
    // Data from SpellShapeshift.dbc (in Cataclysm)
    uint32_t Shapeshifts;
    uint32_t ShapeshiftsExcluded;
    // Data from SpellTargetRestrictions.dbc (in Cataclysm)
    uint32_t Targets;
    uint32_t TargetCreatureType;
    // Data from SpellCastingRequirements.dbc (in Cataclysm)
    uint32_t RequiresSpellFocus;
    uint32_t FacingCasterFlags;
    // Data from SpellAuraRestrictions.dbc (in Cataclysm)
    uint32_t CasterAuraState;
    uint32_t TargetAuraState;
    uint32_t CasterAuraStateNot;
    uint32_t TargetAuraStateNot;
    uint32_t casterAuraSpell;
    uint32_t targetAuraSpell;
    uint32_t casterAuraSpellNot;
    uint32_t targetAuraSpellNot;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t CastingTimeIndex;
    // Data from SpellCooldowns.dbc (in Cataclysm)
    uint32_t RecoveryTime;
    uint32_t CategoryRecoveryTime;
    // Data from SpellInterrupts.dbc (in Cataclysm)
    uint32_t InterruptFlags;
    uint32_t AuraInterruptFlags;
    uint32_t ChannelInterruptFlags;
    // Data from SpellAuraOptions.dbc (in Cataclysm)
    uint32_t procFlags;
    uint32_t procChance;
    uint32_t procCharges;
    // Data from SpellLevels.dbc (in Cataclysm)
    uint32_t maxLevel;
    uint32_t baseLevel;
    uint32_t spellLevel;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t DurationIndex;
    int32_t powerType;
    // Data from SpellPower.dbc (in Cataclysm)
    uint32_t manaCost;
    uint32_t manaCostPerlevel;
    uint32_t manaPerSecond;
    uint32_t manaPerSecondPerLevel;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t rangeIndex;
    float speed;
    // Data from SpellAuraOptions.dbc (in Cataclysm)
    uint32_t MaxStackAmount;
    // Data from SpellTotems.dbc (in Cataclysm)
    uint32_t Totem[MAX_SPELL_TOTEMS];
    // Data from SpellReagents.dbc (in Cataclysm)
    int32_t Reagent[MAX_SPELL_REAGENTS];
    uint32_t ReagentCount[MAX_SPELL_REAGENTS];
    // Data from SpellEquippedItems.dbc (in Cataclysm)
    int32_t EquippedItemClass;
    int32_t EquippedItemSubClass;
    int32_t EquippedItemInventoryTypeMask;
    // Data from SpellEffect.dbc (in Cataclysm)
    uint32_t Effect[MAX_SPELL_EFFECTS];
    int32_t EffectDieSides[MAX_SPELL_EFFECTS];
    float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];
    int32_t EffectBasePoints[MAX_SPELL_EFFECTS];
    uint32_t EffectMechanic[MAX_SPELL_EFFECTS];
    uint32_t EffectImplicitTargetA[MAX_SPELL_EFFECTS];
    uint32_t EffectImplicitTargetB[MAX_SPELL_EFFECTS];
    uint32_t EffectRadiusIndex[MAX_SPELL_EFFECTS];
    uint32_t EffectApplyAuraName[MAX_SPELL_EFFECTS];
    uint32_t EffectAmplitude[MAX_SPELL_EFFECTS];
    float EffectMultipleValue[MAX_SPELL_EFFECTS];
    uint32_t EffectChainTarget[MAX_SPELL_EFFECTS];
    uint32_t EffectItemType[MAX_SPELL_EFFECTS];                 //ItemEntryId
    int32_t EffectMiscValue[MAX_SPELL_EFFECTS];                 //can be: creature, go, area, smt, speed
    int32_t EffectMiscValueB[MAX_SPELL_EFFECTS];                //can be: speed slot-type, summon
    uint32_t EffectTriggerSpell[MAX_SPELL_EFFECTS];
    float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS];
    uint32_t EffectSpellClassMask[MAX_SPELL_EFFECTS][3];
#if VERSION_STRING >= Cata
    uint32_t EffectRadiusMaxIndex[MAX_SPELL_EFFECTS];
    uint32_t EffectSpellId[MAX_SPELL_EFFECTS];
    uint32_t EffectIndex[MAX_SPELL_EFFECTS];
#endif
    // Data from Spell.dbc (in Cataclysm)
    uint32_t SpellVisual;
    uint32_t spellIconID;
    uint32_t activeIconID;
    uint32_t spellPriority;
    std::string Name;
    std::string Rank;
    // Data from SpellPower.dbc (in Cataclysm)
    uint32_t ManaCostPercentage;
    // Data from SpellCategories.dbc (in Cataclysm)
    uint32_t StartRecoveryCategory;
    // Data from SpellCooldowns.dbc (in Cataclysm)
    uint32_t StartRecoveryTime;
    // Data from SpellTargetRestrictions.dbc (in Cataclysm)
    uint32_t MaxTargetLevel;
    // Data from SpellClassOptions.dbc (in Cataclysm)
    uint32_t SpellFamilyName;
    uint32_t SpellFamilyFlags[MAX_SPELL_EFFECTS];
    // Data from SpellTargetRestrictions.dbc (in Cataclysm)
    uint32_t MaxTargets;
    // Data from SpellCategories.dbc (in Cataclysm)
    uint32_t DmgClass;
    uint32_t PreventionType;
    // Data from SpellEffect.dbc (in Cataclysm)
    float EffectDamageMultiplier[MAX_SPELL_EFFECTS];
    // Data from SpellTotems.dbc (in Cataclysm)
    uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];     // not used!
    // Data from SpellCastingRequirements.dbc (in Cataclysm)
    int32_t AreaGroupId;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t School;
    uint32_t RuneCostID;
    // Data from SpellEffect.dbc (in Cataclysm)
    float EffectBonusMultiplier[MAX_SPELL_EFFECTS];
    // Data from SpellDifficulty.dbc (in Cataclysm)
    uint32_t SpellDifficultyId;

    // Script links
    void* (*spellScriptLink);
    void* (*auraScriptLink);

public:
#if VERSION_STRING >= Cata
    // DBC links
    uint32_t SpellScalingId;                              // SpellScaling.dbc
    uint32_t SpellAuraOptionsId;                          // SpellAuraOptions.dbc
    uint32_t SpellAuraRestrictionsId;                     // SpellAuraRestrictions.dbc
    uint32_t SpellCastingRequirementsId;                  // SpellCastingRequirements.dbc
    uint32_t SpellCategoriesId;                           // SpellCategories.dbc
    uint32_t SpellClassOptionsId;                         // SpellClassOptions.dbc
    uint32_t SpellCooldownsId;                            // SpellCooldowns.dbc
    uint32_t SpellEquippedItemsId;                        // SpellEquippedItems.dbc
    uint32_t SpellInterruptsId;                           // SpellInterrupts.dbc
    uint32_t SpellLevelsId;                               // SpellLevels.dbc
    uint32_t SpellPowerId;                                // SpellPower.dbc
    uint32_t SpellReagentsId;                             // SpellReagents.dbc
    uint32_t SpellShapeshiftId;                           // SpellShapeshift.dbc
    uint32_t SpellTargetRestrictionsId;                   // SpellTargetRestrictions.dbc
    uint32_t SpellTotemsId;                               // SpellTotems.dbc
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell coefficients

    // Direct damage or direct heal coefficient
    float spell_coeff_direct;

    // Damage or healing over-time coefficient (NOTE: This is per tick, not for full duration, unlike the SQL override variable)
    float spell_coeff_overtime;

    // SQL override coefficients (table spell_coefficient_override)
    float spell_coeff_direct_override;
    float spell_coeff_overtime_override;

    //////////////////////////////////////////////////////////////////////////////////////////
    //custom values

    // from MySQL table spell_proc - 1887 spells
    uint32_t custom_proc_interval;

    // from MySQL table spell_custom_assign - 1970 spells
    uint32_t custom_BGR_one_buff_on_target;

    // from MySQL table spell_custom_assign - 353 spells
    // also flags added in SpellCustomizations::SetMissingCIsFlags
    uint32_t custom_c_is_flags;

    // from MySQL table spell_ranks - 6546 spells
    uint32_t custom_RankNumber;

    // set in HackFixes.cpp for all Dummy Trigger
    uint32_t custom_NameHash;

    // from MySQL table ai_threattospellid - 144 spells
    int32_t custom_ThreatForSpell;

    // from MySQL table ai_threattospellid - 118 spells
    float custom_ThreatForSpellCoef;

    // set in HackFixes.cpp for all spells
    float custom_base_range_or_radius_sqr;

    // set in HackFixes.cpp - 1 spell (26029)
    float cone_width;

    // set in HackFixes.cpp for all spells
    // check out SpellInfo::aiTargetType
    int ai_target_type;

    // set in Hackfixes.cpp - 5 spells
    // from MySQL table spell_custom_assign - 6 spells
    bool custom_self_cast_only;

    // SpellCustomizations::SetOnShapeshiftChange - 2 spells
    bool custom_apply_on_shapeshift_change;

    // set in Hackfixes.cpp - 3 spells
    // set in SpellCustomizations::SetMeleeSpellBool based on school and effect
    bool custom_is_melee_spell;

    // set in Hackfixes.cpp - 1 spells (2094)
    // set in SpellCustomizations::SetRangedSpellBool based on school and dmg type
    bool custom_is_ranged_spell;

    // set in HackFixes.cpp for all spells, based on school
    uint32_t custom_SchoolMask;

    // from MySQL table spell_effects_override - 374 spells
    uint32_t EffectCustomFlag[MAX_SPELL_EFFECTS];
};
