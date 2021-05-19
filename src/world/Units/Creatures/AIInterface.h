/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/RecastIncludes.hpp"
#include "Server/IUpdatable.h"
#include "Units/Creatures/AIEvents.h"
#include "Units/Unit.h"
#include "Macros/AIInterfaceMacros.hpp"
#include "Units/Creatures/CreatureDefines.hpp"
#include "Server/Script/ScriptEvent.hpp"
#include "Movement/WaypointDefines.h"

#include <G3D/Vector3.h>

inline bool inRangeYZX(const float* v1, const float* v2, const float r, const float h)
{
    const float dx = v2[0] - v1[0];
    const float dy = v2[1] - v1[1]; // elevation
    const float dz = v2[2] - v1[2];
    return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
}

struct AbstractFollower;
class AreaBoundary;
class MapMgr;
class Object;
class Creature;
class Unit;
class Player;
class WorldSession;
class SpellCastTargets;

enum AiScriptTypes
{
    AI_SCRIPT_LONER,
    AI_SCRIPT_AGRO,
    AI_SCRIPT_SOCIAL,
    AI_SCRIPT_PET,
    AI_SCRIPT_TOTEM,
    AI_SCRIPT_GUARDIAN, //we got a master but he cannot control us, we follow and battle opposite factions
    AI_SCRIPT_PASSIVE
};

enum ReactStates : uint8_t
{
    REACT_PASSIVE = 0,
    REACT_DEFENSIVE = 1,
    REACT_AGGRESSIVE = 2
};

enum AI_Agent
{
    AGENT_NULL,
    AGENT_MELEE,
    AGENT_RANGED,
    AGENT_FLEE,
    AGENT_SPELL,
    AGENT_CALLFORHELP
};

enum AI_SpellType
{
    STYPE_NULL,
    STYPE_ROOT,
    STYPE_HEAL,
    STYPE_STUN,
    STYPE_FEAR,
    STYPE_SILENCE,
    STYPE_CURSE,
    STYPE_AOEDAMAGE,
    STYPE_DAMAGE,
    STYPE_SUMMON,
    STYPE_BUFF,
    STYPE_DEBUFF
};

enum AI_SpellTargetType
{
    TTYPE_NULL,
    TTYPE_SINGLETARGET,
    TTYPE_DESTINATION,
    TTYPE_SOURCE,
    TTYPE_CASTER,
    TTYPE_OWNER
};

enum AiState : uint8_t
{
    AI_STATE_IDLE = 0,
    AI_STATE_ATTACKING = 1,
    AI_STATE_CASTING = 2,
    AI_STATE_FLEEING = 3,
    AI_STATE_FOLLOWING = 4,
    AI_STATE_EVADE = 5,
    AI_STATE_MOVEWP = 6,
    AI_STATE_FEAR = 7,
    AI_STATE_UNFEARED = 8,
    AI_STATE_WANDER = 9,
    AI_STATE_STOPPED = 10,
    AI_STATE_SCRIPTMOVE = 11,
    AI_STATE_SCRIPTIDLE = 12,
};

class SpellInfo;

const uint32_t AISPELL_ANY_DIFFICULTY = 4;

struct AI_Spell
{
    ~AI_Spell() { autocast_type = (uint32_t)-1; }
    uint32_t entryId;
    uint8_t instance_mode;
    uint16_t agent;
    uint32_t procChance;
    SpellInfo const* spell;
    uint8_t spellType;
    uint8_t spelltargetType;
    uint32_t cooldown;
    uint32_t cooldowntime;
    uint32_t procCount;
    uint32_t procCounter;
    float floatMisc1;
    uint32_t Misc2;
    float minrange;
    float maxrange;
    uint32_t autocast_type;
};

class SERVER_DECL AIInterface : public IUpdatable
{
public:
    AIInterface();
    virtual ~AIInterface();

    //////////////////////////////////////////////////////////////////////////////////////////
    // AI Agent functions
    bool m_canRangedAttack;
    void selectCurrentAgent(Unit* target, uint32_t spellid);
    void initializeSpells();

    void SetCannotReachTarget(bool cannotReach);
    bool CanNotReachTarget() const { return m_cannotReachTarget; }

    void callForHelp(float fRadius);
    void doFleeToGetAssistance();
    void callAssistance();
    bool alreadyCalledForHelp() { return m_AlreadyCallAssistance; }
    void setNoCallAssistance(bool val) { m_AlreadyCallAssistance = val; }
    void setNoSearchAssistance(bool val) { m_AlreadySearchedAssistance = val; }
    bool gasSearchedAssistance() const { return m_AlreadySearchedAssistance; }
    bool canAssistTo(Unit* u, Unit* enemy, bool checkfaction = true);

    inline uint8_t getCurrentAgent() { return static_cast<uint8_t>(m_AiCurrentAgent); }
    void setCurrentAgent(AI_Agent agent) { m_AiCurrentAgent = agent; }

    bool canCallForHelp() { return m_canCallForHelp; }
    void setCanCallForHelp(bool value) { m_canCallForHelp = value; }
    bool canFlee() { return m_canFlee; }
    void setCanFlee(bool value ) { m_canFlee = value; }

