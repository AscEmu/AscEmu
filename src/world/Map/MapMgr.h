/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/MapManagementGlobals.hpp"
#include "MapCell.h"
#include "CellHandler.h"
#include "Management/WorldStatesHandler.h"
#include "MapMgrDefines.hpp"
#include "MapDefines.h"
#include "CThreads.h"
#include "Units/Summons/SummonDefines.hpp"
#include "Objects/CObjectFactory.h"
#include "Server/EventableObject.h"

namespace Arcemu
{
    namespace Utility
    {
        template<typename T>
        class TLSObject;
    }
}

template <typename T>
class CellHandler;
class Map;
class Object;
class MapScriptInterface;
class WorldSession;
class GameObject;
class Creature;
class Player;
class Pet;
class Transporter;
class Corpse;
class CBattleground;
class Instance;
class InstanceScript;
class Summon;
class DynamicObject;
class Unit;

extern Arcemu::Utility::TLSObject<MapMgr*> t_currentMapContext;

typedef std::set<Object*> ObjectSet;
typedef std::set<Object*> UpdateQueue;
typedef std::set<Player*> PUpdateQueue;
typedef std::set<Player*> PlayerSet;
typedef std::set<uint64_t> CombatProgressMap;
typedef std::set<Creature*> CreatureSet;
typedef std::set<GameObject*> GameObjectSet;

typedef std::unordered_map<uint32_t, Object*> StorageMap;
typedef std::unordered_map<uint32_t, Creature*> CreatureSqlIdMap;
typedef std::unordered_map<uint32_t, GameObject*> GameObjectSqlIdMap;

class SERVER_DECL MapMgr : public CellHandler <MapCell>, public EventableObject, public CThread, public WorldStatesHandler::WorldStatesObserver
{
    friend class MapCell;
    friend class MapScriptInterface;

public:

    CObjectFactory ObjectFactory;

    uint32_t GetAreaFlag(float x, float y, float z, bool *is_outdoors = nullptr) const;

    // This will be done in regular way soon
    std::set<MapCell*> m_forcedcells;

    void AddForcedCell(MapCell* c);
    void RemoveForcedCell(MapCell* c);

    Mutex m_objectinsertlock;
    ObjectSet m_objectinsertpool;
    void AddObject(Object*);

    // Local (mapmgr) storage/generation of GameObjects
    uint32_t m_GOHighGuid;
    std::vector<GameObject*> GOStorage;
    GameObject* CreateGameObject(uint32_t entry);
    GameObject* CreateAndSpawnGameObject(uint32_t entryID, float x, float y, float z, float o, float scale);

    uint32_t GenerateGameobjectGuid() { return ++m_GOHighGuid; }

    GameObject* GetGameObject(uint32_t guid);

    // Local (mapmgr) storage/generation of Creatures
    uint32_t m_CreatureHighGuid;
    std::vector<Creature*> CreatureStorage;
    CreatureSet::iterator creature_iterator;        /// required by owners despawning creatures and deleting *(++itr)
    uint64_t GenerateCreatureGUID(uint32_t entry);
    Creature* CreateCreature(uint32_t entry);
    Creature* CreateAndSpawnCreature(uint32_t pEntry, float pX, float pY, float pZ, float pO);

    Creature* GetCreature(uint32_t guid);


    //////////////////////////////////////////////////////////////////////////////////////////
    /// Summon* CreateSummon(uint32_t entry, SummonType type)
    /// Summon factory function, creates and returns the appropriate summon subclass.
    ///
    /// \param uint32_t entry     -  entry of the summon (NPC id)
    /// \param SummonType type  -  Type of the summon
    ///
    /// \return pointer to a summon
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    Summon* CreateSummon(uint32_t entry, SummonType type);


    // Local (mapmgr) storage/generation of DynamicObjects
    uint32_t m_DynamicObjectHighGuid;
    typedef std::unordered_map<uint32_t, DynamicObject*> DynamicObjectStorageMap;
    DynamicObjectStorageMap m_DynamicObjectStorage;
    DynamicObject* CreateDynamicObject();

    DynamicObject* GetDynamicObject(uint32_t guid);

    // Local (mapmgr) storage of pets
    typedef std::unordered_map<uint32_t, Pet*> PetStorageMap;
    PetStorageMap m_PetStorage;
    PetStorageMap::iterator pet_iterator;
    Pet* GetPet(uint32_t guid);


    // Local (mapmgr) storage of players for faster lookup
    // double typedef lolz// a compile breaker..
    typedef std::unordered_map<uint32_t, Player*> PlayerStorageMap;
    PlayerStorageMap m_PlayerStorage;
    Player* GetPlayer(uint32_t guid);

    // Local (mapmgr) storage of combats in progress
    CombatProgressMap _combatProgress;
    void AddCombatInProgress(uint64_t guid);

    void RemoveCombatInProgress(uint64_t guid);

    // Lookup Wrappers
    Unit* GetUnit(const uint64_t & guid);
    Object* _GetObject(const uint64_t & guid);

    bool runThread() override;
    bool Do();

    MapMgr(Map* map, uint32_t mapid, uint32_t instanceid);
    ~MapMgr();

