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
constexpr uint32_t ZANGARMARSH_ZONE_ID = 3521;

// buffs
constexpr uint32_t SPELL_ZM_ALLIANCE_BUFF = 33795; // Strength of the Storm
constexpr uint32_t SPELL_ZM_HORDE_BUFF = 33796; // Hand of the Warchief

// WorldStates (placeholder IDs)
constexpr uint32_t WORLDSTATE_ZM_PUMP_WEST = 0x5100;
constexpr uint32_t WORLDSTATE_ZM_PUMP_EAST = 0x5101;
constexpr uint32_t WORLDSTATE_ZM_PUMP_SOUTH = 0x5102;

// owner values
constexpr uint32_t OWNER_NONE = 0;
constexpr uint32_t OWNER_ALLIANCE = 1;
constexpr uint32_t OWNER_HORDE = 2;

////////////////////////////////////////////////////////////////////////////////
// static data
static uint32_t sPumpWorldStates[3] =
{
    WORLDSTATE_ZM_PUMP_WEST,
    WORLDSTATE_ZM_PUMP_EAST,
    WORLDSTATE_ZM_PUMP_SOUTH
};

static uint32_t sZoneOwner = OWNER_NONE;

////////////////////////////////////////////////////////////////////////////////
// helpers
static uint32_t Zangarmarsh_GetZoneOwner(WorldStatesHandler& worldStates)
{
    uint32_t alliance = 0;
    uint32_t horde = 0;

    for (uint32_t ws : sPumpWorldStates)
    {
        uint32_t owner = worldStates.GetWorldStateForZone(ZANGARMARSH_ZONE_ID, 0, ws);

        if (owner == OWNER_ALLIANCE)
            ++alliance;
        else if (owner == OWNER_HORDE)
            ++horde;
    }

    if (alliance >= 2)
        return OWNER_ALLIANCE;

    if (horde >= 2)
        return OWNER_HORDE;

    return OWNER_NONE;
}

static void Zangarmarsh_RecalculateZoneOwner(WorldMap* map)
{
    if (!map)
        return;

    WorldStatesHandler& worldStates = map->getWorldStatesHandler();
    uint32_t newOwner = Zangarmarsh_GetZoneOwner(worldStates);

    if (newOwner == sZoneOwner)
        return;

    uint32_t oldOwner = sZoneOwner;
    sZoneOwner = newOwner;

    for (auto& itr : map->getPlayers())
    {
        Player* player = itr.second;
        if (!player || player->getZoneId() != ZANGARMARSH_ZONE_ID)
            continue;

        // remove old buffs
        if (oldOwner == OWNER_ALLIANCE)
            player->removeAllAurasById(SPELL_ZM_ALLIANCE_BUFF);

        if (oldOwner == OWNER_HORDE)
            player->removeAllAurasById(SPELL_ZM_HORDE_BUFF);

        // apply new buffs
        if (newOwner == OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
            player->castSpell(player, SPELL_ZM_ALLIANCE_BUFF, true);

        if (newOwner == OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
            player->castSpell(player, SPELL_ZM_HORDE_BUFF, true);
    }
}

////////////////////////////////////////////////////////////////////////////////
// hooks
void Zangarmarsh_OnEnterWorld(Player* player)
{
    if (!player || player->getZoneId() != ZANGARMARSH_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    Zangarmarsh_RecalculateZoneOwner(map);

    if (sZoneOwner == OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
        player->castSpell(player, SPELL_ZM_ALLIANCE_BUFF, true);

    if (sZoneOwner == OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
        player->castSpell(player, SPELL_ZM_HORDE_BUFF, true);
}

void Zangarmarsh_OnLogout(Player* player){ }

void Zangarmarsh_OnZoneChange(Player* player, uint32_t newZone, uint32_t oldZone)
{
    if (!player)
        return;

    if (newZone != ZANGARMARSH_ZONE_ID && oldZone != ZANGARMARSH_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    Zangarmarsh_RecalculateZoneOwner(map);

    if (newZone == ZANGARMARSH_ZONE_ID)
    {
        if (sZoneOwner == OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
            player->castSpell(player, SPELL_ZM_ALLIANCE_BUFF, true);

        if (sZoneOwner == OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
            player->castSpell(player, SPELL_ZM_HORDE_BUFF, true);
    }

    if (oldZone == ZANGARMARSH_ZONE_ID)
    {
        player->removeAllAurasById(SPELL_ZM_ALLIANCE_BUFF);
        player->removeAllAurasById(SPELL_ZM_HORDE_BUFF);
    }
}

////////////////////////////////////////////////////////////////////////////////
// setup
void SetupZangarmarsh(ScriptMgr* mgr)
{
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)&Zangarmarsh_OnEnterWorld);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_LOGOUT, (void*)&Zangarmarsh_OnLogout);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ZONE, (void*)&Zangarmarsh_OnZoneChange);
}

