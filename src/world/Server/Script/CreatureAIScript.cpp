/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/



#include "CreatureAIScript.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Management/Faction.h"
#include "Spell/Definitions/PowerType.hpp"
#include "Movement/Spline/MoveSplineInit.h"
#include "Movement/WaypointManager.h"

void SummonList::summon(Creature const* summon)
{
    _storage.push_back(summon->getGuid());
}

void SummonList::despawn(Creature const* summon)
{
    _storage.remove(summon->getGuid());
}

void SummonList::despawnEntry(uint32_t entry)
{
    for (StorageType::iterator i = _storage.begin(); i != _storage.end();)
    {
        Creature* summon = _creature->getWorldMapCreature(*i);
        if (!summon)
        {
            i = _storage.erase(i);
        }
        else if (summon->getEntry() == entry)
        {
            i = _storage.erase(i);
            summon->Despawn(1000, 0);
        }
        else
        {
            ++i;
        }
    }
}

void SummonList::despawnAll()
{
    while (!_storage.empty())
    {
        Creature* summon = _creature->getWorldMapCreature(_storage.front());
        _storage.pop_front();
        if (summon)
            summon->Despawn(1000, 0);
    }
}

void SummonList::removeNotExisting()
{
    for (StorageType::iterator i = _storage.begin(); i != _storage.end();)
    {
        if (_creature->getWorldMapCreature(*i))
            ++i;
        else
            i = _storage.erase(i);
    }
}

bool SummonList::hasEntry(uint32_t entry) const
{
    for (uint64_t const& guid : _storage)
    {
        Creature* summon = _creature->getWorldMapCreature(guid);
        if (summon && summon->getEntry() == entry)
            return true;
    }

    return false;
}

CreatureAIScript::CreatureAIScript(Creature* creature) : mScriptPhase(0), summons(creature), mCreatureTimerCount(0), mAIUpdateFrequency(defaultUpdateFrequency),
isIdleEmoteEnabled(false), idleEmoteTimerId(0), idleEmoteTimeMin(0), idleEmoteTimeMax(0), _creature(creature), linkedCreatureAI(nullptr)
{
    mCreatureTimerIds.clear();
    mCreatureTimer.clear();

    mCustomAIUpdateDelayTimerId = 0;
    mCustomAIUpdateDelay = 0;

    m_oldAIUpdate.resetInterval(1000);
}

