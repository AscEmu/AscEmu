/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Object.hpp"
#include "UnitDefines.hpp"
#include "Macros/UnitMacros.hpp"
#include "Data/WoWUnit.hpp"
#include "ThreatHandler.h"
#include "CombatHandler.hpp"
#include "Management/Loot/Loot.hpp"
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
#include "Spell/SpellProc.hpp"

#include <array>
#include <list>
#include <optional>
#include <unordered_set>

#include "Spell/SpellDefines.hpp"
#include "Spell/Definitions/AuraRemoveMode.hpp"

namespace WDB::Structures
{
#if VERSION_STRING >= Cata
    struct MountCapabilityEntry;
#endif
}

namespace MySQLStructure
{
    struct NpcScriptText;
}

class SpellCastTargets;
struct AbstractFollower;
enum SummonSlot : uint8_t;
struct DamageSplitTarget;
template <class T>
using Optional = std::optional<T>;

class AIInterface;
class Aura;
class DynamicObject;
class GameObject;
class Group;
class Object;
class Spell;
class SpellProc;
class SummonHandler;
class TotemSummon;
class Vehicle;
class MovementManager;
struct FactionDBC;
struct SpellForcedBasePoints;

namespace MovementMgr
{
    class MoveSpline;
}

enum MovementGeneratorType : uint8_t;
enum SpellCastResult : uint8_t;

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

typedef std::array<std::unique_ptr<Aura>, AuraSlots::TOTAL_SLOT_END> AuraArray;
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

    void Update(unsigned long time_passed);                                 // hides function Object::Update
    // void AddToWorld();                                                   // not used
    // void AddToWorld(WorldMap* pMapMgr);                                  // not used
    // void PushToWorld(WorldMap*);                                         // not used
    virtual void RemoveFromWorld(bool free_guid);                           // hides virtual function Object::RemoveFromWorld
    // void OnPrePushToWorld();                                             // not used
    virtual void OnPushToWorld();                                           // hides virtual function Object::OnPushToWorld
    // void OnPreRemoveFromWorld();                                         // not used
    // void OnRemoveFromWorld();                                            // not used
    virtual void die(Unit* pAttacker, uint32_t damage, uint32_t spellid);

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
    void setFullHealth();
    void setHealthPct(uint32_t val);

    uint32_t getPower(PowerType type) const;
    void setPower(PowerType type, uint32_t value, bool sendPacket = true, bool skipObjectUpdate = false);
    void modPower(PowerType type, int32_t value);

    uint32_t getMaxHealth() const;
    void setMaxHealth(uint32_t maxHealth);
    void modMaxHealth(int32_t maxHealth);

    uint32_t getMaxPower(PowerType type) const;
    void setMaxPower(PowerType type, uint32_t value);
    void modMaxPower(PowerType type, int32_t value);

    float getPowerRegeneration(PowerType type) const;
    void setPowerRegeneration(PowerType type, float value);
    // Interrupted means 'while in combat' or 'while casting'
    float getPowerRegenerationWhileInterrupted(PowerType type) const;
    void setPowerRegenerationWhileInterrupted(PowerType type, float value);

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

#if VERSION_STRING >= WotLK
    // Returns item entry in wotlk and above
    uint32_t getVirtualItemSlotId(uint8_t slot) const;
#else
    // Returns item display id in classic and tbc
    uint32_t getVirtualItemDisplayId(uint8_t slot) const;
#endif
    void setVirtualItemSlotId(uint8_t slot, uint32_t item_id);

#if VERSION_STRING < WotLK
    uint64_t getVirtualItemInfo(uint8_t slot) const;
    unit_virtual_item_info getVirtualItemInfoFields(uint8_t slot) const;
    void setVirtualItemInfo(uint8_t slot, uint64_t item_info);
#endif

    uint32_t getUnitFlags() const;
    void setUnitFlags(uint32_t unitFlags);
    void addUnitFlags(uint32_t unitFlags);
    void removeUnitFlags(uint32_t unitFlags);
    bool hasUnitFlags(uint32_t unitFlags) const;

    // helper
    bool isInCombat() const { return getCombatHandler().isInCombat(); }
    bool isInCombatWith(Unit const* victim) const { return victim && getCombatHandler().isInPreCombatWithUnit(victim); }

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
    void resetDisplayId();

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

