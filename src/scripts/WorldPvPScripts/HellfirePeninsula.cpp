/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Script/ScriptMgr.hpp"
#include "Server/Script/HookInterfaceDefines.hpp"

#include "Map/Maps/WorldMap.hpp"  
#include "Management/WorldStates.hpp"
#include "Management/WorldStatesHandler.hpp"

#include "Objects/Units/Players/Player.hpp"

#include <cstdint>

////////////////////////////////////////////////////////////////////////////////
// defines
constexpr uint32_t HELLFIRE_ZONE_ID = 3483;
constexpr uint32_t SPELL_HELLFIRE_ALLIANCE_BUFF = 32071; // Honor Hold Favor
constexpr uint32_t SPELL_HELLFIRE_HORDE_BUFF = 32073; // Thrallmar Favor

// WorldStates
constexpr uint32_t WORLDSTATE_TOWER_OVERLOOK = 0x5000;
constexpr uint32_t WORLDSTATE_TOWER_BROKEN_HILL = 0x5001;
constexpr uint32_t WORLDSTATE_TOWER_STADIUM = 0x5002;

// Tower owner values
constexpr uint32_t TOWER_OWNER_NONE = 0;
constexpr uint32_t TOWER_OWNER_ALLIANCE = 1;
constexpr uint32_t TOWER_OWNER_HORDE = 2;

////////////////////////////////////////////////////////////////////////////////
// static data
static uint32_t sTowerWorldStates[3] =
{
    WORLDSTATE_TOWER_OVERLOOK,
    WORLDSTATE_TOWER_BROKEN_HILL,
    WORLDSTATE_TOWER_STADIUM
};

static uint32_t sZoneOwner = TOWER_OWNER_NONE;

////////////////////////////////////////////////////////////////////////////////
// helpers
static uint32_t Hellfire_GetZoneOwner(WorldStatesHandler& worldStates)
{
    uint32_t allianceCount = 0;
    uint32_t hordeCount = 0;

    for (uint32_t worldState : sTowerWorldStates)
    {
        uint32_t owner = worldStates.GetWorldStateForZone(HELLFIRE_ZONE_ID, 0, worldState);

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

static void Hellfire_RecalculateZoneOwner(WorldMap* map)
{
    if (!map)
        return;

    WorldStatesHandler& worldStates = map->getWorldStatesHandler();
    uint32_t newOwner = Hellfire_GetZoneOwner(worldStates);

    if (newOwner == sZoneOwner)
        return;

    uint32_t oldOwner = sZoneOwner;
    sZoneOwner = newOwner;

    for (auto& itr : map->getPlayers())
    {
        Player* player = itr.second;
        if (!player)
            continue;

        if (player->getZoneId() != HELLFIRE_ZONE_ID)
            continue;

        // remove old buffs
        if (oldOwner == TOWER_OWNER_ALLIANCE)
            player->removeAllAurasById(SPELL_HELLFIRE_ALLIANCE_BUFF);

        if (oldOwner == TOWER_OWNER_HORDE)
            player->removeAllAurasById(SPELL_HELLFIRE_HORDE_BUFF);

        // apply new buffs
        if (newOwner == TOWER_OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
        {
            player->castSpell(player, SPELL_HELLFIRE_ALLIANCE_BUFF, true);
        }

        if (newOwner == TOWER_OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
        {
            player->castSpell(player, SPELL_HELLFIRE_HORDE_BUFF, true);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// hooks
void Hellfire_OnEnterWorld(Player* player)
{
    if (!player || player->getZoneId() != HELLFIRE_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    // sch - first check
    Hellfire_RecalculateZoneOwner(map);

    // sch - second check
    uint32_t owner = sZoneOwner;

    if (owner == TOWER_OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
        player->castSpell(player, SPELL_HELLFIRE_ALLIANCE_BUFF, true);

    if (owner == TOWER_OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
        player->castSpell(player, SPELL_HELLFIRE_HORDE_BUFF, true);
}

void Hellfire_OnLogout(Player* player){ }

void Hellfire_OnZoneChange(Player* player, uint32_t newZone, uint32_t oldZone)
{
    if (!player)
        return;

    if (newZone != HELLFIRE_ZONE_ID && oldZone != HELLFIRE_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    uint32_t owner = Hellfire_GetZoneOwner(map->getWorldStatesHandler());

    // in the zone
    if (newZone == HELLFIRE_ZONE_ID)
    {
        if (owner == TOWER_OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
            player->castSpell(player, SPELL_HELLFIRE_ALLIANCE_BUFF, true);

        if (owner == TOWER_OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
            player->castSpell(player, SPELL_HELLFIRE_HORDE_BUFF, true);
    }

    //  left the zone
    if (oldZone == HELLFIRE_ZONE_ID)
    {
        player->removeAllAurasById(SPELL_HELLFIRE_ALLIANCE_BUFF);
        player->removeAllAurasById(SPELL_HELLFIRE_HORDE_BUFF);
    }
}

////////////////////////////////////////////////////////////////////////////////
// setup
void SetupHellfirePeninsula(ScriptMgr* mgr)
{
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)&Hellfire_OnEnterWorld);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_LOGOUT, (void*)&Hellfire_OnLogout);    
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ZONE, (void*)&Hellfire_OnZoneChange);
}