CreatureAIScript::~CreatureAIScript()
{
    //notify our linked creature that we are being deleted.
    if (linkedCreatureAI != nullptr)
        linkedCreatureAI->removeLinkToCreatureAIScript();

    _waypointStore.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Event default management
void CreatureAIScript::_internalOnDied(Unit* killer)
{
    sLogger.debug("CreatureAIScript::_internalOnDied() called");

    enableOnIdleEmote(false);

    _cancelAllTimers();
    _removeAllAuras();

    RemoveAIUpdateEvent();
    sendRandomDBChatMessage(mEmotesOnDied, killer);

    // Reset Events
    scriptEvents.resetEvents();

    // Remove Summons
    summons.despawnAll();

    // Finish Encounter
    if (getInstanceScript() != nullptr)
    {
#if VERSION_STRING >= WotLK
        getInstanceScript()->updateEncountersStateForCreature(getCreature()->getEntry(), getCreature()->getWorldMap()->getDifficulty());
#endif
    }

    resetScriptPhase();
}

void CreatureAIScript::_internalOnTargetDied(Unit* target)
{
    sLogger.debug("CreatureAIScript::_internalOnTargetDied() called");

    sendRandomDBChatMessage(mEmotesOnTargetDied, target);
}

void CreatureAIScript::_internalOnCombatStart(Unit* target)
{
    sLogger.debug("CreatureAIScript::_internalOnEnterCombat() called");

    enableOnIdleEmote(false);

    setAIAgent(AGENT_MELEE);

    sendRandomDBChatMessage(mEmotesOnCombatStart, target);

    if(isScriptPhase(0))
        setScriptPhase(1);

    RegisterAIUpdateEvent(mAIUpdateFrequency);
}

void CreatureAIScript::_internalOnCombatStop()
{
    sLogger.debug("CreatureAIScript::_internalOnCombatStop() called");

    _cancelAllTimers();
    _removeAllAuras();
    setAIAgent(AGENT_NULL);
    getCreature()->getAIInterface()->setAiState(AI_STATE_IDLE);
    RemoveAIUpdateEvent();

    resetScriptPhase();
    enableOnIdleEmote(true);
}

void CreatureAIScript::_internalAIUpdate(unsigned long time_passed)
{
    //sLogger.debug("CreatureAIScript::_internalAIUpdate() called");

    updateAITimers(time_passed);
    AIUpdate(time_passed);

    // idleemotes
    if (!_isInCombat() && isIdleEmoteEnabled)
    {
        if (_isTimerFinished(getIdleEmoteTimerId()))
        {
            sendRandomDBChatMessage(mEmotesOnIdle, nullptr);
            generateNextRandomIdleEmoteTime();
        }
    }

    // old AIUpdate stuff is now handled by customAIUpdateTimer. Keep this until all scripts are updated to new logic.
    if (mCustomAIUpdateDelayTimerId != 0)
    {
        if (!_isTimerFinished(mCustomAIUpdateDelayTimerId))
            return;

        AIUpdate();

        _resetTimer(mCustomAIUpdateDelayTimerId, mCustomAIUpdateDelay);
    }
    else
    {
        m_oldAIUpdate.updateTimer(time_passed);

        // old Timer AIUpdate
        if (m_oldAIUpdate.isTimePassed())
        {
            AIUpdate();
            m_oldAIUpdate.resetInterval(1000);
        }
    }
}

void CreatureAIScript::_internalOnScriptPhaseChange()
{
    sLogger.debug("CreatureAIScript::_internalOnScriptPhaseChange() called");

    getCreature()->GetScript()->OnScriptPhaseChange(getScriptPhase());
}

//////////////////////////////////////////////////////////////////////////////////////////
// player
Player* CreatureAIScript::getNearestPlayer()
{
    if (_creature->getWorldMap()->getInterface() == nullptr)
        return nullptr;

    return _creature->getWorldMap()->getInterface()->getPlayerNearestCoords(_creature->GetPositionX(), _creature->GetPositionY(), _creature->GetPositionZ());
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
    if (_creature->getWorldMap()->getInterface() == nullptr)
        return nullptr;

    return _creature->getWorldMap()->getInterface()->getCreatureNearestCoords(posX, posY, posZ, entry);
}

void CreatureAIScript::GetCreatureListWithEntryInGrid(std::list<Creature*>& container, uint32 entry, float maxSearchRange /*= 250.0f*/)
{
    if (_creature->getWorldMap()->getInterface() == nullptr)
        return;

    _creature->getWorldMap()->getInterface()->getCreatureListWithEntryInRange(_creature, container, entry, maxSearchRange);
}

Creature* CreatureAIScript::findNearestCreature(uint32_t entry, float maxSearchRange /*= 250.0f*/)
{
    if (_creature->getWorldMap()->getInterface() == nullptr)
        return nullptr;

    return _creature->getWorldMap()->getInterface()->findNearestCreature(_creature, entry, maxSearchRange);
}

void CreatureAIScript::GetGameObjectListWithEntryInGrid(std::list<GameObject*>& container, uint32 entry, float maxSearchRange /*= 250.0f*/)
{
    if (_creature->getWorldMap()->getInterface() == nullptr)
        return;

    _creature->getWorldMap()->getInterface()->getGameObjectListWithEntryInRange(_creature, container, entry, maxSearchRange);
}

GameObject* CreatureAIScript::findNearestGameObject(uint32_t entry, float maxSearchRange /*= 250.0f*/)
{
    if (_creature->getWorldMap()->getInterface() == nullptr)
        return nullptr;

    return _creature->getWorldMap()->getInterface()->findNearestGameObject(_creature, entry, maxSearchRange);
}

float CreatureAIScript::getRangeToObject(Object* object)
{
    return _creature->CalcDistance(object);
}

CreatureAIScript* CreatureAIScript::spawnCreatureAndGetAIScript(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId /* = 0*/, uint32_t phase /*= 1*/)
{
    Creature* creature = spawnCreature(entry, posX, posY, posZ, posO, factionId, phase);
    return (creature ? creature->GetScript() : nullptr);
}

Creature* CreatureAIScript::spawnCreature(uint32_t entry, LocationVector pos, uint32_t factionId /*= 0*/, uint32_t phase /*= 1*/)
{
    return spawnCreature(entry, pos.x, pos.y, pos.z, pos.o, factionId, phase);
}

Creature* CreatureAIScript::spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId /* = 0*/, uint32_t phase /*= 1*/)
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(entry);
    if (creatureProperties == nullptr)
    {
        sLogger.failure("tried to create an invalid creature with entry %u!", entry);
        return nullptr;
    }

    if (_creature->getWorldMap()->getInterface() == nullptr)
        return nullptr;

    Creature* creature = _creature->getWorldMap()->getInterface()->spawnCreature(entry, LocationVector(posX, posY, posZ, posO), true, true, 0, 0, phase);
    if (creature == nullptr)
        return nullptr;

    if (factionId != 0)
        creature->setFaction(factionId);
    else
        creature->setFaction(creatureProperties->Faction);

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
        _creature->getAIInterface()->setCurrentAgent(agent);
}

