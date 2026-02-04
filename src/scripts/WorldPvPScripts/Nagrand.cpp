/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Script/ScriptMgr.hpp"
#include "Server/Script/HookInterfaceDefines.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/CreatureAIScript.hpp"

#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Map/Maps/WorldMap.hpp"
#include "Management/WorldStatesHandler.hpp"
#include "Management/WorldStates.hpp"
#include "Objects/GameObject.h"
#include "Server/Master.h"
#include "Server/Opcodes.hpp"
#include "Chat/ChatCommandHandler.hpp"

#include <cstdint>
#include <vector>
#include <cmath>

////////////////////////////////////////////////////////////////////////////////
// zone
constexpr uint32_t NAGRAND_ZONE_ID = 3518;
// constexpr uint32_t NAGRAND_MAP_ID = 530;

// owners
constexpr uint32_t OWNER_NONE = 0;
constexpr uint32_t OWNER_ALLIANCE = 1;
constexpr uint32_t OWNER_HORDE = 2;

// banner/capture point
constexpr uint32_t GO_HALAA_CAPTURE = 182210;

// buffs
constexpr uint32_t SPELL_ALLIANCE_BUFF = 35001;
constexpr uint32_t SPELL_HORDE_BUFF = 35002;

// token spells (honorable kill reward)
constexpr uint32_t SPELL_HALAA_TOKEN_ALLIANCE = 33005;
constexpr uint32_t SPELL_HALAA_TOKEN_HORDE = 33004;

// halaa banner position
constexpr float HALAA_X = -1572.62f;
constexpr float HALAA_Y = 7945.14f;
constexpr float HALAA_Z = -22.2746f;
constexpr float HALAA_O = 2.74889f;

// capture
constexpr float CAPTURE_RADIUS = 50.0f;
constexpr uint32_t CAPTURE_TICK_MS = 2000;
constexpr uint32_t CAPTURE_HALAA_SPEED = 5; // % per tick

// distance for honorable kill token award
constexpr float HALAA_PVP_DISTANCE = 300.0f;

// artkit
constexpr uint32_t ARTKIT_HALAA_HORDE = 1;
constexpr uint32_t ARTKIT_HALAA_ALLIANCE = 2;
constexpr uint32_t ARTKIT_HALAA_NEUTRAL = 21;

// guard npcs
constexpr uint32_t NPC_HALAA_GUARD_HORDE = 18192;
constexpr uint32_t NPC_HALAA_GUARD_ALLIANCE = 18256;
constexpr uint32_t HALAA_GUARD_COUNT = 15;

// global state
static uint32_t sZoneOwner = OWNER_NONE;
static uint32_t sCaptureProgress = 50; // = 50 (neutral)
static uint32_t halaaGuards = 0;

////////////////////////////////////////////////////////////////////////////////
// helpers
static void Nagrand_UpdatePlayersBuff(WorldMap* map)
{
    if (!map)
        return;

    for (auto const& pair : map->getPlayers())
    {
        Player* player = pair.second;
        if (!player || player->getZoneId() != NAGRAND_ZONE_ID)
            continue;

        // remove old buffs
        player->removeAllAurasById(SPELL_ALLIANCE_BUFF);
        player->removeAllAurasById(SPELL_HORDE_BUFF);

        // apply new buff if in owning team
        if (sZoneOwner == OWNER_ALLIANCE && player->GetTeam() == TEAM_ALLIANCE)
            player->castSpell(player, SPELL_ALLIANCE_BUFF, true);
        else if (sZoneOwner == OWNER_HORDE && player->GetTeam() == TEAM_HORDE)
            player->castSpell(player, SPELL_HORDE_BUFF, true);
    }
}

static void Nagrand_RecountHalaaGuards(WorldMap* map)
{
    uint32_t alive = 0;

    for (Creature* c : map->getCreatures())
    {
        if (!c || !c->isAlive())
            continue;

        if (c->getEntry() == NPC_HALAA_GUARD_ALLIANCE || c->getEntry() == NPC_HALAA_GUARD_HORDE)
            ++alive;
    }

    halaaGuards = std::min(alive, HALAA_GUARD_COUNT);
    map->getWorldStatesHandler().SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_GUARD_UI_VALUE, halaaGuards);
}

