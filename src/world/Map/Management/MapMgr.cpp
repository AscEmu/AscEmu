/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WoWGuid.h"
#include "MapMgr.hpp"
#include "Objects/DynamicObject.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Objects/Units/Unit.hpp"
#include "VMapFactory.h"
#include "MMapFactory.h"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Map/Maps/InstanceMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStructures.hpp"

MapMgr::MapMgr() = default;
MapMgr::~MapMgr() = default;

MapMgr& MapMgr::getInstance()
{
    static MapMgr mInstance;
    return mInstance;
}

void MapMgr::initialize()
{
    // Generate Instances based on WorldMapInfo Table
    const auto mapInfoStore = sMySQLStore.getWorldMapInfoStore();
    for (auto mapInfo = mapInfoStore->begin(); mapInfo != mapInfoStore->end(); ++mapInfo)
    {
        if (mapInfo->second.mapid >= MAX_NUM_MAPS)
        {
            sLogger.failure("InstanceMgr : One or more of your worldmap_info rows specifies an invalid map: {}", mapInfo->second.mapid);
            continue;
        }

        createBaseMap(mapInfo->second.mapid);
    }
}

void MapMgr::initializeInstanceIds()
{
    // Initialize Instance Id Pool
    instanceIdPool.fill(1, 10000);
}

void MapMgr::shutdown()
{
    std::scoped_lock<std::mutex> lock(m_mapsLock);

    // Continents
    for (auto map = m_WorldMaps.cbegin(); map != m_WorldMaps.cend();)
    {
        map->second->unloadAll(true);
        map = m_WorldMaps.erase(map);
    }

    // Instances
    for (auto ini = m_InstancedMaps.cbegin(); ini != m_InstancedMaps.cend();)
    {
        ini->second->unloadAll(true);
        ini = m_InstancedMaps.erase(ini);
    }

    m_pendingRemoveMaps.clear();
}

void MapMgr::removeInstance(uint32_t instanceId)
{
    // get the Lock so we cant Update and Delete at the same time :)
    std::scoped_lock<std::mutex> lock(m_mapsLock);

    auto ini = m_InstancedMaps.find(instanceId);
    if (ini != m_InstancedMaps.end())
    {
        if (ini->second->isUnloadPending())
        {
            auto&& mapHolder = std::move(m_InstancedMaps.extract(ini));
            mapHolder.mapped()->shutdownMapThread();
            // Wait for thread to finish its work before freeing memory
            m_pendingRemoveMaps.emplace_back(std::move(mapHolder.mapped()));
        }
    }
}

void MapMgr::addMapToRemovePool(WorldMap const* map)
{
    std::scoped_lock<std::mutex> lock(m_mapsLock);
    auto itr = m_WorldMaps.find(map->getBaseMap()->getMapId());
    if (itr != m_WorldMaps.cend())
    {
        auto&& mapHolder = std::move(m_WorldMaps.extract(itr));
        mapHolder.mapped()->shutdownMapThread();
        // Wait for thread to finish its work before freeing memory
        m_pendingRemoveMaps.emplace_back(std::move(mapHolder.mapped()));
    }
}

void MapMgr::update()
{
    std::scoped_lock<std::mutex> lock(m_mapsLock);
    for (auto itr = m_pendingRemoveMaps.cbegin(); itr != m_pendingRemoveMaps.cend();)
    {
        if ((*itr)->isMapReadyForDelete())
            itr = m_pendingRemoveMaps.erase(itr);
        else
            ++itr;
    }
}

void MapMgr::createBaseMap(uint32_t mapId)
{
    BaseMap* map = findBaseMap(mapId);

    if (map == nullptr)
    {
        std::scoped_lock<std::mutex> lock(m_mapsLock);

        // Only Create Valid Maps
        const auto mapEntry = sMapStore.lookupEntry(mapId);
        if (mapEntry == nullptr)
            return;

        const auto mapInfo = sMySQLStore.getWorldMapInfo(mapId);
        if (mapInfo == nullptr)
            return;

        m_BaseMaps.insert_or_assign(mapId, std::make_unique<BaseMap>(mapId, mapInfo, mapEntry));

        if (!mapEntry->instanceable())
        {
            m_WorldMaps.insert_or_assign(mapId, createWorldMap(mapId, worldConfig.server.mapUnloadTime * 1000));
        }
    }
}

BaseMap* MapMgr::findBaseMap(uint32_t mapId) const
{
    const auto& iter = m_BaseMaps.find(mapId);
    return (iter == m_BaseMaps.end() ? nullptr : iter->second.get());
}