uint8_t CreatureAIScript::getAIAgent()
{
    return _creature->getAIInterface()->getCurrentAgent();
}

//////////////////////////////////////////////////////////////////////////////////////////
// movement
void CreatureAIScript::setRooted(bool set)
{
    _creature->setControlled(set, UNIT_STATE_ROOTED);
}

void CreatureAIScript::setFlyMode(bool fly)
{
    if (fly && !_creature->IsFlying())
    {
        _creature->setMoveCanFly(true);
    }
    else if (!fly && _creature->IsFlying())
    {
        _creature->setMoveCanFly(false);
    }
}

bool CreatureAIScript::isRooted()
{
    return _creature->isRooted();
}

void CreatureAIScript::moveTo(float posX, float posY, float posZ, bool setRun /*= true*/)
{
    if (setRun)
        _creature->setMoveWalk(false);
    else
        _creature->setMoveWalk(true);

    _creature->getMovementManager()->movePoint(0, posX, posY, posZ);
}

// Replace this with splines
void CreatureAIScript::MoveTeleport(float posX, float posY, float posZ, float posO /*= 0.0f*/)
{
    getCreature()->SetPosition(posX, posY, posZ, posO, false);

    WorldPacket data(SMSG_MONSTER_MOVE, 50);
    data << getCreature()->GetNewGUID();
    data << uint8_t(0);
    data << getCreature()->GetPositionX();
    data << getCreature()->GetPositionY();
    data << getCreature()->GetPositionZ();
    data << Util::getMSTime();
    data << uint8_t(0x0);
    data << uint32_t(0x100);
    data << uint32_t(1);
    data << uint32_t(1);
    data << posX;
    data << posY;
    data << posZ;
    getCreature()->sendMessageToSet(&data, false);
}

// Replace this with splines
void CreatureAIScript::MoveTeleport(LocationVector loc)
{
    getCreature()->SetPosition(loc, false);

    WorldPacket data(SMSG_MONSTER_MOVE, 50);
    data << getCreature()->GetNewGUID();
    data << uint8_t(0);
    data << getCreature()->GetPositionX();
    data << getCreature()->GetPositionY();
    data << getCreature()->GetPositionZ();
    data << Util::getMSTime();
    data << uint8_t(0x0);
    data << uint32_t(0x100);
    data << uint32_t(1);
    data << uint32_t(1);
    data << loc.x;
    data << loc.y;
    data << loc.z;
    getCreature()->sendMessageToSet(&data, false);
}

void CreatureAIScript::moveToUnit(Unit* unit)
{
    if (unit != nullptr)
        moveTo(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ());
}

void CreatureAIScript::moveToSpawn()
{
    _creature->getMovementManager()->moveTargetedHome();
}

void CreatureAIScript::stopMovement()
{
    _creature->stopMoving();
}

//////////////////////////////////////////////////////////////////////////////////////////
// wp movement
void CreatureAIScript::loadCustomWaypoins(uint32_t pathid)
{
    auto path = sWaypointMgr->getCustomScriptWaypointPath(pathid);

    if (!path)
        return;

    for (auto node : path->nodes)
    {
        addWaypoint(pathid, node);
    }
}

