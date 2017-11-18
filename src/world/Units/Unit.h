/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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

// MIT Start
#include "Objects/Object.h"
#include "Spell/SpellDefines.hpp"

#include "UnitDefines.hpp"
#include "Management/LootMgr.h"
#include "Spell/SpellProc.h"
#include "Objects/Object.h"
#include "Units/Summons/SummonHandler.h"
#include "Movement/UnitMovementManager.hpp"
#include "Spell/Definitions/School.h"
#include "Storage/MySQLStructures.h"

class AIInterface;
class Aura;
class DynamicObject;
class GameObject;
class Group;
class Object;
class Pet;
class Spell;
class SpellProc;
class Vehicle;

struct FactionDBC;

enum UnitSpeedType
{
    TYPE_WALK           = 0,
    TYPE_RUN            = 1,
    TYPE_RUN_BACK       = 2,
    TYPE_SWIM           = 3,
    TYPE_SWIM_BACK      = 4,
    TYPE_TURN_RATE      = 5,
    TYPE_FLY            = 6,
    TYPE_FLY_BACK       = 7,
    TYPE_PITCH_RATE     = 8
};

// MIT End
// AGPL Start
//these refer to visibility ranges. We need to store each stack of the aura and not just visible count.
#define MAX_POSITIVE_VISUAL_AURAS_START 0
#define MAX_POSITIVE_VISUAL_AURAS_END 40

#define MAX_NEGATIVE_VISUAL_AURAS_START MAX_POSITIVE_VISUAL_AURAS_END       // 40 buff slots, 16 debuff slots.
#define MAX_NEGATIVE_VISUAL_AURAS_END (MAX_POSITIVE_VISUAL_AURAS_END + 16)  // 40 buff slots, 16 debuff slots.

//you hardly get to this but since i was testing i got to it :) : 20 items * 11 (enchants) + 61 talents
#define MAX_PASSIVE_AURAS_START 0                                                   // these are reserved for talents. No need to check them for removes ?
#define MAX_PASSIVE_AURAS_END (MAX_PASSIVE_AURAS_START + 140)                       // these are reserved for talents. No need to check them for removes ?

#define MAX_POSITIVE_AURAS_EXTEDED_START MAX_PASSIVE_AURAS_END                      //these are not talents.These are stacks from visible auras
#define MAX_POSITIVE_AURAS_EXTEDED_END (MAX_POSITIVE_AURAS_EXTEDED_START + 100)     //these are not talents.These are stacks from visible auras

#define MAX_NEGATIVE_AURAS_EXTEDED_START MAX_POSITIVE_AURAS_EXTEDED_END             //these are not talents.These are stacks from visible auras
#define MAX_NEGATIVE_AURAS_EXTEDED_END (MAX_NEGATIVE_AURAS_EXTEDED_START + 100)     //these are not talents.These are stacks from visible auras

#define MAX_REMOVABLE_AURAS_START (MAX_POSITIVE_AURAS_EXTEDED_START)        //do we need to handle talents at all ?
#define MAX_REMOVABLE_AURAS_END (MAX_NEGATIVE_AURAS_EXTEDED_END)            //do we need to handle talents at all ?

#define MAX_TOTAL_AURAS_START (MAX_PASSIVE_AURAS_START)
#define MAX_TOTAL_AURAS_END (MAX_REMOVABLE_AURAS_END)

bool SERVER_DECL Rand(float);

#define SPELL_GROUPS    96          // This is actually on 64 bits !
#define DIMINISHING_GROUP_COUNT 15

#define UNIT_TYPE_HUMANOID_BIT (1 << (UNIT_TYPE_HUMANOID-1)) // should get computed by precompiler ;)

typedef std::unordered_map<uint32, uint64> UniqueAuraTargetMap;