#if VERSION_STRING < WotLK
    uint8_t getPetLoyalty() const;
    void setPetLoyalty(uint8_t loyalty);
#elif VERSION_STRING < Mop
    uint8_t getPetTalentPoints() const;
    void setPetTalentPoints(uint8_t talentPoints);
#endif

    uint8_t getStandStateFlags() const;
    void setStandStateFlags(uint8_t standStateFlags);
    void addStandStateFlags(uint8_t standStateFlags);
    void removeStandStateFlags(uint8_t standStateFlags);

#if VERSION_STRING != Classic
    uint8_t getAnimationFlags() const;
    void setAnimationFlags(uint8_t animationFlags);
#endif
    //bytes_1 end

    // Note; this is not same as serverside PetCache::number or Pet::m_petId, this is clientside pet number which is pet's low guid
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

#if VERSION_STRING < Mop
    uint32_t getNpcFlags() const;
    void setNpcFlags(uint32_t npcFlags);
    void addNpcFlags(uint32_t npcFlags);
    void removeNpcFlags(uint32_t npcFlags);
#else
    uint64_t getNpcFlags() const;
    void setNpcFlags(uint64_t npcFlags);
    void addNpcFlags(uint64_t npcFlags);
    void removeNpcFlags(uint64_t npcFlags);
#endif

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

#if VERSION_STRING == TBC
    uint8_t getPositiveAuraLimit() const;
    void setPositiveAuraLimit(uint8_t limit);
#elif VERSION_STRING >= WotLK
    uint8_t getPvpFlags() const;
    void setPvpFlags(uint8_t pvpFlags);
    void addPvpFlags(uint8_t pvpFlags);
    void removePvpFlags(uint8_t pvpFlags);
#endif

#if VERSION_STRING >= TBC
    uint8_t getPetFlags() const;
    void setPetFlags(uint8_t petFlags);
    void addPetFlags(uint8_t petFlags);
    void removePetFlags(uint8_t petFlags);
#endif

    //bytes_1 in classic
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

    int32_t getCalculatedAttackPower() const;
    int32_t getCalculatedRangedAttackPower() const;

    bool canReachWithAttack(Unit* unitTarget);
    bool canBeginCombat(Unit* target);  //used in CreatureAIScript

    virtual void calculateDamage();

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
    std::unique_ptr<MovementMgr::MoveSpline> movespline;

    void followerAdded(AbstractFollower* f) { m_followingMe.insert(f); }
    void followerRemoved(AbstractFollower* f) { m_followingMe.erase(f); }
    void removeAllFollowers();
    virtual float getFollowAngle() const { return static_cast<float>(M_PI / 2); }

    MovementManager* getMovementManager() { return i_movementManager.get(); }
    MovementManager const* getMovementManager() const { return i_movementManager.get(); }

    virtual bool canFly();

    void stopMoving();
    void pauseMovement(uint32_t timer = 0, uint8_t slot = 0, bool forced = true); // timer in ms
    void resumeMovement(uint32_t timer = 0, uint8_t slot = 0); // timer in ms

private:
    std::unordered_set<AbstractFollower*> m_followingMe;

protected:
    std::unique_ptr<MovementManager> i_movementManager;

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
    Unit* m_controledUnit = this;
    Player* m_playerControler = nullptr;  

    //////////////////////////////////////////////////////////////////////////////////////////
    // AI Stuff
protected:
    std::unique_ptr<AIInterface> m_aiInterface;
    bool m_useAI = false;
    uint32_t m_lastAiInterfaceUpdateTime = 0;

public:
    AIInterface* getAIInterface() const { return m_aiInterface.get(); }

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

    SpellCastResult castSpell(uint64_t targetGuid, uint32_t spellId, bool triggered = false);
    SpellCastResult castSpell(Unit* target, uint32_t spellId, bool triggered = false);
    SpellCastResult castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, bool triggered = false);
    SpellCastResult castSpell(Unit* target, SpellInfo const* spellInfo, bool triggered = false);
    SpellCastResult castSpell(uint64_t targetGuid, uint32_t spellId, SpellForcedBasePoints forcedBasepoints, bool triggered = false);
    SpellCastResult castSpell(Unit* target, uint32_t spellId, SpellForcedBasePoints forcedBasePoints, bool triggered = false);
    SpellCastResult castSpell(Unit* target, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasePoints, int32_t spellCharges, bool triggered = false);
    SpellCastResult castSpell(SpellCastTargets targets, uint32_t spellId, bool triggered = false);
    SpellCastResult castSpell(SpellCastTargets targets, SpellInfo const* spellInfo, bool triggered = false);
    SpellCastResult castSpellLoc(const LocationVector location, uint32_t spellId, bool triggered = false);
    SpellCastResult castSpellLoc(const LocationVector location, SpellInfo const* spellInfo, bool triggered = false);
    void eventCastSpell(Unit* target, SpellInfo const* spellInfo);

    SpellCastResult castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasepoints, bool triggered);
    SpellCastResult castSpell(Unit* target, SpellInfo const* spellInfo, SpellForcedBasePoints forcedBasepoints, bool triggered);

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
    float m_spellCritPercentage = 0.0f;

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

    int32_t m_silenced = 0;

