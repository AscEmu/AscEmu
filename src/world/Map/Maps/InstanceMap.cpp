/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "InstanceMap.hpp"

#include "InstanceMgr.hpp"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Definitions.h"
#include "Server/Opcodes.hpp"
#include "Server/Packets/SmsgInstanceLockWarningQuery.h"
#include "Server/Packets/SmsgInstanceSaveCreated.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

InstanceMap::InstanceMap(BaseMap* baseMap, uint32_t id, uint32_t expiry, uint32_t InstanceId, uint8_t SpawnMode, PlayerTeam InstanceTeam)
    : WorldMap(baseMap, id, expiry, InstanceId, SpawnMode), instanceTeam(InstanceTeam)
{
    // WorldMap
    m_unloadTimer = expiry;

    pInstance = this;

    //lets initialize visibility distance for Instance
    InstanceMap::initVisibilityDistance();
    syncVisibilitySubscriptionRadius();
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
        spawnMgr_->deleteRespawnTimes();
    }

    WorldMap::unloadAll(onShutdown);
}

void InstanceMap::initVisibilityDistance()
{
    //init visibility distance for instances
    setVisibilityDistance(500.0f);
}

void InstanceMap::permBindAllPlayers()
{
    if (!getBaseMap()->isInstanceMap())
        return;

    InstanceSaved* save = sInstanceMgr.getInstanceSave(getInstanceId());
    if (!save)
    {
        sLogger.failure("Cannot bind players to instance map (Name: {}, Entry: {}, Difficulty: {}, ID: {}) because no instance save is available!", getBaseMap()->getMapName(), getBaseMap()->getMapId(), getDifficulty(), getInstanceId());
        return;
    }

    thread_local std::vector<Player*> s_players;
    registry_->snapshotPlayers(s_players);

    // perm bind all players that are currently inside the instance
    registry_->forEachPinned(s_players, [&](Player& player)
        {
            // never instance bind GMs with GM mode enabled
            if (player.isGMFlagSet())
                return;

            InstancePlayerBind* bind = player.getBoundInstance(save->getMapId(), save->getDifficulty());
            if (bind && bind->perm)
            {
                if (bind->save && bind->save->getInstanceId() != save->getInstanceId())
                {
                    sLogger.failure("Player ({}, Name: {}) is in instance map (Name: {}, Entry: {}, Difficulty: {}, ID: {}) that is being bound, but already has a save for the map on ID {}!", player.getGuidLow(), player.getName(), getBaseMap()->getMapName(), save->getMapId(), save->getDifficulty(), save->getInstanceId(), bind->save->getInstanceId());
                }
                else if (!bind->save)
                {
                    sLogger.failure("Player ({}, Name: {}) is in instance map (Name: {}, Entry: {}, Difficulty: {}, ID: {}) that is being bound, but already has a bind (without associated save) for the map!", player.getGuidLow(), player.getName(), getBaseMap()->getMapName(), save->getMapId(), save->getDifficulty(), save->getInstanceId());
                }
            }
            else
            {
                player.bindToInstance(save, true);
                SmsgInstanceSaveCreated managedPacket;
                player.getSession()->sendManagedPacket(managedPacket);
#if VERSION_STRING > TBC
                player.getSession()->sendCalendarRaidLockout(save, true);
#endif

                // if group leader is in instance, group also gets bound
                if (const auto group = player.getGroup())
                    if (group->GetLeader()->guid == player.getGuidLow())
                        group->bindToInstance(save, true);
            }
        });
}

bool InstanceMap::addPlayerToMap(Player* player)
{
    if (getBaseMap()->isInstanceMap())
    {
        const auto group = player->getGroup();

        // get or create an instance save for the map
        InstanceSaved* mapSave = sInstanceMgr.getInstanceSave(getInstanceId());
        if (!mapSave)
        {
            sLogger.debug("creating instance save for map {} spawnmode {} with instance id {}", getBaseMap()->getMapId(), getSpawnMode(), getInstanceId());
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
                    sLogger.debug("player {} {} is permanently bound to instance {} {}, {}, {}, {}, {}, {} but is being put into instance {} {}, {}, {}, {}, {}, {}", player->getName(), player->getGuid(), getBaseMap()->getMapName(), playerBind->save->getMapId(), playerBind->save->getInstanceId(), playerBind->save->getDifficulty(), playerBind->save->getPlayerCount(), playerBind->save->getGroupCount(), playerBind->save->canReset(), getBaseMap()->getMapName(), mapSave->getMapId(), mapSave->getInstanceId(), mapSave->getDifficulty(), mapSave->getPlayerCount(), mapSave->getGroupCount(), mapSave->canReset());
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
                        sLogger.debug("player {} {} is being put into instance {} {}, {}, {}, {}, {}, {} but is in a group and is bound to instance {}, {}, {}, {}, {}, {}!", player->getName(), player->getGuid(), getBaseMap()->getMapName(), mapSave->getMapId(), mapSave->getInstanceId(), mapSave->getDifficulty(), mapSave->getPlayerCount(), mapSave->getGroupCount(), mapSave->canReset(), playerBind->save->getMapId(), playerBind->save->getInstanceId(), playerBind->save->getDifficulty(), playerBind->save->getPlayerCount(), playerBind->save->getGroupCount(), playerBind->save->canReset());
                        if (groupBind)
                            sLogger.debug("the group is bound to the instance {} {}, {}, {}, {}, {}, {}", getBaseMap()->getMapName(), groupBind->save->getMapId(), groupBind->save->getInstanceId(), groupBind->save->getDifficulty(), groupBind->save->getPlayerCount(), groupBind->save->getGroupCount(), groupBind->save->canReset());
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
                            sLogger.debug("player {} {} is being put into instance {}, {}, {} but his group is bound to instance {}, {}, {}! \n", player->getName(), player->getGuid(), mapSave->getMapId(), mapSave->getInstanceId(), mapSave->getDifficulty(), groupBind->save->getMapId(), groupBind->save->getInstanceId(), groupBind->save->getDifficulty());
                            sLogger.debug("MapSave players: {}, group count: {} \n", mapSave->getPlayerCount(), mapSave->getGroupCount());
                            if (groupBind->save)
                                sLogger.debug("GroupBind save players: {}, group count: {}", groupBind->save->getPlayerCount(), groupBind->save->getGroupCount());
                            else
                                sLogger.debug("GroupBind save NULL");
                            return false;
                        }
                        // if the group/leader is permanently bound to the instance
                        // players also become permanently bound when they enter
                        if (groupBind->perm)
                        {
                            SmsgInstanceLockWarningQuery managedPacket{ uint32_t(getScript() ? getScript()->getCompletedEncounterMask() : 0) };
                            player->getSession()->sendManagedPacket(managedPacket);
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

        // Admission succeeded. Only now commit bookkeeping that should not be
        // changed by a rejected bind/mismatch attempt.
        if (!group || !group->isLFGGroup())
            player->addInstanceEnterTime(getInstanceId(), Util::getTimeNow());

        // for normal instances cancel the reset schedule when the
        // first player enters (no players yet)
        setResetSchedule(false);

        sLogger.info("Player '{}' entered instance '{}' of map '{}'.", player->getName(), getInstanceId(), getBaseMap()->getMapName());
    }

    // Disable unload only after the map-specific admission path has succeeded.
    m_unloadTimer = 0;
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
            sInstanceMgr.addResetEvent(on, save->getResetTime(), InstanceMgr::InstResetEvent(0, getBaseMap()->getMapId(), InstanceDifficulty::Difficulties(getSpawnMode()), static_cast<uint16_t>(getInstanceId())));
    }
}