//////////////////////////////////////////////////////////////////////////////////////////
/// Checks for conditions specified in subclasses on Auras. When calling operator()
/// it tells if the conditions are met.
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL AuraCondition
{
public:

    virtual bool operator()(Aura* /*aura*/)
    {
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Performs the actions specified in subclasses on the Aura, when calling operator().
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL AuraAction
{
public:

    virtual void operator()(Aura* /*aura*/) {}
};

struct ReflectSpellSchool
{
    uint32 spellId;
    uint32 charges;
    int32 school;
    int32 chance;
    bool infront;
};

struct OnHitSpell
{
    uint32 spellid;
    uint32 mindmg;
    uint32 maxdmg;
};

struct AreaAura
{
    uint32 auraid;
    Unit* caster;
};

typedef struct
{
    SpellInfo* spell_info;
    uint32 charges;
} ExtraStrike;

struct AuraCheckResponse
{
    uint32 Error;
    uint32 Misc;
};

#define UNIT_SUMMON_SLOTS 6

typedef std::list<struct ProcTriggerSpellOnSpell> ProcTriggerSpellOnSpellList;

class Unit;
class SERVER_DECL CombatStatusHandler
{
    typedef std::set<uint64> AttackerMap;
    typedef std::set<uint32> HealedSet;      // Must Be Players!

    HealedSet m_healers;
    HealedSet m_healed;

    Unit* m_Unit;

    bool m_lastStatus;

    AttackerMap m_attackTargets;

    uint64 m_primaryAttackTarget;

    public:

        CombatStatusHandler() : m_Unit(nullptr), m_lastStatus(false), m_primaryAttackTarget(0) {}

        AttackerMap m_attackers;

        void AddAttackTarget(const uint64 & guid);                      // this means we clicked attack, not actually striked yet, so they shouldn't be in combat.
        void ClearPrimaryAttackTarget();                                // means we deselected the unit, stopped attacking it.

        void OnDamageDealt(Unit* pTarget);                              // this is what puts the other person in combat.
        void WeHealed(Unit* pHealTarget);                               // called when a player heals another player, regardless of combat state.

        void RemoveAttacker(Unit* pAttacker, const uint64 & guid);      // this means we stopped attacking them totally. could be because of deaggro, etc.
        void RemoveAttackTarget(Unit* pTarget);                         // means our DoT expired.

        void UpdateFlag();                                              // detects if we have changed combat state (in/out), and applies the flag.
        bool IsInCombat() const;                                        // checks if we are in combat or not.
        void OnRemoveFromWorld();                                       // called when we are removed from world, kills all references to us.

        void Vanished()
        {
            ClearAttackers();
            ClearHealers();
        }

        const uint64 & GetPrimaryAttackTarget() { return m_primaryAttackTarget; }
        void SetUnit(Unit* p) { m_Unit = p; }
        void TryToClearAttackTargets();                                 // for pvp timeout
        void AttackersForgetHate();                                     // used right now for Feign Death so attackers go home

    protected:

        bool InternalIsInCombat();                                      // called by UpdateFlag, do not call from anything else!
        bool IsAttacking(Unit* pTarget);                                // internal function used to determine if we are still attacking target x.
        void AddAttacker(const uint64 & guid);                          // internal function to add an attacker
        void RemoveHealed(Unit* pHealTarget);                           // usually called only by updateflag
        void ClearHealers();                                            // this is called on instance change.
        void ClearAttackers();                                          // means we vanished, or died.
        void ClearMyHealers();
};
// AGPL End

// MIT Start
class SERVER_DECL Unit : public Object
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement
private:

    int32_t m_rootCounter;

public:

    void setMoveWaterWalk();
    void setMoveLandWalk();
    void setMoveFeatherFall();
    void setMoveNormalFall();
    void setMoveHover(bool set_hover);
    void setMoveCanFly(bool set_fly);
    void setMoveRoot(bool set_root);
    bool isRooted() const;

    void setMoveSwim(bool set_swim);
    void setMoveDisableGravity(bool disable_gravity);
    void setMoveWalk(bool set_walk);

    // Speed
private:

    float m_currentSpeedWalk;
    float m_currentSpeedRun;
    float m_currentSpeedRunBack;
    float m_currentSpeedSwim;
    float m_currentSpeedSwimBack;
    float m_currentTurnRate;
    float m_currentSpeedFly;
    float m_currentSpeedFlyBack;
    float m_currentPitchRate;

    float m_basicSpeedWalk;
    float m_basicSpeedRun;
    float m_basicSpeedRunBack;
    float m_basicSpeedSwim;
    float m_basicSpeedSwimBack;
    float m_basicTurnRate;
    float m_basicSpeedFly;
    float m_basicSpeedFlyBack;
    float m_basicPitchRate;

public:

    float getSpeedForType(UnitSpeedType speed_type, bool get_basic = false);
    void setSpeedForType(UnitSpeedType speed_type, float speed, bool set_basic = false);
    void resetCurrentSpeed();

    void sendMoveSplinePaket(UnitSpeedType speed_type);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Internal States
private:

    uint32_t m_unitState;

public:

    void addUnitStateFlag(uint32_t state_flag) { m_unitState |= state_flag; };
    bool hasUnitStateFlag(uint32_t state_flag) { return (m_unitState & state_flag ? true : false); }
    void removeUnitStateFlag(uint32_t state_flag) { m_unitState &= ~state_flag; };
    uint32_t getUnitStateFlags() { return m_unitState; };


    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells
    void playSpellVisual(uint64_t guid, uint32_t spell_id);
    void applyDiminishingReturnTimer(uint32_t* duration, SpellInfo* spell);
    void removeDiminishingReturnTimer(SpellInfo* spell);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Aura
    Aura* getAuraWithId(uint32_t spell_id);
    Aura* getAuraWithId(uint32_t* auraId);
    Aura* getAuraWithIdForGuid(uint32_t* auraId, uint64 guid);

    Aura* getAuraWithIdForGuid(uint32_t spell_id, uint64_t target_guid);
    Aura* getAuraWithAuraEffect(uint32_t aura_effect);

    bool hasAurasWithId(uint32_t auraId);
    bool hasAurasWithId(uint32_t* auraId);

    uint32_t getAuraCountForId(uint32_t auraId);

    void removeAllAurasById(uint32_t auraId);
    void removeAllAurasById(uint32_t* auraId);
    void removeAllAurasByIdForGuid(uint32_t auraId, uint64_t guid);
    uint32_t removeAllAurasByIdReturnCount(uint32_t auraId);

    uint64_t getSingleTargetGuidForAura(uint32_t spellId);
    uint64_t getSingleTargetGuidForAura(uint32_t* spellIds, uint32_t* index);

    void setSingleTargetGuidForAura(uint32_t spellId, uint64_t guid);
    void removeSingleTargetGuidForAura(uint32_t spellId);


    // Do not alter anything below this line
    // -------------------------------------
private:
    // MIT End
    // AGPL Start
public:
    void CombatStatusHandler_UpdatePvPTimeout();
    void CombatStatusHandler_ResetPvPTimeout();

    virtual ~Unit();

    friend class AIInterface;
    friend class Aura;
    TransportData m_transportData;

    virtual bool Teleport(const LocationVector& vec, MapMgr* map) = 0;

    void Update(unsigned long time_passed);
    virtual void RemoveFromWorld(bool free_guid);
    virtual void OnPushToWorld();

    virtual void Deactivate(MapMgr* mgr);

    virtual bool IsPvPFlagged() = 0;
    virtual void SetPvPFlag() = 0;
    virtual void RemovePvPFlag() = 0;

    virtual bool IsFFAPvPFlagged() = 0;
    virtual void SetFFAPvPFlag() = 0;
    virtual void RemoveFFAPvPFlag() = 0;

    virtual bool IsSanctuaryFlagged() = 0;
    virtual void SetSanctuaryFlag() = 0;
    virtual void RemoveSanctuaryFlag() = 0;


    void setAttackTimer(int32 time, bool offhand);
    bool isAttackReady(bool offhand);

    void SetDualWield(bool enabled);

    bool  canReachWithAttack(Unit* pVictim);

    /// Stats
    uint32 getLevel() { return m_uint32Values[UNIT_FIELD_LEVEL]; };
    void setLevel(uint32 level);
    void modLevel(int32 mod) { ModUnsigned32Value(UNIT_FIELD_LEVEL, mod); };
    uint32 getClassMask() { return 1 << (getClass() - 1); }
    uint32 getRaceMask() { return 1 << (getRace() - 1); }
    uint8 getStandState() { return ((uint8)m_uint32Values[UNIT_FIELD_BYTES_1]); }

    //// Combat
    uint32 GetSpellDidHitResult(Unit* pVictim, uint32 weapon_damage_type, SpellInfo* ability);
    void Strike(Unit* pVictim, uint32 weapon_damage_type, SpellInfo* ability, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool force_crit = false);
    uint32 m_procCounter;
    uint32 HandleProc(uint32 flag, Unit* Victim, SpellInfo* CastingSpell, bool is_triggered = false, uint32 dmg = -1, uint32 abs = 0, uint32 weapon_damage_type = 0);
    void HandleProcDmgShield(uint32 flag, Unit* attacker);//almost the same as handleproc :P
    bool IsCriticalDamageForSpell(Object* victim, SpellInfo* spell);
    float GetCriticalDamageBonusForSpell(Object* victim, SpellInfo* spell, float amount);
    bool IsCriticalHealForSpell(Object* victim, SpellInfo* spell);
    float GetCriticalHealBonusForSpell(Object* victim, SpellInfo* spell, float amount);

    void RemoveExtraStrikeTarget(SpellInfo* spell_info);
    void AddExtraStrikeTarget(SpellInfo* spell_info, uint32 charges);

    int32 GetAP();
    int32 GetRAP();

    uint8 CastSpell(Unit* Target, uint32 SpellID, bool triggered);
    uint8 CastSpell(Unit* Target, SpellInfo* Sp, bool triggered);
    uint8 CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered);
    uint8 CastSpell(uint64 targetGuid, SpellInfo* Sp, bool triggered);
    uint8 CastSpell(Unit* Target, uint32 SpellID, uint32 forced_basepoints, bool triggered);
    uint8 CastSpell(Unit* Target, SpellInfo* Sp, uint32 forced_basepoints, bool triggered);
    uint8 CastSpell(Unit* Target, uint32 SpellID, uint32 forced_basepoints, int32 charges, bool triggered);
    uint8 CastSpell(Unit* Target, SpellInfo* Sp, uint32 forced_basepoints, int32 charges, bool triggered);
    void CastSpellAoF(LocationVector lv, SpellInfo* Sp, bool triggered);
    void EventCastSpell(Unit* Target, SpellInfo* Sp);

    bool IsInInstance();
    void CalculateResistanceReduction(Unit* pVictim, dealdamage* dmg, SpellInfo* ability, float ArmorPctReduce);
    void RegenerateHealth();
    void RegeneratePower(bool isinterrupted);
    void setHRegenTimer(uint32 time) { m_H_regenTimer = static_cast<uint16>(time); }
    void setPRegenTimer(uint32 time) { m_P_regenTimer = static_cast<uint16>(time); }
    void DelayPowerRegeneration(uint32 time) { m_P_regenTimer = static_cast<uint16>(time); if (!m_interruptedRegenTime) m_interruptedRegenTime = 2000; }
    void DeMorph();
    uint32 ManaShieldAbsorb(uint32 dmg);
    void smsg_AttackStart(Unit* pVictim);
    void smsg_AttackStop(Unit* pVictim);
    void smsg_AttackStop(uint64 victimGuid);

    bool IsDazed();
    //this function is used for creatures to get chance to daze for another unit
    float get_chance_to_daze(Unit* target);

    // Stealth
    int32 GetStealthLevel() { return m_stealthLevel; }
    int32 GetStealthDetectBonus() { return m_stealthDetectBonus; }
    void SetStealth(uint32 id) { m_stealth = id; }
    bool IsStealth() { return (m_stealth != 0 ? true : false); }
    float detectRange;

    // Invisibility
    void SetInvisibility(uint32 id) { m_invisibility = id; }
    bool IsInvisible() { return (m_invisible != 0 ? true : false); }
    uint32 m_invisibility;
    bool m_invisible;
    uint8 m_invisFlag;
    int32 m_invisDetect[INVIS_FLAG_TOTAL];
    void SetInvisFlag(uint8 pInvisFlag)
    {
        m_invisFlag = pInvisFlag;
        m_invisible = pInvisFlag != INVIS_FLAG_NORMAL;

        UpdateVisibility();
    }
    uint8 GetInvisFlag() { return m_invisFlag; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // AURAS
    //////////////////////////////////////////////////////////////////////////////////////////

    bool HasAura(uint32 spellid);                   //this checks passive auras too
    uint16 GetAuraStackCount(uint32 spellid);
    bool HasAuraVisual(uint32 visualid);            //not spell id!!!
    bool HasBuff(uint32 spelllid);                  //this does not check passive auras & it was visible auras
    bool HasBuff(uint32 spelllid, uint64 guid);     //this does not check passive auras & it was visible auras
    bool HasAuraWithMechanics(uint32 mechanic);     //this checks passive auras too
    bool HasAurasOfBuffType(uint32 buff_type, const uint64 & guid, uint32 skip);
    bool HasAuraWithName(uint32 name);
    uint32 GetAuraCountWithName(uint32 name);
    uint32 GetAuraCountWithDispelType(uint32 dispel_type, uint64 guid);
    Aura * GetAuraWithSlot(uint32 slot);
    void AddAura(Aura* aur);
    bool RemoveAura(Aura* aur);
    bool RemoveAura(uint32 spellId);
    bool RemoveAura(uint32 spellId, uint64 guid);
    bool RemoveAuraByItemGUID(uint32 spellId, uint64 guid);
    bool RemoveAuras(uint32* SpellIds);
    bool RemoveAurasByHeal();

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Performs the specified action on the auras that meet the specified condition
    /// \param     AuraAction *action        -  The action to perform
    /// \param     AuraCondition *condition  -  The condition that the aura(s) need to meet
    /// \returns true if at least one action was performed, false otherwise.
    //////////////////////////////////////////////////////////////////////////////////////////
    bool AuraActionIf(AuraAction* action, AuraCondition* condition);

    void RemoveAurasByInterruptFlag(uint32 flag);
    void RemoveAurasByInterruptFlagButSkip(uint32 flag, uint32 skip);
    void RemoveAurasByBuffType(uint32 buff_type, const uint64 & guid, uint32 skip);
    void RemoveAurasOfSchool(uint32 School, bool Positive, bool Immune);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Removes all area auras casted by us from the targets, and clears the target sets.
    /// \param none        \return none
    //////////////////////////////////////////////////////////////////////////////////////////
    void ClearAllAreaAuraTargets();

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Removes all Area Auras that are from other Units.
    /// \param none        \return none
    //////////////////////////////////////////////////////////////////////////////////////////
    void RemoveAllAreaAuraByOther();


    void EventRemoveAura(uint32 SpellId) { RemoveAura(SpellId); }

    //! Remove all auras
    void RemoveAllAuras();
    void RemoveAllNonPersistentAuras();
    void RemoveAllAuraType(uint32 auratype);                    //ex:to remove morph spells
    bool RemoveAllAurasByMechanic(uint32 MechanicType, uint32 MaxDispel, bool HostileOnly);       // Removes all (de)buffs on unit of a specific mechanic type.
    void RemoveAllMovementImpairing();
    void RemoveAllAurasByRequiredShapeShift(uint32 mask);

    void RemoveNegativeAuras();
    // Temporary remove all auras

    bool SetAurDuration(uint32 spellId, Unit* caster, uint32 duration);
    bool SetAurDuration(uint32 spellId, uint32 duration);
    void DropAurasOnDeath();
    bool IsControlledByPlayer();

    // ProcTrigger
    std::list<SpellProc*> m_procSpells;
    SpellProc* AddProcTriggerSpell(uint32 spell_id, uint32 orig_spell_id, uint64 caster, uint32 procChance, uint32 procFlags, uint32 procCharges, uint32* groupRelation, uint32* procClassMask = nullptr, Object* obj = nullptr);
    SpellProc* AddProcTriggerSpell(SpellInfo* spell, SpellInfo* orig_spell, uint64 caster, uint32 procChance, uint32 procFlags, uint32 procCharges, uint32* groupRelation, uint32* procClassMask = nullptr, Object* obj = nullptr);
    SpellProc* AddProcTriggerSpell(SpellInfo* sp, uint64 caster, uint32* groupRelation, uint32* procClassMask = nullptr, Object* obj = nullptr);
    SpellProc* GetProcTriggerSpell(uint32 spellId, uint64 casterGuid = 0);
    void RemoveProcTriggerSpell(uint32 spellId, uint64 casterGuid = 0, uint64 misc = 0);

    bool IsPoisoned();

    void GiveGroupXP(Unit* pVictim, Player* PlayerInGroup);

    /// Combat / Death Status
    bool isAlive() { return m_deathState == ALIVE; };
    bool IsDead() { return  m_deathState != ALIVE; };
    virtual void setDeathState(DeathState s)
    {
        m_deathState = s;
        if (m_deathState == JUST_DIED) DropAurasOnDeath();
    };
    DeathState getDeathState() { return m_deathState; }
    void OnDamageTaken();

    //caller is the caster
    int32 GetSpellDmgBonus(Unit* pVictim, SpellInfo* spellInfo, int32 base_dmg, bool isdot);

    float CalcSpellDamageReduction(Unit* victim, SpellInfo* spell, float res);

    uint32 m_addDmgOnce;
    uint32 m_ObjectSlots[4];
    uint32 m_triggerSpell;
    uint32 m_triggerDamage;
    uint32 m_canMove;
    void Possess(Unit* pTarget, uint32 delay = 0);
    void UnPossess();
    SummonHandler summonhandler;

    // Spell Effect Variables
    int32 m_silenced;
    bool m_damgeShieldsInUse;

    std::list<struct DamageProc> m_damageShields;
    std::list<struct ReflectSpellSchool*> m_reflectSpellSchool;

    void RemoveReflect(uint32 spellid, bool apply);

    struct DamageSplitTarget* m_damageSplitTarget;

    std::map<uint32, struct SpellCharge> m_chargeSpells;

    std::deque<uint32> m_chargeSpellRemoveQueue;

    bool m_chargeSpellsInUse;
    void SetOnMeleeSpell(uint32 spell, uint8 ecn = 0) { m_meleespell = spell; m_meleespell_ecn = ecn; }
    uint32 GetOnMeleeSpell() { return m_meleespell; }
    uint8 GetOnMeleeSpellEcn() { return m_meleespell_ecn; }
    void CastOnMeleeSpell();

    uint32 DoDamageSplitTarget(uint32 res, uint32 school_type, bool melee_dmg);

    // Spell Crit
    float spellcritperc;

    // AIInterface
    AIInterface* GetAIInterface() { return m_aiInterface; }
    void ReplaceAIInterface(AIInterface* new_interface);
    void ClearHateList();
    void WipeHateList();
    void WipeTargetList();
    void setAItoUse(bool value) { m_useAI = value; }

    int32 GetThreatModifyer() { return m_threatModifyer; }
    void ModThreatModifyer(int32 mod) { m_threatModifyer += mod; }
    int32 GetGeneratedThreatModifyer(uint32 school) { return m_generatedThreatModifyer[school]; }
    void ModGeneratedThreatModifyer(uint32 school, int32 mod) { m_generatedThreatModifyer[school] += mod; }

    void SetHitFromMeleeSpell(float value) { m_hitfrommeleespell = value; }
    float GetHitFromMeleeSpell() { return m_hitfrommeleespell; }
    float m_hitfrommeleespell;

    // DK:Affect
    uint32 IsPacified() { return m_pacified; }
    uint32 IsStunned() { return m_stunned; }
    uint32 IsFeared() { return m_fearmodifiers; }
    uint32 GetResistChanceMod() { return m_resistChance; }
    void SetResistChanceMod(uint32 amount) { m_resistChance = amount; }

    uint16 HasNoInterrupt() { return m_noInterrupt; }
    bool setDetectRangeMod(uint64 guid, int32 amount);
    void unsetDetectRangeMod(uint64 guid);
    int32 getDetectRangeMod(uint64 guid);
    void Heal(Unit* target, uint32 SpellId, uint32 amount);
    void Energize(Unit* target, uint32 SpellId, uint32 amount, uint32 type);

    Loot loot;
    uint32 SchoolCastPrevent[SCHOOL_COUNT];
    int32 MechanicDurationPctMod[28];

    virtual int32 GetDamageDoneMod(uint32 /*school*/) { return 0; }
    virtual float GetDamageDonePctMod(uint32 /*school*/) { return 0; }

    int32 DamageTakenMod[SCHOOL_COUNT];
    float DamageTakenPctMod[SCHOOL_COUNT];
    float DamageTakenPctModOnHP35;
    float CritMeleeDamageTakenPctMod[SCHOOL_COUNT];
    float CritRangedDamageTakenPctMod[SCHOOL_COUNT];
    int32 RangedDamageTaken;
    void CalcDamage();
    float BaseDamage[2];
    float BaseOffhandDamage[2];
    float BaseRangedDamage[2];
    uint32 AbsorbDamage(uint32 School, uint32* dmg);  //returns amt of absorbed dmg, decreases dmg by absorbed value
    int32 RAPvModifier;
    int32 APvModifier;
    uint64 stalkedby;
    uint32 dispels[10];
    bool trackStealth;
    uint32 MechanicsDispels[32];
    float MechanicsResistancesPCT[32];
    float ModDamageTakenByMechPCT[32];
    int32 DoTPctIncrease[SCHOOL_COUNT];
    float AOEDmgMod;
    float m_ignoreArmorPctMaceSpec;
    float m_ignoreArmorPct;

    //SM
    int32* SM_FDamageBonus; //flat
    int32* SM_PDamageBonus; //pct

    int32* SM_FDur; //flat
    int32* SM_PDur; //pct

    int32* SM_FThreat; //flat
    int32* SM_PThreat; //Pct

    int32* SM_FEffect1_Bonus; //flat
    int32* SM_PEffect1_Bonus; //Pct

    int32* SM_FCharges; //flat
    int32* SM_PCharges; //Pct

    int32* SM_FRange; //flat
    int32* SM_PRange; //pct

    int32* SM_FRadius; //flat
    int32* SM_PRadius; //pct

    int32* SM_CriticalChance; //flat

    int32* SM_FMiscEffect; //flat
    int32* SM_PMiscEffect; //pct

    int32* SM_PNonInterrupt; //Pct

    int32* SM_FCastTime; //flat
    int32* SM_PCastTime; //pct

    int32* SM_FCooldownTime; //flat
    int32* SM_PCooldownTime; //Pct

    int32* SM_FEffect2_Bonus; //flat
    int32* SM_PEffect2_Bonus; //Pct

    int32* SM_FCost; //flat
    int32* SM_PCost; //Pct

    int32* SM_PCriticalDamage; //Pct

    int32* SM_FHitchance; //flat

    int32* SM_FAdditionalTargets; //flat

    int32* SM_FChanceOfSuccess; //flat

    int32* SM_FAmptitude; //flat
    int32* SM_PAmptitude; //Pct

    int32* SM_PJumpReduce; //Pct

    int32* SM_FGlobalCooldown; //flat
    int32* SM_PGlobalCooldown; //pct

    int32* SM_FDOT; //flat
    int32* SM_PDOT; //pct

    int32* SM_FEffect3_Bonus; //flat
    int32* SM_PEffect3_Bonus; //Pct

    int32* SM_FPenalty; //flat
    int32* SM_PPenalty; //Pct

    int32* SM_FEffectBonus; //flat
    int32* SM_PEffectBonus; //pct

    int32* SM_FRezist_dispell; //flat
    int32* SM_PRezist_dispell; //Pct

    void InheritSMMods(Unit* inherit_from);

    //Events
    void Emote(EmoteType emote);
    void EventAddEmote(EmoteType emote, uint32 time);
    void EmoteExpire();
    void setEmoteState(uint8 emote) { m_emoteState = emote; };
    uint32 GetOldEmote() { return m_oldEmote; }
    void EventAurastateExpire(uint32 aurastateflag) { RemoveFlag(UNIT_FIELD_AURASTATE, aurastateflag); }    //hmm this looks like so not necessary :S
    void EventHealthChangeSinceLastUpdate();

    // Stun Immobilize
    uint32 trigger_on_stun;        //bah, warrior talent but this will not get triggered on triggered spells if used on proc so I'm forced to used a special variable
    uint32 trigger_on_stun_chance;
    uint32 trigger_on_stun_victim;
    uint32 trigger_on_stun_chance_victim;

    void SetTriggerStunOrImmobilize(uint32 newtrigger, uint32 new_chance, bool is_victim = false)
    {
        if (is_victim == false)
        {
            trigger_on_stun = newtrigger;
            trigger_on_stun_chance = new_chance;
        }
        else
        {
            trigger_on_stun_victim = newtrigger;
            trigger_on_stun_chance_victim = new_chance;
        }
    }
    void EventStunOrImmobilize(Unit* proc_target, bool is_victim = false);

    ///\todo Remove this hack
    // Chill
    uint32 trigger_on_chill;         //mage "Frostbite" talent chill
    uint32 trigger_on_chill_chance;
    uint32 trigger_on_chill_victim;
    uint32 trigger_on_chill_chance_victim;

    void SetTriggerChill(uint32 newtrigger, uint32 new_chance, bool is_victim = false)
    {
        if (is_victim == false)
        {
            trigger_on_chill = newtrigger;
            trigger_on_chill_chance = new_chance;
        }
        else
        {
            trigger_on_chill_victim = newtrigger;
            trigger_on_chill_chance_victim = new_chance;
        }
    }
    void EventChill(Unit* proc_target, bool is_victim = false);

    bool IsSitting();
    void SetStandState(uint8 standstate);

    StandState GetStandState()
    {
        uint32 bytes1 = getUInt32Value(UNIT_FIELD_BYTES_1);
        return StandState(uint8(bytes1));
    }

    uint32 GetFaction() { return getUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }

    void SetFaction(uint32 factionId)
    {
        setUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, factionId);
        _setFaction();
    }

    virtual void SendChatMessage(uint8 type, uint32 lang, const char* msg, uint32 delay = 0) = 0;
    virtual void SendChatMessageToPlayer(uint8 type, uint32 lang, const char* msg, Player* plr) = 0;
    void SendChatMessageAlternateEntry(uint32 entry, uint8 type, uint32 lang, const char* msg);
    void RegisterPeriodicChatMessage(uint32 delay, uint32 msgid, std::string message, bool sendnotify);

    int GetHealthPct()
    {
        //shitty db? pet/guardian bug?
        if (getUInt32Value(UNIT_FIELD_HEALTH) == 0 || getUInt32Value(UNIT_FIELD_MAXHEALTH) == 0)
            return 0;

        return (int)(getUInt32Value(UNIT_FIELD_HEALTH) * 100 / getUInt32Value(UNIT_FIELD_MAXHEALTH));
    };

    void SetHealthPct(uint32 val) { if (val > 0) SetHealth(float2int32(val * 0.01f * getUInt32Value(UNIT_FIELD_MAXHEALTH))); };

    int GetManaPct()
    {
        if (GetPower(0) == 0 || GetMaxPower(0) == 0)  //POWER_TYPE_MANA
            return 0;

        return (int)(GetPower(0) * 100 / GetMaxPower(0));
    };

    //In-Range
    virtual void AddInRangeObject(Object* pObj);
    virtual void OnRemoveInRangeObject(Object* pObj);
    void ClearInRangeSet();

    uint32 m_CombatUpdateTimer;

    void setcanparry(bool newstatus) { can_parry = newstatus; }

    std::map<uint32, Aura*> tmpAura;

    uint32 BaseResistance[SCHOOL_COUNT];        //there are resistances for silence, fear, mechanics ....
    uint32 BaseStats[5];

    int32 HealDoneMod[SCHOOL_COUNT];
    float HealDonePctMod[SCHOOL_COUNT];

    int32 HealTakenMod[SCHOOL_COUNT];
    float HealTakenPctMod[SCHOOL_COUNT];

    uint32 SchoolImmunityList[SCHOOL_COUNT];
    float SpellCritChanceSchool[SCHOOL_COUNT];

    int32 PowerCostMod[SCHOOL_COUNT];
    float PowerCostPctMod[SCHOOL_COUNT];        // armor penetration & spell penetration

    int32 AttackerCritChanceMod[SCHOOL_COUNT];
    uint32 SpellDelayResist[SCHOOL_COUNT];

    int32 CreatureAttackPowerMod[12];
    int32 CreatureRangedAttackPowerMod[12];

    int32 PctRegenModifier;
    float PctPowerRegenModifier[4];

    void UpdatePowerAmm();

    // Auras Modifiers
    int32 m_pacified;
    int32 m_interruptRegen;
    int32 m_resistChance;
    int32 m_powerRegenPCT;
    int32 m_stunned;
    int32 m_extraattacks;
    bool m_extrastriketarget;
    int32 m_extrastriketargetc;
    std::list<ExtraStrike*> m_extraStrikeTargets;
    int32 m_fearmodifiers;
    int64 m_magnetcaster;       // Unit who acts as a magnet for this unit

                                //Combat Mod Results:
    int32 m_CombatResult_Dodge;
    int32 m_CombatResult_Parry; // is not implented yet

                                // aurastate counters
    int8 asc_frozen;
    int8 asc_enraged;
    int8 asc_seal;
    int8 asc_bleed;

    uint16 m_noInterrupt;
    bool disarmed;
    uint64 m_detectRangeGUID[5];
    int32  m_detectRangeMOD[5];

    // Affect Speed
    int32 m_speedModifier;
    int32 m_slowdown;
    float m_maxSpeed;
    std::map< uint32, int32 > speedReductionMap;
    bool GetSpeedDecrease();
    int32 m_mountedspeedModifier;
    int32 m_flyspeedModifier;

    void UpdateSpeed();

    // Escort Quests
    void MoveToWaypoint(uint32 wp_id);

    void RemoveStealth()
    {
        if (m_stealth != 0)
        {
            RemoveAura(m_stealth);
            m_stealth = 0;
        }
    }

    void RemoveInvisibility()
    {
        if (m_invisibility != 0)
        {
            RemoveAura(m_invisibility);
            m_invisibility = 0;
        }
    }

    uint32 m_stealth;
    bool m_can_stealth;

    Aura* m_auras[MAX_TOTAL_AURAS_END];

    int32 m_modlanguage;

    uint32 GetCharmTempVal() { return m_charmtemp; }
    void SetCharmTempVal(uint32 val) { m_charmtemp = val; }

    void DisableAI() { m_useAI = false; }
    void EnableAI() { m_useAI = true; }

    void Phase(uint8 command = PHASE_SET, uint32 newphase = 1);

    bool Tagged;
    uint64 TaggerGuid;
    void Tag(uint64 TaggerGUID);
    void UnTag();
    bool IsTagged();
    bool IsTaggable();
    uint64 GetTaggerGUID();
    bool isLootable();

    virtual bool isTrainingDummy() { return false; }

    void SetFacing(float newo);     //only working if creature is idle

    AuraCheckResponse AuraCheck(SpellInfo* proto, Object* caster = nullptr);
    AuraCheckResponse AuraCheck(SpellInfo* proto, Aura* aur, Object* caster = nullptr);

    uint16 m_diminishCount[DIMINISHING_GROUP_COUNT];
    uint8 m_diminishAuraCount[DIMINISHING_GROUP_COUNT];
    uint16 m_diminishTimer[DIMINISHING_GROUP_COUNT];
    bool m_diminishActive;

    void SetDiminishTimer(uint32 index)
    {
        m_diminishTimer[index] = 15000;
    }

    DynamicObject* dynObj;

    //! returns: aura stack count
    uint8 m_auraStackCount[MAX_NEGATIVE_VISUAL_AURAS_END];

    void SendFullAuraUpdate();
    void SendAuraUpdate(uint32 AuraSlot, bool remove);
    uint32 ModVisualAuraStackCount(Aura* aur, int32 count);
    uint8 FindVisualSlot(uint32 SpellId, bool IsPos);
    uint32 m_auravisuals[MAX_NEGATIVE_VISUAL_AURAS_END];

    bool bProcInUse;
    bool bInvincible;
    Player* m_redirectSpellPackets;
    void UpdateVisibility();

    //solo target auras
    uint32 polySpell;

    struct
    {
        int32 amt;
        int32 max;
    } m_soulSiphon;

    uint32 m_cTimer;
    void EventUpdateFlag();
    CombatStatusHandler CombatStatus;
    bool m_temp_summon;

    void EventStopChanneling(bool abort);
    void EventStrikeWithAbility(uint64 guid, SpellInfo* sp, uint32 damage);
    void DispelAll(bool positive);

    void SendPowerUpdate(bool self);
    void SendPeriodicAuraLog(const WoWGuid & CasterGUID, const WoWGuid & casterGUID, uint32 SpellID, uint32 School, uint32 Amount, uint32 abs_dmg, uint32 resisted_damage, uint32 Flags, bool is_critical);
    void SendPeriodicHealAuraLog(const WoWGuid & CasterGUID, const WoWGuid & TargetGUID, uint32 SpellID, uint32 healed, uint32 over_healed, bool is_critical);

    void EventModelChange();
    inline float GetModelHalfSize() { return m_modelhalfsize * GetScale(); }

    void RemoveFieldSummon();

    float GetBlockFromSpell() { return m_blockfromspell; }
    float GetParryFromSpell() { return m_parryfromspell; }
    float GetDodgeFromSpell() { return m_dodgefromspell; }
    void SetBlockFromSpell(float value) { m_blockfromspell = value; }
    void SetParryFromSpell(float value) { m_parryfromspell = value; }
    void SetDodgeFromSpell(float value) { m_dodgefromspell = value; }

    void AggroPvPGuards();

    virtual void SetShapeShift(uint8 ss) { setByteValue(UNIT_FIELD_BYTES_2, 3, ss); }
    uint8 GetShapeShift() { return getByteValue(UNIT_FIELD_BYTES_2, 3); }
    uint32 GetShapeShiftMask() { return ((uint32)1 << (GetShapeShift() - 1)); }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Unit properties
    //////////////////////////////////////////////////////////////////////////////////////////
    void SetCharmedUnitGUID(uint64 GUID) { setUInt64Value(UNIT_FIELD_CHARM, GUID); }
    void SetSummonedUnitGUID(uint64 GUID) { setUInt64Value(UNIT_FIELD_SUMMON, GUID); }
    void SetSummonedCritterGUID(uint64 GUID)
    {
#if VERSION_STRING > WotLK
        setUInt64Value(UNIT_FIELD_CRITTER, GUID);
#endif
    }

    void SetCharmedByGUID(uint64 GUID) { setUInt64Value(UNIT_FIELD_CHARMEDBY, GUID); }
    void SetSummonedByGUID(uint64 GUID) { setUInt64Value(UNIT_FIELD_SUMMONEDBY, GUID); }
    void SetCreatedByGUID(uint64 GUID) { setUInt64Value(UNIT_FIELD_CREATEDBY, GUID); }


    uint64 GetCharmedUnitGUID() { return getUInt64Value(UNIT_FIELD_CHARM); }
    uint64 GetSummonedUnitGUID() { return getUInt64Value(UNIT_FIELD_SUMMON); }
    uint64 GetSummonedCritterGUID()
    {
#if VERSION_STRING > WotLK
        return getUInt64Value(UNIT_FIELD_CRITTER);
#else
        return 0;
#endif
    }

    uint64 GetCharmedByGUID() { return getUInt64Value(UNIT_FIELD_CHARMEDBY); }
    uint64 GetSummonedByGUID() { return getUInt64Value(UNIT_FIELD_SUMMONEDBY); }
    uint64 GetCreatedByGUID() { return getUInt64Value(UNIT_FIELD_CREATEDBY); }

    void SetTargetGUID(uint64 GUID) { setUInt64Value(UNIT_FIELD_TARGET, GUID); }
    uint64 GetTargetGUID() { return getUInt64Value(UNIT_FIELD_TARGET); }

    void SetChannelSpellTargetGUID(uint64 GUID) { setUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, GUID); }
    void SetChannelSpellId(uint32 SpellId) { setUInt32Value(UNIT_CHANNEL_SPELL, SpellId); }

    uint64 GetChannelSpellTargetGUID() { return getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT); }
    uint32 GetChannelSpellId() { return getUInt32Value(UNIT_CHANNEL_SPELL); }

    void SetEquippedItem(uint8 slot, uint32 id) { setUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + slot, id); }
    uint32 GetEquippedItem(uint8 slot) { return getUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + slot); }

    void SetBaseAttackTime(uint8 slot, uint32 time) { setUInt32Value(UNIT_FIELD_BASEATTACKTIME + slot, time); }
    uint32 GetBaseAttackTime(uint8 slot) { return getUInt32Value(UNIT_FIELD_BASEATTACKTIME + slot); }
    void ModBaseAttackTime(uint8 slot, int32 mod) { ModUnsigned32Value(UNIT_FIELD_BASEATTACKTIME + slot, mod); }

    void SetBoundingRadius(float rad) { setFloatValue(UNIT_FIELD_BOUNDINGRADIUS, rad); }
    float GetBoundingRadius() { return getFloatValue(UNIT_FIELD_BOUNDINGRADIUS); }

    void SetCombatReach(float len) { setFloatValue(UNIT_FIELD_COMBATREACH, len); }
    float GetCombatReach() { return getFloatValue(UNIT_FIELD_COMBATREACH); }

    void SetDisplayId(uint32 id) { setUInt32Value(UNIT_FIELD_DISPLAYID, id); }
    uint32 GetDisplayId() { return getUInt32Value(UNIT_FIELD_DISPLAYID); }

    void SetNativeDisplayId(uint32 id) { setUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, id); }
    uint32 GetNativeDisplayId() { return getUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetMinDamage(float amt) { setFloatValue(UNIT_FIELD_MINDAMAGE, amt); }
    float GetMinDamage() { return getFloatValue(UNIT_FIELD_MINDAMAGE); }

    void SetMaxDamage(float amt) { setFloatValue(UNIT_FIELD_MAXDAMAGE, amt); }
    float GetMaxDamage() { return getFloatValue(UNIT_FIELD_MAXDAMAGE); }

    void SetMinOffhandDamage(float amt) { setFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, amt); }
    float GetMinOffhandDamage() { return getFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE); }

    void SetMaxOffhandDamage(float amt) { setFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, amt); }
    float GetMaxOffhandDamage() { return getFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE); }

    void SetMinRangedDamage(float amt) { setFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, amt); }
    float GetMinRangedDamage() { return getFloatValue(UNIT_FIELD_MINRANGEDDAMAGE); }

    void SetMaxRangedDamage(float amt) { setFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, amt); }
    float GetMaxRangedDamage() { return getFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetMount(uint32 id) { setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, id); }
    uint32 GetMount() { return getUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }

    void SetCastSpeedMod(float amt) { setFloatValue(UNIT_MOD_CAST_SPEED, amt); }
    float GetCastSpeedMod() { return getFloatValue(UNIT_MOD_CAST_SPEED); }
    void ModCastSpeedMod(float mod) { ModFloatValue(UNIT_MOD_CAST_SPEED, mod); }

    void SetCreatedBySpell(uint32 id) { setUInt32Value(UNIT_CREATED_BY_SPELL, id); }
    uint32 GetCreatedBySpell() { return getUInt32Value(UNIT_CREATED_BY_SPELL); }

    void SetEmoteState(uint32 id) { setUInt32Value(UNIT_NPC_EMOTESTATE, id); }
    uint32 GetEmoteState() { return getUInt32Value(UNIT_NPC_EMOTESTATE); }

    void SetStat(uint16_t stat, uint32 amt) { setUInt32Value(UNIT_FIELD_STAT0 + stat, amt); }
    uint32 GetStat(uint16_t stat) { return getUInt32Value(UNIT_FIELD_STAT0 + stat); }

    void SetResistance(uint16_t type, uint32 amt) { setUInt32Value(UNIT_FIELD_RESISTANCES + type, amt); }
    uint32 GetResistance(uint16_t type) { return getUInt32Value(UNIT_FIELD_RESISTANCES + type); }

    void SetBaseMana(uint32 amt) { setUInt32Value(UNIT_FIELD_BASE_MANA, amt); }
    uint32 GetBaseMana() { return getUInt32Value(UNIT_FIELD_BASE_MANA); }

    void SetBaseHealth(uint32 amt) { setUInt32Value(UNIT_FIELD_BASE_HEALTH, amt); }
    uint32 GetBaseHealth() { return getUInt32Value(UNIT_FIELD_BASE_HEALTH); }

    void SetPowerCostMultiplier(uint16_t school, float amt) { setFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school, amt); }
    void ModPowerCostMultiplier(uint16_t school, float amt) { ModFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school, amt); }
    float GetPowerCostMultiplier(uint16_t school) { return getFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetAttackPower(uint32 amt) { setUInt32Value(UNIT_FIELD_ATTACK_POWER, amt); }
    uint32 GetAttackPower() { return getUInt32Value(UNIT_FIELD_ATTACK_POWER); }

    //\todo fix this
    void SetAttackPowerMods(uint32 amt)
    {
#if VERSION_STRING != Cata
        setUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS, amt);
#else
        if (amt == 0) { return; }
#endif
    }

    //\todo fix this
    uint32 GetAttackPowerMods()
    {
#if VERSION_STRING != Cata
        return getUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);