WaypointNode CreatureAIScript::createWaypoint(uint32_t pId, uint32_t pWaittime, uint32_t pMoveType, LocationVector pCoords)
{
    WaypointNode waypoint;
    waypoint.id = pId;
    waypoint.x = pCoords.x;
    waypoint.y = pCoords.y;
    waypoint.z = pCoords.z;
    waypoint.orientation = pCoords.o;
    waypoint.moveType = pMoveType;

    if (waypoint.moveType >= WAYPOINT_MOVE_TYPE_MAX)
    {
        sLogger.failure("Waypoint %u has invalid move_type, setting default", waypoint.id);
        waypoint.moveType = WAYPOINT_MOVE_TYPE_WALK;
    }

    waypoint.delay = pWaittime;
    waypoint.eventId = 0;
    waypoint.eventChance = 0;

    return waypoint;
}

void CreatureAIScript::addWaypoint(uint32_t pathid ,WaypointNode pWayPoint)
{
    WaypointPath& path = _waypointStore[pathid];
    path.id = pathid;
    path.nodes.push_back(std::move(pWayPoint));
}

WaypointPath* CreatureAIScript::getCustomPath(uint32_t pathId)
{
    auto itr = _waypointStore.find(pathId);
    if (itr != _waypointStore.end())
        return &itr->second;

    return nullptr;
}

void CreatureAIScript::setWaypointToMove(uint32_t pathId, uint32_t pWaypointId)
{
    auto _path = getCustomPath(pathId);
    WaypointNode const &waypoint = _path->nodes[pWaypointId];

    MovementNew::MoveSplineInit init(getCreature());
    init.MoveTo(waypoint.x, waypoint.y, waypoint.z);

    //! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
    if (waypoint.orientation && waypoint.delay)
        init.SetFacing(waypoint.orientation);

    switch (waypoint.moveType)
    {
    case WAYPOINT_MOVE_TYPE_LAND:
        init.SetAnimation(UnitBytes1_AnimationFlags::UNIT_BYTE1_FLAG_GROUND);
        break;
    case WAYPOINT_MOVE_TYPE_TAKEOFF:
        init.SetAnimation(UnitBytes1_AnimationFlags::UNIT_BYTE1_FLAG_HOVER);
        break;
    case WAYPOINT_MOVE_TYPE_RUN:
        init.SetWalk(false);
        break;
    case WAYPOINT_MOVE_TYPE_WALK:
        init.SetWalk(true);
        break;
    default:
        break;
    }

    init.Launch();
}

void CreatureAIScript::stopWaypointMovement()
{
    getCreature()->stopMoving();
}

uint32_t CreatureAIScript::getCurrentWaypoint()
{
    return getCreature()->getCurrentWaypointInfo().first;
}

size_t CreatureAIScript::getWaypointCount(uint32_t pathId)
{
    if (getCustomPath(pathId))
        return getCustomPath(pathId)->nodes.size();

    return 0;
}

