/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CombatHandler.hpp"
#include "Management/LootMgr.h"
#include "Macros/UnitMacros.hpp"
#include "Movement/AbstractFollower.h"
#include "Objects/Object.h"
#include "Objects/Units/Creatures/Summons/SummonHandler.h"
#include "Spell/Definitions/AuraEffects.hpp"
#include "Spell/Definitions/AuraSlots.hpp"
#include "Spell/Definitions/AuraStates.hpp"
#include "Spell/Definitions/DispelType.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/Definitions/ProcFlags.hpp"
#include "Spell/Definitions/School.hpp"
#include "Spell/Definitions/SpellMechanics.hpp"
#include "Spell/Definitions/SpellModifierType.hpp"
#include "Spell/Definitions/SpellTypes.hpp"
#include "Spell/SpellCastTargets.hpp"
#include "Spell/SpellDefines.hpp"
#include "Spell/SpellProc.hpp"
#include "Storage/MySQLStructures.h"
#include "ThreatHandler.h"
#include "UnitDefines.hpp"

#include <array>
#include <list>
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
struct SpellForcedBasePoints;

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

typedef std::unordered_map<uint32_t, uint64_t> UniqueAuraTargetMap;

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

typedef std::array<Aura*, AuraSlots::TOTAL_SLOT_END> AuraArray;
typedef std::list<AuraEffectModifier const*> AuraEffectList;
typedef std::array<AuraEffectList, TOTAL_SPELL_AURAS> AuraEffectListArray;
typedef std::array<uint32_t /*spellId*/, AuraSlots::NEGATIVE_VISUAL_SLOT_END> VisualAuraArray;

struct WoWUnit;

class SERVER_DECL Unit : public Object
{
protected:
    Unit();

public: //\todo Zyres: public fpr LuaEngine, sort out why
    virtual ~Unit();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Essential functions

    void Update(unsigned long time_passed);             // hides function Object::Update
    // void AddToWorld();                               // not used
    // void AddToWorld(WorldMap* pMapMgr);                // not used
    // void PushToWorld(WorldMap*);                       // not used
    virtual void RemoveFromWorld(bool free_guid);       // hides virtual function Object::RemoveFromWorld
    // void OnPrePushToWorld();                         // not used
    virtual void OnPushToWorld();                       // hides virtual function Object::OnPushToWorld
    // void OnPreRemoveFromWorld();                     // not used
    // void OnRemoveFromWorld();                        // not used

private:
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    const WoWUnit* unitData() const { return reinterpret_cast<WoWUnit*>(wow_data); }

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

    // helper
    bool isCharmed() const { return !getCharmedByGuid(); }

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
    inline void setFullHealth() { setHealth(getMaxHealth()); }

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

    // helper
    void setFaction(uint32_t factionId)
    {
        setFactionTemplate(factionId);
        setServersideFaction();
    }

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

    // helper
    bool isInCombat() const { return getCombatHandler().isInCombat(); }
    virtual bool canSwim();

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

#if VERSION_STRING < Mop
    uint32_t getDynamicFlags() const;
    void setDynamicFlags(uint32_t dynamicFlags);
    void addDynamicFlags(uint32_t dynamicFlags);
    void removeDynamicFlags(uint32_t dynamicFlags);
    bool hasDynamicFlags(uint32_t dynamicFlags) const;
#endif

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
    void addPvpFlags(uint8_t pvpFlags);
    void removePvpFlags(uint8_t pvpFlags);

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
    // Area/Map/Phase & Position
public:
    void setLocationWithoutUpdate(LocationVector& location);

    virtual void setPhase(uint8_t command = PHASE_SET, uint32_t newPhase = 1);

    bool isWithinCombatRange(Unit* obj, float dist2compare);
    bool isWithinMeleeRange(Unit* obj) { return isWithinMeleeRangeAt(GetPosition(), obj); }
    bool isWithinMeleeRangeAt(LocationVector const& pos, Unit* obj);
    float getMeleeRange(Unit* target);

    bool isInInstance() const;
    virtual bool isInWater() const;
    bool isUnderWater() const;
    bool isInAccessiblePlaceFor(Creature* c) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Combat
public:
    int32_t m_CombatResult_Dodge = 0;
    int32_t m_CombatResult_Parry = 0;

    CombatHandler& getCombatHandler();
    CombatHandler const& getCombatHandler() const;

private:
    CombatHandler m_combatHandler;

