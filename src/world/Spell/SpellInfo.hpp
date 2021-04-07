/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Definitions/AuraEffects.h"
#include "Definitions/PowerType.h"
#include "SpellDefines.hpp"
#include "SpellScript.h"

#include "CommonTypes.hpp"
#include "WorldConf.h"
#include <string>
#include "Log.hpp"

class Item;
class Player;
class Unit;

class SERVER_DECL SpellInfo
{
public:
    SpellInfo();
    ~SpellInfo() = default;

    friend class SpellMgr;

    // helper functions
    bool hasEffect(uint32_t effect) const;
    bool hasEffectApplyAuraName(uint32_t auraType) const;
    bool hasCustomFlagForEffect(uint32_t effectIndex, uint32_t flag) const;

    bool isDamagingSpell() const;
    bool isHealingSpell() const;
    int firstBeneficialEffect() const;

    uint8_t getFirstSchoolFromSchoolMask() const;

    bool isDamagingEffect(uint8_t effIndex) const;
    bool isHealingEffect(uint8_t effIndex) const;
    bool hasDamagingEffect() const;
    bool hasHealingEffect() const;

    // Checks if spell (in most cases an aura) affects another spell, based on spell group mask
    // Note; this can cause false positives if aura has multiple effects
    // Use isAuraEffectAffectingSpell in that case to check individual effects
    bool isAffectingSpell(SpellInfo const* spellInfo) const;
    bool isEffectIndexAffectingSpell(uint8_t effIndex, SpellInfo const* spellInfo) const;
    bool isAuraEffectAffectingSpell(AuraEffect auraEffect, SpellInfo const* spellInfo) const;

    // Returns true if powertype is valid for current expansion
    bool hasValidPowerType() const;
    int32_t getBasePowerCost(Unit* caster) const;

    bool isNegativeAura() const;

    uint32_t getSpellDefaultDuration(Unit const* caster) const;

    bool hasTargetType(uint32_t type) const;
    uint32_t getRequiredTargetMaskForEffectTarget(uint32_t implicitTarget, uint8_t effectIndex) const;
    uint32_t getRequiredTargetMaskForEffect(uint8_t effectIndex) const;
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

    int32_t calculateEffectValue(uint8_t effIndex, Unit* unitCaster = nullptr, Item* itemCaster = nullptr, uint32_t forcedBasePoints = 0) const;

    bool doesEffectApplyAura(uint8_t effIndex) const;

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
    PowerType getPowerType() const { return powerType; }
    uint32_t getManaCost() const { return manaCost; }
    uint32_t getManaCostPerlevel() const { return manaCostPerlevel; } // not used!
    uint32_t getManaPerSecond() const { return manaPerSecond; } // not used!
    uint32_t getManaPerSecondPerLevel() const { return manaPerSecondPerLevel; } // not used!
    uint32_t getRangeIndex() const { return rangeIndex; }
    float getSpeed() const { return speed; }
    uint32_t getMaxstack() const { return MaxStackAmount; }

    uint32_t getTotem(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_TOTEMS)
        {
            LogError("Totem index id %u is invalid!", idx);
            return 0;
        }