#else
        return 0;
#endif
    }

    //\todo fix this
    void ModAttackPowerMods(uint32 amt)
    {
#if VERSION_STRING != Cata
        ModUnsigned32Value(UNIT_FIELD_ATTACK_POWER_MODS, amt);
#else
        if (amt == 0) { return; }
#endif
    }

    void SetAttackPowerMultiplier(float amt) { setFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, amt); }
    float GetAttackPowerMultiplier() { return getFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER); }
    void ModAttackPowerMultiplier(float amt) { ModFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, amt); }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetRangedAttackPower(uint32 amt) { setUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, amt); }
    uint32 GetRangedAttackPower() { return getUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER); }

    //\todo fix this
    void SetRangedAttackPowerMods(uint32 amt)
    {
#if VERSION_STRING != Cata
        setUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS, amt);
#else
        if (amt == 0) { return; }
#endif
    }

    //\todo fix this
    uint32 GetRangedAttackPowerMods()
    {
#if VERSION_STRING != Cata
        return getUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS);
#else
        return 0;
#endif
    }

    //\todo fix this
    void ModRangedAttackPowerMods(uint32 amt)
    {
#if VERSION_STRING != Cata
        ModUnsigned32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS, amt);
#else
        if (amt == 0) { return; }
#endif
    }

    void SetRangedAttackPowerMultiplier(float amt) { setFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, amt); }
    float GetRangedAttackPowerMultiplier() { return getFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER); }
    void ModRangedAttackPowerMultiplier(float amt) { ModFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, amt); }

    //////////////////////////////////////////////////////////////////////////////////////////
    // bytes 0

    void setRace(uint8 race) { setByteValue(UNIT_FIELD_BYTES_0, 0, race); }
    uint8 getRace() { return getByteValue(UNIT_FIELD_BYTES_0, 0); }

    void setClass(uint8 class_) { setByteValue(UNIT_FIELD_BYTES_0, 1, class_); }
    uint8 getClass() { return getByteValue(UNIT_FIELD_BYTES_0, 1); }

    uint8 getGender() { return getByteValue(UNIT_FIELD_BYTES_0, 2); }
    void setGender(uint8 gender) { setByteValue(UNIT_FIELD_BYTES_0, 2, gender); }

    void SetPowerType(uint8 type) { setByteValue(UNIT_FIELD_BYTES_0, 3, type); }
    uint8 GetPowerType() { return getByteValue(UNIT_FIELD_BYTES_0, 3); }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void SetHealth(uint32 val) { setUInt32Value(UNIT_FIELD_HEALTH, val); }
    void SetMaxHealth(uint32 val) { setUInt32Value(UNIT_FIELD_MAXHEALTH, val); }

    uint32 GetHealth()    const { return getUInt32Value(UNIT_FIELD_HEALTH); }
    uint32 GetMaxHealth() const { return getUInt32Value(UNIT_FIELD_MAXHEALTH); }

    void ModHealth(int32 val) { ModUnsigned32Value(UNIT_FIELD_HEALTH, val); }
    void ModMaxHealth(int32 val) { ModUnsigned32Value(UNIT_FIELD_MAXHEALTH, val); }

    void SetPower(uint32 type, int32 value);

    void ModPower(uint16_t index, int32 value)
    {
        int32 power = static_cast<int32>(m_uint32Values[UNIT_FIELD_POWER1 + index]);
        int32 maxpower = static_cast<int32>(m_uint32Values[UNIT_FIELD_MAXPOWER1 + index]);

        if (value <= power)
            setUInt32Value(UNIT_FIELD_POWER1 + index, 0);
        else
            setUInt32Value(UNIT_FIELD_POWER1 + index, power + value);

        if ((value + power) > maxpower)
            setUInt32Value(UNIT_FIELD_POWER1 + index, maxpower);
        else
            setUInt32Value(UNIT_FIELD_POWER1 + index, power + value);
    }

    uint32 GetPower(uint16_t index) { return getUInt32Value(UNIT_FIELD_POWER1 + index); }

    void SetMaxPower(uint16_t index, uint32 value) { setUInt32Value(UNIT_FIELD_MAXPOWER1 + index, value); }

    void ModMaxPower(uint16_t index, int32 value) { ModUnsigned32Value(UNIT_FIELD_MAXPOWER1 + index, value); }

    uint32 GetMaxPower(uint16_t index) { return getUInt32Value(UNIT_FIELD_MAXPOWER1 + index); }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void TakeDamage(Unit* pAttacker, uint32 damage, uint32 spellid, bool no_remove_auras = false);
    virtual void Die(Unit* pAttacker, uint32 damage, uint32 spellid);
    virtual bool isCritter() { return false; }

    virtual void HandleKnockback(Object* caster, float horizontal, float vertical);

    void AddGarbagePet(Pet* pet);

    virtual void BuildPetSpellList(WorldPacket & data);

    uint64 GetAuraUpdateMaskForRaid() const { return m_auraRaidUpdateMask; }
    void ResetAuraUpdateMaskForRaid() { m_auraRaidUpdateMask = 0; }
    void SetAuraUpdateMaskForRaid(uint8 slot) { m_auraRaidUpdateMask |= (uint64(1) << slot); }
    void UpdateAuraForGroup(uint8 slot);
    void HandleUpdateFieldChange(uint32 Index);

    Movement::UnitMovementManager m_movementManager;
