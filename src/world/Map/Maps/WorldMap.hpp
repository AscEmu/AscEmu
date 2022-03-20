/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Creatures/Summons/SummonDefines.hpp"
#include "Storage/MySQLStructures.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Cells/CellHandler.hpp"
#include "Management/WorldStatesHandler.h"
#include "DynamicTree.h"
#include "Map/Cells/TerrainMgr.hpp"

template <typename T>
class CellHandler;
class InstanceScript;
class MapScriptInterface;
class Object;
class InstanceMap;
class CreatureGroup;
enum LineOfSightChecks : uint8_t;

struct CorpseInfo
{
    time_t time;
    uint64_t guid;

    CorpseInfo(time_t time, uint64_t guid)
        : time(time), guid(guid)
    {}
};

struct RespawnInfo
{
    SpawnObjectType type;
    uint64_t spawnId;
    uint32_t entry;
    time_t time;

    Object* obj;
    float cellX;
    float cellY;
};

struct CompareRespawnInfo
{
    bool operator()(RespawnInfo* a, RespawnInfo* b)
    {
        if (a == b)
            return false;
        if (a->time != b->time)
            return (a->time > b->time);
        if (a->spawnId != b->spawnId)
            return a->spawnId < b->spawnId;
        ASSERT(a->type != b->type, "Duplicate respawn entry for spawnId (%u,%u) found!", a->type, a->spawnId);
        return a->type < b->type;
    }
};

typedef std::unordered_map<uint32_t, RespawnInfo*> RespawnInfoMap;

inline bool operator==(const RespawnInfo& a, const RespawnInfo& b)
{
    if (a.spawnId != b.spawnId)
        return false;
    if (a.entry != b.entry)
        return false;
    if (a.type != b.type)
        return false;

    return true;
}

class respawnQueue : public std::priority_queue<RespawnInfo*, std::vector<RespawnInfo*>, CompareRespawnInfo>
{
public:
    bool remove(RespawnInfo* value)
    {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end())
        {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        }
        else
        {
            return false;
        }
        return false;
    }

    void clear()
    {
        this->c.clear();
    }
};

template <typename T>
struct CompareTimeAndGuid
{
    bool operator()(T const& p1, T const& p2)
    {
        return p1.time < p2.time && p1.guid < p2.guid;
    }
};

typedef std::unordered_map<uint32_t, Player*> PlayerStorageMap;
typedef std::vector<Creature*> CreaturesStorageMap;
typedef std::unordered_map<uint32_t, Pet*> PetStorageMap;
typedef std::vector<GameObject*> GameObjectStorageMap;
typedef std::unordered_map<uint32_t, DynamicObject*> DynamicObjectStorageMap;
typedef std::set<Transporter*> TransportsContainer;

typedef std::set<Creature*> CreatureSet;
typedef std::set<Object*> ObjectSet;
typedef std::set<Creature*> ActiveCreatureSet;
typedef std::set<GameObject*> ActiveGameObjectSet;

typedef std::set<Object*> UpdateQueue;
typedef std::set<Player*> PUpdateQueue;

typedef std::set<uint64_t> CombatProgressMap;
typedef std::unordered_map<uint32, Creature*> CreatureSqlIdMap;
typedef std::unordered_map<uint32, GameObject*> GameObjectSqlIdMap;

class SERVER_DECL WorldMap : public CellHandler <MapCell>, public EventableObject, public WorldStatesHandler::WorldStatesObserver
{
    friend class MapScriptInterface;
    friend class MapCell;
public:
    WorldMap(BaseMap* baseMap, uint32_t id, time_t, uint32_t InstanceId, uint8_t SpawnMode);
    virtual ~WorldMap();

    virtual void initialize();
    virtual void update(uint32_t);
    virtual void unloadAll();

    void setUnloadPending(bool value) { m_unloadPending = value; }
    bool isUnloadPending() { return m_unloadPending; }

    float getVisibilityRange() const { return m_VisibleDistance; }
    float getUpdateDistance(Object* curObj, Object* obj, Player* plObj);
    virtual void initVisibilityDistance();

    void outOfMapBoundariesTeleport(Object* object);

    std::mutex m_Updatelock;
    std::mutex m_objectinsertlock;
    ObjectSet m_objectinsertpool;
    void AddObject(Object*);
    void PushObject(Object* obj);
    void PushStaticObject(Object* obj);
    void RemoveObject(Object* obj, bool free_guid);

    void addObjectToActiveSet(Object* obj);
    void removeObjectFromActiveSet(Object* obj);