    //////////////////////////////////////////////////////////////////////////////////////////
    // MovementInfo (from class Object)
public:
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

    //helpers
    bool isRooted() const;
    bool isMoving() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_MOVING_MASK); }
    bool isTurning() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_TURNING_MASK); }
    bool IsFlying() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_FLYING_MASK); }
    bool IsFalling() const;

    bool isWalking() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_WALK); }
    bool isHovering() const { return obj_movement_info.hasMovementFlag(MOVEFLAG_HOVER); }

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
    void setMoveSwim(bool set_swim);
    void setMoveDisableGravity(bool disable_gravity);
    void setMoveWalk(bool set_walk);
    void setFacing(float newo);     //only working if creature is idle
    void setAnimationTier(AnimationTier  tier);

    //////////////////////////////////////////////////////////////////////////////////////////
    // used for handling fall
    float m_zAxisPosition = 0.0f;
    int32_t m_safeFall = 0;
    bool m_noFallDamage = false;

    void handleFall(MovementInfo const& movementInfo);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Speed
private:
    UnitSpeedInfo m_UnitSpeedInfo;

    //\todo Zyres: guess that should be part of UnitSpeedInfo
    int32_t m_speedModifier = 0;
    int32_t m_mountedspeedModifier = 0;
    int32_t m_flyspeedModifier = 0;
    int32_t m_slowdown = 0;
    float m_maxSpeed = 0;

    std::map<uint32_t, int32_t> speedReductionMap;

public:
    uint8_t m_forced_speed_changes[MAX_SPEED_TYPE] = { 0 };

    UnitSpeedInfo const* getSpeedInfo() const { return &m_UnitSpeedInfo; }
    float getSpeedRate(UnitSpeedType type, bool current) const;
    void resetCurrentSpeeds();
    UnitSpeedType getFastestSpeedType() const;

    void propagateSpeedChange();
    void setSpeedRate(UnitSpeedType mtype, float rate, bool current);

    bool getSpeedDecrease();
    void updateSpeed();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement spline
    MovementNew::MoveSpline* movespline;

    void followerAdded(AbstractFollower* f) { m_followingMe.insert(f); }
    void followerRemoved(AbstractFollower* f) { m_followingMe.erase(f); }
    void removeAllFollowers();
    virtual float getFollowAngle() const { return static_cast<float>(M_PI / 2); }

    MovementManager* getMovementManager() { return i_movementManager; }
    MovementManager const* getMovementManager() const { return i_movementManager; }

    virtual bool canFly();

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
    void updateSplineMovement(uint32_t t_diff);
    void updateSplinePosition();

    int32_t m_splineSyncTimer = 5000;

public:
    void sendMoveSplinePaket(UnitSpeedType speed_type);
    void disableSpline();
    bool isSplineEnabled() const;

    void jumpTo(float speedXY, float speedZ, bool forward = true, Optional<LocationVector> dest = {});
    void jumpTo(Object* obj, float speedZ, bool withOrientation = false);

    virtual void handleKnockback(Object* caster, float horizontal, float vertical);

    virtual MovementGeneratorType getDefaultMovementType() const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Mover
    Unit* mControledUnit = this;
    Player* mPlayerControler = nullptr;  

    //////////////////////////////////////////////////////////////////////////////////////////
    // AI Stuff
protected:
    AIInterface* m_aiInterface;
    bool m_useAI = false;
    uint32_t m_lastAiInterfaceUpdateTime = 0;

