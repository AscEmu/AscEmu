/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "MapMgr.h"
#include "WorldCreator.h"
#include "Server/Packets/SmsgUpdateLastInstance.h"
#include "Server/Packets/SmsgUpdateInstanceOwnership.h"
#include "Server/Packets/SmsgInstanceReset.h"

using namespace AscEmu::Packets;

//MIT
inline bool checkInstanceGroup(Instance* instance, Group* group)
{
    return instance->m_creatorGroup == 0 || group && instance->m_creatorGroup == group->GetID();
}

SERVER_DECL InstanceMgr sInstanceMgr;

//\brief:   1) We should create a new instance for every row in worldmap_info
//             wodlmap_info already checks if a mapId is valid. (Check)
//          2) If there is a record in instances, save new InstanceMap in m_instances
//          3) Store loaded maps in m_maps[] (Maps*)
//          4) Generate instances and store them in m_singleMaps[] (MapMgr*) if mapId is continent
//             MapMgr creates a new thread for m_singleMaps[] and m_instances[]

void InstanceMgr::Load()
{
    m_InstanceHigh = getNextInstanceId();

    generateInstances();

    loadInstanceResetTimes();

    deleteExpiredAndInvalidInstances();

    loadAndApplySavedInstanceValues();

}

void InstanceMgr::SaveInstanceToDB(Instance* instance)
{
    if (instance != nullptr)
    {
        if (instance->m_mapInfo->type == INSTANCE_NONRAID || instance->m_isBattleground)
            return;

        CharacterDatabase.Execute("DELETE FROM instances WHERE id = %u;", instance->m_instanceId);

        std::stringstream insertStream;

        insertStream << "INSERT INTO instances VALUES("
            << instance->m_instanceId << ","
            << instance->m_mapId << ","
            << instance->m_creation << ","
            << static_cast<uint32_t>(instance->m_expiration) << ",'";

        for (auto itr : instance->m_killedNpcs)
            insertStream << itr << " ";

        insertStream << "',"
            << static_cast<uint32_t>(instance->m_difficulty) << ","
            << instance->m_creatorGroup << ","
            << instance->m_creatorGuid << ","
            << instance->m_persistent << ")";

        CharacterDatabase.Execute(insertStream.str().c_str());
    }
}

void InstanceMgr::Shutdown()
{
    for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
    {
        if (m_instances[i] != nullptr)
        {
            for (auto instanceMap = m_instances[i]->begin(); instanceMap != m_instances[i]->end(); ++instanceMap)
            {
                if (instanceMap->second->m_mapMgr)
                    instanceMap->second->m_mapMgr->KillThread();

                delete instanceMap->second;
            }

            delete m_instances[i];
            m_instances[i] = nullptr;
        }

        if (m_singleMaps[i] != nullptr)
        {
            auto mapMgr = m_singleMaps[i];
            mapMgr->KillThread();
            delete mapMgr;
            m_singleMaps[i] = nullptr;
        }

        if (m_maps[i] != nullptr)
        {
            delete m_maps[i];
            m_maps[i] = nullptr;
        }
    }
}

Map* InstanceMgr::GetMap(uint32_t mapid)
{
    if (mapid >= MAX_NUM_MAPS)
        return nullptr;

    return m_maps[mapid];
}