bool CreatureAIScript::hasWaypoints(uint32_t pathId)
{
    if (getCustomPath(pathId))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// combat setup

bool CreatureAIScript::canEnterCombat()
{
    return _creature->getAIInterface()->getAllowedToEnterCombat();
}

void CreatureAIScript::setCanEnterCombat(bool enterCombat)
{
    //Zyres 10/21/2017 creatures can be attackable even if they can not enter combat... the following line is not correct.
    if (enterCombat)
    {
        _creature->removeUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }
    else
    {
        _creature->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
    }

    _creature->getAIInterface()->setAllowedToEnterCombat(enterCombat);
}

bool CreatureAIScript::_isInCombat()
{
    return _creature->getCombatHandler().isInCombat();
}

void CreatureAIScript::_delayNextAttack(uint32_t milliseconds)
{
    _creature->setAttackTimer(MELEE, milliseconds);
}

void CreatureAIScript::_setMeleeDisabled(bool disable)
{
    _creature->getAIInterface()->setMeleeDisabled(disable);
}

bool CreatureAIScript::_isMeleeDisabled()
{
    return _creature->getAIInterface()->isMeleeDisabled();
}

void CreatureAIScript::_setRangedDisabled(bool disable)
{
    _creature->getAIInterface()->setRangedDisabled(disable);
}

bool CreatureAIScript::_isRangedDisabled()
{
    return _creature->getAIInterface()->isRangedDisabled();
}

void CreatureAIScript::_setCastDisabled(bool disable)
{
    _creature->getAIInterface()->setCastDisabled(disable);
}

bool CreatureAIScript::_isCastDisabled()
{
    return _creature->getAIInterface()->isCastDisabled();
}

void CreatureAIScript::_setTargetingDisabled(bool disable)
{
    _creature->getAIInterface()->setTargetingDisabled(disable);
}

bool CreatureAIScript::_isTargetingDisabled()
{
    return _creature->getAIInterface()->isTargetingDisabled();
}

void CreatureAIScript::_clearHateList()
{
    _creature->getThreatManager().resetAllThreat();
}

void CreatureAIScript::_wipeHateList()
{
    _creature->getThreatManager().clearAllThreat();
}

int32_t CreatureAIScript::_getHealthPercent()
{
    return _creature->getHealthPct();
}

int32_t CreatureAIScript::_getManaPercent()
{
    return _creature->getPowerPct(POWER_TYPE_MANA);
}

void CreatureAIScript::_regenerateHealth()
{
    _creature->RegenerateHealth();
}

bool CreatureAIScript::_isCasting()
{
    return _creature->isCastingSpell();
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
        timerId = 0;
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

    sLogger.debug("CreatureAIScript::_cancelAllTimers() - all cleared!");
}

uint32_t CreatureAIScript::_getTimerCount()
{
    if (InstanceScript* inScript = getInstanceScript())
        return static_cast<uint32_t>(mCreatureTimerIds.size());

    return static_cast<uint32_t>(mCreatureTimer.size());
}

void CreatureAIScript::updateAITimers(unsigned long time_passed)
{
    for (auto& TimerIter : mCreatureTimer)
    {
        if (TimerIter.second > 0)
        {
            if (TimerIter.second > time_passed)
                TimerIter.second -= time_passed;
            else
                TimerIter.second = 0;
        }
    }
}

void CreatureAIScript::displayCreatureTimerList(Player* player)
{
    player->broadcastMessage("=== Timers for creature %s ===", getCreature()->GetCreatureProperties()->Name.c_str());

    if (mCreatureTimerIds.empty() && mCreatureTimer.empty())
    {
        player->broadcastMessage("  No Timers available!");
    }
    else
    {
        if (InstanceScript* inScript = getInstanceScript())
        {
            for (const auto& intTimer : mCreatureTimerIds)
                player->broadcastMessage("  TimerId (%u)  %u ms left", intTimer, _getTimeForTimer(intTimer));
        }
        else
        {
            for (const auto& intTimer : mCreatureTimer)
                player->broadcastMessage("  TimerId (%u)  %u ms left", intTimer.first, intTimer.second);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// ai upodate frequency
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
    _creature->setScale(scale);
}

float CreatureAIScript::_getScale()
{
    return _creature->getScale();
}

void CreatureAIScript::_setDisplayId(uint32_t displayId)
{
    _creature->setDisplayId(displayId);
}

void CreatureAIScript::_setWieldWeapon(bool setWieldWeapon)
{
    if (setWieldWeapon && _creature->getSheathType() != SHEATH_STATE_MELEE)
    {
        _creature->setSheathType(SHEATH_STATE_MELEE);
    }
    else if (!setWieldWeapon && _creature->getSheathType() != SHEATH_STATE_UNARMED)
    {
        _creature->setSheathType(SHEATH_STATE_UNARMED);
    }
}

void CreatureAIScript::_setDisplayWeapon(bool setMainHand, bool setOffHand)
{
    _setDisplayWeaponIds(setMainHand ? _creature->getVirtualItemSlotId(MELEE) : 0, setOffHand ? _creature->getVirtualItemSlotId(OFFHAND) : 0);
}

void CreatureAIScript::_setDisplayWeaponIds(uint32_t itemId1, uint32_t itemId2)
{
    _creature->setVirtualItemSlotId(MELEE, itemId1);
    _creature->setVirtualItemSlotId(OFFHAND, itemId2);
}

//////////////////////////////////////////////////////////////////////////////////////////
// spell

CreatureAISpells* CreatureAIScript::addAISpell(uint32_t spellId, float castChance, uint32_t targetType, uint32_t duration /*= 0*/, uint32_t cooldown /*= 0*/, bool forceRemove /*= false*/, bool isTriggered /*= false*/)
{
    auto aiSpell = getCreature()->getAIInterface()->addAISpell(spellId, castChance, targetType, duration, cooldown, forceRemove, isTriggered);

    if (aiSpell)
        return aiSpell;

    return nullptr;
}

void CreatureAIScript::_applyAura(uint32_t spellId)
{
    _creature->castSpell(_creature, sSpellMgr.getSpellInfo(spellId), true);
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
            _creature->castSpell(static_cast<Player*>(object), spellId, triggered);
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
                _creature->castSpell(static_cast<Player*>(object), spellId, triggered);
        }
    }
}

void CreatureAIScript::_castAISpell(CreatureAISpells* aiSpell)
{
    if (!aiSpell)
    {
        sLogger.failure("CreatureAISpells tried to cast nonexistant Spell");
        return;
    }

    getCreature()->getAIInterface()->castAISpell(aiSpell);
}

void CreatureAIScript::castSpellOnRandomTarget(CreatureAISpells* AiSpell)
{
    if (!AiSpell)
    {
        sLogger.failure("CreatureAISpells tried to cast nonexistant Spell");
        return;
    }

    getCreature()->getAIInterface()->castSpellOnRandomTarget(AiSpell);
}

void CreatureAIScript::_setTargetToChannel(Unit* target, uint32_t spellId)
{
    if (target != nullptr)
    {
        _creature->setChannelObjectGuid(target->getGuid());
        _creature->setChannelSpellId(spellId);
    }
    else
    {
        _unsetTargetToChannel();
    }
}

void CreatureAIScript::_unsetTargetToChannel()
{
    _creature->setChannelObjectGuid(0);
    _creature->setChannelSpellId(0);
}

Unit* CreatureAIScript::_getTargetToChannel()
{
    return _creature->getWorldMap()->getUnit(_creature->getChannelObjectGuid());
}

//////////////////////////////////////////////////////////////////////////////////////////
// gameobject

GameObject* CreatureAIScript::getNearestGameObject(uint32_t entry)
{
    return getNearestGameObject(_creature->GetPositionX(), _creature->GetPositionY(), _creature->GetPositionZ(), entry);
}

GameObject* CreatureAIScript::getNearestGameObject(float posX, float posY, float posZ, uint32_t entry)
{
    if (_creature->getWorldMap()->getInterface() == nullptr)
        return nullptr;

    return _creature->getWorldMap()->getInterface()->getGameObjectNearestCoords(posX, posY, posZ, entry);
}

//////////////////////////////////////////////////////////////////////////////////////////
// chat message

void CreatureAIScript::sendChatMessage(uint8_t type, uint32_t soundId, std::string text)
{
    if (text.empty() == false)
        _creature->sendChatMessage(type, LANG_UNIVERSAL, text.c_str());

    if (soundId > 0)
        _creature->PlaySoundToSet(soundId);
}

void CreatureAIScript::sendDBChatMessage(uint32_t textId, Unit* target/* = nullptr*/)
{
    _creature->SendScriptTextChatMessage(textId, target);
}

void CreatureAIScript::sendRandomDBChatMessage(std::vector<uint32_t> emoteVector, Unit* target)
{
    if (!emoteVector.empty())
    {
        uint32_t randomUInt = (emoteVector.size() > 1) ? Util::getRandomUInt(static_cast<uint32_t>(emoteVector.size() - 1)) : 0;

        sendDBChatMessage(emoteVector[randomUInt], target);
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
                sLogger.debug("CreatureAIScript::addEmoteForEvent : Invalid event type: %u !", eventType);
                break;
        }
    }
    else
    {
        sLogger.debug("CreatureAIScript::addEmoteForEvent : id: %u is not available in table npc_script_text!", scriptTextId);
    }
}