    void PushObject(Object* obj);
    void PushStaticObject(Object* obj);
    void RemoveObject(Object* obj, bool free_guid);
    void ChangeObjectLocation(Object* obj); // update inrange lists
    void ChangeFarsightLocation(Player* plr, DynamicObject* farsight);

    /// Mark object as updated
    void ObjectUpdated(Object* obj);
    void UpdateCellActivity(uint32_t x, uint32_t y, uint32_t radius);

    // Terrain Functions
    float GetLandHeight(float x, float y, float z);

    float GetADTLandHeight(float x, float y);

    bool IsUnderground(float x, float y, float z);

    bool GetLiquidInfo(float x, float y, float z, float& liquidlevel, uint32_t& liquidtype);

    float GetLiquidHeight(float x, float y);

    uint8_t GetLiquidType(float x, float y);

    const ::DBC::Structures::AreaTableEntry* GetArea(float x, float y, float z);

    bool isInLineOfSight(float x, float y, float z, float x2, float y2, float z2);

    uint32_t GetMapId();

    void PushToProcessed(Player* plr);

    bool HasPlayers();

    bool IsCombatInProgress();
    void TeleportPlayers();

    uint32_t GetInstanceID();

    MySQLStructure::MapInfo const* GetMapInfo();

    bool _shutdown;

    MapScriptInterface* GetInterface();

    virtual int32_t event_GetInstanceID() override;

    uint32_t GetPlayerCount();

    void _PerformObjectDuties();
    uint32_t mLoopCounter;
    uint32_t lastGameobjectUpdate;
    uint32_t lastUnitUpdate;
    void EventCorpseDespawn(uint64_t guid);

    time_t InactiveMoveTime;
    uint8_t iInstanceMode;

    void UnloadCell(uint32_t x, uint32_t y);
    void EventRespawnCreature(Creature* c, uint16_t x, uint16_t y);
    void EventRespawnGameObject(GameObject* o, uint16_t x, uint16_t y);
    void SendChatMessageToCellPlayers(Object* obj, WorldPacket* packet, uint32_t cell_radius, uint32_t langpos, int32_t lang, WorldSession* originator);
    void SendPvPCaptureMessage(int32_t ZoneMask, uint32_t ZoneId, const char* Message, ...);
    void SendPacketToAllPlayers(WorldPacket* packet) const;
    void SendPacketToPlayersInZone(uint32_t zone, WorldPacket* packet) const;

    Instance* pInstance;
    void BeginInstanceExpireCountdown();

    /// better hope to clear any references to us when calling this :P
    void InstanceShutdown();

    /// kill the worker thread only
    void KillThread();

    float GetFirstZWithCPZ(float x, float y, float z);

    //////////////////////////////////////////////////////////////////////////////////////////
    ///Finds and returns the nearest GameObject with this type from Object's inrange set.
    /// \param    Object* o - Pointer to the Object that's inrange set we are searching
    /// \param    uint32_t type - Type of the GameObject we want to find
    /// \return a pointer to the GameObject if found, NULL if there isn't such a GameObject.
    //////////////////////////////////////////////////////////////////////////////////////////
    GameObject* FindNearestGoWithType(Object* o, uint32_t type);

protected:

    /// Collect and send updates to clients
    void _UpdateObjects();

private:

    /// Objects that exist on map
    uint32_t _mapId;
    std::set<Object*> _mapWideStaticObjects;

    bool _CellActive(uint32_t x, uint32_t y);
    void UpdateInRangeSet(Object* obj, Player* plObj, MapCell* cell, ByteBuffer** buf);

    //Zyres: Refactoring 05/04/2016
    float GetUpdateDistance(Object* curObj, Object* obj, Player* plObj);
    void OutOfMapBoundariesTeleport(Object* object);

public:

    /// Distance a Player can "see" other objects and receive updates from them (!! ALREADY dist*dist !!)
    float m_UpdateDistance;

private:

    // Update System
    Mutex m_updateMutex;
    UpdateQueue _updates;
    PUpdateQueue _processQueue;

    // Sessions
    std::set<WorldSession*> Sessions;

    // Map Information
    MySQLStructure::MapInfo const* pMapInfo;
    uint32_t m_instanceID;

    MapScriptInterface* ScriptInterface;

    TerrainHolder* _terrain;

public:

#ifdef WIN32
    DWORD threadid;
#endif
    GameObjectSet activeGameObjects;
    CreatureSet activeCreatures;
    EventableObjectHolder eventHolder;
    CBattleground* m_battleground;
    std::set<Corpse*> m_corpses;
    CreatureSqlIdMap _sqlids_creatures;
    GameObjectSqlIdMap _sqlids_gameobjects;

    // Script related
    InstanceScript* GetScript();
    void LoadInstanceScript();
    void CallScriptUpdate();

    Creature* GetSqlIdCreature(uint32_t sqlid);
    GameObject* GetSqlIdGameObject(uint32_t sqlid);
    std::deque<uint32_t> _reusable_guids_gameobject;
    std::deque<uint32_t> _reusable_guids_creature;

    bool forced_expire;
    bool thread_kill_only;
    bool thread_running;

    WorldStatesHandler& GetWorldStatesHandler();

    void onWorldStateUpdate(uint32_t zone, uint32_t field, uint32_t value) override;

protected:

    InstanceScript* mInstanceScript;

private:

    WorldStatesHandler worldstateshandler;
};
