/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Spell/SpellMgr.h"
#include "Chat/ChatDefines.hpp"
#include "Management/Item.h"
#include "Units/Creatures/AIInterface.h"
#include "ScriptMgr.h"

class Creature;
class CreatureAIScript;

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
    TARGET_CUSTOM
};

class SERVER_DECL CreatureAISpells
{
public:
    CreatureAISpells(SpellInfo const* spellInfo, float castChance, uint32_t targetType, uint32_t duration, uint32_t cooldown, bool forceRemove, bool isTriggered)
    {
        mSpellInfo = spellInfo;
        mCastChance = castChance;
        mTargetType = targetType;
        mDuration = duration;

        mDurationTimerId = 0;

        mCooldown = cooldown;
        mCooldownTimerId = 0;
        mForceRemoveAura = forceRemove;
        mIsTriggered = isTriggered;

        mMaxStackCount = 1;

        mMinPositionRangeToCast = 0.0f;
        mMaxPositionRangeToCast = 0.0f;

        mMinHpRangeToCast = 0;
        mMaxHpRangeToCast = 100;

        if (mSpellInfo != nullptr)
        {
            mMinPositionRangeToCast = GetMinRange(sSpellRangeStore.LookupEntry(mSpellInfo->getRangeIndex()));
            mMaxPositionRangeToCast = GetMaxRange(sSpellRangeStore.LookupEntry(mSpellInfo->getRangeIndex()));
        }

        mAttackStopTimer = 0;

        mCustomTargetCreature = nullptr;
    }

    ~CreatureAISpells()
    {
    }

    SpellInfo const* mSpellInfo;
    float mCastChance;
    uint32_t mTargetType;
    uint32_t mDuration;

    void setdurationTimer(uint32_t durationTimer);

    uint32_t mDurationTimerId;

    void setCooldownTimerId(uint32_t cooldownTimer);

    uint32_t mCooldown;
    uint32_t mCooldownTimerId;

    bool mForceRemoveAura;
    bool mIsTriggered;

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

    void sendRandomEmote(CreatureAIScript* creatureAI);

    uint32_t mMaxStackCount;

    void setMaxStackCount(uint32_t stackCount);
    uint32_t getMaxStackCount();

    float mMinPositionRangeToCast;
    float mMaxPositionRangeToCast;

    bool isDistanceInRange(float targetDistance);
    void setMinMaxDistance(float minDistance, float maxDistance);

    // if it is not a random target type it sets the hp range when the creature can cast this spell
    // if it is a random target it controles when the spell can be cast based on the target hp
    int mMinHpRangeToCast;
    int mMaxHpRangeToCast;

    bool isHpInPercentRange(int targetHp);
    void setMinMaxPercentHp(int minHp, int maxHp);

    typedef std::vector<uint32_t> ScriptPhaseList;
    ScriptPhaseList mPhaseList;

    void setAvailableForScriptPhase(std::vector<uint32_t> phaseVector);
    bool isAvailableForScriptPhase(uint32_t scriptPhase);

    uint32_t mAttackStopTimer;
    void setAttackStopTimer(uint32_t attackStopTime);
    uint32_t getAttackStopTimer();

    std::string mAnnouncement;
    void setAnnouncement(std::string announcement);
    void sendAnnouncement(CreatureAIScript* creatureAI);

    Creature* mCustomTargetCreature;
    void setCustomTarget(Creature* targetCreature);
    Creature* getCustomTarget();
};

struct scriptEvent
{
    int32_t timer;
    uint32_t bossPhase;
};

class scriptEventMap
{
    typedef std::multimap < uint32_t, scriptEvent> eventMap;

public:
    scriptEventMap() : bossPhase(0) {}

    void resetEvents()
    {
        eventMapStore.clear();
        bossPhase = 0;
    }

    void updateEvents(int32_t diff, uint32_t phase)
    {
        bossPhase = phase;

        if (!eventMapStore.empty())
        {
            for (eventMap::iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                if (itr->second.bossPhase == bossPhase)   
                    itr->second.timer = itr->second.timer - diff;
                else if (itr->second.bossPhase == 0)
                    itr->second.timer = itr->second.timer - diff;

                ++itr;
            }
        }
    }