private:
    bool m_canDualWield = false;

    std::list<std::unique_ptr<SpellProc>> m_procSpells;

    std::list<AuraEffectModifier const*> m_spellModifiers[MAX_SPELLMOD_TYPE];

    uint32_t m_spellImmunityMask = SPELL_IMMUNITY_NONE;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Aura
    void addAura(std::unique_ptr<Aura> aur);
    uint8_t findVisualSlotForAura(Aura const* aur) const;

    Aura* getAuraWithId(uint32_t spell_id) const;
    Aura* getAuraWithId(uint32_t const* auraId) const;
    Aura* getAuraWithIdForGuid(uint32_t const* auraId, uint64_t guid) const;
    Aura* getAuraWithIdForGuid(uint32_t spell_id, uint64_t guid) const;
    Aura* getAuraWithAuraEffect(AuraEffect aura_effect) const;
    Aura* getAuraWithAuraEffectForGuid(AuraEffect aura_effect, uint64_t guid) const;
    Aura* getAuraWithVisualSlot(uint8_t visualSlot) const;
    // Note; this is internal serverside aura slot, not the slot in client
    // For clientside slot use getAuraWithVisualSlot
    Aura* getAuraWithAuraSlot(uint16_t auraSlot) const;

    int32_t getTotalIntDamageForAuraEffect(AuraEffect aura_effect) const;
    int32_t getTotalIntDamageForAuraEffectByMiscValue(AuraEffect aura_effect, int32_t miscValue) const;
    float_t getTotalFloatDamageForAuraEffect(AuraEffect aura_effect) const;
    float_t getTotalFloatDamageForAuraEffectByMiscValue(AuraEffect aura_effect, int32_t miscValue) const;
    // Returns 1.0f if there are no provided aura effects
    float_t getTotalPctMultiplierForAuraEffect(AuraEffect aura_effect) const;
    // Returns 1.0f if there are no provided aura effects
    float_t getTotalPctMultiplierForAuraEffectByMiscValue(AuraEffect aura_effect, int32_t miscValue) const;

    bool hasAurasWithId(uint32_t auraId) const;
    bool hasAurasWithId(uint32_t const* auraId) const;
    bool hasAurasWithIdForGuid(uint32_t auraId, uint64_t guid) const;
    bool hasAurasWithIdForGuid(uint32_t const* auraId, uint64_t guid) const;
    bool hasAuraWithAuraEffect(AuraEffect type) const;
    bool hasAuraWithAuraEffectForGuid(AuraEffect type, uint64_t guid) const;
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

    void eventRemoveAura(uint32_t spellId) { removeAllAurasById(spellId); }

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

    bool isPoisoned();
    bool isDazed() const;

private:
    // Inserts aura into aura containers
    void _addAura(std::unique_ptr<Aura> aur);
    // Inserts aura effect into aura effect list
    void _addAuraEffect(AuraEffectModifier const* aurEff);
    // Erases aura from aura containers
    std::unique_ptr<Aura> _removeAura(Aura* aur);
    // Erases aura effect from aura effect list
    void _removeAuraEffect(AuraEffectModifier const* aurEff);
    void _updateAuras(unsigned long diff);

    uint32_t m_transformAura = 0;

    AuraArray m_auraList;
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

    void updateVisibility();

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
    void regeneratePower(PowerType type, uint16_t timePassed);
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

protected:
    uint16_t m_healthRegenerateTimer = 0;
