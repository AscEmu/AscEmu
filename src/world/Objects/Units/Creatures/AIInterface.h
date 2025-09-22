/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/RecastIncludes.hpp"
#include "Objects/Units/Creatures/AIEvents.h"
#include "Macros/AIInterfaceMacros.hpp"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Server/Script/ScriptEvent.hpp"
#include "Chat/ChatDefines.hpp"
#include "Storage/MySQLStructures.h"
#include <functional>
#include "Utilities/TimeTracker.hpp"

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
class CreatureAIScript;
class CreatureGroup;
class SpellInfo;

enum MovementGeneratorType : uint8_t;
enum SpellCastResult : uint8_t;

enum AI_SCRIPT_EVENT_TYPES
{
    onLoad              = 0,
    onEnterCombat       = 1,
    onLeaveCombat       = 2,
    onDied              = 3,
    onTargetDied        = 4,
    onAIUpdate          = 5,
    onCallForHelp       = 6,
    onRandomWaypoint    = 7,
    onDamageTaken       = 8,
    onFlee              = 9,
    onTaunt             = 10
};

enum AI_SCRIPT_ACTION_TYPES
{
    actionNone          = 0,
    actionSpell         = 1,
    actionSendMessage   = 2,
    actionPhaseChange   = 3
};

struct AI_SCRIPT_SENDMESSAGES
{
    uint32_t textId;
    float canche;
    uint32_t phase;
    float healthPrecent;
    uint32_t count;
    uint32_t maxCount;
};

typedef std::vector<std::unique_ptr<AI_SCRIPT_SENDMESSAGES>> definedEmoteVector;

enum ReactStates : uint8_t
{
    REACT_PASSIVE = 0,
    REACT_DEFENSIVE = 1,
    REACT_AGGRESSIVE = 2
};

enum AI_Agent : uint8_t
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

enum AISpellTargetType
{
    TARGET_SELF,
    TARGET_VARIOUS,
    TARGET_ATTACKING,
    TARGET_DESTINATION,
    TARGET_SOURCE,
    TARGET_RANDOM_FRIEND,
    TARGET_RANDOM_SINGLE,
    TARGET_RANDOM_DESTINATION,
    TARGET_CLOSEST,
    TARGET_FURTHEST,
    TARGET_CUSTOM,
    TARGET_FUNCTION
};

enum FleeState : uint8_t
{
    FLEE_NONE           = 0,
    FLEE_RANDOM_MOVE    = 1,
    FLEE_SEEK_ASSIST    = 2,
};

class SERVER_DECL CreatureAISpells
{
public:
    CreatureAISpells(SpellInfo const* spellInfo, float castChance, uint32_t targetType, uint32_t duration, uint32_t cooldown, bool forceRemove, bool isTriggered);
    ~CreatureAISpells() = default;

    SpellInfo const* mSpellInfo;
    float mCastChance;
    uint32_t mTargetType;
    uint8_t scriptType;

    std::function<Unit* ()> getTargetFunction = nullptr;

    std::unique_ptr<Util::SmallTimeTracker> mDurationTimer;
    std::unique_ptr<Util::SmallTimeTracker> mCooldownTimer;

    uint32_t mDuration;
    void setdurationTimer(uint32_t durationTimer);
    void setCooldownTimer(uint32_t cooldownTimer);
    uint32_t mCooldown;

    uint32_t mMaxCount;
    uint32_t mCastCount;
    void setMaxCastCount(uint32_t castCount);
    const uint32_t getMaxCastCount();
    const uint32_t getCastCount();
    void setCastCount(uint32_t count) { mCastCount = count; }
    void increaseCastCount() { ++mCastCount; }

    bool mForceRemoveAura;
    bool mIsTriggered;

    AI_SpellType spell_type;

    //Zyres: temp boolean to determine if its coming from db or not
    bool fromDB = false;

    // non db script messages
    struct AISpellEmotes
    {
        AISpellEmotes(std::string pText, uint8_t pType, uint32_t pSoundId)
        {
            mText = (!pText.empty() ? pText : "");
            mType = pType;
            mSoundId = pSoundId;
        }

        std::string mText;
        uint8_t mType;
        uint32_t mSoundId;
    };
    typedef std::vector<AISpellEmotes> AISpellEmoteArray;
    AISpellEmoteArray mAISpellEmote;

    void addDBEmote(uint32_t textId);
    void addEmote(std::string pText, uint8_t pType = CHAT_MSG_MONSTER_YELL, uint32_t pSoundId = 0);

    void sendRandomEmote(Unit* creatureAI);

    uint32_t mMaxStackCount;

