/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "InstanceMap.hpp"
#include "InstanceDefines.hpp"
#include "Server/Definitions.h"
#include "Server/Script/ScriptMgr.h"

InstanceMap::InstanceMap(BaseMap* baseMap, uint32_t id, uint32_t expiry, uint32_t InstanceId, uint8_t SpawnMode, PlayerTeam InstanceTeam)
    : WorldMap(baseMap, id, expiry, InstanceId, SpawnMode), instanceTeam(InstanceTeam)
{
    // WorldMap
    m_unloadTimer = expiry;

    pInstance = this;

    //lets initialize visibility distance for Instance
    InstanceMap::initVisibilityDistance();
}

void InstanceMap::update(uint32_t t_diff)
{
    WorldMap::update(t_diff);

    if (isUnloadPending())
        unloadAll();

    if (canUnload(t_diff))
    {
        // Remove all Players
        removeAllPlayers();

        // Unload Us :)
        setUnloadPending(true);
    }
}

void InstanceMap::unloadAll(bool onShutdown/* = false*/)
{
    if (m_resetAfterUnload == true)
    {
        deleteRespawnTimes();
    }

    WorldMap::unloadAll(onShutdown);
}

void InstanceMap::initVisibilityDistance()
{
    //init visibility distance for instances
    m_VisibleDistance = 500 * 500;
}

void InstanceMap::permBindAllPlayers()
{
    if (!getBaseMap()->isDungeon())
        return;

    InstanceSaved* save = sInstanceMgr.getInstanceSave(getInstanceId());
    if (!save)
    {
        sLogger.failure("Cannot bind players to instance map (Name: %s, Entry: %u, Difficulty: %u, ID: %u) because no instance save is available!", getBaseMap()->getMapName().c_str(), getBaseMap()->getMapId(), getDifficulty(), getInstanceId());
        return;
    }

    // perm bind all players that are currently inside the instance
    for (const auto& itr : getPlayers())
    {
        Player* player = itr.second;
        // never instance bind GMs with GM mode enabled
        if (player->isGMFlagSet())
            continue;

        InstancePlayerBind* bind = player->getBoundInstance(save->getMapId(), save->getDifficulty());
        if (bind && bind->perm)
        {
            if (bind->save && bind->save->getInstanceId() != save->getInstanceId())
            {
                sLogger.failure("Player (%s, Name: %s) is in instance map (Name: %s, Entry: %u, Difficulty: %u, ID: %u) that is being bound, but already has a save for the map on ID %u!", player->getGuidLow(), player->getName().c_str(), getBaseMap()->getMapName().c_str() , save->getMapId(), save->getDifficulty(), save->getInstanceId(), bind->save->getInstanceId());
            }
            else if (!bind->save)
            {
                sLogger.failure("Player (%s, Name: %s) is in instance map (Name: %s, Entry: %u, Difficulty: %u, ID: %u) that is being bound, but already has a bind (without associated save) for the map!", player->getGuidLow(), player->getName().c_str(), getBaseMap()->getMapName().c_str(), save->getMapId(), save->getDifficulty(), save->getInstanceId());
            }
        }
        else
        {
            player->bindToInstance(save, true);
            WorldPacket data(SMSG_INSTANCE_SAVE_CREATED, 4);
            data << uint32_t(0);
            player->sendPacket(&data);
#if VERSION_STRING > TBC
            player->getSession()->sendCalendarRaidLockout(save, true);
#endif

            // if group leader is in instance, group also gets bound
            if (Group* group = player->getGroup())
                if (group->GetLeader()->guid == player->getGuidLow())
                    group->bindToInstance(save, true);
        }
    }
}