    void addEvent(uint32_t eventId, int32_t time, uint32_t phase = 0)
    {
        scriptEventData.timer = time;
        scriptEventData.bossPhase = phase;

        removeEvent(eventId);
            
        eventMapStore.insert(eventMap::value_type(eventId, scriptEventData));
    }

    void removeEvent(uint32_t eventId)
    {
        if (!eventMapStore.empty())
        {
            for (eventMap::const_iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                if (itr->first == eventId)
                {
                    eventMapStore.erase(itr);
                    itr = eventMapStore.begin();
                    break;
                }
                else
                    ++itr;
            }
        }
    }

    // Return the first finished event
    // In order for example if you have multiple event with 10 seconds timer they get executed in event id order --> 1 .... 2 .... 3
    uint32_t getFinishedEvent()
    {
        uint32_t scriptEventId = 0;

        if (!eventMapStore.empty())
        {
            for (eventMap::const_iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                if (itr->second.bossPhase == bossPhase && itr->second.timer <= 0)
                {
                    scriptEventId = itr->first;
                    eventMapStore.erase(itr);
                    return scriptEventId;
                }
                else if (itr->second.bossPhase == 0 && itr->second.timer <= 0)
                {
                    scriptEventId = itr->first;
                    eventMapStore.erase(itr);                   
                    return scriptEventId;
                }
                else
                    ++itr;
            }
        }
        return scriptEventId;
    }

    void delayEvent(uint32_t eventId, int32_t delay)
    {
        if (!eventMapStore.empty())
        {
            for (auto itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                // Only Delay Timers that are not Finished
                if (itr->second.timer > 0 && itr->first == eventId)
                {
                    itr->second.timer = itr->second.timer + delay;
                    break;
                }
                ++itr;
            }
        }
    }

    void delayAllEvents(int32_t delay, uint32_t phase = 0)
    {
        if (!eventMapStore.empty())
        {
            for (auto itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                // Only Delay Timers that are not Finished and in our Current Phase
                if (itr->second.timer > 0 && itr->second.bossPhase == phase)
                    itr->second.timer = itr->second.timer + delay;
                else if (itr->second.timer > 0 && itr->second.bossPhase == 0)
                    itr->second.timer = itr->second.timer + delay;

                ++itr;
            }
        }
    }

private:
    uint32_t bossPhase;
    scriptEvent scriptEventData;
    eventMap eventMapStore;
};

class SERVER_DECL CreatureAIScript
{
public:

    CreatureAIScript(Creature* creature);
    virtual ~CreatureAIScript();

    virtual void OnCombatStart(Unit* /*_target*/) {}
    virtual void OnCombatStop(Unit* /*_target*/) {}
    virtual void OnDamageTaken(Unit* /*_attacker*/, uint32_t /*_amount*/) {}
    virtual void OnCastSpell(uint32_t /*_spellId*/) {}
    virtual void OnTargetParried(Unit* /*_target*/) {}
    virtual void OnTargetDodged(Unit* /*_target*/) {}
    virtual void OnTargetBlocked(Unit* /*_target*/, int32 /*_amount*/) {}
    virtual void OnTargetCritHit(Unit* /*_target*/, int32 /*_amount*/) {}
    virtual void OnTargetDied(Unit* /*_target*/) {}
    virtual void OnParried(Unit* /*_target*/) {}
    virtual void OnDodged(Unit* /*_target*/) {}
    virtual void OnBlocked(Unit* /*_target*/, int32 /*_amount*/) {}
    virtual void OnCritHit(Unit* /*_target*/, int32 /*_amount*/) {}
    virtual void OnHit(Unit* /*_target*/, float /*_amount*/) {}
    virtual void OnDied(Unit* /*_killer*/) {}
    virtual void OnAssistTargetDied(Unit* /*_assistTarget*/) {}
    virtual void OnFear(Unit* /*_fearer*/, uint32_t /*_spellId*/) {}
    virtual void OnFlee(Unit* /*_flee*/) {}
    virtual void OnCallForHelp() {}
    virtual void OnLoad() {}
    virtual void OnDespawn() {}
    virtual void OnReachWP(uint32_t /*_waypointId*/, bool /*_isForwards*/) {}
    virtual void OnLootTaken(Player* /*player*/, ItemProperties const* /*_itemProperties*/) {}
    virtual void AIUpdate() {}
    virtual void OnEmote(Player* /*_player*/, EmoteType /*_emote*/) {}
    virtual void StringFunctionCall(int) {}

