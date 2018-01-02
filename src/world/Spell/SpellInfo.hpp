/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
    bool HasEffect(uint32_t effect) const;
    bool HasEffectApplyAuraName(uint32_t aura_name);
    bool HasCustomFlagForEffect(uint32_t effect, uint32_t flag);

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
    bool IsPrimaryProfessionSkill(uint32_t skill_id);

    bool isDeathPersistent() const;

    bool appliesAreaAura(uint32_t aura) const;
    uint32_t GetAreaAuraEffectId();

    uint32_t getId() const { return Id; }
    void setId(uint32_t value) { Id = value; }

    uint32_t getCategory() const { return Category; }

    uint32_t getDispelType() const { return DispelType; }
    void setDispelType(uint32_t value) { DispelType = value; }              // used in HackFixes.cpp

    uint32_t getMechanicsType() const { return MechanicsType; }
    void setMechanicsType(uint32_t value) { MechanicsType = value; }        // used in HackFixes.cpp

    uint32_t getAttributes() const { return Attributes; }
    void setAttributes(uint32_t value) { Attributes = value; }              // used in HackFixes.cpp / SpellCustomizations.cpp
    void addAttributes(uint32_t value) { Attributes |= value; }             // used in HackFixes.cpp
    void removeAttributes(uint32_t value) { Attributes &= ~value; }         // used in HackFixes.cpp

    uint32_t getAttributesEx() const { return AttributesEx; }
    void setAttributesEx(uint32_t value) { AttributesEx = value; }          // used in HackFixes.cpp / SpellCustomizations.cpp
    void addAttributesEx(uint32_t value) { AttributesEx |= value; }         // used in HackFixes.cpp

    uint32_t getAttributesExB() const { return AttributesExB; }
    void setAttributesExB(uint32_t value) { AttributesExB = value; }        // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getAttributesExC() const { return AttributesExC; }
    void setAttributesExC(uint32_t value) { AttributesExC = value; }        // used in SpellCustomizations.cpp
    void addAttributesExC(uint32_t value) { AttributesExC |= value; }       // used in HackFixes.cpp

    uint32_t getAttributesExD() const { return AttributesExD; }
    void setAttributesExD(uint32_t value) { AttributesExD = value; }        // used in SpellCustomizations.cpp

    uint32_t getAttributesExE() const { return AttributesExE; }
    void setAttributesExE(uint32_t value) { AttributesExE = value; }        // used in SpellCustomizations.cpp

    uint32_t getAttributesExF() const { return AttributesExF; }
    void setAttributesExF(uint32_t value) { AttributesExF = value; }        // used in SpellCustomizations.cpp

    uint32_t getAttributesExG() const { return AttributesExG; }
    void setAttributesExG(uint32_t value) { AttributesExG = value; }        // used in SpellCustomizations.cpp

    uint32_t getRequiredShapeShift() const { return RequiredShapeShift; }
    void setRequiredShapeShift(uint32_t value) { RequiredShapeShift = value; } // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getShapeshiftExclude() const { return ShapeshiftExclude; }
    void setShapeshiftExclude(uint32_t value) { ShapeshiftExclude = value; } // used in SpellCustomizations.cpp

    uint32_t getTargets() const { return Targets; }                           // not used!
    void setTargets(uint32_t value) { Targets = value; }                    // used in SpellCustomizations.cpp

    uint32_t getTargetCreatureType() const { return TargetCreatureType; }
    void setTargetCreatureType(uint32_t value) { TargetCreatureType = value; } // used in SpellCustomizations.cpp

    uint32_t getRequiresSpellFocus() const { return RequiresSpellFocus; }
    void setRequiresSpellFocus(uint32_t value) { RequiresSpellFocus = value; } // used in SpellCustomizations.cpp

    uint32_t getFacingCasterFlags() const { return FacingCasterFlags; }
    void setFacingCasterFlags(uint32_t value) { FacingCasterFlags = value; } // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getCasterAuraState() const { return CasterAuraState; }
    void setCasterAuraState(uint32_t value) { CasterAuraState = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getTargetAuraState() const { return TargetAuraState; }
    void setTargetAuraState(uint32_t value) { TargetAuraState = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getCasterAuraStateNot() const { return CasterAuraStateNot; }
    void setCasterAuraStateNot(uint32_t value) { CasterAuraStateNot = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getTargetAuraStateNot() const { return TargetAuraStateNot; }
    void setTargetAuraStateNot(uint32_t value) { TargetAuraStateNot = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getCasterAuraSpell() const { return casterAuraSpell; }
    void setCasterAuraSpell(uint32_t value) { casterAuraSpell = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getTargetAuraSpell() const { return targetAuraSpell; }
    void setTargetAuraSpell(uint32_t value) { targetAuraSpell = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getCasterAuraSpellNot() const { return casterAuraSpellNot; }
    void setCasterAuraSpellNot(uint32_t value) { casterAuraSpellNot = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getTargetAuraSpellNot() const { return targetAuraSpellNot; }
    void setTargetAuraSpellNot(uint32_t value) { targetAuraSpellNot = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getCastingTimeIndex() const { return CastingTimeIndex; }
    void setCastingTimeIndex(uint32_t value) { CastingTimeIndex = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getRecoveryTime() const { return RecoveryTime; }
    void setRecoveryTime(uint32_t value) { RecoveryTime = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp / Spell_ClassScripts.cpp

    uint32_t getCategoryRecoveryTime() const { return CategoryRecoveryTime; }
    void setCategoryRecoveryTime(uint32_t value) { CategoryRecoveryTime = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getInterruptFlags() const { return InterruptFlags; }
    void setInterruptFlags(uint32_t value) { InterruptFlags = value; }    // used in SpellCustomizations.cpp
    void removeInterruptFlags(uint32_t value) { InterruptFlags |= ~value; }    // used in HackFixes.cpp

    uint32_t getAuraInterruptFlags() const { return AuraInterruptFlags; }
    void addAuraInterruptFlags(uint32_t value) { AuraInterruptFlags |= value; }    // used in HackFixes.cpp
    void setAuraInterruptFlags(uint32_t value) { AuraInterruptFlags = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getChannelInterruptFlags() const { return ChannelInterruptFlags; }
    void setChannelInterruptFlags(uint32_t value) { ChannelInterruptFlags = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getProcFlags() const { return procFlags; }
    void setProcFlags(uint32_t value) { procFlags = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp
    void addProcFlags(uint32_t value) { procFlags |= value; }    // used in HackFixes.cpp

    uint32_t getProcChance() const { return procChance; }
    void setProcChance(uint32_t value) { procChance = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getProcCharges() const { return procCharges; }
    void setProcCharges(uint32_t value) { procCharges = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getMaxLevel() const { return maxLevel; }
    void setMaxLevel(uint32_t value) { maxLevel = value; }    // used in SpellCustomizations.cpp

    uint32_t getBaseLevel() const { return baseLevel; }
    void setBaseLevel(uint32_t value) { baseLevel = value; }    // used in SpellCustomizations.cpp

    uint32_t getSpellLevel() const { return spellLevel; }
    void setSpellLevel(uint32_t value) { spellLevel = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getDurationIndex() const { return DurationIndex; }
    void setDurationIndex(uint32_t value) { DurationIndex = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp / SpellEffects.cpp

    int32_t getPowerType() const { return powerType; }
    void setPowerType(int32_t value) { powerType = value; }    // used in SpellCustomizations.cpp

    uint32_t getManaCost() const { return manaCost; }
    void setManaCost(uint32_t value) { manaCost = value; }    // used in SpellCustomizations.cpp

    uint32_t getManaCostPerlevel() const { return manaCostPerlevel; }           // not used!
    void setManaCostPerlevel(uint32_t value) { manaCostPerlevel = value; }    // used in SpellCustomizations.cpp

    uint32_t getManaPerSecond() const { return manaPerSecond; }           // not used!
    void setManaPerSecond(uint32_t value) { manaPerSecond = value; }    // used in SpellCustomizations.cpp

    uint32_t getManaPerSecondPerLevel() const { return manaPerSecondPerLevel; }           // not used!
    void setManaPerSecondPerLevel(uint32_t value) { manaPerSecondPerLevel = value; }    // used in SpellCustomizations.cpp

    uint32_t getRangeIndex() const { return rangeIndex; }
    void setRangeIndex(uint32_t value) { rangeIndex = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    float getSpeed() const { return speed; }
    void setSpeed(float value) { speed = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getMaxstack() const { return maxstack; }
    void setMaxstack(uint32_t value) { maxstack = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getTotem(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEMS);
        return Totem[idx];
    }

    void setTotem(uint32_t totemId, uint8_t idx)                      // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEMS);
        Totem[idx] = totemId;
    }

    uint32_t getReagent(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS);
        return Reagent[idx];
    }

    void setReagent(uint32_t reagentId, uint8_t idx)                      // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS);
        Reagent[idx] = reagentId;
    }

    uint32_t getReagentCount(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS);
        return ReagentCount[idx];
    }

    void setReagentCount(uint32_t reagentId, uint8_t idx)                 // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_REAGENTS);
        ReagentCount[idx] = reagentId;
    }

    int32_t getEquippedItemClass() const { return EquippedItemClass; }
    void setEquippedItemClass(int32_t value) { EquippedItemClass = value; }    // used in HackFixes.cpp / SpellCustomizations.cpp

    int32_t getEquippedItemSubClass() const { return EquippedItemSubClass; }
    void setEquippedItemSubClass(int32_t value) { EquippedItemSubClass = value; }    // used in SpellCustomizations.cpp

    uint32_t getEffect(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return Effect[idx];
    }

    void setEffect(uint32_t effectId, uint8_t idx)                          // used in HackFixes.cpp / SpellCustomizations.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        Effect[idx] = effectId;
    }

    int32_t getEffectDieSides(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectDieSides[idx];
    }

    void setEffectDieSides(int32_t effecSide, uint8_t idx)                 // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectDieSides[idx] = effecSide;
    }

    float getEffectRealPointsPerLevel(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectRealPointsPerLevel[idx];
    }

    void setEffectRealPointsPerLevel(float pointsPerLevel, uint8_t idx)   // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectRealPointsPerLevel[idx] = pointsPerLevel;
    }

    int32_t getEffectBasePoints(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectBasePoints[idx];
    }

    void setEffectBasePoints(int32_t pointsPerLevel, uint8_t idx)               // used in HackFixes.cpp / SpellCustomizations.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectBasePoints[idx] = pointsPerLevel;
    }

    int32_t getEffectMechanic(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectMechanic[idx];
    }

    void setEffectMechanic(int32_t mechanic, uint8_t idx)                       // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectMechanic[idx] = mechanic;
    }

    uint32_t getEffectImplicitTargetA(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectImplicitTargetA[idx];
    }

    void setEffectImplicitTargetA(uint32_t targetA, uint8_t idx)                // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectImplicitTargetA[idx] = targetA;
    }

    uint32_t getEffectImplicitTargetB(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectImplicitTargetB[idx];
    }

    void setEffectImplicitTargetB(uint32_t targetB, uint8_t idx)                // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectImplicitTargetB[idx] = targetB;
    }

    uint32_t getEffectRadiusIndex(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectRadiusIndex[idx];
    }

    void setEffectRadiusIndex(uint32_t radiusIndex, uint8_t idx)                // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectRadiusIndex[idx] = radiusIndex;
    }

    uint32_t getEffectApplyAuraName(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectApplyAuraName[idx];
    }

    void setEffectApplyAuraName(uint32_t auraName, uint8_t idx)                 // used in HackFixes.cpp / SpellCustomizations.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectApplyAuraName[idx] = auraName;
    }

    uint32_t getEffectAmplitude(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectAmplitude[idx];
    }

    void setEffectAmplitude(uint32_t amplitude, uint8_t idx)                    // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectAmplitude[idx] = amplitude;
    }

    float getEffectMultipleValue(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectMultipleValue[idx];
    }

    void setEffectMultipleValue(float multiply, uint8_t idx)                   // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectMultipleValue[idx] = multiply;
    }

    uint32_t getEffectChainTarget(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectChainTarget[idx];
    }

    void setEffectChainTarget(uint32_t chainTarget, uint8_t idx)                // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectChainTarget[idx] = chainTarget;
    }

    uint32_t getEffectItemType(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectItemType[idx];
    }

    void setEffectItemType(uint32_t itemEntryId, uint8_t idx)                   // used in SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectItemType[idx] = itemEntryId;
    }

    int32_t getEffectMiscValue(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectMiscValue[idx];
    }

    void setEffectMiscValue(int32_t misc, uint8_t idx)                          // used in HackFixes.cpp / SpellCustomizations.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectMiscValue[idx] = misc;
    }

    int32_t getEffectMiscValueB(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectMiscValueB[idx];
    }

    void setEffectMiscValueB(int32_t miscB, uint8_t idx)                        // used in SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectMiscValueB[idx] = miscB;
    }

    uint32_t getEffectTriggerSpell(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectTriggerSpell[idx];
    }

    void setEffectTriggerSpell(uint32_t spell, uint8_t idx)                     // used in SpellCustomizations.cpp / ObjectMgr.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectTriggerSpell[idx] = spell;
    }

    float getEffectPointsPerComboPoint(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectPointsPerComboPoint[idx];
    }

    void setEffectPointsPerComboPoint(float effectPoints, uint8_t idx)          // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        EffectPointsPerComboPoint[idx] = effectPoints;
    }

    uint32_t getSpellVisual() const { return SpellVisual; }
    void setSpellVisual(uint32_t value) { SpellVisual = value; }                 // used in SpellCustomizations.cpp

    uint32_t getField114() const { return field114; }
    void setField114(uint32_t value) { field114 = value; }                       // used in SpellCustomizations.cpp

    uint32_t getSpellIconID() const { return spellIconID; }
    void setSpellIconID(uint32_t value) { spellIconID = value; }                 // used in SpellCustomizations.cpp

    uint32_t getActiveIconID() const { return activeIconID; }
    void setActiveIconID(uint32_t value) { activeIconID = value; }               // used in SpellCustomizations.cpp

    std::string getName() const { return Name; }
    void setName(std::string value) { Name = value; }               // used in SpellCustomizations.cpp

    std::string getRank() const { return Rank; }
    void setRank(std::string value) { Rank = value; }               // used in SpellCustomizations.cpp

    std::string getDescription() const { return Description; }
    void setDescription(std::string value) { Description = value; }               // used in SpellCustomizations.cpp

    std::string getBuffDescription() const { return BuffDescription; }
    void setBuffDescription(std::string value) { BuffDescription = value; }               // used in SpellCustomizations.cpp

    uint32_t getManaCostPercentage() const { return ManaCostPercentage; }
    void setManaCostPercentage(uint32_t value) { ManaCostPercentage = value; }        // used in SpellCustomizations.cpp

    uint32_t getStartRecoveryCategory() const { return StartRecoveryCategory; }
    void setStartRecoveryCategory(uint32_t value) { StartRecoveryCategory = value; }        // used in SpellCustomizations.cpp

    uint32_t getStartRecoveryTime() const { return StartRecoveryTime; }
    void setStartRecoveryTime(uint32_t value) { StartRecoveryTime = value; }        // used in SpellCustomizations.cpp

    uint32_t getMaxTargetLevel() const { return MaxTargetLevel; }
    void setMaxTargetLevel(uint32_t value) { MaxTargetLevel = value; }        // used in SpellCustomizations.cpp

    uint32_t getSpellFamilyName() const { return SpellFamilyName; }
    void setSpellFamilyName(uint32_t value) { SpellFamilyName = value; }        // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getSpellGroupType(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return SpellGroupType[idx];
    }

    uint32_t* getSpellGroupType()
    {
        return SpellGroupType;
    }

    void setSpellGroupType(uint32_t value, uint8_t idx)                             // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        SpellGroupType[idx] = value;
    }

    uint32_t getMaxTargets() const { return MaxTargets; }
    void setMaxTargets(uint32_t value) { MaxTargets = value; }        // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getSpell_Dmg_Type() const { return Spell_Dmg_Type; }
    void setSpell_Dmg_Type(uint32_t value) { Spell_Dmg_Type = value; }        // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getPreventionType() const { return PreventionType; }
    void setPreventionType(uint32_t value) { PreventionType = value; }        // used in SpellCustomizations.cpp

    float getDmg_multiplier(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return dmg_multiplier[idx];
    }

    void setDmg_multiplier(float dmgMultiplier, uint8_t idx)                       // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        dmg_multiplier[idx] = dmgMultiplier;
    }

    uint32_t getTotemCategory(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEM_CATEGORIES);
        return TotemCategory[idx];
    }

    void setTotemCategory(uint32_t category, uint8_t idx)                           // used in SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_TOTEM_CATEGORIES);
        TotemCategory[idx] = category;
    }

    int32_t getRequiresAreaId() const { return RequiresAreaId; }
    void setRequiresAreaId(int32_t value) { RequiresAreaId = value; }   // used in SpellCustomizations.cpp

    uint32_t getSchool() const { return School; }
    void setSchool(uint32_t value) { School = value; }                  // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getRuneCostID() const { return RuneCostID; }
    void setRuneCostID(uint32_t value) { RuneCostID = value; }          // used in pellCustomizations.cpp

    uint32_t getSpellDifficultyID() const { return SpellDifficultyID; }
    void setSpellDifficultyID(uint32_t value) { SpellDifficultyID = value; }          // used in pellCustomizations.cpp

