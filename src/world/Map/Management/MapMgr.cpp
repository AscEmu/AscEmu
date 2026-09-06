/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WoWGuid.hpp"
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
#include "Management/Battleground/Battleground.hpp"
#include "Map/Maps/InstanceMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/DatabaseDefinition.hpp"
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

    if (const auto ini = m_InstancedMaps.find(instanceId); ini != m_InstancedMaps.end())
    {
        if (ini->second->isUnloadPending())
        {
            auto node = m_InstancedMaps.extract(ini);
            auto mapPtr = std::move(node.mapped());

            mapPtr->shutdownMapThread();

            // Wait for thread to finish its work before freeing memory
            m_pendingRemoveMaps.emplace_back(std::move(mapPtr));
        }
    }
}

void MapMgr::addMapToRemovePool(WorldMap const* map)
{
    if (map == nullptr || map->getBaseMap() == nullptr)
    {
        return;
    }

    std::scoped_lock<std::mutex> lock(m_mapsLock);

    if (const auto itr = m_WorldMaps.find(map->getBaseMap()->getMapId()); itr != m_WorldMaps.cend())
    {
        auto node = m_WorldMaps.extract(itr);
        auto mapPtr = std::move(node.mapped());

        mapPtr->shutdownMapThread();

        // Wait for thread to finish its work before freeing memory
        m_pendingRemoveMaps.emplace_back(std::move(mapPtr));
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

        if (!mapEntry->isInstanceableMap())
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
    map->getSpawnManager().loadRespawnTimes();

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
    if (iter == m_InstancedMaps.end())
        return nullptr;

    WorldMap* map = iter->second.get();
    if (!map || !map->getBaseMap()->isInstanceMap())
        return nullptr;

    return static_cast<InstanceMap*>(map);
}

std::list<InstanceMap*> MapMgr::findInstancedMaps(uint32_t mapId)
{
    std::list<InstanceMap*> list;

    for (auto const& maps : m_InstancedMaps)
    {
        if (maps.second->getBaseMap()->getMapId() == mapId && maps.second->getBaseMap()->isInstanceMap())
            list.push_back(static_cast<InstanceMap*>(maps.second.get()));
    }

    return list;
}

WorldMap* MapMgr::findWorldMap(uint32_t mapId, uint32_t instanceId) const
{
    BaseMap* baseMap = findBaseMap(mapId);
    if (!baseMap)
        return nullptr;

    if (baseMap->isInstanceableMap())
    {
        const auto iter = m_InstancedMaps.find(instanceId);
        if (iter == m_InstancedMaps.end())
            return nullptr;

        WorldMap* map = iter->second.get();
        if (!map || map->getBaseMap()->getMapId() != mapId)
            return nullptr;

        return map;
    }

    if (instanceId == 0)
        return findWorldMap(mapId);

    return nullptr;
}

bool MapMgr::isBattlegroundLoginValid(uint32_t mapId, uint32_t instanceId, PlayerTeam team)
{
    std::scoped_lock<std::mutex> lock(m_mapsLock);

    const auto iter = m_InstancedMaps.find(instanceId);
    if (iter == m_InstancedMaps.end())
        return false;

    WorldMap* map = iter->second.get();
    if (!map || map->getBaseMap()->getMapId() != mapId || !map->getBaseMap()->isBattlegroundOrArena())
        return false;

    auto* battlegroundMap = static_cast<BattlegroundMap*>(map);
    Battleground* battleground = battlegroundMap->getBattleground();
    if (!battleground || battleground->hasEnded())
        return false;

    return battleground->hasFreeSlots(team, battleground->getType());
}

WorldMap* MapMgr::createInstanceForPlayer(uint32_t mapId, Player* player, uint32_t requestedInstanceId /*= 0*/)
{
    const auto& baseMap = findBaseMap(mapId);
    if (!baseMap || baseMap->isBattlegroundOrArena())
        return nullptr;

    const auto difficulty = player->getGroup()
        ? player->getGroup()->getDifficulty(baseMap->isRaid())
        : player->getDifficulty(baseMap->isRaid());

    InstancePlayerBind* playerBind = player->getBoundInstance(mapId, difficulty);
    InstanceSaved* playerSave = playerBind ? playerBind->save : nullptr;

    InstanceSaved* groupSave = nullptr;
    if (Group* group = player->getGroup())
    {
        if (InstanceGroupBind* groupBind = group->getBoundInstance(baseMap))
            groupSave = groupBind->save;
    }

    // A permanent character bind and a group bind may never point at different
    // runs of the same instance.
    if (playerBind && playerBind->perm && playerSave && groupSave &&
        playerSave->getInstanceId() != groupSave->getInstanceId())
    {
        return nullptr;
    }

    InstanceSaved* selectedSave = nullptr;

    if (playerBind && playerBind->perm && playerSave)
        selectedSave = playerSave;
    else if (groupSave)
        selectedSave = groupSave;
    else if (playerSave)
        selectedSave = playerSave;

    // An explicit runtime instance is a constraint, not an override. Current
    // player/group bindings remain authoritative.
    if (requestedInstanceId && selectedSave &&
        selectedSave->getInstanceId() != requestedInstanceId)
    {
        return nullptr;
    }

    if (selectedSave)
    {
        const uint32_t selectedInstanceId = selectedSave->getInstanceId();

        if (WorldMap* map = findInstanceMap(selectedInstanceId))
            return map->getBaseMap()->getMapId() == mapId ? map : nullptr;

        if (groupSave && playerBind && !playerBind->perm && playerSave &&
            playerSave->getInstanceId() != groupSave->getInstanceId())
        {
            player->unbindInstance(mapId, difficulty);
        }

        return createInstance(
            mapId,
            selectedInstanceId,
            selectedSave,
            selectedSave->getDifficulty(),
            player->getTeam());
    }

    if (requestedInstanceId)
    {
        // No current binding exists. Restore the exact stored run only while its
        // persisted instance record is still valid.
        if (WorldMap* map = findInstanceMap(requestedInstanceId))
            return map->getBaseMap()->getMapId() == mapId ? map : nullptr;

        InstanceSaved* requestedSave = sInstanceMgr.getInstanceSave(requestedInstanceId);

        if (!requestedSave)
        {
            if (auto result = CharacterDatabase.query(
                "SELECT difficulty, resettime FROM instance WHERE id = %u AND map = %u AND (resettime = 0 OR resettime > UNIX_TIMESTAMP())",
                requestedInstanceId, mapId))
            {
                Field* fields = result->fetch();
                const auto savedDifficulty = InstanceDifficulty::Difficulties(fields[0].asUint8());
                const time_t resetTime = time_t(fields[1].asUint64());

                requestedSave = sInstanceMgr.addInstanceSave(
                    mapId,
                    requestedInstanceId,
                    savedDifficulty,
                    resetTime,
                    true,
                    true);
            }
        }

        if (!requestedSave || requestedSave->getMapId() != mapId)
            return nullptr;

        return createInstance(
            mapId,
            requestedInstanceId,
            requestedSave,
            requestedSave->getDifficulty(),
            player->getTeam());
    }

    const uint32_t newInstanceId = instanceIdPool.generateId();
    return createInstance(mapId, newInstanceId, nullptr, difficulty, player->getTeam());
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
    map->getSpawnManager().loadRespawnTimes();

    // Initialize Map Script and Load Static Spawns
    map->initialize();

    // Load Saved Data when possible
    bool load_data = save != nullptr;
    map->createInstanceData(load_data);
    
    // Instance scripts may address objects anywhere on the map, so keep every
    // terrain-backed grid loaded. Client visibility remains distance based.
    map->setAllGridsForcedActive(true);

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

    // Battleground logic may address objects anywhere on the map, so keep every
    // terrain-backed grid loaded. Client visibility remains distance based.
    map->setAllGridsForcedActive(true);

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
        if (baseMap->isBattlegroundOrArena())
        {
            // Battleground and arena maps are created by their own manager.
            // A transfer must resolve the exact existing runtime instance and
            // must never try to create a normal dungeon instance for it.
            map = instanceId ? findWorldMap(mapId, instanceId) : nullptr;
        }
        else if (baseMap->isInstanceableMap())
        {
            map = createInstanceForPlayer(mapId, player, instanceId);
        }
        else
        {
            map = findWorldMap(mapId);
        }
    }
    return map;
}