static void setBannerArtKit(GameObject* go)
{
    if (!go)
        return;

    uint32_t artkit = (sZoneOwner == OWNER_ALLIANCE) ? ARTKIT_HALAA_ALLIANCE : (sZoneOwner == OWNER_HORDE) ? ARTKIT_HALAA_HORDE : ARTKIT_HALAA_NEUTRAL;
    go->setArtKit(artkit);
}

static void Nagrand_RespawnHalaaBanner(WorldMap* map)
{
    if (!map)
        return;

    // remove existing banners
    for (auto* go : map->getGameObjects())
    {
        if (!go)
            continue;
        if (go->getEntry() == GO_HALAA_CAPTURE)
        {
            setBannerArtKit(go);
            if (!go->IsInWorld())
                go->PushToWorld(map);
        }
    }
}

static void Nagrand_InitHalaaWorldStates(WorldMap* map)
{
    if (!map)
        return;

    WorldStatesHandler& ws = map->getWorldStatesHandler();

    ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_NEUTRAL, sZoneOwner == OWNER_NONE ? 100 : 0);
    ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_HORDE, sZoneOwner == OWNER_HORDE ? 100 : 0);
    ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_ALLIANCE, sZoneOwner == OWNER_ALLIANCE ? 100 : 0);

    ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, 0);
    ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, 0);

    if (sZoneOwner != OWNER_NONE)
    {
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_GUARD_UI_VALUE, halaaGuards);
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_GUARDS_TOTAL, HALAA_GUARD_COUNT);
    }
}

////////////////////////////////////////////////////////////////////////////////
// capture progress
static void Nagrand_UpdateCaptureProgress(WorldMap* map, uint32_t capturingTeam)
{
    if (!map)
        return;

    if (sZoneOwner == capturingTeam)
        return;

    WorldStatesHandler& ws = map->getWorldStatesHandler();

    if (sCaptureProgress + CAPTURE_HALAA_SPEED >= 100)
        sCaptureProgress = 100;
    else
        sCaptureProgress += CAPTURE_HALAA_SPEED;

    // set assaulted WorldStates for UI progress
    if (capturingTeam == OWNER_ALLIANCE)
    {
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, sCaptureProgress);
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, 0);
    }
    else if (capturingTeam == OWNER_HORDE)
    {
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, sCaptureProgress);
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, 0);
    }

    // debug log
    // DLLLogDetail("Halaa capture progress: Team=%s, Progress=%u%%", team == OWNER_ALLIANCE ? "Alliance" : "Horde", sCaptureProgress);

    // capture finished
    if (sCaptureProgress >= 100)
    {
        uint32_t lastOwner = sZoneOwner;
        sZoneOwner = capturingTeam;
        sCaptureProgress = 50;

        // update WorldStates to show new owner
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_ALLIANCE, sZoneOwner == OWNER_ALLIANCE ? 100 : 0);
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_HORDE, sZoneOwner == OWNER_HORDE ? 100 : 0);
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_NEUTRAL, sZoneOwner == OWNER_NONE ? 100 : 0);

        // reset assaulted bars
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, 0);
        ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, 0);

        Nagrand_UpdatePlayersBuff(map);
        Nagrand_RespawnHalaaBanner(map);
        Nagrand_RecountHalaaGuards(map);

        // DLLLogDetail("Halaa captured by %s!", team == OWNER_ALLIANCE ? "Alliance" : "Horde");
    }
}

////////////////////////////////////////////////////////////////////////////////
// halaa banner
class HalaaGuardAI : public CreatureAIScript
{
public:
    explicit HalaaGuardAI(Creature* creature) : CreatureAIScript(creature) {}
    static CreatureAIScript* Create(Creature* creature) { return new HalaaGuardAI(creature); }

    void OnLoad() override
    {
        WorldMap* map = getCreature() ? getCreature()->getWorldMap() : nullptr;
        if (!map)
            return;

        if (halaaGuards < HALAA_GUARD_COUNT)
            ++halaaGuards;

        Nagrand_RecountHalaaGuards(map);
    }

    void OnDied(Unit* /*killer*/) override
    {
        WorldMap* map = getCreature() ? getCreature()->getWorldMap() : nullptr;
        if (!map)
            return;

        if (halaaGuards > 0)
            --halaaGuards;

        Nagrand_RecountHalaaGuards(map);
    }
};