        return Totem[idx];
    }

    int32_t getReagent(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_REAGENTS)
        {
            LogError("Reagent index id %u is invalid!", idx);
            return 0;
        }

        return Reagent[idx];
    }

    uint32_t getReagentCount(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_REAGENTS)
        {
            LogError("ReagentCount index id %u is invalid!", idx);
            return 0;
        }

        return ReagentCount[idx];
    }

    int32_t getEquippedItemClass() const { return EquippedItemClass; }
    int32_t getEquippedItemSubClass() const { return EquippedItemSubClass; }
    int32_t getEquippedItemInventoryTypeMask() const { return EquippedItemInventoryTypeMask; }

    uint32_t getEffect(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return Effect[idx];
    }

    int32_t getEffectDieSides(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectDieSides[idx];
    }

    float getEffectRealPointsPerLevel(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0.f;
        }

        return EffectRealPointsPerLevel[idx];
    }

    int32_t getEffectBasePoints(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectBasePoints[idx];
    }

    uint32_t getEffectMechanic(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectMechanic[idx];
    }

    uint32_t getEffectImplicitTargetA(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectImplicitTargetA[idx];
    }

    uint32_t getEffectImplicitTargetB(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectImplicitTargetB[idx];
    }

    uint32_t getEffectRadiusIndex(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectRadiusIndex[idx];
    }

    uint32_t getEffectApplyAuraName(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectApplyAuraName[idx];
    }

    uint32_t getEffectAmplitude(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectAmplitude[idx];
    }

    float getEffectMultipleValue(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectMultipleValue[idx];
    }

    uint32_t getEffectChainTarget(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectChainTarget[idx];
    }

    uint32_t getEffectItemType(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectItemType[idx];
    }

    int32_t getEffectMiscValue(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectMiscValue[idx];
    }

    int32_t getEffectMiscValueB(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectMiscValueB[idx];
    }

    uint32_t getEffectTriggerSpell(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectTriggerSpell[idx];
    }

    float getEffectPointsPerComboPoint(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectPointsPerComboPoint[idx];
    }

    uint32_t getEffectSpellClassMask(uint8_t idx1, uint8_t idx2) const
    {
        if (idx1 >= MAX_SPELL_EFFECTS || idx2 >= MAX_SPELL_EFFECTS)
        {
            LogError("Totem index id %u or effect index %u is invalid!", idx1, idx2);
            return 0;
        }

        return EffectSpellClassMask[idx1][idx2];
    }

    uint32_t* getEffectSpellClassMask(uint8_t idx1)
    {
        if (idx1 >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx1);
            return 0;
        }

        return EffectSpellClassMask[idx1];
    }

    uint32_t getSpellVisual(uint8_t visualIndex) const { return SpellVisual[visualIndex]; }
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
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return SpellFamilyFlags[idx];
    }

    const uint32_t* getSpellFamilyFlags() const { return SpellFamilyFlags; }

    uint32_t getMaxTargets() const { return MaxTargets; }
    uint32_t getDmgClass() const { return DmgClass; }
    uint32_t getPreventionType() const { return PreventionType; }

    float getEffectDamageMultiplier(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectDamageMultiplier[idx];
    }

#if VERSION_STRING > Classic
    uint32_t getTotemCategory(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_TOTEM_CATEGORIES)
        {
            LogError("TotemCategory index id %u is invalid!", idx);
            return 0;
        }

        return TotemCategory[idx];
    }