std::unique_ptr<WorldMap> MapMgr::createWorldMap(uint32_t mapId, uint32_t unloadTime) const
{
    const auto& baseMap = findBaseMap(mapId);
    if (baseMap == nullptr)
        return nullptr;

    sLogger.debug("MapMgr::createWorldMap Create Continent {} for Map {}", baseMap->getMapName(), mapId);

    auto map = std::make_unique<WorldMap>(baseMap, mapId, unloadTime, 0, InstanceDifficulty::Difficulties::DUNGEON_NORMAL);

    // Load Saved Respawns when existing
    map->loadRespawnTimes();

    // Initialize Map Script and Load Static Spawns
    map->initialize();

    // Scheduling the new map for running
    map->startMapThread();

    return map;
}

WorldMap* MapMgr::findWorldMap(uint32_t mapid) const
{
    const auto& iter = m_WorldMaps.find(mapid);
    return (iter == m_WorldMaps.end() ? nullptr : iter->second.get());
}

InstanceMap* MapMgr::findInstanceMap(uint32_t instanceId) const
{
    const auto& iter = m_InstancedMaps.find(instanceId);
    return (iter == m_InstancedMaps.end() ? nullptr : static_cast<InstanceMap*>(iter->second.get()));
}

std::list<InstanceMap*> MapMgr::findInstancedMaps(uint32_t mapId)
{
    std::list<InstanceMap*> list;

    for (auto const& maps : m_InstancedMaps)
    {
        if (maps.second->getBaseMap()->getMapId() == mapId && maps.second->getBaseMap()->isDungeon())
            list.push_back(static_cast<InstanceMap*>(maps.second.get()));
    }

    return list;
}

WorldMap* MapMgr::findWorldMap(uint32_t mapId, uint32_t instanceId) const
{
    WorldMap* map = nullptr;
    BaseMap* baseMap = findBaseMap(mapId);
    if (!baseMap)
        return nullptr;

    if (baseMap->instanceable())
    {
        map = findInstanceMap(instanceId);
    }
    else if (instanceId == 0)
    {
        map = findWorldMap(mapId);
    }
    return map;
}

WorldMap* MapMgr::createInstanceForPlayer(uint32_t mapId, Player* player, uint32_t loginInstanceId /*= 0*/)
{
    const auto& baseMap = findBaseMap(mapId);
    if (baseMap == nullptr)
        return nullptr;

    WorldMap* map = nullptr;
    uint32_t newInstanceId = 0;

    if (baseMap->isBattlegroundOrArena())
    {
        return nullptr;
    }
    else
    {
        InstancePlayerBind* pBind = player->getBoundInstance(baseMap->getMapId(), player->getDifficulty(baseMap->isRaid()));
        InstanceSaved* pSave = pBind ? pBind->save : nullptr;

        if (!pBind || !pBind->perm)
        {
            // if the player has a saved instance id on login,
            // we use this instance
            // or port him to the entrance
            if (loginInstanceId)
            {
                map = findInstanceMap(loginInstanceId);
                if (!map && pSave && pSave->getInstanceId() == loginInstanceId)
                    map = createInstance(mapId, loginInstanceId, pSave, pSave->getDifficulty(), player->getTeam());
                return map;
            }

            if (const auto group = player->getGroup())
            {
                if (const InstanceGroupBind* groupBind = group->getBoundInstance(baseMap))
                {
                    // solo instance saves should be reset when entering a group's instance
                    player->unbindInstance(baseMap->getMapId(), player->getDifficulty(baseMap->isRaid()));
                    pSave = groupBind->save;
                }
            }
        }

        if (pSave)
        {
            // solo/permanent/group saves
            newInstanceId = pSave->getInstanceId();
            map = findInstanceMap(newInstanceId);
            if (!map)
                map = createInstance(mapId, newInstanceId, pSave, pSave->getDifficulty(), player->getTeam());
        }
        else
        {
            // when we land here the Instance gets Created for the first Time
            newInstanceId = instanceIdPool.generateId();
            InstanceDifficulty::Difficulties diff = player->getGroup() ? player->getGroup()->getDifficulty(baseMap->isRaid()) : player->getDifficulty(baseMap->isRaid());
            map = findInstanceMap(newInstanceId);
            if (!map)
                map = createInstance(mapId, newInstanceId, nullptr, diff, player->getTeam());
        }
    }

    if (map)
        sLogger.debug("MapMgr::createInstanceForPlayer Create Instance {} for Map {}", baseMap->getMapName(), mapId);

    return map;
}