bool InstanceMap::addPlayerToMap(Player* player)
{
    // disable unload Timer
    m_unloadTimer = 0;

    if (getBaseMap()->isDungeon())
    {
        Group* group = player->getGroup();

        // increase Instance Hourly Limit
        if (!group || !group->isLFGGroup())
            player->addInstanceEnterTime(getInstanceId(), Util::getTimeNow());

        // get or create an instance save for the map
        InstanceSaved* mapSave = sInstanceMgr.getInstanceSave(getInstanceId());
        if (!mapSave)
        {
            sLogger.debug("creating instance save for map %d spawnmode %d with instance id %d", getBaseMap()->getMapId(), getSpawnMode(), getInstanceId());
            mapSave = sInstanceMgr.addInstanceSave(getBaseMap()->getMapId(), getInstanceId(), InstanceDifficulty::Difficulties(getSpawnMode()), 0, true);
        }

        if (mapSave)
        {
            // check for existing instance binds
            InstancePlayerBind* playerBind = player->getBoundInstance(getBaseMap()->getMapId(), InstanceDifficulty::Difficulties(getSpawnMode()));
            if (playerBind && playerBind->perm)
            {
                // cannot enter other instances if bound permanently
                if (playerBind->save != mapSave)
                {
                    sLogger.debug("player % s % s is permanently bound to instance % s % d, % d, % d, % d, % d, % d but he is being put into instance % s % d, % d, % d, % d, % d, % d", player->getName().c_str(), player->getGuid(), getBaseMap()->getMapName().c_str(), playerBind->save->getMapId(), playerBind->save->getInstanceId(), playerBind->save->getDifficulty(), playerBind->save->getPlayerCount(), playerBind->save->getGroupCount(), playerBind->save->canReset(), getBaseMap()->getMapName().c_str(), mapSave->getMapId(), mapSave->getInstanceId(), mapSave->getDifficulty(), mapSave->getPlayerCount(), mapSave->getGroupCount(), mapSave->canReset());
                    return false;
                }
            }
            else
            {
                if (group)
                {
                    // solo saves should have been reset when the map was loaded
                    InstanceGroupBind* groupBind = group->getBoundInstance(getBaseMap());
                    if (playerBind && playerBind->save != mapSave)
                    {
                        sLogger.debug("player %s %s is being put into instance %s %d, %d, %d, %d, %d, %d but he is in group %s and is bound to instance %d, %d, %d, %d, %d, %d!", player->getName().c_str(), player->getGuid(), getBaseMap()->getMapName().c_str(), mapSave->getMapId(), mapSave->getInstanceId(), mapSave->getDifficulty(), mapSave->getPlayerCount(), mapSave->getGroupCount(), mapSave->canReset(), group->GetLeader()->guid, playerBind->save->getMapId(), playerBind->save->getInstanceId(), playerBind->save->getDifficulty(), playerBind->save->getPlayerCount(), playerBind->save->getGroupCount(), playerBind->save->canReset());
                        if (groupBind)
                            sLogger.debug("the group is bound to the instance %s %d, %d, %d, %d, %d, %d", getBaseMap()->getMapName().c_str(), groupBind->save->getMapId(), groupBind->save->getInstanceId(), groupBind->save->getDifficulty(), groupBind->save->getPlayerCount(), groupBind->save->getGroupCount(), groupBind->save->canReset());
                        return false;
                    }
                    // bind to the group or keep using the group save
                    if (!groupBind)
                    {
                        group->bindToInstance(mapSave, false);
                    }
                    else
                    {
                        // cannot jump to a different instance without resetting it
                        if (groupBind->save != mapSave)
                        {
                            sLogger.debug("player %s %s is being put into instance %d, %d, %d but he is in group %s which is bound to instance %d, %d, %d! \n", player->getName().c_str(), player->getGuid(), mapSave->getMapId(), mapSave->getInstanceId(), mapSave->getDifficulty(), group->GetLeader()->guid, groupBind->save->getMapId(), groupBind->save->getInstanceId(), groupBind->save->getDifficulty());
                            sLogger.debug("MapSave players: %d, group count: %d \n", mapSave->getPlayerCount(), mapSave->getGroupCount());
                            if (groupBind->save)
                                sLogger.debug("GroupBind save players: %d, group count: %d", groupBind->save->getPlayerCount(), groupBind->save->getGroupCount());
                            else
                                sLogger.debug("GroupBind save NULL");
                            return false;
                        }
                        // if the group/leader is permanently bound to the instance
                        // players also become permanently bound when they enter
                        if (groupBind->perm)
                        {
                            WorldPacket data(SMSG_INSTANCE_LOCK_WARNING_QUERY, 9);
                            data << uint32_t(60000);
                            data << uint32_t(getScript() ? getScript()->getCompletedEncounterMask() : 0);
                            data << uint8_t(0);
                            player->sendPacket(&data);
                            player->setPendingBind(mapSave->getInstanceId(), 60000);
                        }
                    }
                }
                else
                {
                    // set up a solo bind or continue using it
                    if (!playerBind)
                        player->bindToInstance(mapSave, false);
                    else
                        // cannot jump to a different instance without resetting it
                        ASSERT(playerBind->save == mapSave);
                }
            }
        }

        // for normal instances cancel the reset schedule when the
        // first player enters (no players yet)
        setResetSchedule(false);

        sLogger.info("Player '%s' entered instance '%u' of map '%s' \n", player->getName().c_str(), getInstanceId(), getBaseMap()->getMapName().c_str());
    }

    return true;
}

void InstanceMap::removePlayerFromMap(Player* /*player*/)
{
    // if last player set unload timer
    if (!m_unloadTimer && getPlayerCount() == 1)
        m_unloadTimer = m_unloadWhenEmpty ? 1U : worldConfig.server.mapUnloadTime * IN_MILLISECONDS;

    // for normal instances schedule the reset after all players have left
    setResetSchedule(true);
    sInstanceMgr.unloadInstanceSave(getInstanceId());
}

