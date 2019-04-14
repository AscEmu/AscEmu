/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "AI/MovementAI.h"
#include "Objects/Object.h"
#include "Spell/SpellDefines.hpp"

#include "UnitDefines.hpp"
#include "Management/LootMgr.h"
#include "Spell/SpellProc.h"
#include "Objects/Object.h"
#include "Units/Summons/SummonHandler.h"
#include "Movement/UnitMovementManager.hpp"
#include "Spell/Definitions/AuraEffects.h"
#include "Spell/Definitions/AuraStates.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/School.h"
#include "Storage/MySQLStructures.h"

#if VERSION_STRING < Cata
#include "Data/MovementInfo.h"
#endif

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
    SpellInfo const* spell_info;
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

public:CombatStatusHandler() : m_Unit(nullptr), m_lastStatus(false), m_primaryAttackTarget(0) {}

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
struct WoWUnit;
class SERVER_DECL Unit : public Object
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    const WoWUnit* unitData() const { return reinterpret_cast<WoWUnit*>(wow_data); }

    MovementAI m_movementAI;
public:
    MovementAI& getMovementAI();
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

    uint64_t getChannelObjectGuid() const;
    void setChannelObjectGuid(uint64_t guid);

    uint32_t getChannelSpellId() const;
    void setChannelSpellId(uint32_t spell_id);

    //bytes_0 begin
    uint32_t getBytes0() const;
    void setBytes0(uint32_t bytes);

    uint8_t getRace() const;
    void setRace(uint8_t race);
    uint32_t getRaceMask() { return 1 << (getRace() - 1); }

    uint8_t getClass() const;
    void setClass(uint8_t class_);
    uint32_t getClassMask() { return 1 << (getClass() - 1); }

    uint8_t getGender() const;
    void setGender(uint8_t gender);

    uint8_t getPowerType() const;
    void setPowerType(uint8_t powerType);
    //bytes_0 end

    uint32_t getHealth() const;
    void setHealth(uint32_t health);
    void modHealth(int32_t health);

    uint32_t getPower(uint16_t index) const;
    void setPower(uint16_t index, uint32_t value);
    void modPower(uint16_t index, int32_t value);

    uint32_t getMaxHealth() const;
    void setMaxHealth(uint32_t maxHealth);
    void modMaxHealth(int32_t maxHealth);

    void setMaxMana(uint32_t maxMana);

    uint32_t getMaxPower(uint16_t index) const;
    void setMaxPower(uint16_t index, uint32_t value);
    void modMaxPower(uint16_t index, int32_t value);

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

    uint32_t getBaseMana() const;
    void setBaseMana(uint32_t baseMana);

    uint32_t getBaseHealth() const;
    void setBaseHealth(uint32_t baseHealth);

    //byte_2 begin
    uint32_t getBytes2() const;
    void setBytes2(uint32_t bytes);

    uint8_t getSheathType() const;
    void setSheathType(uint8_t sheathType);

    uint8_t getPvpFlags() const;
    void setPvpFlags(uint8_t pvpFlags);

    uint8_t getPetFlags() const;
    void setPetFlags(uint8_t petFlags);

    uint8_t getShapeShiftForm() const;
    void setShapeShiftForm(uint8_t shapeShiftForm);
    uint32_t getShapeShiftMask() { return 1 << (getShapeShiftForm() - 1); }
    //bytes_2 end

    uint32_t getAttackPower() const;
    void setAttackPower(uint32_t value);

    int32_t getRangedAttackPower() const;
    void setRangedAttackPower(int32_t power);

    float getMinRangedDamage() const;
    void setMinRangedDamage(float damage);

    float getMaxRangedDamage() const;
    void setMaxRangedDamage(float damage);

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

    float getSpeedForType(UnitSpeedType speed_type, bool get_basic = false) const;
    float getFlySpeed() const;
    float getSwimSpeed() const;
    float getRunSpeed() const;
    UnitSpeedType getFastestSpeedType() const;
    float getFastestSpeed() const;

    void setSpeedForType(UnitSpeedType speed_type, float speed, bool set_basic = false);
    void resetCurrentSpeed();

    void sendMoveSplinePaket(UnitSpeedType speed_type);

    // Mover
    Unit* mControledUnit;
    Player* mPlayerControler;

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
    void castSpellLoc(const LocationVector location, SpellInfo const* spellInfo, bool triggered);
    void eventCastSpell(Unit* target, SpellInfo const* spellInfo);

    void castSpell(uint64_t targetGuid, SpellInfo const* spellInfo, uint32_t forcedBasepoints, bool triggered);
    void castSpell(Unit* target, SpellInfo const* spellInfo, uint32_t forcedBasepoints, bool triggered);

