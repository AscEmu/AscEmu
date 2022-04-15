/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "TLSObject.h"
#include "Objects/DynamicObject.h"
#include "Map/Cells/CellHandler.hpp"
#include "CThreads.h"
#include "Management/WorldStatesHandler.h"
#include "Objects/Item.h"
#include "Map/Area/AreaStorage.hpp"
#include "CrashHandler.h"
#include "Objects/Units/Creatures/Summons/Summon.h"
#include "Objects/Units/Unit.h"
#include "VMapFactory.h"
#include "MMapFactory.h"
#include "Storage/MySQLDataStore.hpp"
#include "Macros/ScriptMacros.hpp"
#include "MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Server/Packets/SmsgUpdateWorldState.h"
#include "Server/Packets/SmsgDefenseMessage.h"
#include "Server/Script/ScriptMgr.h"

#include "shared/WoWGuid.h"

using namespace AscEmu::Packets;

MapMgr& MapMgr::getInstance()
{
    static MapMgr mInstance;
    return mInstance;
}

void MapMgr::initialize()
{
    // Spawn Threads based on current hardware
    // may return 0 when not able to detect
    auto processor_count = std::thread::hardware_concurrency();
    updater.activate(processor_count);

    // Generate Instances based on WorldMapInfo Table
    const auto mapInfoStore = sMySQLStore.getWorldMapInfoStore();
    for (auto mapInfo = mapInfoStore->begin(); mapInfo != mapInfoStore->end(); ++mapInfo)
    {
        if (mapInfo->second.mapid >= MAX_NUM_MAPS)
        {
            sLogger.failure("InstanceMgr : One or more of your worldmap_info rows specifies an invalid map: %u", mapInfo->second.mapid);
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
    // Continents
    WorldMapContainer::iterator map = m_WorldMaps.begin();
    for (; map != m_WorldMaps.end(); ++map)
    {
        map->second->unloadAll();
        delete map->second;
        m_WorldMaps.erase(map);
    }

    // Instances
    InstancedMapContainer::iterator ini = m_InstancedMaps.begin();
    for (; ini != m_InstancedMaps.end(); ++ini)
    {
        ini->second->unloadAll();
        delete ini->second;
        m_InstancedMaps.erase(ini);
    }

    // Close all Threads
    updater.shutdown();
}

void MapMgr::update(uint32_t /*diff*/)
{
    uint32_t mstime = Util::getMSTime();
    uint32_t difftime = mstime - lastMapMgrUpdate;

    std::lock_guard<std::mutex> lock(m_mapsLock);

    // Only add new Jobs when the Old Ones are done
    if (updater.getQueueSize())
        return;

    // Continents
    WorldMapContainer::iterator map = m_WorldMaps.begin();
    for (; map != m_WorldMaps.end(); ++map)
        updater.addJob(*map->second, difftime);

    // Instances
    InstancedMapContainer::iterator ini = m_InstancedMaps.begin();
    for (; ini != m_InstancedMaps.end(); ++ini)
    {
        if (!ini->second->isUnloadPending())
        {
            updater.addJob(*ini->second, difftime);
        }
    }

    lastMapMgrUpdate = mstime;
}

void MapMgr::removeInstance(uint32_t instanceId)
{
    // get the Lock so we cant Update and Delete at the same time :)
    std::lock_guard<std::mutex> lock(m_mapsLock);

    auto ini = m_InstancedMaps.find(instanceId);
    if (ini != m_InstancedMaps.end())
    {
        if (ini->second->isUnloadPending())
        {
            delete ini->second;
            m_InstancedMaps.erase(ini);
        }
    }
}

void MapMgr::createBaseMap(uint32_t mapId)
{
    BaseMap* map = findBaseMap(mapId);

    if (map == nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mapsLock);

        // Only Create Valid Maps
        const auto mapEntry = sMapStore.LookupEntry(mapId);
        if (mapEntry == nullptr)
            return;

        const auto mapInfo = sMySQLStore.getWorldMapInfo(mapId);
        if (mapInfo == nullptr)
            return;

        m_BaseMaps[mapId] = new BaseMap(mapId, mapInfo, mapEntry);

        if (!mapEntry->instanceable())
        {
            m_WorldMaps[mapId] = createWorldMap(mapId, 300000);
        }
    }
}

BaseMap* MapMgr::findBaseMap(uint32_t mapId) const
{
    const auto& iter = m_BaseMaps.find(mapId);
    return (iter == m_BaseMaps.end() ? nullptr : iter->second);
}

WorldMap* MapMgr::createWorldMap(uint32_t mapId, uint32_t unloadTime)
{
    const auto& baseMap = findBaseMap(mapId);
    if (baseMap == nullptr)
        return nullptr;

    sLogger.debug("MapMgr::createWorldMap Create Continent %s for Map %u", baseMap->getMapName().c_str(), mapId);

    WorldMap* map = new WorldMap(baseMap, mapId, time_t(unloadTime), 0, InstanceDifficulty::Difficulties::DUNGEON_NORMAL);

    map->initialize();

    // Load Saved Respawns when existing
    map->loadRespawnTimes();

    return map;
}

WorldMap* MapMgr::findWorldMap(uint32_t mapid) const
{
    const auto& iter = m_WorldMaps.find(mapid);
    return (iter == m_WorldMaps.end() ? nullptr : iter->second);
}

InstanceMap* MapMgr::findInstanceMap(uint32_t instanceId) const
{
    const auto& iter = m_InstancedMaps.find(instanceId);
    return (iter == m_InstancedMaps.end() ? nullptr : reinterpret_cast<InstanceMap*>(iter->second));
}

std::list<InstanceMap*> MapMgr::findInstancedMaps(uint32_t mapId)
{
    std::list<InstanceMap*> list;

    for (auto const& maps : m_InstancedMaps)
    {
        if (maps.second->getBaseMap()->getMapId() == mapId && maps.second->getBaseMap()->isDungeon())
            list.push_back(reinterpret_cast<InstanceMap*>(maps.second));
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

            InstanceGroupBind* groupBind = nullptr;
            Group* group = player->getGroup();
            if (group)
            {
                groupBind = group->getBoundInstance(baseMap);
                if (groupBind)
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
        sLogger.debug("MapMgr::createInstanceForPlayer Create Instance %s for Map %u", baseMap->getMapName().c_str(), mapId);

    return map;
}

InstanceMap* MapMgr::createInstance(uint32_t mapId, uint32_t InstanceId, InstanceSaved* save, InstanceDifficulty::Difficulties difficulty, PlayerTeam InstanceTeam)
{
    // load/create a map
    std::lock_guard<std::mutex> lock(m_mapsLock);

    // make sure we have a valid BaseMap
    const auto& baseMap = findBaseMap(mapId);
    if (!baseMap)
    {
        sLogger.failure("MapMgr::createInstance: no BaseMap for map %u", mapId);
        return nullptr;
    }

    // make sure we have a valid map id
    DBC::Structures::MapEntry const* entry = sMapStore.LookupEntry(mapId);
    if (!entry)
    {
        sLogger.failure("MapMgr::createInstance: no entry for map %u", mapId);
        return nullptr;
    }

    const auto mapInfo = sMySQLStore.getWorldMapInfo(mapId);
    if (mapInfo == nullptr)
    {
        sLogger.failure("MapMgr::createInstance: no WorldMapInfo for map %u", mapId);
        return nullptr;
    }

    // some instances only have one difficulty
    getDownscaledMapDifficultyData(mapId, difficulty);

    sLogger.debug("MapMgr::createInstance Create %s map instance %d for %u created with difficulty %s", save ? "" : "new ", InstanceId, mapId, difficulty ? "heroic" : "normal");

    InstanceMap* map = new InstanceMap(baseMap, mapId, time_t(300000), InstanceId, difficulty, InstanceTeam);

    // Load Saved Respawns when existing
    map->loadRespawnTimes();

    map->initialize();

    // Load Saved Data when possible
    bool load_data = save != nullptr;
    map->createInstanceData(load_data);
    
    // In Instances we load all Cells
    map->updateAllCells(true);

    // Add current Instance to our Active Instances
    m_InstancedMaps[InstanceId] = map;
    return map;
}

BattlegroundMap* MapMgr::createBattleground(uint32_t mapId, uint32_t InstanceId)
{
    std::lock_guard<std::mutex> lock(m_mapsLock);

    uint32_t newInstanceId = instanceIdPool.generateId();
    if (!newInstanceId)
        return nullptr;

    // make sure we have a valid BaseMap
    const auto& baseMap = findBaseMap(mapId);
    if (!baseMap)
    {
        sLogger.failure("MapMgr::createInstance: no BaseMap for map %u", mapId);
        return nullptr;
    }

    uint8_t spawnMode = InstanceDifficulty::Difficulties::DUNGEON_NORMAL;

    BattlegroundMap* map = new BattlegroundMap(baseMap, mapId, time_t(300000), InstanceId, spawnMode);

    m_InstancedMaps[InstanceId] = map;
    return map;
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
    DBC::Structures::MapEntry const* entry = sMapStore.LookupEntry(mapid);
    if (!entry)
        return CANNOT_ENTER_NO_ENTRY;

    if (!entry->isDungeon())
        return CAN_ENTER;

    MySQLStructure::MapInfo const* mapInfo = sMySQLStore.getWorldMapInfo(mapid);
    if (!mapInfo && mapInfo->isNonInstanceMap())
        return CANNOT_ENTER_UNINSTANCED_DUNGEON;

    InstanceDifficulty::Difficulties targetDifficulty, requestedDifficulty;
    targetDifficulty = requestedDifficulty = player->getDifficulty(entry->isRaid());

    // Get the highest available difficulty if current setting is higher than the instance allows
    DBC::Structures::MapDifficulty const* mapDiff = getDownscaledMapDifficultyData(entry->id, targetDifficulty);
    if (!mapDiff)
        return CANNOT_ENTER_DIFFICULTY_UNAVAILABLE;

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

#if VERSION_STRING <= WotLK
    char const* mapName = entry->map_name[0];
#else
    char const* mapName = entry->map_name;
#endif

    Group* group = player->getGroup();
    if (entry->isRaid()) // can only enter in a raid group
        if ((!group || !group->isRaidGroup()) && !player->m_cheats.hasTriggerpassCheat)
            return CANNOT_ENTER_NOT_IN_RAID;

    if (!player->isAlive())
    {
        // only let us enter when its the instance our corpse is in
        uint32_t corpseInstance = player->getCorpseInstanceId();

        const auto instance = sMapMgr.findWorldMap(mapid, corpseInstance);
        if (instance->getBaseMap()->getMapId() != mapid)
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