    bool canUnload(uint32_t diff)
    {
        if (!m_unloadTimer)
            return false;

        if (m_unloadTimer <= diff)
            return true;

        m_unloadTimer -= diff;
        return false;
    }

    virtual bool addPlayerToMap(Player*) { return true; }
    virtual void removePlayerFromMap(Player*) {};

    virtual EnterState cannotEnter(Player* /*player*/) { return EnterState::CAN_ENTER; }

    // Storage
    PlayerStorageMap const& getPlayers() const;
    CreaturesStorageMap const& getCreatures() const;
    PetStorageMap const& getPets() const;
    GameObjectStorageMap const& getGameObjects() const;
    DynamicObjectStorageMap const& getDynamicObjects() const;
    TransportsContainer const& getTransports() const;

    std::deque<uint32_t> _reusable_guids_gameobject;
    std::deque<uint32_t> _reusable_guids_creature;

    // Difficulty
    InstanceDifficulty::Difficulties getDifficulty() const { return InstanceDifficulty::Difficulties(getSpawnMode()); }
    bool isRegularDifficulty();
    DBC::Structures::MapDifficulty const* getMapDifficulty();

    // Area and Zone Management
    bool getAreaInfo(uint32_t phaseMask, float x, float y, float z, uint32_t& mogpflags, int32_t& adtId, int32_t& rootId, int32_t& groupId);
    uint32_t getAreaId(uint32_t phaseMask, float x, float y, float z);
    uint32_t getAreaId(uint32_t phaseMask, LocationVector const& pos) { return getAreaId(phaseMask, pos.getPositionX(), pos.getPositionY(), pos.getPositionZ()); }
    uint32_t getZoneId(uint32_t phaseMask, float x, float y, float z);
    uint32_t getZoneId(uint32_t phaseMask, LocationVector const& pos) { return getZoneId(phaseMask, pos.getPositionX(), pos.getPositionY(), pos.getPositionZ()); }
    void getZoneAndAreaId(uint32_t phaseMask, uint32_t& zoneid, uint32_t& areaid, float x, float y, float z);
    void getZoneAndAreaId(uint32_t phaseMask, uint32_t& zoneid, uint32_t& areaid, LocationVector const& pos) { getZoneAndAreaId(phaseMask, zoneid, areaid, pos.getPositionX(), pos.getPositionY(), pos.getPositionZ()); }

    // Water
    ZLiquidStatus getLiquidStatus(uint32_t phaseMask, float x, float y, float z, uint8_t ReqLiquidType, LiquidData* data = nullptr, float collisionHeight = 2.03128f);
    float getWaterLevel(float x, float y);
    bool isInWater(uint32_t phaseMask, float x, float y, float z, LiquidData* data = nullptr);
    bool isUnderWater(uint32_t phaseMask, float x, float y, float z);