public:
    AIInterface* getAIInterface() const { return m_aiInterface; }

    void setAItoUse(bool value) { m_useAI = value; }
    bool isAIEnabled() { return m_useAI; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Internal States
private:
    uint32_t m_unitState = 0;

public:
    void addUnitStateFlag(uint32_t state_flag) { m_unitState |= state_flag; }
    bool hasUnitStateFlag(uint32_t state_flag) const { return (m_unitState & state_flag ? true : false); }
    void removeUnitStateFlag(uint32_t state_flag) { m_unitState &= ~state_flag; }
    uint32_t getUnitStateFlags() { return m_unitState; }

    // helper
    bool isInEvadeMode() const { return hasUnitStateFlag(UNIT_STATE_EVADING); }

    void setControlled(bool apply, UnitStates state);
    void applyControlStatesIfNeeded();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells
    void playSpellVisual(uint32_t visual_id, uint32_t type);
    void applyDiminishingReturnTimer(uint32_t* duration, SpellInfo const* spell);
    void removeDiminishingReturnTimer(SpellInfo const* spell);

    bool canDualWield() const;
    void setDualWield(bool enable);

    void castSpell(uint64_t targetGuid, uint32_t spellId, bool triggered = false);
    void castSpell(Unit* target, uint32_t spellId, bool triggered = false);
    void castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, bool triggered = false);
    void castSpell(Unit* target, SpellInfo const* spellInfo, bool triggered = false);
    void castSpell(uint64_t targetGuid, uint32_t spellId, SpellForcedBasePoints forcedBasepoints, bool triggered = false);
    void castSpell(Unit* target, uint32_t spellId, SpellForcedBasePoints forcedBasePoints, bool triggered = false);
    void castSpell(Unit* target, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasePoints, int32_t spellCharges, bool triggered = false);
    void castSpell(SpellCastTargets targets, uint32_t spellId, bool triggered = false);
    void castSpell(SpellCastTargets targets, SpellInfo const* spellInfo, bool triggered = false);
    void castSpellLoc(const LocationVector location, uint32_t spellId, bool triggered = false);
    void castSpellLoc(const LocationVector location, SpellInfo const* spellInfo, bool triggered = false);
    void eventCastSpell(Unit* target, SpellInfo const* spellInfo);

    void castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasepoints, bool triggered);
    void castSpell(Unit* target, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasepoints, bool triggered);

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

    void addSpellImmunity(SpellImmunityMask immunityMask, bool apply);
    uint32_t getSpellImmunity() const;
    bool hasSpellImmunity(SpellImmunityMask immunityMask) const;

private:
    bool m_canDualWield = false;

    std::list<SpellProc*> m_procSpells;

    std::list<AuraEffectModifier const*> m_spellModifiers[MAX_SPELLMOD_TYPE];

    uint32_t m_spellImmunityMask = SPELL_IMMUNITY_NONE;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Aura
    void addAura(Aura* aur);
    uint8_t findVisualSlotForAura(Aura const* aur) const;

    Aura* getAuraWithId(uint32_t spell_id);
    Aura* getAuraWithId(uint32_t const* auraId);
    Aura* getAuraWithIdForGuid(uint32_t const* auraId, uint64_t guid);
    Aura* getAuraWithIdForGuid(uint32_t spell_id, uint64_t guid);
    Aura* getAuraWithAuraEffect(AuraEffect aura_effect);
    Aura* getAuraWithVisualSlot(uint8_t visualSlot);
    // Note; this is internal serverside aura slot, not the slot in client
    // For clientside slot use getAuraWithVisualSlot
    Aura* getAuraWithAuraSlot(uint16_t auraSlot);

    bool hasAurasWithId(uint32_t auraId) const;
    bool hasAurasWithId(uint32_t const* auraId) const;
    bool hasAuraWithAuraEffect(AuraEffect type) const;
    bool hasAuraWithMechanic(SpellMechanic mechanic) const;
    bool hasAuraWithSpellType(SpellTypes type, uint64_t casterGuid = 0, uint32_t skipSpellId = 0) const;

    bool hasAuraState(AuraState state, SpellInfo const* spellInfo = nullptr, Unit const* caster = nullptr) const;
    void addAuraStateAndAuras(AuraState state);
    void removeAuraStateAndAuras(AuraState state);

    uint32_t getAuraCountForId(uint32_t auraId) const;
    uint32_t getAuraCountForEffect(AuraEffect aura_effect) const;
    uint32_t getAuraCountWithDispelType(DispelType type, uint64_t casterGuid = 0) const;

    void removeAllAuras();
    void removeAllAurasById(uint32_t auraId, AuraRemoveMode mode = AURA_REMOVE_BY_SERVER);
    void removeAllAurasById(uint32_t const* auraId, AuraRemoveMode mode = AURA_REMOVE_BY_SERVER);
    void removeAllAurasByIdForGuid(uint32_t auraId, uint64_t guid, AuraRemoveMode mode = AURA_REMOVE_BY_SERVER);
    void removeAllAurasByAuraInterruptFlag(uint32_t auraInterruptFlag, uint32_t skipSpellId = 0);
    // Can remove only the effect from aura, or (by default) entire aura
    void removeAllAurasByAuraEffect(AuraEffect effect, uint32_t skipSpell = 0, bool removeOnlyEffect = false, uint64_t casterGuid = 0, AuraRemoveMode mode = AURA_REMOVE_BY_SERVER);
    void removeAllAurasBySpellMechanic(SpellMechanic mechanic, bool negativeOnly = true);
    void removeAllAurasBySpellMechanic(SpellMechanic const* mechanic, bool negativeOnly = true);
    void removeAllAurasBySpellType(SpellTypes type, uint64_t casterGuid = 0, uint32_t skipSpellId = 0);
    void removeAllAurasBySchoolMask(SchoolMask schoolMask, bool negativeOnly = true, bool isImmune = false);
    void removeAllNegativeAuras();
    // Does not remove passive auras
    void removeAllPositiveAuras();
    void removeAllNonPersistentAuras();
    void removeAuraByItemGuid(uint32_t auraId, uint64_t itemguid);
    uint32_t removeAllAurasByIdReturnCount(uint32_t auraId, AuraRemoveMode mode = AURA_REMOVE_BY_SERVER);

    uint64_t getSingleTargetGuidForAura(uint32_t spellId);
    uint64_t getSingleTargetGuidForAura(uint32_t const* spellIds, uint32_t* index);
    void setSingleTargetGuidForAura(uint32_t spellId, uint64_t guid);
    void removeSingleTargetGuidForAura(uint32_t spellId);

    void clearAllAreaAuraTargets();
    void removeAllAreaAurasCastedByOther();

    uint32_t getTransformAura() const;
    void setTransformAura(uint32_t auraId);

    // Sends packet for new or removed aura
    void sendAuraUpdate(Aura* aur, bool remove);
    void sendFullAuraUpdate();
    // Sends packet for periodic aura log
    // Returns true if packet could be sent
    bool sendPeriodicAuraLog(const WoWGuid& casterGuid, const WoWGuid& targetGuid, SpellInfo const* spellInfo, uint32_t amount, uint32_t overKillOrOverHeal, uint32_t absorbed, uint32_t resisted, AuraEffect auraEffect, bool isCritical, uint32_t miscValue = 0, float gainMultiplier = 0.0f);

    AuraArray const& getAuraList() const;
    AuraEffectList const& getAuraEffectList(AuraEffect effect) const;
    VisualAuraArray const& getVisualAuraList() const;