    void setMaxStackCount(uint32_t stackCount);
    const uint32_t getMaxStackCount();

    float mMinPositionRangeToCast;
    float mMaxPositionRangeToCast;

    const bool isDistanceInRange(float targetDistance);
    void setMinMaxDistance(float minDistance, float maxDistance);

    // if it is not a random target type it sets the hp range when the creature can cast this spell
    // if it is a random target it controles when the spell can be cast based on the target hp
    float mMinHpRangeToCast;
    float mMaxHpRangeToCast;

    const bool isHpInPercentRange(float targetHp);
    void setMinMaxPercentHp(float minHp, float maxHp);

    typedef std::vector<uint32_t> ScriptPhaseList;
    ScriptPhaseList mPhaseList;

    void setAvailableForScriptPhase(std::vector<uint32_t> phaseVector);
    bool isAvailableForScriptPhase(uint32_t scriptPhase);

    uint32_t mAttackStopTimer;
    void setAttackStopTimer(uint32_t attackStopTime);
    uint32_t getAttackStopTimer();

    std::string mAnnouncement;
    void setAnnouncement(std::string announcement);
    void sendAnnouncement(Unit* pUnit);

    Unit* mCustomTargetCreature;
    void setCustomTarget(Unit* targetCreature);
    Unit* getCustomTarget();
};

using CreatureAISpellsArray = std::vector<std::unique_ptr<CreatureAISpells>>;
using UnitArray = std::vector<Unit*>;

static inline constexpr uint32_t AISPELL_ANY_DIFFICULTY = 4;
// Global spell cooldown or minimum time in millis to wait between spells
static inline constexpr uint32_t AISPELL_GLOBAL_COOLDOWN = 1200;

enum TargetFilter : uint32_t
{
    // Standard filters
    TargetFilter_None = 0,                              // 0
    TargetFilter_Closest = 1 << 0,                      // 1
    TargetFilter_Friendly = 1 << 1,                     // 2
    TargetFilter_NotCurrent = 1 << 2,                   // 4
    TargetFilter_Wounded = 1 << 3,                      // 8
    TargetFilter_SecondMostHated = 1 << 4,              // 16
    TargetFilter_Aggroed = 1 << 5,                      // 32
    TargetFilter_Corpse = 1 << 6,                       // 64
    TargetFilter_InMeleeRange = 1 << 7,                 // 128
    TargetFilter_InRangeOnly = 1 << 8,                  // 256
    TargetFilter_IgnoreSpecialStates = 1 << 9,          // 512 - not really a TargetFilter, more like requirement for spell
    TargetFilter_IgnoreLineOfSight = 1 << 10,           // 1024
    TargetFilter_Current = 1 << 11,                     // 2048
    TargetFilter_LowestHealth = 1 << 12,                // 4096
    TargetFilter_Health = 1 << 13,                      // 8192
    TargetFilter_AOE = 1 << 14,                         // 16348 - not really a Filter just no Target for AOE spells
    TargetFilter_Self = 1 << 15,                        // 32768 - mostlikely return ourself unless we set aura filtering and we dont have it
    TargetFilter_Caster = 1 << 16,                      // 65536 - Mana Based Class
    TargetFilter_Casting = 1 << 17,                     // 131072 - Target is Casting currently
    TargetFilter_Player = 1 << 18,                      // 262144 - Players Only

    // Predefined filters
    TargetFilter_ClosestFriendly = TargetFilter_Closest | TargetFilter_Friendly,                // 3
    TargetFilter_ClosestNotCurrent = TargetFilter_Closest | TargetFilter_NotCurrent,            // 5
    TargetFilter_WoundedFriendly = TargetFilter_Wounded | TargetFilter_Friendly,                // 10
    TargetFilter_FriendlyCorpse = TargetFilter_Corpse | TargetFilter_Friendly,                  // 66
    TargetFilter_ClosestFriendlyCorpse = TargetFilter_Closest | TargetFilter_FriendlyCorpse,    // 67
    TargetFilter_CurrentInRangeOnly = TargetFilter_Current | TargetFilter_InRangeOnly,          // 2304
    TargetFilter_WoundedFriendlyLowestHealth = TargetFilter_Wounded | TargetFilter_Friendly | TargetFilter_LowestHealth, // 4106
    TargetFilter_WoundedFriendlyLowestHealthInRange = TargetFilter_Wounded | TargetFilter_Friendly | TargetFilter_LowestHealth | TargetFilter_InRangeOnly, // 4362
    TargetFilter_CasterWhileCasting = TargetFilter_Casting | TargetFilter_Caster, // 196608
    TargetFilter_SelfBelowHealth = TargetFilter_Self | TargetFilter_Health  // 40960
};

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