    // Line of Sight
    bool isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t phasemask, LineOfSightChecks checks);
    bool getObjectHitPos(uint32_t phasemask, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float modifyDist);

    // Dynamic Map
    DynamicMapTree getDynamicTree() const { return _dynamicTree; }
    void balance() { _dynamicTree.balance(); }
    void removeGameObjectModel(GameObjectModel const& model) { _dynamicTree.remove(model); }
    void insertGameObjectModel(GameObjectModel const& model) { _dynamicTree.insert(model); }
    bool containsGameObjectModel(GameObjectModel const& model) const { return _dynamicTree.contains(model); }
    float getGameObjectFloor(uint32_t phasemask, float x, float y, float z, float maxSearchDist = 50.0f) const
    {
        return _dynamicTree.getHeight(x, y, z, maxSearchDist, phasemask);
    }

    // Terrain
    TerrainHolder* getTerrain() const { return _terrain; }
    float getWaterOrGroundLevel(uint32_t phasemask, float x, float y, float z, float* ground = nullptr, bool swim = false, float collisionHeight = 2.03128f);
    float getHeight(float x, float y, float z, bool checkVMap = true, float maxSearchDist = 50.0f) const;
    float getGridHeight(float x, float y) const;
    float getHeight(LocationVector const& pos, bool vmap = true, float maxSearchDist = 50.0f) const { return getHeight(pos.getPositionX(), pos.getPositionY(), pos.getPositionZ(), vmap, maxSearchDist); }
    // phasemask seems to be invalid when loading into a map                                                                                                                                                // phase
    float getHeight(uint32_t phasemask, float x, float y, float z, bool vmap = true, float maxSearchDist = 50.0f) const { return std::max<float>(getHeight(x, y, z, vmap, maxSearchDist), getGameObjectFloor(1, x, y, z, maxSearchDist)); }
    float getHeight(uint32_t phasemask, LocationVector const& pos, bool vmap = true, float maxSearchDist = 50.0f) const { return getHeight(phasemask, pos.getPositionX(), pos.getPositionY(), pos.getPositionZ(), vmap, maxSearchDist); }

    // Instance
    uint32_t getInstanceId() const { return _instanceId; }
    void setInstanceId(uint32_t instanceId) { _instanceId = instanceId; }
    uint8_t getSpawnMode() const { return (_instanceSpawnMode); }
    void setSpawnMode(uint8_t mode) { _instanceSpawnMode = mode; }
    bool isRaidOrHeroicDungeon();
    bool isHeroic();
    bool is25ManRaid();

    // Player
    Player* getPlayer(uint32_t guid);
    uint32_t getPlayerCount();
    bool hasPlayers();
    virtual void removeAllPlayers();

    // Creatures
    uint32_t m_CreatureHighGuid;
    Creature* createCreature(uint32_t entry);
    Creature* createAndSpawnCreature(uint32_t pEntry, float pX, float pY, float pZ, float pO);

    uint64_t generateCreatureGuid(uint32_t entry, bool canUseOldGuid = true);

    Creature* getCreature(uint32_t guid);
    Creature* getSqlIdCreature(uint32_t sqlid);

    std::unordered_map<uint32_t /*leaderSpawnId*/, CreatureGroup*> CreatureGroupHolder;

    // Pets
    Pet* getPet(uint32_t guid);

    // GameObject
    uint32_t m_GOHighGuid;
    GameObject* createGameObject(uint32_t entry);
    GameObject* createAndSpawnGameObject(uint32_t entryID, float x, float y, float z, float o, float scale);

    uint32_t generateGameobjectGuid() { return ++m_GOHighGuid; }

    GameObject* getGameObject(uint32_t guid);
    GameObject* getSqlIdGameObject(uint32_t sqlid);

    // DynamicObjects
    uint32_t m_DynamicObjectHighGuid;
    DynamicObject* createDynamicObject();
    DynamicObject* getDynamicObject(uint32_t guid);

    // Summons
    Summon* createSummon(uint32_t entry, SummonType type, uint32_t duration);

    // Transports
    bool addToMapMgr(Transporter* obj);
    void removeFromMapMgr(Transporter* obj);

    // Corpse
    void addCorpseDespawn(uint64_t guid, time_t time);
    std::priority_queue<CorpseInfo, std::vector<CorpseInfo>, CompareTimeAndGuid<CorpseInfo>> _corpseDespawnTimes;

    // Lookup Wrappers
    Unit* getUnit(const uint64_t& guid);
    Object* getObject(const uint64_t& guid);

    // Base Template
    BaseMap* getBaseMap() const { return m_baseMap; }

    // Cell Handling
    void addForcedCell(MapCell* c);
    void removeForcedCell(MapCell* c);
    void addForcedCell(MapCell* c, uint32_t range);
    void removeForcedCell(MapCell* c, uint32_t range);

    bool cellHasAreaID(uint32_t x, uint32_t y, uint16_t& AreaID);
    void updateAllCells(bool apply, uint32_t areamask);
    void updateAllCells(bool apply);
    void updateCellActivity(uint32_t x, uint32_t y, uint32_t radius);

    void setCellIdle(uint16_t x, uint16_t y, MapCell* cell);
    void unloadCell(uint32_t x, uint32_t y);
    bool isCellActive(uint32_t x, uint32_t y);

    void updateInRangeSet(Object* obj, Player* plObj, MapCell* cell, ByteBuffer** buf);

    void changeObjectLocation(Object* obj);
    void changeFarsightLocation(Player* plr, DynamicObject* farsight);

    std::set<MapCell*> m_forcedcells;

    // Packts
    void sendChatMessageToCellPlayers(Object* obj, WorldPacket* packet, uint32_t cell_radius, uint32_t langpos, int32_t lang, WorldSession* originator);
    void sendPvPCaptureMessage(int32_t ZoneMask, uint32_t ZoneId, const char* Message, ...);
    void sendPacketToAllPlayers(WorldPacket* packet) const;
    void sendPacketToPlayersInZone(uint32_t zone, WorldPacket* packet) const;

    EventableObjectHolder eventHolder;
    ObjectSet _mapWideStaticObjects;
    ActiveGameObjectSet activeGameObjects;
    ActiveCreatureSet activeCreatures;
    std::set<Corpse*> m_corpses;

    // Respawn Handling
    void loadRespawnTimes();
    void saveRespawnTime(SpawnObjectType type, uint32_t spawnId, uint32_t entry, time_t respawnTime, uint32_t cellX, uint32_t cellY, bool startup = false);
    void saveRespawnDB(RespawnInfo const& info);
    bool addRespawn(RespawnInfo const& info);
    void removeRespawnTime(SpawnObjectType type, uint32_t spawnId);

    void deleteRespawnTimes() { unloadAllRespawnInfos(); deleteRespawnTimesInDB(getBaseMap()->getMapId(), getInstanceId()); }
    static void deleteRespawnTimesInDB(uint16_t mapId, uint32_t instanceId);

    void unloadAllRespawnInfos();

    void deleteRespawn(RespawnInfo* info);
    void deleteRespawnFromDB(SpawnObjectType type, uint32_t spawnId);

    void processRespawns();
    bool checkRespawn(RespawnInfo* info);
    void doRespawn(SpawnObjectType type, Object* obj,uint32_t spawnId, float cellX, float cellY);

    void getRespawnInfo(std::vector<RespawnInfo*>& respawnData, SpawnObjectTypeMask types) const;
    RespawnInfo* getRespawnInfo(SpawnObjectType type, uint32_t spawnId) const;

    respawnQueue _respawnTimes;
    RespawnInfoMap _creatureRespawnTimesBySpawnId;
    RespawnInfoMap _gameObjectRespawnTimesBySpawnId;
    RespawnInfoMap& getRespawnMapForType(SpawnObjectType type)
    {
        switch (type)
        {
            case SPAWN_TYPE_CREATURE:
                return _creatureRespawnTimesBySpawnId;
            case SPAWN_TYPE_GAMEOBJECT:
                return _gameObjectRespawnTimesBySpawnId;
            default:
                break;
        }
    }
    RespawnInfoMap const& getRespawnMapForType(SpawnObjectType type) const
    {
        switch (type)
        {
            case SPAWN_TYPE_CREATURE:
                return _creatureRespawnTimesBySpawnId;
            case SPAWN_TYPE_GAMEOBJECT:
                return _gameObjectRespawnTimesBySpawnId;
            default:
                break;
        }
    }

    void respawnBossLinkedGroups(uint32_t bossId);
    void spawnManualGroup(uint32_t groupId);

    CreatureSqlIdMap _sqlids_creatures;
    GameObjectSqlIdMap _sqlids_gameobjects;

    // Update Timers
    uint32_t _sessionUpdateTimer;
    uint32_t _respawnUpdateTimer;
    uint32_t _dynamicUpdateTimer;
    uint32_t _gameObjectUpdateTimer;

    // Worldstates
    WorldStatesHandler& getWorldStatesHandler();
    void onWorldStateUpdate(uint32_t zone, uint32_t field, uint32_t value) override;

    // Update System
    std::mutex m_updateMutex;
    UpdateQueue _updates;
    PUpdateQueue _processQueue;
    void updateObjects();
    void pushToProcessed(Player* plr);
    // Mark object as updated
    void objectUpdated(Object* obj);

    // Combat Progress
    CombatProgressMap _combatProgress;
    bool isCombatInProgress();
    void addCombatInProgress(uint64_t guid);
    void removeCombatInProgress(uint64_t guid);

    // Script related
    InstanceScript* getScript();
    void loadInstanceScript();
    void callScriptUpdate();

    MapScriptInterface* getInterface();
    InstanceMap* getInstance() { return pInstance; }

private:
    WorldStatesHandler worldstateshandler;
    MapScriptInterface* ScriptInterface;
    bool m_unloadPending = false;

    TerrainHolder* _terrain;
    uint32_t _instanceId;
    uint8_t _instanceSpawnMode = InstanceDifficulty::Difficulties::DUNGEON_NORMAL;

    // Storage
    PlayerStorageMap m_PlayerStorage;
    CreaturesStorageMap m_CreatureStorage;
    PetStorageMap m_PetStorage;
    GameObjectStorageMap m_GameObjectStorage;
    DynamicObjectStorageMap m_DynamicObjectStorage;
    TransportsContainer m_TransportStorage;
    std::mutex m_transportsLock;

    // Sessions
    std::set<WorldSession*> Sessions;

protected:
    InstanceScript* mInstanceScript;
    DynamicMapTree _dynamicTree;
    uint32_t m_unloadTimer;
    float m_VisibleDistance;
    BaseMap* m_baseMap;
    InstanceMap* pInstance;
};