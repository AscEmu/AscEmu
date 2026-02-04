/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Script/ScriptMgr.hpp"
#include "Server/Script/HookInterfaceDefines.hpp"

#include "Map/Maps/WorldMap.hpp"
#include <Management/WorldStates.hpp>
#include <Management/WorldStatesHandler.hpp>
#include <Management/HonorHandler.h>

#include <Objects/Units/Players/Player.hpp>

#include <cstdint>

////////////////////////////////////////////////////////////////////////////////
// defines
constexpr uint32_t SILITHUS_ZONE_ID = 1377; // Silithus
constexpr uint32_t SPELL_SILITHUS_ALLIANCE_BUFF = 39199; // Example
constexpr uint32_t SPELL_SILITHUS_HORDE_BUFF = 39201; // Example

// world states)
constexpr uint32_t WORLDSTATE_SILO_TOWER_1 = 0x6000;
constexpr uint32_t WORLDSTATE_SILO_TOWER_2 = 0x6001;

// tower owner values
constexpr uint32_t TOWER_OWNER_NONE = 0;
constexpr uint32_t TOWER_OWNER_ALLIANCE = 1;
constexpr uint32_t TOWER_OWNER_HORDE = 2;

////////////////////////////////////////////////////////////////////////////////
// static data
static uint32_t sSilithusWorldStates[2] =
{
    WORLDSTATE_SILO_TOWER_1,
    WORLDSTATE_SILO_TOWER_2
};

static uint32_t sSilithusZoneOwner = TOWER_OWNER_NONE;

////////////////////////////////////////////////////////////////////////////////
// helpers
static uint32_t Silithus_GetZoneOwner(WorldStatesHandler& worldStates)
{
    uint32_t allianceCount = 0;
    uint32_t hordeCount = 0;

    for (uint32_t worldState : sSilithusWorldStates)
    {
        uint32_t owner = worldStates.GetWorldStateForZone(SILITHUS_ZONE_ID, 0, worldState);

        switch (owner)
        {
            case TOWER_OWNER_ALLIANCE: 
                ++allianceCount;
                break;
            case TOWER_OWNER_HORDE:
                ++hordeCount;
                break;
            default:
                break;
        }
    }

    if (allianceCount > hordeCount)
        return TOWER_OWNER_ALLIANCE;
    if (hordeCount > allianceCount)
        return TOWER_OWNER_HORDE;
    return TOWER_OWNER_NONE;
}

static void Silithus_RecalculateZoneOwner(WorldMap* map)
{
    if (!map)
        return;

    WorldStatesHandler& worldStates = map->getWorldStatesHandler();
    uint32_t newOwner = Silithus_GetZoneOwner(worldStates);

    if (newOwner == sSilithusZoneOwner)
        return;

    uint32_t oldOwner = sSilithusZoneOwner;
    sSilithusZoneOwner = newOwner;

    for (auto& itr : map->getPlayers())
    {
        Player* player = itr.second;
        if (!player)
            continue;
        if (player->getZoneId() != SILITHUS_ZONE_ID)
            continue;

        // remove old buffs
        if (oldOwner == TOWER_OWNER_ALLIANCE)
            player->removeAllAurasById(SPELL_SILITHUS_ALLIANCE_BUFF);

        if (oldOwner == TOWER_OWNER_HORDE)
            player->removeAllAurasById(SPELL_SILITHUS_HORDE_BUFF);

        // apply new buffs
        if (newOwner == TOWER_OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
            player->castSpell(player, SPELL_SILITHUS_ALLIANCE_BUFF, true);

        if (newOwner == TOWER_OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
            player->castSpell(player, SPELL_SILITHUS_HORDE_BUFF, true);
    }
}

////////////////////////////////////////////////////////////////////////////////
// hooks
void Silithus_OnEnterWorld(Player* player)
{
    if (!player || player->getZoneId() != SILITHUS_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    Silithus_RecalculateZoneOwner(map);

    uint32_t owner = sSilithusZoneOwner;
    if (owner == TOWER_OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
        player->castSpell(player, SPELL_SILITHUS_ALLIANCE_BUFF, true);
    if (owner == TOWER_OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
        player->castSpell(player, SPELL_SILITHUS_HORDE_BUFF, true);
}

void Silithus_OnZoneChange(Player* player, uint32_t newZone, uint32_t oldZone)
{
    if (!player)
        return;

    if (newZone != SILITHUS_ZONE_ID && oldZone != SILITHUS_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    uint32_t owner = Silithus_GetZoneOwner(map->getWorldStatesHandler());

    // entered zone
    if (newZone == SILITHUS_ZONE_ID)
    {
        if (owner == TOWER_OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
            player->castSpell(player, SPELL_SILITHUS_ALLIANCE_BUFF, true);

        if (owner == TOWER_OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
            player->castSpell(player, SPELL_SILITHUS_HORDE_BUFF, true);
    }

    // left zone
    if (oldZone == SILITHUS_ZONE_ID)
    {
        player->removeAllAurasById(SPELL_SILITHUS_ALLIANCE_BUFF);
        player->removeAllAurasById(SPELL_SILITHUS_HORDE_BUFF);
    }
}

////////////////////////////////////////////////////////////////////////////////
// setup
void SetupSilithus(ScriptMgr* mgr)
{
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)&Silithus_OnEnterWorld);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ZONE, (void*)&Silithus_OnZoneChange);
}