#if VERSION_STRING < WotLK
    // Mana and Energy
    uint16_t m_manaEnergyRegenerateTimer = 0;
    uint16_t m_rageRegenerateTimer = 0;
    uint16_t m_focusRegenerateTimer = 0;

    // Classic and TBC dont have these in unitdata
    float_t m_manaRegeneration = 0.0f;
    float_t m_manaRegenerationWhileCasting = 0.0f;
    float_t m_rageRegeneration = 0.0f;
    float_t m_rageRegenerationWhileCombat = 0.0f;
    float_t m_focusRegeneration = 0.0f;
    float_t m_energyRegeneration = 0.0f;
#else
    uint16_t m_powerRegenerateTimer = 0;
    uint16_t m_powerUpdatePacketTime = 0;
#endif
    void _regeneratePowersAtRegenUpdate(PowerType type);

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Stats and formulas, defined in UnitStats.cpp
    void updateEnergyRegeneration(bool initialUpdate = false);
    void updateFocusRegeneration(bool initialUpdate = false);

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

    virtual bool isPvpFlagSet() const;
    virtual void setPvpFlag();
    virtual void removePvpFlag();

    virtual bool isFfaPvpFlagSet() const;
    virtual void setFfaPvpFlag();
    virtual void removeFfaPvpFlag();

    virtual bool isSanctuaryFlagSet() const;
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
    void addHealthBatchEvent(std::unique_ptr<HealthBatchEvent> batch);
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
    void removeGameObject(uint32_t spellId, bool del);

    uint32_t m_objectSlots[4] = { 0 };

    void removeAllGameObjects();

    void deMorph();

private:
    uint32_t m_attackTimer[TOTAL_WEAPON_DAMAGE_TYPES] = {0};
    //\ todo: there seems to be new haste update fields in playerdata in cata, and moved to unitdata in mop
    float m_attackSpeed[TOTAL_WEAPON_DAMAGE_TYPES] = { 1.0f, 1.0f, 1.0f };

    void _updateHealth();
    // Handles some things on each damage event in the batch
    uint32_t _handleBatchDamage(HealthBatchEvent const* batch, uint32_t* rageGenerated);
    // Handles some things on each healing event in the batch
    uint32_t _handleBatchHealing(HealthBatchEvent const* batch, uint32_t* absorbedHeal);
    std::vector<std::unique_ptr<HealthBatchEvent>> m_healthBatch;
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

    // Returns unit's current pet from SummonInterface
    Pet* getPet() const;
    TotemSummon* getTotem(SummonSlot slot) const;

    SummonHandler* getSummonInterface();
    SummonHandler const* getSummonInterface() const;

private:
    std::unique_ptr<SummonHandler> m_summonInterface;

#ifdef FT_VEHICLES
    //////////////////////////////////////////////////////////////////////////////////////////
    // Vehicle
protected:
    Vehicle* m_vehicle = nullptr;               // The Unit's own vehicle component
    std::unique_ptr<Vehicle> m_vehicleKit;      // The vehicle the unit is attached to

public:
    bool createVehicleKit(uint32_t id, uint32_t creatureEntry);
    void removeVehicleKit();
    Vehicle* getVehicleKit() const { return m_vehicleKit.get(); }
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

    bool isMounted() const;
    void mount(uint32_t mount, uint32_t vehicleId = 0, uint32_t creatureEntry = 0);
    void dismount(bool resummonPet = true);

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
    bool m_taggedBySummon = false;

public:
    void setTaggerGuid(Unit const* tagger);
    uint64_t getTaggerGuid() const;

    bool isTagged() const;
    bool isTaggableFor(Unit const* unit) const;

    bool isTaggedByPlayerOrItsGroup(Player* tagger);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Loot
    Loot loot;

    bool isLootable();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
public:
    void buildMovementPacket(ByteBuffer* data);
    void buildMovementPacket(ByteBuffer* data, float x, float y, float z, float o);

    void possess(Unit* unitTarget, uint32_t delay = 0);
    void unPossess();

    // noInterrupt counter set through possess/unpossess
    uint16_t hasNoInterrupt() const { return m_noInterrupt; }
    uint16_t m_noInterrupt = 0;

    void removeGarbage();
    void addGarbageAura(std::unique_ptr<Aura> aur);
    void addGarbagePet(Pet* pet);

protected:
    std::list<std::unique_ptr<Aura>> m_GarbageAuras;
    std::list<Pet*> m_GarbagePets;

