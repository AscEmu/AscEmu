/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Object.h"
#include "UnitDefines.hpp"
#include "Management/LootMgr.h"
#include "Macros/UnitMacros.hpp"
#include "Units/Summons/SummonHandler.h"
#include "Spell/Definitions/AuraEffects.hpp"
#include "Spell/Definitions/AuraStates.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/Definitions/ProcFlags.hpp"
#include "Spell/Definitions/School.hpp"
#include "Spell/Definitions/SpellModifierType.hpp"
#include "Spell/SpellDefines.hpp"
#include "Spell/SpellProc.hpp"
#include "Storage/MySQLStructures.h"
#include "ThreatHandler.h"
#include "Movement/AbstractFollower.h"
#include <optional>

struct DamageSplitTarget;
template <class T>
using Optional = std::optional<T>;

class AIInterface;
class Aura;
class DynamicObject;
class GameObject;
class Group;
class Object;
class Pet;
class Spell;
class SpellProc;
class TotemSummon;
class Vehicle;
class MovementManager;
struct FactionDBC;

namespace MovementNew {
class MoveSpline;
}

enum MovementGeneratorType : uint8_t;

enum UnitSpeedType : uint8_t
{
    TYPE_WALK           = 0,
    TYPE_RUN            = 1,
    TYPE_RUN_BACK       = 2,
    TYPE_SWIM           = 3,
    TYPE_SWIM_BACK      = 4,
    TYPE_TURN_RATE      = 5,
    TYPE_FLY            = 6,
    TYPE_FLY_BACK       = 7,
    TYPE_PITCH_RATE     = 8,
    MAX_SPEED_TYPE      = 9
};

struct UnitSpeedInfo
{
    UnitSpeedInfo()
    {
        // Current speed
        m_currentSpeedRate[TYPE_WALK]       = 2.5f;
        m_currentSpeedRate[TYPE_RUN]        = 7.0f;
        m_currentSpeedRate[TYPE_RUN_BACK]   = 4.5f;
        m_currentSpeedRate[TYPE_SWIM]       = 4.722222f;
        m_currentSpeedRate[TYPE_SWIM_BACK]  = 2.5f;
        m_currentSpeedRate[TYPE_TURN_RATE]  = 3.141594f;
        m_currentSpeedRate[TYPE_FLY]        = 7.0f;
        m_currentSpeedRate[TYPE_FLY_BACK]   = 4.5f;
        m_currentSpeedRate[TYPE_PITCH_RATE] = 3.14f;

        // Basic speeds
        m_basicSpeedRate[TYPE_WALK]         = 2.5f;
        m_basicSpeedRate[TYPE_RUN]          = 7.0f;
        m_basicSpeedRate[TYPE_RUN_BACK]     = 4.5f;
        m_basicSpeedRate[TYPE_SWIM]         = 4.722222f;
        m_basicSpeedRate[TYPE_SWIM_BACK]    = 2.5f;
        m_basicSpeedRate[TYPE_TURN_RATE]    = 3.141594f;
        m_basicSpeedRate[TYPE_FLY]          = 7.0f;
        m_basicSpeedRate[TYPE_FLY_BACK]     = 4.5f;
        m_basicSpeedRate[TYPE_PITCH_RATE]   = 3.14f;
    }

    UnitSpeedInfo(UnitSpeedInfo const& speedInfo)
    {
        // Current speed
        for (uint8_t i = 0; i < MAX_SPEED_TYPE; ++i)
        {
            m_currentSpeedRate[i] = speedInfo.m_currentSpeedRate[i];
            m_basicSpeedRate[i] = speedInfo.m_basicSpeedRate[i];
        }
    }

    float m_currentSpeedRate[MAX_SPEED_TYPE];
    float m_basicSpeedRate[MAX_SPEED_TYPE];
};

struct HealthBatchEvent
{
    Unit* caster = nullptr; // the unit who created this damage or healing event
    DamageInfo damageInfo = DamageInfo();

    bool isPeriodic = false;
    bool isHeal = false;

    bool isEnvironmentalDamage = false;
    EnviromentalDamage environmentType = DAMAGE_EXHAUSTED;

    bool isLeech = false;
    float leechMultipleValue = 0.0f;

    SpellInfo const* spellInfo = nullptr;
};

typedef std::unordered_map<uint32, uint64> UniqueAuraTargetMap;

//////////////////////////////////////////////////////////////////////////////////////////
/// Checks for conditions specified in subclasses on Auras. When calling operator()
/// it tells if the conditions are met.
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
class SERVER_DECL AuraAction
{
public:

    virtual void operator()(Aura* /*aura*/) {}
};

struct ReflectSpellSchool
{
    uint32_t spellId;
    uint32_t charges;
    int32_t school;
    int32_t chance;
    bool infront;
};

struct OnHitSpell
{
    uint32_t spellid;
    uint32_t mindmg;
    uint32_t maxdmg;
};

struct AreaAura
{
    uint32_t auraid;
    Unit* caster;
};

typedef struct
{
    SpellInfo const* spell_info;
    uint32_t charges;
} ExtraStrike;

struct AuraCheckResponse
{
    uint32_t Error;
    uint32_t Misc;
};

typedef std::list<struct ProcTriggerSpellOnSpell> ProcTriggerSpellOnSpellList;

class Unit;
class SERVER_DECL CombatStatusHandler
{
    typedef std::set<uint64_t> AttackerMap;
    typedef std::set<uint32_t> HealedSet; // Must Be Players!

    HealedSet m_healers;
    HealedSet m_healed;

    Unit* m_Unit;

    bool m_lastStatus = false;

    AttackerMap m_attackTargets;

    uint64_t m_primaryAttackTarget = 0;

public:
    CombatStatusHandler(Unit* _unit) : m_Unit(_unit) {}

    AttackerMap m_attackers;

    void AddAttackTarget(const uint64_t& guid);                      // this means we clicked attack, not actually striked yet, so they shouldn't be in combat.
    void ClearPrimaryAttackTarget();                                // means we deselected the unit, stopped attacking it.

    void OnDamageDealt(Unit* pTarget);                              // this is what puts the other person in combat.
    void WeHealed(Unit* pHealTarget);                               // called when a player heals another player, regardless of combat state.

    void RemoveAttacker(Unit* pAttacker, const uint64_t& guid);      // this means we stopped attacking them totally. could be because of deaggro, etc.
    void RemoveAttackTarget(Unit* pTarget);                         // means our DoT expired.

    void UpdateFlag();                                              // detects if we have changed combat state (in/out), and applies the flag.
    bool IsInCombat() const;                                        // checks if we are in combat or not.
    void OnRemoveFromWorld();                                       // called when we are removed from world, kills all references to us.

    void Vanished()
    {
        ClearAttackers();
        ClearHealers();
    }

