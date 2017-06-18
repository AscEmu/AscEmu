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

/// this is required so creature will not try to reposition itself to obtain perfect combat range. Not using this might lead to exploits
#define DISTANCE_TO_SMALL_TO_WALK 2.0f

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
    AI_STATE_IDLE,
    AI_STATE_ATTACKING,
    AI_STATE_CASTING,
    AI_STATE_FLEEING,
    AI_STATE_FOLLOWING,
    AI_STATE_EVADE,
    AI_STATE_MOVEWP,
    AI_STATE_FEAR,
    AI_STATE_UNFEARED,
    AI_STATE_WANDER,
    AI_STATE_STOPPED,
    AI_STATE_SCRIPTMOVE,
    AI_STATE_SCRIPTIDLE
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

const uint32 AISPELL_ANY_DIFFICULTY = 4;

struct AI_Spell
{
    ~AI_Spell() { autocast_type = (uint32) - 1; }
    uint32 entryId;
    uint32 instance_mode;
    uint16 agent;
    uint32 procChance;
    SpellInfo* spell;
    uint8 spellType;
    uint8 spelltargetType;
    uint32 cooldown;
    uint32 cooldowntime;
    uint32 procCount;
    uint32 procCounter;
    float floatMisc1;
    uint32 Misc2;
    float minrange;
    float maxrange;
    uint32 autocast_type;
};


typedef std::unordered_map<uint64, int32> TargetMap;

typedef std::set<Unit*> AssistTargetSet;
typedef std::map<uint32, AI_Spell*> SpellMap;

//MIT start
class SERVER_DECL AIInterface : public IUpdatable
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Waypoint functions
    private:

        Movement::WaypointMovementScript mWaypointScriptType;

    public:

        void setWaypointScriptType(Movement::WaypointMovementScript wp_script) { mWaypointScriptType = wp_script; }
        Movement::WaypointMovementScript getWaypointScriptType() { return mWaypointScriptType; }
        bool isWaypointScriptType(Movement::WaypointMovementScript wp_script) { return wp_script == mWaypointScriptType; }

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

        AiState mAiState;

    public:

        void setAiState(AiState ai_state) { mAiState = ai_state; }
        uint32_t getAiState() { return mAiState; }
        bool isAiState(AiState ai_state) { return ai_state == mAiState; }

