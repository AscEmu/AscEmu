/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "CreatureAIScript.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Objects/Faction.h"


//////////////////////////////////////////////////////////////////////////////////////////

void CreatureAISpells::setdurationTimer(uint32_t durationTimer)
{
    mDurationTimerId = durationTimer;
}

void CreatureAISpells::setCooldownTimerId(uint32_t cooldownTimer)
{
    mCooldownTimerId = cooldownTimer;
}

void CreatureAISpells::addDBEmote(uint32_t textId)
{
    MySQLStructure::NpcScriptText const* npcScriptText = sMySQLStore.getNpcScriptText(textId);
    if (npcScriptText != nullptr)
        addEmote(npcScriptText->text, npcScriptText->type, npcScriptText->sound);
    else
        LogDebugFlag(LF_SCRIPT_MGR, "A script tried to add a spell emote with %u! Id is not available in table npc_script_text.", textId);
}

void CreatureAISpells::addEmote(std::string pText, uint8_t pType, uint32_t pSoundId)
{
    if (!pText.empty() || pSoundId)
        mAISpellEmote.push_back(AISpellEmotes(pText, pType, pSoundId));
}

void CreatureAISpells::sendRandomEmote(CreatureAIScript* creatureAI)
{
    if (!mAISpellEmote.empty() && creatureAI != nullptr)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "AISpellEmotes::sendRandomEmote() : called");

        uint32_t randomUInt = (mAISpellEmote.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(mAISpellEmote.size() - 1)) : 0;
        creatureAI->getCreature()->SendChatMessage(mAISpellEmote[randomUInt].mType, LANG_UNIVERSAL, mAISpellEmote[randomUInt].mText.c_str());

        if (mAISpellEmote[randomUInt].mSoundId != 0)
            creatureAI->getCreature()->PlaySoundToSet(mAISpellEmote[randomUInt].mSoundId);
    }
}

void CreatureAISpells::setMaxStackCount(uint32_t stackCount)
{
    mMaxStackCount = stackCount;
}

uint32_t CreatureAISpells::getMaxStackCount()
{
    return mMaxStackCount;
}

bool CreatureAISpells::isDistanceInRange(float targetDistance)
{
    if (targetDistance >= mMinPositionRangeToCast && targetDistance <= mMaxPositionRangeToCast)
        return true;

    return false;
}

void CreatureAISpells::setMinMaxDistance(float minDistance, float maxDistance)
{
    mMinPositionRangeToCast = minDistance;
    mMaxPositionRangeToCast = maxDistance;
}

bool CreatureAISpells::isHpInPercentRange(int targetHp)
{
    if (targetHp >= mMinHpRangeToCast && targetHp <= mMaxHpRangeToCast)
        return true;

    return false;
}

void CreatureAISpells::setMinMaxPercentHp(int minHp, int maxHp)
{
    mMinHpRangeToCast = minHp;
    mMaxHpRangeToCast = maxHp;
}

void CreatureAISpells::setAvailableForScriptPhase(std::vector<uint32_t> phaseVector)
{
    for (const auto& phase : phaseVector)
    {
        mPhaseList.push_back(phase);
    }
}

bool CreatureAISpells::isAvailableForScriptPhase(uint32_t scriptPhase)
{
    if (mPhaseList.empty())
        return true;

    for (const auto& availablePhase : mPhaseList)
    {
        if (availablePhase == scriptPhase)
            return true;
    }

    return false;
}

void CreatureAISpells::setAttackStopTimer(uint32_t attackStopTime)
{
    mAttackStopTimer = attackStopTime;
}

uint32_t CreatureAISpells::getAttackStopTimer()
{
    return mAttackStopTimer;
}

void CreatureAISpells::setAnnouncement(std::string announcement)
{
    mAnnouncement = announcement;
}

void CreatureAISpells::sendAnnouncement(CreatureAIScript* creatureAI)
{
    if (!mAnnouncement.empty() && creatureAI != nullptr)
    {
        LogDebugFlag(LF_SCRIPT_MGR, "AISpellEmotes::sendAnnouncement() : called");

        creatureAI->getCreature()->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, mAnnouncement.c_str());
    }
}

void CreatureAISpells::setCustomTarget(Creature* targetCreature)
{
    mCustomTargetCreature = targetCreature;
}

Creature* CreatureAISpells::getCustomTarget()
{
    return mCustomTargetCreature;
}

//////////////////////////////////////////////////////////////////////////////////////////

CreatureAIScript::CreatureAIScript(Creature* creature) : _creature(creature), linkedCreatureAI(nullptr), mScriptPhase(0), mAIUpdateFrequency(defaultUpdateFrequency),
mCreatureTimerCount(0), isIdleEmoteEnabled(false), idleEmoteTimerId(0), idleEmoteTimeMin(0), idleEmoteTimeMax(0)
{
    mCreatureTimerIds.clear();
    mCreatureTimer.clear();

    mCustomAIUpdateDelayTimerId = 0;
    mCustomAIUpdateDelay = 0;
    registerAiUpdateFrequency();

    //new CreatureAISpell handling
    mSpellWaitTimerId = _addTimer(defaultUpdateFrequency);
    mCurrentSpellTarget = nullptr;
    mLastCastedSpell = nullptr;
}

CreatureAIScript::~CreatureAIScript()
{
    //notify our linked creature that we are being deleted.
    if (linkedCreatureAI != nullptr)
        linkedCreatureAI->removeLinkToCreatureAIScript();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Event default management
void CreatureAIScript::_internalOnDied()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnDied() called");

    enableOnIdleEmote(false);

    _cancelAllTimers();
    _removeAllAuras();

    removeAiUpdateFrequency();

    RemoveAIUpdateEvent();
    sendRandomDBChatMessage(mEmotesOnDied);

    resetScriptPhase();
}

void CreatureAIScript::_internalOnTargetDied()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnTargetDied() called");

    sendRandomDBChatMessage(mEmotesOnTargetDied);
}