////////////////////////////////////////////////////////////////////////////////
// stubs (Wyvern/Roost/BombWagon AI)
class DestroyedWyvernRoostAI : public GameObjectAIScript
{
public:
    explicit DestroyedWyvernRoostAI(GameObject* go) : GameObjectAIScript(go) {}
    static GameObjectAIScript* Create(GameObject* go) { return new DestroyedWyvernRoostAI(go); }
    void OnSpawn() override {}
    void OnActivate(Player* /*player*/) override {}
};

class WyvernRoostAI : public GameObjectAIScript
{
public:
    explicit WyvernRoostAI(GameObject* go) : GameObjectAIScript(go) {}
    static GameObjectAIScript* Create(GameObject* go) { return new WyvernRoostAI(go); }
    void OnSpawn() override {}
    void OnActivate(Player* /*player*/) override {}
};

class BombWagonAI : public GameObjectAIScript
{
public:
    explicit BombWagonAI(GameObject* go) : GameObjectAIScript(go) {}
    static GameObjectAIScript* Create(GameObject* go) { return new BombWagonAI(go); }
    void OnSpawn() override {}
    void OnActivate(Player* /*player*/) override {}
};

////////////////////////////////////////////////////////////////////////////////
// halaa banner GO
class NagrandHalaaBannerGO : public GameObjectAIScript
{
public:
    explicit NagrandHalaaBannerGO(GameObject* go) : GameObjectAIScript(go) {}
    static GameObjectAIScript* Create(GameObject* go) { return new NagrandHalaaBannerGO(go); }

    void OnSpawn() override
    {
        setBannerArtKit(_gameobject);
        RegisterAIUpdateEvent(CAPTURE_TICK_MS);
    }

    void OnDespawn() override
    {
        RemoveAIUpdateEvent();
    }

    void OnActivate(Player* player) override
    {
        if (!player || player->getZoneId() != NAGRAND_ZONE_ID)
            return;

        WorldMap* map = player->getWorldMap();
        if (!map)
            return;

        Nagrand_InitHalaaWorldStates(map);
        Nagrand_RespawnHalaaBanner(map);
    }

    void AIUpdate() override
    {
        if (!_gameobject || !_gameobject->IsInWorld())
            return;

        WorldMap* map = _gameobject->getWorldMap();
        if (!map)
            return;

        uint32_t allianceCount = 0;
        uint32_t hordeCount = 0;
        std::vector<Player*> playersInRange;

        for (auto const& pr : map->getPlayers())
        {
            Player* plr = pr.second;
            if (!plr || plr->getZoneId() != NAGRAND_ZONE_ID)
                continue;

            float dx = plr->GetPositionX() - HALAA_X;
            float dy = plr->GetPositionY() - HALAA_Y;
            float distance = sqrtf(dx*dx + dy*dy);

            if (distance > CAPTURE_RADIUS || plr->IsFlying())
            {
                plr->sendWorldStateUpdate(WORLDSTATE_NA_ALLIANCE_UI_SHOW, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HORDE_UI_SHOW, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, 0);
            }

            if (plr->isAlive() && !plr->isInvisible() && !plr->isStealthed() && plr->isPvpFlagSet())
            {
                if (plr->GetTeam() == TEAM_ALLIANCE)
                    ++allianceCount;
                else
                    ++hordeCount;

                playersInRange.push_back(plr);
            }
        }

        if (playersInRange.empty())
        {
            WorldStatesHandler& ws = map->getWorldStatesHandler();
            ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, 0);
            ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, 0);
            return;
        }

        if (allianceCount > 0 && hordeCount == 0)
            Nagrand_UpdateCaptureProgress(map, OWNER_ALLIANCE);
        else if (hordeCount > 0 && allianceCount == 0)
            Nagrand_UpdateCaptureProgress(map, OWNER_HORDE);
        else
        {
            WorldStatesHandler& ws = map->getWorldStatesHandler();
            ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, 0);
            ws.SetWorldStateForZone(NAGRAND_ZONE_ID, 0, WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, 0);

            for (Player* plr : playersInRange)
            {
                plr->sendWorldStateUpdate(WORLDSTATE_NA_ALLIANCE_UI_SHOW, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HORDE_UI_SHOW, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, 0);
            }
            return;
        }

        for (Player* plr : playersInRange)
        {
            if (!plr)
                continue;

            if (sZoneOwner == OWNER_ALLIANCE || sZoneOwner == OWNER_HORDE)
            {
                plr->sendWorldStateUpdate(WORLDSTATE_NA_ALLIANCE_UI_SHOW, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HORDE_UI_SHOW, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, 0);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, 0);
            }

            if (allianceCount > 0 && hordeCount == 0)
            {
                plr->sendWorldStateUpdate(WORLDSTATE_NA_ALLIANCE_UI_SHOW, 1);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HALAA_ALLIANCE_ASSAULTED, sCaptureProgress);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HORDE_UI_SHOW, 0);
            }
            else if (hordeCount > 0 && allianceCount == 0)
            {
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HORDE_UI_SHOW, 1);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_HALAA_HORDE_ASSAULTED, sCaptureProgress);
                plr->sendWorldStateUpdate(WORLDSTATE_NA_ALLIANCE_UI_SHOW, 0);
            }
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// hooks
static void Nagrand_OnPlayerEnterWorld(Player* player)
{
    if (!player || player->getZoneId() != NAGRAND_ZONE_ID)
        return;

    WorldMap* map = player->getWorldMap();
    if (!map)
        return;

    Nagrand_InitHalaaWorldStates(map);
    Nagrand_RespawnHalaaBanner(map);
    Nagrand_UpdatePlayersBuff(map);
}

