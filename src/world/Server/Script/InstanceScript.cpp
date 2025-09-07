/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "InstanceScript.hpp"

#include <sstream>

#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Management/LFG/LFGMgr.hpp"
#include "Map/Maps/InstanceMap.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/Packets/SmsgUpdateInstanceEncounterUnit.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStructures.hpp"

InstanceScript::InstanceScript(WorldMap* pMapMgr) : mInstance(pMapMgr)
{
    Difficulty = pMapMgr->getDifficulty();
    registerUpdateEvent();
}

#if VERSION_STRING > TBC
// Update Achievement Criteria for all players in instance
void InstanceScript::updateAchievementCriteria(AchievementCriteriaTypes type, uint32_t miscValue1 /*= 0*/, uint32_t miscValue2 /*= 0*/, Unit* unit /*= nullptr*/)
{
    if (!getInstance()->getPlayerCount())
        return;

    for (const auto& itr : getInstance()->getPlayers())
        if (Player* player = itr.second)
            player->updateAchievementCriteria(type, miscValue1, miscValue2, 0, unit);
}
#endif

void InstanceScript::setZoneMusic(uint32_t zoneId, uint32_t musicId)
{
    WorldPacket data(SMSG_PLAY_MUSIC, 4);
    data << uint32_t(musicId);
    sWorld.sendZoneMessage(&data, zoneId);
}

WorldMap* InstanceScript::getWorldMap() { return mInstance; }
InstanceMap* InstanceScript::getInstance() { return mInstance->getInstance(); }
uint8_t InstanceScript::GetDifficulty() { return Difficulty; }

//////////////////////////////////////////////////////////////////////////////////////////
// data