private:
    // Inserts aura into aura containers
    void _addAura(Aura* aur);
    // Inserts aura effect into aura effect list
    void _addAuraEffect(AuraEffectModifier const* aurEff);
    // Erases aura from aura containers
    void _removeAura(Aura* aur);
    // Erases aura effect from aura effect list
    void _removeAuraEffect(AuraEffectModifier const* aurEff);
    void _updateAuras(unsigned long diff);

    uint32_t m_transformAura = 0;

    AuraArray m_auraList = { nullptr };
    AuraEffectListArray m_auraEffectList;
    VisualAuraArray m_auraVisualList = { 0 };

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
    uint8_t getPctFromMaxHealth(uint8_t pct) const;
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
    int32_t m_modlanguage = -1;

    std::unique_ptr<WorldPacket> createChatPacket(uint8_t type, uint32_t language, std::string msg, Unit* receiver = nullptr, uint32_t sessionLanguage = 0);
    void sendChatMessage(uint8_t type, uint32_t language, std::string msg, Unit* receiver = nullptr, uint32_t sessionLanguage = 0);
    void sendChatMessage(uint8_t type, uint32_t language, std::string msg, uint32_t delay);
    void sendChatMessage(MySQLStructure::NpcScriptText const* text, uint32_t delay, Unit* target = nullptr);

    void sendChatMessageToPlayer(uint8_t type, uint32_t language, std::string msg, Player* plr);

    void sendChatMessageAlternateEntry(uint32_t entry, uint8_t type, uint32_t lang, std::string msg);

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    void setAttackTimer(WeaponDamageType type, uint32_t time);
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
    void eventAddEmote(EmoteType emote, uint32_t time);
    void emoteExpire();
    uint32_t getOldEmote() const;

    // Note; calling this method will not cause combat or any threat to victim
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

    void smsg_AttackStart(Unit* pVictim);
    void smsg_AttackStop(Unit* pVictim);

    virtual void addToInRangeObjects(Object* pObj);
    virtual void onRemoveInRangeObject(Object* pObj);
    void clearInRangeSets();

    bool setDetectRangeMod(uint64_t guid, int32_t amount);
    void unsetDetectRangeMod(uint64_t guid);
    int32_t getDetectRangeMod(uint64_t guid) const;

    virtual bool isCritter() { return false; }

    void knockbackFrom(float x, float y, float speedXY, float speedZ);

    virtual bool isTrainingDummy() { return false; }

    GameObject* getGameObject(uint32_t spellId) const;
    void addGameObject(GameObject* gameObj);
    void removeGameObject(GameObject* gameObj, bool del);
    void removeGameObject(uint32_t spellid, bool del);
    void removeAllGameObjects();

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

    uint32_t m_oldEmote = 0;