    virtual void OnEnterVehicle() {}
    virtual void OnExitVehicle() {}
    virtual void OnFirstPassengerEntered(Unit* /*_passenger*/) {}
    virtual void OnVehicleFull() {}
    virtual void OnLastPassengerLeft(Unit* /*_passenger*/) {}

    virtual void OnScriptPhaseChange(uint32_t /*_phaseId*/) {}
    virtual void OnHitBySpell(uint32_t /*_spellId*/, Unit* /*_caster*/) {}

    // Data sharing between scripts
    virtual void SetCreatureData(uint32_t /*type*/, uint32_t /*data*/) {}
    virtual void SetCreatureData64(uint32_t /*type*/, uint64_t /*data*/) {}
    virtual uint32_t GetCreatureData(uint32_t /*type*/) const { return 0; }
    virtual uint64_t GetCreatureData64(uint32_t /*type*/) const { return 0; }

    virtual void Destroy() { delete this; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Event default management
    // \brief: These functions are called internal for script events. Do NOT use them in your scripts!
    void _internalOnDied();
    void _internalOnTargetDied();
    void _internalOnCombatStart();
    void _internalOnCombatStop();
    void _internalAIUpdate();
    void _internalOnScriptPhaseChange();

    //////////////////////////////////////////////////////////////////////////////////////////
    // player
    Player* getNearestPlayer();

    //////////////////////////////////////////////////////////////////////////////////////////
    // creature
    CreatureAIScript* getNearestCreatureAI(uint32_t entry);

    Creature* getNearestCreature(uint32_t entry);
    Creature* getNearestCreature(float posX, float posY, float posZ, uint32_t entry);

    float getRangeToObject(Object* object);

    CreatureAIScript* spawnCreatureAndGetAIScript(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0, uint32_t phase = 1);

    Creature* spawnCreature(uint32_t entry, LocationVector pos, uint32_t factionId = 0, uint32_t phase = 1);
    Creature* spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0, uint32_t phase = 1);
    void despawn(uint32_t delay = 2000, uint32_t respawnTime = 0);

    bool isAlive();

    //////////////////////////////////////////////////////////////////////////////////////////
    // AIAgent
    void setAIAgent(AI_Agent agent);
    uint8_t getAIAgent();

    //////////////////////////////////////////////////////////////////////////////////////////
    // movement
    void setRooted(bool set);
    bool isRooted();

    void setFlyMode(bool fly);

    // single point movement
    void moveTo(float posX, float posY, float posZ, bool setRun = true);
    void moveToUnit(Unit* unit);
    void moveToSpawn();
    void stopMovement();

    //////////////////////////////////////////////////////////////////////////////////////////
    // wp movement
    Movement::WayPoint* CreateWaypoint(int pId, uint32_t pWaittime, uint32_t pMoveFlag, Movement::Location pCoords);
    void AddWaypoint(Movement::WayPoint* pWayPoint);
    void ForceWaypointMove(uint32_t pWaypointId);
    void SetWaypointToMove(uint32_t pWaypointId);
    void StopWaypointMovement();
    void SetWaypointMoveType(Movement::WaypointMovementScript wp_move_script_type);
    uint32_t GetCurrentWaypoint();
    size_t GetWaypointCount();
    bool HasWaypoints();

    //////////////////////////////////////////////////////////////////////////////////////////
    // combat setup
    bool canEnterCombat();
    void setCanEnterCombat(bool enterCombat);
    bool _isInCombat();
    void _delayNextAttack(int32_t milliseconds);

    void _setMeleeDisabled(bool disable);
    bool _isMeleeDisabled();
    void _setRangedDisabled(bool disable);
    bool _isRangedDisabled();
    void _setCastDisabled(bool disable);
    bool _isCastDisabled();
    void _setTargetingDisabled(bool disable);
    bool _isTargetingDisabled();