protected:

    Unit();
    void RemoveGarbage();
    void AddGarbageAura(Aura* aur);
    void AddGarbageSpell(Spell* sp);

    uint32 m_meleespell;
    uint8 m_meleespell_ecn;         // extra_cast_number

    uint16 m_H_regenTimer;
    uint16 m_P_regenTimer;
    uint32 m_interruptedRegenTime;  //PowerInterruptedegenTimer.

    uint32 m_attackTimer;           // timer for attack
    uint32 m_attackTimer_1;
    bool m_dualWield;

    std::list<Aura*> m_GarbageAuras;
    std::list<Spell*> m_GarbageSpells;
    std::list<Pet*> m_GarbagePets;

    /// Combat
    DeathState m_deathState;

    // Stealth
    uint32 m_stealthLevel;
    uint32 m_stealthDetectBonus;

    // DK:pet

    // AI
    AIInterface* m_aiInterface;
    bool m_useAI;
    bool can_parry;         //will be enabled by block spell
    int32 m_threatModifyer;
    int32 m_generatedThreatModifyer[SCHOOL_COUNT];

    int32 m_manashieldamt;
    uint32 m_manaShieldId;

    // Quest emote
    uint8 m_emoteState;
    uint32 m_oldEmote;

    // Some auras can only be cast on one target at a time
    // This will map aura spell id to target guid
    UniqueAuraTargetMap m_singleTargetAura;

    uint32 m_charmtemp;

    bool m_extraAttackCounter;

    float m_modelhalfsize;      // used to calculate if something is in range of this unit

    float m_blockfromspell;
    float m_dodgefromspell;
    float m_parryfromspell;
    uint32 m_BlockModPct;       // is % but does not need float and does not need /100!

    Vehicle* currentvehicle;    // The vehicle the unit is attached to
    Vehicle* vehicle;           // The Unit's own vehicle component
    uint64 m_auraRaidUpdateMask;

