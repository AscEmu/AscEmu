/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#ifndef SPELLAURAS_H
#define SPELLAURAS_H

#include "SpellMgr.h"
#include "Spell.h"
#include "SpellEffects.h"

/// 4-bit flag
enum AURA_FLAGS
{
    AFLAG_EMPTY         = 0x00,
    AFLAG_EFFECT_1      = 0x01,
    AFLAG_EFFECT_2      = 0x02,
    AFLAG_EFFECT_3      = 0x04,
    AFLAG_NOT_CASTER    = 0x08,
    AFLAG_SET           = 0x09,
    AFLAG_CANCELLABLE   = 0x10,
    AFLAG_DURATION      = 0x20,
    AFLAG_HIDE          = 0x40, // Seems to hide the aura and tell client the aura was removed
    AFLAG_NEGATIVE      = 0x80
};

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
    FLAG_PERIODIC_DAMAGE            = 2,
    FLAG_PERIODIC_TRIGGER_SPELL     = 4,
    FLAG_PERIODIC_HEAL              = 8,
    FLAG_PERIODIC_LEECH             = 16,
    FLAG_PERIODIC_ENERGIZE          = 32
};

struct Modifier
{
    //uint32 m_actamt;       // actual amt, for percent and stuff
    uint32 m_type;           // What does it modify? (str,int,hp)
    int32 m_amount;          // By how much does it mod?
    int32_t m_miscValue;       // Misc Value
    uint8_t m_effectIndex;

    ///needed for per level effect
    int32 realamount;
    //need this to store % values or they cannot be reverted correctly (i think :D)
    signed int fixed_amount[SCHOOL_COUNT];
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

class SERVER_DECL Aura : public EventableObject
{
    public:

        Aura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL);
        ~Aura();

        void Remove();
        void AddMod(uint32_t t, int32_t a, int32_t miscValue, uint8_t effectIndex);

        inline SpellInfo* GetSpellInfo() const { return m_spellInfo; }
        inline uint32 GetSpellId() const { return m_spellInfo->getId(); }
        inline bool IsPassive() { if (!m_spellInfo) return false; return (m_spellInfo->IsPassive() && !m_areaAura); }
#ifdef AE_TBC
    // MIT
    void addAuraVisual();
    // MIT End
#endif

        void ResetDuration();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Refreshes the aura, resets the duration
        /// \param none     \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void Refresh();

        inline int32 GetDuration() const { return m_duration; }
        void SetDuration(int32 duration)
        {
            m_duration = duration;
            ResetDuration();
        }

        inline uint16 GetAuraSlot() const { return m_auraSlot; }
        void SetAuraSlot(uint16 slot) { m_auraSlot = slot; }

        inline bool IsPositive() { return m_positive > 0; }
        void SetNegative(signed char value = 1) { m_positive -= value; }
        void SetPositive(signed char value = 1) { m_positive += value; }

        Object* GetCaster();
        inline uint64 GetCasterGUID() { return m_casterGuid; }
        Unit* GetUnitCaster();
        Player* GetPlayerCaster();
        inline Unit* GetTarget() { return m_target; }

        void ApplyModifiers(bool apply);
        void UpdateModifiers();

        //////////////////////////////////////////////////////////////////////////////////////////
        // Area Auras
        //////////////////////////////////////////////////////////////////////////////////////////

        void EventUpdateAreaAura(float r);

        void EventUpdateGroupAA(float r);
        void EventUpdateRaidAA(float r);
        void EventUpdatePetAA(float r);
        void EventUpdateFriendAA(float r);
        void EventUpdateEnemyAA(float r);
        void EventUpdateOwnerAA(float r);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Removes the Area Aura from all targets and clears the target set.
        /// \param none    \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void ClearAATargets();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if the Aura is an area Aura.
        /// \param none    \return true if it is false otherwise.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool IsAreaAura();

        //////////////////////////////////////////////////////////////////////////////////////////

        bool DotCanCrit();

        //! GetTimeLeft() milliseconds
        uint32 GetTimeLeft()
        {
            if (m_duration == -1)
                return (uint32)-1;
            int32 n = int32((UNIXTIME - time_t(expirytime)) * 1000);
            if (n >= m_duration)
                return 0;
            else
                return (m_duration - n);
        }

