/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#ifndef WOWSERVER_AIINTERFACE_H
#define WOWSERVER_AIINTERFACE_H

#include "Map/RecastIncludes.hpp"
#include "Server/IUpdatable.h"
#include "Units/Creatures/AIEvents.h"
#include "Units/Unit.h"
#include "Units/Creatures/CreatureDefines.hpp"
#include "Movement/UnitMovementManager.hpp"

#include <G3D/Vector3.h>

/// ms smoother server/client side moving vs less cpu/ less b/w
#define UNIT_MOVEMENT_INTERPOLATE_INTERVAL 400/*750*/

/// we most likely will have to kill players and only then check mobs
#define TARGET_UPDATE_INTERVAL_ON_PLAYER 1000

/// this is a multiple of PLAYER_TARGET_UPDATE_INTERVAL
#define TARGET_UPDATE_INTERVAL 5000
#define PLAYER_SIZE 1.5f

#define ENABLE_CREATURE_DAZE
#ifdef ENABLE_CREATURE_DAZE
#define CREATURE_SPELL_TO_DAZE 1604

/// for the beginners this means 45 degrees
#define CREATURE_DAZE_TRIGGER_ANGLE M_H_PI

/// minimal level of the target player to daze, from 3.3.0
#define CREATURE_DAZE_MIN_LEVEL 6
#endif

// not try to reposition creature to obtain perfect combat range
const float minWalkDistance = 2.0f;

//!!! it is in seconds and not Milliseconds
#define MOB_SPELLCAST_GLOBAL_COOLDOWN 2 //there are individual cooldown and global ones. Global cooldown stops mob from casting 1 instant spell on you per second
#define MOB_SPELLCAST_REFRESH_COOLDOWN_INTERVAL 2

//#define INHERIT_FOLLOWED_UNIT_SPEED 1

//Pathfinding stuff
#define VERTEX_SIZE 3
#define MAX_PATH_LENGTH 512
#define SMOOTH_PATH_STEP_SIZE   6.0f
#define SMOOTH_PATH_SLOP        0.4f

inline bool inRangeYZX(const float* v1, const float* v2, const float r, const float h)
{
    const float dx = v2[0] - v1[0];
    const float dy = v2[1] - v1[1]; // elevation
    const float dz = v2[2] - v1[2];
    return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
}

class MapMgr;
class Object;
class Creature;
class Unit;
class Player;
class WorldSession;
class SpellCastTargets;

enum WalkMode
{
    WALKMODE_SPRINT,
    WALKMODE_RUN,
    WALKMODE_WALK
};

enum SplinePriority
{
    SPLINE_PRIORITY_MOVEMENT,
    SPLINE_PRIORITY_REDIRECTION
};

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

enum AiState
{
    AI_STATE_IDLE       = 0,
    AI_STATE_ATTACKING  = 1,
    AI_STATE_CASTING    = 2,
    AI_STATE_FLEEING    = 3,
    AI_STATE_FOLLOWING  = 4,
    AI_STATE_EVADE      = 5,
    AI_STATE_MOVEWP     = 6,
    AI_STATE_FEAR       = 7,
    AI_STATE_UNFEARED   = 8,
    AI_STATE_WANDER     = 9,
    AI_STATE_STOPPED    = 10,
    AI_STATE_SCRIPTMOVE = 11,
    AI_STATE_SCRIPTIDLE = 12
};

enum MovementState
{
    MOVEMENTSTATE_MOVE,
    MOVEMENTSTATE_FOLLOW,
    MOVEMENTSTATE_STOP,
    MOVEMENTSTATE_FOLLOW_OWNER
};

enum CreatureState
{
    STOPPED,
    MOVING,
    ATTACKING
};


class SpellInfo;

const uint32_t AISPELL_ANY_DIFFICULTY = 4;