class SERVER_DECL AIInterface
{
public:
    AIInterface();
    // todo: why virtual? -Appled
    virtual ~AIInterface();

    void Init(Unit* un, Unit* owner = nullptr);

    void initialiseScripts(uint32_t entry);
    void addEmoteFromDatabase(std::vector<MySQLStructure::CreatureAIScripts> const& scripts, definedEmoteVector& emoteVector);
    void addSpellFromDatabase(std::vector<MySQLStructure::CreatureAIScripts> const& scripts);

    Unit* getUnit() const;
    Unit* getPetOwner() const;
    Unit* getCurrentTarget() const;

    // Event Handler
    void handleEvent(uint32_t event, Unit* pUnit, uint32_t misc1);

    void update(unsigned long time_passed);
    void updateAIScript(unsigned long time_passed);
    void updateEmotes(unsigned long time_passed);

private:
    Unit* m_Unit = nullptr;
    Unit* m_PetOwner = nullptr;
    Unit* m_currentTarget = nullptr;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Combat
public:
    void combatStart(Unit* target);
    void combatStop();

    // Called when unit takes damage or get hits by spell
    void onHostileAction(Unit* pUnit, SpellInfo const* spellInfo = nullptr, bool ignoreThreatRedirects = false);

    void setCurrentTarget(Unit* pUnit);

    Unit* findTarget();
    void findFriends(float sqrtRange);

    bool canOwnerAttackUnit(Unit* pUnit) const;
    bool canOwnerAssistUnit(Unit* pUnit) const;
    bool isAlreadyAssisting(Unit const* helper) const;

    bool isEngaged() const;
    void setEngagedByAssist();

    void enterEvadeMode();

    bool canReachTarget() const;
    void setCannotReachTarget(bool cannotReach);

    void addBoundary(std::unique_ptr<AreaBoundary const> boundary, bool overrideDefault = false, bool reverseBoundary = false);
    void setDefaultBoundary();
    bool isWithinBoundary() const;
    bool isWithinBoundary(LocationVector const& pos) const;

    bool isIgnoringCreatureCombat() const;
    bool isIgnoringPlayerCombat() const;
    void setIgnoreCreatureCombat(bool apply);
    void setIgnorePlayerCombat(bool apply);

    bool isAllowedToEnterCombat() const;
    void setAllowedToEnterCombat(bool value);

private:
    // Called every 1500ms to find an enemy to attack or friends to assist unit
    void _updateTargets();
    // Called Eacht AIUpdate Tick to select a new Target
    bool _updateCurrentTarget();
    Unit* _selectCurrentTarget() const;

    bool _canEvade() const;

    std::unique_ptr<Util::SmallTimeTracker> m_targetUpdateTimer;
    std::set<Unit const*> m_assistTargets;
    bool m_isEngaged = false;
    bool m_isEngagedByAssist = false;

    bool m_cannotReachTarget = false;
    std::unique_ptr<Util::SmallTimeTracker> m_cannotReachTimer;

    std::vector<std::unique_ptr<AreaBoundary const>> m_boundaries;
    bool m_reverseBoundary = false;
    bool m_disableDynamicBoundary = false;
    std::unique_ptr<Util::SmallTimeTracker> m_boundaryCheckTime;

    bool m_canEnterCombat = true;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Combat AI
public:
    void attackStartIfCan(Unit* target);
    // Skips target checks
    void attackStartUnsafe(Unit* target);
    void attackStop();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Agent AI
public:
    bool canFlee() const;
    void setCanFlee(bool value);
    void stopFleeing();

    bool canCallForHelp() const;
    void setCanCallForHelp(bool value);
    // Used by LuaEngine
    void setCallForHelpHealth(float health);

    void handleAgentFlee(uint32_t p_time);
    void handleAgentCallForHelp();

private:
    // Returns true/false if found a friend
    bool _findFriendWhileFleeing();
    bool m_canFlee = false;
    float m_fleeHealth = 0.0f;
    uint32_t m_fleeDuration = 0;
    bool m_hasFleed = false;
    FleeState m_fleeState = FLEE_NONE;
    std::unique_ptr<Util::SmallTimeTracker> m_fleeTimer;

    bool m_canCallForHelp = false;
    float m_callForHelpHealth = 0.0f;
    bool m_hasCalledForHelp = false;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells
public:
    CreatureAISpellsArray const& getCreatureAISpells() const;