#if VERSION_STRING != Cata
    uint32_t getModalNextSpell() const { return modalNextSpell; }           // not used!
    void setModalNextSpell(uint32_t value) { modalNextSpell = value; }    // used in SpellCustomizations.cpp

    uint32_t getRequiredItemFlags() const { return RequiredItemFlags; }
    void setRequiredItemFlags(uint32_t value) { RequiredItemFlags = value; }    // used in SpellCustomizations.cpp

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

    void setEffectSpellClassMask(uint32_t spellClass, uint8_t idx1, uint8_t idx2)           // used in HackFixes.cpp / SpellCustomizations.cpp
    {
        ARCEMU_ASSERT(idx1 < MAX_SPELL_EFFECTS && idx2 < MAX_SPELL_EFFECTS);
        EffectSpellClassMask[idx1][idx2] = spellClass;
    }

    uint32_t getSpellPriority() const { return spellPriority; }
    void setSpellPriority(uint32_t value) { spellPriority = value; }               // used in SpellCustomizations.cpp

    int32_t getStanceBarOrder() const { return StanceBarOrder; }
    void setStanceBarOrder(int32_t value) { StanceBarOrder = value; }        // used in HackFixes.cpp / SpellCustomizations.cpp

    uint32_t getMinFactionID() const { return MinFactionID; }
    void setMinFactionID(uint32_t value) { MinFactionID = value; }        // used in SpellCustomizations.cpp

    uint32_t getMinReputation() const { return MinReputation; }
    void setMinReputation(uint32_t value) { MinReputation = value; }        // used in SpellCustomizations.cpp

    uint32_t getRequiredAuraVision() const { return RequiredAuraVision; }
    void setRequiredAuraVision(uint32_t value) { RequiredAuraVision = value; }        // used in SpellCustomizations.cpp


    //////////////////////////////////////////////////////////////////////////////////////////
    //custom values
    uint32_t getCustom_proc_interval() const { return custom_proc_interval; }
    uint32_t getCustom_BGR_one_buff_on_target() const { return custom_BGR_one_buff_on_target; }
    uint32_t getCustom_c_is_flags() const { return custom_c_is_flags; }
    uint32_t getCustom_RankNumber() const { return custom_RankNumber; }
    uint32_t getCustom_ThreatForSpell() const { return custom_ThreatForSpell; }
    float getCustom_ThreatForSpellCoef() const { return custom_ThreatForSpellCoef; }

    uint32_t getCustom_spell_coef_flags() const { return custom_spell_coef_flags; }
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
    bool getCustom_is_melee_spell() const { return custom_is_melee_spell; }
    bool getCustom_is_ranged_spell() const { return custom_is_ranged_spell; }
    uint32_t getCustom_SchoolMask() const { return custom_SchoolMask; }
    uint32_t getCustomFlags() const { return CustomFlags; }

    uint32_t getEffectCustomFlag(uint8_t idx) const
    {
        ARCEMU_ASSERT(idx < MAX_SPELL_EFFECTS);
        return EffectCustomFlag[idx];
    }

    bool CheckLocation(uint32_t map_id, uint32_t zone_id, uint32_t area_id, Player* player = NULL);
        