public:
    virtual void deactivate(WorldMap* mgr);

    float getChanceToDaze(Unit* target);

    void eventModelChange();

    void aggroPvPGuards();

    // Stun Immobilize
    uint32_t m_triggerOnStun = 0;
    uint32_t m_triggerOnStunChance = 100;
    uint32_t m_triggerOnStunVictim = 0;
    uint32_t m_triggerOnStunChanceVictim = 100;

    void setTriggerStunOrImmobilize(uint32_t newTrigger, uint32_t newChance, bool isVictim = false);
    void eventStunOrImmobilize(Unit* unitProcTarget, bool isVictim = false);

    // Chill
    uint32_t m_triggerOnChill = 0;
    uint32_t m_triggerOnChillChance = 100;
    uint32_t m_triggerOnChillVictim = 0;
    uint32_t m_triggerOnChillChanceVictim = 100;

    void setTriggerChill(uint32_t newTrigger, uint32_t newChance, bool isVictim = false);
    void eventChill(Unit* unitProcTarget, bool isVictim = false);

    void removeExtraStrikeTarget(SpellInfo const* spellInfo);
    void addExtraStrikeTarget(SpellInfo const* spellInfo, uint32_t charges);

    uint32_t doDamageSplitTarget(uint32_t res, SchoolMask schoolMask, bool isMeleeDmg);
    std::unique_ptr<DamageSplitTarget> m_damageSplitTarget;

    void removeReflect(uint32_t spellId, bool apply);
    std::list<std::unique_ptr<ReflectSpellSchool>> m_reflectSpellSchool;

    void castOnMeleeSpell();

    uint64_t getAuraUpdateMaskForRaid() const { return m_auraRaidUpdateMask; }
    void resetAuraUpdateMaskForRaid() { m_auraRaidUpdateMask = 0; }
    void setAuraUpdateMaskForRaid(uint8_t slot) { m_auraRaidUpdateMask |= (uint64_t(1) << slot); }

protected:
    uint64_t m_auraRaidUpdateMask = 0;

public:
    void updateAuraForGroup(uint8_t slot);

    void giveGroupXP(Unit* unitVictim, Player* playerInGroup);

    void calculateResistanceReduction(Unit* unitVictim, DamageInfo* damageInfo, SpellInfo const* spellInfoAbility, float armorPctReduce);

    //\todo: isn't the aura timed or triggered after healing?
    bool removeAurasByHeal();

    bool auraActionIf(AuraAction* auraAction, AuraCondition* auraCondition);

    uint32_t getManaShieldAbsorbedDamage(uint32_t damage);

    AuraCheckResponse auraCheck(SpellInfo const* spellInfo, Object* caster = nullptr);
    AuraCheckResponse auraCheck(SpellInfo const* spellInfo, Aura* aura, Object* caster = nullptr);

public:
    friend class AIInterface;
    friend class Aura;

    /// Combat
    uint32_t getSpellDidHitResult(Unit* pVictim, uint32_t weapon_damage_type, Spell* castingSpell);

    DamageInfo strike(Unit* pVictim, WeaponDamageType weaponType, SpellInfo const* ability, int32_t add_damage, int32_t pct_dmg_mod, uint32_t exclusive_damage, bool disable_proc, bool skip_hit_check, bool force_crit = false, Spell* castingSpell = nullptr);

protected:
    bool m_extraAttackCounter = false;

public:
    // triggeredFromAura is set if castingSpell has been triggered from aura, not if the proc is triggered from aura
    uint32_t handleProc(uint32_t flag, Unit* Victim, SpellInfo const* CastingSpell, DamageInfo damageInfo, bool isSpellTriggered, ProcEvents procEvent = PROC_EVENT_DO_ALL, Aura* triggeredFromAura = nullptr);

    void handleProcDmgShield(uint32_t flag, Unit* attacker);//almost the same as handleproc :P
    bool m_damageShieldsInUse = false;
    std::list<DamageProc> m_damageShields;

    uint32_t m_canMove = 0; //Zyres: set true in Spell::finish check for false in AIInterface::selectCurrentAgent

#if VERSION_STRING >= Cata
    WDB::Structures::MountCapabilityEntry const* getMountCapability(uint32_t mountType);
