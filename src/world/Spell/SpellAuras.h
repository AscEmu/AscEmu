/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>
#include <set>

#include "Definitions/AuraEffects.hpp"
#include "Definitions/AuraRemoveMode.hpp"
#include "Objects/Item.hpp"
#include "Objects/Object.hpp"
#include "Server/EventableObject.h"
#include "Objects/Units/Unit.hpp"

enum AURA_INTERNAL_USAGE_FLAGS
{
    //if all 3 mods are resisted then we can send client as a fully resisted spell.
    //don't change the value of these !
    MOD_0_RESISTED      = 1,
    MOD_1_RESISTED      = 2,
    MOD_2_RESISTED      = 4
};

enum AuraTickFlags
{
    FLAG_PERIODIC_NONE              = 0,
    FLAG_PERIODIC_DAMAGE            = 2,
    FLAG_PERIODIC_TRIGGER_SPELL     = 4,
    FLAG_PERIODIC_HEAL              = 8,
    FLAG_PERIODIC_LEECH             = 16,
    FLAG_PERIODIC_ENERGIZE          = 32
};

struct ProcTriggerSpellOnSpell
{
    uint32 origId;
    uint32 spellId;
    uint64 caster;
    uint32 procChance;
    uint32 procFlags;
    uint32 RemainingCharges;
    uint32 LastTrigger;
    void* owner;                //mark the owner of this proc to know which one to delete
};

struct DamageSplitTarget
{
    uint64 m_target;            // we store them
    uint32 m_spellId;
    float m_pctDamageSplit;     // % of taken damage to transfer (i.e. Soul Link)
    uint32 m_flatDamageSplit;   // flat damage to transfer (i.e. Blessing of Sacrifice)
    uint8 damage_type;          // bitwise 0-127 thingy
    void* creator;
};

typedef std::set< uint64 > AreaAuraList;

// APGL End
// MIT Start

/// 4-bit flag
enum AuraUpdateFlags : uint8_t
{
    AFLAG_EMPTY                 = 0x00,
    AFLAG_EFFECT_1              = 0x01,
    AFLAG_EFFECT_2              = 0x02,
    AFLAG_EFFECT_3              = 0x04,
    AFLAG_IS_CASTER             = 0x08,
    AFLAG_SET                   = 0x09,
    AFLAG_CANCELLABLE           = 0x10,
    AFLAG_DURATION              = 0x20,
#if VERSION_STRING < Cata
    AFLAG_HIDE                  = 0x40, // Seems to hide the aura and tell client the aura was removed
#else
    AFLAG_SEND_EFFECT_AMOUNT    = 0x40, // used with AFLAG_EFFECT_0/1/2
#endif
    AFLAG_NEGATIVE              = 0x80,

    AFLAG_MASK_ALL              = 0xFF
};

struct SERVER_DECL AuraEffectModifier
{
public:
    void setAuraEffectType(AuraEffect type);
    AuraEffect getAuraEffectType() const;

    void setEffectDamage(int32_t value);
    void setEffectDamage(float_t value);
    int32_t getEffectDamage() const;
    float_t getEffectFloatDamage() const;

    void setEffectBaseDamage(int32_t baseValue);
    int32_t getEffectBaseDamage() const;

    void setEffectFixedDamage(int32_t fixedValue);
    int32_t getEffectFixedDamage() const;

    void setEffectMiscValue(int32_t miscValue);
    int32_t getEffectMiscValue() const;

    void setEffectAmplitude(int32_t amplitude);
    int32_t getEffectAmplitude() const;

    void setEffectDamageFraction(float_t fraction);
    float_t getEffectDamageFraction() const;

    void setEffectPercentModifier(float_t pctMod);
    float_t getEffectPercentModifier() const;

    void setEffectDamageStatic(bool);
    bool isEffectDamageStatic() const;

    void setEffectIndex(uint8_t effIndex);
    uint8_t getEffectIndex() const;

    void setEffectActive(bool set);
    bool isActive() const;

    void applyEffect(bool apply, bool skipScriptCheck = false);