protected:
    typedef std::list<GameObject*> GameObjectList;
    GameObjectList m_gameObj;

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
    // Summons

    TotemSummon* getTotem(SummonSlot slot) const;

    SummonHandler* getSummonInterface() const;

private:
    SummonHandler* m_summonInterface = nullptr;

#ifdef FT_VEHICLES
    //////////////////////////////////////////////////////////////////////////////////////////
    // Vehicle
protected:
    Vehicle* m_vehicle = nullptr;           // The Unit's own vehicle component
    Vehicle* m_vehicleKit = nullptr;        // The vehicle the unit is attached to

public:
    bool createVehicleKit(uint32_t id, uint32_t creatureEntry);
    void removeVehicleKit();
    Vehicle* getVehicleKit() const { return m_vehicleKit; }
    Vehicle* getVehicle() const { return m_vehicle; }
    void setVehicle(Vehicle* vehicle) { m_vehicle = vehicle; }
    bool isOnVehicle(Unit const* vehicle) const;
    Unit* getVehicleBase() const;
    Unit* getVehicleRoot() const;
    Creature* getVehicleCreatureBase() const;
    void handleSpellClick(Unit* clicker, int8_t seatId = -1);
    void callEnterVehicle(Unit* base, int8_t seatId = -1);
    void callExitVehicle(LocationVector const* exitPosition = nullptr);
    void callChangeSeat(int8_t seatId, bool next = true);

    // shouldnt be called directly always use the functions above
    void exitVehicle(LocationVector const* exitPosition = nullptr);
    void enterVehicle(Vehicle* vehicle, int8_t seatId);
#else
public:
    void handleSpellClick(Unit* clicker);
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Unit Owner
public:
    bool isUnitOwnerInParty(Unit* unit);
    bool isUnitOwnerInRaid(Unit* unit);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Threat Management
private:
    friend class ThreatManager;
    ThreatManager m_threatManager = this;

    //\todo: Why do we have these two vars here instead of ThreatManager?
    int32_t m_threatModifyer = 0;
    int32_t m_generatedThreatModifyer[TOTAL_SPELL_SCHOOLS] = { 0 };