    const uint64_t& GetPrimaryAttackTarget() { return m_primaryAttackTarget; }
    void SetUnit(Unit* p) { m_Unit = p; }
    void TryToClearAttackTargets();                                 // for pvp timeout
    void AttackersForgetHate();                                     // used right now for Feign Death so attackers go home

protected:
    bool InternalIsInCombat();                                      // called by UpdateFlag, do not call from anything else!
    bool IsAttacking(Unit* pTarget);                                // internal function used to determine if we are still attacking target x.
    void AddAttacker(const uint64_t& guid);                          // internal function to add an attacker
    void RemoveHealed(Unit* pHealTarget);                           // usually called only by updateflag
    void ClearHealers();                                            // this is called on instance change.
    void ClearAttackers();                                          // means we vanished, or died.
    void ClearMyHealers();
};

struct WoWUnit;

class SERVER_DECL Unit : public Object
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    const WoWUnit* unitData() const { return reinterpret_cast<WoWUnit*>(wow_data); }

    friend class ThreatManager;
    ThreatManager m_threatManager = this;
public:
    void setLocationWithoutUpdate(LocationVector& location);
public:
    uint64_t getCharmGuid() const;
    void setCharmGuid(uint64_t guid);

    uint64_t getSummonGuid() const;
    void setSummonGuid(uint64_t guid);

#if VERSION_STRING > TBC
    uint64_t getCritterGuid() const;
    void setCritterGuid(uint64_t guid);
#endif

    uint64_t getCharmedByGuid() const;
    void setCharmedByGuid(uint64_t guid);

    uint64_t getSummonedByGuid() const;
    void setSummonedByGuid(uint64_t guid);

    uint64_t getCreatedByGuid() const;
    void setCreatedByGuid(uint64_t guid);

    uint64_t getTargetGuid() const;
    void setTargetGuid(uint64_t guid);

    uint64_t getTransGuid();

    uint64_t getChannelObjectGuid() const;
    void setChannelObjectGuid(uint64_t guid);

    uint32_t getChannelSpellId() const;
    void setChannelSpellId(uint32_t spell_id);

    //bytes_0 begin
    uint32_t getBytes0() const;
    void setBytes0(uint32_t bytes);

    uint8_t getBytes0ByOffset(uint32_t offset) const;
    void setBytes0ForOffset(uint32_t offset, uint8_t value);

    uint8_t getRace() const;
    void setRace(uint8_t race);
    uint32_t getRaceMask() const { return 1 << (getRace() - 1); }

    uint8_t getClass() const;
    void setClass(uint8_t class_);
    uint32_t getClassMask() const { return 1 << (getClass() - 1); }

    uint8_t getGender() const;
    void setGender(uint8_t gender);

    PowerType getPowerType() const;
    void setPowerType(uint8_t powerType);
    //bytes_0 end

    uint32_t getHealth() const;
    void setHealth(uint32_t health);
    void modHealth(int32_t health);

    uint32_t getPower(PowerType type, bool inRealTime = true) const;
    void setPower(PowerType type, uint32_t value, bool sendPacket = true);
    void modPower(PowerType type, int32_t value);

    uint32_t getMaxHealth() const;
    void setMaxHealth(uint32_t maxHealth);
    void modMaxHealth(int32_t maxHealth);

    uint32_t getMaxPower(PowerType type) const;
    void setMaxPower(PowerType type, uint32_t value);
    void modMaxPower(PowerType type, int32_t value);

#if VERSION_STRING >= WotLK
    float getPowerRegeneration(PowerType type) const;
    void setPowerRegeneration(PowerType type, float value);
    float getManaRegeneration() const;
    void setManaRegeneration(float value);

    // In cata+ these mean 'while in combat'
    float getPowerRegenerationWhileCasting(PowerType type) const;
    void setPowerRegenerationWhileCasting(PowerType type, float value);
    float getManaRegenerationWhileCasting() const;
    void setManaRegenerationWhileCasting(float value);
#endif

    uint32_t getLevel() const;
    void setLevel(uint32_t level);

    uint32_t getFactionTemplate() const;
    void setFactionTemplate(uint32_t id);

    uint32_t getVirtualItemSlotId(uint8_t slot) const;
    void setVirtualItemSlotId(uint8_t slot, uint32_t item_id);

#if VERSION_STRING < WotLK
    uint32_t getVirtualItemInfo(uint8_t offset) const;
    void setVirtualItemInfo(uint8_t offset, uint32_t item_info);
#endif

    uint32_t getUnitFlags() const;
    void setUnitFlags(uint32_t unitFlags);
    void addUnitFlags(uint32_t unitFlags);
    void removeUnitFlags(uint32_t unitFlags);
    bool hasUnitFlags(uint32_t unitFlags) const;

#if VERSION_STRING > Classic
    uint32_t getUnitFlags2() const;
    void setUnitFlags2(uint32_t unitFlags2);
    void addUnitFlags2(uint32_t unitFlags2);
    void removeUnitFlags2(uint32_t unitFlags2);
    bool hasUnitFlags2(uint32_t unitFlags2) const;
#endif

#if VERSION_STRING < WotLK
    uint32_t getAura(uint8_t slot) const;
    void setAura(Aura const* aur, bool apply);

    uint32_t getAuraFlags(uint8_t slot) const;
    void setAuraFlags(Aura const* aur, bool apply);

    uint32_t getAuraLevel(uint8_t slot) const;
    void setAuraLevel(Aura* aur);

    uint32_t getAuraApplication(uint8_t slot) const;
    void setAuraApplication(Aura const* aur);
#endif

    uint32_t getAuraState() const;
    void setAuraState(uint32_t state);
    void addAuraState(uint32_t state);
    void removeAuraState(uint32_t state);

    uint32_t getBaseAttackTime(uint8_t slot) const;
    void setBaseAttackTime(uint8_t slot, uint32_t time);
    void modBaseAttackTime(uint8_t slot, int32_t modTime);

    float getBoundingRadius() const;
    void setBoundingRadius(float radius);

    float getCombatReach() const;
    void setCombatReach(float radius);

    uint32_t getDisplayId() const;
    void setDisplayId(uint32_t id);

    uint32_t getNativeDisplayId() const;
    void setNativeDisplayId(uint32_t id);

    uint32_t getMountDisplayId() const;
    void setMountDisplayId(uint32_t id);

    float getMinDamage() const;
    void setMinDamage(float damage);

    float getMaxDamage() const;
    void setMaxDamage(float damage);

    float getMinOffhandDamage() const;
    void setMinOffhandDamage(float damage);

    float getMaxOffhandDamage() const;
    void setMaxOffhandDamage(float damage);

    //bytes_1 begin
    uint32_t getBytes1() const;
    void setBytes1(uint32_t bytes);

    uint8_t getBytes1ByOffset(uint32_t offset) const;
    void setBytes1ForOffset(uint32_t offset, uint8_t value);

    uint8_t getStandState() const;
    void setStandState(uint8_t standState);

    uint8_t getPetTalentPoints() const;
    void setPetTalentPoints(uint8_t talentPoints);

    uint8_t getStandStateFlags() const;
    void setStandStateFlags(uint8_t standStateFlags);

    uint8_t getAnimationFlags() const;
    void setAnimationFlags(uint8_t animationFlags);
    //bytes_1 end

    uint32_t getPetNumber() const;
    void setPetNumber(uint32_t timestamp);

    uint32_t getPetNameTimestamp() const;
    void setPetNameTimestamp(uint32_t timestamp);

    uint32_t getPetExperience() const;
    void setPetExperience(uint32_t experience);

    uint32_t getPetNextLevelExperience() const;
    void setPetNextLevelExperience(uint32_t experience);

    uint32_t getDynamicFlags() const;
    void setDynamicFlags(uint32_t dynamicFlags);
    void addDynamicFlags(uint32_t dynamicFlags);
    void removeDynamicFlags(uint32_t dynamicFlags);

    float getModCastSpeed() const;
    void setModCastSpeed(float modifier);
    void modModCastSpeed(float modifier);

    uint32_t getCreatedBySpellId() const;
    void setCreatedBySpellId(uint32_t id);

    uint32_t getNpcFlags() const;
    void setNpcFlags(uint32_t npcFlags);
    void addNpcFlags(uint32_t npcFlags);
    void removeNpcFlags(uint32_t npcFlags);

    uint32_t getEmoteState() const;
    void setEmoteState(uint32_t id);

    uint32_t getStat(uint8_t stat) const;
    void setStat(uint8_t stat, uint32_t value);