struct AI_Spell
{
    ~AI_Spell() { autocast_type = (uint32_t) - 1; }
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


typedef std::unordered_map<uint64_t, int32_t> TargetMap;

typedef std::set<Unit*> AssistTargetSet;
typedef std::map<uint32_t, AI_Spell*> SpellMap;


//MIT start
class SERVER_DECL AIInterface : public IUpdatable
{
    public:

        AIInterface();
        virtual ~AIInterface();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Waypoint / movement functions
    private:

        Movement::WaypointMovementScript mWaypointScriptType;
        int32_t mNextPoint;
        bool mWaitTimerSetOnWP;

    public:

        void setWaypointScriptType(Movement::WaypointMovementScript wp_script);
        Movement::WaypointMovementScript getWaypointScriptType();
        bool isWaypointScriptType(Movement::WaypointMovementScript wp_script);

        void setupAndMoveToNextWaypoint();
        void generateWaypointScriptCircle();
        void generateWaypointScriptRandom();
        void generateWaypointScriptForwad();
        void generateWaypointScriptWantedWP();
        void generateWaypointScriptPatrol();

        void updateOrientation();

        void setFormationMovement();
        void setFearRandomMovement();
        void setPetFollowMovement();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Waypoint functions
    private:

        Movement::WayPointMap* mWayPointMap;
        bool mWaypointMapIsLoadedFromDB;
        uint32_t mCurrentWaypoint;
        bool mMoveWaypointsBackwards;

        bool mShowWayPoints;
        bool mShowWayPointsBackwards;

    public:

        // \note wp may point to free'd memory after calling this, use bool addWayPointUnsafe(WayPoint* wp) instead to manually handle possible errors.
        void addWayPoint(Movement::WayPoint* waypoint);
        // caller delete wp if it wasn't added.
        bool addWayPointUnsafe(Movement::WayPoint* waypoint);

        Movement::WayPoint* getWayPoint(uint32_t waypointId);
        
        bool saveWayPoints();
        void deleteWayPointById(uint32_t waypointId);
        void deleteAllWayPoints();

        bool hasWayPoints();
        uint32_t getCurrentWayPointId();
        void changeWayPointId(uint32_t oldWaypointId, uint32_t newWaypointId);
        uint32_t getWayPointsCount();

        void setWayPointToMove(uint32_t waypointId);

        bool activateShowWayPoints(Player* player, bool showBackwards);
        void activateShowWayPointsBackwards(bool set);
        bool isShowWayPointsActive();
        bool isShowWayPointsBackwardsActive();
        bool hideWayPoints(Player* player);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spline functions
    private:

        uint32_t mWalkMode;

        //\note First element in the spline (m_currentMoveSpline[0]) is always the position the creature started moving from. 
        //      Index is always set to 1 when movement is started, as index 0 is referenced for first move.
        uint32_t mSplinePriority;

    public:

        void setFacing(float orientation);

        void setWalkMode(uint32_t mode);
        bool hasWalkMode(uint32_t mode) const;
        uint32_t getWalkMode() const;

        void setSplineFlying() const;
        bool isFlying();
        void unsetSplineFlying();

        void setSplineSprint();
        void setSplineRun();
        void setSplineWalk();

        void unsetSpline();

        void splineMoveKnockback(float x, float y, float z, float horizontal, float vertical);
        void splineMoveJump(float x, float y, float z, float o = 0, float speedZ = 5.0f, bool hugearc = false);
        void splineMoveFalling(float x, float y, float z, float o = 0);

        void splineMoveCharge(Unit* targetUnit, float distance = 3.0f);

        void generateSplinePathToTarget(Unit* targetUnit, float distance);
        void sendSplineMoveToPoint(LocationVector pos);
        bool generateAndSendSplinePath(float x, float y, float z, float o = 0.0f);

    //////////////////////////////////////////////////////////////////////////////////////////
    // AI Script functions
    private:

        AiScriptTypes mAiScriptType;

    public:

        void setAiScriptType(AiScriptTypes ai_type) { mAiScriptType = ai_type; }
        uint32_t getAiScriptType() { return mAiScriptType; }
        bool isAiScriptType(AiScriptTypes ai_type) { return ai_type == mAiScriptType; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // AI State functions
    private:

        uint32_t mAiState;

    public:

        void setAiState(uint32_t ai_state) { mAiState = ai_state; }
        void removeAiState(uint32_t ai_state) { mAiState &= ~ai_state; }
        uint32_t getAiState() { return mAiState; }
        bool isAiState(uint32_t ai_state) { return ai_state == mAiState; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Creature State functions
    private:

        CreatureState mCreatureState;

    public:

        void setCreatureState(CreatureState newState) { mCreatureState = newState; }
        bool isCreatureState(CreatureState newState) { return mCreatureState == newState; }
        CreatureState getCreatureState() { return mCreatureState; }

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


// MIT end
    public:

        // Misc
        void Init(Unit* un, AiScriptTypes at, Movement::WaypointMovementScript mt);
        void Init(Unit* un, AiScriptTypes at, Movement::WaypointMovementScript mt, Unit* owner);   // used for pets
        Unit* GetUnit() const;
        Unit* GetPetOwner() const;
        void DismissPet();
        void SetUnitToFollow(Unit* un);
        void SetUnitToFollow(uint64_t guid) { m_UnitToFollow = guid; };
        void ResetUnitToFollow() { m_UnitToFollow = 0; };

        void SetUnitToFear(Unit* un);
        void SetUnitToFear(uint64_t guid)  { m_UnitToFear = guid; };
        void ResetUnitToFear() { m_UnitToFear = 0; };

        void SetUnitToFollowBackup(Unit* un);
        void SetUnitToFollowBackup(uint64_t guid) { m_UnitToFollow_backup = guid; };
        void SetFollowDistance(float dist) { FollowDistance = dist; };
        void SetUnitToFollowAngle(float angle) { m_fallowAngle = angle; }
        bool setInFront(Unit* target);
        Unit* getUnitToFollow();
        uint64_t getUnitToFollowGUID() { return m_UnitToFollow; }
        Unit* getUnitToFollowBackup();
        uint64_t getUnitToFollowBackupGUID() { return m_UnitToFollow_backup; }
        Unit* getUnitToFear();
        uint64_t getUnitToFearGUID() { return m_UnitToFear; }
        Creature* getFormationLinkTarget();

        inline uint8_t getCurrentAgent() { return static_cast<uint8_t>(m_aiCurrentAgent); }
        void setCurrentAgent(AI_Agent agent) { m_aiCurrentAgent = agent; }
        uint32_t getThreatByGUID(uint64_t guid);
        uint32_t getThreatByPtr(Unit* obj);
        Unit* GetMostHated();
        Unit* GetSecondHated();
        bool modThreatByGUID(uint64_t guid, int32_t mod);
        bool modThreatByPtr(Unit* obj, int32_t mod);
        void RemoveThreatByGUID(uint64_t guid);
        void RemoveThreatByPtr(Unit* obj);
        inline AssistTargetSet GetAssistTargets() { return m_assistTargets; }
        inline void LockAITargets(bool lock)
        {
            lock ? m_aiTargetsLock.Acquire() : m_aiTargetsLock.Release();
        };
        inline TargetMap* GetAITargets() { return &m_aiTargets; }
        void addAssistTargets(Unit* Friends);
        void ClearHateList();
        void WipeHateList();
        void WipeTargetList();
        bool taunt(Unit* caster, bool apply = true);
        Unit* getTauntedBy();
        bool GetIsTaunted();
        Unit* getSoullinkedWith();
        void SetSoulLinkedWith(Unit* target);
        bool GetIsSoulLinked();
        inline size_t getAITargetsCount() { return m_aiTargets.size(); }
        inline uint32_t getOutOfCombatRange() { return m_outOfCombatRange; }
        void setOutOfCombatRange(uint32_t val) { m_outOfCombatRange = val; }

        // Spell
        void CastSpell(Unit* caster, SpellInfo const* spellInfo, SpellCastTargets targets);
        SpellInfo const* getSpellEntry(uint32_t spellId);
        SpellCastTargets setSpellTargets(SpellInfo const* spellInfo, Unit* target) const;
        AI_Spell* getSpell();
        void addSpellToList(AI_Spell* sp);

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

        void OnDeath(Object* pKiller);
        void AttackReaction(Unit* pUnit, uint32_t damage_dealt, uint32_t spellId = 0);
        void HealReaction(Unit* caster, Unit* victim, SpellInfo const* sp, uint32_t amount);
        void EventAiInterfaceParamsetFinish();
        void EventChangeFaction(Unit* ForceAttackersToHateThisInstead = NULL);    /// we have to tell our current enemies to stop attacking us, we should also forget about our targets

        // Update
        void Update(unsigned long time_passed);

        void SetReturnPosition();

        void _UpdateTotem(uint32_t p_time);

        // Calculation
        float _CalcAggroRange(Unit* target);
        void _CalcDestinationAndMove(Unit* target, float dist);
        float _CalcCombatRange(Unit* target, bool ranged);
        float _CalcDistanceFromHome();
        uint32_t _CalcThreat(uint32_t damage, SpellInfo const* sp, Unit* Attacker);

        void SetAllowedToEnterCombat(bool val) { m_AllowedToEnterCombat = val; }
        inline bool GetAllowedToEnterCombat(void) { return m_AllowedToEnterCombat; }

        void CheckTarget(Unit* target);

        bool m_canMove;

        //visibility
        uint32_t faction_visibility;

        bool onGameobject;

        bool m_canFlee;
        bool m_canCallForHelp;
        bool m_canRangedAttack;
        float m_FleeHealth;
        uint32_t m_FleeDuration;
        float m_CallForHelpHealth;
        uint32_t m_totemspelltimer;
        uint32_t m_totemspelltime;
        SpellInfo const* totemspell;

        uint32_t m_totalMoveTime;
        inline void AddStopTime(uint32_t Time) { m_moveTimer += Time; }
        inline void SetNextSpell(AI_Spell* sp) { m_nextSpell = sp; }
        Unit* getNextTarget();
        void setNextTarget(Unit* nextTarget);
        void setNextTarget(uint64_t nextTarget);
        void resetNextTarget();

        uint64_t m_formationLinkTarget;
        float m_formationFollowDistance;
        float m_formationFollowAngle;
        uint32_t m_formationLinkSqlId;

        void WipeReferences();
        TimedEmoteList* timed_emotes;
        inline void SetPetOwner(Unit* owner) { m_PetOwner = owner; }

        std::list<AI_Spell*> m_spells;

        bool waiting_for_cooldown;

        uint32_t next_spell_time;

        void CheckNextSpell(AI_Spell* sp)
        {
            if (m_nextSpell == sp)
                m_nextSpell = 0;
        }

        void ResetProcCounts();

        // deletes the old waypoint map as default. In case m_custom_waypoint_map is used, just call SetWaypointMap(NULL): this will delete m_custom_waypoint_map too.
        void SetWaypointMap(Movement::WayPointMap* m, bool delete_old_map = true);
        inline Movement::WayPointMap* GetWaypointMap() { return mWayPointMap; }
        void LoadWaypointMapFromDB(uint32_t spawnid);
        bool m_isGuard;
        bool m_isNeutralGuard;
        // bool m_fastMove;
        void setGuardTimer(uint32_t timer) { m_guardTimer = timer; }
        virtual void _UpdateCombat(uint32_t p_time);

    protected:

        bool UnsafeCanOwnerAttackUnit(Unit* pUnit);        /// this is designed for internal use only
        bool m_AllowedToEnterCombat;

        // Update
        void _UpdateTargets();
        void _UpdateMovement(uint32_t p_time);
        void _UpdateTimer(uint32_t p_time);

        bool m_updateAssist;
        bool m_updateTargets;
        uint32_t m_updateAssistTimer;
        uint32_t m_updateTargetsTimer;
        uint32_t m_updateTargetsTimer2;

        // Misc
        Unit* FindTarget();
        Unit* FindTargetForSpell(AI_Spell* sp);
        bool FindFriends(float dist);
        AI_Spell* m_nextSpell;
        uint64_t m_nextTarget;
        uint32_t m_fleeTimer;
        bool m_hasFleed;
        bool m_hasCalledForHelp;
        uint32_t m_outOfCombatRange;

        // Unit* gracefull_hit_on_target;
        Unit* m_Unit;
        Unit* m_PetOwner;
        float FollowDistance;
        float FollowDistance_backup;
        float m_fallowAngle;

        // std::set<AI_Target> m_aiTargets;
        Mutex m_aiTargetsLock;
        TargetMap m_aiTargets;
        AssistTargetSet m_assistTargets;

        AI_Agent m_aiCurrentAgent;

        Unit* tauntedBy;        // This mob will hit only tauntedBy mob.
        bool isTaunted;
        Unit* soullinkedWith;   // This mob can be hit only by a soul linked unit
        bool isSoulLinked;

        // Movement
        float m_walkSpeed;
        float m_runSpeed;
        float m_flySpeed;

        float m_last_target_x;
        float m_last_target_y;

    public:
        
        bool MoveTo(float x, float y, float z, float o = 0.0f);
        bool MoveDone() const;

        void SendCurrentMove(Player* plyr/*uint64_t guid*/);
        void SendMoveToPacket();
        bool StopMovement(uint32_t time);

        void AddSpline(float x, float y, float z);

        void UpdateSpeeds();

        void UpdateMovementSpline();

        void OnMoveCompleted();

        void MoveEvadeReturn();

        bool CreatePath(float x, float y, float z, bool onlytest = false);
        dtStatus findSmoothPath(const float* startPos, const float* endPos, const dtPolyRef* polyPath, const uint32_t polyPathSize, float* smoothPath, int* smoothPathSize, bool & usedOffmesh, const uint32_t maxSmoothPathSize, dtNavMesh* mesh, dtNavMeshQuery* query, dtQueryFilter & filter);
        bool getSteerTarget(const float* startPos, const float* endPos, const float minTargetDist, const dtPolyRef* path, const uint32_t pathSize, float* steerPos, unsigned char & steerPosFlag, dtPolyRef & steerPosRef, dtNavMeshQuery* query);
        uint32_t fixupCorridor(dtPolyRef* path, const uint32_t npath, const uint32_t maxPath, const dtPolyRef* visited, const uint32_t nvisited);

        //Path creation helpers
        bool CanCreatePath(float x, float y, float z) { return CreatePath(x, y, z, true); }

    protected:

        //Return position after attacking a mob
        float m_returnX;
        float m_returnY;
        float m_returnZ;

        float m_combatResetX;
        float m_combatResetY;
        float m_combatResetZ;

        float m_lastFollowX;
        float m_lastFollowY;

        uint64_t m_UnitToFollow;
        uint64_t m_UnitToFollow_backup; // used unly when forcing creature to wander (blind spell) so when effect wears off we can follow our master again (guardian)
        uint64_t m_UnitToFear;

        uint32_t m_timeToMove;
        uint32_t m_timeMoved;
        uint32_t m_moveTimer;
        uint32_t m_FearTimer;
        uint32_t m_WanderTimer;

        //Movement::WaypointMovementScript m_MovementType;
        MovementState m_MovementState;
        uint32_t m_guardTimer;
        int32_t m_currentHighestThreat;
        std::list<spawn_timed_emotes*>::iterator next_timed_emote;
        uint32_t timed_emote_expire;

        float last_updated_orientation;

    public:

        bool m_is_in_instance;
        bool skip_reset_hp;

        void WipeCurrentTarget();

        void SetCreatureProtoDifficulty(uint32_t entry);
        uint8_t GetDifficultyType();
};

#endif  //WOWSERVER_AIINTERFACE_H