    void setAura(Aura* aur);
    Aura* getAura() const;

private:
    AuraEffect mAuraEffect = SPELL_AURA_NONE;   // Effect type
    int32_t mDamage = 0;                        // Effect calculated amount
    float_t mRealDamage = 0.0f;                 // Effect exact calculated damage
    int32_t mBaseDamage = 0;                    // Effect base amount
    int32_t mFixedDamage = 0;                   // For example used with auras that increase your spell power by % of some stat
    int32_t miscValue = 0;                      // Misc Value
    int32_t mAmplitude = 0;                     // Effect amplitude
    float_t mDamageFraction = 0.0f;             // Leftover damage from previous tick which will be added to next tick
    float_t mEffectPctModifier = 1.0f;          // Effect percent modifier
    bool mEffectDamageStatic = false;           // If effect damage is set to static, effect will not gain spell power bonuses
    bool mActive = false;                       // Is effect active
    uint8_t effIndex = 0;

    Aura* mAura = nullptr;
};

class SERVER_DECL Aura : public EventableObject
{
    friend class AbsorbAura;

    public:
        AuraEffectModifier const* getAuraEffect(uint8_t effIndex) const;
        AuraEffectModifier* getModifiableAuraEffect(uint8_t effIndex);
        bool hasAuraEffect(AuraEffect auraEffect) const;
        void addAuraEffect(AuraEffect auraEffect, int32_t damage, int32_t miscValue, float_t effectPctModifier, bool isStaticDamage, uint8_t effIndex, bool reapplying = false);
        void addAuraEffect(AuraEffectModifier const* auraEffect, bool reapplying = false);
        void removeAuraEffect(uint8_t effIndex, bool reapplying = false);
        void removeAllAuraEffects();
        // Returns how many active aura effects the aura has
        uint8_t getAppliedEffectCount() const;

        uint16_t getAuraSlot() const;
        void setAuraSlot(uint16_t slot);

        int32_t getEffectDamage(uint8_t effIndex) const;
        int32_t getEffectDamageByEffect(AuraEffect auraEffect) const;

        void removeAura(AuraRemoveMode mode = AURA_REMOVE_BY_SERVER);
        bool isDeleted() const;

        bool canPeriodicEffectCrit();

        void applyModifiers(bool apply, AuraEffect applyOnlyFor = SPELL_AURA_NONE);
        void updateModifiers();

        int32_t getTimeLeft() const;
        void setTimeLeft(int32_t dur);
        int32_t getMaxDuration() const;
        void setMaxDuration(int32_t dur);
        int32_t getOriginalDuration() const;
        void setOriginalDuration(int32_t dur);
        // Overrides original duration
        void setNewMaxDuration(int32_t dur, bool refreshDuration = true);

        // Does not return 0 to avoid division by zero
        // Returns 1 with permanent auras or with invalid effindex
        uint16_t getPeriodicTickCountForEffect(uint8_t effIndex) const;

        // Refresh resets aura's duration and charges to max and recalculates modifiers
        // Mods are saved only in special situations
        void refreshOrModifyStack(bool saveMods = false, int16_t modifyStackAmount = 0);

        uint8_t getStackCount() const;
        uint16_t getCharges() const;
        void setCharges(uint16_t count, bool sendUpdatePacket = true);
        void removeCharge();

        uint8_t getAuraFlags() const;

        void setNegative(bool negative);
        bool isNegative() const;

        // Set on aura apply
        float_t getCritChance() const;
        int32_t getSpellPowerBonus() const;
        int32_t getHealPowerBonus() const;
        uint32_t getAttackPowerBonus() const;

        void addUsedSpellModifier(AuraEffectModifier const* aurEff);
        void removeUsedSpellModifier(AuraEffectModifier const* aurEff);
        void takeUsedSpellModifiers();

        Unit* getOwner() const;
        Player* getPlayerOwner() const;
        Object* getCaster() const;
        uint64_t getCasterGuid() const;

        void update(unsigned long diff, bool skipDurationCheck = false);

        virtual bool isAbsorbAura() const;

        SpellInfo const* getSpellInfo() const;
        uint32_t getSpellId() const;

        // Aura effect handlers

        // Used with effects that are not implemented yet
        void spellAuraEffectNotImplemented(AuraEffectModifier* aurEff, bool apply);
        // Used with effects that are not used or are handled elsewhere
        void spellAuraEffectNotUsed(AuraEffectModifier* aurEff, bool apply);

