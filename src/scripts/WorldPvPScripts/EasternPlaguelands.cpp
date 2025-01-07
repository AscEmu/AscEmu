/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Script/ScriptMgr.hpp"
#include "Server/Script/HookInterfaceDefines.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Management/WorldStates.hpp"
#include "Management/WorldStatesHandler.hpp"
#include "Management/HonorHandler.h"

#include "Objects/Units/Players/Player.hpp"

#include <cstdint>

////////////////////////////////////////////////////////////////////////////////
// defines
constexpr uint32_t EPL_ZONE_ID = 139;

// owners
constexpr uint32_t OWNER_NONE = 0;
constexpr uint32_t OWNER_ALLIANCE = 1;
constexpr uint32_t OWNER_HORDE = 2;

// world states
constexpr uint32_t WORLDSTATE_EPL_SHOW_UI = 2357;
constexpr uint32_t WORLDSTATE_EPL_NORTHPASS = 2353;

////////////////////////////////////////////////////////////////////////////////
// static
static uint32_t sTowerStates[1] =
{
    WORLDSTATE_EPL_NORTHPASS
};

static uint32_t sZoneOwner = OWNER_NONE;

////////////////////////////////////////////////////////////////////////////////
// helpers
static uint32_t EPL_GetZoneOwner(WorldStatesHandler& ws)
{
    uint32_t alliance = 0;
    uint32_t horde = 0;

    for (uint32_t state : sTowerStates)
    {
        uint32_t owner = ws.GetWorldStateForZone(EPL_ZONE_ID, 0, state);

        if (owner == OWNER_ALLIANCE)
            ++alliance;
        else if (owner == OWNER_HORDE)
            ++horde;
    }

    if (alliance > 0 && horde == 0)
        return OWNER_ALLIANCE;

    if (horde > 0 && alliance == 0)
        return OWNER_HORDE;

    return OWNER_NONE;
}

static void EPL_RecalculateZoneOwner(WorldMap* map)
{
    if (!map)
        return;

    auto& ws = map->getWorldStatesHandler();
    uint32_t newOwner = EPL_GetZoneOwner(ws);

    if (newOwner == sZoneOwner)
        return;

    sZoneOwner = newOwner;

    for (auto& itr : map->getPlayers())
    {
        Player* player = itr.second;
        if (!player || player->getZoneId() != EPL_ZONE_ID)
            continue;

        // esc buff
    }
}

////////////////////////////////////////////////////////////////////////////////
// hooks
void EPL_OnEnterWorld(Player* player)
{
    if (!player || player->getZoneId() != EPL_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    auto& ws = map->getWorldStatesHandler();

    ws.SetWorldStateForZone(EPL_ZONE_ID, 0, WORLDSTATE_EPL_SHOW_UI, 1);

    EPL_RecalculateZoneOwner(map);
}

////////////////////////////////////////////////////////////////////////////////
// setup
void SetupEasternPlaguelands(ScriptMgr* mgr)
{
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)&EPL_OnEnterWorld);
}