//used for debug
std::string InstanceScript::getDataStateString(uint8_t state)
{
    switch (state)
    {
        case NotStarted:
            return "Not started";
        case InProgress:
            return "In Progress";
        case Failed:
            return "Failed";
        case Performed:
            return "Performed";
        case PreProgress:
            return "PreProgress";
        default:
            return "Invalid";
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance Script Data Fast Access
void InstanceScript::setupInstanceData(ObjectData const* creatureData, ObjectData const* gameObjectData)
{
    if (creatureData)
        setupObjectData(creatureData, _creatureInfo);

    if (gameObjectData)
        setupObjectData(gameObjectData, _gameObjectInfo);
}

void InstanceScript::setupObjectData(ObjectData const* data, ObjectInfoMap& objectInfo)
{
    while (data->entry)
    {
        ASSERT(objectInfo.find(data->entry) == objectInfo.end());
        objectInfo[data->entry] = data->type;
        ++data;
    }
}
void InstanceScript::addObject(Object* obj)
{
    WoWGuid guid = obj->getGuid();

    if (obj->isCreature())
    {
        ObjectInfoMap::const_iterator j = _creatureInfo.find(obj->getEntry());
        if (j != _creatureInfo.end())
            _objectGuids[j->second] = guid.getGuidLowPart();
    }
    else if (obj->isGameObject())
    {
        ObjectInfoMap::const_iterator j = _gameObjectInfo.find(obj->getEntry());
        if (j != _gameObjectInfo.end())
            _objectGuids[j->second] = guid.getGuidLowPart();
    }
}

void InstanceScript::removeObject(Object* obj)
{
    WoWGuid guid = obj->getGuid();

    if (obj->isCreature())
    {
        ObjectInfoMap::const_iterator j = _creatureInfo.find(obj->getEntry());
        if (j != _creatureInfo.end())
        {
            ObjectGuidMap::iterator i = _objectGuids.find(j->second);
            if (i != _objectGuids.end() && i->second == guid.getGuidLowPart())
                _objectGuids.erase(i);
        }
    }
    else if (obj->isGameObject())
    {
        ObjectInfoMap::const_iterator j = _gameObjectInfo.find(obj->getEntry());
        if (j != _gameObjectInfo.end())
        {
            ObjectGuidMap::iterator i = _objectGuids.find(j->second);
            if (i != _objectGuids.end() && i->second == guid.getGuidLowPart())
                _objectGuids.erase(i);
        }
    }
}

uint32_t InstanceScript::getGuidFromData(uint32_t type)
{
    ObjectGuidMap::const_iterator i = _objectGuids.find(type);
    if (i != _objectGuids.end())
        return i->second;

    return 0;
}

Creature* InstanceScript::getCreatureFromData(uint32_t type)
{
    return GetCreatureByGuid(getGuidFromData(type));
}

GameObject* InstanceScript::getGameObjectFromData(uint32_t type)
{
    return GetGameObjectByGuid(getGuidFromData(type));
}


//////////////////////////////////////////////////////////////////////////////////////////
// encounters
void InstanceScript::generateBossDataState()
{
    const auto* encounters = sObjectMgr.getDungeonEncounterList(getWorldMap()->getBaseMap()->getMapId(), getWorldMap()->getDifficulty());
    uint32_t i = 0;

    for (DungeonEncounterList::const_iterator itr = encounters->begin(); itr != encounters->end(); ++itr, ++i)
    {
        const auto encounter = itr->get();

        BossInfo* bossInfo = &bosses[i];
        bossInfo->entry = encounter->creditEntry;
        bossInfo->state = NotStarted;
    }

    // Set States
    for (i = 0; i < bosses.size(); ++i)
        setBossState(i, NotStarted);
}

bool InstanceScript::setBossState(uint32_t id, EncounterStates state)
{
    if (id < bosses.size())
    {
        BossInfo* bossInfo = &bosses[id];
        if (bossInfo->state == InvalidState) // loading
        {
            bossInfo->state = state;
            return false;
        }
        else
        {
            if (bossInfo->state == state)
                return false;

            if (bossInfo->state == Performed)
            {
                return false;
            }

            bossInfo->state = state;
            saveToDB();
        }

        OnEncounterStateChange(id, state);

        if (state == NotStarted)
            getInstance()->respawnBossLinkedGroups(bossInfo->entry);

        return true;
    }
    return false;
}
std::vector<BossInfo> InstanceScript::getBosses() { return bosses; }

EncounterStates InstanceScript::getBossState(uint32_t id) const { return id < bosses.size() ? bosses[id].state : InvalidState; }

uint32_t InstanceScript::getEncounterCount() const { return static_cast<uint32_t>(bosses.size()); }

void InstanceScript::saveToDB()
{
    std::string data = getSaveData();
    if (data.empty())
        return;

    CharacterDatabase.Execute("UPDATE instance SET completedEncounters=%u, data=\'%s\' WHERE id=%u", getCompletedEncounterMask(), data.c_str(), mInstance->getInstanceId());
}

void InstanceScript::loadSavedInstanceData(char const* data)
{
    if (!data)
    {
        sLogger.failure("Unable to load Saved Instance Data for Instance {} (Map {}, Instance Id: {}).", mInstance->getBaseMap()->getMapName(), mInstance->getBaseMap()->getMapId(), mInstance->getInstanceId());
        return;
    }

    std::istringstream loadStream(data);

    readSaveDataBossStates(loadStream);
    readSaveDataExtended(loadStream);

    sLogger.debug("Saved Instance Data Loaded for Instance {} (Map {}, Instance Id: {}) is complete.", mInstance->getBaseMap()->getMapName(), mInstance->getBaseMap()->getMapId(), mInstance->getInstanceId());
}

void InstanceScript::readSaveDataBossStates(std::istringstream& data)
{
    const auto* encounters = sObjectMgr.getDungeonEncounterList(getWorldMap()->getBaseMap()->getMapId(), getWorldMap()->getDifficulty());
    size_t i = 0;

    for (DungeonEncounterList::const_iterator itr = encounters->begin(); itr != encounters->end(); ++itr, ++i)
    {
        const auto encounter = itr->get();

        BossInfo* bossInfo = &bosses[i];
        bossInfo->entry = encounter->creditEntry;
    }

    uint32_t bossId = 0;
    for (std::vector<BossInfo>::iterator itr = bosses.begin(); itr != bosses.end(); ++itr, ++bossId)
    {
        uint32_t buff;
        data >> buff;
        if (buff == InProgress || buff == Failed || buff == PreProgress)
            buff = NotStarted;

        if (buff < InvalidState)
        {
            setBossState(bossId, EncounterStates(buff));
        }
    }
}

void InstanceScript::writeSaveDataBossStates(std::ostringstream& data)
{
    for (auto const& bossInfo : bosses)
        data << uint32_t(bossInfo.state) << ' ';
}

std::string InstanceScript::getSaveData()
{
    std::ostringstream saveStream;

    writeSaveDataBossStates(saveStream);
    writeSaveDataExtended(saveStream);

    return saveStream.str();
}

#if VERSION_STRING >= WotLK
void InstanceScript::updateEncounterState(EncounterCreditType type, uint32_t creditEntry)
{
    DungeonEncounterList const* encounters = sObjectMgr.getDungeonEncounterList(mInstance->getBaseMap()->getMapId(), mInstance->getDifficulty());
    if (!encounters)
        return;

    uint32_t dungeonId = 0;

    for (auto const& encounter : *encounters)
    {
        if (encounter->creditType == type && encounter->creditEntry == creditEntry)
        {
            completedEncounters |= 1 << encounter->dbcEntry->encounterIndex;
            if (encounter->lastEncounterDungeon)
            {
                dungeonId = encounter->lastEncounterDungeon;
                break;
            }
        }
    }

    if (dungeonId)
    {
        for (auto const& playersPair : mInstance->getPlayers())
        {
            if (Player* player = playersPair.second)
            {
                if (const auto group = player->getGroup())
                {
                    if (group->isLFGGroup())
                    {
                        sLfgMgr.RewardDungeonDoneFor(dungeonId, player);
                        return;
                    }
                }
            }
        }
    }
}

void InstanceScript::useDoorOrButton(GameObject* pGameObject, uint32_t withRestoreTime, bool useAlternativeState)
{
    if (!pGameObject)
        return;

    if (pGameObject->getGoType() == GAMEOBJECT_TYPE_DOOR || pGameObject->getGoType() == GAMEOBJECT_TYPE_BUTTON)
    {
        if (pGameObject->getLootState() == GO_READY)
            pGameObject->useDoorOrButton(withRestoreTime, useAlternativeState);
        else if (pGameObject->getLootState() == GO_ACTIVATED)
            pGameObject->resetDoorOrButton();
    }
}

void InstanceScript::closeDoorOrButton(GameObject* pGameObject)
{
    if (!pGameObject)
        return;

    if (pGameObject->getGoType() == GAMEOBJECT_TYPE_DOOR || pGameObject->getGoType() == GAMEOBJECT_TYPE_BUTTON)
    {
        if (pGameObject->getLootState() == GO_ACTIVATED)
            pGameObject->resetDoorOrButton();
    }
}

void InstanceScript::updateEncountersStateForCreature(uint32_t creditEntry, uint8_t /*difficulty*/)
{
    updateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, creditEntry);
}

void InstanceScript::updateEncountersStateForSpell(uint32_t creditEntry, uint8_t /*difficulty*/)
{
    updateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, creditEntry);
}
#endif