static void Nagrand_OnZoneChange(Player* player, uint32_t newZone, uint32_t oldZone)
{
    if (!player)
        return;

    if (oldZone == NAGRAND_ZONE_ID)
    {
        player->removeAllAurasById(SPELL_ALLIANCE_BUFF);
        player->removeAllAurasById(SPELL_HORDE_BUFF);
    }

    if (newZone == NAGRAND_ZONE_ID)
    {
        WorldMap* map = player->getWorldMap();
        if (map)
            Nagrand_UpdatePlayersBuff(map);
    }
}

static void Nagrand_OnLogout(Player* player)
{
    if (!player)
        return;
    player->removeAllAurasById(SPELL_ALLIANCE_BUFF);
    player->removeAllAurasById(SPELL_HORDE_BUFF);
}

static void Nagrand_OnHonorableKill(Player* killer, Player* victim)
{
    if (!killer || !victim)
        return;
    if (killer->getZoneId() != NAGRAND_ZONE_ID)
        return;

    float dx = killer->GetPositionX() - HALAA_X;
    float dy = killer->GetPositionY() - HALAA_Y;
    float dz = killer->GetPositionZ() - HALAA_Z;
    float dist2 = dx*dx + dy*dy + dz*dz;
    if (dist2 > HALAA_PVP_DISTANCE * HALAA_PVP_DISTANCE)
        return;

    if (killer->GetTeam() == TEAM_ALLIANCE)
        killer->castSpell(killer, SPELL_HALAA_TOKEN_ALLIANCE, false);
    else
        killer->castSpell(killer, SPELL_HALAA_TOKEN_HORDE, false);
}

////////////////////////////////////////////////////////////////////////////////
// setup
void SetupNagrand(ScriptMgr* mgr)
{
    // hooks
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)&Nagrand_OnPlayerEnterWorld);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ZONE, (void*)&Nagrand_OnZoneChange);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_LOGOUT, (void*)&Nagrand_OnLogout);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_HONORABLE_KILL, (void*)&Nagrand_OnHonorableKill);

    // go script
    mgr->register_gameobject_script(GO_HALAA_CAPTURE, &NagrandHalaaBannerGO::Create);

    // guard creatures
    mgr->register_creature_script(NPC_HALAA_GUARD_ALLIANCE, &HalaaGuardAI::Create);
    mgr->register_creature_script(NPC_HALAA_GUARD_HORDE, &HalaaGuardAI::Create);

    // stubs (Wyvern/Roost/BombWagon AI)
    // mgr->register_gameobject_script(WYVERN_ROOST_ID, &WyvernRoostAI::Create);
    // mgr->register_gameobject_script(DESTROYED_ROOST_ID, &DestroyedWyvernRoostAI::Create);
    // mgr->register_gameobject_script(BOMB_WAGON_ID, &BombWagonAI::Create);
}