#if VERSION_STRING > Classic
    uint32_t getPosStat(uint8_t stat) const;
    void setPosStat(uint8_t stat, uint32_t value);

    uint32_t getNegStat(uint8_t stat) const;
    void setNegStat(uint8_t stat, uint32_t value);
#endif

    uint32_t getResistance(uint8_t type) const;
    void setResistance(uint8_t type, uint32_t value);

#if VERSION_STRING > Classic
    uint32_t getResistanceBuffModPositive(uint8_t type) const;
    void setResistanceBuffModPositive(uint8_t type, uint32_t value);

    uint32_t getResistanceBuffModNegative(uint8_t type) const;
    void setResistanceBuffModNegative(uint8_t type, uint32_t value);
#endif

    uint32_t getBaseMana() const;
    void setBaseMana(uint32_t baseMana);

    uint32_t getBaseHealth() const;
    void setBaseHealth(uint32_t baseHealth);

    //byte_2 begin
    uint32_t getBytes2() const;
    void setBytes2(uint32_t bytes);

    uint8_t getBytes2ByOffset(uint32_t offset) const;
    void setBytes2ForOffset(uint32_t offset, uint8_t value);

    uint8_t getSheathType() const;
    void setSheathType(uint8_t sheathType);

    uint8_t getPvpFlags() const;
    void setPvpFlags(uint8_t pvpFlags);

    uint8_t getPetFlags() const;
    void setPetFlags(uint8_t petFlags);

    uint8_t getShapeShiftForm() const;
    void setShapeShiftForm(uint8_t shapeShiftForm);
    uint32_t getShapeShiftMask() const { return 1 << (getShapeShiftForm() - 1); }
    //bytes_2 end

    uint32_t getAttackPower() const;
    void setAttackPower(uint32_t value);

    int32_t getRangedAttackPower() const;
    void setRangedAttackPower(int32_t power);

    float getMinRangedDamage() const;
    void setMinRangedDamage(float damage);

    float getMaxRangedDamage() const;
    void setMaxRangedDamage(float damage);

    uint32_t getPowerCostModifier(uint16_t school) const;
    void setPowerCostModifier(uint16_t school, uint32_t modifier);
    void modPowerCostModifier(uint16_t school, int32_t modifier);

    float getPowerCostMultiplier(uint16_t school) const;
    void setPowerCostMultiplier(uint16_t school, float multiplier);
    void modPowerCostMultiplier(uint16_t school, float multiplier);

    int32_t getAttackPowerMods() const;
    void setAttackPowerMods(int32_t modifier);
    void modAttackPowerMods(int32_t modifier);

    float getAttackPowerMultiplier() const;
    void setAttackPowerMultiplier(float multiplier);
    void modAttackPowerMultiplier(float multiplier);

    int32_t getRangedAttackPowerMods() const;
    void setRangedAttackPowerMods(int32_t modifier);
    void modRangedAttackPowerMods(int32_t modifier);

    float getRangedAttackPowerMultiplier() const;
    void setRangedAttackPowerMultiplier(float multiplier);
    void modRangedAttackPowerMultiplier(float multiplier);

#if VERSION_STRING >= WotLK
    float getHoverHeight() const;
    void setHoverHeight(float height);
#endif
    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement
private:
    int32_t m_rootCounter = 0;

public:
    void setInFront(Object const* target);
    void setFacingTo(float const ori, bool force = true);
    void setFacingToObject(Object* object, bool force = true);
    void setMoveWaterWalk();
    void setMoveLandWalk();
    void setMoveFeatherFall();
    void setMoveNormalFall();
    void setMoveHover(bool set_hover);
    void setMoveCanFly(bool set_fly);
    void setMoveRoot(bool set_root);
    bool isRooted() const;
    bool isMoving() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_MOVING_MASK); }
    bool isTurning() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_TURNING_MASK); }
    bool IsFlying() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_FLYING_MASK); }
    bool IsFalling() const;
    virtual bool canSwim();
    virtual bool isInWater() const;
    bool isUnderWater() const;
    bool isInAccessiblePlaceFor(Creature* c) const;

    uint64_t getCharmerOrOwnerGUID() const override { return isCharmed() ? getCharmedByGuid() : getSummonedByGuid(); }
    bool isCharmed() const { return !getCharmedByGuid(); }

    void setControlled(bool apply, UnitStates state);
    void applyControlStatesIfNeeded();

    virtual bool canFly();

    bool isWalking() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_WALK); }
    bool isHovering() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_HOVER); }

    bool isInCombat() const { return hasUnitFlags(UNIT_FLAG_COMBAT); }
    bool isInEvadeMode() { return hasUnitStateFlag(UNIT_STATE_EVADING); }

    bool isWithinCombatRange(Unit* obj, float dist2compare);
    bool isWithinMeleeRange(Unit* obj) { return isWithinMeleeRangeAt(GetPosition(), obj); }
    bool isWithinMeleeRangeAt(LocationVector const& pos, Unit* obj);
    float getMeleeRange(Unit* target);

    void setMoveSwim(bool set_swim);
    void setMoveDisableGravity(bool disable_gravity);
    void setMoveWalk(bool set_walk);
    void handleFall(MovementInfo const& movementInfo);
 
    // Speed
    UnitSpeedInfo const* getSpeedInfo() const { return &m_UnitSpeedInfo; }
    float getSpeedRate(UnitSpeedType type, bool current) const;
    void resetCurrentSpeeds();
    UnitSpeedType getFastestSpeedType() const;

    void propagateSpeedChange();
    void setSpeedRate(UnitSpeedType mtype, float rate, bool current);

    uint8_t m_forced_speed_changes[MAX_SPEED_TYPE] = {0};

    // Movement info
    MovementNew::MoveSpline* movespline;

    void followerAdded(AbstractFollower* f) { m_followingMe.insert(f); }
    void followerRemoved(AbstractFollower* f) { m_followingMe.erase(f); }
    void removeAllFollowers();
    virtual float getFollowAngle() const { return static_cast<float>(M_PI / 2); }

    MovementManager* getMovementManager() { return i_movementManager; }
    MovementManager const* getMovementManager() const { return i_movementManager; }

    void stopMoving();
    void pauseMovement(uint32_t timer = 0, uint8_t slot = 0, bool forced = true); // timer in ms
    void resumeMovement(uint32_t timer = 0, uint8_t slot = 0); // timer in ms