InstanceMap* MapMgr::createInstance(uint32_t mapId, uint32_t InstanceId, InstanceSaved* save, InstanceDifficulty::Difficulties difficulty, PlayerTeam InstanceTeam)
{
    // load/create a map
    std::scoped_lock<std::mutex> lock(m_mapsLock);

    // make sure we have a valid BaseMap
    const auto& baseMap = findBaseMap(mapId);
    if (!baseMap)
    {
        sLogger.failure("MapMgr::createInstance: no BaseMap for map {}", mapId);
        return nullptr;
    }

    // make sure we have a valid map id
    WDB::Structures::MapEntry const* entry = sMapStore.lookupEntry(mapId);
    if (!entry)
    {
        sLogger.failure("MapMgr::createInstance: no entry for map {}", mapId);
        return nullptr;
    }

    const auto mapInfo = sMySQLStore.getWorldMapInfo(mapId);
    if (mapInfo == nullptr)
    {
        sLogger.failure("MapMgr::createInstance: no WorldMapInfo for map {}", mapId);
        return nullptr;
    }

    // some instances only have one difficulty
#if VERSION_STRING > TBC
    getDownscaledMapDifficultyData(mapId, difficulty);
#endif
    sLogger.debug("MapMgr::createInstance Create {} map instance {} for {} created with difficulty {}", save ? "" : "new ", InstanceId, mapId, difficulty ? "heroic" : "normal");

    auto map = std::make_unique<InstanceMap>(baseMap, mapId, worldConfig.server.mapUnloadTime * 1000, InstanceId, difficulty, InstanceTeam);

    // Load Saved Respawns when existing
    map->loadRespawnTimes();

    // Initialize Map Script and Load Static Spawns
    map->initialize();

    // Load Saved Data when possible
    bool load_data = save != nullptr;
    map->createInstanceData(load_data);
    
    // In Instances we load all Cells
    map->updateAllCells(true);

    // Save pointer to InstanceMap to avoid casting later -Appled
    auto* instMap = map.get();
    // Add current Instance to our Active Instances
    const auto [_, emplaced] = m_InstancedMaps.try_emplace(InstanceId, std::move(map));

    if (!emplaced)
        return nullptr;

    // Scheduling the new map for running
    instMap->startMapThread();

    return instMap;
}

BattlegroundMap* MapMgr::createBattleground(uint32_t mapId)
{
    std::scoped_lock<std::mutex> lock(m_mapsLock);

    uint32_t newInstanceId = instanceIdPool.generateId();
    if (!newInstanceId)
        return nullptr;

    // make sure we have a valid BaseMap
    const auto& baseMap = findBaseMap(mapId);
    if (!baseMap)
    {
        sLogger.failure("MapMgr::createInstance: no BaseMap for map {}", mapId);
        return nullptr;
    }

    uint8_t spawnMode = InstanceDifficulty::Difficulties::DUNGEON_NORMAL;

    auto map = std::make_unique<BattlegroundMap>(baseMap, mapId, worldConfig.server.mapUnloadTime * 1000, newInstanceId, spawnMode);

    // Initialize Map Script and Load Static Spawns
    map->initialize();

    // In Battlegrounds we load all Cells
    map->updateAllCells(true);

    // Save pointer to BattlegroundMap to avoid casting later -Appled
    auto* bgMap = map.get();
    const auto [_, emplaced] = m_InstancedMaps.try_emplace(newInstanceId, std::move(map));

    if (!emplaced)
        return nullptr;

    // Scheduling the new map for running
    bgMap->startMapThread();

    return bgMap;
}

WorldMap* MapMgr::createMap(uint32_t mapId, Player* player, uint32_t instanceId)
{
    WorldMap* map = nullptr;
    BaseMap* baseMap = findBaseMap(mapId);

    if (baseMap)
    {
        if (baseMap->instanceable())
        {
            // instance check.
            map = createInstanceForPlayer(mapId, player, instanceId);
        }
        else
        {
            // main continent check.
            map = findWorldMap(mapId);
        }
    }
    return map;
}