#endif

    int32_t getRequiresAreaId() const { return AreaGroupId; }
    uint32_t getSchoolMask() const { return SchoolMask; }
    uint32_t getRuneCostID() const { return RuneCostID; }

    float getEffectBonusMultiplier(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

        return EffectBonusMultiplier[idx];
    }

    float* getEffectBonusMultiplier()
    {
        return EffectBonusMultiplier;
    }

    uint32_t getSpellDifficultyID() const { return SpellDifficultyId; }

    //////////////////////////////////////////////////////////////////////////////////////////
    //custom values
    uint32_t getCustom_BGR_one_buff_on_target() const { return custom_BGR_one_buff_on_target; }
    uint32_t getCustom_c_is_flags() const { return custom_c_is_flags; }
    uint32_t getCustom_RankNumber() const { return custom_RankNumber; }
    uint32_t getCustom_ThreatForSpell() const { return custom_ThreatForSpell; }
    float getCustom_ThreatForSpellCoef() const { return custom_ThreatForSpellCoef; }

    float getCustom_base_range_or_radius_sqr() const { return custom_base_range_or_radius_sqr; }
    float getCone_width() const { return cone_width; }
    int getAi_target_type() const { return ai_target_type; }

    uint32_t getEffectCustomFlag(uint8_t idx) const
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return 0;
        }

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
    void setPowerType(PowerType value) { powerType = value; }
    void setManaCost(uint32_t value) { manaCost = value; }
    void setManaCostPerlevel(uint32_t value) { manaCostPerlevel = value; }
    void setManaPerSecond(uint32_t value) { manaPerSecond = value; }
    void setManaPerSecondPerLevel(uint32_t value) { manaPerSecondPerLevel = value; }
    void setRangeIndex(uint32_t value) { rangeIndex = value; }    // used in HackFixes.cpp
    void setSpeed(float value) { speed = value; }    // used in HackFixes.cpp
    void setMaxstack(uint32_t value) { MaxStackAmount = value; }    // used in HackFixes.cpp

    void setTotem(uint32_t totemId, uint8_t idx)                      // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_TOTEMS)
        {
            LogError("Totem index id %u is invalid!", idx);
            return;
        }

        Totem[idx] = totemId;
    }

    void setReagent(int32_t reagentId, uint8_t idx)                      // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_REAGENTS)
        {
            LogError("Spellreagents index id %u is invalid!", idx);
            return;
        }

        Reagent[idx] = reagentId;
    }

    void setReagentCount(uint32_t reagentId, uint8_t idx)                 // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_REAGENTS)
        {
            LogError("Reagentcount index id %u is invalid!", idx);
            return;
        }

        ReagentCount[idx] = reagentId;
    }

    void setEquippedItemClass(int32_t value) { EquippedItemClass = value; }    // used in HackFixes.cpp
    void setEquippedItemSubClass(int32_t value) { EquippedItemSubClass = value; }
    void setEquippedItemInventoryTypeMask(int32_t value) { EquippedItemInventoryTypeMask = value; }

    void setEffect(uint32_t effectId, uint8_t idx)                          // used in HackFixes.cpp / ObjectMgr.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        Effect[idx] = effectId;
    }

    void setEffectDieSides(int32_t effecSide, uint8_t idx)                 // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectDieSides[idx] = effecSide;
    }

    void setEffectRealPointsPerLevel(float pointsPerLevel, uint8_t idx)   // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectRealPointsPerLevel[idx] = pointsPerLevel;
    }

    void setEffectBasePoints(int32_t pointsPerLevel, uint8_t idx)               // used in HackFixes.cpp / ObjectMgr.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectBasePoints[idx] = pointsPerLevel;
    }

    void setEffectMechanic(uint32_t mechanic, uint8_t idx)                       // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectMechanic[idx] = mechanic;
    }

    void setEffectImplicitTargetA(uint32_t targetA, uint8_t idx)                // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectImplicitTargetA[idx] = targetA;
    }

    void setEffectImplicitTargetB(uint32_t targetB, uint8_t idx)                // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectImplicitTargetB[idx] = targetB;
    }

    void setEffectRadiusIndex(uint32_t radiusIndex, uint8_t idx)                // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectRadiusIndex[idx] = radiusIndex;
    }

    void setEffectApplyAuraName(uint32_t auraName, uint8_t idx)                 // used in HackFixes.cpp / ObjectMgr.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectApplyAuraName[idx] = auraName;
    }

    void setEffectAmplitude(uint32_t amplitude, uint8_t idx)                    // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectAmplitude[idx] = amplitude;
    }

    void setEffectMultipleValue(float multiply, uint8_t idx)                   // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectMultipleValue[idx] = multiply;
    }

    void setEffectChainTarget(uint32_t chainTarget, uint8_t idx)                // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectChainTarget[idx] = chainTarget;
    }

    void setEffectItemType(uint32_t itemEntryId, uint8_t idx)
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectItemType[idx] = itemEntryId;
    }

    void setEffectMiscValue(int32_t misc, uint8_t idx)                          // used in HackFixes.cpp / ObjectMgr.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectMiscValue[idx] = misc;
    }

    void setEffectMiscValueB(int32_t miscB, uint8_t idx)
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectMiscValueB[idx] = miscB;
    }

    void setEffectTriggerSpell(uint32_t spell, uint8_t idx)                     // used in ObjectMgr.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectTriggerSpell[idx] = spell;
    }

    void setEffectPointsPerComboPoint(float effectPoints, uint8_t idx)          // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectPointsPerComboPoint[idx] = effectPoints;
    }

    void setEffectSpellClassMask(uint32_t spellClass, uint8_t idx1, uint8_t idx2)           // used in HackFixes.cpp
    {
        if (idx1 >= MAX_SPELL_EFFECTS || idx2 >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id1 %u or id2 %u is invalid!", idx1, idx2);
            return;
        }

        EffectSpellClassMask[idx1][idx2] = spellClass;
    }

    void setSpellVisual(uint8_t visualIndex, uint32_t value) { SpellVisual[visualIndex] = value; }
    void setSpellIconID(uint32_t value) { spellIconID = value; }
    void setActiveIconID(uint32_t value) { activeIconID = value; }
    void setSpellPriority(uint32_t value) { spellPriority = value; }
    void setName(const std::string value) { Name = value; }
    void setRank(const std::string value) { Rank = value; }
    void setManaCostPercentage(uint32_t value) { ManaCostPercentage = value; }
    void setStartRecoveryCategory(uint32_t value) { StartRecoveryCategory = value; }
    void setStartRecoveryTime(uint32_t value) { StartRecoveryTime = value; }
    void setMaxTargetLevel(uint32_t value) { MaxTargetLevel = value; }
    void setSpellFamilyName(uint32_t value) { SpellFamilyName = value; }        // used in HackFixes.cpp

    void setSpellFamilyFlags(uint32_t value, uint8_t idx)                             // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        SpellFamilyFlags[idx] = value;
    }

    void setMaxTargets(uint32_t value) { MaxTargets = value; }        // used in HackFixes.cpp
    void setDmgClass(uint32_t value) { DmgClass = value; }        // used in HackFixes.cpp
    void setPreventionType(uint32_t value) { PreventionType = value; }

    void setEffectDamageMultiplier(float dmgMultiplier, uint8_t idx)                       // used in HackFixes.cpp
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectDamageMultiplier[idx] = dmgMultiplier;
    }