    bool m_canFlee;
    float m_FleeHealth;
    uint32_t m_FleeDuration;
    bool m_canCallForHelp;
    float m_CallForHelpHealth;

private:
    AI_Agent m_AiCurrentAgent;
    bool m_hasFleed;
    SmallTimeTracker m_fleeTimer;

protected:
    bool m_AlreadyCallAssistance;
    bool m_AlreadySearchedAssistance;

    //////////////////////////////////////////////////////////////////////////////////////////
    // AI Script functions
private:
    AiScriptTypes m_AiScriptType;

public:
    void setAiScriptType(AiScriptTypes ai_type) { m_AiScriptType = ai_type; }
    AiScriptTypes getAiScriptType() const { return m_AiScriptType; }
    bool isAiScriptType(AiScriptTypes ai_type) const { return ai_type == m_AiScriptType; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // AI State functions
public:
    void setAiState(AiState ai_state) { m_AiState = ai_state; }
    void removeAiState(AiState ai_state) { m_AiState &= ~ai_state; }
    uint32_t getAiState() const { return m_AiState; }
    bool isAiState(AiState ai_state) const { return ai_state == m_AiState; }

private:
    uint32_t m_AiState;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Combat functions
public:
    void JustEnteredCombat(Unit* pUnit);
    bool isEngaged() { return m_isEngaged; }

    bool isImmuneToNPC() { return m_Unit->hasUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT); }
    bool isImmuneToPC() { return m_Unit->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT); }
    void setImmuneToNPC(bool apply);
    void setImmuneToPC(bool apply);

    // Called when unit takes damage or get hits by spell
    void onHostileAction(Unit* pUnit, SpellInfo const* spellInfo = nullptr, bool ignoreThreatRedirects = false);

    void setAllowedToEnterCombat(bool val) { canEnterCombat = val; }
    inline bool getAllowedToEnterCombat(void) { return canEnterCombat; }

    void setReactState(ReactStates st) { m_reactState = st; }
    ReactStates getReactState() const { return m_reactState; }
    bool hasReactState(ReactStates state) const { return (m_reactState == state); }
    void initializeReactState();

private:
    bool m_isEngaged;

protected:
    ReactStates m_reactState;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Combat behavior
private:
    bool mIsCombatDisabled;
    bool mIsMeleeDisabled;
    bool mIsRangedDisabled;
    bool mIsCastDisabled;
    bool mIsTargetingDisabled;