EnterState MapMgr::canPlayerEnter(uint32_t mapid, uint32_t minLevel, Player* player, bool loginCheck)
{
    WDB::Structures::MapEntry const* entry = sMapStore.lookupEntry(mapid);
    if (!entry)
        return CANNOT_ENTER_NO_ENTRY;

    if (!entry->isDungeon())
        return CAN_ENTER;

    MySQLStructure::MapInfo const* mapInfo = sMySQLStore.getWorldMapInfo(mapid);
    if (!mapInfo && mapInfo->isNonInstanceMap())
        return CANNOT_ENTER_UNINSTANCED_DUNGEON;

    InstanceDifficulty::Difficulties targetDifficulty, requestedDifficulty;
    targetDifficulty = requestedDifficulty = player->getDifficulty(entry->isRaid());

#if VERSION_STRING > TBC
    // Get the highest available difficulty if current setting is higher than the instance allows
    WDB::Structures::MapDifficulty const* mapDiff = getDownscaledMapDifficultyData(entry->id, targetDifficulty);
    if (!mapDiff)
        return CANNOT_ENTER_DIFFICULTY_UNAVAILABLE;
#endif

    //Bypass checks for GMs
    if (player->isGMFlagSet())
        return CAN_ENTER;

    //Other requirements
    if (!mapInfo || !mapInfo->hasFlag(WMI_INSTANCE_ENABLED))
        return CANNOT_ENTER_UNSPECIFIED_REASON;

    if (mapInfo->hasFlag(WMI_INSTANCE_XPACK_01) && !player->getSession()->HasFlag(ACCOUNT_FLAG_XPACK_01) && !player->getSession()->HasFlag(ACCOUNT_FLAG_XPACK_02))
        return CANNOT_ENTER_XPACK01;

    if (mapInfo->hasFlag(WMI_INSTANCE_XPACK_02) && !player->getSession()->HasFlag(ACCOUNT_FLAG_XPACK_02))
        return CANNOT_ENTER_XPACK02;

    if (minLevel && player->getLevel() < minLevel)
        return CANNOT_ENTER_MIN_LEVEL;

    if (mapInfo->required_quest_A && (player->getTeam() == TEAM_ALLIANCE) && !player->hasQuestFinished(mapInfo->required_quest_A))
        return CANNOT_ENTER_ATTUNE_QA;

    if (mapInfo->required_quest_H && (player->getTeam() == TEAM_HORDE) && !player->hasQuestFinished(mapInfo->required_quest_H))
        return CANNOT_ENTER_ATTUNE_QH;

    if (mapInfo->required_item && !player->getItemInterface()->GetItemCount(mapInfo->required_item, true))
        return CANNOT_ENTER_ATTUNE_ITEM;

    if (player->getDungeonDifficulty() >= InstanceDifficulty::DUNGEON_HEROIC &&
        mapInfo->isMultimodeDungeon()
        && ((mapInfo->heroic_key_1 > 0 && !player->getItemInterface()->GetItemCount(mapInfo->heroic_key_1, false))
            && (mapInfo->heroic_key_2 > 0 && !player->getItemInterface()->GetItemCount(mapInfo->heroic_key_2, false))
            )
        )
        return CANNOT_ENTER_KEY;

    if (!mapInfo->isNonInstanceMap() && player->getDungeonDifficulty() >= InstanceDifficulty::DUNGEON_HEROIC && player->getLevel() < mapInfo->minlevel_heroic)
        return CANNOT_ENTER_MIN_LEVEL_HC;

    const auto group = player->getGroup();
    if (entry->isRaid()) // can only enter in a raid group
        if ((!group || !group->isRaidGroup()) && !player->m_cheats.hasTriggerpassCheat)
            return CANNOT_ENTER_NOT_IN_RAID;

    if (!player->isAlive())
    {
        // only let us enter when its the instance our corpse is in
        uint32_t corpseInstance = player->getCorpseInstanceId();

        const auto instance = sMapMgr.findWorldMap(mapid, corpseInstance);
        if (instance == nullptr || instance->getBaseMap()->getMapId() != mapid)
            return CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE;
    }

    //Get instance where player's group is bound & its map
    if (!loginCheck && group)
    {
        InstanceGroupBind* boundInstance = group->getBoundInstance(entry);
        if (boundInstance && boundInstance->save)
            if (WorldMap* boundMap = sMapMgr.findWorldMap(mapid, boundInstance->save->getInstanceId()))
                if (EnterState denyReason = boundMap->cannotEnter(player))
                    return denyReason;
    }

    // players are only allowed to enter 5 instances per hour
    if (entry->isDungeon() && (!player->getGroup() || (player->getGroup() && !player->getGroup()->isLFGGroup())))
    {
        uint32_t instanceIdToCheck = 0;
        if (InstanceSaved* save = player->getInstanceSave(mapid, entry->isRaid()))
            instanceIdToCheck = save->getInstanceId();

        if (!player->checkInstanceCount(instanceIdToCheck) && !player->isDead())
            return CANNOT_ENTER_TOO_MANY_INSTANCES;
    }

    return CAN_ENTER;
}