private:
    bool m_canDualWield;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Aura
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

    uint64_t getSingleTargetGuidForAura(uint32_t spellId);
    uint64_t getSingleTargetGuidForAura(uint32_t* spellIds, uint32_t* index);

    void setSingleTargetGuidForAura(uint32_t spellId, uint64_t guid);
    void removeSingleTargetGuidForAura(uint32_t spellId);

    void removeAllAurasByAuraEffect(AuraEffect effect);

#ifdef AE_TBC
    uint32_t addAuraVisual(uint32_t spell_id, uint32_t count, bool positive);
    uint32_t addAuraVisual(uint32_t spell_id, uint32_t count, bool positive, bool &skip_client_update);
    void setAuraSlotLevel(uint32_t slot, bool positive);
#endif

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
     int32_t m_stealthLevel[STEALTH_FLAG_TOTAL];
     int32_t m_stealthDetection[STEALTH_FLAG_TOTAL];
     // Invisibility
     int32_t m_invisibilityLevel[INVIS_FLAG_TOTAL];
     int32_t m_invisibilityDetection[INVIS_FLAG_TOTAL];

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    void setAttackTimer(WeaponDamageType type, int32_t time);
    uint32_t getAttackTimer(WeaponDamageType type) const;
    bool isAttackReady(WeaponDamageType type) const;
    void resetAttackTimers();

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

    bool isSitting() const;

    uint8_t getHealthPct() const;
    uint8_t getPowerPct(PowerType powerType) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Death
protected:
    DeathState m_deathState;

public:
    bool isAlive() const;
    bool justDied() const;
    bool isDead() const;
    virtual void setDeathState(DeathState state);
    DeathState getDeathState() const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement

    MovementInfo movement_info;

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
    // Vehicle
protected:
    Vehicle* m_currentVehicle;    // The vehicle the unit is attached to
    Vehicle* m_vehicle;           // The Unit's own vehicle component

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

    // Do not alter anything below this line
    // -------------------------------------