public:
    void setCombatDisabled(bool disable) { mIsCombatDisabled = disable; }
    bool isCombatDisabled() { return mIsCombatDisabled; }

    void setMeleeDisabled(bool disable) { mIsMeleeDisabled = disable; }
    bool isMeleeDisabled() { return mIsMeleeDisabled; }

    void setRangedDisabled(bool disable) { mIsRangedDisabled = disable; }
    bool isRangedDisabled() { return mIsRangedDisabled; }

    void setCastDisabled(bool disable) { mIsCastDisabled = disable; }
    bool isCastDisabled() { return mIsCastDisabled; }

    void setTargetingDisabled(bool disable) { mIsTargetingDisabled = disable; }
    bool isTargetingDisabled() { return mIsTargetingDisabled; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc functions
    void Init(Unit* un, AiScriptTypes at);
    void Init(Unit* un, AiScriptTypes at, Unit* owner);   // used for pets
    Unit* getUnit() const;
    Unit* getPetOwner() const;
    Unit* getCurrentTarget() const;
    LocationVector m_lasttargetPosition;

    bool isGuard() { return m_isNeutralGuard; }
    void setGuard(bool value) { m_isNeutralGuard = value; }
    void setCurrentTarget(Unit* pUnit) { m_target = pUnit; }
    float calcCombatRange(Unit* target, bool ranged);

    void updateEmotes(unsigned long time_passed);
    void eventAiInterfaceParamsetFinish();
    TimedEmoteList* timed_emotes;

    bool moveTo(float x, float y, float z, float o = 0.0f, bool running = false);
    void calcDestinationAndMove(Unit* target, float dist);

    // boundary system methods
    bool checkBoundary();
    CreatureBoundary const getBoundary() const { return _boundary; }
    void addBoundary(AreaBoundary const* boundary, bool overrideDefault = false, bool negativeBoundaries = false);
    void setDefaultBoundary();
    void clearBoundary();
    static bool isInBounds(CreatureBoundary const* boundary, LocationVector who);
    bool isInBoundary(LocationVector who) const;
    bool isInBoundary() const;
    void doImmediateBoundaryCheck() { m_boundaryCheckTime.resetInterval(0); }

    bool canUnitEvade(unsigned long time_passed);
    void enterEvadeMode();
    bool _enterEvadeMode();

    void initGroupThreat(Unit* target);
    void instanceCombatProgress(bool activate);

    // Event Handler
    void HandleEvent(uint32_t event, Unit* pUnit, uint32_t misc1);

    void EventForceRedirected(Unit* pUnit, uint32_t misc1);
    void EventHostileAction(Unit* pUnit, uint32_t misc1);
    void EventUnitDied(Unit* pUnit, uint32_t misc1);
    void EventUnwander(Unit* pUnit, uint32_t misc1);
    void EventWander(Unit* pUnit, uint32_t misc1);
    void EventUnfear(Unit* pUnit, uint32_t misc1);
    void EventFear(Unit* pUnit, uint32_t misc1);
    void EventFollowOwner(Unit* pUnit, uint32_t misc1);
    void EventDamageTaken(Unit* pUnit, uint32_t misc1);
    void EventLeaveCombat(Unit* pUnit, uint32_t misc1);
    void EventEnterCombat(Unit* pUnit, uint32_t misc1);
    void EventChangeFaction(Unit* ForceAttackersToHateThisInstead = NULL);    /// we have to tell our current enemies to stop attacking us, we should also forget about our targets
    void OnDeath(Object* pKiller);

    // Update
    void Update(unsigned long time_passed);
    void updateTargets(unsigned long time_passed);
    void updateVictim(Unit* victim);

    float calcAggroRange(Unit* target);
    bool canOwnerAttackUnit(Unit* pUnit);        /// this is designed for internal use only
    Unit* findTarget();
    void updateCombat(uint32_t p_time);
    void updateTotem(uint32_t p_time);

    Unit* m_Unit;
    Unit* m_PetOwner;
    Unit* m_target;
    bool m_isNeutralGuard;
    uint32_t faction_visibility;

    // Difficulty
    void setCreatureProtoDifficulty(uint32_t entry);
    uint8_t getDifficultyType();
    bool m_is_in_instance;

protected:
    SmallTimeTracker m_boundaryCheckTime;
    CreatureBoundary _boundary;
    bool _negateBoundary;

private:
    bool m_disableDynamicBoundary = false;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement functions
public:
    virtual void onMovementGeneratorFinalized(MovementGeneratorType /*type*/) { }

    // Called at waypoint reached or point movement finished
    virtual void movementInform(uint32_t /*type*/, uint32_t /*id*/);

    bool canCreatePath(float x, float y, float z);
    dtStatus findSmoothPath(const float* startPos, const float* endPos, const dtPolyRef* polyPath, const uint32 polyPathSize, float* smoothPath, int* smoothPathSize, bool & usedOffmesh, const uint32 maxSmoothPathSize, dtNavMesh* mesh, dtNavMeshQuery* query, dtQueryFilter & filter);
    bool getSteerTarget(const float* startPos, const float* endPos, const float minTargetDist, const dtPolyRef* path, const uint32 pathSize, float* steerPos, unsigned char & steerPosFlag, dtPolyRef & steerPosRef, dtNavMeshQuery* query);
    uint32 fixupCorridor(dtPolyRef* path, const uint32 npath, const uint32 maxPath, const dtPolyRef* visited, const uint32 nvisited);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Waypoint functions
private:
    bool mShowWayPoints;

public:
    bool hasWayPoints();
    uint32_t getCurrentWayPointId();
    uint32_t getWayPointsCount();

    void setWayPointToMove(uint32_t waypointId);

    bool activateShowWayPoints(Player* player, bool showBackwards);
    bool isShowWayPointsActive();
    bool hideWayPoints(Player* player);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Pet functions
    inline void SetPetOwner(Unit* owner) { m_PetOwner = owner; }
    void setUnitToFollow(Unit* pUnit) { m_UnitToFollow = pUnit; }
    Unit* getUnitToFollow() { return m_UnitToFollow; }

protected:
    Unit* m_UnitToFollow;   // used in scripts

    //////////////////////////////////////////////////////////////////////////////////////////
    // Totem functions
public:
    uint32_t m_totemspelltimer;
    uint32_t m_totemspelltime;
    SpellInfo const* totemspell;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Waypoint functions
public:
    virtual void waypointStarted(uint32_t /*nodeId*/, uint32_t /*pathId*/) { }
    virtual void waypointReached(uint32_t /*nodeId*/, uint32_t /*pathId*/) { }
    virtual void waypointPathEnded(uint32_t /*nodeId*/, uint32_t /*pathId*/) { }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell functions
public:
    void castSpell(Unit* caster, SpellInfo const* spellInfo, SpellCastTargets targets);
    SpellInfo const* getSpellEntry(uint32_t spellId);
    SpellCastTargets setSpellTargets(SpellInfo const* spellInfo, Unit* target, uint8_t targettype) const;

    std::list<AI_Spell*> m_spells;
    void addSpellToList(AI_Spell* sp);
    AI_Spell* getSpell(uint32_t entry);
    void setNextSpell(uint32_t spellId);
    void removeNextSpell(uint32_t spellId);

    //////////////////////////////////////////////////////////////////////////////////////////
    // script events
    // \brief: 
protected:
    scriptEventMap spellEvents;

protected:
    bool canEnterCombat;

    std::list<spawn_timed_emotes*>::iterator next_timed_emote;
    uint32_t timed_emote_expire;

    bool m_cannotReachTarget;
    SmallTimeTracker m_noTargetTimer;
    SmallTimeTracker m_cannotReachTimer;
    SmallTimeTracker m_updateTargetsTimer;
};