private:
    std::unordered_set<AbstractFollower*> m_followingMe;

protected:
    MovementManager* i_movementManager;

    void setFeared(bool apply);
    void setConfused(bool apply);
    void setStunned(bool apply);

private:
    UnitSpeedInfo m_UnitSpeedInfo;
    void updateSplineMovement(uint32 t_diff);
    void updateSplinePosition();

    int32_t m_splineSyncTimer = 5000;

public:
    void sendMoveSplinePaket(UnitSpeedType speed_type);
    void disableSpline();
    bool isSplineEnabled() const;

    virtual MovementGeneratorType getDefaultMovementType() const;

    // Mover
    Unit* mControledUnit = this;
    Player* mPlayerControler = nullptr;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Internal States
private:
    uint32_t m_unitState = 0;

public:
    void addUnitStateFlag(uint32_t state_flag) { m_unitState |= state_flag; };
    bool hasUnitStateFlag(uint32_t state_flag) { return (m_unitState & state_flag ? true : false); }
    void removeUnitStateFlag(uint32_t state_flag) { m_unitState &= ~state_flag; };
    uint32_t getUnitStateFlags() { return m_unitState; };


    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells
    void playSpellVisual(uint32_t visual_id, uint32_t type);
    void applyDiminishingReturnTimer(uint32_t* duration, SpellInfo const* spell);
    void removeDiminishingReturnTimer(SpellInfo const* spell);

    bool canDualWield() const;
    void setDualWield(bool enable);

    void castSpell(uint64_t targetGuid, uint32_t spellId, bool triggered);
    void castSpell(Unit* target, uint32_t spellId, bool triggered);
    void castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, bool triggered);
    void castSpell(Unit* target, SpellInfo const* spellInfo, bool triggered);
    void castSpell(uint64_t targetGuid, uint32_t spellId, uint32_t forcedBasepoints, bool triggered);
    void castSpell(Unit* target, uint32_t spellId, uint32_t forcedBasePoints, bool triggered);
    void castSpell(Unit* target, SpellInfo const* spellInfo, uint32_t forcedBasePoints, int32_t spellCharges, bool triggered);
    void castSpellLoc(const LocationVector location, uint32_t spellId, bool triggered);
    void castSpellLoc(const LocationVector location, SpellInfo const* spellInfo, bool triggered);
    void eventCastSpell(Unit* target, SpellInfo const* spellInfo);

    void castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, uint32_t forcedBasepoints, bool triggered);
    void castSpell(Unit* target, SpellInfo const* spellInfo, uint32_t forcedBasepoints, bool triggered);

    SpellProc* addProcTriggerSpell(uint32_t spellId, uint32_t originalSpellId, uint64_t casterGuid, uint32_t procChance, SpellProcFlags procFlags, SpellExtraProcFlags exProcFlags, uint32_t const* spellFamilyMask, uint32_t const* procClassMask = nullptr, Aura* createdByAura = nullptr, Object* obj = nullptr);
    // Gets proc chance and proc flags from spellInfo
    SpellProc* addProcTriggerSpell(SpellInfo const* spellInfo, uint64_t casterGuid, Aura* createdByAura = nullptr, uint32_t const* procClassMask = nullptr, Object* obj = nullptr);
    // Gets proc chance and proc flags from aura
    SpellProc* addProcTriggerSpell(SpellInfo const* spellInfo, Aura* createdByAura, uint64_t casterGuid, uint32_t const* procClassMask = nullptr, Object* obj = nullptr);
    // Uses entered proc chance and proc flags
    SpellProc* addProcTriggerSpell(SpellInfo const* spellInfo, SpellInfo const* originalSpellInfo, uint64_t casterGuid, uint32_t procChance, uint32_t procFlags, uint32_t const* procClassMask = nullptr, Aura* createdByAura = nullptr, Object* obj = nullptr);
    SpellProc* addProcTriggerSpell(SpellInfo const* spellInfo, SpellInfo const* originalSpellInfo, uint64_t casterGuid, uint32_t procChance, SpellProcFlags procFlags, SpellExtraProcFlags exProcFlags, uint32_t const* spellFamilyMask, uint32_t const* procClassMask = nullptr, Aura* createdByAura = nullptr, Object* obj = nullptr);
    SpellProc* getProcTriggerSpell(uint32_t spellId, uint64_t casterGuid) const;
    void removeProcTriggerSpell(uint32_t spellId, uint64_t casterGuid = 0, uint64_t misc = 0);
    void clearProcCooldowns();

    float_t applySpellDamageBonus(SpellInfo const* spellInfo, int32_t baseDmg, float_t effectPctModifier = 1.0f, bool isPeriodic = false, Spell* castingSpell = nullptr, Aura* aur = nullptr);
    float_t applySpellHealingBonus(SpellInfo const* spellInfo, int32_t baseHeal, float_t effectPctModifier = 1.0f, bool isPeriodic = false, Spell* castingSpell = nullptr, Aura* aur = nullptr);

    float_t getCriticalChanceForDamageSpell(Spell* spell, Aura* aura, Unit* target);
    float_t getCriticalChanceForHealSpell(Spell* spell, Aura* aura, Unit* target);
    bool isCriticalDamageForSpell(Object* target, Spell* spell);
    bool isCriticalHealForSpell(Object* target, Spell* spell);
    float_t getCriticalDamageBonusForSpell(float_t damage, Unit* target, Spell* spell, Aura* aura);
    float_t getCriticalHealBonusForSpell(float_t heal, Spell* spell, Aura* aura);

    void sendSpellNonMeleeDamageLog(Object* caster, Object* target, SpellInfo const* spellInfo, uint32_t damage, uint32_t absorbedDamage, uint32_t resistedDamage, uint32_t blockedDamage, uint32_t overKill, bool isPeriodicDamage, bool isCriticalHit);
    void sendSpellHealLog(Object* caster, Object* target, uint32_t spellId, uint32_t healAmount, bool isCritical, uint32_t overHeal, uint32_t absorbedHeal);
    // Sends packet for damage immune
    void sendSpellOrDamageImmune(uint64_t casterGuid, Unit* target, uint32_t spellId);
    void sendAttackerStateUpdate(const WoWGuid& attackerGuid, const WoWGuid& victimGuid, HitStatus hitStatus, uint32_t damage, uint32_t overKill, DamageInfo damageInfo, uint32_t absorbedDamage, VisualState visualState, uint32_t blockedDamage, uint32_t rageGain);

    void addSpellModifier(AuraEffectModifier const* aurEff, bool apply);
    template <typename T> void applySpellModifiers(SpellModifierType modType, T* value, SpellInfo const* spellInfo, Spell* castingSpell = nullptr, Aura* castingAura = nullptr);
    template <typename T> void getTotalSpellModifiers(SpellModifierType modType, T baseValue, int32_t* flatMod, int32_t* pctMod, SpellInfo const* spellInfo, Spell* castingSpell = nullptr, Aura* castingAura = nullptr, bool checkOnly = false);