void InstanceMap::sendResetWarnings(uint32_t timeLeft)
{
    const uint32_t mapId = getBaseMap()->getMapId();
    const bool     isRaid = getBaseMap()->isRaid();

    thread_local std::vector<Player*> s_players;
    registry_->snapshotPlayers(s_players);

    registry_->forEachPinned(s_players, [&](Player& player)
        {
            if (player.getWorldMap() != this)
                return;

            player.sendInstanceResetWarning(mapId, player.getDifficulty(isRaid), timeLeft, /*raidWarning=*/false);
        });
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

    // A player already transferring from another map may not enter during an
    // active encounter. A login restore is allowed to reattach to its own run.
    if (player->IsInWorld() && getBaseMap()->isRaid() && isCombatInProgress())
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

    auto const& encounters = sObjectMgr.getDungeonEncounterList(getBaseMap()->getMapId(), getDifficulty());
    if (!encounters)
        return;

    uint32_t bossNumber = static_cast<uint32_t>(encounters->size());

    if (!getScript()->getEncounterCount())
        getScript()->setBossNumber(bossNumber);

    if (load)
    {
        auto result = CharacterDatabase.query("SELECT data, completedEncounters FROM instance WHERE map = %u AND id = %u", getBaseMap()->getMapId(), getInstanceId());

        if (result)
        {
            Field* fields = result->fetch();
            std::string data = fields[0].asCString();

            getScript()->setCompletedEncountersMask(fields[1].asUint32());
            if (!data.empty())
            {
                sLogger.debug("Loading instance data for `{}` with id {}", getBaseMap()->getMapName(), getInstanceId());
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
    const uint32_t mapId = getBaseMap()->getMapId();
    const InstanceDifficulty::Difficulties difficulty = getDifficulty();

    if (getPlayerCount())
    {
        thread_local std::vector<Player*> s_players;
        registry_->snapshotPlayers(s_players);

        if (method == INSTANCE_RESET_ALL || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            // notify the players to leave the instance so it can be reset
            registry_->forEachPinned(s_players, [&](Player& p)
                {
                    if (p.getWorldMap() != this) return;
                    p.sendResetFailedNotify(mapId);
                });
        }
        else
        {
            bool doUnload = true;

            if (method == INSTANCE_RESET_GLOBAL)
            {
                // set the homebind timer for players inside (1 minute)
                registry_->forEachPinned(s_players, [&](Player& p)
                    {
                        if (p.getWorldMap() != this) return;

                        if (auto* bind = p.getBoundInstance(mapId, difficulty))
                        {
                            if (bind->extendState && bind->save && bind->save->getInstanceId() == getInstanceId())
                                doUnload = false;
                        }
                    });

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
    auto result = CharacterDatabase.query("SELECT guid FROM character_instance WHERE instance = %u and permanent = 1", getInstanceId());
    if (result)
    {
        return true;
    }
    return false;
}

uint32_t InstanceMap::getMaxPlayers()
{
    WDB::Structures::MapDifficulty const* mapDiff = getMapDifficulty();
    if (mapDiff && mapDiff->maxPlayers)
        return mapDiff->maxPlayers;

#if VERSION_STRING > TBC
    return getBaseMap()->getMapEntry()->maxPlayers;
#else
    return getBaseMap()->getMapInfo()->playerlimit;
#endif
}

PlayerTeam InstanceMap::getTeamIdInInstance() { return instanceTeam; }
uint32_t InstanceMap::getTeamInInstance() { return instanceTeam == TEAM_ALLIANCE ? ALLIANCE : HORDE; }