public:
    ThreatManager& getThreatManager() { return m_threatManager; }
    ThreatManager const& getThreatManager() const { return m_threatManager; }

    void clearHateList();
    void wipeHateList();
    void wipeTargetList();

    int32_t getThreatModifyer() { return m_threatModifyer; }
    void modThreatModifyer(int32_t mod) { m_threatModifyer += mod; }

    int32_t getGeneratedThreatModifyer(uint32_t school) { return m_generatedThreatModifyer[school]; }
    void modGeneratedThreatModifyer(uint32_t school, int32_t mod) { m_generatedThreatModifyer[school] += mod; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Tagging (helper for dynamic flags)
    //\todo: Zyres: tagging is Creature related, maybe move this to the correct class
private:
    uint64_t m_taggerGuid = 0;

public:
    void setTaggerGuid(uint64_t guid);
    uint64_t getTaggerGuid() const;

    bool isTagged() const;
    bool isTaggable() const;

    bool isTaggedByPlayerOrItsGroup(Player* tagger);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Loot
    //\todo Zyres: you can loot only creatures, maybe this is the wrong place for member
    Loot loot;

    bool isLootable();

    // Do not alter anything below this line
    //////////////////////////////////////////////////////////////////////////////////////////

    // MIT End
    // AGPL Start
public:

    friend class AIInterface;
    friend class Aura;

    virtual void Deactivate(WorldMap* mgr);

    bool  canReachWithAttack(Unit* pVictim);

    //// Combat
    uint32_t GetSpellDidHitResult(Unit* pVictim, uint32_t weapon_damage_type, Spell* castingSpell);
    DamageInfo Strike(Unit* pVictim, WeaponDamageType weaponType, SpellInfo const* ability, int32_t add_damage, int32_t pct_dmg_mod, uint32_t exclusive_damage, bool disable_proc, bool skip_hit_check, bool force_crit = false, Spell* castingSpell = nullptr);
    // triggeredFromAura is set if castingSpell has been triggered from aura, not if the proc is triggered from aura
    uint32_t HandleProc(uint32_t flag, Unit* Victim, SpellInfo const* CastingSpell, DamageInfo damageInfo, bool isSpellTriggered, ProcEvents procEvent = PROC_EVENT_DO_ALL, Aura* triggeredFromAura = nullptr);
    void HandleProcDmgShield(uint32_t flag, Unit* attacker);//almost the same as handleproc :P

    void RemoveExtraStrikeTarget(SpellInfo const* spell_info);
    void AddExtraStrikeTarget(SpellInfo const* spell_info, uint32_t charges);

    int32_t GetAP();
    int32_t GetRAP();

    void CalculateResistanceReduction(Unit* pVictim, DamageInfo* dmg, SpellInfo const* ability, float ArmorPctReduce);
    void DeMorph();
    uint32_t ManaShieldAbsorb(uint32_t dmg);

    bool IsDazed();
    //this function is used for creatures to get chance to daze for another unit
    float get_chance_to_daze(Unit* target);

    //////////////////////////////////////////////////////////////////////////////////////////
    // AURAS
    //////////////////////////////////////////////////////////////////////////////////////////

    //\todo: isn't the aura timed or triggered after healing?
    bool RemoveAurasByHeal();

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Performs the specified action on the auras that meet the specified condition
    /// \param     AuraAction *action        -  The action to perform
    /// \param     AuraCondition *condition  -  The condition that the aura(s) need to meet
    /// \returns true if at least one action was performed, false otherwise.
    //////////////////////////////////////////////////////////////////////////////////////////
    bool AuraActionIf(AuraAction* action, AuraCondition* condition);

    void EventRemoveAura(uint32_t SpellId) { removeAllAurasById(SpellId); }

    bool IsPoisoned();

    void GiveGroupXP(Unit* pVictim, Player* PlayerInGroup);

    void OnDamageTaken();

    uint32_t m_addDmgOnce = 0;
    uint32_t m_ObjectSlots[4] = {0};
    uint32_t m_triggerSpell = 0;
    uint32_t m_triggerDamage = 0;
    uint32_t m_canMove = 0;
    void Possess(Unit* pTarget, uint32_t delay = 0);
    void UnPossess();

    // Spell Effect Variables
    int32_t m_silenced = 0;
    bool m_damgeShieldsInUse = false;
#if VERSION_STRING == Cata
    DBC::Structures::MountCapabilityEntry const* getMountCapability(uint32_t mountType);
#endif
    std::list<struct DamageProc> m_damageShields;
    std::list<struct ReflectSpellSchool*> m_reflectSpellSchool;

    void RemoveReflect(uint32_t spellid, bool apply);

    DamageSplitTarget* m_damageSplitTarget = nullptr;

    void SetOnMeleeSpell(uint32_t spell, uint8_t ecn = 0) { m_meleespell = spell; m_meleespell_ecn = ecn; }
    uint32_t GetOnMeleeSpell() { return m_meleespell; }
    uint8_t GetOnMeleeSpellEcn() { return m_meleespell_ecn; }
    void CastOnMeleeSpell();

    uint32_t DoDamageSplitTarget(uint32_t res, SchoolMask schoolMask, bool melee_dmg);

    // Spell Crit
    float spellcritperc = 0.0f;

    void SetHitFromMeleeSpell(float value) { m_hitfrommeleespell = value; }
    float GetHitFromMeleeSpell() { return m_hitfrommeleespell; }
    float m_hitfrommeleespell = 0.0f;

    // DK:Affect
    //\todo: these local vars can be replaced by proper aura effect handling.
    uint32_t IsPacified() { return m_pacified; }
    uint32_t IsStunned() { return m_stunned; }
    uint32_t IsFeared() { return m_fearmodifiers; }

    uint32_t GetResistChanceMod() { return m_resistChance; }
    void SetResistChanceMod(uint32_t amount) { m_resistChance = amount; }

    uint16_t HasNoInterrupt() { return m_noInterrupt; }

    uint32_t SchoolCastPrevent[TOTAL_SPELL_SCHOOLS] = {0};
    int32_t MechanicDurationPctMod[28] = {0};

    virtual int32_t GetDamageDoneMod(uint16_t /*school*/) { return 0; }
    virtual float GetDamageDonePctMod(uint16_t /*school*/) { return 0; }

    int32_t DamageTakenMod[TOTAL_SPELL_SCHOOLS] = {0};
    float DamageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};
    float DamageTakenPctModOnHP35 = 1;
    float CritMeleeDamageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};
    float CritRangedDamageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};
    int32_t RangedDamageTaken = 0;
    void CalcDamage();
    float BaseDamage[2] = {0};
    float BaseOffhandDamage[2] = {0};
    float BaseRangedDamage[2] = {0};
    int32_t RAPvModifier = 0;
    int32_t APvModifier = 0;
    uint64_t stalkedby = 0;
    uint32_t dispels[10] = {0};
    bool trackStealth = false;
    uint32_t MechanicsDispels[32] = {0};
    float MechanicsResistancesPCT[32] = {0};
    float ModDamageTakenByMechPCT[32] = {0};
    int32_t DoTPctIncrease[TOTAL_SPELL_SCHOOLS] = {0};
    float AOEDmgMod = 1.0f;
    float m_ignoreArmorPctMaceSpec = 0.0f;
    float m_ignoreArmorPct = 0.0f;

    // Stun Immobilize
    uint32_t trigger_on_stun = 0; // bah, warrior talent but this will not get triggered on triggered spells if used on proc so I'm forced to used a special variable
    uint32_t trigger_on_stun_chance = 100;
    uint32_t trigger_on_stun_victim = 0;
    uint32_t trigger_on_stun_chance_victim = 100;

    void SetTriggerStunOrImmobilize(uint32_t newtrigger, uint32_t new_chance, bool is_victim = false)
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
    uint32_t trigger_on_chill = 0;         //mage "Frostbite" talent chill
    uint32_t trigger_on_chill_chance = 100;
    uint32_t trigger_on_chill_victim = 0;
    uint32_t trigger_on_chill_chance_victim = 100;

    void SetTriggerChill(uint32_t newtrigger, uint32_t new_chance, bool is_victim = false)
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

    void SetHealthPct(uint32_t val) { if (val > 0) setHealth(float2int32(val * 0.01f * getMaxHealth())); };

    void setcanparry(bool newstatus) { can_parry = newstatus; }

    std::map<uint32_t, Aura*> tmpAura;

    uint32_t BaseResistance[TOTAL_SPELL_SCHOOLS] = {0};        // there are resistances for silence, fear, mechanics ....
    uint32_t BaseStats[5] = {0};

    int32_t HealDoneMod[TOTAL_SPELL_SCHOOLS] = {0};
    float HealDonePctMod[TOTAL_SPELL_SCHOOLS] = {0};

    int32_t HealTakenMod[TOTAL_SPELL_SCHOOLS] = {0};
    float HealTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};

    uint32_t SchoolImmunityList[TOTAL_SPELL_SCHOOLS] = {0};
    float SpellCritChanceSchool[TOTAL_SPELL_SCHOOLS] = {0};

    float PowerCostPctMod[TOTAL_SPELL_SCHOOLS] = {0};        // armor penetration & spell penetration

    int32_t AttackerCritChanceMod[TOTAL_SPELL_SCHOOLS] = {0};
    uint32_t SpellDelayResist[TOTAL_SPELL_SCHOOLS] = {0};

    int32_t CreatureAttackPowerMod[12] = {0};
    int32_t CreatureRangedAttackPowerMod[12] = {0};

    int32_t PctRegenModifier = 0;
    // SPELL_AURA_MOD_POWER_REGEN_PERCENT
    float PctPowerRegenModifier[TOTAL_PLAYER_POWER_TYPES];

    // Auras Modifiers
    int32_t m_pacified = 0;
    int32_t m_interruptRegen = 0;
    int32_t m_resistChance = 0;
    int32_t m_powerRegenPCT = 0;
    int32_t m_stunned = 0;
    int32_t m_extraattacks = 0;
    bool m_extrastriketarget = false;
    int32_t m_extrastriketargetc = 0;
    std::list<ExtraStrike*> m_extraStrikeTargets;
    int32_t m_fearmodifiers = 0;
    int64_t m_magnetcaster = 0;   // Unit who acts as a magnet for this unit

    // aurastate counters
    int8_t asc_frozen = 0;
    int8_t asc_enraged = 0;
    int8_t asc_seal = 0;
    int8_t asc_bleed = 0;

    uint16_t m_noInterrupt = 0;
    bool disarmed = false;
    uint64_t m_detectRangeGUID[5] = {0};
    int32_t  m_detectRangeMOD[5] = {0};

    uint32_t GetCharmTempVal() { return m_charmtemp; }
    void SetCharmTempVal(uint32_t val) { m_charmtemp = val; }

    AuraCheckResponse AuraCheck(SpellInfo const* proto, Object* caster = nullptr);
    AuraCheckResponse AuraCheck(SpellInfo const* proto, Aura* aur, Object* caster = nullptr);

    uint16_t m_diminishCount[DIMINISHING_GROUP_COUNT] = {0};
    uint8_t m_diminishAuraCount[DIMINISHING_GROUP_COUNT] = {0};
    uint16_t m_diminishTimer[DIMINISHING_GROUP_COUNT] = {0};
    bool m_diminishActive = false;

    void SetDiminishTimer(uint32_t index)
    {
        m_diminishTimer[index] = 15000;
    }

    DynamicObject* dynObj = nullptr;

    bool bProcInUse = false;
    bool bInvincible = false;
    Player* m_redirectSpellPackets = nullptr;
    void UpdateVisibility();

    struct
    {
        int32_t amt = 0;
        int32_t max = 0;
    } m_soulSiphon;

    void EventModelChange();
    inline float GetModelHalfSize() { return m_modelhalfsize * getScale(); }
    float getCollisionHeight() const override;

    void RemoveFieldSummon();

    float GetBlockFromSpell() { return m_blockfromspell; }
    float GetParryFromSpell() { return m_parryfromspell; }
    float GetDodgeFromSpell() { return m_dodgefromspell; }
    void SetBlockFromSpell(float value) { m_blockfromspell = value; }
    void SetParryFromSpell(float value) { m_parryfromspell = value; }
    void SetDodgeFromSpell(float value) { m_dodgefromspell = value; }

    void AggroPvPGuards();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void die(Unit* pAttacker, uint32_t damage, uint32_t spellid);
    
    void AddGarbagePet(Pet* pet);

    virtual void BuildPetSpellList(WorldPacket & data);

    uint64_t GetAuraUpdateMaskForRaid() const { return m_auraRaidUpdateMask; }
    void ResetAuraUpdateMaskForRaid() { m_auraRaidUpdateMask = 0; }
    void SetAuraUpdateMaskForRaid(uint8_t slot) { m_auraRaidUpdateMask |= (uint64_t(1) << slot); }
    void UpdateAuraForGroup(uint8_t slot);

protected:
    
    void RemoveGarbage();
    void AddGarbageAura(Aura* aur);

    uint32_t m_meleespell = 0;
    uint8_t m_meleespell_ecn = 0;         // extra_cast_number

    std::list<Aura*> m_GarbageAuras;
    std::list<Pet*> m_GarbagePets;

    // DK:pet
    
    bool can_parry = false;         //will be enabled by block spell

    int32_t m_manashieldamt = 0;
    uint32_t m_manaShieldId = 0;

    // Some auras can only be cast on one target at a time
    // This will map aura spell id to target guid
    UniqueAuraTargetMap m_singleTargetAura;

    uint32_t m_charmtemp = 0;

    bool m_extraAttackCounter = false;

    float m_modelhalfsize = 1.0f;      // used to calculate if something is in range of this unit

    float m_blockfromspell = 0.0f;
    float m_dodgefromspell = 0.0f;
    float m_parryfromspell = 0.0f;
    uint32_t m_BlockModPct = 0;       // is % but does not need float and does not need /100!

    
    uint64_t m_auraRaidUpdateMask = 0;

public:
    
    void BuildMovementPacket(ByteBuffer* data);
    void BuildMovementPacket(ByteBuffer* data, float x, float y, float z, float o);

    // AGPL End
};