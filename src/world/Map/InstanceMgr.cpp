/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include <cstdint>
#include "WorldCreator.h"
#include "Storage/MySQLDataStore.hpp"
#include "MapMgr.h"

void InstanceMgr::generateInstances()
{
    const auto mapInfoStore = sMySQLStore.getWorldMapInfoStore();
    for (auto mapInfo = mapInfoStore->begin(); mapInfo != mapInfoStore->end(); ++mapInfo)
    {
        if (mapInfo->second.mapid >= MAX_NUM_MAPS)
        {
            LogError("InstanceMgr : One or more of your worldmap_info rows specifies an invalid map: %u", mapInfo->second.mapid);
            continue;
        }

        if (m_maps[mapInfo->second.mapid] == nullptr)
        {
            _CreateMap(mapInfo->second.mapid);
        }
    }
}

void InstanceMgr::loadInstanceResetTimes()
{
    QueryResult* result = CharacterDatabase.Query("SELECT setting_id, setting_value FROM server_settings WHERE setting_id LIKE 'next_instance_reset_%%'");
    if (result)
    {
        do
        {
            const char* id = result->Fetch()[0].GetString();
            const uint32_t value = result->Fetch()[1].GetUInt32();
            if (strlen(id) <= 20)
                continue;

            uint32_t mapId = atoi(id + 20);
            if (mapId >= MAX_NUM_MAPS)
                continue;

            m_nextInstanceReset[mapId] = value;
        }
        while (result->NextRow());
        delete result;
    }
}

void InstanceMgr::deleteExpiredAndInvalidInstances()
{
    LogDetail("InstanceMgr : Deleting Expired Instances...");

    CharacterDatabase.Execute("DELETE FROM `instances` WHERE expiration > 0 AND expiration <= %u", UNIXTIME);

    CharacterDatabase.Execute("DELETE FROM `instances` WHERE mapid >= %u", MAX_NUM_MAPS);

    QueryResult* mapIdResult = CharacterDatabase.Query("SELECT mapid FROM `instances`");
    if (mapIdResult)
    {
        do
        {
            const uint32_t mapId = mapIdResult->Fetch()[0].GetUInt32();

            const auto mapInfo = sMySQLStore.getWorldMapInfo(mapId);
            if (mapInfo == nullptr)
                CharacterDatabase.Execute("DELETE FROM `instances` WHERE mapid = %u", mapId);

        } while (mapIdResult->NextRow());

        delete mapIdResult;
    }

    //                                                     0         1           2
    QueryResult* result = CharacterDatabase.Query("SELECT id, creator_group, persistent FROM `instances`");
    if (result)
    {
        do
        {
            const uint32_t instanceId = result->Fetch()[0].GetUInt32();
            const uint32_t creatorGroup = result->Fetch()[1].GetUInt32();
            const bool isPersisten = result->Fetch()[2].GetBool();

            if (!isPersisten && creatorGroup && sObjectMgr.GetGroupById(creatorGroup) == nullptr)
                CharacterDatabase.Execute("DELETE FROM `instances` WHERE `id` = %u", instanceId);

        } while (result->NextRow());

        delete result;
    }

    CharacterDatabase.Execute("DELETE FROM `instanceids` WHERE instanceid NOT IN (SELECT id FROM `instances`)");
}

void InstanceMgr::loadAndApplySavedInstanceValues()
{
    LogDetail("InstanceMgr : Loading saved instances...");

    uint32_t count = 0;

    //                                                     0    1        2         3               4              5            6              7            8
    QueryResult* result = CharacterDatabase.Query("SELECT id, mapid, creation, expiration, killed_npc_guids, difficulty, creator_group, creator_guid, persistent FROM `instances`");
    if (result)
    {
        do
        {
            const uint32_t mapId = result->Fetch()[1].GetUInt32();
            const auto mapInfo = sMySQLStore.getWorldMapInfo(mapId);

            auto instance = new Instance();

            instance->m_mapInfo = mapInfo;
            instance->m_instanceId = result->Fetch()[0].GetUInt32();
            instance->m_mapId = mapId;
            instance->m_creation = result->Fetch()[2].GetUInt32();
            instance->m_expiration = result->Fetch()[3].GetUInt32();

            const std::string npcString = result->Fetch()[4].GetString();
            auto npcStrings = Util::SplitStringBySeperator(npcString, " ");
            for (const auto npcString : npcStrings)
            {
                if (uint32_t val = atol(npcString.c_str()))
                    instance->m_killedNpcs.insert(val);
            }

            instance->m_difficulty = result->Fetch()[5].GetUInt8();
            instance->m_creatorGroup = result->Fetch()[6].GetUInt32();
            instance->m_creatorGuid = result->Fetch()[7].GetUInt32();
            instance->m_persistent = result->Fetch()[8].GetBool();
            if (instance->m_persistent)
                instance->m_creatorGroup = 0;

            instance->m_mapMgr = nullptr;
            instance->m_isBattleground = false;

            if (m_instances[instance->m_mapId] == nullptr)
                m_instances[instance->m_mapId] = new InstanceMap;

            m_instances[instance->m_mapId]->insert(InstanceMap::value_type(instance->m_instanceId, instance));

            ++count;

        } while (result->NextRow());

        delete result;
    }

    LogDetail("InstanceMgr : %u saved instances loaded.", count);
}

uint32_t InstanceMgr::getNextInstanceId()
{
    if (m_InstanceHigh == 0)
    {
        if (QueryResult* result = CharacterDatabase.Query("SELECT MAX(id) FROM instances"))
            return result->Fetch()[0].GetUInt32() + 1;

        return 1;
    }

    m_mapLock.Acquire();
    const auto nextInstanceId = m_InstanceHigh++;
    m_mapLock.Release();

    return nextInstanceId;
}