        bool HasModType(uint32 type)
        {
            for (uint8 x = 0; x < m_modcount; ++x)
                if (m_modList[x].m_type == type)
                    return true;
            return false;
        }
        // Aura Handlers
        void SpellAuraNULL(bool apply);
        void SpellAuraBindSight(bool apply);
        void SpellAuraModPossess(bool apply);
        void SpellAuraPeriodicDamage(bool apply);
        void SpellAuraDummy(bool apply);
        void SpellAuraModConfuse(bool apply);
        void SpellAuraModCharm(bool apply);
        void SpellAuraModFear(bool apply);
        void SpellAuraPeriodicHeal(bool apply);
        void SpellAuraModAttackSpeed(bool apply);
        void SpellAuraModThreatGenerated(bool apply);
        void SpellAuraModTaunt(bool apply);
        void SpellAuraModStun(bool apply);
        void SpellAuraModDamageDone(bool apply);
        void SpellAuraModDamageTaken(bool apply);
        void SpellAuraDamageShield(bool apply);
        void SpellAuraModStealth(bool apply);
        void SpellAuraModDetect(bool apply);
        void SpellAuraModInvisibility(bool apply);
        void SpellAuraModInvisibilityDetection(bool apply);
        void SpellAuraModTotalHealthRegenPct(bool apply);
        void SpellAuraModTotalManaRegenPct(bool apply);
        void SpellAuraModResistance(bool apply);
        void SpellAuraPeriodicTriggerSpell(bool apply);
        void SpellAuraPeriodicEnergize(bool apply);
        void SpellAuraModPacify(bool apply);
        void SpellAuraModRoot(bool apply);
        void SpellAuraModSilence(bool apply);
        void SpellAuraReflectSpells(bool apply);
        void SpellAuraModStat(bool apply);
        void SpellAuraModSkill(bool apply);
        void SpellAuraModIncreaseSpeed(bool apply);
        void SpellAuraModDecreaseSpeed(bool apply);
        void SpellAuraModIncreaseHealth(bool apply);
        void SpellAuraModIncreaseEnergy(bool apply);
        void SpellAuraModShapeshift(bool apply);
        void SpellAuraModEffectImmunity(bool apply);
        void SpellAuraModStateImmunity(bool apply);
        void SpellAuraModSchoolImmunity(bool apply);
        void SpellAuraModDmgImmunity(bool apply);
        void SpellAuraModDispelImmunity(bool apply);
        void SpellAuraProcTriggerSpell(bool apply);
        void SpellAuraProcTriggerDamage(bool apply);
        void SpellAuraTrackCreatures(bool apply);
        void SpellAuraTrackResources(bool apply);
        void SpellAuraModParryPerc(bool apply);
        void SpellAuraModDodgePerc(bool apply);
        void SpellAuraModBlockPerc(bool apply);
        void SpellAuraModCritPerc(bool apply);
        void SpellAuraPeriodicLeech(bool apply);
        void SpellAuraModHitChance(bool apply);
        void SpellAuraModSpellHitChance(bool apply);
        void SpellAuraTransform(bool apply);
        void SpellAuraModSpellCritChance(bool apply);
        void SpellAuraIncreaseSwimSpeed(bool apply);
        void SpellAuraModCratureDmgDone(bool apply);
        void SpellAuraPacifySilence(bool apply);
        void SpellAuraModScale(bool apply);
        void SpellAuraPeriodicHealthFunnel(bool apply);
        void SpellAuraPeriodicManaLeech(bool apply);
        void SpellAuraModCastingSpeed(bool apply);
        void SpellAuraFeignDeath(bool apply);
        void SpellAuraModDisarm(bool apply);
        void SpellAuraModStalked(bool apply);
        virtual void SpellAuraSchoolAbsorb(bool apply);
        void SpellAuraModSpellCritChanceSchool(bool apply);
        void SpellAuraModPowerCost(bool apply);
        void SpellAuraModPowerCostSchool(bool apply);
        void SpellAuraReflectSpellsSchool(bool apply);
        void SpellAuraModLanguage(bool apply);
        void SpellAuraAddFarSight(bool apply);
        void SpellAuraMechanicImmunity(bool apply);
        void SpellAuraMounted(bool apply);
        void SpellAuraModDamagePercDone(bool apply);
        void SpellAuraModPercStat(bool apply);
        void SpellAuraSplitDamage(bool apply);
        void SpellAuraWaterBreathing(bool apply);
        void SpellAuraModBaseResistance(bool apply);
        void SpellAuraModRegen(bool apply);
        void SpellAuraModPowerRegen(bool apply);
        void SpellAuraChannelDeathItem(bool apply);
        void SpellAuraModDamagePercTaken(bool apply);
        void SpellAuraModRegenPercent(bool apply);
        void SpellAuraPeriodicDamagePercent(bool apply);
        void SpellAuraModResistChance(bool apply);
        void SpellAuraModDetectRange(bool apply);
        void SpellAuraPreventsFleeing(bool apply);
        void SpellAuraModUnattackable(bool apply);
        void SpellAuraInterruptRegen(bool apply);
        void SpellAuraGhost(bool apply);
        void SpellAuraMagnet(bool apply);
        void SpellAuraManaShield(bool apply);
        void SpellAuraSkillTalent(bool apply);
        void SpellAuraModAttackPower(bool apply);
        void SpellAuraVisible(bool apply);
        void SpellAuraModResistancePCT(bool apply);
        void SpellAuraModCreatureAttackPower(bool apply);
        void SpellAuraModTotalThreat(bool apply);
        void SpellAuraWaterWalk(bool apply);
        void SpellAuraFeatherFall(bool apply);
        void SpellAuraHover(bool apply);
        void SpellAuraAddFlatModifier(bool apply);
        void SpellAuraAddPctMod(bool apply);
        void SpellAuraAddClassTargetTrigger(bool apply);
        void SpellAuraModPowerRegPerc(bool apply);
        void SpellAuraOverrideClassScripts(bool apply);
        void SpellAuraModRangedDamageTaken(bool apply);
        void SpellAuraModHealing(bool apply);
        void SpellAuraIgnoreRegenInterrupt(bool apply);
        void SpellAuraModMechanicResistance(bool apply);
        void SpellAuraModHealingPCT(bool apply);
        void SpellAuraUntrackable(bool apply);
        void SpellAuraEmphaty(bool apply);
        void SpellAuraModOffhandDamagePCT(bool apply);
        void SpellAuraModPenetration(bool apply);
        void SpellAuraModRangedAttackPower(bool apply);
        void SpellAuraModMeleeDamageTaken(bool apply);
        void SpellAuraModMeleeDamageTakenPct(bool apply);
        void SpellAuraRAPAttackerBonus(bool apply);
        void SpellAuraModIncreaseSpeedAlways(bool apply);
        void SpellAuraModIncreaseMountedSpeed(bool apply);
        void SpellAuraModCreatureRangedAttackPower(bool apply);
        void SpellAuraModIncreaseEnergyPerc(bool apply);
        void SpellAuraModIncreaseHealthPerc(bool apply);
        void SpellAuraModManaRegInterrupt(bool apply);
        void SpellAuraModHealingDone(bool apply);
        void SpellAuraModHealingDonePct(bool apply);
        void SpellAuraModTotalStatPerc(bool apply);
        void SpellAuraModHaste(bool apply);
        void SpellAuraForceReaction(bool apply);
        void SpellAuraModRangedHaste(bool apply);
        void SpellAuraModRangedAmmoHaste(bool apply);
        void SpellAuraModBaseResistancePerc(bool apply);
        void SpellAuraModResistanceExclusive(bool apply);
        void SpellAuraSafeFall(bool apply);
        void SpellAuraRetainComboPoints(bool apply);
        void SpellAuraResistPushback(bool apply);
        void SpellAuraModShieldBlockPCT(bool apply);
        void SpellAuraTrackStealthed(bool apply);
        void SpellAuraModDetectedRange(bool apply);
        void SpellAuraSplitDamageFlat(bool apply);
        void SpellAuraModStealthLevel(bool apply);
        void SpellAuraModUnderwaterBreathing(bool apply);
        void SpellAuraModReputationAdjust(bool apply);
        void SpellAuraNoPVPCredit(bool apply);
        void SpellAuraModHealthRegInCombat(bool apply);
        void SpellAuraPowerBurn(bool apply);
        void SpellAuraModCritDmgPhysical(bool apply);
        void SpellAuraModPAttackPower(bool apply);
        void SpellAuraModRangedAttackPowerPct(bool apply);
        void SpellAuraAPAttackerBonus(bool apply);
        void SpellAuraIncreaseDamageTypePCT(bool apply);
        void SpellAuraIncreaseCricticalTypePCT(bool apply);
        void SpellAuraIncreasePartySpeed(bool apply);
        void SpellAuraIncreaseSpellDamageByAttribute(bool apply);
        void SpellAuraIncreaseHealingByAttribute(bool apply);
        void SpellAuraIncreaseArmorByPctInt(bool apply);
        void SpellAuraReduceAttackerMHitChance(bool apply);
        void SpellAuraReduceAttackerRHitChance(bool apply);
        void SpellAuraReduceAttackerSHitChance(bool apply);
        void SpellAuraReduceEnemyMCritChance(bool apply);
        void SpellAuraReduceEnemyRCritChance(bool apply);
        void SpellAuraLimitSpeed(bool apply);
        void SpellAuraIncreaseTimeBetweenAttacksPCT(bool apply);
        void SpellAuraIncreaseAllWeaponSkill(bool apply);
        void SpellAuraModAttackerCritChance(bool apply);
        void SpellAuraIncreaseHitRate(bool apply);
        void SpellAuraReduceCritMeleeAttackDmg(bool apply);
        void SpellAuraReduceCritRangedAttackDmg(bool apply);
        void SpellAuraEnableFlight(bool apply);
        void SpellAuraEnableFlightWithUnmountedSpeed(bool apply);
        void SpellAuraIncreaseRageFromDamageDealtPCT(bool apply);
        void SpellAuraIncreaseFlightSpeed(bool apply);
        void SpellAuraIncreaseMovementAndMountedSpeed(bool apply);
        void SpellAuraIncreaseRating(bool apply);
        void SpellAuraRegenManaStatPCT(bool apply);
        void SpellAuraSpellHealingStatPCT(bool apply);
        void SpellAuraModStealthDetection(bool apply);
        void SpellAuraReduceAOEDamageTaken(bool apply);
        void SpellAuraIncreaseMaxHealth(bool apply);
        void SpellAuraSpiritOfRedemption(bool apply);
        void SpellAuraIncreaseAttackerSpellCrit(bool apply);
        void SpellAuraIncreaseRepGainPct(bool apply);
        void SpellAuraIncreaseRAPbyStatPct(bool apply);
        void SpellAuraModBlockValue(bool apply);
        void SpellAuraAllowFlight(bool apply);
        void SpellAuraFinishingMovesCannotBeDodged(bool apply);
        void SpellAuraExpertise(bool apply);
        void SpellAuraForceMoveForward(bool apply);
        void SpellAuraComprehendLang(bool apply);
        void SpellAuraPeriodicTriggerDummy(bool apply);
        void SpellAuraModPossessPet(bool apply);
        void SpellAuraModHealingByAP(bool apply);
        void SpellAuraModSpellDamageByAP(bool apply);
        void SpellAuraMeleeHaste(bool apply);
        void SpellAuraReduceEffectDuration(bool apply);
        void HandleAuraControlVehicle(bool apply);
        void EventPeriodicDrink(uint32 amount);
        void SpellAuraMirrorImage(bool apply);
        void SpellAuraModCombatResultChance(bool apply);
        void SpellAuraAddHealth(bool apply);
        void SpellAuraRemoveReagentCost(bool apply);
        void SpellAuraPeriodicTriggerSpellWithValue(bool apply);
        void SpellAuraModMechanicDmgTakenPct(bool apply);
        void SpellAuraBlockMultipleDamage(bool apply);
        void SpellAuraIgnoreTargetAuraState(bool apply);
        void SpellAuraAllowOnlyAbility(bool apply);
        void SpellAuraIncreaseAPbyStatPct(bool apply);
        void SpellAuraModSpellDamageDOTPct(bool apply);
        void SpellAuraConsumeNoAmmo(bool apply);
        void SpellAuraIgnoreShapeshift(bool apply);
        void SpellAuraPhase(bool apply);
        void SpellAuraMirrorImage2(bool apply);
        void SpellAuraModIgnoreArmorPct(bool apply);
        void SpellAuraModBaseHealth(bool apply);
        void SpellAuraModAttackPowerOfArmor(bool apply);
        void SpellAuraDeflectSpells(bool apply);
        void SpellAuraCallStabledPet(bool apply);
        void SpellAuraConvertRune(bool apply);
        void UpdateAuraModDecreaseSpeed();