#endif
    void setOnMeleeSpell(uint32_t spellId, uint8_t ecn = 0) { m_meleeSpell = spellId; m_meleeSpell_ecn = ecn; }
    uint32_t getOnMeleeSpell() const { return m_meleeSpell; }
    uint8_t getOnMeleeSpellEcn() const { return m_meleeSpell_ecn; }

protected:
    uint32_t m_meleeSpell = 0;
    uint8_t m_meleeSpell_ecn = 0;         // extra_cast_number

public:
    void setHitFromMeleeSpell(float value) { m_hitFromMeleeSpell = value; }
    float getHitFromMeleeSpell() { return m_hitFromMeleeSpell; }
    float m_hitFromMeleeSpell = 0.0f;

    //\todo: these local vars can be replaced by proper aura effect handling.
    // pacified counter
    uint32_t isPacified() const { return m_pacified; }
    int32_t m_pacified = 0;
    // stunned counter
    uint32_t isStunned() const { return m_stunned; }
    int32_t m_stunned = 0;
    // fear counter
    uint32_t isFeared() const { return m_fearModifiers; }
    int32_t m_fearModifiers = 0;

    uint32_t getResistChanceMod() const { return m_resistChance; }
    void setResistChanceMod(uint32_t amount) { m_resistChance = amount; }
    int32_t m_resistChance = 0;

    // Used in Aura::SpellEffectInterruptCast
    uint32_t m_schoolCastPrevent[TOTAL_SPELL_SCHOOLS] = {0};

    // Used in Aura::SpellAuraReduceEffectDuration
    int32_t m_mechanicDurationPctMod[28] = {0};

    virtual int32_t GetDamageDoneMod(uint16_t /*school*/) { return 0; }
    virtual float GetDamageDonePctMod(uint16_t /*school*/) { return 0; }

    // Used in Aura::SpellAuraModDamageTaken & Unit:strike
    int32_t m_damageTakenMod[TOTAL_SPELL_SCHOOLS] = {0};

    // Used in Aura::SpellAuraModDamagePercTaken
    float m_damageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};

    // Used in Aura::SpellAuraModDamagePercTaken
    float m_damageTakenPctModOnHP35 = 1;

    // Used in Aura::SpellAuraReduceCritMeleeAttackDmg
    float m_critMeleeDamageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};

    // Used in Aura::SpellAuraReduceCritRangedAttackDmg
    float m_critRangedDamageTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};

    // Used in Aura::SpellAuraModRangedDamageTaken
    int32_t m_rangedDamageTaken = 0;

    // Used instead of WoWUnit field
    // note: this is base damage whereas WoWUnit field is the calculated damage -Appled
    float m_baseDamage[2] = {0};

    // Used instead of WoWUnit field
    // note: this is base damage whereas WoWUnit field is the calculated damage -Appled
    float m_baseOffhandDamage[2] = {0};

    // Used instead of WoWUnit field
    // note: this is base damage whereas WoWUnit field is the calculated damage -Appled
    float m_baseRangedDamage[2] = {0};

    // Used in Aura::SpellAuraRAPAttackerBonus
    int32_t m_rangeAttackPowerModifier = 0;

    // Used in Aura::SpellAuraAPAttackerBonus
    int32_t m_attackPowerModifier = 0;

    // Used in Aura::SpellAuraModStalked
    uint64_t m_stalkedByGuid = 0;

    // Used in Aura::SpellAuraModDispelImmunity
    uint32_t m_dispels[10] = {0};

    // Used in Aura::SpellAuraTrackStealthed
    bool m_trackStealth = false;

    // Used in several aura functions
    uint32_t m_mechanicsDispels[32] = {0};

    // Used in Aura::SpellAuraModMechanicResistance
    float m_mechanicsResistancesPct[32] = {0};

    // Used in Aura::SpellAuraModMechanicDmgTakenPct
    float m_modDamageTakenByMechPct[32] = {0};

    // Used in Aura::SpellAuraModSpellDamageDOTPct
    int32_t m_DoTPctIncrease[TOTAL_SPELL_SCHOOLS] = {0};

    // Used in Aura::SpellAuraReduceAOEDamageTaken
    float m_AOEDmgMod = 1.0f;

    // Used in Aura::SpellAuraModIgnoreArmorPct
    float m_ignoreArmorPctMaceSpec = 0.0f;
    float m_ignoreArmorPct = 0.0f;

    void setcanparry(bool newstatus) { m_canParry = newstatus; }

protected:
    bool m_canParry = false;         //will be enabled by block spell