void InstanceMap::setResetSchedule(bool on)
{
    // only for normal instances
    // the reset time is only scheduled when there are no payers inside
    if (getBaseMap()->isDungeon() && !getPlayerCount() && !isRaidOrHeroicDungeon())
    {
        if (InstanceSaved* save = sInstanceMgr.getInstanceSave(getInstanceId()))
            sInstanceMgr.addResetEvent(on, save->getResetTime(), InstanceMgr::InstResetEvent(0, getBaseMap()->getMapId(), InstanceDifficulty::Difficulties(getSpawnMode()), getInstanceId()));
    }
}

void InstanceMap::sendResetWarnings(uint32_t timeLeft)
{
    for (const auto& itr : getPlayers())
        itr.second->sendInstanceResetWarning(getBaseMap()->getMapId(), itr.second->getDifficulty(getBaseMap()->isRaid()), timeLeft, false);
}

EnterState InstanceMap::cannotEnter(Player* player)
{
    if (isUnloadPending())
        return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;

    if (player->getWorldMap() == this)
        return CANNOT_ENTER_ALREADY_IN_MAP;

    // allow GM's to enter
    if (player->isGMFlagSet())
        return WorldMap::cannotEnter(player);

    // cannot enter if the instance is full (player cap)
    uint32_t maxPlayers = getMaxPlayers();
    if (getPlayerCount() >= maxPlayers)
        return CANNOT_ENTER_MAX_PLAYERS;

    // cannot enter while an encounter is in progress (unless this is a relog, in which case it is permitted)
    if (!player->IsInWorld() && getBaseMap()->isRaid() && isCombatInProgress())
        return CANNOT_ENTER_ENCOUNTER;

    // cannot enter if player is permanent saved to a different instance id
    if (InstancePlayerBind* playerBind = player->getBoundInstance(getBaseMap()->getMapId(), getDifficulty()))
        if (playerBind->perm && playerBind->save)
            if (playerBind->save->getInstanceId() != getInstanceId())
                return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;

    return WorldMap::cannotEnter(player);
}

void InstanceMap::createInstanceData(bool load)
{
    if (getScript() == nullptr)
        return;

    auto const& encounters = sObjectMgr.GetDungeonEncounterList(getBaseMap()->getMapId(), getDifficulty());

    uint32_t bossNumber = static_cast<uint32_t>(encounters->size());

    if (!getScript()->getEncounterCount())
        getScript()->setBossNumber(bossNumber);

    if (load)
    {
        auto result = CharacterDatabase.Query("SELECT data, completedEncounters FROM instance WHERE map = %u AND id = %u", getBaseMap()->getMapId(), getInstanceId());

        if (result)
        {
            Field* fields = result->Fetch();
            std::string data = fields[0].GetString();

            getScript()->setCompletedEncountersMask(fields[1].GetUInt32());
            if (!data.empty())
            {
                sLogger.debug("Loading instance data for `%s` with id %u", getBaseMap()->getMapName().c_str(), getInstanceId());
                getScript()->loadSavedInstanceData(data.c_str());
            }
        }
    }
    else
    {
        getScript()->generateBossDataState();
    }
}

bool InstanceMap::reset(uint8_t method)
{
    if (getPlayerCount())
    {
        if (method == INSTANCE_RESET_ALL || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            // notify the players to leave the instance so it can be reset
            for (const auto& itr : getPlayers())
                itr.second->sendResetFailedNotify(getBaseMap()->getMapId());
        }
        else
        {
            bool doUnload = true;
            if (method == INSTANCE_RESET_GLOBAL)
            {
                // set the homebind timer for players inside (1 minute)
                for (const auto& itr : getPlayers())
                {
                    InstancePlayerBind* bind = itr.second->getBoundInstance(getBaseMap()->getMapId(), getDifficulty());
                    if (bind && bind->extendState && bind->save->getInstanceId() == getInstanceId())
                        doUnload = false;
                }

                if (doUnload && hasPermBoundPlayers()) // check if any unloaded players have a nonexpired save to this
                    doUnload = false;
            }

            if (doUnload)
            {
                // the unload timer is not started
                // instead the map will unload immediately after the players have left
                m_unloadWhenEmpty = true;
                m_resetAfterUnload = true;
            }
        }
    }
    else
    {
        // unloaded at next update
        m_unloadTimer = 1;
        m_resetAfterUnload = !(method == INSTANCE_RESET_GLOBAL && hasPermBoundPlayers());
    }

    return (getPlayerCount() == 0);
}

bool InstanceMap::hasPermBoundPlayers()
{
    auto result = CharacterDatabase.Query("SELECT guid FROM character_instance WHERE instance = %u and permanent = 1", getInstanceId());
    if (result)
    {
        return true;
    }
    return false;
}

uint32_t InstanceMap::getMaxPlayers()
{
    DBC::Structures::MapDifficulty const* mapDiff = getMapDifficulty();
    if (mapDiff && mapDiff->maxPlayers)
        return mapDiff->maxPlayers;

#if VERSION_STRING > TBC
    return getBaseMap()->getMapEntry()->max_players;
#else
    return getBaseMap()->getMapInfo()->playerlimit;
#endif
}