public:

    void SetCurrentVehicle(Vehicle* v) { currentvehicle = v; }
    void EnterVehicle(uint64 guid, uint32 delay);
    Vehicle* GetCurrentVehicle();

    Vehicle* GetVehicleComponent();

    virtual void AddVehicleComponent(uint32 /*creatureEntry*/, uint32 /*vehicleId*/) {}
    virtual void RemoveVehicleComponent() {}

    void SendHopOnVehicle(Unit* vehicleowner, uint32 seat);
    void SendHopOffVehicle(Unit* vehicleowner, LocationVector &landposition);

    Unit* GetVehicleBase();

    virtual Group* GetGroup() { return nullptr; }
    bool InParty(Unit* u);
    bool InRaid(Unit* u);
    const CombatStatusHandler* getcombatstatus() const { return &CombatStatus; }

    bool m_noFallDamage;
    float z_axisposition;
    int32 m_safeFall;

    void SendEnvironmentalDamageLog(uint64 guid, uint8 type, uint32 damage);

    void BuildHeartBeatMsg(WorldPacket* data);

    void BuildMovementPacket(ByteBuffer* data);
    void BuildMovementPacket(ByteBuffer* data, float x, float y, float z, float o);

    MovementInfo* GetMovementInfo() { return &movement_info; }