        void SendModifierLog(int32** m, int32 v, uint32* mask, uint8 type, bool pct = false);
        void SendDummyModifierLog(std::map<SpellInfo*, uint32> * m, SpellInfo* spellInfo, uint32 i, bool apply, bool pct = false);

        // Events
        void EventPeriodicDamage(uint32);
        void EventPeriodicDamagePercent(uint32);
        void EventPeriodicHeal(uint32);
        void EventPeriodicTriggerSpell(SpellInfo* spellInfo, bool overridevalues, int32 overridevalue);
        void EventPeriodicTrigger(uint32 amount, uint32 type);
        void EventPeriodicEnergize(uint32, uint32);
        void EventPeriodicEnergizeVariable(uint32, uint32);
        void EventPeriodicHeal1(uint32);
        void EventPeriodicLeech(uint32);
        void EventPeriodicBurn(uint32, uint32);
        void EventPeriodicHealthFunnel(uint32);
        void EventPeriodicManaLeech(uint32);
        void EventPeriodicHealPct(float);
        void EventPeriodicManaPct(float);
        void EventPeriodicRegenManaStatPct(uint32 perc, uint32 stat);
        void EventPeriodicTriggerDummy();

        void RelocateEvents();
        int32 event_GetInstanceID();
        bool WasCastInDuel() { return m_castInDuel; }