uint32_t InstanceMgr::PreTeleport(uint32_t mapid, Player* plr, uint32_t instanceid)
{
    if (mapid >= MAX_NUM_MAPS)
        return INSTANCE_ABORT_NOT_FOUND;

    const auto mapInfo = sMySQLStore.getWorldMapInfo(mapid);
    if (mapInfo == nullptr)
        return INSTANCE_ABORT_NOT_FOUND;

    // main continent check.
    if (mapInfo->type == INSTANCE_NULL)
    {
        // this will be useful when clustering comes into play.
        // we can check if the destination world server is online or not and then cancel them before they load.
        return m_singleMaps[mapid] != nullptr ? INSTANCE_OK : INSTANCE_ABORT_NOT_FOUND;
    }

    // shouldn't happen
    if (mapInfo->type == INSTANCE_BATTLEGROUND)
        return INSTANCE_ABORT_NOT_FOUND;

    Group* pGroup = plr->getGroup();

    // players without groups cannot enter raids and heroic instances

    if (pGroup == nullptr &&
        mapInfo->type == INSTANCE_RAID &&
        !plr->m_cheats.hasTriggerpassCheat)
        return INSTANCE_ABORT_NOT_IN_RAID_GROUP;

    if (pGroup == nullptr &&
        (mapInfo->type == INSTANCE_NONRAID && plr->getDungeonDifficulty() == MODE_HEROIC) &&
        !plr->m_cheats.hasTriggerpassCheat)
        return INSTANCE_ABORT_NOT_IN_RAID_GROUP;


    // players without raid groups cannot enter raid instances
    if (pGroup != nullptr && pGroup->getGroupType() != GROUP_TYPE_RAID && mapInfo->type == INSTANCE_RAID && !plr->m_cheats.hasTriggerpassCheat)
        return INSTANCE_ABORT_NOT_IN_RAID_GROUP;

    // We deny transfer if we requested a heroic instance of a map that has no heroic mode
    // We are trying to enter into a non-multi instance with a heroic group, downscaling
    if (mapInfo->type == INSTANCE_NONRAID && plr->getDungeonDifficulty() == MODE_HEROIC)
    {
        plr->setDungeonDifficulty(MODE_NORMAL);
        plr->sendDungeonDifficultyPacket();

        if (pGroup != nullptr)
            pGroup->SetDungeonDifficulty(MODE_NORMAL);
    }

    // if it's not a normal / 10men normal then check if we even have this mode
    if (mapInfo->type == INSTANCE_RAID && plr->getRaidDifficulty() != MODE_NORMAL_10MEN)
    {
        uint8_t newtype = 0;

        if (!mapInfo->hasDifficulty(plr->getRaidDifficulty()))
        {
            // no it doesn't so we will downscale it

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // This part is totally speculative, if you know how this is done actually then do change it
            //
            switch (plr->getRaidDifficulty())
            {
                case MODE_NORMAL_25MEN:
                case MODE_HEROIC_10MEN:
                {
                    newtype = MODE_NORMAL_10MEN;
                    break;
                }
                case MODE_HEROIC_25MEN:
                {
                    newtype = MODE_NORMAL_25MEN;
                    break;
                }
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // check if we have this mode
            if (!mapInfo->hasDifficulty(newtype))
            {
                //appearantly we don't so we set to 10men normal, which is the default for old raids too
                //regardless of their playerlimit
                newtype = MODE_NORMAL_10MEN;
            }

            // Setting the new mode on us and our group
            if (plr->getRaidDifficulty() != newtype)
            {
                plr->setRaidDifficulty(newtype);
                plr->sendRaidDifficultyPacket();

                if (pGroup != nullptr)
                    pGroup->SetRaidDifficulty(newtype);
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // If we are here, it means:
    // 1.) We're a simple non-raid and non-heroic dungeon
    // 2.) We're a multi-dungeon set to heroic and we are in a group
    // 3.) We're a raid, and we are in a raid group
    // 4.) We're a raid, we are in a raid group, and we have the right mode set
    //
    // So, first we have to check if they have an instance on this map already, if so, allow them to teleport to that.
    // Otherwise, we will try to create them a new one.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    m_mapLock.Acquire();
    InstanceMap* instancemap = m_instances[mapid];

    // If there are no instances of this map yet, we need to create the map
    if (instancemap == nullptr)
    {
        if (instanceid != 0)
        {
            m_mapLock.Release();
            return INSTANCE_ABORT_NOT_FOUND;
        }

        m_instances[mapid] = new InstanceMap;
        instancemap = m_instances[mapid];
    }
    else
    {
        // this is the case when we enter an already existing instance (with summons for example)
        if (instanceid != 0)
        {
            const auto itr = instancemap->find(instanceid);
            if (itr != instancemap->end())
            {
                Instance* in = itr->second;

                if (!checkInstanceGroup(in, pGroup))
                {
                    // Another group is already playing in this instance of the dungeon...
                    m_mapLock.Release();
                    sChatHandler.SystemMessage(plr->GetSession(), "Another group is already inside this instance of the dungeon.");
                    return INSTANCE_ABORT_NOT_IN_RAID_GROUP;
                }

                // Try to add instance ID to player
                plr->SetPersistentInstanceId(in);

                // Set current group
                if (pGroup)
                    in->m_creatorGroup = pGroup->GetID();

                m_mapLock.Release();
                return INSTANCE_OK;
            }

            m_mapLock.Release();
            return INSTANCE_ABORT_NOT_FOUND;

        }
        else  // this is the case when we enter the normal way (e.g. we enter thru the portal)
        {
            Instance* in = nullptr;
            if (pGroup != nullptr) // we are in a group
            {

                uint8_t grpdiff;

                // We want to use the raid difficulty for raids, and dungeon difficulty for dungeons
                if (mapInfo->type == INSTANCE_RAID)
                    grpdiff = pGroup->m_raiddifficulty;
                else
                    grpdiff = pGroup->m_difficulty;

                if ((mapInfo->type == INSTANCE_MULTIMODE && grpdiff == MODE_HEROIC) || mapInfo->type == INSTANCE_RAID)
                {
                    // This is the case when we don't have this map on this difficulty saved yet for the player entering
                    if (plr->GetPersistentInstanceId(mapid, grpdiff) == 0)
                    {
                        // The group has this instance saved already so we will use it
                        if (pGroup->m_instanceIds[mapid][grpdiff] != 0)
                        {
                            in = sInstanceMgr.GetInstanceByIds(mapid, pGroup->m_instanceIds[mapid][grpdiff]);
                        }
                        else if (worldConfig.instance.useGroupLeaderInstanceId)
                        {
                            PlayerInfo* pLeaderInfo = pGroup->GetLeader();
                            if (pLeaderInfo)
                            {
                                pLeaderInfo->savedInstanceIdsLock.Acquire();
                                const auto itrLeader = pLeaderInfo->savedInstanceIds[grpdiff].find(mapid);
                                if (itrLeader != pLeaderInfo->savedInstanceIds[grpdiff].end())
                                {
                                    in = sInstanceMgr.GetInstanceByIds(mapid, (*itrLeader).second);
                                }
                                pLeaderInfo->savedInstanceIdsLock.Release();
                            }
                        }
                    }

                    // If we have it saved to the player then use that
                    if (in == nullptr && plr->GetPersistentInstanceId(mapid, grpdiff) != 0)
                    {
                        in = sInstanceMgr.GetInstanceByIds(mapid, plr->GetPersistentInstanceId(mapid, grpdiff));
                    }
                }
                else
                {
                    if (pGroup->m_instanceIds[mapid][grpdiff] != 0)
                    {
                        in = sInstanceMgr.GetInstanceByIds(mapid, pGroup->m_instanceIds[mapid][grpdiff]);
                    }
                }
            }

            // We are not in a group, so we will look for an instance that we own and has the right difficulty
            if (in == nullptr)
            {
                uint32_t diff;

                if (mapInfo->type == INSTANCE_RAID)
                    diff = plr->getRaidDifficulty();
                else
                    diff = plr->getDungeonDifficulty();

                for (auto itr = instancemap->begin(); itr != instancemap->end();)
                {
                    in = itr->second;
                    ++itr;

                    if (in->m_difficulty == diff && PlayerOwnsInstance(in, plr))
                        break;

                    in = nullptr;
                }
            }

            // We've found an instance!
            if (in != nullptr)
            {
                m_mapLock.Release();

                // check the player count and in combat status.
                if (in->m_mapMgr)
                {
                    if (in->m_mapMgr->IsCombatInProgress())
                        return INSTANCE_ABORT_ENCOUNTER;

                    if (in->m_mapMgr->GetPlayerCount() >= mapInfo->playerlimit)
                        return INSTANCE_ABORT_FULL;
                }

                if (!checkInstanceGroup(in, pGroup))
                {
                    // Another group is already playing in this instance of the dungeon...
                    sChatHandler.SystemMessage(plr->GetSession(), "Another group is already inside this instance of the dungeon.");
                    return INSTANCE_ABORT_NOT_IN_RAID_GROUP;
                }

                // Try to add instance ID to player
                plr->SetPersistentInstanceId(in);

                // Set current group
                if (pGroup)
                    in->m_creatorGroup = pGroup->GetID();

                plr->SetInstanceID(in->m_instanceId);

                // found our instance, allow him in.
                return INSTANCE_OK;
            }
        }
    }

    // if we're here, it means we need to create a new instance.
    Instance* in = new Instance;
    in->m_creation = Util::getMSTime();

    switch (mapInfo->type)
    {
        case INSTANCE_NONRAID:
        case INSTANCE_MULTIMODE:
            in->m_difficulty = plr->getDungeonDifficulty();
            break;
        case INSTANCE_RAID:
            in->m_difficulty = plr->getRaidDifficulty();
            break;
    }

    in->m_instanceId = getNextInstanceId();
    in->m_mapId = mapid;
    in->m_mapInfo = mapInfo;
    in->m_mapMgr = nullptr;        // always start off without a map manager, it is created in GetInstance()
    in->m_isBattleground = false;
    in->m_persistent = in->isPersistent() && sObjectMgr.GetDungeonEncounterList(mapid, plr->getDungeonDifficulty()) == nullptr;
    in->m_creatorGuid = pGroup ? 0 : plr->getGuidLow();        // creator guid is 0 if its owned by a group.
    in->m_creatorGroup = pGroup ? pGroup->GetID() : 0;

    if (worldConfig.instance.isRelativeExpirationEnabled)
    {
        if (mapInfo->type == INSTANCE_MULTIMODE && in->m_difficulty == MODE_HEROIC)
            in->m_expiration = UNIXTIME + TimeVars::Day;
        else
            in->m_expiration = (mapInfo->type == INSTANCE_NONRAID || (mapInfo->type == INSTANCE_MULTIMODE && in->m_difficulty == MODE_NORMAL)) ? 0 : UNIXTIME + mapInfo->cooldown;
    }
    else
    {
        if (mapInfo->type == INSTANCE_MULTIMODE && in->m_difficulty >= MODE_HEROIC)
        {
            in->m_expiration = UNIXTIME - (UNIXTIME % TimeVars::Day) + ((UNIXTIME % TimeVars::Day) > (worldConfig.instance.relativeDailyHeroicInstanceResetHour * TimeVars::Hour) ? 82800 : -3600) + ((worldConfig.instance.relativeDailyHeroicInstanceResetHour - worldConfig.server.gmtTimeZone) * TimeVars::Hour);
        }
        else if (in->isPersistent())
        {
            if (m_nextInstanceReset[in->m_mapId] == 0)
            {
                m_nextInstanceReset[in->m_mapId] = UNIXTIME - (UNIXTIME % TimeVars::Day) - ((worldConfig.server.gmtTimeZone + 1) * TimeVars::Hour) + (in->m_mapInfo->cooldown == 0 ? TimeVars::Day : in->m_mapInfo->cooldown);
                CharacterDatabase.Execute("DELETE FROM server_settings WHERE setting_id LIKE 'next_instance_reset_%u';", in->m_mapId);
                CharacterDatabase.Execute("INSERT INTO server_settings VALUES ('next_instance_reset_%u', '%u')", in->m_mapId, m_nextInstanceReset[in->m_mapId]);
            }
            if (m_nextInstanceReset[in->m_mapId] + (TimeVars::Minute * 15) < UNIXTIME)
            {
                do
                {
                    time_t tmp = m_nextInstanceReset[in->m_mapId];
                    if (tmp + (TimeVars::Minute * 15) < UNIXTIME)
                        m_nextInstanceReset[in->m_mapId] = tmp + (in->m_mapInfo->cooldown == 0 ? TimeVars::Day : in->m_mapInfo->cooldown);
                }
                while (m_nextInstanceReset[in->m_mapId] + (TimeVars::Minute * 15) < UNIXTIME);
                CharacterDatabase.Execute("DELETE FROM server_settings WHERE setting_id LIKE 'next_instance_reset_%u';", in->m_mapId);
                CharacterDatabase.Execute("INSERT INTO server_settings VALUES ('next_instance_reset_%u', '%u')", in->m_mapId, m_nextInstanceReset[in->m_mapId]);
            }
            in->m_expiration = m_nextInstanceReset[in->m_mapId];
        }
        else
        {
            in->m_expiration = (mapInfo->type == INSTANCE_NONRAID || (mapInfo->type == INSTANCE_MULTIMODE && in->m_difficulty == MODE_NORMAL)) ? 0 : UNIXTIME + mapInfo->cooldown;
        }
    }

    plr->SetInstanceID(in->m_instanceId);
    LOG_DEBUG("Creating instance for player %u and group %u on map %u. (%u)", in->m_creatorGuid, in->m_creatorGroup, in->m_mapId, in->m_instanceId);

    // save our new instance to the database.
    SaveInstanceToDB(in);

    // apply it in the instance map
    instancemap->insert(InstanceMap::value_type(in->m_instanceId, in));

    // Try to add instance ID to player
    plr->SetPersistentInstanceId(in);

    // instance created ok, i guess? return the ok for him to transport.
    m_mapLock.Release();

    return INSTANCE_OK;
}

MapMgr* InstanceMgr::GetMapMgr(uint32_t mapId)
{
    return m_singleMaps[mapId];
}

bool InstanceMgr::InstanceExists(uint32_t mapid, uint32_t instanceId)
{
    return GetInstanceByIds(mapid, instanceId) != nullptr;
}

Instance* InstanceMgr::GetInstanceByIds(uint32_t mapid, uint32_t instanceId)
{
    if (mapid > MAX_NUM_MAPS)
        return nullptr;

    if (mapid == MAX_NUM_MAPS)
    {
        for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
        {
            Instance* in = GetInstanceByIds(i, instanceId);
            if (in != nullptr)
                return in;
        }

        return nullptr;
    }

    InstanceMap* map = m_instances[mapid];
    if (map == nullptr)
        return nullptr;

    const auto instance = map->find(instanceId);
    return instance == map->end() ? nullptr : instance->second;
}

MapMgr* InstanceMgr::GetInstance(Object* obj)
{
    const auto mapInfo = sMySQLStore.getWorldMapInfo(obj->GetMapId());
    if (mapInfo == nullptr || obj->GetMapId() >= MAX_NUM_MAPS)
        return nullptr;

    if (obj->isPlayer())
    {
        // players can join instances based on their groups/solo status.
        if (auto player = dynamic_cast<Player*>(obj))
        {
            // single-instance maps never go into the instance set.
            if (mapInfo->type == INSTANCE_NULL)
                return m_singleMaps[player->GetMapId()];

            m_mapLock.Acquire();
            auto instanceMap = m_instances[player->GetMapId()];
            if (instanceMap != nullptr)
            {
                // check our saved instance id. see if its valid, and if we can join before trying to find one.
                auto itr = instanceMap->find(player->GetInstanceID());
                if (itr != instanceMap->end())
                {
                    if (itr->second->m_mapMgr == nullptr)
                    {
                        itr->second->m_mapMgr = _CreateInstance(itr->second);
                    }

                    if (itr->second->m_mapMgr)
                    {
                        m_mapLock.Release();
                        return itr->second->m_mapMgr;
                    }
                }

                // iterate over our instances, and see if any of them are owned/joinable by him.
                for (itr = instanceMap->begin(); itr != instanceMap->end();)
                {
                    Instance* in = itr->second;
                    ++itr;

                    uint32_t difficulty;

                    if (in->m_mapInfo->type == INSTANCE_RAID)
                        difficulty = player->getRaidDifficulty();
                    else
                        difficulty = player->getDungeonDifficulty();

                    if (in->m_difficulty == difficulty && PlayerOwnsInstance(in, player))
                    {
                        // this is our instance.
                        if (in->m_mapMgr == nullptr)
                        {
                            /*if (plr->m_TeleportState == 1)
                            {
                            // the player is loading. boot him out to the entry point, we don't want to spam useless instances on startup.
                            m_mapLock.Release();
                            return NULL;
                            }*/

                            // create the actual instance.
                            in->m_mapMgr = _CreateInstance(in);
                            m_mapLock.Release();
                            return in->m_mapMgr;
                        }
                        else
                        {
                            // instance is already created.
                            m_mapLock.Release();
                            return in->m_mapMgr;
                        }
                    }
                }
            }

            // if we're here, it means there are no instances on that map, or none of the instances on that map are joinable
            // by this player.
            m_mapLock.Release();
            return nullptr;
        }
    }
    else
    {
        // units are *always* limited to their set instance ids.
        if (mapInfo->type == INSTANCE_NULL)
            return m_singleMaps[obj->GetMapId()];

        m_mapLock.Acquire();
        auto instancemap = m_instances[obj->GetMapId()];
        if (instancemap)
        {
            auto itr = instancemap->find(obj->GetInstanceID());
            if (itr != instancemap->end())
            {
                // we never create instances just for units.
                m_mapLock.Release();
                return itr->second->m_mapMgr;
            }
        }

        // instance is non-existent (shouldn't really happen for units...)
        m_mapLock.Release();
        return nullptr;
    }
    return nullptr;
}

MapMgr* InstanceMgr::_CreateInstance(uint32_t mapid, uint32_t instanceid)
{
    const auto mapInfo = sMySQLStore.getWorldMapInfo(mapid);

    ARCEMU_ASSERT(mapInfo != nullptr && mapInfo->type == INSTANCE_NULL);
    ARCEMU_ASSERT(mapid < MAX_NUM_MAPS && m_maps[mapid] != nullptr);

    LogNotice("InstanceMgr : Creating continent %s.", m_maps[mapid]->GetMapName().c_str());

    const auto newMap = new MapMgr(m_maps[mapid], mapid, instanceid);

    ARCEMU_ASSERT(newMap != nullptr);

    // Scheduling the new map for running
    ThreadPool.ExecuteTask(newMap);
    m_singleMaps[mapid] = newMap;

    return newMap;
}

MapMgr* InstanceMgr::_CreateInstance(Instance* in)
{
    if (m_maps[in->m_mapId] == nullptr)
        return nullptr;

    LogNotice("InstanceMgr : Creating saved instance %u (%s)", in->m_instanceId, m_maps[in->m_mapId]->GetMapName().c_str());
    ARCEMU_ASSERT(in->m_mapMgr == NULL);

    // we don't have to check for world map info here, since the instance wouldn't have been saved if it didn't have any.
    in->m_mapMgr = new MapMgr(m_maps[in->m_mapId], in->m_mapId, in->m_instanceId);
    in->m_mapMgr->pInstance = in;
    in->m_mapMgr->iInstanceMode = in->m_difficulty;
    in->m_mapMgr->InactiveMoveTime = 60 + UNIXTIME;

    ThreadPool.ExecuteTask(in->m_mapMgr);
    return in->m_mapMgr;
}

void InstanceMgr::_CreateMap(uint32_t mapid)
{
    if (mapid >= MAX_NUM_MAPS)
        return;

    const auto mapInfo = sMySQLStore.getWorldMapInfo(mapid);
    if (mapInfo == nullptr)
        return;

    if (m_maps[mapid] != nullptr)
        return;

    m_maps[mapid] = new Map(mapid, mapInfo);
    if (mapInfo->type == INSTANCE_NULL)
    {
        // we're a continent, create the instance.
        _CreateInstance(mapid, getNextInstanceId());
    }
}

void InstanceMgr::ResetSavedInstances(Player* plr)
{
    if (!plr->IsInWorld() || plr->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL)
        return;

    m_mapLock.Acquire();
    for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
    {
        if (m_instances[i] != nullptr)
        {
            auto instancemap = m_instances[i];
            for (auto itr = instancemap->begin(); itr != instancemap->end();)
            {
                auto instance = itr->second;
                ++itr;

                if (instance->isResetable() && (checkInstanceGroup(instance, plr->getGroup()) || plr->getGuidLow() == instance->m_creatorGuid))
                {
                    if (instance->m_mapMgr && instance->m_mapMgr->HasPlayers())
                    {
                        plr->GetSession()->SystemMessage("Failed to reset instance %u (%s), due to players still inside.", instance->m_instanceId, instance->m_mapMgr->GetMapInfo()->name.c_str());
                        continue;
                    }

                    // <mapid> has been reset.
                    plr->GetSession()->SendPacket(SmsgInstanceReset(instance->m_mapId).serialise().get());

                    // destroy the instance
                    _DeleteInstance(instance, true);
                }
            }
        }
    }
    m_mapLock.Release();
}

void InstanceMgr::OnGroupDestruction(Group* pGroup)
{
    m_mapLock.Acquire();
    for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
    {
        auto instanceMap = m_instances[i];
        if (instanceMap)
        {
            for (auto itr = instanceMap->begin(); itr != instanceMap->end();)
            {
                auto instance = itr->second;
                ++itr;

                if (instance->m_mapMgr && instance->m_creatorGroup && instance->m_creatorGroup == pGroup->GetID())
                {
                    if (instance->isResetable())
                    {
                        _DeleteInstance(instance, false);
                    }
                    else if (instance->m_mapMgr->HasPlayers())
                    {
                        for (auto playerStorageMap = instance->m_mapMgr->m_PlayerStorage.begin(); playerStorageMap != instance->m_mapMgr->m_PlayerStorage.end(); ++playerStorageMap)
                        {
                            if ((*playerStorageMap).second->IsInWorld() && !(*playerStorageMap).second->raidgrouponlysent && (*playerStorageMap).second->GetInstanceID() == static_cast<int32_t>(instance->m_instanceId))
                            {
                                (*playerStorageMap).second->sendRaidGroupOnly(60000, 1);
                                (*playerStorageMap).second->raidgrouponlysent = true;

                                sEventMgr.AddEvent((*playerStorageMap).second, &Player::EjectFromInstance, EVENT_PLAYER_EJECT_FROM_INSTANCE, 60000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                            }
                        }
                    }
                }
            }
        }
    }
    m_mapLock.Release();
}

bool InstanceMgr::_DeleteInstance(Instance* in, bool ForcePlayersOut)
{
    m_mapLock.Acquire();

    if (in->m_mapMgr)
    {
        // "ForcePlayersOut" will teleport the players in this instance to their entry point/hearthstone.
        // otherwise, they will get a 60 second timeout telling them they are not in this instance's group.
        if (in->m_mapMgr->HasPlayers())
        {
            if (ForcePlayersOut)
            {
                in->m_mapMgr->InstanceShutdown();
            }
            else
            {
                in->m_mapMgr->BeginInstanceExpireCountdown();
                in->m_mapMgr->pInstance = nullptr;
            }
        }
        else
        {
            in->m_mapMgr->InstanceShutdown();
        }
    }

    // remove the instance from the large map.
    auto instanceMap = m_instances[in->m_mapId];
    if (instanceMap)
    {
        const auto itr = instanceMap->find(in->m_instanceId);
        if (itr != instanceMap->end())
            instanceMap->erase(itr);
    }

    // cleanup corpses, database references
    {
        CharacterDatabase.Execute("DELETE FROM corpses WHERE instanceid = %u", in->m_instanceId);

        CharacterDatabase.Execute("DELETE FROM instances WHERE id = %u", in->m_instanceId);

        CharacterDatabase.Execute("DELETE FROM instanceids WHERE mapId = %u AND instanceId = %u AND mode = %u", in->m_mapId, in->m_instanceId, in->m_difficulty);
    }

    // delete the instance pointer.
    delete in;

    m_mapLock.Release();

    return true;
}

void InstanceMgr::CheckForExpiredInstances()
{
    m_mapLock.Acquire();
    for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
    {
        auto instanceMap = m_instances[i];
        if (instanceMap)
        {
            for (auto itr = instanceMap->begin(); itr != instanceMap->end();)
            {
                auto instance = itr->second;
                ++itr;

                // use a "soft" delete here.
                if (instance->m_mapInfo->type != INSTANCE_NONRAID && !(instance->m_mapInfo->type == INSTANCE_MULTIMODE && instance->m_difficulty == MODE_NORMAL) && HasInstanceExpired(instance))
                    _DeleteInstance(instance, false);
            }

        }
    }
    m_mapLock.Release();
}

void InstanceMgr::BuildSavedInstancesForPlayer(Player* plr)
{
    if (!plr->IsInWorld() || plr->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL)
    {
        m_mapLock.Acquire();
        for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
        {
            if (m_instances[i] != nullptr)
            {
                auto instanceMap = m_instances[i];
                for (auto itr = instanceMap->begin(); itr != instanceMap->end();)
                {
                    const auto instance = itr->second;
                    ++itr;

                    if (PlayerOwnsInstance(instance, plr) && instance->m_mapInfo->type == INSTANCE_NONRAID)
                    {
                        m_mapLock.Release();

                        plr->GetSession()->SendPacket(SmsgUpdateLastInstance(instance->m_mapId).serialise().get());

                        plr->GetSession()->SendPacket(SmsgUpdateInstanceOwnership(0x01).serialise().get());

                        return;
                    }
                }
            }
        }
        m_mapLock.Release();
    }

    plr->GetSession()->SendPacket(SmsgUpdateInstanceOwnership(0x00).serialise().get());
}

void InstanceMgr::BuildRaidSavedInstancesForPlayer(Player* plr)
{
    uint32_t counter = 0;

    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 4);

    auto _counter = data.wpos();

    data << counter;
    m_mapLock.Acquire();

    for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
    {
        if (m_instances[i] != nullptr)
        {
            auto instanceMap = m_instances[i];
            for (auto itr = instanceMap->begin(); itr != instanceMap->end();)
            {
                const auto instance = itr->second;
                ++itr;

                if (instance->m_persistent && PlayerOwnsInstance(instance, plr))
                {
#if VERSION_STRING <= TBC
                    data << uint32_t(instance->m_mapId);
                    if (instance->m_expiration > UNIXTIME)
                        data << uint32_t(instance->m_expiration - UNIXTIME);
                    else
                        data << uint32_t(0);

                    data << uint64_t(instance->m_instanceId);

                    data << uint32_t(0);    //unknown
#else
                    data << uint32_t(instance->m_mapId);
                    data << uint32_t(instance->m_difficulty);
                    data << uint64_t(instance->m_instanceId);
                    data << uint8_t(1);                                    // expired = 0
                    data << uint8_t(0);                                    // extended = 1

                    if (instance->m_expiration > UNIXTIME)
                        data << uint32_t(instance->m_expiration - UNIXTIME);
                    else
                        data << uint32_t(0);
#endif
#if VERSION_STRING >= Cata
                    data << uint32_t(0); // completed encounter mask
#endif

                    ++counter;
                }
            }
        }
    }
    m_mapLock.Release();

    data.put<uint32_t>(_counter, counter);
    plr->GetSession()->SendPacket(&data);
}

void InstanceMgr::PlayerLeftGroup(Group* pGroup, Player* pPlayer)
{
    m_mapLock.Acquire();
    for (uint32_t i = 0; i < MAX_NUM_MAPS; ++i)
    {
        auto instanceMap = m_instances[i];
        if (instanceMap)
        {
            for (auto itr = instanceMap->begin(); itr != instanceMap->end();)
            {
                const auto instance = itr->second;
                ++itr;

                if (instance->m_creatorGroup && instance->m_creatorGroup == pGroup->GetID())
                {
                    // better make sure we're actually in that instance.. :P
                    if (!pPlayer->raidgrouponlysent && pPlayer->GetInstanceID() == static_cast<int32_t>(instance->m_instanceId))
                    {
                        pPlayer->sendRaidGroupOnly(60000, 1);
                        pPlayer->raidgrouponlysent = true;

                        sEventMgr.AddEvent(pPlayer, &Player::EjectFromInstance, EVENT_PLAYER_EJECT_FROM_INSTANCE, 60000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

                        m_mapLock.Release();
                        return;
                    }
                }
            }
        }
    }
    m_mapLock.Release();
}

bool InstanceMgr::PlayerOwnsInstance(Instance* pInstance, Player* pPlayer)
{
    // Expired?
    if (pInstance->m_expiration && (UNIXTIME + 20) >= pInstance->m_expiration)
    {
        _DeleteInstance(pInstance, true);
        return false;
    }

    // Persistent instance handling
    if (pInstance->m_persistent)
    {
        return (pPlayer->GetPersistentInstanceId(pInstance->m_mapId, pInstance->m_difficulty) == pInstance->m_instanceId);
    }

    // Default instance handling
    if ((pPlayer->getGroup() && pInstance->m_creatorGroup == pPlayer->getGroup()->GetID()) || pPlayer->getGuidLow() == pInstance->m_creatorGuid)
    {
        return true;
    }

    return false;
}

bool InstanceMgr::HasInstanceExpired(Instance* pInstance)
{
    // expired?
    if (pInstance->m_expiration && (UNIXTIME + 20) >= pInstance->m_expiration)
        return true;

    return false;
}

MapMgr* InstanceMgr::CreateBattlegroundInstance(uint32_t mapid)
{
    // shouldn't happen
    if (mapid >= MAX_NUM_MAPS)
        return nullptr;

    if (!m_maps[mapid])
    {
        _CreateMap(mapid);
        if (!m_maps[mapid])
            return nullptr;
    }

    auto mapMgr = new MapMgr(m_maps[mapid], mapid, getNextInstanceId());
    auto instance = new Instance();

    instance->m_creation = Util::getMSTime();
    instance->m_creatorGroup = 0;
    instance->m_creatorGuid = 0;
    instance->m_difficulty = 0;
    instance->m_expiration = 0;
    instance->m_instanceId = mapMgr->GetInstanceID();
    instance->m_isBattleground = true;
    instance->m_persistent = false;
    instance->m_mapId = mapid;
    instance->m_mapInfo = sMySQLStore.getWorldMapInfo(mapid);
    instance->m_mapMgr = mapMgr;

    m_mapLock.Acquire();

    if (m_instances[mapid] == nullptr)
        m_instances[mapid] = new InstanceMap;

    m_instances[mapid]->insert(std::make_pair(instance->m_instanceId, instance));

    m_mapLock.Release();
    ThreadPool.ExecuteTask(mapMgr);

    return mapMgr;
}

MapMgr* InstanceMgr::CreateInstance(uint32_t /*instanceType*/, uint32_t mapid)
{
    if (mapid >= MAX_NUM_MAPS)
        return nullptr;

    if (!m_maps[mapid])
    {
        _CreateMap(mapid);

        if (!m_maps[mapid])
            return nullptr;
    }

    auto mapMgr = new MapMgr(m_maps[mapid], mapid, getNextInstanceId());
    auto instance = new Instance();

    instance->m_creation = Util::getMSTime();
    instance->m_creatorGroup = 0;
    instance->m_creatorGuid = 0;
    instance->m_difficulty = 0;
    instance->m_expiration = 0;
    instance->m_instanceId = mapMgr->GetInstanceID();
    instance->m_persistent = false;
    instance->m_mapId = mapid;
    instance->m_mapInfo = sMySQLStore.getWorldMapInfo(mapid);
    instance->m_mapMgr = mapMgr;

    m_mapLock.Acquire();

    if (m_instances[mapid] == nullptr)
        m_instances[mapid] = new InstanceMap;

    m_instances[mapid]->insert(std::make_pair(instance->m_instanceId, instance));

    m_mapLock.Release();
    ThreadPool.ExecuteTask(mapMgr);

    return mapMgr;
}

// should be safe to do for instances of any non-world maps
void InstanceMgr::SafeDeleteInstance(MapMgr* mgr)
{
    if (!mgr)
        return;

    // Shutdown Instance
    mgr->TeleportPlayers(); // Get everyone out
    mgr->InstanceShutdown();

    DeleteBattlegroundInstance(mgr->GetMapId(), mgr->GetInstanceID());
}

void InstanceMgr::DeleteBattlegroundInstance(uint32_t mapid, uint32_t instanceid)
{
    m_mapLock.Acquire();

    const auto itr = m_instances[mapid]->find(instanceid);
    if (itr == m_instances[mapid]->end())
    {
        LOG_ERROR("Could not delete battleground instance!");
        m_mapLock.Release();
        return;
    }

    _DeleteInstance(itr->second, true);

    m_mapLock.Release();
}