EnterState MapMgr::canPlayerEnter(uint32_t mapid, uint32_t minLevel, Player* player, bool loginCheck, uint32_t requestedInstanceId)
{
    WDB::Structures::MapEntry const* entry = sMapStore.lookupEntry(mapid);
    if (!entry)
        return CANNOT_ENTER_NO_ENTRY;

    if (!entry->isInstanceMap())
        return CAN_ENTER;

    if (entry->isBattlegroundOrArena())
    {
        if (!requestedInstanceId)
            return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;

        WorldMap* battlegroundMap = findWorldMap(mapid, requestedInstanceId);
        if (!battlegroundMap || !battlegroundMap->getBaseMap()->isBattlegroundOrArena())
            return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;

        return CAN_ENTER;
    }

    MySQLStructure::MapInfo const* mapInfo = sMySQLStore.getWorldMapInfo(mapid);
    if (!mapInfo)
        return CANNOT_ENTER_UNINSTANCED_DUNGEON;

    const auto group = player->getGroup();
    InstanceDifficulty::Difficulties targetDifficulty = group
        ? group->getDifficulty(entry->isRaid())
        : player->getDifficulty(entry->isRaid());

#if VERSION_STRING > TBC
    WDB::Structures::MapDifficulty const* mapDiff = getDownscaledMapDifficultyData(entry->id, targetDifficulty);
    if (!mapDiff)
        return CANNOT_ENTER_DIFFICULTY_UNAVAILABLE;
#endif

    if (player->isGMFlagSet())
        return CAN_ENTER;

    if (!mapInfo->hasFlag(WMI_INSTANCE_ENABLED))
        return CANNOT_ENTER_UNSPECIFIED_REASON;

    if (mapInfo->hasFlag(WMI_INSTANCE_XPACK_01) &&
        !player->getSession()->HasFlag(ACCOUNT_FLAG_XPACK_01) &&
        !player->getSession()->HasFlag(ACCOUNT_FLAG_XPACK_02))
    {
        return CANNOT_ENTER_XPACK01;
    }

    if (mapInfo->hasFlag(WMI_INSTANCE_XPACK_02) &&
        !player->getSession()->HasFlag(ACCOUNT_FLAG_XPACK_02))
    {
        return CANNOT_ENTER_XPACK02;
    }

    if (minLevel && player->getLevel() < minLevel)
        return CANNOT_ENTER_MIN_LEVEL;

    if (mapInfo->required_quest_A && player->getTeam() == TEAM_ALLIANCE &&
        !player->hasQuestFinished(mapInfo->required_quest_A))
    {
        return CANNOT_ENTER_ATTUNE_QA;
    }

    if (mapInfo->required_quest_H && player->getTeam() == TEAM_HORDE &&
        !player->hasQuestFinished(mapInfo->required_quest_H))
    {
        return CANNOT_ENTER_ATTUNE_QH;
    }

    if (mapInfo->required_item &&
        !player->getItemInterface()->GetItemCount(mapInfo->required_item, true))
    {
        return CANNOT_ENTER_ATTUNE_ITEM;
    }

    if (player->getDungeonDifficulty() >= InstanceDifficulty::DUNGEON_HEROIC &&
        mapInfo->isMultimodeDungeon() &&
        ((mapInfo->heroic_key_1 > 0 && !player->getItemInterface()->GetItemCount(mapInfo->heroic_key_1, false)) &&
         (mapInfo->heroic_key_2 > 0 && !player->getItemInterface()->GetItemCount(mapInfo->heroic_key_2, false))))
    {
        return CANNOT_ENTER_KEY;
    }

    if (!entry->isWorldMap() &&
        player->getDungeonDifficulty() >= InstanceDifficulty::DUNGEON_HEROIC &&
        player->getLevel() < mapInfo->minlevel_heroic)
    {
        return CANNOT_ENTER_MIN_LEVEL_HC;
    }

    if (entry->isRaid() &&
        (!group || !group->isRaidGroup()) &&
        !player->m_cheats.hasTriggerpassCheat)
    {
        return CANNOT_ENTER_NOT_IN_RAID;
    }

    if (!player->isAlive())
    {
        if (!player->hasCorpseData() || player->getCorpseMapId() != mapid)
            return CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE;
    }

    InstancePlayerBind* playerBind = player->getBoundInstance(mapid, targetDifficulty);
    InstanceSaved* playerSave = playerBind ? playerBind->save : nullptr;

    InstanceSaved* groupSave = nullptr;
    if (group)
    {
        if (InstanceGroupBind* groupBind = group->getBoundInstance(targetDifficulty, mapid))
            groupSave = groupBind->save;
    }

    if (playerBind && playerBind->perm && playerSave && groupSave &&
        playerSave->getInstanceId() != groupSave->getInstanceId())
    {
        return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;
    }

    InstanceSaved* selectedSave = nullptr;
    if (playerBind && playerBind->perm && playerSave)
        selectedSave = playerSave;
    else if (groupSave)
        selectedSave = groupSave;
    else if (playerSave)
        selectedSave = playerSave;

    if (requestedInstanceId && selectedSave &&
        selectedSave->getInstanceId() != requestedInstanceId)
    {
        return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;
    }

    if (!loginCheck)
    {
        uint32_t targetInstanceId = requestedInstanceId;
        if (!targetInstanceId && selectedSave)
            targetInstanceId = selectedSave->getInstanceId();

        if (targetInstanceId)
        {
            if (WorldMap* targetMap = findWorldMap(mapid, targetInstanceId))
            {
                if (EnterState denyReason = targetMap->cannotEnter(player))
                    return denyReason;
            }
        }
    }

    if (!group || !group->isLFGGroup())
    {
        const uint32_t instanceIdToCheck = selectedSave ? selectedSave->getInstanceId() : requestedInstanceId;
        if (!player->checkInstanceCount(instanceIdToCheck) && !player->isDead())
            return CANNOT_ENTER_TOO_MANY_INSTANCES;
    }

    return CAN_ENTER;
}
