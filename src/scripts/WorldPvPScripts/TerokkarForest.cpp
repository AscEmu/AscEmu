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
constexpr uint32_t TEROKKAR_ZONE_ID = 3518;

// buffs
constexpr uint32_t SPELL_TF_ALLIANCE_BUFF = 34567; 
constexpr uint32_t SPELL_TF_HORDE_BUFF    = 34568;

// WorldStates
constexpr uint32_t WORLDSTATE_TF_TOWER1 = 0x5200;
constexpr uint32_t WORLDSTATE_TF_TOWER2 = 0x5201;
constexpr uint32_t WORLDSTATE_TF_TOWER3 = 0x5202;

// owner values
constexpr uint32_t OWNER_NONE     = 0;
constexpr uint32_t OWNER_ALLIANCE = 1;
constexpr uint32_t OWNER_HORDE    = 2;

////////////////////////////////////////////////////////////////////////////////
// static data
static uint32_t sTowerWorldStates[3] =
{
    WORLDSTATE_TF_TOWER1,
    WORLDSTATE_TF_TOWER2,
    WORLDSTATE_TF_TOWER3
};

static uint32_t sZoneOwner = OWNER_NONE;

////////////////////////////////////////////////////////////////////////////////
// helpers
static uint32_t Terokkar_GetZoneOwner(WorldStatesHandler& worldStates)
{
    uint32_t alliance = 0;
    uint32_t horde = 0;

    for (uint32_t ws : sTowerWorldStates)
    {
        uint32_t owner = worldStates.GetWorldStateForZone(TEROKKAR_ZONE_ID, 0, ws);
        if (owner == OWNER_ALLIANCE)
            ++alliance;
        if (owner == OWNER_HORDE)
            ++horde;
    }

    if (alliance >= 2)
        return OWNER_ALLIANCE;
    if (horde >= 2)
        return OWNER_HORDE;
    return OWNER_NONE;
}

static void Terokkar_RecalculateZoneOwner(WorldMap* map)
{
    if (!map)
        return;

    WorldStatesHandler& worldStates = map->getWorldStatesHandler();
    uint32_t newOwner = Terokkar_GetZoneOwner(worldStates);

    if (newOwner == sZoneOwner)
        return;

    uint32_t oldOwner = sZoneOwner;
    sZoneOwner = newOwner;

    for (auto& itr : map->getPlayers())
    {
        Player* player = itr.second;
        if (!player || player->getZoneId() != TEROKKAR_ZONE_ID)
            continue;

        // remove old buffs
        if (oldOwner == OWNER_ALLIANCE)
            player->removeAllAurasById(SPELL_TF_ALLIANCE_BUFF);
        if (oldOwner == OWNER_HORDE)
            player->removeAllAurasById(SPELL_TF_HORDE_BUFF);

        // apply new buffs
        if (newOwner == OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
            player->castSpell(player, SPELL_TF_ALLIANCE_BUFF, true);
        if (newOwner == OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
            player->castSpell(player, SPELL_TF_HORDE_BUFF, true);
    }
}

////////////////////////////////////////////////////////////////////////////////
// hooks
void Terokkar_OnEnterWorld(Player* player)
{
    if (!player || player->getZoneId() != TEROKKAR_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    Terokkar_RecalculateZoneOwner(map);

    if (sZoneOwner == OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
        player->castSpell(player, SPELL_TF_ALLIANCE_BUFF, true);
    if (sZoneOwner == OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
        player->castSpell(player, SPELL_TF_HORDE_BUFF, true);
}

void Terokkar_OnLogout(Player* player){ }

void Terokkar_OnZoneChange(Player* player, uint32_t newZone, uint32_t oldZone)
{
    if (!player)
        return;

    if (newZone != TEROKKAR_ZONE_ID && oldZone != TEROKKAR_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    Terokkar_RecalculateZoneOwner(map);

    if (newZone == TEROKKAR_ZONE_ID)
    {
        if (sZoneOwner == OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
            player->castSpell(player, SPELL_TF_ALLIANCE_BUFF, true);
        if (sZoneOwner == OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
            player->castSpell(player, SPELL_TF_HORDE_BUFF, true);
    }

    if (oldZone == TEROKKAR_ZONE_ID)
    {
        player->removeAllAurasById(SPELL_TF_ALLIANCE_BUFF);
        player->removeAllAurasById(SPELL_TF_HORDE_BUFF);
    }
}

////////////////////////////////////////////////////////////////////////////////
// setup
void SetupTerokkarForest(ScriptMgr* mgr)
{
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)&Terokkar_OnEnterWorld);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_LOGOUT, (void*)&Terokkar_OnLogout);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ZONE, (void*)&Terokkar_OnZoneChange);
}

