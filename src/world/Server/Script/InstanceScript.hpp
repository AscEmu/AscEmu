/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "AEVersion.hpp"
#include "ScriptEvent.hpp"
#include "ScriptMgrDefines.hpp"
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Map/Maps/InstanceDefines.hpp"

enum AchievementCriteriaTypes : uint8_t;
enum EncounterCreditType : uint8_t;
enum EncounterStates : uint8_t;
struct ObjectData;
class Object;
class InstanceMap;
class Transporter;
class Unit;
class Creature;
class SpellInfo;
class Spell;
class Player;
class GameObject;
class WorldMap;

class scriptEventMap;

class SERVER_DECL InstanceScript
{
public:
    InstanceScript(WorldMap* pMapMgr);
    virtual ~InstanceScript() {}

    // Procedures that had been here before
    virtual GameObject* GetObjectForOpenLock(Player* /*pCaster*/, Spell* /*pSpell*/, SpellInfo const* /*pSpellEntry*/) { return NULL; }
    virtual void SetLockOptions(uint32_t /*pEntryId*/, GameObject* /*pGameObject*/) {}
    virtual uint32_t GetRespawnTimeForCreature(uint32_t /*pEntryId*/, Creature* /*pCreature*/) { return 240000; }

    // Player
    virtual void OnPlayerDeath(Player* /*pVictim*/, Unit* /*pKiller*/) {}

    // Spawn Groups
    virtual void OnSpawnGroupKilled(uint32_t /*groupId*/) {}

    // Area and AreaTrigger
    virtual void OnPlayerEnter(Player* /*pPlayer*/) {}
    virtual void OnAreaTrigger(Player* /*pPlayer*/, uint32_t /*pAreaId*/) {}
    virtual void OnZoneChange(Player* /*pPlayer*/, uint32_t /*pNewZone*/, uint32_t /*pOldZone*/) {}

    // Creature / GameObject - part of it is simple reimplementation for easier use Creature / GO < --- > Script
    virtual void OnCreatureDeath(Creature* /*pVictim*/, Unit* /*pKiller*/) {}
    virtual void OnCreaturePushToWorld(Creature* /*pCreature*/) {}
    virtual void OnGameObjectActivate(GameObject* /*pGameObject*/, Player* /*pPlayer*/) {}
    virtual void OnGameObjectPushToWorld(GameObject* /*pGameObject*/) {}

    // Standard virtual methods
    virtual void OnLoad() {}
    virtual void UpdateEvent() {}

    virtual void OnEncounterStateChange(uint32_t /*entry*/, uint32_t /*state*/) {}
    virtual void TransportBoarded(Unit* /*punit*/, Transporter* /*transport*/) {}
    virtual void TransportUnboarded(Unit* /*punit*/, Transporter* /*transport*/) {}

    virtual void Destroy() {}

    // Something to return Instance's MapMgr
    WorldMap* getWorldMap();
    InstanceMap* getInstance();
    uint8_t GetDifficulty();

    //////////////////////////////////////////////////////////////////////////////////////////
    // data
    void addObject(Object* obj);
    void removeObject(Object* obj);

    uint32_t getGuidFromData(uint32_t type);
    Creature* getCreatureFromData(uint32_t type);
    GameObject* getGameObjectFromData(uint32_t type);

    // not saved to database, only for scripting
    virtual void setupInstanceData(ObjectData const* creatureData, ObjectData const* gameObjectData);
    virtual void setLocalData(uint32_t /*type*/, uint32_t /*data*/) {}
    virtual void setLocalData64(uint32_t /*type*/, uint64_t /*data*/) {}
    virtual uint32_t getLocalData(uint32_t /*type*/) const { return 0; }
    virtual Creature* getLocalCreatureData(uint32_t /*type*/) const { return nullptr; }
    virtual uint64_t getLocalData64(uint32_t /*type*/) const { return 0; }
    virtual void DoAction(int32_t /*action*/) {}
    virtual void TransporterEvents(Transporter* /*transport*/, uint32_t /*eventId*/) {}
    uint8_t Difficulty;

#if VERSION_STRING > TBC
    // Update Achievement Criteria for all players in instance
    void updateAchievementCriteria(AchievementCriteriaTypes type, uint32_t miscValue1 = 0, uint32_t miscValue2 = 0, Unit* unit = nullptr);
#endif

    void setZoneMusic(uint32_t zoneId, uint32_t musicId);

    //////////////////////////////////////////////////////////////////////////////////////////
    // encounters

    // called for all initialized instancescripts!
    void generateBossDataState();
    void loadSavedInstanceData(char const* data);
    void sendUnitEncounter(uint32_t type, Unit* unit = nullptr, uint8_t value_a = 0, uint8_t value_b = 0);

    virtual bool setBossState(uint32_t id, EncounterStates state);
    std::vector<BossInfo> getBosses();
    EncounterStates getBossState(uint32_t id) const;
    //used for debug
    std::string getDataStateString(uint8_t state);