        // This stuff can be cached in spellproto.
        bool IsCombatStateAffecting();

        inline bool TargetWasImuneToMods()
        {
            return (m_modcount && (((m_flags & MOD_0_RESISTED) + (m_flags & MOD_1_RESISTED) + (m_flags & MOD_2_RESISTED)) == m_modcount));
        }

        int32 GetModAmount(uint32 i) { if (i < 3) return m_modList[i].m_amount; return 0; }
        int32 GetModAmountByMod() { return mod->m_amount; };
        uint32 GetAuraFlags() { return m_flags; }
        void AssignModifiers(Aura* aura);

        virtual bool IsAbsorb() { return false; }

        SpellInfo* m_spellInfo;
        AreaAuraList targets; // This is only used for AA
        uint64 m_casterGuid;
        uint16 m_auraSlot;
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
        Unit* m_target;
        Player* p_target;
        uint32 expirytime;
        int32 m_duration;       // In Milliseconds
        //	bool m_positive;
        signed char m_positive;
        uint32	m_dynamicValue;
        uint32	m_flags;
        uint32 m_modcount;
        Modifier m_modList[3];
        Modifier* mod;

        void SendInterrupted(uint8 result, Object* m_caster);
        void SendChannelUpdate(uint32 time, Object* m_caster);
        void SendTickImmune(Unit* target, Unit* caster);