void InstanceScript::sendUnitEncounter(uint32_t type, Unit* unit, uint8_t value_a, uint8_t value_b)
{
    WorldMap* instance = getInstance();
    instance->sendPacketToAllPlayers(AscEmu::Packets::SmsgUpdateInstanceEncounterUnit(type, unit ? unit->GetNewGUID() : WoWGuid(), value_a, value_b).serialise().get());
}

void InstanceScript::displayDataStateList(Player* player)
{
    player->broadcastMessage("=== DataState for instance %s ===", mInstance->getBaseMap()->getMapInfo()->name.c_str());

    for (const auto& encounters : bosses)
    {
        CreatureProperties const* creature = sMySQLStore.getCreatureProperties(encounters.entry);
        if (creature != nullptr)
        {
            player->broadcastMessage("  Boss '%s' (%u) - %s", creature->Name.c_str(), encounters.entry, getDataStateString(encounters.state).c_str());
        }
        else
        {
            GameObjectProperties const* gameobject = sMySQLStore.getGameObjectProperties(encounters.entry);
            if (gameobject != nullptr)
                player->broadcastMessage("  Object '%s' (%u) - %s", gameobject->name.c_str(), encounters.entry, getDataStateString(encounters.state).c_str());
            else
                player->broadcastMessage("  MiscData %u - %s", encounters.entry, getDataStateString(encounters.state).c_str());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// timers

uint32_t InstanceScript::addTimer(uint32_t durationInMs)
{
    uint32_t timerId = ++mTimerCount;
    mTimers.push_back(std::make_pair(timerId, durationInMs));

    return timerId;
}

uint32_t InstanceScript::getTimeForTimer(uint32_t timerId)
{
    for (const auto& intTimer : mTimers)
    {
        if (intTimer.first == timerId)
            return intTimer.second;
    }

    return 0;
}

void InstanceScript::removeTimer(uint32_t& timerId)
{
    for (InstanceTimerArray::iterator intTimer = mTimers.begin(); intTimer != mTimers.end(); ++intTimer)
    {
        if (intTimer->first == timerId)
        {
            mTimers.erase(intTimer);
            timerId = 0;
            break;
        }
    }
}

void InstanceScript::resetTimer(uint32_t timerId, uint32_t durationInMs)
{
    for (auto& intTimer : mTimers)
    {
        if (intTimer.first == timerId)
            intTimer.second = durationInMs;
    }
}

bool InstanceScript::isTimerFinished(uint32_t timerId)
{
    for (const auto& intTimer : mTimers)
    {
        if (intTimer.first == timerId)
            return intTimer.second == 0;
    }

    return false;
}

void InstanceScript::cancelAllTimers()
{
    mTimers.clear();
    mTimerCount = 0;
}

void InstanceScript::updateTimers()
{
    for (auto& TimerIter : mTimers)
    {
        if (TimerIter.second > 0)
        {
            int leftTime = TimerIter.second - getUpdateFrequency();
            if (leftTime > 0)
                TimerIter.second -= getUpdateFrequency();
            else
                TimerIter.second = 0;
        }
    }
}

void InstanceScript::displayTimerList(Player* player)
{
    player->broadcastMessage("=== Timers for instance %s ===", mInstance->getBaseMap()->getMapInfo()->name.c_str());

    if (mTimers.empty())
    {
        player->broadcastMessage("  No Timers available!");
    }
    else
    {
        for (const auto& intTimer : mTimers)
            player->broadcastMessage("  TimerId (%u)  %u ms left", intTimer.first, intTimer.second);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// instance update

void InstanceScript::registerUpdateEvent()
{
    sEventMgr.AddEvent(mInstance, &WorldMap::callScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, getUpdateFrequency(), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void InstanceScript::modifyUpdateEvent(uint32_t frequencyInMs)
{
    if (getUpdateFrequency() != frequencyInMs)
    {
        setUpdateFrequency(frequencyInMs);
        sEventMgr.ModifyEventTimeAndTimeLeft(mInstance, EVENT_SCRIPT_UPDATE_EVENT, getUpdateFrequency());
    }
}

void InstanceScript::removeUpdateEvent()
{
    sEventMgr.RemoveEvents(mInstance, EVENT_SCRIPT_UPDATE_EVENT);
}

//////////////////////////////////////////////////////////////////////////////////////////
// misc

void InstanceScript::setCellForcedStates(float xMin, float xMax, float yMin, float yMax, bool forceActive /*true*/)
{
    if (xMin == xMax || yMin == yMax)
        return;

    float Y = yMin;
    while (xMin < xMax)
    {
        while (yMin < yMax)
        {
            MapCell* CurrentCell = mInstance->getCellByCoords(xMin, yMin);
            if (forceActive && CurrentCell == nullptr)
            {
                CurrentCell = mInstance->createByCoords(xMin, yMin);
                if (CurrentCell != nullptr)
                    CurrentCell->init(mInstance->getPosX(xMin), mInstance->getPosY(yMin), mInstance);
            }

            if (CurrentCell != nullptr)
            {
                if (forceActive)
                    mInstance->addForcedCell(CurrentCell);
                else
                    mInstance->removeForcedCell(CurrentCell);
            }

            yMin += 40.0f;
        }

        yMin = Y;
        xMin += 40.0f;
    }
}

Creature* InstanceScript::spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId /* = 0*/)
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(entry);
    if (creatureProperties == nullptr)
    {
        sLogger.failure("tried to create a invalid creature with entry {}!", entry);
        return nullptr;
    }

    Creature* creature = mInstance->getInterface()->spawnCreature(entry, LocationVector(posX, posY, posZ, posO), true, true, 0, 0);
    if (creature == nullptr)
        return nullptr;

    if (factionId != 0)
        creature->setFaction(factionId);
    else
        creature->setFaction(creatureProperties->Faction);

    return creature;
}

Creature* InstanceScript::getCreatureBySpawnId(uint32_t entry)
{
    return mInstance->getSqlIdCreature(entry);
}

Creature* InstanceScript::GetCreatureByGuid(uint32_t guid)
{
    return mInstance->getCreature(guid);
}

CreatureSet InstanceScript::getCreatureSetForEntry(uint32_t entry, bool debug /*= false*/, Player* player /*= nullptr*/)
{
    CreatureSet creatureSet;
    uint32_t countCreatures = 0;
    for (auto creature : mInstance->getCreatures())
    {
        if (creature != nullptr)
        {

            if (creature->getEntry() == entry)
            {
                creatureSet.insert(creature);
                ++countCreatures;
            }
        }
    }

    if (debug == true)
    {
        if (player != nullptr)
            player->broadcastMessage("%u Creatures with entry %u found.", countCreatures, entry);
    }

    return creatureSet;
}

CreatureSet InstanceScript::getCreatureSetForEntries(std::vector<uint32_t> entryVector)
{
    CreatureSet creatureSet;
    for (auto creature : mInstance->getCreatures())
    {
        if (creature != nullptr)
        {
            for (auto entry : entryVector)
            {
                if (creature->getEntry() == entry)
                    creatureSet.insert(creature);
            }
        }
    }

    return creatureSet;
}

Creature* InstanceScript::findNearestCreature(Object* pObject, uint32_t entry, float maxSearchRange /*= 250.0f*/)
{
    Creature* pCreature = mInstance->getInterface()->findNearestCreature(pObject, entry, maxSearchRange);
    return pCreature;
}

GameObject* InstanceScript::spawnGameObject(uint32_t entry, float posX, float posY, float posZ, float posO, bool addToWorld /*= true*/, uint32_t misc1 /*= 0*/, uint32_t phase /*= 0*/)
{
    GameObject* spawnedGameObject = mInstance->getInterface()->spawnGameObject(entry, LocationVector(posX, posY, posZ, posO), addToWorld, misc1, phase);
    return spawnedGameObject;
}

GameObject* InstanceScript::getGameObjectBySpawnId(uint32_t entry)
{
    return mInstance->getSqlIdGameObject(entry);
}

GameObject* InstanceScript::GetGameObjectByGuid(uint32_t guid)
{
    return mInstance->getGameObject(guid);
}

GameObject* InstanceScript::getClosestGameObjectForPosition(uint32_t entry, float posX, float posY, float posZ)
{
    GameObjectSet gameObjectSet = getGameObjectsSetForEntry(entry);

    if (gameObjectSet.size() == 0)
        return nullptr;

    if (gameObjectSet.size() == 1)
        return *(gameObjectSet.begin());

    float distance = 99999;
    float nearestDistance = 99999;

    for (auto gameobject : gameObjectSet)
    {
        if (gameobject != nullptr)
        {
            distance = getRangeToObjectForPosition(gameobject, posX, posY, posZ);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                return gameobject;
            }
        }
    }

    return nullptr;
}

InstanceScript::GameObjectSet InstanceScript::getGameObjectsSetForEntry(uint32_t entry)
{
    GameObjectSet gameobjectSet;
    for (auto gameobject : mInstance->getGameObjects())
    {
        if (gameobject != nullptr)
        {
            if (gameobject->getEntry() == entry)
                gameobjectSet.insert(gameobject);
        }
    }

    return gameobjectSet;
}

float InstanceScript::getRangeToObjectForPosition(Object* object, float posX, float posY, float posZ)
{
    if (object == nullptr)
        return 0.0f;

    LocationVector pos = object->GetPosition();
    float dX = pos.x - posX;
    float dY = pos.y - posY;
    float dZ = pos.z - posZ;

    return sqrtf(dX * dX + dY * dY + dZ * dZ);
}

void InstanceScript::setGameObjectStateForEntry(uint32_t entry, uint8_t state)
{
    if (entry == 0)
        return;

    GameObjectSet gameObjectSet = getGameObjectsSetForEntry(entry);

    if (gameObjectSet.size() == 0)
        return;

    for (auto gameobject : gameObjectSet)
    {
        if (gameobject != nullptr)
            gameobject->setState(state);
    }
}