        void spellAuraEffectBindSight(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPossess(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicDamage(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectDummy(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModConfuse(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModCharm(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModFear(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicHeal(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModAttackSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModThreatGenerated(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModTaunt(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModStun(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDamageDone(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDamageTaken(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectDamageShield(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModStealth(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModStealthDetection(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModInvisibility(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModInvisibilityDetection(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicHealPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicPowerPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModResistance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicTriggerSpell(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicEnergize(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPacify(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModRoot(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModSilence(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReflectSpells(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModStat(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModSkill(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModIncreaseSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDecreaseSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModIncreaseHealth(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModIncreaseEnergy(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModShapeshift(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModEffectImmunity(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModStateImmunity(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModSchoolImmunity(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDmgImmunity(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDispelImmunity(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectProcTriggerSpell(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectProcTriggerDamage(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectTrackCreatures(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectTrackResources(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModParryPerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDodgePerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModBlockPerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModCritPerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicLeech(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModHitChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModSpellHitChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectTransform(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModSpellCritChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseSwimSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModCratureDmgDone(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPacifySilence(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModScale(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicHealthFunnel(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicManaLeech(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModCastingSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectFeignDeath(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDisarm(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModStalked(AuraEffectModifier* aurEff, bool apply);
        virtual void spellAuraEffectSchoolAbsorb(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModSpellCritChanceSchool(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPowerCost(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPowerCostSchool(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReflectSpellsSchool(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModLanguage(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectAddFarSight(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectMechanicImmunity(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectMounted(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDamagePercDone(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPercStat(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectSplitDamage(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectWaterBreathing(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModBaseResistance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPowerRegen(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectChannelDeathItem(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDamagePercTaken(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModRegenPercent(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicDamagePercent(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModResistChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDetectRange(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPreventsFleeing(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModUnattackable(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectInterruptRegen(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectGhost(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectMagnet(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectManaShield(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectSkillTalent(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModAttackPower(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectVisible(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModResistancePCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModCreatureAttackPower(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModTotalThreat(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectWaterWalk(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectFeatherFall(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectHover(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectAddModifier(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectAddClassTargetTrigger(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPowerRegPerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectOverrideClassScripts(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModRangedDamageTaken(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModHealing(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIgnoreRegenInterrupt(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModMechanicResistance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModHealingPCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectUntrackable(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectEmphaty(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModOffhandDamagePCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPenetration(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModRangedAttackPower(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModMeleeDamageTaken(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModMeleeDamageTakenPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectRAPAttackerBonus(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModIncreaseSpeedAlways(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModIncreaseMountedSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModCreatureRangedAttackPower(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModIncreaseEnergyPerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModIncreaseHealthPerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModManaRegInterrupt(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModHealingDone(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModHealingDonePct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModTotalStatPerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModHaste(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectForceReaction(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModRangedHaste(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModRangedAmmoHaste(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModBaseResistancePerc(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModResistanceExclusive(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectSafeFall(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectRetainComboPoints(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectResistPushback(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModShieldBlockPCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectTrackStealthed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModDetectedRange(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectSplitDamageFlat(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModStealthLevel(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModUnderwaterBreathing(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModReputationAdjust(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectNoPVPCredit(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModHealthRegInCombat(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicPowerBurn(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModCritDmgPhysical(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPAttackPower(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModRangedAttackPowerPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectAPAttackerBonus(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseDamageTypePCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseCricticalTypePCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreasePartySpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseSpellDamageByAttribute(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseHealingByAttribute(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseArmorByPctInt(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceAttackerMHitChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceAttackerRHitChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceAttackerSHitChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceEnemyMCritChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceEnemyRCritChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectLimitSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseTimeBetweenAttacksPCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseAllWeaponSkill(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModAttackerCritChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseHitRate(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceCritMeleeAttackDmg(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceCritRangedAttackDmg(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectEnableFlight(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectEnableFlightWithUnmountedSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseRageFromDamageDealtPCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseFlightSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseMovementAndMountedSpeed(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseRating(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectRegenManaStatPCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectSpellHealingStatPCT(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectDetectStealth(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceAOEDamageTaken(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseMaxHealth(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectSpiritOfRedemption(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseAttackerSpellCrit(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseRepGainPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseRAPbyStatPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModBlockValue(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectAllowFlight(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectFinishingMovesCannotBeDodged(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectExpertise(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectForceMoveForward(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectComprehendLang(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicTriggerDummy(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModPossessPet(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModHealingByAP(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModSpellDamageByAP(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectMeleeHaste(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectReduceEffectDuration(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectMirrorImage(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModCombatResultChance(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectAddHealth(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectRemoveReagentCost(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPeriodicTriggerSpellWithValue(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModMechanicDmgTakenPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectBlockMultipleDamage(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectAllowOnlyAbility(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIncreaseAPbyStatPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModSpellDamageDOTPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectConsumeNoAmmo(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectIgnoreShapeshift(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectPhase(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectMirrorImage2(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModIgnoreArmorPct(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModBaseHealth(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectModAttackPowerOfArmor(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectDeflectSpells(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectCallStabledPet(AuraEffectModifier* aurEff, bool apply);
        void spellAuraEffectConvertRune(AuraEffectModifier* aurEff, bool apply);

    private:
        AuraEffectModifier m_auraEffects[MAX_SPELL_EFFECTS];
        uint8_t m_auraEffectCount = 0;
        bool m_checkAuraEffects = false;
        // Do not update aura while updating modifiers
        bool m_updatingModifiers = false;

        uint16_t m_auraSlot = 0xFFFF;

        // Time left
        int32_t m_duration = 0;
        // Maximum duration
        int32_t m_maxDuration = 0;
        // Original duration before haste
        int32_t m_originalDuration = 0;
        //\ todo: verify maximum possible stack count but it is likely 255 -Appled
        uint8_t m_stackCount = 1;
        uint16_t m_charges = 0;
        uint16_t m_originalCharges = 0;

        // Following values are calculated only on aura apply, not on every tick
        void _calculateCritChance();
        void _calculateSpellPowerBonus();
        void _calculateAttackPowerBonus();
        void _calculateSpellHaste();
        void _calculateEffectAmplitude(uint8_t effIndex);
        bool _canHasteAffectDuration();
        float_t m_critChance = 0.0f;
        int32_t m_spellPowerBonus = 0;
        int32_t m_healPowerBonus = 0;
        uint32_t m_attackPowerBonus = 0;
        float_t m_spellHaste = 0.0f;

        std::map<AuraEffectModifier const*, bool> m_usedModifiers;

        bool mPositive = true;

        // Aura could have effects with different amplitude
        int32_t m_periodicTimer[MAX_SPELL_EFFECTS];
        void periodicTick(AuraEffectModifier* aurEff);

        bool m_isGarbage = false;

        uint64_t m_casterGuid = 0;
        Unit* m_target = nullptr;
        Player* p_target = nullptr;

        SpellInfo const* m_spellInfo = nullptr;

    // MIT End
    // APGL Start
    public:

        Aura(SpellInfo const* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL);
        ~Aura();

        inline bool IsPassive() const { if (!m_spellInfo) return false; return (m_spellInfo->isPassive() && !m_areaAura); }

        Unit* GetUnitCaster();
        Player* GetPlayerCaster();

        //////////////////////////////////////////////////////////////////////////////////////////
        // Area Auras
        //////////////////////////////////////////////////////////////////////////////////////////

        void EventUpdateAreaAura(uint8_t effIndex, float r);

        void EventUpdateGroupAA(AuraEffectModifier* aurEff, float r);
        void EventUpdateRaidAA(AuraEffectModifier* aurEff, float r);
        void EventUpdatePetAA(AuraEffectModifier* aurEff, float r);
        void EventUpdateFriendAA(AuraEffectModifier* aurEff, float r);
        void EventUpdateEnemyAA(AuraEffectModifier* aurEff, float r);
        void EventUpdateOwnerAA(AuraEffectModifier* aurEff, float r);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Removes the Area Aura from all targets and clears the target set.
        /// \param none    \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void ClearAATargets();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if the Aura is an area Aura.
        /// \param none    \return true if it is false otherwise.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool IsAreaAura() const;

        //////////////////////////////////////////////////////////////////////////////////////////

        // Legacy Aura Handlers
        void SpellAuraBindSight(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPossess(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModConfuse(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModCharm(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModFear(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModAttackSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModThreatGenerated(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModTaunt(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDetaunt(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModStun(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDamageDone(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDamageTaken(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraDamageShield(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModStealth(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModStealthDetection(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModInvisibility(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModInvisibilityDetection(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModResistance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPacify(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModRoot(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModSilence(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReflectSpells(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModStat(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModSkill(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModIncreaseSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDecreaseSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModIncreaseHealth(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModIncreaseEnergy(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModEffectImmunity(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModStateImmunity(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModSchoolImmunity(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDmgImmunity(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDispelImmunity(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraProcTriggerSpell(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraProcTriggerDamage(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraTrackCreatures(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraTrackResources(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModParryPerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDodgePerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModBlockPerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModCritPerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModHitChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModSpellHitChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModSpellCritChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseSwimSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModCratureDmgDone(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraPacifySilence(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModScale(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModCastingSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraFeignDeath(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDisarm(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModStalked(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModSpellCritChanceSchool(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPowerCost(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPowerCostSchool(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReflectSpellsSchool(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModLanguage(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraAddFarSight(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraMechanicImmunity(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraMounted(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDamagePercDone(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPercStat(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraSplitDamage(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraWaterBreathing(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModBaseResistance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraChannelDeathItem(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDamagePercTaken(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModRegenPercent(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModResistChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDetectRange(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraPreventsFleeing(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModUnattackable(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraInterruptRegen(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraGhost(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraMagnet(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraManaShield(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraSkillTalent(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModAttackPower(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraVisible(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModResistancePCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModCreatureAttackPower(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModTotalThreat(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraWaterWalk(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraFeatherFall(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraHover(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraAddClassTargetTrigger(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPowerRegPerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraOverrideClassScripts(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModRangedDamageTaken(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModHealing(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIgnoreRegenInterrupt(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModMechanicResistance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModHealingPCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraUntrackable(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraEmphaty(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModOffhandDamagePCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPenetration(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModRangedAttackPower(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModMeleeDamageTaken(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModMeleeDamageTakenPct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraRAPAttackerBonus(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModIncreaseSpeedAlways(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModIncreaseMountedSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModCreatureRangedAttackPower(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModIncreaseEnergyPerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModIncreaseHealthPerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModManaRegInterrupt(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModHealingDone(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModHealingDonePct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModTotalStatPerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModHaste(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraForceReaction(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModRangedHaste(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModRangedAmmoHaste(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModBaseResistancePerc(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModResistanceExclusive(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraSafeFall(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraResistPushback(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModShieldBlockPCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraTrackStealthed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModDetectedRange(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraSplitDamageFlat(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModStealthLevel(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModUnderwaterBreathing(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModReputationAdjust(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraNoPVPCredit(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModHealthRegInCombat(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModCritDmgPhysical(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPAttackPower(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModRangedAttackPowerPct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraAPAttackerBonus(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseDamageTypePCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseCricticalTypePCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreasePartySpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseSpellDamageByAttribute(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseHealingByAttribute(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseArmorByPctInt(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceAttackerMHitChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceAttackerRHitChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceAttackerSHitChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceEnemyMCritChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceEnemyRCritChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraLimitSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseTimeBetweenAttacksPCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseAllWeaponSkill(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModAttackerCritChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseHitRate(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceCritMeleeAttackDmg(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceCritRangedAttackDmg(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraEnableFlight(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraEnableFlightWithUnmountedSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseRageFromDamageDealtPCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseFlightSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseMovementAndMountedSpeed(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseRating(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraRegenManaStatPCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraSpellHealingStatPCT(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceAOEDamageTaken(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseMaxHealth(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraSpiritOfRedemption(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseAttackerSpellCrit(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseRepGainPct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseRAPbyStatPct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModBlockValue(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraAllowFlight(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraFinishingMovesCannotBeDodged(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraExpertise(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraForceMoveForward(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraComprehendLang(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModPossessPet(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModHealingByAP(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModSpellDamageByAP(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraMeleeHaste(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraReduceEffectDuration(AuraEffectModifier* aurEff, bool apply);
        void HandleAuraControlVehicle(AuraEffectModifier* aurEff, bool apply);
        void EventPeriodicDrink(uint32 amount);
        void SpellAuraMirrorImage(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModCombatResultChance(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraAddHealth(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraRemoveReagentCost(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModMechanicDmgTakenPct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraBlockMultipleDamage(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraAllowOnlyAbility(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraIncreaseAPbyStatPct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModSpellDamageDOTPct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraConsumeNoAmmo(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraPhase(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraMirrorImage2(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModIgnoreArmorPct(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModBaseHealth(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraModAttackPowerOfArmor(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraDeflectSpells(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraCallStabledPet(AuraEffectModifier* aurEff, bool apply);
        void SpellAuraConvertRune(AuraEffectModifier* aurEff, bool apply);
        void UpdateAuraModDecreaseSpeed(AuraEffectModifier* aurEff);

        void SendDummyModifierLog(std::map<SpellInfo*, uint32> * m, SpellInfo* spellInfo, uint32 i, bool apply, bool pct = false);

        // Events
        void EventPeriodicHeal1(uint32);

        void RelocateEvents();
        int32 event_GetInstanceID();
        bool WasCastInDuel() { return m_castInDuel; }

        // This stuff can be cached in spellproto.
        bool IsCombatStateAffecting();

        uint32 GetAuraFlags() { return m_flags; }

        AreaAuraList targets; // This is only used for AA
        uint32 m_castedItemId;
        uint64 itemCasterGUID;
        bool m_areaAura; // Area aura stuff -> never passive.
        uint8 m_visualSlot;
        uint32 pSpellId; // This represents the triggering spell id
        bool m_castInDuel;

    private:

        uint32 GetCasterFaction() { return m_casterfaction; }
        void SetCasterFaction(uint32 faction) { m_casterfaction = faction; }

        inline bool IsInrange(float x1, float y1, float z1, Object* o, float square_r)
        {
            float t;
            float r;
            t = x1 - o->GetPositionX();
            r = t * t;
            t = y1 - o->GetPositionY();
            r += t * t;
            t = z1 - o->GetPositionZ();
            r += t * t;
            return (r <= square_r);
        }

    protected:

        uint32 m_casterfaction;
        uint32 m_dynamicValue;
        uint32 m_flags;

        void SendChannelUpdate(uint32 time, Object* m_caster);

    public:
        bool m_temporary;       // Skip saving
        bool m_deleted;
        int16 m_interrupted;
        bool m_ignoreunapply;   // \\\"special\\\" case, for unapply

        inline bool IsInterrupted() { return (m_interrupted >= 0); }
};

// APGL End
// MIT Start

class AbsorbAura : public Aura
{
    public:
        AbsorbAura(SpellInfo* spellInfo, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr);
        static Aura* Create(SpellInfo* spellInfo, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr);

        virtual uint32_t absorbDamage(SchoolMask schoolMask, uint32_t* dmg, bool checkOnly);
        uint32_t getRemainingAbsorbAmount() const;
        uint32_t getTotalAbsorbAmount() const;

        void spellAuraEffectSchoolAbsorb(AuraEffectModifier* aurEff, bool apply);

        bool isAbsorbAura() const override;

    protected:
        uint32_t calcAbsorbAmount(AuraEffectModifier* aurEff);

        uint32_t m_totalAbsorbValue = 0;
        // Remaining absorb value
        uint32_t m_absorbValue = 0;
        // How many percentages of the damage is absorbed
        uint8_t m_pctAbsorbValue = 100;
        SchoolMask m_absorbSchoolMask = SCHOOL_MASK_NONE;

        uint32_t m_absorbDamageBatch = 0;

        // Legacy script hooks
        virtual int32_t CalcAbsorbAmount(AuraEffectModifier* aurEff) { return aurEff->getEffectDamage(); }
        virtual uint8_t CalcPctDamage() { return 100; }
};

typedef void(Aura::*pSpellAura)(AuraEffectModifier* aurEff, bool apply);