    uint32_t getEncounterCount() const;

    void saveToDB();
    void updateEncounterState(EncounterCreditType type, uint32_t creditEntry);

    void useDoorOrButton(GameObject* pGameObject, uint32_t withRestoreTime = 0, bool useAlternativeState = false);
    void closeDoorOrButton(GameObject* pGameObject);

    // Checks encounter state
    void updateEncountersStateForCreature(uint32_t creditEntry, uint8_t difficulty);
    void updateEncountersStateForSpell(uint32_t creditEntry, uint8_t difficulty);

    // Used only during loading
    void setCompletedEncountersMask(uint32_t newMask) { completedEncounters = newMask; }

    // Returns completed encounters mask for packets
    uint32_t getCompletedEncounterMask() const { return completedEncounters; }

    void readSaveDataBossStates(std::istringstream& data);
    virtual void readSaveDataExtended(std::istringstream& /*data*/) {}
    void writeSaveDataBossStates(std::ostringstream& data);
    virtual void writeSaveDataExtended(std::ostringstream& /*data*/) {}
    virtual std::string getSaveData();

    //used for debug
    void displayDataStateList(Player* player);

    void setBossNumber(uint32_t number) { bosses.resize(number); }

    //////////////////////////////////////////////////////////////////////////////////////////
    // timers

private:
    typedef std::pair<uint32_t, uint32_t> InstanceTimerPair;
    typedef std::vector<InstanceTimerPair> InstanceTimerArray;

    InstanceTimerArray mTimers;
    uint32_t mTimerCount = 0;

    typedef std::map<uint32_t, uint32_t> ObjectInfoMap;
    typedef std::map<uint32_t, uint32_t> ObjectGuidMap;

    // FaST Acess Instance Data
    static void setupObjectData(ObjectData const* creatureData, ObjectInfoMap& objectInfo);
    ObjectInfoMap _creatureInfo;
    ObjectInfoMap _gameObjectInfo;
    ObjectGuidMap _objectGuids;

public:
    uint32_t addTimer(uint32_t durationInMs);
    uint32_t getTimeForTimer(uint32_t timerId);
    uint32_t completedEncounters = 0; // completed encounter mask, bit indexes are DungeonEncounter.dbc boss numbers, used for packets
    void removeTimer(uint32_t& timerId);
    void resetTimer(uint32_t timerId, uint32_t durationInMs);
    bool isTimerFinished(uint32_t timerId);
    void cancelAllTimers();

    //only for internal use!
    void updateTimers();

    //used for debug
    void displayTimerList(Player* player);

    //////////////////////////////////////////////////////////////////////////////////////////
    // instance update

private:
    uint32_t mUpdateFrequency = defaultUpdateFrequency;

public:
    uint32_t getUpdateFrequency() { return mUpdateFrequency; }
    void setUpdateFrequency(uint32_t frequencyInMs) { mUpdateFrequency = frequencyInMs; }

    void registerUpdateEvent();
    void modifyUpdateEvent(uint32_t frequencyInMs);
    void removeUpdateEvent();

    //////////////////////////////////////////////////////////////////////////////////////////
    // script events
protected:
    scriptEventMap scriptEvents;

    //////////////////////////////////////////////////////////////////////////////////////////
    // misc

public:
    typedef std::set<Creature*> CreatureSet;
    typedef std::set<GameObject*> GameObjectSet;

    void setCellForcedStates(float xMin, float xMax, float yMin, float yMax, bool forceActive = true);

    Creature* spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0);
    Creature* getCreatureBySpawnId(uint32_t entry);
    Creature* GetCreatureByGuid(uint32_t guid);
    Creature* findNearestCreature(Object* pObject, uint32_t entry, float maxSearchRange /*= 250.0f*/);

    CreatureSet getCreatureSetForEntry(uint32_t entry, bool debug = false, Player* player = nullptr);
    CreatureSet getCreatureSetForEntries(std::vector<uint32_t> entryVector);

    GameObject* spawnGameObject(uint32_t entry, float posX, float posY, float posZ, float posO, bool addToWorld = true, uint32_t misc1 = 0, uint32_t phase = 0);
    GameObject* getGameObjectBySpawnId(uint32_t entry);
    GameObject* GetGameObjectByGuid(uint32_t guid);

    GameObject* getClosestGameObjectForPosition(uint32_t entry, float posX, float posY, float posZ);

    GameObjectSet getGameObjectsSetForEntry(uint32_t entry);

    float getRangeToObjectForPosition(Object* object, float posX, float posY, float posZ);

    void setGameObjectStateForEntry(uint32_t entry, uint8_t state);

private:
    bool mSpawnsCreated = false;

public:
    bool spawnsCreated() { return mSpawnsCreated; }
    void setSpawnsCreated(bool created = true) { mSpawnsCreated = created; }

protected:
    std::vector<BossInfo> bosses;

    WorldMap* mInstance;
};