void CreatureAIScript::_internalOnCombatStart()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnEnterCombat() called");

    enableOnIdleEmote(false);

    setAIAgent(AGENT_MELEE);

    sendRandomDBChatMessage(mEmotesOnCombatStart);

    setScriptPhase(1);

    RegisterAIUpdateEvent(mAIUpdateFrequency);
}

void CreatureAIScript::_internalOnCombatStop()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnCombatStop() called");

    _cancelAllTimers();
    _removeAllAuras();
    setAIAgent(AGENT_NULL);
    getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
    RemoveAIUpdateEvent();

    resetScriptPhase();
    enableOnIdleEmote(true);
}

void CreatureAIScript::_internalAIUpdate()
{
    //LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalAIUpdate() called");

    updateAITimers();

    // old AIUpdate stuff is now handled by customAIUpdateTimer. Keep this until all scripts are updated to new logic.
    if (mCustomAIUpdateDelayTimerId != 0)
    {
        if (!_isTimerFinished(mCustomAIUpdateDelayTimerId))
            return;

        _resetTimer(mCustomAIUpdateDelayTimerId, mCustomAIUpdateDelay);
    }

    // idleemotes
    if (!_isInCombat() && isIdleEmoteEnabled)
    {
        if (_isTimerFinished(getIdleEmoteTimerId()))
        {
            sendRandomDBChatMessage(mEmotesOnIdle);
            generateNextRandomIdleEmoteTime();
        }
    }

    AIUpdate();

    if (!_isInCombat())
        return;

    newAIUpdateSpellSystem();
}

void CreatureAIScript::_internalOnScriptPhaseChange()
{
    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_internalOnScriptPhaseChange() called");

    getCreature()->GetScript()->OnScriptPhaseChange(getScriptPhase());
}

//////////////////////////////////////////////////////////////////////////////////////////
// player
Player* CreatureAIScript::getNearestPlayer()
{
    return _creature->GetMapMgr()->GetInterface()->GetPlayerNearestCoords(_creature->GetPositionX(), _creature->GetPositionY(), _creature->GetPositionZ());
}

//////////////////////////////////////////////////////////////////////////////////////////
// creature
CreatureAIScript* CreatureAIScript::getNearestCreatureAI(uint32_t entry)
{
    Creature* creature = getNearestCreature(entry);
    return (creature ? creature->GetScript() : nullptr);
}

Creature* CreatureAIScript::getNearestCreature(uint32_t entry)
{
    return getNearestCreature(_creature->GetPositionX(), _creature->GetPositionY(), _creature->GetPositionZ(), entry);
}

Creature* CreatureAIScript::getNearestCreature(float posX, float posY, float posZ, uint32_t entry)
{
    return _creature->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(posX, posY, posZ, entry);
}

float CreatureAIScript::getRangeToObject(Object* object)
{
    return _creature->CalcDistance(object);
}

CreatureAIScript* CreatureAIScript::spawnCreatureAndGetAIScript(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId /* = 0*/)
{
    Creature* creature = spawnCreature(entry, posX, posY, posZ, posO, factionId);
    return (creature ? creature->GetScript() : nullptr);
}

Creature* CreatureAIScript::spawnCreature(uint32_t entry, LocationVector pos, uint32_t factionId /*= 0*/)
{
    return spawnCreature(entry, pos.x, pos.y, pos.z, pos.o, factionId);
}

Creature* CreatureAIScript::spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId /* = 0*/)
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(entry);
    if (creatureProperties == nullptr)
    {
        LOG_ERROR("tried to create an invalid creature with entry %u!", entry);
        return nullptr;
    }

    Creature* creature = _creature->GetMapMgr()->GetInterface()->SpawnCreature(entry, posX, posY, posZ, posO, true, true, 0, 0);
    if (creature == nullptr)
        return nullptr;

    if (factionId != 0)
        creature->SetFaction(factionId);
    else
        creature->SetFaction(creatureProperties->Faction);

    return creature;
}

void CreatureAIScript::despawn(uint32_t delay /*= 2000*/, uint32_t respawnTime /*= 0*/)
{
    _creature->Despawn(delay, respawnTime);
}

bool CreatureAIScript::isAlive()
{
    return _creature->isAlive();
}

//////////////////////////////////////////////////////////////////////////////////////////
// AIAgent
void CreatureAIScript::setAIAgent(AI_Agent agent)
{
    if (agent <= AGENT_CALLFORHELP)
        _creature->GetAIInterface()->setCurrentAgent(agent);
}

uint8_t CreatureAIScript::getAIAgent()
{
    return _creature->GetAIInterface()->getCurrentAgent();
}

//////////////////////////////////////////////////////////////////////////////////////////
// movement
void CreatureAIScript::setRooted(bool set)
{
    _creature->setMoveRoot(set);
}

void CreatureAIScript::setFlyMode(bool fly)
{
    if (fly && !_creature->GetAIInterface()->isFlying())
    {
        _creature->setMoveCanFly(true);
        _creature->GetAIInterface()->setSplineFlying();
    }
    else if (!fly && _creature->GetAIInterface()->isFlying())
    {
        _creature->setMoveCanFly(false);
        _creature->GetAIInterface()->unsetSplineFlying();
    }
}

bool CreatureAIScript::isRooted()
{
    return _creature->GetAIInterface()->m_canMove;
}

void CreatureAIScript::moveTo(float posX, float posY, float posZ, bool setRun /*= true*/)
{
    if (setRun)
        _creature->GetAIInterface()->setWalkMode(WALKMODE_RUN);

    _creature->GetAIInterface()->MoveTo(posX, posY, posZ);
}

void CreatureAIScript::moveToUnit(Unit* unit)
{
    if (unit != nullptr)
        moveTo(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ());
}

void CreatureAIScript::moveToSpawn()
{
    LocationVector spawnPos = _creature->GetSpawnPosition();
    _creature->GetAIInterface()->sendSplineMoveToPoint(spawnPos);
}