    void _clearHateList();
    void _wipeHateList();
    int32_t _getHealthPercent();
    int32_t _getManaPercent();
    void _regenerateHealth();

    bool _isCasting();

    //////////////////////////////////////////////////////////////////////////////////////////
    // script phase
    // \brief: script phase is reset to 0 in _internalOnDied() and _internalOnCombatStop()
private:

    uint32_t mScriptPhase;

public:

    uint32_t getScriptPhase();
    void setScriptPhase(uint32_t scriptPhase);
    void resetScriptPhase();
    bool isScriptPhase(uint32_t scriptPhase);

    //////////////////////////////////////////////////////////////////////////////////////////
     // script events
     // \brief: 
protected:
    scriptEventMap scriptEvents;

    //////////////////////////////////////////////////////////////////////////////////////////
    // timers
    //\brief: timers are stored and updated in InstanceScript if a instance script is
    //        available (instanceUpdateFrequency). If the creature is on a map without a
    //        instance script, the timer gets updated locale (AIUpdateFrequency).
private:

    //reference to instance time - used for creatures located on a map with a instance script.
    typedef std::list<uint32_t> creatureTimerIds;
    creatureTimerIds mCreatureTimerIds;

    //creature timer - used for creatures located on a map with NO instance script.
    typedef std::pair<uint32_t, uint32_t> CreatureTimerPair;
    typedef std::vector<CreatureTimerPair> CreatureTimerArray;

    CreatureTimerArray mCreatureTimer;

    uint32_t mCreatureTimerCount;

public:

    uint32_t _addTimer(uint32_t durationInMs);
    uint32_t _getTimeForTimer(uint32_t timerId);
    void _removeTimer(uint32_t& timerId);
    void _resetTimer(uint32_t timerId, uint32_t durationInMs);
    bool _isTimerFinished(uint32_t timerId);
    void _cancelAllTimers();

    uint32_t _getTimerCount();

    //only for internal use!
    void updateAITimers();

    //used for debug
    void displayCreatureTimerList(Player* player);

    //////////////////////////////////////////////////////////////////////////////////////////
    // ai upodate frequency
private:

    uint32_t mAIUpdateFrequency;

    uint32_t mCustomAIUpdateDelayTimerId;
    uint32_t mCustomAIUpdateDelay;
public:

    //new
    void registerAiUpdateFrequency();
    void removeAiUpdateFrequency();

    //old stuff
    void SetAIUpdateFreq(uint32_t pUpdateFreq);
    uint32_t GetAIUpdateFreq();

    void RegisterAIUpdateEvent(uint32_t frequency);
    void ModifyAIUpdateEvent(uint32_t newfrequency);
    void RemoveAIUpdateEvent();

    //////////////////////////////////////////////////////////////////////////////////////////
    // appearance
    void _setScale(float scale);
    float _getScale();
    void _setDisplayId(uint32_t displayId);
    void _setWieldWeapon(bool setWieldWeapon);
    void _setDisplayWeapon(bool setMainHand, bool setOffHand);
    void _setDisplayWeaponIds(uint32_t itemId1, uint32_t itemId2);

    //////////////////////////////////////////////////////////////////////////////////////////
    // spell
    typedef std::vector<CreatureAISpells*> CreatureAISpellsArray;
    CreatureAISpellsArray mCreatureAISpells;

public:

    uint32_t mSpellWaitTimerId;

    //addAISpell(spellID, Chance, TargetType, Duration (s), waitBeforeNextCast (s))
    CreatureAISpells* addAISpell(uint32_t spellId, float castChance, uint32_t targetType, uint32_t duration = 0, uint32_t cooldown = 0, bool forceRemove = false, bool isTriggered = false);

    void _applyAura(uint32_t spellId);
    void _removeAura(uint32_t spellId);
    void _removeAllAuras();

    void _removeAuraOnPlayers(uint32_t spellId);
    void _castOnInrangePlayers(uint32_t spellId, bool triggered = false);
    void _castOnInrangePlayersWithinDist(float minDistance, float maxDistance, uint32_t spellId, bool triggered = false);

    void _castAISpell(CreatureAISpells* aiSpell);