// MIT end
    public:

        AIInterface();
        virtual ~AIInterface();

        // Misc
        void Init(Unit* un, AiScriptTypes at, Movement::WaypointMovementScript mt);
        void Init(Unit* un, AiScriptTypes at, Movement::WaypointMovementScript mt, Unit* owner);   /// used for pets
        Unit* GetUnit() const;
        Unit* GetPetOwner() const;
        void DismissPet();
        void SetUnitToFollow(Unit* un);
        void SetUnitToFollow(uint64 guid) { m_UnitToFollow = guid; };
        void ResetUnitToFollow() { m_UnitToFollow = 0; };

        void SetUnitToFear(Unit* un);
        void SetUnitToFear(uint64 guid)  { m_UnitToFear = guid; };
        void ResetUnitToFear() { m_UnitToFear = 0; };

        void SetUnitToFollowBackup(Unit* un);
        void SetUnitToFollowBackup(uint64 guid) { m_UnitToFollow_backup = guid; };
        void SetFollowDistance(float dist) { FollowDistance = dist; };
        void SetUnitToFollowAngle(float angle) { m_fallowAngle = angle; }
        bool setInFront(Unit* target);
        Unit* getUnitToFollow();
        uint64 getUnitToFollowGUID() { return m_UnitToFollow; }
        Unit* getUnitToFollowBackup();
        uint64 getUnitToFollowBackupGUID() { return m_UnitToFollow_backup; }
        Unit* getUnitToFear();
        uint64 getUnitToFearGUID() { return m_UnitToFear; }
        Creature* getFormationLinkTarget();
        void setCreatureState(CreatureState state) { m_creatureState = state; }
        
        

        inline uint8 getCurrentAgent() { return static_cast<uint8>(m_aiCurrentAgent); }
        void setCurrentAgent(AI_Agent agent) { m_aiCurrentAgent = agent; }
        uint32 getThreatByGUID(uint64 guid);
        uint32 getThreatByPtr(Unit* obj);
        Unit* GetMostHated();
        Unit* GetSecondHated();
        bool modThreatByGUID(uint64 guid, int32 mod);
        bool modThreatByPtr(Unit* obj, int32 mod);
        void RemoveThreatByGUID(uint64 guid);
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
        inline uint32 getOutOfCombatRange() { return m_outOfCombatRange; }
        void setOutOfCombatRange(uint32 val) { m_outOfCombatRange = val; }

        // Spell
        void CastSpell(Unit* caster, SpellInfo* spellInfo, SpellCastTargets targets);
        SpellInfo* getSpellEntry(uint32 spellId);
        SpellCastTargets setSpellTargets(SpellInfo* spellInfo, Unit* target);
        AI_Spell* getSpell();
        void addSpellToList(AI_Spell* sp);
        ///\todo don't use this until i finish it !!
        /// void CancelSpellCast();

        /// Event Handler
        void HandleEvent(uint32 event, Unit* pUnit, uint32 misc1);

        void EventForceRedirected(Unit* pUnit, uint32 misc1);
        void EventHostileAction(Unit* pUnit, uint32 misc1);
        void EventUnitDied(Unit* pUnit, uint32 misc1);
        void EventUnwander(Unit* pUnit, uint32 misc1);
        void EventWander(Unit* pUnit, uint32 misc1);
        void EventUnfear(Unit* pUnit, uint32 misc1);
        void EventFear(Unit* pUnit, uint32 misc1);
        void EventFollowOwner(Unit* pUnit, uint32 misc1);
        void EventDamageTaken(Unit* pUnit, uint32 misc1);
        void EventLeaveCombat(Unit* pUnit, uint32 misc1);
        void EventEnterCombat(Unit* pUnit, uint32 misc1);

        void OnDeath(Object* pKiller);
        void AttackReaction(Unit* pUnit, uint32 damage_dealt, uint32 spellId = 0);
        void HealReaction(Unit* caster, Unit* victim, SpellInfo* sp, uint32 amount);
        void EventAiInterfaceParamsetFinish();
        void EventChangeFaction(Unit* ForceAttackersToHateThisInstead = NULL);    /// we have to tell our current enemies to stop attacking us, we should also forget about our targets

        // Update
        void Update(unsigned long time_passed);

        void SetReturnPosition();

        void _UpdateTotem(uint32 p_time);

        // Movement
        void SendMoveToPacket();
        //void SendMoveToSplinesPacket(std::list<Waypoint> wp, bool run);
        bool MoveTo(float x, float y, float z);
        void UpdateSpeeds();

        //Move flag updating
    bool Flying() const;
    void SetFly() const;
    void SetSprint();
    void SetRun();
    void SetWalk();

    void SetWalkMode(uint32 mode);
    bool HasWalkMode(uint32 mode) const;
    uint32 GetWalkMode() const;

    void StopFlying();

        //Movement::Spline::MoveSpline m_spline;
        uint32 m_walkMode;

        void UpdateMove();
        void SendCurrentMove(Player* plyr/*uint64 guid*/);
        bool StopMovement(uint32 time);
        uint32 getCurrentWaypoint() { return m_currentWaypoint; }
        void changeWayPointID(uint32 oldwpid, uint32 newwpid);
        /// \note Adds a WayPoint, handling possible errors occurred when adding it. Pay attention: wp may point to free'd memory after calling this, use bool addWayPoint(WayPoint* wp) instead to manually handle possible errors.
        void addWayPoint(Movement::WayPoint* wp);
        /// returns true if the WayPoint was added, false otherwise. The caller MUST delete wp if it wasn't added.
        bool addWayPointUnsafe(Movement::WayPoint* wp);
        bool saveWayPoints();
        bool showWayPoints(Player* pPlayer, bool Backwards);
        bool hideWayPoints(Player* pPlayer);
        Movement::WayPoint* getWayPoint(uint32 wpid);
        void deleteWayPoint(uint32 wpid);
        void deleteWaypoints();
        inline bool hasWaypoints() { return m_waypoints != NULL; }
        //inline void setMoveType(uint32 movetype) { m_moveType = movetype; }

        //inline uint32 getMoveType() { return m_moveType; }
        void setWaypointToMove(uint32 id) { m_currentWaypoint = id; }
        bool IsFlying();

        // Calculation
        float _CalcAggroRange(Unit* target);
        void _CalcDestinationAndMove(Unit* target, float dist);
        float _CalcCombatRange(Unit* target, bool ranged);
        float _CalcDistanceFromHome();
        uint32 _CalcThreat(uint32 damage, SpellInfo* sp, Unit* Attacker);

        void SetAllowedToEnterCombat(bool val) { m_AllowedToEnterCombat = val; }
        inline bool GetAllowedToEnterCombat(void) { return m_AllowedToEnterCombat; }

        void CheckTarget(Unit* target);
        

        // Movement
        bool m_canMove;
        bool m_WayPointsShowing;
        bool m_WayPointsShowBackwards;
        uint32 m_currentWaypoint;
        bool m_moveBackward;

        //visibility
        uint32 faction_visibility;

        bool onGameobject;
        CreatureState m_creatureState;
        size_t GetWayPointsCount()
        {
            if (m_waypoints && !m_waypoints->empty())
                return m_waypoints->size() - 1;    /* ignore 0 */
            else
                return 0;
        }
        bool m_canFlee;
        bool m_canCallForHelp;
        bool m_canRangedAttack;
        float m_FleeHealth;
        uint32 m_FleeDuration;
        float m_CallForHelpHealth;
        uint32 m_totemspelltimer;
        uint32 m_totemspelltime;
        SpellInfo* totemspell;

        uint32 m_totalMoveTime;
        inline void AddStopTime(uint32 Time) { m_moveTimer += Time; }
        inline void SetNextSpell(AI_Spell* sp) { m_nextSpell = sp; }
        Unit* getNextTarget();
        void setNextTarget(Unit* nextTarget);
        void setNextTarget(uint64 nextTarget);
        void resetNextTarget();

        uint64 m_formationLinkTarget;
        float m_formationFollowDistance;
        float m_formationFollowAngle;
        uint32 m_formationLinkSqlId;

        void WipeReferences();
        TimedEmoteList* timed_emotes;
        inline void SetPetOwner(Unit* owner) { m_PetOwner = owner; }

        std::list<AI_Spell*> m_spells;

        bool disable_combat;

        bool disable_melee;
        bool disable_ranged;
        bool disable_spell;
        bool disable_targeting;

        bool waiting_for_cooldown;

        uint32 next_spell_time;

        void CheckNextSpell(AI_Spell* sp)
        {
            if (m_nextSpell == sp)
                m_nextSpell = 0;
        }

        void ResetProcCounts();

        /// deletes the old waypoint map as default. In case m_custom_waypoint_map is used, just call SetWaypointMap(NULL): this will delete m_custom_waypoint_map too.
        void SetWaypointMap(Movement::WayPointMap* m, bool delete_old_map = true);
        inline Movement::WayPointMap* GetWaypointMap() { return m_waypoints; }
        void LoadWaypointMapFromDB(uint32 spawnid);
        bool m_isGuard;
        bool m_isNeutralGuard;
        // bool m_fastMove;
        void setGuardTimer(uint32 timer) { m_guardTimer = timer; }
        virtual void _UpdateCombat(uint32 p_time);

    protected:

        bool UnsafeCanOwnerAttackUnit(Unit* pUnit);        /// this is designed for internal use only
        bool m_AllowedToEnterCombat;

        // Update
        void _UpdateTargets();
        void _UpdateMovement(uint32 p_time);
        void _UpdateTimer(uint32 p_time);
        void AddSpline(float x, float y, float z);
        bool Move(float & x, float & y, float & z);
        void OnMoveCompleted();

        void MoveEvadeReturn();

        bool CreatePath(float x, float y, float z,  bool onlytest = false);
        dtStatus findSmoothPath(const float* startPos, const float* endPos, const dtPolyRef* polyPath, const uint32 polyPathSize, float* smoothPath, int* smoothPathSize, bool & usedOffmesh, const uint32 maxSmoothPathSize, dtNavMesh* mesh, dtNavMeshQuery* query, dtQueryFilter & filter);
        bool getSteerTarget(const float* startPos, const float* endPos, const float minTargetDist, const dtPolyRef* path, const uint32 pathSize, float* steerPos, unsigned char & steerPosFlag, dtPolyRef & steerPosRef, dtNavMeshQuery* query);
        uint32 fixupCorridor(dtPolyRef* path, const uint32 npath, const uint32 maxPath, const dtPolyRef* visited, const uint32 nvisited);

        bool m_updateAssist;
        bool m_updateTargets;
        uint32 m_updateAssistTimer;
        uint32 m_updateTargetsTimer;
        uint32 m_updateTargetsTimer2;

        // Misc
        Unit* FindTarget();
        Unit* FindTargetForSpell(AI_Spell* sp);
        bool FindFriends(float dist);
        AI_Spell* m_nextSpell;
        uint64 m_nextTarget;
        uint32 m_fleeTimer;
        bool m_hasFleed;
        bool m_hasCalledForHelp;
        uint32 m_outOfCombatRange;

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

        Unit* tauntedBy;        /// This mob will hit only tauntedBy mob.
        bool isTaunted;
        Unit* soullinkedWith;   /// This mob can be hit only by a soul linked unit
        bool isSoulLinked;

        // Movement
        float m_walkSpeed;
        float m_runSpeed;
        float m_flySpeed;

        float m_last_target_x;
        float m_last_target_y;


        /**
        Splines
        \note First element in the spline (m_currentMoveSpline[0]) is always the position the creature started moving from. Index is always set to 1 when movement is started, as index 0 is referenced for first move.
        */
        //std::vector<::Movement::Spline::SplinePoint> m_currentMoveSpline;
        //::Movement::Spline::MoveSpline m_moveSpline;
        //uint32 m_currentMoveSplineIndex;
        //uint32 m_currentSplineUpdateCounter;
        //float m_currentSplineFinalOrientation;
        //float m_splinetrajectoryVertical;
        //uint32 m_splinetrajectoryTime;
        //uint32 m_currentSplineTotalMoveTime;
        uint32 m_splinePriority;

        //Return position after attacking a mob
        float m_returnX;
        float m_returnY;
        float m_returnZ;

        float m_combatResetX;
        float m_combatResetY;
        float m_combatResetZ;

        float m_lastFollowX;
        float m_lastFollowY;
        //typedef std::map<uint32, WayPoint*> WayPointMap;

        uint64 m_UnitToFollow;
        uint64 m_UnitToFollow_backup;   /// used unly when forcing creature to wander (blind spell) so when effect wears off we can follow our master again (guardian)
        uint64 m_UnitToFear;

        uint32 m_timeToMove;
        uint32 m_timeMoved;
        uint32 m_moveTimer;
        uint32 m_FearTimer;
        uint32 m_WanderTimer;

        //Movement::WaypointMovementScript m_MovementType;
        MovementState m_MovementState;
        uint32 m_guardTimer;
        int32 m_currentHighestThreat;
        std::list<spawn_timed_emotes*>::iterator next_timed_emote;
        uint32 timed_emote_expire;
    private:

        /// specifies if m_waypoints was loaded from DB, so shared between other AIInterface instances.
        bool m_waypointsLoadedFromDB;
        Movement::WayPointMap* m_waypoints;

    public:

        bool m_is_in_instance;
        bool skip_reset_hp;

        void WipeCurrentTarget();

        void UpdateMovementSpline();
    bool MoveDone() const;
        bool CanCreatePath(float x, float y, float z) { return CreatePath(x, y, z, true); }
        void MoveKnockback(float x, float y, float z, float horizontal, float vertical);
        void MoveJump(float x, float y, float z, float o = 0, float speedZ = 5.0f, bool hugearc = false);
        void MoveTeleport(float x, float y, float z, float o = 0);
        void MoveFalling(float x, float y, float z, float o = 0);
        bool MoveCharge(float x, float y, float z);

        void SetCreatureProtoDifficulty(uint32 entry);
        uint8 GetDifficultyType();

};


#endif  //WOWSERVER_AIINTERFACE_H