private:
    bool m_canDualWield = false;

    std::list<SpellProc*> m_procSpells;

    std::list<AuraEffectModifier const*> m_spellModifiers[MAX_SPELLMOD_TYPE];

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Aura
    void addAura(Aura* aur);
    uint8_t findVisualSlotForAura(bool isPositive) const;

    Aura* getAuraWithId(uint32_t spell_id);
    Aura* getAuraWithId(uint32_t* auraId);
    Aura* getAuraWithIdForGuid(uint32_t* auraId, uint64 guid);
    Aura* getAuraWithIdForGuid(uint32_t spell_id, uint64_t target_guid);
    Aura* getAuraWithAuraEffect(AuraEffect aura_effect);

    bool hasAurasWithId(uint32_t auraId);
    bool hasAurasWithId(uint32_t* auraId);
    bool hasAuraWithAuraEffect(AuraEffect type) const;
    bool hasAuraState(AuraState state, SpellInfo const* spellInfo = nullptr, Unit const* caster = nullptr) const;

    void addAuraStateAndAuras(AuraState state);
    void removeAuraStateAndAuras(AuraState state);

    uint32_t getAuraCountForId(uint32_t auraId);

    void removeAllAurasById(uint32_t auraId);
    void removeAllAurasById(uint32_t* auraId);
    void removeAllAurasByIdForGuid(uint32_t auraId, uint64_t guid);
    uint32_t removeAllAurasByIdReturnCount(uint32_t auraId);
    // Can remove only the effect from aura, or (by default) entire aura
    void removeAllAurasByAuraEffect(AuraEffect effect, uint32_t skipSpell = 0, bool removeOnlyEffect = false);

    uint64_t getSingleTargetGuidForAura(uint32_t spellId);
    uint64_t getSingleTargetGuidForAura(uint32_t* spellIds, uint32_t* index);

    void setSingleTargetGuidForAura(uint32_t spellId, uint64_t guid);
    void removeSingleTargetGuidForAura(uint32_t spellId);

    uint32_t getTransformAura() const;
    void setTransformAura(uint32_t auraId);

    // Sends packet for new or removed aura
    void sendAuraUpdate(Aura* aur, bool remove);
    void sendFullAuraUpdate();
    // Sends packet for periodic aura log
    // Returns true if packet could be sent
    bool sendPeriodicAuraLog(const WoWGuid& casterGuid, const WoWGuid& targetGuid, SpellInfo const* spellInfo, uint32_t amount, uint32_t overKillOrOverHeal, uint32_t absorbed, uint32_t resisted, AuraEffect auraEffect, bool isCritical, uint32_t miscValue = 0, float gainMultiplier = 0.0f);

private:
    void _updateAuras(unsigned long diff);

    uint32_t m_transformAura = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Visibility system
    bool canSee(Object* const obj);

    // Stealth
    int32_t getStealthLevel(StealthFlag flag) const;
    int32_t getStealthDetection(StealthFlag flag) const;
    void modStealthLevel(StealthFlag flag, const int32_t amount);
    void modStealthDetection(StealthFlag flag, const int32_t amount);
    bool isStealthed() const;

    // Invisibility
    int32_t getInvisibilityLevel(InvisibilityFlag flag) const;
    int32_t getInvisibilityDetection(InvisibilityFlag flag) const;
    void modInvisibilityLevel(InvisibilityFlag flag, const int32_t amount);
    void modInvisibilityDetection(InvisibilityFlag flag, const int32_t amount);
    bool isInvisible() const;

    void setVisible(const bool visible);

 private:
     // Stealth
     int32_t m_stealthLevel[STEALTH_FLAG_TOTAL] = {0};
     int32_t m_stealthDetection[STEALTH_FLAG_TOTAL] = {0};
     // Invisibility
     int32_t m_invisibilityLevel[INVIS_FLAG_TOTAL] = {0};
     int32_t m_invisibilityDetection[INVIS_FLAG_TOTAL] = {0};

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Health and power
    void regenerateHealthAndPowers(uint16_t timePassed);
    void regeneratePower(PowerType type);
    void interruptHealthRegeneration(uint32_t timeInMS);
    bool isHealthRegenerationInterrupted() const;
#if VERSION_STRING < Cata
    void interruptPowerRegeneration(uint32_t timeInMS);
    bool isPowerRegenerationInterrupted() const;
#endif

    void energize(Unit* target, uint32_t spellId, uint32_t amount, PowerType type, bool sendPacket = true);
    void sendSpellEnergizeLog(Unit* target, uint32_t spellId, uint32_t amount, PowerType type);

    uint8_t getHealthPct() const;
    uint8_t getPowerPct(PowerType powerType) const;

    void sendPowerUpdate(bool self);

private:
    // Converts power type to power index
    uint8_t getPowerIndexFromDBC(PowerType type) const;

    uint32_t m_healthRegenerationInterruptTime = 0;
#if VERSION_STRING < Cata
    // Five second mana regeneration interrupt timer
    uint32_t m_powerRegenerationInterruptTime = 0;
#endif

    // The leftover power from power regeneration which will be added to new value on next power update
    float_t m_powerFractions[TOTAL_PLAYER_POWER_TYPES] = {0};

#if VERSION_STRING >= WotLK
    // Powers in real time
    uint32_t m_manaAmount = 0;
    uint32_t m_rageAmount = 0;
    uint32_t m_focusAmount = 0;
    uint32_t m_energyAmount = 0;
    uint32_t m_runicPowerAmount = 0;

    uint32_t m_powerUpdatePacketTime = REGENERATION_PACKET_UPDATE_INTERVAL;
#endif