#if VERSION_STRING != Cata
    uint32 GetUnitMovementFlags() const { return movement_info.flags; }   //checked
    void SetUnitMovementFlags(uint32 f) { movement_info.flags = f; }
    void AddUnitMovementFlag(uint32 f) { movement_info.flags |= f; }
    void RemoveUnitMovementFlag(uint32 f) { movement_info.flags &= ~f; }
    bool HasUnitMovementFlag(uint32 f) const { return (movement_info.flags & f) != 0; }

    uint16 GetExtraUnitMovementFlags() const { return movement_info.flags2; }
    void AddExtraUnitMovementFlag(uint16 f2) { movement_info.flags2 |= f2; }
    bool HasExtraUnitMovementFlag(uint16 f2) const { return (movement_info.flags2 & f2) != 0; }
#else
    MovementFlags GetUnitMovementFlags() const { return movement_info.getMovementFlags(); }   //checked
    void SetUnitMovementFlags(MovementFlags f) { movement_info.setMovementFlags(f); }
    void AddUnitMovementFlag(MovementFlags f) { movement_info.addMovementFlag(f); }
    void RemoveUnitMovementFlag(MovementFlags f) { movement_info.removeMovementFlag(f); }
    bool HasUnitMovementFlag(MovementFlags f) const { return (movement_info.getMovementFlags() & f) != 0; }

    MovementFlags2 GetExtraUnitMovementFlags() const { return movement_info.getMovementFlags2(); }
    void AddExtraUnitMovementFlag(MovementFlags2 f2) { movement_info.addMovementFlags2(f2); }
    bool HasExtraUnitMovementFlag(MovementFlags2 f2) const { return (movement_info.getMovementFlags2() & f2) != 0; }
#endif

    MovementInfo movement_info;
    // AGPL End
};