void CreatureAIScript::stopMovement()
{
    _creature->GetAIInterface()->StopMovement(0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// wp movement
Movement::WayPoint* CreatureAIScript::CreateWaypoint(int pId, uint32_t pWaittime, uint32_t pMoveFlag, Movement::Location pCoords)
{
    Movement::WayPoint* wp = _creature->CreateWaypointStruct();
    wp->id = pId;
    wp->x = pCoords.x;
    wp->y = pCoords.y;
    wp->z = pCoords.z;
    wp->o = pCoords.o;
    wp->waittime = pWaittime;
    wp->flags = pMoveFlag;
    wp->forwardemoteoneshot = false;
    wp->forwardemoteid = 0;
    wp->backwardemoteoneshot = false;
    wp->backwardemoteid = 0;
    wp->forwardskinid = 0;
    wp->backwardskinid = 0;
    return wp;
}

void CreatureAIScript::AddWaypoint(Movement::WayPoint* pWayPoint)
{
    _creature->GetAIInterface()->addWayPoint(pWayPoint);
}

void CreatureAIScript::ForceWaypointMove(uint32_t pWaypointId)
{
    if (canEnterCombat())
        _creature->GetAIInterface()->SetAllowedToEnterCombat(false);

    if (isRooted())
        setRooted(false);

    stopMovement();
    _creature->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
    SetWaypointToMove(pWaypointId);
}

void CreatureAIScript::SetWaypointToMove(uint32_t pWaypointId)
{
    _creature->GetAIInterface()->setWayPointToMove(pWaypointId);
}

void CreatureAIScript::StopWaypointMovement()
{
    setAIAgent(AGENT_NULL);
    _creature->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
    SetWaypointToMove(0);
}

void CreatureAIScript::SetWaypointMoveType(Movement::WaypointMovementScript wp_move_script_type)
{
    _creature->GetAIInterface()->setWaypointScriptType(wp_move_script_type);

}

uint32_t CreatureAIScript::GetCurrentWaypoint()
{
    return _creature->GetAIInterface()->getCurrentWayPointId();
}

size_t CreatureAIScript::GetWaypointCount()
{
    return _creature->GetAIInterface()->getWayPointsCount();
}

bool CreatureAIScript::HasWaypoints()
{
    return _creature->GetAIInterface()->hasWayPoints();
}

//////////////////////////////////////////////////////////////////////////////////////////
// combat setup

bool CreatureAIScript::canEnterCombat()
{
    return _creature->GetAIInterface()->GetAllowedToEnterCombat();
}

void CreatureAIScript::setCanEnterCombat(bool enterCombat)
{
    //Zyres 10/21/2017 creatures can be attackable even if they can not enter combat... the following line is not correct.
    if (enterCombat)
    {
        _creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }
    else
    {
        _creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }

    _creature->GetAIInterface()->SetAllowedToEnterCombat(enterCombat);
}

bool CreatureAIScript::_isInCombat()
{
    return _creature->CombatStatus.IsInCombat();
}

void CreatureAIScript::_delayNextAttack(int32_t milliseconds)
{
    _creature->setAttackTimer(milliseconds, false);
}

void CreatureAIScript::_setMeleeDisabled(bool disable)
{
    _creature->GetAIInterface()->setMeleeDisabled(disable);
}

bool CreatureAIScript::_isMeleeDisabled()
{
    return _creature->GetAIInterface()->isMeleeDisabled();
}

void CreatureAIScript::_setRangedDisabled(bool disable)
{
    _creature->GetAIInterface()->setRangedDisabled(disable);
}

bool CreatureAIScript::_isRangedDisabled()
{
    return _creature->GetAIInterface()->isRangedDisabled();
}

void CreatureAIScript::_setCastDisabled(bool disable)
{
    _creature->GetAIInterface()->setCastDisabled(disable);
}

bool CreatureAIScript::_isCastDisabled()
{
    return _creature->GetAIInterface()->isCastDisabled();
}

void CreatureAIScript::_setTargetingDisabled(bool disable)
{
    _creature->GetAIInterface()->setTargetingDisabled(disable);
}

bool CreatureAIScript::_isTargetingDisabled()
{
    return _creature->GetAIInterface()->isTargetingDisabled();
}

void CreatureAIScript::_clearHateList()
{
    _creature->GetAIInterface()->ClearHateList();
}

void CreatureAIScript::_wipeHateList()
{
    _creature->GetAIInterface()->WipeHateList();
}

int32_t CreatureAIScript::_getHealthPercent()
{
    return _creature->GetHealthPct();
}

int32_t CreatureAIScript::_getManaPercent()
{
    return _creature->GetManaPct();
}

void CreatureAIScript::_regenerateHealth()
{
    _creature->RegenerateHealth();
    _creature->RegeneratePower(false);
}

bool CreatureAIScript::_isCasting()
{
    return _creature->isCastingNonMeleeSpell();
}

//////////////////////////////////////////////////////////////////////////////////////////
// script phase
uint32_t CreatureAIScript::getScriptPhase()
{
    return mScriptPhase;
}

void CreatureAIScript::setScriptPhase(uint32_t scriptPhase)
{
    if (isScriptPhase(scriptPhase) == false)
    {
        mScriptPhase = scriptPhase;

        if (getScriptPhase() != 0)
            _internalOnScriptPhaseChange();
    }
}

void CreatureAIScript::resetScriptPhase()
{
    setScriptPhase(0);
}

bool CreatureAIScript::isScriptPhase(uint32_t scriptPhase)
{
    return (getScriptPhase() == scriptPhase);
}

//////////////////////////////////////////////////////////////////////////////////////////
// timers

uint32_t CreatureAIScript::_addTimer(uint32_t durationInMs)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        uint32_t timerId = inScript->addTimer(durationInMs);
        mCreatureTimerIds.push_back(timerId);

        return timerId;
    }
    else
    {
        uint32_t timerId = ++mCreatureTimerCount;
        mCreatureTimer.push_back(std::make_pair(timerId, durationInMs));

        return timerId;
    }
}

uint32_t CreatureAIScript::_getTimeForTimer(uint32_t timerId)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        return inScript->getTimeForTimer(timerId);
    }
    else
    {
        for (const auto& intTimer : mCreatureTimer)
        {
            if (intTimer.first == timerId)
                return intTimer.second;
        }
    }

    return 0;
}