protected:
    uint16_t m_healthRegenerateTimer = 0;
    // Mana and Energy
    uint16_t m_manaEnergyRegenerateTimer = 0;
    uint16_t m_focusRegenerateTimer = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Chat
    std::unique_ptr<WorldPacket> createChatPacket(uint8_t type, uint32_t language, std::string msg, Unit* receiver = nullptr, uint32_t sessionLanguage = 0);
    void sendChatMessage(uint8_t type, uint32_t language, std::string msg, Unit* receiver = nullptr, uint32_t sessionLanguage = 0);
    void sendChatMessage(uint8_t type, uint32_t language, std::string msg, uint32_t delay);
    void sendChatMessage(MySQLStructure::NpcScriptText const* text, uint32_t delay, Unit* target = nullptr);

    void sendChatMessageToPlayer(uint8_t type, uint32_t language, std::string msg, Player* plr);

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    void setAttackTimer(WeaponDamageType type, int32_t time);
    uint32_t getAttackTimer(WeaponDamageType type) const;
    bool isAttackReady(WeaponDamageType type) const;
    void resetAttackTimer(WeaponDamageType type);
    void modAttackSpeedModifier(WeaponDamageType type, int32_t amount);
    float getAttackSpeedModifier(WeaponDamageType type) const;

    void sendEnvironmentalDamageLogPacket(uint64_t guid, uint8_t type, uint32_t damage, uint64_t unk = 0);

    virtual bool isPvpFlagSet();
    virtual void setPvpFlag();
    virtual void removePvpFlag();

    virtual bool isFfaPvpFlagSet();
    virtual void setFfaPvpFlag();
    virtual void removeFfaPvpFlag();

    virtual bool isSanctuaryFlagSet();
    virtual void setSanctuaryFlag();
    virtual void removeSanctuaryFlag();

    void restoreDisplayId();

    bool isSitting() const;
    void emote(EmoteType emote);
    void eventAddEmote(EmoteType emote, uint32 time);
    void emoteExpire();
    uint32_t getOldEmote() const;

    void dealDamage(Unit* victim, uint32_t damage, uint32_t spellId, bool removeAuras = true);
    void takeDamage(Unit* attacker, uint32_t damage, uint32_t spellId);
    // Quick method to create a simple damaging health batch event
    void addSimpleDamageBatchEvent(uint32_t damage, Unit* attacker = nullptr, SpellInfo const* spellInfo = nullptr);
    // Quick method to create a simple environmental damage health batch event
    void addSimpleEnvironmentalDamageBatchEvent(EnviromentalDamage type, uint32_t damage, uint32_t absorbedDamage = 0);
    // Quick method to create a simple healing health batch event
    void addSimpleHealingBatchEvent(uint32_t heal, Unit* healer = nullptr, SpellInfo const* spellInfo = nullptr);
    void addHealthBatchEvent(HealthBatchEvent* batch);
    uint32_t calculateEstimatedOverKillForCombatLog(uint32_t damage) const;
    uint32_t calculateEstimatedOverHealForCombatLog(uint32_t heal) const;
    void clearHealthBatch();
    // For preventing memory corruption
    void clearCasterFromHealthBatch(Unit const* caster);

    // Modifies dmg and returns absorbed amount
    uint32_t absorbDamage(SchoolMask schoolMask, uint32_t* dmg, bool checkOnly = true);

    //\ todo: should this and other tag related variables be under Creature class?
    bool isTaggedByPlayerOrItsGroup(Player* tagger);

private:
    uint32_t m_attackTimer[TOTAL_WEAPON_DAMAGE_TYPES] = {0};
    //\ todo: there seems to be new haste update fields in playerdata in cata, and moved to unitdata in mop
    float m_attackSpeed[TOTAL_WEAPON_DAMAGE_TYPES] = { 1.0f, 1.0f, 1.0f };

    void _updateHealth();
    // Handles some things on each damage event in the batch
    uint32_t _handleBatchDamage(HealthBatchEvent const* batch, uint32_t* rageGenerated);
    // Handles some things on each healing event in the batch
    uint32_t _handleBatchHealing(HealthBatchEvent const* batch, uint32_t* absorbedHeal);
    std::vector<HealthBatchEvent*> m_healthBatch;
    uint16_t m_healthBatchTime = HEALTH_BATCH_INTERVAL;

    uint32_t m_lastSpellUpdateTime = 0;
    uint32_t m_lastSummonUpdateTime = 0;

    uint32_t m_oldEmote = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Death
protected:
    DeathState m_deathState = ALIVE;

public:
    bool isAlive() const;
    bool justDied() const;
    bool isDead() const;
    virtual void setDeathState(DeathState state);
    DeathState getDeathState() const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement
    MovementInfo* getMovementInfo();

    uint32_t getUnitMovementFlags() const;
    void setUnitMovementFlags(uint32_t f);
    void addUnitMovementFlag(uint32_t f);
    void removeUnitMovementFlag(uint32_t f);
    bool hasUnitMovementFlag(uint32_t f) const;

    //\brief: this is not uint16_t on version < wotlk
    uint16_t getExtraUnitMovementFlags() const;
    void addExtraUnitMovementFlag(uint16_t f2);
    bool hasExtraUnitMovementFlag(uint16_t f2) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Summons

    TotemSummon* getTotem(TotemSlots slot) const;

    SummonHandler* getSummonInterface() const;

private:
    SummonHandler* m_summonInterface = nullptr;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Vehicle
protected:
    Vehicle* m_currentVehicle = nullptr;    // The vehicle the unit is attached to
    Vehicle* m_vehicle = nullptr;           // The Unit's own vehicle component

public:
    Vehicle* getCurrentVehicle() const;
    void setCurrentVehicle(Vehicle* vehicle);
    void addPassengerToVehicle(uint64_t vehicleGuid, uint32_t delay);

    Vehicle* getVehicleComponent() const;
    Unit* getVehicleBase();

    virtual void addVehicleComponent(uint32 /*creatureEntry*/, uint32 /*vehicleId*/) {}
    virtual void removeVehicleComponent() {}

    void sendHopOnVehicle(Unit* vehicleOwner, uint32_t seat);
    void sendHopOffVehicle(Unit* vehicleOwner, LocationVector& /*landPosition*/);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Unit Owner

    bool isUnitOwnerInParty(Unit* unit);
    bool isUnitOwnerInRaid(Unit* unit);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Threat Management