private:
    uint32_t m_attackTimer[3];
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
    uint32 GetSpellDidHitResult(Unit* pVictim, uint32 weapon_damage_type, SpellInfo const* ability);
    void Strike(Unit* pVictim, uint32 weapon_damage_type, SpellInfo const* ability, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool force_crit = false);
    uint32 m_procCounter;
    uint32 HandleProc(uint32 flag, Unit* Victim, SpellInfo const* CastingSpell, bool is_triggered = false, uint32 dmg = -1, uint32 abs = 0, uint32 weapon_damage_type = 0);
    void HandleProcDmgShield(uint32 flag, Unit* attacker);//almost the same as handleproc :P
    bool IsCriticalDamageForSpell(Object* victim, SpellInfo const* spell);
    float GetCriticalDamageBonusForSpell(Object* victim, SpellInfo const* spell, float amount);
    bool IsCriticalHealForSpell(Object* victim, SpellInfo const* spell);
    float GetCriticalHealBonusForSpell(Object* victim, SpellInfo const* spell, float amount);

    void RemoveExtraStrikeTarget(SpellInfo const* spell_info);
    void AddExtraStrikeTarget(SpellInfo const* spell_info, uint32 charges);

    int32 GetAP();
    int32 GetRAP();

    bool IsInInstance();
    void CalculateResistanceReduction(Unit* pVictim, dealdamage* dmg, SpellInfo const* ability, float ArmorPctReduce);
    void RegenerateHealth();
    void RegeneratePower(bool isinterrupted);
    void setHRegenTimer(uint32 time) { m_H_regenTimer = static_cast<uint16>(time); }
    void setPRegenTimer(uint32 time) { m_P_regenTimer = static_cast<uint16>(time); }
    void DelayPowerRegeneration(uint32 time) { m_P_regenTimer = static_cast<uint16>(time); if (!m_interruptedRegenTime) m_interruptedRegenTime = 2000; }
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

    // ProcTrigger
    std::list<SpellProc*> m_procSpells;
    SpellProc* AddProcTriggerSpell(uint32 spell_id, uint32 orig_spell_id, uint64 caster, uint32 procChance, uint32 procFlags, uint32 procCharges, uint32* groupRelation, uint32* procClassMask = nullptr, Object* obj = nullptr);
    SpellProc* AddProcTriggerSpell(SpellInfo const* spell, SpellInfo const* orig_spell, uint64 caster, uint32 procChance, uint32 procFlags, uint32 procCharges, uint32* groupRelation, uint32* procClassMask = nullptr, Object* obj = nullptr);
    SpellProc* AddProcTriggerSpell(SpellInfo const* sp, uint64 caster, uint32* groupRelation, uint32* procClassMask = nullptr, Object* obj = nullptr);
    SpellProc* GetProcTriggerSpell(uint32 spellId, uint64 casterGuid = 0);
    void RemoveProcTriggerSpell(uint32 spellId, uint64 casterGuid = 0, uint64 misc = 0);

    bool IsPoisoned();

    void GiveGroupXP(Unit* pVictim, Player* PlayerInGroup);

    void OnDamageTaken();

    //caller is the caster
    int32 GetSpellDmgBonus(Unit* pVictim, SpellInfo const* spellInfo, int32 base_dmg, bool isdot);

    float CalcSpellDamageReduction(Unit* victim, SpellInfo const* spell, float res);

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

    virtual int32 GetDamageDoneMod(uint16_t /*school*/) { return 0; }
    virtual float GetDamageDonePctMod(uint16_t /*school*/) { return 0; }

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
    uint32 GetOldEmote() { return m_oldEmote; }
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

    void SetFaction(uint32 factionId)
    {
        setFactionTemplate(factionId);
        setServersideFaction();
    }

    virtual void SendChatMessage(uint8 type, uint32 lang, const char* msg, uint32 delay = 0) = 0;
    virtual void SendChatMessageToPlayer(uint8 type, uint32 lang, const char* msg, Player* plr) = 0;
    void SendChatMessageAlternateEntry(uint32 entry, uint8 type, uint32 lang, const char* msg);
    void RegisterPeriodicChatMessage(uint32 delay, uint32 msgid, std::string message, bool sendnotify);

    void SetHealthPct(uint32 val) { if (val > 0) setHealth(float2int32(val * 0.01f * getMaxHealth())); };

    //In-Range
    virtual void addToInRangeObjects(Object* pObj);
    virtual void onRemoveInRangeObject(Object* pObj);
    void clearInRangeSets();

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

    AuraCheckResponse AuraCheck(SpellInfo const* proto, Object* caster = nullptr);
    AuraCheckResponse AuraCheck(SpellInfo const* proto, Aura* aur, Object* caster = nullptr);

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
    void ModVisualAuraStackCount(Aura* aur, int32 count);
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
    void EventStrikeWithAbility(uint64 guid, SpellInfo const* sp, uint32 damage);
    void DispelAll(bool positive);

    void SendPowerUpdate(bool self);
    void SendPeriodicAuraLog(const WoWGuid & CasterGUID, const WoWGuid & casterGUID, uint32 SpellID, uint32 School, uint32 Amount, uint32 abs_dmg, uint32 resisted_damage, uint32 Flags, bool is_critical);
    void SendPeriodicHealAuraLog(const WoWGuid & CasterGUID, const WoWGuid & TargetGUID, uint32 SpellID, uint32 healed, uint32 over_healed, bool is_critical);

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

    std::list<Aura*> m_GarbageAuras;
    std::list<Spell*> m_GarbageSpells;
    std::list<Pet*> m_GarbagePets;

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

    
    uint64 m_auraRaidUpdateMask;

public:

    virtual Group* GetGroup() { return nullptr; }
    
    const CombatStatusHandler* getcombatstatus() const { return &CombatStatus; }

    bool m_noFallDamage;
    float z_axisposition;
    int32 m_safeFall;

    void BuildHeartBeatMsg(WorldPacket* data);

    void BuildMovementPacket(ByteBuffer* data);
    void BuildMovementPacket(ByteBuffer* data, float x, float y, float z, float o);


    // AGPL End
};