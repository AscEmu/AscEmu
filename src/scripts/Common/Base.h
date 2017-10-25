/*
 * Moon++ Scripts for Ascent MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BASE_H
#define _BASE_H

#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/MapScriptInterface.h"
#include <Spell/Customization/SpellCustomizations.hpp>
#include "../Common/Instance_Base.h"

#define MOONSCRIPT_FACTORY_FUNCTION(ClassName, ParentClassName)\
public:\
    ADD_CREATURE_FACTORY_FUNCTION(ClassName);\
    typedef ParentClassName ParentClass;


enum EventType
{
    Event_OnCombatStart,
    Event_OnTargetDied,
    Event_OnDied,
    Event_OnTaunt
};

enum BehaviorType
{
    Behavior_Default,
    Behavior_Melee,
    Behavior_Ranged,
    Behavior_Spell,
    Behavior_Flee,
    Behavior_CallForHelp
};

struct EmoteDesc
{
    EmoteDesc(const char* pText, uint8 pType, uint32 pSoundId)
    {
        mText = (pText && strlen(pText) > 0) ? pText : "";
        mType = pType;
        mSoundId = pSoundId;
    }

    std::string mText;
    uint8 mType;
    uint32 mSoundId;
};

struct LocationExtra
{
    float x;
    float y;
    float z;
    float o;
    uint32 addition;
};

struct LootDesc
{
    uint32 mItemID;
    uint32 mChance;
    uint32 mMinCount;
    uint32 mMaxCount;
    uint32 mFFA;
};

enum TargetGenerator
{
    // Self
    TargetGen_Self,                         // Target self (Note: doesn't always mean self, also means the spell can choose various target)

    // Current
    TargetGen_Current,                      // Current highest aggro (attacking target)
    TargetGen_Destination,                  // Target is a destination coordinates (X, Y, Z)

    // Second most hated
    TargetGen_SecondMostHated,              // Second highest aggro

    // Predefined target
    TargetGen_Predefined,                   // Pre-defined target unit

    // Random Unit
    TargetGen_RandomUnit,                   // Random target unit (players, totems, pets, etc.)
    TargetGen_RandomUnitDestination,        // Random destination coordinates (X, Y, Z)
    TargetGen_RandomUnitApplyAura,          // Random target unit to self cast aura

    // Random Player
    TargetGen_RandomPlayer,                 // Random target player
    TargetGen_RandomPlayerDestination,      // Random player destination coordinates (X, Y, Z)
    TargetGen_RandomPlayerApplyAura         // Random target player to self cast aura
};

enum TargetFilter
{
    // Standard filters
    TargetFilter_None                   = 0,            // 0
    TargetFilter_Closest                = 1 << 0,       // 1
    TargetFilter_Friendly               = 1 << 1,       // 2
    TargetFilter_NotCurrent             = 1 << 2,       // 4
    TargetFilter_Wounded                = 1 << 3,       // 8
    TargetFilter_SecondMostHated        = 1 << 4,       // 16
    TargetFilter_Aggroed                = 1 << 5,       // 32
    TargetFilter_Corpse                 = 1 << 6,       // 64
    TargetFilter_InMeleeRange           = 1 << 7,       // 128
    TargetFilter_InRangeOnly            = 1 << 8,       // 256
    TargetFilter_IgnoreSpecialStates    = 1 << 9,       // 512 - not really a TargetFilter, more like requirement for spell
    TargetFilter_IgnoreLineOfSight      = 1 << 10,      // 1024

    // Predefined filters
    TargetFilter_ClosestFriendly        = TargetFilter_Closest | TargetFilter_Friendly,         // 3
    TargetFilter_ClosestNotCurrent      = TargetFilter_Closest | TargetFilter_NotCurrent,       // 5
    TargetFilter_WoundedFriendly        = TargetFilter_Wounded | TargetFilter_Friendly,         // 10
    TargetFilter_FriendlyCorpse         = TargetFilter_Corpse | TargetFilter_Friendly,          // 66
    TargetFilter_ClosestFriendlyCorpse  = TargetFilter_Closest | TargetFilter_FriendlyCorpse    // 67
};

enum RangeStatus
{
    RangeStatus_Ok,
    RangeStatus_TooFar,
    RangeStatus_TooClose
};

class TargetType;
class SpellDesc;
class MoonScriptCreatureAI;
class MoonScriptBossAI;
struct EventStruct;
class Unit;
struct SpellEntry;

typedef void(*EventFunc)(MoonScriptCreatureAI* pCreatureAI, int32 pMiscVal);
typedef void(*SpellFunc)(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);
typedef std::vector<EmoteDesc*> EmoteArray;
typedef std::vector<Player*> PlayerArray;
typedef std::vector<Unit*> UnitArray;
typedef std::vector<SpellDesc*> SpellDescArray;
typedef std::list<SpellDesc*> SpellDescList;
typedef std::pair<int32, SpellDesc*> PhaseSpellPair;
typedef std::vector<PhaseSpellPair> PhaseSpellArray;
typedef std::pair<int32, int32> TimerPair;
typedef std::vector<TimerPair> TimerArray;
typedef std::vector<LootDesc> LootTable;
typedef std::pair<RangeStatus, float> RangeStatusPair;
typedef std::vector<EventStruct*> EventArray;


struct EventStruct
{
    EventStruct(int32 pEventId, int32 pEventTimer, EventFunc pEvent, bool pRepeatable, bool pFinished, int32 pEventTimerConst, int32 pMiscVal = 0)
    {
        mEventId = pEventId;
        mEventTimer = pEventTimer;
        mEvent = pEvent;
        mRepeatable = pRepeatable;
        mFinished = pFinished;
        mEventTimerConst = pEventTimerConst;
        mMiscVal = pMiscVal;
    }

    int32 mEventId;
    int32 mEventTimer;
    EventFunc mEvent;
    bool mRepeatable;
    bool mFinished;
    int32 mEventTimerConst;
    int32 mMiscVal;
};


//////////////////////////////////////////////////////////////////////////////////////////
//Class TargetType
class TargetType
{
    public:
    TargetType(uint32 pTargetGen = TargetGen_Self, TargetFilter pTargetFilter = TargetFilter_None, uint32 pMinTargetNumber = 0, uint32 pMaxTargetNumber = 0);
    ~TargetType();

    uint32            mTargetGenerator;    // Defines what kind of target should we try to find
    TargetFilter    mTargetFilter;        // Defines filter of target
    uint32            mTargetNumber[2];    // 0: Defines min. number of creature on hatelist (0 - any, 1 - the most hated etc.)
    // 1: Defines max. number of creature on hatelist (0 - any, HateList.size + 1 - the least hated etc.)
};

// Pre-made TargetTypes
#define Target_Self TargetType()
#define Target_Current TargetType(TargetGen_Current)
#define Target_SecondMostHated TargetType(TargetGen_SecondMostHated)
#define Target_Destination TargetType(TargetGen_Destination)
#define Target_Predefined TargetType(TargetGen_Predefined)
#define Target_RandomPlayer TargetType(TargetGen_RandomPlayer)
#define Target_RandomPlayerNotCurrent TargetType(TargetGen_RandomPlayer, TargetFilter_NotCurrent)
#define Target_RandomPlayerDestination TargetType(TargetGen_RandomPlayerDestination)
#define Target_RandomPlayerApplyAura TargetType(TargetGen_RandomPlayerApplyAura)
#define Target_RandomUnit TargetType(TargetGen_RandomUnit)
#define Target_RandomUnitNotCurrent TargetType(TargetGen_RandomUnit, TargetFilter_NotCurrent)
#define Target_RandomDestination TargetType(TargetGen_RandomUnitDestination)
#define Target_RandomUnitApplyAura TargetType(TargetGen_RandomUnitApplyAura)
#define Target_RandomFriendly TargetType(TargetGen_RandomUnit, TargetFilter_Friendly)
#define Target_WoundedPlayer TargetType(TargetGen_RandomPlayer, TargetFilter_Wounded)
#define Target_WoundedUnit TargetType(TargetGen_RandomUnit, TargetFilter_Wounded)
#define Target_WoundedFriendly TargetType(TargetGen_RandomUnit, TargetFilter_WoundedFriendly)
#define Target_ClosestPlayer TargetType(TargetGen_RandomPlayer, TargetFilter_Closest)
#define Target_ClosestPlayerNotCurrent TargetType(TargetGen_RandomPlayer, TargetFilter_ClosestNotCurrent)
#define Target_ClosestUnit TargetType(TargetGen_RandomUnit, TargetFilter_Closest)
#define Target_ClosestUnitNotCurrent TargetType(TargetGen_RandomUnit, TargetFilter_ClosestNotCurrent)
#define Target_ClosestFriendly TargetType(TargetGen_RandomUnit, TargetFilter_ClosestFriendly)
#define Target_ClosestCorpse TargetType(TargetGen_RandomUnit, TargetFilter_ClosestFriendlyCorpse)
#define Target_RandomCorpse TargetType(TargetGen_RandomUnit, TargetFilter_FriendlyCorpse)


//////////////////////////////////////////////////////////////////////////////////////////
//Class SpellDesc
class SpellDesc
{
    public:

        SpellDesc(SpellInfo* pInfo, SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange, float pMaxRange, 
                  bool pStrictRange, const char* pText, uint8 pTextType, uint32 pSoundId, const char* pAnnouncement);

        virtual ~SpellDesc();

        EmoteDesc* AddEmote(const char* pText, uint8 pType = CHAT_MSG_MONSTER_YELL, uint32 pSoundId = 0);

        void TriggerCooldown(uint32 pCurrentTime = 0);
        void AddAnnouncement(const char* pText);

        SpellInfo* mInfo;              //Spell Entry information (generally you either want a SpellEntry OR a SpellFunc, not both)
        SpellFunc mSpellFunc;           //Spell Function to be executed (generally you either want a SpellEntry OR a SpellFunc, not both)
        TargetType mTargetType;         //Target type (see class above)

        float mChance;                  //Percent of the cast of this spell in a total of 100% of the attacks
        float mCastTime;                //Duration of the spell cast (seconds). Zero means the spell is instant cast
        int32 mCooldown;                //Spell cooldown (seconds)

        float mMinRange;                //Minimum range for spell to be cast
        float mMaxRange;                //Maximum range for spell to be cast
        bool mStrictRange;              //If true, creature won't run to (or away of) target, it will instead skip that attack

        EmoteArray mEmotes;             //Emotes (random) shouted on spell cast
        bool mEnabled;                  //True if the spell is enabled for casting, otherwise it will never be scheduled (useful for bosses with phases, etc.)
        Unit* mPredefinedTarget;        //Pre-defined Target Unit (Only valid with target type TargetGen_Predefined);

        const char* mAnnouncement;      //Announce spell cast

        //Those are not properties, they are data member used by the evaluation system
        uint32 mLastCastTime;           //Last time at which the spell casted (used to implement cooldown), set to 0
};


//////////////////////////////////////////////////////////////////////////////////////////
//Class MoonScriptCreatureAI
class MoonScriptCreatureAI : public CreatureAIScript
{
    public:
        MoonScriptCreatureAI(Creature* pCreature);
        virtual ~MoonScriptCreatureAI();

        //Movement
        void MoveTo(Unit* pUnit, RangeStatusPair pRangeStatus = std::make_pair(RangeStatus_TooFar, 0.0f));

        //Behavior
        void SetBehavior(BehaviorType pBehavior);
        BehaviorType GetBehavior();
        
        void AggroNearestUnit(uint32 pInitialThreat = 1);
        void AggroRandomUnit(uint32 pInitialThreat = 1);
        void AggroNearestPlayer(uint32 pInitialThreat = 1);
        void AggroRandomPlayer(uint32 pInitialThreat = 1);

        //Instances
        bool IsHeroic();
        uint32 HeroicInt(uint32 pNormal, uint32 pHeroic) { return IsHeroic() ? pHeroic : pNormal; }
        MoonInstanceScript* GetInstanceScript();

        void CastOnAllInrangePlayers(uint32 pSpellId, bool pTriggered = false);
        void CastOnInrangePlayers(float pDistanceMin, float pDistanceMax, uint32 pSpellId, bool pTriggered = false);

        MoonScriptCreatureAI* GetNearestCreature(uint32 pCreatureId = 0);
        MoonScriptCreatureAI* SpawnCreature(uint32 pCreatureId, float pX, float pY, float pZ, float pO = 0, bool pForceSameFaction = false, uint32 pPhase = 1);

        //Spells
        SpellDesc* AddSpell(uint32 pSpellId, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange = 0, float pMaxRange = 0, bool pStrictRange = false, const char* pText = NULL, uint8 pTextType = CHAT_MSG_MONSTER_YELL, uint32 pSoundId = 0, const char* pAnnouncement = NULL);
        SpellDesc* AddSpellFunc(SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange = 0, float pMaxRange = 0, bool pStrictRange = false, const char* pText = NULL, uint8 pTextType = CHAT_MSG_MONSTER_YELL, uint32 pSoundId = 0, const char* pAnnouncement = NULL);
        void CastSpell(SpellDesc* pSpell);
        void CastSpellNowNoScheduling(SpellDesc* pSpell);
        SpellDesc* FindSpellById(uint32 pSpellId);
        SpellDesc* FindSpellByFunc(SpellFunc pFnc);
        bool IsCasting();
        void ApplyAura(uint32 pSpellId);
        void RemoveAura(uint32 pSpellId);
        void RemoveAuraOnPlayers(uint32 pSpellId);
        void RemoveAllAuras();
        void TriggerCooldownOnAllSpells();
        void CancelAllCooldowns();

        //Emotes
        EmoteDesc* AddEmote(EventType pEventType, const char* pText, uint8 pType, uint32 pSoundId = 0);
        EmoteDesc* AddEmote(EventType pEventType, uint32_t scripttext);
        void RemoveAllEmotes(EventType pEventType);

        void Announce(const char* pText);

        //Timers and Events
        int32 AddTimer(int32 pDurationMillisec);
        int32 GetTimer(int32 pTimerId);
        void RemoveTimer(int32 & pTimerId);
        void ResetTimer(int32 pTimerId, int32 pDurationMillisec);
        bool IsTimerFinished(int32 pTimerId);
        void CancelAllTimers();
        uint32 GetTimerCount() { return mTimerCount; }
        int32 AddEvent(uint32 pEventId, int32 pTriggerTimer, EventFunc pEvent, int32 pMiscVal = 0, bool pRepeatable = false);
        void ResetEvent(uint32 pEventId, int32 pNewTriggerTimer, bool pRepeatable = false);
        void RemoveEvent(uint32 pEventId);
        void RemoveAllEvents();
        bool HasEvents() { return mEventCount > 0 ? true : false; }
        uint32 GetEventCount() { return mEventCount; }

        //Waypoints
        Movement::WayPoint* CreateWaypoint(int pId, uint32 pWaittime, uint32 pMoveFlag, Movement::Location pCoords);
        Movement::WayPoint* CreateWaypoint(int pId, uint32 pWaittime, Movement::LocationWithFlag wp_info);
        void AddWaypoint(Movement::WayPoint* pWayPoint);
        void ForceWaypointMove(uint32 pWaypointId);
        void SetWaypointToMove(uint32 pWaypointId);
        void StopWaypointMovement();
        void SetWaypointMoveType(Movement::WaypointMovementScript wp_move_script_type);
        uint32 GetCurrentWaypoint();
        size_t GetWaypointCount();
        bool HasWaypoints();

        //Others
        void SetTargetToChannel(Unit* pTarget, uint32 pSpellId);
        Unit* GetTargetToChannel();

        //Options
        void SetAIUpdateFreq(uint32 pUpdateFreq);
        uint32 GetAIUpdateFreq();

        //Reimplemented Events
        virtual void OnCombatStart(Unit* pTarget);
        virtual void OnCombatStop(Unit* pTarget);
        virtual void OnTargetDied(Unit* pTarget);
        virtual void OnDied(Unit* pKiller);
        virtual void AIUpdate();
        virtual void Destroy();

    protected:

        bool IsSpellScheduled(SpellDesc* pSpell);
        bool CastSpellInternal(SpellDesc* pSpell, uint32 pCurrentTime = 0);
        void CastSpellOnTarget(Unit* pTarget, TargetType pType, SpellInfo* pEntry, bool pInstant);
        int32 CalcSpellAttackTime(SpellDesc* pSpell);
        void CancelAllSpells();

        RangeStatusPair GetSpellRangeStatusToUnit(Unit* pTarget, SpellDesc* pSpell);
        Unit* GetTargetForSpell(SpellDesc* pSpell);
        Unit* GetBestPlayerTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f);
        Unit* GetBestUnitTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f);
        Unit* ChooseBestTargetInArray(UnitArray & pTargetArray, TargetFilter pFilter);
        Unit* GetNearestTargetInArray(UnitArray & pTargetArray);
        Unit* GetSecondMostHatedTargetInArray(UnitArray & pTargetArray);
        bool IsValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f);
        void PushRunToTargetCache(Unit* pTarget, SpellDesc* pSpell, RangeStatusPair pRangeStatus = std::make_pair(RangeStatus_TooFar, 0.0f));
        void PopRunToTargetCache();

        void RandomEmote(EmoteArray & pEmoteArray);

        SpellDescArray mSpells;
        SpellDescList mQueuedSpells;
        SpellDescList mScheduledSpells;

        Unit* mRunToTargetCache;
        SpellDesc* mRunToTargetSpellCache;

        EmoteArray mOnCombatStartEmotes;
        EmoteArray mOnTargetDiedEmotes;
        EmoteArray mOnDiedEmotes;
        EmoteArray mOnTauntEmotes;

        TimerArray mTimers;
        int32 mTimerIdCounter;
        uint32 mTimerCount;
        uint32 mEventCount;
        uint32 mAIUpdateFrequency;
        uint32 mBaseAttackTime;
        EventArray mEvents;
};


//////////////////////////////////////////////////////////////////////////////////////////
//Class MoonScriptBossAI
class MoonScriptBossAI : public MoonScriptCreatureAI
{
    public:

        MoonScriptBossAI(Creature* pCreature);
        virtual ~MoonScriptBossAI();

        //Basic Interface
        SpellDesc* AddPhaseSpell(int32 pPhase, SpellDesc* pSpell);
        int32 GetPhase();
        void SetPhase(int32 pPhase, SpellDesc* pPhaseChangeSpell = NULL);
        void SetEnrageInfo(SpellDesc* pSpell, int32 pTriggerMilliseconds);

        //Reimplemented Events
        virtual void OnCombatStart(Unit* pTarget);
        virtual void OnCombatStop(Unit* pTarget);
        virtual void AIUpdate();

    protected:

        int32 mPhaseIndex;
        PhaseSpellArray mPhaseSpells;
        SpellDesc* mEnrageSpell;
        int32 mEnrageTimerDuration;
        int32 mEnrageTimer;
};


//Premade Spell Functions
void SpellFunc_ClearHateList(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_Disappear(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_Reappear(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);


//Premade Event Functions
void EventFunc_ApplyAura(MoonScriptCreatureAI* pCreatureAI, int32 pMiscVal);
void EventFunc_ChangeGoState(MoonScriptCreatureAI* pCreatureAI, int32 pMiscVal);
void EventFunc_RemoveUnitFieldFlags(MoonScriptCreatureAI* pCreatureAI, int32 pMiscVal);


//STL Utilities
template <class Type> inline void DeleteArray(std::vector<Type> pVector)
{
    typename std::vector<Type>::iterator Iter = pVector.begin();
    for (; Iter != pVector.end(); ++Iter)
    {
        delete(*Iter);
    }
    pVector.clear();
}

//Warning: do not use if item can reside more than once in same vector
template <class Type> inline void DeleteItem(std::vector<Type> pVector, Type pItem)
{
    typename std::vector<Type>::iterator Iter = std::find(pVector.begin(), pVector.end(), pItem);
    if (Iter != pVector.end())
    {
        delete(*Iter);
        pVector.erase(Iter);
    }
}

#endif // _BASE_H