void CreatureAIScript::_removeTimer(uint32_t& timerId)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        uint32_t mTimerId = timerId;
        inScript->removeTimer(timerId);
        if (timerId == 0)
            mCreatureTimerIds.remove(mTimerId);
    }
    else
    {
        for (CreatureTimerArray::iterator intTimer = mCreatureTimer.begin(); intTimer != mCreatureTimer.end(); ++intTimer)
        {
            if (intTimer->first == timerId)
            {
                mCreatureTimer.erase(intTimer);
                timerId = 0;
                break;
            }
        }
    }
}

void CreatureAIScript::_resetTimer(uint32_t timerId, uint32_t durationInMs)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        inScript->resetTimer(timerId, durationInMs);
    }
    else
    {
        for (auto& intTimer : mCreatureTimer)
        {
            if (intTimer.first == timerId)
                intTimer.second = durationInMs;
        }
    }
}

bool CreatureAIScript::_isTimerFinished(uint32_t timerId)
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        return inScript->isTimerFinished(timerId);
    }
    else
    {
        for (const auto& intTimer : mCreatureTimer)
        {
            if (intTimer.first == timerId)
                return intTimer.second == 0;
        }
    }

    return false;
}

void CreatureAIScript::_cancelAllTimers()
{
    if (InstanceScript* inScript = getInstanceScript())
    {
        for (auto& timer : mCreatureTimerIds)
            _removeTimer(timer);

        mCreatureTimerIds.clear();
    }
    else
    {
        mCreatureTimer.clear();
    }

    LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::_cancelAllTimers() - all cleared!");
}

uint32_t CreatureAIScript::_getTimerCount()
{
    if (InstanceScript* inScript = getInstanceScript())
        return static_cast<uint32_t>(mCreatureTimerIds.size());

    return static_cast<uint32_t>(mCreatureTimer.size());
}

void CreatureAIScript::updateAITimers()
{
    for (auto& TimerIter : mCreatureTimer)
    {
        if (TimerIter.second > 0)
        {
            int leftTime = TimerIter.second - mAIUpdateFrequency;
            if (leftTime > 0)
                TimerIter.second -= mAIUpdateFrequency;
            else
                TimerIter.second = 0;
        }
    }
}