    CreatureAISpells* getAISpell(uint32_t spellId) const;
    CreatureAISpells* addAISpell(uint32_t spellId, float castChance, uint32_t targetType, uint32_t durationInSec = 0, uint32_t cooldownInSec = 0, bool forceRemove = false, bool isTriggered = false);
    void removeAISpell(uint32_t spellId);

    // Used only with player summons for now
    void updateOutOfCombatSpells(unsigned long time_passed);

private:
    CreatureAISpellsArray mCreatureAISpells;

    bool _cleanUpExpiredAISpell(CreatureAISpells const* aiSpell) const;

    std::unique_ptr<Util::SmallTimeTracker> m_outOfCombatSpellTimer;

    //////////////////////////////////////////////////////////////////////////////////////////
    // AI Agent functions
public:
    void selectCurrentAgent(Unit* target, uint32_t spellid);
    void initializeSpells();

    inline uint8_t getCurrentAgent() { return static_cast<uint8_t>(m_AiCurrentAgent); }
    void setCurrentAgent(AI_Agent agent) { m_AiCurrentAgent = agent; }

private:
    AI_Agent m_AiCurrentAgent = AGENT_NULL;
    bool m_canRangedAttack = false;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Combat functions
public:
    void setReactState(ReactStates st) { m_reactState = st; }
    ReactStates getReactState() const { return m_reactState; }
    bool hasReactState(ReactStates state) const { return (m_reactState == state); }
    void initializeReactState();

private:
    ReactStates m_reactState = REACT_AGGRESSIVE;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Combat behavior
private:
    bool mIsCombatDisabled = false;
    bool mIsMeleeDisabled = false;
    bool mIsRangedDisabled = false;
    bool mIsCastDisabled = false;
    bool mIsTargetingDisabled = false;

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
    LocationVector m_lasttargetPosition{ 0, 0, 0, 0 };

    bool isGuard() const { return m_isNeutralGuard; }
    void setGuard(bool value) { m_isNeutralGuard = value; }
    float calcCombatRange(Unit* target, bool ranged);

    void eventAiInterfaceParamsetFinish();
    TimedEmoteList* timed_emotes = nullptr;

    bool moveTo(float x, float y, float z, float o = 0.0f, bool running = false);
    void calcDestinationAndMove(Unit* target, float dist);

    void initGroupThreat(Unit* target);
    void instanceCombatProgress(bool activate);

    void eventForceRedirected(Unit* pUnit, uint32_t misc1);
    void eventHostileAction(Unit* pUnit, uint32_t misc1);
    void eventUnitDied(Unit* pUnit, uint32_t misc1);
    void eventUnwander(Unit* pUnit, uint32_t misc1);
    void eventWander(Unit* pUnit, uint32_t misc1);
    void eventUnfear(Unit* pUnit, uint32_t misc1);
    void eventFear(Unit* pUnit, uint32_t misc1);
    void eventFollowOwner(Unit* pUnit, uint32_t misc1);
    void eventDamageTaken(Unit* pUnit, uint32_t misc1);
    void eventLeaveCombat(Unit* pUnit, uint32_t misc1);
    void eventEnterCombat(Unit* pUnit, uint32_t misc1);
    void eventOnTaunt(Unit* pUnit);
    void eventOnLoad();
    void eventChangeFaction(Unit* ForceAttackersToHateThisInstead = NULL);    /// we have to tell our current enemies to stop attacking us, we should also forget about our targets
    void eventOnTargetDied(Object* pKiller);

    float calcAggroRange(Unit* target);
    
    void updateAgent(uint32_t p_time);
    void updateTotem(uint32_t p_time);

    void handleAgentMelee();
    void handleAgentRanged();
    void handleAgentSpell(uint32_t spellId);

    bool m_isNeutralGuard = false;
    uint32_t faction_visibility = 0;

    // Difficulty
    void setCreatureProtoDifficulty(uint32_t entry);
    uint8_t getDifficultyType();
    bool m_is_in_instance = false;

    uint8_t internalPhase = 0;

    std::vector<MySQLStructure::CreatureAIScripts> onLoadScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onCombatStartScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onAIUpdateScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onLeaveCombatScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onDiedScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onKilledScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onCallForHelpScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onRandomWaypointScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onDamageTakenScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onFleeScripts;
    std::vector<MySQLStructure::CreatureAIScripts> onTauntScripts;

private:
    definedEmoteVector mEmotesOnLoad;
    definedEmoteVector mEmotesOnCombatStart;
    definedEmoteVector mEmotesOnLeaveCombat;
    definedEmoteVector mEmotesOnTargetDied;
    definedEmoteVector mEmotesOnAIUpdate;
    definedEmoteVector mEmotesOnDied;
    definedEmoteVector mEmotesOnDamageTaken;
    definedEmoteVector mEmotesOnCallForHelp;
    definedEmoteVector mEmotesOnFlee;
    definedEmoteVector mEmotesOnTaunt;
    definedEmoteVector mEmotesOnRandomWaypoint;

public:
    void sendStoredText(definedEmoteVector& store, Unit* target);