#if VERSION_STRING > Classic
    void setTotemCategory(uint32_t category, uint8_t idx)
    {
        if (idx >= MAX_SPELL_TOTEM_CATEGORIES)
        {
            LogError("TotemCategory index id %u is invalid!", idx);
            return;
        }

        TotemCategory[idx] = category;
    }
#endif

    void setRequiresAreaId(int32_t value) { AreaGroupId = value; }
    void setSchoolMask(uint32_t value) { SchoolMask = value; }                  // used in HackFixes.cpp
    void setRuneCostID(uint32_t value) { RuneCostID = value; }

    void setEffectBonusMultiplier(float value, uint8_t idx)
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectBonusMultiplier[idx] = value;
    }

    void setSpellDifficultyID(uint32_t value) { SpellDifficultyId = value; }

#if VERSION_STRING >= Cata
    void setEffectRadiusMaxIndex(uint32_t value, uint8_t idx)
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectRadiusMaxIndex[idx] = value;
    }

    void setEffectSpellId(uint32_t value, uint8_t idx)
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectSpellId[idx] = value;
    }

    void setEffectIndex(uint32_t value, uint8_t idx)
    {
        if (idx >= MAX_SPELL_EFFECTS)
        {
            LogError("Effect index id %u is invalid!", idx);
            return;
        }

        EffectIndex[idx] = value;
    }
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Applied values from DBC
    uint32_t Id = 0;
    // Data from SpellCategories.dbc (in Cataclysm)
    uint32_t Category = 0;
    uint32_t DispelType = 0;
    uint32_t MechanicsType = 0;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t Attributes = 0;
    uint32_t AttributesEx = 0;
    uint32_t AttributesExB = 0;
    uint32_t AttributesExC = 0;
    uint32_t AttributesExD = 0;
    uint32_t AttributesExE = 0;
    uint32_t AttributesExF = 0;
    uint32_t AttributesExG = 0;
    uint32_t AttributesExH = 0;
    uint32_t AttributesExI = 0;
    uint32_t AttributesExJ = 0;
    // Data from SpellShapeshift.dbc (in Cataclysm)
    uint32_t Shapeshifts = 0;
    uint32_t ShapeshiftsExcluded = 0;
    // Data from SpellTargetRestrictions.dbc (in Cataclysm)
    uint32_t Targets = 0;
    uint32_t TargetCreatureType = 0;
    // Data from SpellCastingRequirements.dbc (in Cataclysm)
    uint32_t RequiresSpellFocus = 0;
    uint32_t FacingCasterFlags = 0;
    // Data from SpellAuraRestrictions.dbc (in Cataclysm)
    uint32_t CasterAuraState = 0;
    uint32_t TargetAuraState = 0;
    uint32_t CasterAuraStateNot = 0;
    uint32_t TargetAuraStateNot = 0;
    uint32_t casterAuraSpell = 0;
    uint32_t targetAuraSpell = 0;
    uint32_t casterAuraSpellNot = 0;
    uint32_t targetAuraSpellNot = 0;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t CastingTimeIndex = 0;
    // Data from SpellCooldowns.dbc (in Cataclysm)
    uint32_t RecoveryTime = 0;
    uint32_t CategoryRecoveryTime = 0;
    // Data from SpellInterrupts.dbc (in Cataclysm)
    uint32_t InterruptFlags = 0;
    uint32_t AuraInterruptFlags = 0;
    uint32_t ChannelInterruptFlags = 0;
    // Data from SpellAuraOptions.dbc (in Cataclysm)
    uint32_t procFlags = 0;
    uint32_t procChance = 0;
    uint32_t procCharges = 0;
    // Data from SpellLevels.dbc (in Cataclysm)
    uint32_t maxLevel = 0;
    uint32_t baseLevel = 0;
    uint32_t spellLevel = 0;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t DurationIndex = 0;
    PowerType powerType = POWER_TYPE_MANA;
    // Data from SpellPower.dbc (in Cataclysm)
    uint32_t manaCost = 0;
    uint32_t manaCostPerlevel = 0;
    uint32_t manaPerSecond = 0;
    uint32_t manaPerSecondPerLevel = 0;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t rangeIndex = 0;
    float speed = 0.0f;
    // Data from SpellAuraOptions.dbc (in Cataclysm)
    uint32_t MaxStackAmount = 0;
    // Data from SpellTotems.dbc (in Cataclysm)
    uint32_t Totem[MAX_SPELL_TOTEMS];
    // Data from SpellReagents.dbc (in Cataclysm)
    int32_t Reagent[MAX_SPELL_REAGENTS];
    uint32_t ReagentCount[MAX_SPELL_REAGENTS];
    // Data from SpellEquippedItems.dbc (in Cataclysm)
    int32_t EquippedItemClass = -1;
    int32_t EquippedItemSubClass = 0;
    int32_t EquippedItemInventoryTypeMask = 0;
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
    uint32_t SpellVisual[2];
    uint32_t spellIconID = 0;
    uint32_t activeIconID = 0;
    uint32_t spellPriority = 0;
    std::string Name = "";
    std::string Rank = "";
    // Data from SpellPower.dbc (in Cataclysm)
    uint32_t ManaCostPercentage = 0;
    // Data from SpellCategories.dbc (in Cataclysm)
    uint32_t StartRecoveryCategory = 0;
    // Data from SpellCooldowns.dbc (in Cataclysm)
    uint32_t StartRecoveryTime = 0;
    // Data from SpellTargetRestrictions.dbc (in Cataclysm)
    uint32_t MaxTargetLevel = 0;
    // Data from SpellClassOptions.dbc (in Cataclysm)
    uint32_t SpellFamilyName = 0;
    uint32_t SpellFamilyFlags[MAX_SPELL_EFFECTS];
    // Data from SpellTargetRestrictions.dbc (in Cataclysm)
    uint32_t MaxTargets = 0;
    // Data from SpellCategories.dbc (in Cataclysm)
    uint32_t DmgClass = 0;
    uint32_t PreventionType = 0;
    // Data from SpellEffect.dbc (in Cataclysm)
    float EffectDamageMultiplier[MAX_SPELL_EFFECTS];