void CreatureAIScript::displayCreatureTimerList(Player* player)
{
    player->BroadcastMessage("=== Timers for creature %s ===", getCreature()->GetCreatureProperties()->Name.c_str());

    if (mCreatureTimerIds.empty() && mCreatureTimer.empty())
    {
        player->BroadcastMessage("  No Timers available!");
    }
    else
    {
        if (InstanceScript* inScript = getInstanceScript())
        {
            for (const auto& intTimer : mCreatureTimerIds)
                player->BroadcastMessage("  TimerId (%u)  %u ms left", intTimer, _getTimeForTimer(intTimer));
        }
        else
        {
            for (const auto& intTimer : mCreatureTimer)
                player->BroadcastMessage("  TimerId (%u)  %u ms left", intTimer.first, intTimer.second);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// ai upodate frequency
void CreatureAIScript::registerAiUpdateFrequency()
{
    sEventMgr.AddEvent(_creature, &Creature::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, mAIUpdateFrequency, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void CreatureAIScript::removeAiUpdateFrequency()
{
    sEventMgr.RemoveEvents(_creature, EVENT_SCRIPT_UPDATE_EVENT);
}

// old stuff
void CreatureAIScript::SetAIUpdateFreq(uint32_t pUpdateFreq)
{
    if (mCustomAIUpdateDelay != pUpdateFreq)
    {
        mCustomAIUpdateDelay = pUpdateFreq;
        _resetTimer(mCustomAIUpdateDelayTimerId, mCustomAIUpdateDelay);
    }
}

uint32_t CreatureAIScript::GetAIUpdateFreq()
{
    return mAIUpdateFrequency;
}

void CreatureAIScript::RegisterAIUpdateEvent(uint32_t frequency)
{
    if (mCustomAIUpdateDelayTimerId == 0)
    {
        mCustomAIUpdateDelayTimerId = _addTimer(frequency);
        mCustomAIUpdateDelay = frequency;
    }
    else
    {
        ModifyAIUpdateEvent(frequency);
    }
}

void CreatureAIScript::ModifyAIUpdateEvent(uint32_t newfrequency)
{
    if (mCustomAIUpdateDelayTimerId != 0)
    {
        _resetTimer(mCustomAIUpdateDelayTimerId, newfrequency);
        mCustomAIUpdateDelay = newfrequency;
    }
}

void CreatureAIScript::RemoveAIUpdateEvent()
{
    if (mCustomAIUpdateDelayTimerId != 0)
    {
        _removeTimer(mCustomAIUpdateDelayTimerId);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// appearance

void CreatureAIScript::_setScale(float scale)
{
    _creature->setFloatValue(OBJECT_FIELD_SCALE_X, scale);
}

float CreatureAIScript::_getScale()
{
    return _creature->getFloatValue(OBJECT_FIELD_SCALE_X);
}

void CreatureAIScript::_setDisplayId(uint32_t displayId)
{
    _creature->SetDisplayId(displayId);
}

void CreatureAIScript::_setWieldWeapon(bool setWieldWeapon)
{
    if (setWieldWeapon && _creature->getUInt32Value(UNIT_FIELD_BYTES_2) != 1)
    {
        _creature->setUInt32Value(UNIT_FIELD_BYTES_2, 1);
    }
    else if (!setWieldWeapon && _creature->getUInt32Value(UNIT_FIELD_BYTES_2) != 0)
    {
        _creature->setUInt32Value(UNIT_FIELD_BYTES_2, 0);
    }
}

void CreatureAIScript::_setDisplayWeapon(bool setMainHand, bool setOffHand)
{
    _setDisplayWeaponIds(setMainHand ? _creature->GetEquippedItem(MELEE) : 0, setOffHand ? _creature->GetEquippedItem(OFFHAND) : 0);
}

void CreatureAIScript::_setDisplayWeaponIds(uint32_t itemId1, uint32_t itemId2)
{
    _creature->SetEquippedItem(MELEE, itemId1);
    _creature->SetEquippedItem(OFFHAND, itemId2);
}

//////////////////////////////////////////////////////////////////////////////////////////
// spell

CreatureAISpells* CreatureAIScript::addAISpell(uint32_t spellId, float castChance, uint32_t targetType, uint32_t duration /*= 0*/, uint32_t cooldown /*= 0*/, bool forceRemove /*= false*/, bool isTriggered /*= false*/)
{
    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
    if (spellInfo != nullptr)
    {
        uint32_t spellDuration = duration * 1000;
        if (spellDuration == 0)
            spellDuration = spellInfo->getSpellDuration(nullptr);

        uint32_t spellCooldown = cooldown * 1000;
        if (spellCooldown == 0)
            spellCooldown = spellInfo->getSpellDuration(nullptr);

        CreatureAISpells* newAISpell = new CreatureAISpells(spellInfo, castChance, targetType, spellDuration, spellCooldown, forceRemove, isTriggered);

        mCreatureAISpells.push_back(newAISpell);

        newAISpell->setdurationTimer(_addTimer(spellDuration));
        newAISpell->setCooldownTimerId(_addTimer(0));

        return newAISpell;
    }

    LOG_ERROR("tried to add invalid spell with id %u", spellId);

    // assert spellInfo can not be nullptr!
    ARCEMU_ASSERT(spellInfo != nullptr);
    return nullptr;
}

void CreatureAIScript::_applyAura(uint32_t spellId)
{
    _creature->CastSpell(_creature, sSpellCustomizations.GetSpellInfo(spellId), true);
}

void CreatureAIScript::_removeAura(uint32_t spellId)
{
    _creature->RemoveAura(spellId);
}

void CreatureAIScript::_removeAllAuras()
{
    _creature->RemoveAllAuras();
}

void CreatureAIScript::_removeAuraOnPlayers(uint32_t spellId)
{
    for (auto object : _creature->getInRangePlayersSet())
    {
        if (object != nullptr)
            static_cast<Player*>(object)->RemoveAura(spellId);
    }
}

void CreatureAIScript::_castOnInrangePlayers(uint32_t spellId, bool triggered)
{
    for (auto object : _creature->getInRangePlayersSet())
    {
        if (object != nullptr)
            _creature->CastSpell(static_cast<Player*>(object), spellId, triggered);
    }
}

void CreatureAIScript::_castOnInrangePlayersWithinDist(float minDistance, float maxDistance, uint32_t spellId, bool triggered)
{
    for (auto object : _creature->getInRangePlayersSet())
    {
        if (object != nullptr)
        {
            float distanceToPlayer = object->GetDistance2dSq(this->getCreature());
            if (distanceToPlayer >= minDistance && distanceToPlayer <= maxDistance)
                _creature->CastSpell(static_cast<Player*>(object), spellId, triggered);
        }
    }
}

void CreatureAIScript::_castAISpell(CreatureAISpells* aiSpell)
{
    Unit* target = getCreature()->GetAIInterface()->getNextTarget();
    switch (aiSpell->mTargetType)
    {
        case TARGET_SELF:
        case TARGET_VARIOUS:
        {
            getCreature()->CastSpell(getCreature(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
            mLastCastedSpell = aiSpell;
        } break;
        case TARGET_ATTACKING:
        {
            getCreature()->CastSpell(target, aiSpell->mSpellInfo, aiSpell->mIsTriggered);
            mCurrentSpellTarget = target;
            mLastCastedSpell = aiSpell;
        } break;
        case TARGET_DESTINATION:
        {
            getCreature()->CastSpellAoF(target->GetPosition(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
            mCurrentSpellTarget = target;
            mLastCastedSpell = aiSpell;
        } break;
        case TARGET_RANDOM_FRIEND:
        case TARGET_RANDOM_SINGLE:
        case TARGET_RANDOM_DESTINATION:
        {
            castSpellOnRandomTarget(aiSpell);
            mLastCastedSpell = aiSpell;
        } break;
        case TARGET_CUSTOM:
        {
            if (aiSpell->getCustomTarget() != nullptr)
                getCreature()->CastSpell(aiSpell->getCustomTarget(), aiSpell->mSpellInfo, aiSpell->mIsTriggered);
        } break;
    }
}

void CreatureAIScript::_setTargetToChannel(Unit* target, uint32_t spellId)
{
    if (target != nullptr)
    {
        _creature->SetChannelSpellTargetGUID(target->GetGUID());
        _creature->SetChannelSpellId(spellId);
    }
    else
    {
        _unsetTargetToChannel();
    }
}

void CreatureAIScript::_unsetTargetToChannel()
{
    _creature->SetChannelSpellTargetGUID(0);
    _creature->SetChannelSpellId(0);
}

Unit* CreatureAIScript::_getTargetToChannel()
{
    return _creature->GetMapMgr()->GetUnit(_creature->getUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
}

void CreatureAIScript::newAIUpdateSpellSystem()
{
    if (mLastCastedSpell)
    {
        if (!_isTimerFinished(mSpellWaitTimerId))
        {
            // spell has a min/max range
            if (!getCreature()->isCastingNonMeleeSpell() && (mLastCastedSpell->mMaxPositionRangeToCast > 0.0f || mLastCastedSpell->mMinPositionRangeToCast > 0.0f))
            {
                // if we have a current target and spell is not triggered
                if (mCurrentSpellTarget != nullptr && !mLastCastedSpell->mIsTriggered)
                {
                    // interrupt spell if we are not in  required range
                    const float targetDistance = getCreature()->GetPosition().Distance2DSq(mCurrentSpellTarget->GetPositionX(), mCurrentSpellTarget->GetPositionY());
                    if (!mLastCastedSpell->isDistanceInRange(targetDistance))
                    {
                        LogDebugFlag(LF_SCRIPT_MGR, "Target outside of spell range (%u)! Min: %f Max: %f, distance to Target: %f", mLastCastedSpell->mSpellInfo->getId(), mLastCastedSpell->mMinPositionRangeToCast, mLastCastedSpell->mMaxPositionRangeToCast, targetDistance);
                        getCreature()->interruptSpell();
                        mLastCastedSpell = nullptr;
                    }
                }
            }
        }
        else
        {
            // spell gets not interupted after casttime(duration) so we can send the emote.
            mLastCastedSpell->sendRandomEmote(this);

            // override attack stop timer if needed
            if (mLastCastedSpell->getAttackStopTimer() != 0)
                getCreature()->setAttackTimer(mLastCastedSpell->getAttackStopTimer(), false);

            mLastCastedSpell = nullptr;
        }
    }

    // cleanup exeeded spells
    for (const auto& AISpell : mCreatureAISpells)
    {
        if (AISpell != nullptr)
        {
            // stop spells and remove aura in case of duration
            if (_isTimerFinished(AISpell->mDurationTimerId) && AISpell->mForceRemoveAura)
            {
                getCreature()->interruptSpell();
                _removeAura(AISpell->mSpellInfo->getId());
            }
        }
    }

    // cast one spell and check if spell is done (duration)
    if (_isTimerFinished(mSpellWaitTimerId))
    {
        CreatureAISpells* usedSpell = nullptr;

        float randomChance = Util::getRandomFloat(100.0f);
        std::random_shuffle(mCreatureAISpells.begin(), mCreatureAISpells.end());
        for (const auto& AISpell : mCreatureAISpells)
        {
            if (AISpell != nullptr)
            {
                // spell was casted before, check if the wait time is done
                if (!_isTimerFinished(AISpell->mCooldownTimerId))
                    continue;

                // is bound to a specific phase (all greater than 0)
                if (!AISpell->isAvailableForScriptPhase(getScriptPhase()))
                    continue;

                // aura stacking
                if (getCreature()->getAuraCountForId(AISpell->mSpellInfo->getId()) >= AISpell->getMaxStackCount())
                    continue;

                // hp range
                if (!AISpell->isHpInPercentRange(getCreature()->GetHealthPct()))
                    continue;

                // no random chance (cast in script)
                if (AISpell->mCastChance == 0.0f)
                    continue;

                // do not cast any spell while stunned/feared/silenced/charmed/confused
                if (getCreature()->hasUnitStateFlag(UNIT_STATE_STUN | UNIT_STATE_FEAR | UNIT_STATE_SILENCE | UNIT_STATE_CHARM | UNIT_STATE_CONFUSE))
                    break;

                // random chance for shuffeld array should do the job
                if (randomChance < AISpell->mCastChance)
                {
                    usedSpell = AISpell;
                    break;
                }
            }
        }

        if (usedSpell != nullptr)
        {
            Unit* target = getCreature()->GetAIInterface()->getNextTarget();
            switch (usedSpell->mTargetType)
            {
                case TARGET_SELF:
                case TARGET_VARIOUS:
                {
                    getCreature()->CastSpell(getCreature(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_ATTACKING:
                {
                    getCreature()->CastSpell(target, usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mCurrentSpellTarget = target;
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_DESTINATION:
                {
                    getCreature()->CastSpellAoF(target->GetPosition(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                    mCurrentSpellTarget = target;
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_RANDOM_FRIEND:
                case TARGET_RANDOM_SINGLE:
                case TARGET_RANDOM_DESTINATION:
                {
                    castSpellOnRandomTarget(usedSpell);
                    mLastCastedSpell = usedSpell;
                } break;
                case TARGET_CUSTOM:
                {
                    // nos custom target set, no spell cast.
                    if (usedSpell->getCustomTarget() != nullptr)
                        getCreature()->CastSpell(usedSpell->getCustomTarget(), usedSpell->mSpellInfo, usedSpell->mIsTriggered);
                } break;
            }

            // send announcements on casttime beginn
            usedSpell->sendAnnouncement(this);

            // reset cast wait timer for CreatureAIScript - Important for _internalAIUpdate
            _resetTimer(mSpellWaitTimerId, usedSpell->mDuration);

            // reset spell timers to cleanup exceeded spells
            _resetTimer(usedSpell->mDurationTimerId, usedSpell->mDuration);
            _resetTimer(usedSpell->mCooldownTimerId, usedSpell->mCooldown);

        }
    }
}

void CreatureAIScript::castSpellOnRandomTarget(CreatureAISpells* AiSpell)
{
    if (AiSpell == nullptr)
        return;

    // helper for following code
    bool isTargetRandFriend = (AiSpell->mTargetType == TARGET_RANDOM_FRIEND ? true : false);

    // if we already cast a spell, do not set/cast another one!
    if (!getCreature()->isCastingNonMeleeSpell()
        && getCreature()->GetAIInterface()->getNextTarget())
    {
        // set up targets in range by position, relation and hp range
        std::vector<Unit*> possibleUnitTargets;

        for (const auto& inRangeObject : getCreature()->getInRangeObjectsSet())
        {
            if (((isTargetRandFriend && isFriendly(getCreature(), inRangeObject))
                || (!isTargetRandFriend && isHostile(getCreature(), inRangeObject) && inRangeObject != getCreature())) && inRangeObject->IsUnit())
            {
                Unit* inRangeTarget = static_cast<Unit*>(inRangeObject);

                if (
                    inRangeTarget->isAlive() && AiSpell->isDistanceInRange(getCreature()->GetDistance2dSq(inRangeTarget))
                    && ((AiSpell->isHpInPercentRange(inRangeTarget->GetHealthPct()) && isTargetRandFriend)
                    || (getCreature()->GetAIInterface()->getThreatByPtr(inRangeTarget) > 0 && isHostile(getCreature(), inRangeTarget))))
                {
                    possibleUnitTargets.push_back(inRangeTarget);
                }
            }
        }

        // add us as a friendly target.
        if (AiSpell->isHpInPercentRange(getCreature()->GetHealthPct()) && isTargetRandFriend)
            possibleUnitTargets.push_back(getCreature());

        // no targets in our range for hp range and firendly targets
        if (possibleUnitTargets.empty())
            return;

        // get a random target
        uint32_t randomIndex = Util::getRandomUInt(0, static_cast<uint32_t>(possibleUnitTargets.size() - 1));
        Unit* randomTarget = possibleUnitTargets[randomIndex];

        if (randomTarget == nullptr)
            return;

        switch (AiSpell->mTargetType)
        {
            case TARGET_RANDOM_FRIEND:
            case TARGET_RANDOM_SINGLE:
            {
                getCreature()->CastSpell(randomTarget, AiSpell->mSpellInfo, AiSpell->mIsTriggered);
                mCurrentSpellTarget = randomTarget;
            } break;
            case TARGET_RANDOM_DESTINATION:
                getCreature()->CastSpellAoF(randomTarget->GetPosition(), AiSpell->mSpellInfo, AiSpell->mIsTriggered);
                break;
        }

        possibleUnitTargets.clear();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// gameobject

GameObject* CreatureAIScript::getNearestGameObject(uint32_t entry)
{
    return getNearestGameObject(_creature->GetPositionX(), _creature->GetPositionY(), _creature->GetPositionZ(), entry);
}

GameObject* CreatureAIScript::getNearestGameObject(float posX, float posY, float posZ, uint32_t entry)
{
    return _creature->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(posX, posY, posZ, entry);
}

//////////////////////////////////////////////////////////////////////////////////////////
// chat message

void CreatureAIScript::sendChatMessage(uint8_t type, uint32_t soundId, std::string text)
{
    if (text.empty() == false)
        _creature->SendChatMessage(type, LANG_UNIVERSAL, text.c_str());

    if (soundId > 0)
        _creature->PlaySoundToSet(soundId);
}

void CreatureAIScript::sendDBChatMessage(uint32_t textId)
{
    _creature->SendScriptTextChatMessage(textId);
}

void CreatureAIScript::sendRandomDBChatMessage(std::vector<uint32_t> emoteVector)
{
    if (!emoteVector.empty())
    {
        uint32_t randomUInt = (emoteVector.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(emoteVector.size() - 1)) : 0;

        sendDBChatMessage(emoteVector[randomUInt]);
    }
}

void CreatureAIScript::addEmoteForEvent(uint32_t eventType, uint32_t scriptTextId)
{
    MySQLStructure::NpcScriptText const* ct = sMySQLStore.getNpcScriptText(scriptTextId);
    if (ct != nullptr)
    {
        switch (eventType)
        {
            case Event_OnCombatStart:
                mEmotesOnCombatStart.push_back(scriptTextId);
                break;
            case Event_OnTargetDied:
                mEmotesOnTargetDied.push_back(scriptTextId);
                break;
            case Event_OnDied:
                mEmotesOnDied.push_back(scriptTextId);
                break;
            case Event_OnTaunt:
                mEmotesOnTaunt.push_back(scriptTextId);
                break;
            case Event_OnIdle:
                mEmotesOnIdle.push_back(scriptTextId);
                break;
            default:
                LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::addEmoteForEvent : Invalid event type: %u !", eventType);
                break;
        }
    }
    else
    {
        LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::addEmoteForEvent : id: %u is not available in table npc_script_text!", scriptTextId);
    }
}

void CreatureAIScript::sendAnnouncement(std::string stringAnnounce)
{
    if (!stringAnnounce.empty())
        _creature->SendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, stringAnnounce.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
// idle emote timer

void CreatureAIScript::enableOnIdleEmote(bool enable, uint32_t durationInMs /*= 0*/)
{
    if (enable && mEmotesOnIdle.empty())
    {
        LogDebugFlag(LF_SCRIPT_MGR, "CreatureAIScript::enableOnIdleEmote : no IdleEvents available!");
        return;
    }

    isIdleEmoteEnabled = enable;

    if (isIdleEmoteEnabled)
    {
        setIdleEmoteTimerId(_addTimer(durationInMs));
    }
    else
    {
        uint32_t idleTimerId = getIdleEmoteTimerId();
        _removeTimer(idleTimerId);
    }
}

void CreatureAIScript::setIdleEmoteTimerId(uint32_t timerId)
{
    idleEmoteTimerId = timerId;
}

uint32_t CreatureAIScript::getIdleEmoteTimerId()
{
    return idleEmoteTimerId;
}

void CreatureAIScript::resetIdleEmoteTime(uint32_t durationInMs)
{
    _resetTimer(idleEmoteTimerId, durationInMs);
}

void CreatureAIScript::setRandomIdleEmoteTime(uint32_t minTime, uint32_t maxTime)
{
    idleEmoteTimeMin = minTime;
    idleEmoteTimeMax = maxTime;
}

void CreatureAIScript::generateNextRandomIdleEmoteTime()
{
    resetIdleEmoteTime(Util::getRandomUInt(idleEmoteTimeMin, idleEmoteTimeMax));
}

//////////////////////////////////////////////////////////////////////////////////////////
// instance

InstanceScript* CreatureAIScript::getInstanceScript()
{
    MapMgr* mapMgr = _creature->GetMapMgr();
    return (mapMgr) ? mapMgr->GetScript() : nullptr;
}

bool CreatureAIScript::_isHeroic()
{
    MapMgr* mapMgr = _creature->GetMapMgr();
    if (mapMgr == nullptr || mapMgr->iInstanceMode != MODE_HEROIC)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// linked creature AI scripts

void CreatureAIScript::removeLinkToCreatureAIScript()
{
    linkedCreatureAI = nullptr;
}

void CreatureAIScript::setLinkedCreatureAIScript(CreatureAIScript* creatureAI)
{
    //notify our linked creature that we are not more linked
    if (linkedCreatureAI != nullptr)
        linkedCreatureAI->removeLinkToCreatureAIScript();

    //link to the new creature
    linkedCreatureAI = creatureAI;
}

//////////////////////////////////////////////////////////////////////////////////////////
// target

Unit* CreatureAIScript::getBestPlayerTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //Build potential target list
    UnitArray TargetArray;
    for (const auto& PlayerIter : getCreature()->getInRangePlayersSet())
    {
        if (PlayerIter && isValidUnitTarget(PlayerIter, pTargetFilter, pMinRange, pMaxRange))
            TargetArray.push_back(static_cast<Unit*>(PlayerIter));
    }

    return getBestTargetInArray(TargetArray, pTargetFilter);
}

Unit* CreatureAIScript::getBestUnitTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange)
{
    //potential target list
    UnitArray TargetArray;
    if (pTargetFilter & TargetFilter_Friendly)
    {
        for (const auto& ObjectIter : getCreature()->getInRangeObjectsSet())
        {
            if (ObjectIter && isValidUnitTarget(ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(ObjectIter));
        }

        if (isValidUnitTarget(getCreature(), pTargetFilter))
            TargetArray.push_back(getCreature());    //add self as possible friendly target
    }
    else
    {
        for (const auto& ObjectIter : getCreature()->getInRangeOppositeFactionSet())
        {
            if (ObjectIter && isValidUnitTarget(ObjectIter, pTargetFilter, pMinRange, pMaxRange))
                TargetArray.push_back(static_cast<Unit*>(ObjectIter));
        }
    }

    return getBestTargetInArray(TargetArray, pTargetFilter);
}

Unit* CreatureAIScript::getBestTargetInArray(UnitArray & pTargetArray, TargetFilter pTargetFilter)
{
    //only one possible target, return it
    if (pTargetArray.size() == 1)
        return pTargetArray[0];

    //closest unit if requested
    if (pTargetFilter & TargetFilter_Closest)
        return getNearestTargetInArray(pTargetArray);

    //second most hated if requested
    if (pTargetFilter & TargetFilter_SecondMostHated)
        return getSecondMostHatedTargetInArray(pTargetArray);

    //random unit in array
    return (pTargetArray.size() > 1) ? pTargetArray[Util::getRandomUInt((uint32_t)pTargetArray.size() - 1)] : nullptr;
}

Unit* CreatureAIScript::getNearestTargetInArray(UnitArray& pTargetArray)
{
    Unit* NearestUnit = nullptr;

    float Distance, NearestDistance = 99999;
    for (const auto& UnitIter : pTargetArray)
    {
        if (UnitIter != nullptr)
        {
            Distance = getRangeToObject(static_cast<Unit*>(UnitIter));
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestUnit = UnitIter;
            }
        }
    }

    return NearestUnit;
}

Unit* CreatureAIScript::getSecondMostHatedTargetInArray(UnitArray & pTargetArray)
{
    Unit* TargetUnit = nullptr;
    Unit* MostHatedUnit = nullptr;

    Unit* CurrentTarget = static_cast<Unit*>(getCreature()->GetAIInterface()->getNextTarget());
    uint32_t Threat = 0;
    uint32_t HighestThreat = 0;

    for (const auto& UnitIter : pTargetArray)
    {
        if (UnitIter != nullptr)
        {
            TargetUnit = static_cast<Unit*>(UnitIter);
            if (TargetUnit != CurrentTarget)
            {
                Threat = getCreature()->GetAIInterface()->getThreatByPtr(TargetUnit);
                if (Threat > HighestThreat)
                {
                    MostHatedUnit = TargetUnit;
                    HighestThreat = Threat;
                }
            }
        }
    }

    return MostHatedUnit;
}

bool CreatureAIScript::isValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange, float pMaxRange)
{
    if (!pObject->IsUnit())
        return false;

    if (pObject->GetInstanceID() != getCreature()->GetInstanceID())
        return false;

    Unit* UnitTarget = static_cast<Unit*>(pObject);
    //Skip dead (if required), feign death or invisible targets
    if (pFilter & TargetFilter_Corpse)
    {
        if (UnitTarget->isAlive() || !UnitTarget->IsCreature() || static_cast<Creature*>(UnitTarget)->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            return false;
    }
    else if (!UnitTarget->isAlive())
        return false;

    if (UnitTarget->IsPlayer() && static_cast<Player*>(UnitTarget)->m_isGmInvisible)
        return false;

    if (UnitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
        return false;

    // if we apply target filtering
    if (pFilter != TargetFilter_None)
    {
        // units not on threat list
        if ((pFilter & TargetFilter_Aggroed) && getCreature()->GetAIInterface()->getThreatByPtr(UnitTarget) == 0)
            return false;

        // current attacking target if requested
        if ((pFilter & TargetFilter_NotCurrent) && UnitTarget == getCreature()->GetAIInterface()->getNextTarget())
            return false;

        // only wounded targets if requested
        if ((pFilter & TargetFilter_Wounded) && UnitTarget->GetHealthPct() >= 99)
            return false;

        // targets not in melee range if requested
        if ((pFilter & TargetFilter_InMeleeRange) && getRangeToObject(UnitTarget) > getCreature()->GetAIInterface()->_CalcCombatRange(UnitTarget, false))
            return false;

        // targets not in strict range if requested
        if ((pFilter & TargetFilter_InRangeOnly) && (pMinRange > 0 || pMaxRange > 0))
        {
            float Range = getRangeToObject(UnitTarget);
            if (pMinRange > 0 && Range < pMinRange)
                return false;

            if (pMaxRange > 0 && Range > pMaxRange)
                return false;
        }

        // targets not in Line Of Sight if requested
        if ((~pFilter & TargetFilter_IgnoreLineOfSight) && !getCreature()->IsWithinLOSInMap(UnitTarget))
            return false;

        // hostile/friendly
        if ((~pFilter & TargetFilter_Corpse) && (pFilter & TargetFilter_Friendly))
        {
            if (!UnitTarget->CombatStatus.IsInCombat())
                return false; // not-in-combat targets if friendly

            if (isHostile(getCreature(), UnitTarget) || getCreature()->GetAIInterface()->getThreatByPtr(UnitTarget) > 0)
                return false;
        }
    }

    return true;
}