    public:
        uint32_t getExpiryTime() const { return expirytime; }
        bool m_temporary;	    // Skip saving
        bool m_deleted;
        int16 m_interrupted;
        bool m_ignoreunapply;   // \\\"special\\\" case, for unapply

        inline bool IsInterrupted() { return (m_interrupted >= 0); }
};

class AbsorbAura : public Aura
{
    public:

        AbsorbAura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr) :
            Aura(proto, duration, caster, target, temporary, i_caster), m_total_amount(0), m_amount(0), m_pct_damage(0) {}

        static Aura* Create(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
        {
            return new AbsorbAura(proto, duration, caster, target, temporary, i_caster);
        }

        virtual uint32 AbsorbDamage(uint32 School, uint32* dmg);

        void SpellAuraSchoolAbsorb(bool apply);

        bool IsAbsorb() { return true; }

    protected:

        uint32 GetSchoolMask()
        {
            for (uint8 x = 0; x < 3; ++x)
                if (GetSpellInfo()->getEffect(x) == SPELL_EFFECT_APPLY_AURA && GetSpellInfo()->getEffectApplyAuraName(x) == SPELL_AURA_SCHOOL_ABSORB)
                    return m_modList[x].m_miscValue;
            return 0;
        }

        virtual int32 CalcAbsorbAmount() { return mod->m_amount; }
        virtual int32 CalcPctDamage() { return 100; }

        // Total amount to be absorbed
        int32 m_total_amount;

        // Amount left to be absorbed
        int32 m_amount;

        // Pct of damage to absorb
        int32 m_pct_damage;
};

typedef void(Aura::*pSpellAura)(bool apply);

#endif // _SPELLAURAS_H