#if VERSION_STRING > Classic
    // Data from SpellTotems.dbc (in Cataclysm)
    uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];     // not used!
#endif
    // Data from SpellCastingRequirements.dbc (in Cataclysm)
    int32_t AreaGroupId = 0;
    // Data from Spell.dbc (in Cataclysm)
    uint32_t SchoolMask = 0;
    uint32_t RuneCostID = 0;
    // Data from SpellEffect.dbc (in Cataclysm)
    float EffectBonusMultiplier[MAX_SPELL_EFFECTS];
    // Data from SpellDifficulty.dbc (in Cataclysm)
    uint32_t SpellDifficultyId = 0;

    // Script links (Legacy)
    void* (*spellScriptLink) = nullptr;
    void* (*auraScriptLink) = nullptr;

public:
#if VERSION_STRING >= Cata
    // DBC links
    uint32_t SpellScalingId = 0;                              // SpellScaling.dbc
    uint32_t SpellAuraOptionsId = 0;                          // SpellAuraOptions.dbc
    uint32_t SpellAuraRestrictionsId = 0;                     // SpellAuraRestrictions.dbc
    uint32_t SpellCastingRequirementsId = 0;                  // SpellCastingRequirements.dbc
    uint32_t SpellCategoriesId = 0;                           // SpellCategories.dbc
    uint32_t SpellClassOptionsId = 0;                         // SpellClassOptions.dbc
    uint32_t SpellCooldownsId = 0;                            // SpellCooldowns.dbc
    uint32_t SpellEquippedItemsId = 0;                        // SpellEquippedItems.dbc
    uint32_t SpellInterruptsId = 0;                           // SpellInterrupts.dbc
    uint32_t SpellLevelsId = 0;                               // SpellLevels.dbc
    uint32_t SpellPowerId = 0;                                // SpellPower.dbc
    uint32_t SpellReagentsId = 0;                             // SpellReagents.dbc
    uint32_t SpellShapeshiftId = 0;                           // SpellShapeshift.dbc
    uint32_t SpellTargetRestrictionsId = 0;                   // SpellTargetRestrictions.dbc
    uint32_t SpellTotemsId = 0;                               // SpellTotems.dbc
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell coefficients

    // Direct damage or direct heal coefficient
    float_t spell_coeff_direct = -1.0f;

    // Damage or healing over-time coefficient (NOTE: This is per tick)
    float_t spell_coeff_overtime = -1.0f;

    // Attack power coefficient (only set in SQL table spell_coefficient_override)
    float_t spell_ap_coeff_direct = 0.0f;
    // This is per tick
    float_t spell_ap_coeff_overtime = 0.0f;

    //////////////////////////////////////////////////////////////////////////////////////////
    //custom values

    // from MySQL table spell_custom_assign - 1970 spells
    uint32_t custom_BGR_one_buff_on_target = 0;

    // from MySQL table spell_custom_assign - 353 spells
    // also flags added in SpellCustomizations::SetMissingCIsFlags
    uint32_t custom_c_is_flags = 0;

    // from MySQL table spell_ranks - 6546 spells
    uint32_t custom_RankNumber = 0;

    // set in HackFixes.cpp for all Dummy Trigger
    uint32_t custom_NameHash = 0;

    // from MySQL table ai_threattospellid - 144 spells
    int32_t custom_ThreatForSpell = 0;

    // from MySQL table ai_threattospellid - 118 spells
    float custom_ThreatForSpellCoef = 0.0f;

    // set in HackFixes.cpp for all spells
    float custom_base_range_or_radius_sqr = 0.0f;

    // set in HackFixes.cpp - 1 spell (26029)
    float cone_width = 0.0f;

    // set in HackFixes.cpp for all spells
    // check out SpellInfo::aiTargetType
    int ai_target_type = 0;

    // set in Hackfixes.cpp - 5 spells
    // from MySQL table spell_custom_assign - 6 spells
    bool custom_self_cast_only = false;

    // from MySQL table spell_effects_override - 374 spells
    uint32_t EffectCustomFlag[MAX_SPELL_EFFECTS];
};
