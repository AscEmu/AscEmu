/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "InstanceNumberGen.hpp"
#include "Utilities/Util.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include <mutex>
#include <unordered_map>

enum PlayerTeam : uint8_t;

namespace InstanceDifficulty
{
    enum Difficulties : uint8_t;
}

class InstanceSaved;
class BattlegroundMap;
class InstanceMap;
class BaseMap;
class WorldMap;
class Player;

class SERVER_DECL MapMgr
{
private:
    MapMgr();
    ~MapMgr();

public:
    MapMgr(MapMgr&&) = delete;
    MapMgr(MapMgr const&) = delete;
    MapMgr& operator=(MapMgr&&) = delete;
    MapMgr& operator=(MapMgr const&) = delete;

    static MapMgr& getInstance();

    // Start for each row in worldmap_info an Instance
    void initialize();
    void initializeInstanceIds();
    EnterState canPlayerEnter(uint32_t mapid, uint32_t minLevel, Player* player, bool loginCheck = false);
    void shutdown();
    void removeInstance(uint32_t instanceId);
    void addMapToRemovePool(WorldMap const* map);
    void update();

    // BaseMaps
    void createBaseMap(uint32_t mapId);
    BaseMap* findBaseMap(uint32_t mapId) const;

    // WorldMap
    std::unique_ptr<WorldMap> createWorldMap(uint32_t mapId, uint32_t unloadTime) const;
    WorldMap* findWorldMap(uint32_t mapId) const;

    // InstanceMap
    WorldMap* createInstanceForPlayer(uint32_t mapId, Player* player, uint32_t loginInstanceId = 0);
    InstanceMap* findInstanceMap(uint32_t instanceId) const;
    std::list<InstanceMap*> findInstancedMaps(uint32_t mapId);
    InstanceMap* createInstance(uint32_t mapId, uint32_t InstanceId, InstanceSaved* save, InstanceDifficulty::Difficulties difficulty, PlayerTeam InstanceTeam);
    BattlegroundMap* createBattleground(uint32_t mapId);
    WorldMap* createMap(uint32_t mapId, Player* player, uint32_t instanceId = 0);

    // Multi
    WorldMap* findWorldMap(uint32_t mapId, uint32_t instanceId) const;

    // InstanceId Generation
    UniqueNumberPool instanceIdPool;

private:
    typedef std::unordered_map<uint32_t, std::unique_ptr<BaseMap>> BaseMapContainer;
    typedef std::unordered_map<uint32_t, std::unique_ptr<WorldMap>> WorldMapContainer;
    typedef std::unordered_map<uint32_t, std::unique_ptr<WorldMap>> InstancedMapContainer;
    typedef std::list<std::unique_ptr<WorldMap>> MapRemovePool;

    uint32_t lastMapMgrUpdate = Util::getMSTime();

    std::mutex m_mapsLock;

    BaseMapContainer m_BaseMaps;
    WorldMapContainer m_WorldMaps;
    InstancedMapContainer m_InstancedMaps;
    MapRemovePool m_pendingRemoveMaps;
};

#define sMapMgr MapMgr::getInstance()