public:
    ThreatManager& getThreatManager() { return m_threatManager; }
    ThreatManager const& getThreatManager() const { return m_threatManager; }

    // Do not alter anything below this line
    //////////////////////////////////////////////////////////////////////////////////////////

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

    bool  canReachWithAttack(Unit* pVictim);

    //// Combat
    uint32 GetSpellDidHitResult(Unit* pVictim, uint32 weapon_damage_type, Spell* castingSpell);
    DamageInfo Strike(Unit* pVictim, WeaponDamageType weaponType, SpellInfo const* ability, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool force_crit = false, Spell* castingSpell = nullptr);
    // triggeredFromAura is set if castingSpell has been triggered from aura, not if the proc is triggered from aura
    uint32 HandleProc(uint32 flag, Unit* Victim, SpellInfo const* CastingSpell, DamageInfo damageInfo, bool isSpellTriggered, ProcEvents procEvent = PROC_EVENT_DO_ALL, Aura* triggeredFromAura = nullptr);
    void HandleProcDmgShield(uint32 flag, Unit* attacker);//almost the same as handleproc :P

    void RemoveExtraStrikeTarget(SpellInfo const* spell_info);
    void AddExtraStrikeTarget(SpellInfo const* spell_info, uint32 charges);

    int32 GetAP();
    int32 GetRAP();

    bool IsInInstance();
    void CalculateResistanceReduction(Unit* pVictim, DamageInfo* dmg, SpellInfo const* ability, float ArmorPctReduce);
    void DeMorph();
    uint32 ManaShieldAbsorb(uint32 dmg);
    void smsg_AttackStart(Unit* pVictim);
    void smsg_AttackStop(Unit* pVictim);

    bool IsDazed();
    //this function is used for creatures to get chance to daze for another unit
    float get_chance_to_daze(Unit* target);

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
    void RemoveAllAuraType(uint32 auratype); // ex:to remove morph spells
    bool RemoveAllAurasByMechanic(uint32 MechanicType, uint32 MaxDispel, bool HostileOnly); // Removes all (de)buffs on unit of a specific mechanic type.
    void RemoveAllMovementImpairing();

    void RemoveNegativeAuras();
    // Temporary remove all auras

    bool SetAurDuration(uint32 spellId, Unit* caster, uint32 duration);
    bool SetAurDuration(uint32 spellId, uint32 duration);
    void DropAurasOnDeath();

    bool IsPoisoned();

    void GiveGroupXP(Unit* pVictim, Player* PlayerInGroup);

    void OnDamageTaken();

    uint32 m_addDmgOnce = 0;
    uint32 m_ObjectSlots[4] = {0};
    uint32 m_triggerSpell = 0;
    uint32 m_triggerDamage = 0;
    uint32 m_canMove = 0;
    void Possess(Unit* pTarget, uint32 delay = 0);
    void UnPossess();

    // Spell Effect Variables
    int32 m_silenced = 0;
    bool m_damgeShieldsInUse = false;
#if VERSION_STRING == Cata
    DBC::Structures::MountCapabilityEntry const* getMountCapability(uint32_t mountType);