private:

    //////////////////////////////////////////////////////////////////////////////////////////
    // from dbc files
    uint32_t Id;
    uint32_t Category;
    uint32_t DispelType;
    uint32_t MechanicsType;
    uint32_t Attributes;
    uint32_t AttributesEx;
    uint32_t AttributesExB;
    uint32_t AttributesExC;
    uint32_t AttributesExD;
    uint32_t AttributesExE;
    uint32_t AttributesExF;
    uint32_t AttributesExG;
    uint32_t RequiredShapeShift;          // (12-13 Stances[2])
    uint32_t ShapeshiftExclude;           // (14-15 StancesExcluded[2])
    uint32_t Targets;                     // not used!
    uint32_t TargetCreatureType;
    uint32_t RequiresSpellFocus;
    uint32_t FacingCasterFlags;
    uint32_t CasterAuraState;
    uint32_t TargetAuraState;
    uint32_t CasterAuraStateNot;
    uint32_t TargetAuraStateNot;
    uint32_t casterAuraSpell;
    uint32_t targetAuraSpell;
    uint32_t casterAuraSpellNot;
    uint32_t targetAuraSpellNot;
    uint32_t CastingTimeIndex;
    uint32_t RecoveryTime;
    uint32_t CategoryRecoveryTime;
    uint32_t InterruptFlags;
    uint32_t AuraInterruptFlags;
    uint32_t ChannelInterruptFlags;
    uint32_t procFlags;
    uint32_t procChance;
    uint32_t procCharges;
    uint32_t maxLevel;
    uint32_t baseLevel;
    uint32_t spellLevel;
    uint32_t DurationIndex;
    int32_t powerType;
    uint32_t manaCost;
    uint32_t manaCostPerlevel;        // not used!
    uint32_t manaPerSecond;           // not used!
    uint32_t manaPerSecondPerLevel;   // not used!
    uint32_t rangeIndex;
    float speed;
    uint32_t modalNextSpell;          // not used!
    uint32_t maxstack;
    uint32_t Totem[MAX_SPELL_TOTEMS];
    uint32_t Reagent[MAX_SPELL_REAGENTS];
    uint32_t ReagentCount[MAX_SPELL_REAGENTS];
    int32_t EquippedItemClass;
    int32_t EquippedItemSubClass;
    uint32_t RequiredItemFlags;
    uint32_t Effect[MAX_SPELL_EFFECTS];
    int32_t EffectDieSides[MAX_SPELL_EFFECTS];
    float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];
    int32_t EffectBasePoints[MAX_SPELL_EFFECTS];
    int32_t EffectMechanic[MAX_SPELL_EFFECTS];
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
    uint32_t EffectSpellClassMask[3][3];
    uint32_t SpellVisual;
    uint32_t field114;                                          // (131-132 SpellVisual[2])
    uint32_t spellIconID;
    uint32_t activeIconID;
    uint32_t spellPriority;
    std::string Name;
    std::string Rank;
    std::string Description;
    std::string BuffDescription;
    uint32_t ManaCostPercentage;
    uint32_t StartRecoveryCategory;
    uint32_t StartRecoveryTime;
    uint32_t MaxTargetLevel;
    uint32_t SpellFamilyName;
    uint32_t SpellGroupType[MAX_SPELL_EFFECTS];
    uint32_t MaxTargets;
    uint32_t Spell_Dmg_Type;
    uint32_t PreventionType;
    int32_t StanceBarOrder;
    float dmg_multiplier[MAX_SPELL_EFFECTS];
    uint32_t MinFactionID;          // not used!
    uint32_t MinReputation;         // not used!
    uint32_t RequiredAuraVision;    // not used!
    uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];     // not used!
    int32_t RequiresAreaId;
    uint32_t School;
    uint32_t RuneCostID;
    //uint32_t SpellMissileID;
    //uint32_t SpellDescriptionVariable;
    uint32_t SpellDifficultyID;