public:
    std::map<uint32_t, Aura*> m_tempAuraMap;

    uint32_t m_baseResistance[TOTAL_SPELL_SCHOOLS] = {0};        // there are resistances for silence, fear, mechanics ....
    uint32_t m_baseStats[5] = {0};

    int32_t m_healDoneMod[TOTAL_SPELL_SCHOOLS] = {0};
    float m_healDonePctMod[TOTAL_SPELL_SCHOOLS] = {0};

    int32_t m_healTakenMod[TOTAL_SPELL_SCHOOLS] = {0};
    float m_healTakenPctMod[TOTAL_SPELL_SCHOOLS] = {0};

    uint32_t m_schoolImmunityList[TOTAL_SPELL_SCHOOLS] = {0};
    float m_spellCritChanceSchool[TOTAL_SPELL_SCHOOLS] = {0};

    float m_powerCostPctMod[TOTAL_SPELL_SCHOOLS] = {0};        // armor penetration & spell penetration

    int32_t m_attackerCritChanceMod[TOTAL_SPELL_SCHOOLS] = {0};
    uint32_t m_spellDelayResist[TOTAL_SPELL_SCHOOLS] = {0};

    int32_t m_creatureAttackPowerMod[12] = {0};
    int32_t m_creatureRangedAttackPowerMod[12] = {0};

    // Auras Modifiers
    int32_t m_interruptRegen = 0;
    int32_t m_powerRegenPct = 0;
    
    int32_t m_extraAttacks = 0;
    bool m_extraStrikeTarget = false;
    int32_t m_extraStrikeTargetC = 0;
    std::list<std::unique_ptr<ExtraStrike>> m_extraStrikeTargets;
    
    int64_t m_magnetCasterGuid = 0;   // Unit who acts as a magnet for this unit

    // aurastate counters
    int8_t m_ascFrozen = 0;
    int8_t m_ascEnraged = 0;
    int8_t m_ascSeal = 0;
    int8_t m_ascBleed = 0;
    
    bool m_isDisarmed = false;
    uint64_t m_detectRangeGuids[5] = {0};
    int32_t  m_detectRangeMods[5] = {0};

    uint32_t getCharmTempVal() const { return m_tempCharm; }
    void setCharmTempVal(uint32_t val) { m_tempCharm = val; }

protected:
    uint32_t m_tempCharm = 0;

public:
    void setDiminishTimer(uint32_t index) { m_diminishTimer[index] = 15000; }

    uint16_t m_diminishCount[DIMINISHING_GROUP_COUNT] = {0};
    uint8_t m_diminishAuraCount[DIMINISHING_GROUP_COUNT] = {0};
    uint16_t m_diminishTimer[DIMINISHING_GROUP_COUNT] = {0};
    bool m_diminishActive = false;

    DynamicObject* m_dynamicObject = nullptr;

    bool m_isProcInUse = false;
    bool m_isInvincible = false;

    Player* m_redirectSpellPackets = nullptr;

    struct
    {
        int32_t m_amount = 0;
        int32_t m_max = 0;
    } m_soulSiphon;
    
    inline float getModelHalfSize() const { return m_modelHalfSize * getScale(); }

protected:
    float m_modelHalfSize = 1.0f;      // used to calculate if something is in range of this unit

public:
    float getCollisionHeight() const override;

    float getBlockFromSpell() const { return m_blockFromSpell; }
    float getParryFromSpell() const { return m_parryFromSpell; }
    float getDodgeFromSpell() const { return m_dodgeFromSpell; }
    void setBlockFromSpell(float value) { m_blockFromSpell = value; }
    void setParryFromSpell(float value) { m_parryFromSpell = value; }
    void setDodgeFromSpell(float value) { m_dodgeFromSpell = value; }

protected:
    float m_blockFromSpell = 0.0f;
    float m_dodgeFromSpell = 0.0f;
    float m_parryFromSpell = 0.0f;

    // Used in Aura::SpellAuraManaShield
    int32_t m_manashieldAmount = 0;
    uint32_t m_manaShieldId = 0;

    // Some auras can only be cast on one target at a time
    // This will map aura spell id to target guid
    UniqueAuraTargetMap m_singleTargetAura;

    // Used in Aura::SpellAuraBlockMultipleDamage
    uint32_t m_blockModPct = 0;       // is % but does not need float and does not need /100!
};