    Unit* mCurrentSpellTarget = nullptr;

    //////////////////////////////////////////////////////////////////////////////////////////
    // target
    Unit* getBestPlayerTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f);
    Unit* getBestUnitTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f);
    Unit* getBestTargetInArray(UnitArray& pTargetArray, TargetFilter pFilter);
    Unit* getNearestTargetInArray(UnitArray& pTargetArray);
    Unit* getSecondMostHatedTargetInArray(UnitArray& pTargetArray);
    bool isValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Movement functions
public:
    virtual void onMovementGeneratorFinalized(MovementGeneratorType /*type*/) { }

    // Called at waypoint reached or point movement finished
    virtual void movementInform(uint32_t /*type*/, uint32_t /*id*/);

    bool canCreatePath(float x, float y, float z);
    dtStatus findSmoothPath(const float* startPos, const float* endPos, const dtPolyRef* polyPath, const uint32_t polyPathSize, float* smoothPath, uint32_t* smoothPathSize, bool & usedOffmesh, const uint32_t maxSmoothPathSize, dtNavMesh* mesh, dtNavMeshQuery* query, dtQueryFilter & filter);
    bool getSteerTarget(const float* startPos, const float* endPos, const float minTargetDist, const dtPolyRef* path, const uint32_t pathSize, float* steerPos, unsigned char & steerPosFlag, dtPolyRef & steerPosRef, dtNavMeshQuery* query);
    uint32_t fixupCorridor(dtPolyRef* path, const uint32_t npath, const uint32_t maxPath, const dtPolyRef* visited, const uint32_t nvisited);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Waypoint functions
private:
    bool mShowWayPoints = false;

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
    inline void setPetOwner(Unit* owner) { m_PetOwner = owner; }
    void setUnitToFollow(Unit* pUnit) { m_UnitToFollow = pUnit; }
    Unit* getUnitToFollow() { return m_UnitToFollow; }

protected:
    Unit* m_UnitToFollow = nullptr;   // used in scripts

    //////////////////////////////////////////////////////////////////////////////////////////
    // Totem functions
public:
    uint32_t m_totemspelltimer = 0;
    uint32_t m_totemspelltime = 0;
    SpellInfo const* totemspell = nullptr;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Waypoint functions
public:
    virtual void waypointStarted(uint32_t /*nodeId*/, uint32_t /*pathId*/);
    virtual void waypointReached(uint32_t /*nodeId*/, uint32_t /*pathId*/);
    virtual void waypointPathEnded(uint32_t /*nodeId*/, uint32_t /*pathId*/);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell functions
public:
    void castSpell(Unit* caster, SpellInfo const* spellInfo, SpellCastTargets targets);
    SpellInfo const* getSpellEntry(uint32_t spellId);
    SpellCastTargets setSpellTargets(SpellInfo const* spellInfo, Unit* target, uint8_t targettype) const;

    //addAISpell(spellID, Chance, TargetType, Duration (s), waitBeforeNextCast (s))

    std::list<std::unique_ptr<AI_Spell>> m_spells;
    void addSpellToList(std::unique_ptr<AI_Spell> sp);
    AI_Spell* getSpell(uint32_t entry);
    void setNextSpell(uint32_t spellId);
    void removeNextSpell(uint32_t spellId);

    //////////////////////////////////////////////////////////////////////////////////////////
    // spell

    SpellCastResult castAISpell(CreatureAISpells* aiSpell);
    SpellCastResult castAISpell(uint32_t aiSpellId);
    bool hasAISpell(CreatureAISpells* aiSpell);
    bool hasAISpell(uint32_t SpellId);
    SpellCastResult castSpellOnRandomTarget(CreatureAISpells* AiSpell);
    void UpdateAISpells();

    CreatureAISpells* mLastCastedSpell = nullptr;

    std::unique_ptr<Util::SmallTimeTracker> mSpellWaitTimer;

    //////////////////////////////////////////////////////////////////////////////////////////
    // script events
    // \brief: 
protected:
    scriptEventMap spellEvents;

protected:
    TimedEmoteList::iterator next_timed_emote;
    uint32_t timed_emote_expire = 0xFFFFFFFF;

    std::unique_ptr<Util::SmallTimeTracker> m_noTargetTimer;
};