#endif
    std::list<struct DamageProc> m_damageShields;
    std::list<struct ReflectSpellSchool*> m_reflectSpellSchool;

    void RemoveReflect(uint32 spellid, bool apply);

    DamageSplitTarget* m_damageSplitTarget = nullptr;

    void SetOnMeleeSpell(uint32 spell, uint8 ecn = 0) { m_meleespell = spell; m_meleespell_ecn = ecn; }
    uint32 GetOnMeleeSpell() { return m_meleespell; }
    uint8 GetOnMeleeSpellEcn() { return m_meleespell_ecn; }
    void CastOnMeleeSpell();

    uint32 DoDamageSplitTarget(uint32 res, SchoolMask schoolMask, bool melee_dmg);

    // Spell Crit
    float spellcritperc = 0.0f;

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
    float m_hitfrommeleespell = 0.0f;

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

    Loot loot;
    uint32 SchoolCastPrevent[TOTAL_SPELL_SCHOOLS] = {0};
    int32 MechanicDurationPctMod[28] = {0};

    virtual int32 GetDamageDoneMod(uint16_t /*school*/) { return 0; }
    virtual float GetDamageDonePctMod(uint16_t /*school*/) { return 0; }

    int32 DamageTakenMod[TOTAL_SPELL_SCHOOLS] = {0};
    float DamageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};
    float DamageTakenPctModOnHP35 = 1;
    float CritMeleeDamageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};
    float CritRangedDamageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};
    int32 RangedDamageTaken = 0;
    void CalcDamage();
    float BaseDamage[2] = {0};
    float BaseOffhandDamage[2] = {0};
    float BaseRangedDamage[2] = {0};
    int32 RAPvModifier = 0;
    int32 APvModifier = 0;
    uint64 stalkedby = 0;
    uint32 dispels[10] = {0};
    bool trackStealth = false;
    uint32 MechanicsDispels[32] = {0};
    float MechanicsResistancesPCT[32] = {0};
    float ModDamageTakenByMechPCT[32] = {0};
    int32 DoTPctIncrease[TOTAL_SPELL_SCHOOLS] = {0};
    float AOEDmgMod = 1.0f;
    float m_ignoreArmorPctMaceSpec = 0.0f;
    float m_ignoreArmorPct = 0.0f;

    // Stun Immobilize
    uint32 trigger_on_stun = 0; // bah, warrior talent but this will not get triggered on triggered spells if used on proc so I'm forced to used a special variable
    uint32 trigger_on_stun_chance = 100;
    uint32 trigger_on_stun_victim = 0;
    uint32 trigger_on_stun_chance_victim = 100;

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
    uint32 trigger_on_chill = 0;         //mage "Frostbite" talent chill
    uint32 trigger_on_chill_chance = 100;
    uint32 trigger_on_chill_victim = 0;
    uint32 trigger_on_chill_chance_victim = 100;

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

    void SetFaction(uint32 factionId)
    {
        setFactionTemplate(factionId);
        setServersideFaction();
    }

    void SendChatMessageAlternateEntry(uint32 entry, uint8 type, uint32 lang, const char* msg);
    void RegisterPeriodicChatMessage(uint32 delay, uint32 msgid, std::string message, bool sendnotify);

    void SetHealthPct(uint32 val) { if (val > 0) setHealth(float2int32(val * 0.01f * getMaxHealth())); };

    // In-Range
    virtual void addToInRangeObjects(Object* pObj);
    virtual void onRemoveInRangeObject(Object* pObj);
    void clearInRangeSets();

    uint32 m_CombatUpdateTimer = 0;

    void setcanparry(bool newstatus) { can_parry = newstatus; }

    std::map<uint32, Aura*> tmpAura;

    uint32 BaseResistance[TOTAL_SPELL_SCHOOLS] = {0};        // there are resistances for silence, fear, mechanics ....
    uint32 BaseStats[5] = {0};

    int32 HealDoneMod[TOTAL_SPELL_SCHOOLS] = {0};
    float HealDonePctMod[TOTAL_SPELL_SCHOOLS] = {0};

    int32 HealTakenMod[TOTAL_SPELL_SCHOOLS] = {0};
    float HealTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};

    uint32 SchoolImmunityList[TOTAL_SPELL_SCHOOLS] = {0};
    float SpellCritChanceSchool[TOTAL_SPELL_SCHOOLS] = {0};

    float PowerCostPctMod[TOTAL_SPELL_SCHOOLS] = {0};        // armor penetration & spell penetration

    int32 AttackerCritChanceMod[TOTAL_SPELL_SCHOOLS] = {0};
    uint32 SpellDelayResist[TOTAL_SPELL_SCHOOLS] = {0};

    int32 CreatureAttackPowerMod[12] = {0};
    int32 CreatureRangedAttackPowerMod[12] = {0};

    int32 PctRegenModifier = 0;
    // SPELL_AURA_MOD_POWER_REGEN_PERCENT
    float PctPowerRegenModifier[TOTAL_PLAYER_POWER_TYPES];

    // Auras Modifiers
    int32 m_pacified = 0;
    int32 m_interruptRegen = 0;
    int32 m_resistChance = 0;
    int32 m_powerRegenPCT = 0;
    int32 m_stunned = 0;
    int32 m_extraattacks = 0;
    bool m_extrastriketarget = false;
    int32 m_extrastriketargetc = 0;
    std::list<ExtraStrike*> m_extraStrikeTargets;
    int32 m_fearmodifiers = 0;
    int64 m_magnetcaster = 0;   // Unit who acts as a magnet for this unit

    //Combat Mod Results:
    int32 m_CombatResult_Dodge = 0;
    int32 m_CombatResult_Parry = 0; // is not implented yet

                                // aurastate counters
    int8 asc_frozen = 0;
    int8 asc_enraged = 0;
    int8 asc_seal = 0;
    int8 asc_bleed = 0;

    uint16 m_noInterrupt = 0;
    bool disarmed = false;
    uint64 m_detectRangeGUID[5] = {0};
    int32  m_detectRangeMOD[5] = {0};

    // Affect Speed
    int32 m_speedModifier = 0;
    int32 m_slowdown = 0;
    float m_maxSpeed = 0;
    std::map< uint32, int32 > speedReductionMap;
    bool GetSpeedDecrease();
    int32 m_mountedspeedModifier = 0;
    int32 m_flyspeedModifier = 0;

    void UpdateSpeed();

    bool m_can_stealth = true;

    Aura* m_auras[MAX_TOTAL_AURAS_END] = {nullptr};

    int32 m_modlanguage = -1;

    uint32 GetCharmTempVal() { return m_charmtemp; }
    void SetCharmTempVal(uint32 val) { m_charmtemp = val; }

    void DisableAI() { m_useAI = false; }
    void EnableAI() { m_useAI = true; }

    void Phase(uint8 command = PHASE_SET, uint32 newphase = 1);

    bool Tagged = false;
    uint64 TaggerGuid = 0;
    void Tag(uint64 TaggerGUID);
    void UnTag();
    bool IsTagged();
    bool IsTaggable();
    uint64 GetTaggerGUID();
    bool isLootable();

    virtual bool isTrainingDummy() { return false; }

    void SetFacing(float newo);     //only working if creature is idle

    AuraCheckResponse AuraCheck(SpellInfo const* proto, Object* caster = nullptr);
    AuraCheckResponse AuraCheck(SpellInfo const* proto, Aura* aur, Object* caster = nullptr);

    uint16 m_diminishCount[DIMINISHING_GROUP_COUNT] = {0};
    uint8 m_diminishAuraCount[DIMINISHING_GROUP_COUNT] = {0};
    uint16 m_diminishTimer[DIMINISHING_GROUP_COUNT] = {0};
    bool m_diminishActive = false;

    void SetDiminishTimer(uint32 index)
    {
        m_diminishTimer[index] = 15000;
    }

    DynamicObject* dynObj = nullptr;

    uint32 m_auravisuals[MAX_NEGATIVE_VISUAL_AURAS_END] = {0};

    bool bProcInUse = false;
    bool bInvincible = false;
    Player* m_redirectSpellPackets = nullptr;
    void UpdateVisibility();

    struct
    {
        int32 amt = 0;
        int32 max = 0;
    } m_soulSiphon;

    uint32 m_cTimer = 0;
    void EventUpdateFlag();
    CombatStatusHandler CombatStatus;
    bool m_temp_summon = false;

    void DispelAll(bool positive);

    void EventModelChange();
    inline float GetModelHalfSize() { return m_modelhalfsize * getScale(); }

    void RemoveFieldSummon();

    float GetBlockFromSpell() { return m_blockfromspell; }
    float GetParryFromSpell() { return m_parryfromspell; }
    float GetDodgeFromSpell() { return m_dodgefromspell; }
    void SetBlockFromSpell(float value) { m_blockfromspell = value; }
    void SetParryFromSpell(float value) { m_parryfromspell = value; }
    void SetDodgeFromSpell(float value) { m_dodgefromspell = value; }

    void AggroPvPGuards();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void Die(Unit* pAttacker, uint32 damage, uint32 spellid);
    virtual bool isCritter() { return false; }

    void knockbackFrom(float x, float y, float speedXY, float speedZ);
    void jumpTo(float speedXY, float speedZ, bool forward = true, Optional<LocationVector> dest = {});
    void jumpTo(Object* obj, float speedZ, bool withOrientation = false);
    virtual void HandleKnockback(Object* caster, float horizontal, float vertical);

    void AddGarbagePet(Pet* pet);

    virtual void BuildPetSpellList(WorldPacket & data);

    uint64 GetAuraUpdateMaskForRaid() const { return m_auraRaidUpdateMask; }
    void ResetAuraUpdateMaskForRaid() { m_auraRaidUpdateMask = 0; }
    void SetAuraUpdateMaskForRaid(uint8 slot) { m_auraRaidUpdateMask |= (uint64(1) << slot); }
    void UpdateAuraForGroup(uint8 slot);

protected:
    Unit();
    void RemoveGarbage();
    void AddGarbageAura(Aura* aur);

    uint32 m_meleespell = 0;
    uint8 m_meleespell_ecn = 0;         // extra_cast_number

    std::list<Aura*> m_GarbageAuras;
    std::list<Pet*> m_GarbagePets;

    // DK:pet

    // AI
    AIInterface* m_aiInterface;
    bool m_useAI = false;
    bool can_parry = false;         //will be enabled by block spell
    int32 m_threatModifyer = 0;
    int32 m_generatedThreatModifyer[TOTAL_SPELL_SCHOOLS] = {0};

    int32 m_manashieldamt = 0;
    uint32 m_manaShieldId = 0;

    // Some auras can only be cast on one target at a time
    // This will map aura spell id to target guid
    UniqueAuraTargetMap m_singleTargetAura;

    uint32 m_charmtemp = 0;

    bool m_extraAttackCounter = false;

    float m_modelhalfsize = 1.0f;      // used to calculate if something is in range of this unit

    float m_blockfromspell = 0.0f;
    float m_dodgefromspell = 0.0f;
    float m_parryfromspell = 0.0f;
    uint32 m_BlockModPct = 0;       // is % but does not need float and does not need /100!

    
    uint64 m_auraRaidUpdateMask = 0;

public:
    const CombatStatusHandler* getcombatstatus() const { return &CombatStatus; }

    bool m_noFallDamage = false;
    float z_axisposition = 0.0f;
    int32 m_safeFall = 0;

    void BuildMovementPacket(ByteBuffer* data);
    void BuildMovementPacket(ByteBuffer* data, float x, float y, float z, float o);

    // AGPL End
};