public:

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

        // from MySQL table spell_coef_flags - 16499 spells
        uint32_t custom_spell_coef_flags;

        // set in HackFixes.cpp for all spells
        float custom_base_range_or_radius_sqr;

        // set in HackFixes.cpp - 1 spell (26029)
        float cone_width;

        // set in HackFixes.cpp for all spells
        float casttime_coef;

        // set in HackFixes.cpp for most spells
        float fixed_dddhcoef;

        // set in HackFixes.cpp for most spells
        float fixed_hotdotcoef;

        // from MySQL table spell_coef_override - 151 spells
        float Dspell_coef_override;

        // from MySQL table spell_coef_override - 151 spells
        float OTspell_coef_override;

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

        // SpellCustomizations::SetCustomFlags - 1 spell (781)
        uint32_t CustomFlags;

        // from MySQL table spell_effects_override - 374 spells
        uint32_t EffectCustomFlag[MAX_SPELL_EFFECTS];
#else

        //////////////////////////////////////////////////////////////////////////////////////////
        // Applied values from DBC
        uint32_t Id;
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
        uint32_t CastingTimeIndex;
        uint32_t DurationIndex;
        int32_t powerType;            // uint32_t  error: case value evaluates to -2, which cannot be narrowed to type 'uint32' (aka 'unsigned int') [-Wc++11-narrowing]
        uint32_t rangeIndex;
        float speed;
        uint32_t SpellVisual;
        uint32_t field114;
        uint32_t spellIconID;
        uint32_t activeIconID;
        std::string Name;
        std::string Rank;
        std::string Description;
        std::string BuffDescription;
        uint32_t School;
        uint32_t RuneCostID;
        //uint32_t SpellMissileID;
        //uint32_t spellDescriptionVariableID;
        uint32_t SpellDifficultyID;

        //dbc links
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

        // data from SpellScaling.dbc
        // data from SpellAuraOptions.dbc
        uint32_t maxstack;
        uint32_t procChance;
        uint32_t procCharges;
        uint32_t procFlags;

        // data from SpellAuraRestrictions.dbc
        uint32_t CasterAuraState;
        uint32_t TargetAuraState;
        uint32_t CasterAuraStateNot;
        uint32_t TargetAuraStateNot;
        uint32_t casterAuraSpell;
        uint32_t targetAuraSpell;
        uint32_t casterAuraSpellNot;
        uint32_t targetAuraSpellNot;

        // data from SpellCastingRequirements.dbc
        uint32_t FacingCasterFlags;
        int32_t RequiresAreaId;
        uint32_t RequiresSpellFocus;

        // data from SpellCategories.dbc
        uint32_t Category;
        uint32_t DispelType;
        uint32_t Spell_Dmg_Type;
        uint32_t MechanicsType;
        uint32_t PreventionType;
        uint32_t StartRecoveryCategory;

        // data from SpellClassOptions.dbc
        uint32_t SpellGroupType[3];
        uint32_t SpellFamilyName;

        // data from SpellCooldowns.dbc
        uint32_t CategoryRecoveryTime;
        uint32_t RecoveryTime;
        uint32_t StartRecoveryTime;

        // data from SpellEquippedItems.dbc
        int32_t EquippedItemClass;
        int32_t EquippedItemInventoryTypeMask;
        int32_t EquippedItemSubClass;

        // data from SpellInterrupts.dbc
        uint32_t AuraInterruptFlags;
        uint32_t ChannelInterruptFlags;
        uint32_t InterruptFlags;

        // data from SpellLevels.dbc
        uint32_t baseLevel;
        uint32_t maxLevel;
        uint32_t spellLevel;

        // data from SpellPower.dbc
        uint32_t manaCost;
        uint32_t manaCostPerlevel;
        uint32_t ManaCostPercentage;
        uint32_t manaPerSecond;
        uint32_t manaPerSecondPerLevel;

        // data from SpellReagents.dbc
        uint32_t Reagent[MAX_SPELL_REAGENTS];
        uint32_t ReagentCount[MAX_SPELL_REAGENTS];

        // data from SpellShapeshift.dbc
        uint32_t RequiredShapeShift;
        uint32_t ShapeshiftExclude;

        // data from SpellTargetRestrictions.dbc
        uint32_t MaxTargets;
        uint32_t MaxTargetLevel;
        uint32_t TargetCreatureType;
        uint32_t Targets;

        // data from SpellTotems.dbc
        uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];
        uint32_t Totem[MAX_SPELL_TOTEMS];

        // data from SpellEffect.dbc
        uint32_t Effect[MAX_SPELL_EFFECTS];
        float EffectMultipleValue[MAX_SPELL_EFFECTS];
        uint32_t EffectApplyAuraName[MAX_SPELL_EFFECTS];
        uint32_t EffectAmplitude[MAX_SPELL_EFFECTS];
        int32_t EffectBasePoints[MAX_SPELL_EFFECTS];
        float EffectBonusMultiplier[MAX_SPELL_EFFECTS];
        float dmg_multiplier[MAX_SPELL_EFFECTS];
        uint32_t EffectChainTarget[MAX_SPELL_EFFECTS];
        int32_t EffectDieSides[MAX_SPELL_EFFECTS];
        uint32_t EffectItemType[MAX_SPELL_EFFECTS];
        uint32_t EffectMechanic[MAX_SPELL_EFFECTS];
        int32_t EffectMiscValue[MAX_SPELL_EFFECTS];
        int32_t EffectMiscValueB[MAX_SPELL_EFFECTS];
        float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS];
        uint32_t EffectRadiusIndex[MAX_SPELL_EFFECTS];
        uint32_t EffectRadiusMaxIndex[MAX_SPELL_EFFECTS];
        float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];
        uint32_t EffectSpellClassMask[MAX_SPELL_EFFECTS];
        uint32_t EffectTriggerSpell[MAX_SPELL_EFFECTS];
        uint32_t EffectImplicitTargetA[MAX_SPELL_EFFECTS];
        uint32_t EffectImplicitTargetB[MAX_SPELL_EFFECTS];
        uint32_t EffectSpellId[MAX_SPELL_EFFECTS];
        uint32_t EffectIndex[MAX_SPELL_EFFECTS];

        //////////////////////////////////////////////////////////////////////////////////////////
        // custom values
        uint32_t custom_proc_interval;
        uint32_t custom_BGR_one_buff_on_target;
        uint32_t custom_c_is_flags;
        uint32_t custom_RankNumber;
        uint32_t custom_NameHash;
        uint32_t custom_ThreatForSpell;
        float custom_ThreatForSpellCoef;
        uint32_t custom_spell_coef_flags;
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
        bool custom_is_melee_spell;
        bool custom_is_ranged_spell;
        bool CheckLocation(uint32_t map_id, uint32_t zone_id, uint32_t area_id, Player* player = NULL);
        uint32_t custom_SchoolMask;
        uint32_t CustomFlags;
        uint32_t EffectCustomFlag[MAX_SPELL_EFFECTS];
#endif
        void* (*SpellFactoryFunc);
        void* (*AuraFactoryFunc);
};
