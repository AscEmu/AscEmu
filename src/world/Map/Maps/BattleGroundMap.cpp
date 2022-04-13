/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "BattleGroundMap.hpp"
#include "InstanceDefines.hpp"

BattlegroundMap::BattlegroundMap(BaseMap* baseMap, uint32_t id, time_t expiry, uint32_t InstanceId, uint8_t spawnMode)
    : WorldMap(baseMap, id, expiry, InstanceId, spawnMode), m_battleground(nullptr)
{
    //lets initialize visibility distance for Battlegrounds/Arenas
    BattlegroundMap::initVisibilityDistance();
}

BattlegroundMap::~BattlegroundMap()
{
    if (m_battleground)
    {
        sBattlegroundManager.DeleteBattleground(m_battleground);
        m_battleground = nullptr;
    }
}

void BattlegroundMap::update(uint32_t t_diff)
{
    WorldMap::update(t_diff);

    if (isUnloadPending())
        unloadAll();

    if (canUnload(t_diff))
    {
        // Remove all Players
        removeAllPlayers();

        // Add one More Update before we Delete the Map
        sMapMgr.getMapUpdater()->addJob(*this, 10);
        setUnloadPending(true);
    }
}

void BattlegroundMap::initVisibilityDistance()
{
    //init visibility distance for Battlegrounds/Arenas
    m_VisibleDistance = 500 * 500;
}

EnterState BattlegroundMap::cannotEnter(Player* player)
{
    if (player->getWorldMap() == this)
        return CANNOT_ENTER_ALREADY_IN_MAP;

    if (static_cast<uint32_t>(player->GetInstanceID()) != getInstanceId())
        return CANNOT_ENTER_INSTANCE_BIND_MISMATCH;

    return WorldMap::cannotEnter(player);
}

bool BattlegroundMap::addPlayerToMap(Player* player)
{
    return WorldMap::addPlayerToMap(player);
}

void BattlegroundMap::removePlayerFromMap(Player* player)
{
    WorldMap::removePlayerFromMap(player);
}

void BattlegroundMap::setUnload()
{
    m_unloadTimer = 1;
}

void BattlegroundMap::removeAllPlayers()
{
    if (getPlayerCount())
    {
        for (const auto& itr : getPlayers())
        {
            Player* player = itr.second;
            player->safeTeleport(player->getBGEntryMapId(), 0, player->getBGEntryPosition());
        }
    }
}