void CreatureAIScript::sendAnnouncement(std::string stringAnnounce)
{
    if (!stringAnnounce.empty())
        _creature->sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, stringAnnounce.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
// idle emote timer

void CreatureAIScript::enableOnIdleEmote(bool enable, uint32_t durationInMs /*= 0*/)
{
    if (enable && mEmotesOnIdle.empty())
    {
        sLogger.debug("CreatureAIScript::enableOnIdleEmote : no IdleEvents available!");
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
    WorldMap* mapMgr = _creature->getWorldMap();
    return (mapMgr) ? mapMgr->getScript() : nullptr;
}

bool CreatureAIScript::_isHeroic()
{
    WorldMap* mapMgr = _creature->getWorldMap();
    if (mapMgr == nullptr || mapMgr->getDifficulty() != InstanceDifficulty::DUNGEON_HEROIC)
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

Unit* CreatureAIScript::getBestPlayerTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange, int32_t auraId)
{
    //Build potential target list
    UnitArray TargetArray;
    for (const auto& PlayerIter : getCreature()->getInRangePlayersSet())
    {
        if (PlayerIter && isValidUnitTarget(PlayerIter, pTargetFilter, pMinRange, pMaxRange, auraId))
            TargetArray.push_back(static_cast<Unit*>(PlayerIter));
    }

    return getBestTargetInArray(TargetArray, pTargetFilter);
}

Unit* CreatureAIScript::getBestUnitTarget(TargetFilter pTargetFilter, float pMinRange, float pMaxRange, int32_t auraId)
{
    //potential target list
    UnitArray TargetArray;
    if (pTargetFilter & TargetFilter_Friendly)
    {
        for (const auto& ObjectIter : getCreature()->getInRangeObjectsSet())
        {
            if (ObjectIter && isValidUnitTarget(ObjectIter, pTargetFilter, pMinRange, pMaxRange, auraId))
                TargetArray.push_back(static_cast<Unit*>(ObjectIter));
        }

        if (isValidUnitTarget(getCreature(), pTargetFilter, 0.0f, 0.0f, auraId))
            TargetArray.push_back(getCreature());    //add self as possible friendly target
    }
    else
    {
        for (const auto& ObjectIter : getCreature()->getInRangeOppositeFactionSet())
        {
            if (ObjectIter && isValidUnitTarget(ObjectIter, pTargetFilter, pMinRange, pMaxRange, auraId))
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
            Distance = getRangeToObject(UnitIter);
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
    Unit* MostHatedUnit = nullptr;
    Unit* TargetUnit = nullptr;
    Unit* CurrentTarget = getCreature()->getAIInterface()->getCurrentTarget();
    uint32_t Threat = 0;
    uint32_t HighestThreat = 0;

    for (const auto& UnitIter : pTargetArray)
    {
        if (UnitIter != nullptr)
        {
            TargetUnit = static_cast<Unit*>(UnitIter);
            if (TargetUnit != CurrentTarget)
            {
                Threat = static_cast<uint32_t>(getCreature()->getThreatManager().getThreat(TargetUnit));
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

bool CreatureAIScript::isValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange, float pMaxRange, int32_t auraId)
{
    if (!pObject->isCreatureOrPlayer())
        return false;

    if (pObject->GetInstanceID() != getCreature()->GetInstanceID())
        return false;

    Unit* UnitTarget = static_cast<Unit*>(pObject);
    //Skip dead (if required), feign death or invisible targets
    if (pFilter & TargetFilter_Corpse)
    {
        if (UnitTarget->isAlive() || !UnitTarget->isCreature() || static_cast<Creature*>(UnitTarget)->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            return false;
    }
    else if (!UnitTarget->isAlive())
        return false;

    if (UnitTarget->isPlayer() && static_cast<Player*>(UnitTarget)->m_isGmInvisible)
        return false;

    if (UnitTarget->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
        return false;

    // Required Aura
    if (auraId > 0)
    {
        if (!UnitTarget->hasAurasWithId(abs(auraId)))
            return false;
    }
    else
    {
        if (UnitTarget->hasAurasWithId(abs(auraId)))
            return false;
    }

    // if we apply target filtering
    if (pFilter != TargetFilter_None)
    {
        // units not on threat list
        if ((pFilter & TargetFilter_Aggroed) && getCreature()->getThreatManager().getThreat(UnitTarget) == 0)
            return false;

        // current attacking target if requested
        if ((pFilter & TargetFilter_NotCurrent) && UnitTarget == getCreature()->getAIInterface()->getCurrentTarget())
            return false;

        // only wounded targets if requested
        if ((pFilter & TargetFilter_Wounded) && UnitTarget->getHealthPct() >= 99)
            return false;

        // targets not in melee range if requested
        if ((pFilter & TargetFilter_InMeleeRange) && !getCreature()->isWithinCombatRange(UnitTarget, getCreature()->getMeleeRange(UnitTarget)))
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
            if (!UnitTarget->getCombatHandler().isInCombat())
                return false; // not-in-combat targets if friendly

            if (isHostile(getCreature(), UnitTarget) || getCreature()->getThreatManager().getThreat(UnitTarget) > 0)
                return false;
        }

        if ((pFilter & TargetFilter_Current) && UnitTarget != getCreature()->getAIInterface()->getCurrentTarget())
            return false;
    }

    return true;
}