    void _setTargetToChannel(Unit* target, uint32_t spellId);
    void _unsetTargetToChannel();
    Unit* _getTargetToChannel();

    Unit* mCurrentSpellTarget;
    CreatureAISpells* mLastCastedSpell;

    // only for internal use
    void newAIUpdateSpellSystem();
    void castSpellOnRandomTarget(CreatureAISpells* AiSpell);

    //////////////////////////////////////////////////////////////////////////////////////////
    // gameobject
    GameObject* getNearestGameObject(uint32_t entry);
    GameObject* getNearestGameObject(float posX, float posY, float posZ, uint32_t entry);

    //////////////////////////////////////////////////////////////////////////////////////////
    // chat message
    enum EmoteEventType
    {
        Event_OnCombatStart = 0,
        Event_OnTargetDied = 1,
        Event_OnDied = 2,
        Event_OnTaunt = 3,
        Event_OnIdle = 4     // new not part of db definitions!
    };

private:

    typedef std::vector<uint32_t> definedEmoteVector;
    definedEmoteVector mEmotesOnCombatStart;
    definedEmoteVector mEmotesOnTargetDied;
    definedEmoteVector mEmotesOnDied;
    definedEmoteVector mEmotesOnTaunt;
    definedEmoteVector mEmotesOnIdle;

public:

    void sendChatMessage(uint8_t type, uint32_t soundId, std::string text);
    void sendDBChatMessage(uint32_t textId);

    void sendRandomDBChatMessage(std::vector<uint32_t> emoteVector);

    void addEmoteForEvent(uint32_t eventType, uint32_t scriptTextId);

    void sendAnnouncement(std::string stringAnnounce);

    //////////////////////////////////////////////////////////////////////////////////////////
    // idle emote timer
    // \brief: idle timer is seperated from custom timers. If isIdleEmoteEnabled is true,
    //         a random chat message is send by _internalAIUpdate stored in mEmotesOnIdle
private:

    bool isIdleEmoteEnabled;
    uint32_t idleEmoteTimerId;

    uint32_t idleEmoteTimeMin;
    uint32_t idleEmoteTimeMax;

public:

    void enableOnIdleEmote(bool enable, uint32_t durationInMs = 0);
    void setIdleEmoteTimerId(uint32_t timerId);
    uint32_t getIdleEmoteTimerId();
    void resetIdleEmoteTime(uint32_t durationInMs);

    void setRandomIdleEmoteTime(uint32_t minTime, uint32_t maxTime);
    void generateNextRandomIdleEmoteTime();

    //////////////////////////////////////////////////////////////////////////////////////////
    // basic
private:

    Creature* _creature;

public:

    Creature* getCreature() { return _creature; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // instance
    InstanceScript* getInstanceScript();

    bool _isHeroic();

    template<class T> inline
        const T& RAID_MODE(const T& normal10, const T& normal25, const T& heroic10, const T& heroic25) const
    {
        if (_creature->GetMapMgr()->pInstance)
        {
            switch (_creature->GetMapMgr()->pInstance->m_difficulty)
            {
            case MODE_NORMAL_10MEN:
                return normal10;
            case MODE_NORMAL_25MEN:
                return normal25;
            case MODE_HEROIC_10MEN:
                return heroic10;
            case MODE_HEROIC_25MEN:
                return heroic25;
            default:
                break;
            }
        }

        return normal10;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // linked creature AI scripts
private:

    CreatureAIScript* linkedCreatureAI;

public:

    CreatureAIScript* getLinkedCreatureAIScript() { return linkedCreatureAI; }
    void setLinkedCreatureAIScript(CreatureAIScript* creatureAI);
    void removeLinkToCreatureAIScript();

    //////////////////////////////////////////////////////////////////////////////////////////
    // target
    Unit* getBestPlayerTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f);
    Unit* getBestUnitTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f);
    Unit* getBestTargetInArray(UnitArray& pTargetArray, TargetFilter pFilter);
    Unit* getNearestTargetInArray(UnitArray& pTargetArray);
    Unit* getSecondMostHatedTargetInArray(UnitArray& pTargetArray);
    bool isValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f);
};
