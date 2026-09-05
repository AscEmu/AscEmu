/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "BattleForGilneas.hpp"

#include "Chat/ChatDefines.hpp"
#include "Management/HonorHandler.h"
#include "Management/WorldStates.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Server/Master.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Spell/Spell.hpp"
#include "Utilities/CommonTime.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

// Real map-761 buff spawn points, one per node, each cycling between its own real Food/Speed/
// Berserk entries (matching Arathi Basin's own random-buff-type-per-spawn-point idiom) - all
// spawn-confirmed via a Mop-era reference database's own gameobject table.
static uint32_t buffentries[BFG_NUM_CONTROL_POINTS][3] =
{
    { 180144, 180379, 180380 },        // LIGHTHOUSE
    { 180383, 180384, 180148 },        // WATERWORKS
    { 180145, 180381, 180382 },        // MINE
};

// Real map-761 control point coordinates (spawn-confirmed). No dedicated graveyard record was
// found separate from the control point itself, so the spirit guide spawns at the same spot,
// the same relationship Arathi Basin's own graveyards have to its control points being close by.
static float GraveyardLocations[BFG_NUM_CONTROL_POINTS][3] =
{
    { 1057.74f, 1278.26f, 3.17937f },        // LIGHTHOUSE
    { 980.033f, 948.738f, 12.7354f },        // WATERWORKS
    { 1251.01f, 958.269f, 5.66847f },        // MINE
};

// Not independently sourced - close to each side's real gate cluster, the same relationship
// Warsong Gulch/Twin Peaks' own (sourced) starting coords have to their flag stands.
static float NoBaseGYLocations[2][3] =
{
    { 918.299f, 1320.0f, 20.455f },          // ALLIANCE
    { 1395.96f, 995.0f, -13.7897f },         // HORDE
};

static const char* ControlPointNames[BFG_NUM_CONTROL_POINTS] =
{
    "Lighthouse",
    "Waterworks",
    "Mine",
};

// Only the NEUTRAL entry differs per node - the same pattern Arathi Basin's own
// ControlPointGoIds table uses (its 4 assault/controlled entries are identical across all 5 of
// its nodes too). The assault/controlled entries here are Arathi Basin's own real, generic
// banner props reused; the neutral entries are Gilneas' own real per-node banners
// (Lighthouse/Waterworks/Mine Banner).
static uint32_t ControlPointGoIds[BFG_NUM_CONTROL_POINTS][BFG_NUM_SPAWN_TYPES] =
{
    // NEUTRAL    ALLIANCE-ATTACK    HORDE-ATTACK    ALLIANCE-CONTROLLED    HORDE_CONTROLLED
    { 205557,       180085,            180086,         180076,                180078 },        // LIGHTHOUSE
    { 205555,       180085,            180086,         180076,                180078 },        // WATERWORKS
    { 205556,       180085,            180086,         180076,                180078 },        // MINE
};

static float ControlPointCoordinates[BFG_NUM_CONTROL_POINTS][4] =
{
    { 1057.74f, 1278.26f, 3.17937f, 4.90981f },        // LIGHTHOUSE
    { 980.033f, 948.738f, 12.7354f, 5.87746f },        // WATERWORKS
    { 1251.01f, 958.269f, 5.66847f, 5.86425f },        // MINE
};

static float ControlPointRotations[BFG_NUM_CONTROL_POINTS][2] =
{
    { 0.633979f, -0.773350f },        // LIGHTHOUSE
    { 0.201474f, -0.979494f },        // WATERWORKS
    { 0.207939f, -0.978142f },        // MINE
};

static float BuffCoordinates[BFG_NUM_CONTROL_POINTS][4] =
{
    { 1063.56f, 1308.98f, 4.91169f, 4.03171f },        // LIGHTHOUSE
    { 990.297f, 983.373f, 12.9826f, 4.55531f },        // WATERWORKS
    { 1195.73f, 1020.3f, 7.97874f, 5.77704f },         // MINE
};

static float BuffRotations[BFG_NUM_CONTROL_POINTS][2] =
{
    { 0.902585f, -0.430511f },        // LIGHTHOUSE
    { 0.760406f, -0.649448f },        // WATERWORKS
    { 0.250380f, -0.968148f },        // MINE
};

static uint32_t AssaultFields[BFG_NUM_CONTROL_POINTS][2] =
{
    { WORLDSTATE_BFG_CAPTURING_LIGHTHOUSE_ALLIANCE, WORLDSTATE_BFG_CAPTURING_LIGHTHOUSE_HORDE },
    { WORLDSTATE_BFG_CAPTURING_WATERWORKS_ALLIANCE, WORLDSTATE_BFG_CAPTURING_WATERWORKS_HORDE },
    { WORLDSTATE_BFG_CAPTURING_MINE_ALLIANCE, WORLDSTATE_BFG_CAPTURING_MINE_HORDE },
};

static uint32_t OwnedFields[BFG_NUM_CONTROL_POINTS][2] =
{
    { WORLDSTATE_BFG_CAPTURED_LIGHTHOUSE_ALLIANCE, WORLDSTATE_BFG_CAPTURED_LIGHTHOUSE_HORDE },
    { WORLDSTATE_BFG_CAPTURED_WATERWORKS_ALLIANCE, WORLDSTATE_BFG_CAPTURED_WATERWORKS_HORDE },
    { WORLDSTATE_BFG_CAPTURED_MINE_ALLIANCE, WORLDSTATE_BFG_CAPTURED_MINE_HORDE },
};

static uint32_t NeutralFields[BFG_NUM_CONTROL_POINTS] =
{
    WORLDSTATE_BFG_SHOW_LIGHTHOUSE_ICON,
    WORLDSTATE_BFG_SHOW_WATERWORKS_ICON,
    WORLDSTATE_BFG_SHOW_MINE_ICON,
};

// Indexed by number of bases currently held (0-3, Gilneas only has 3 nodes). Re-derived from
// Arathi Basin's own curve, tuned up since there are fewer possible bases to race with -
// pacing/timing, not sourced from a real client value.
static uint32_t ResourceUpdateIntervals[4] =
{
    0,
    10000, // 10 seconds
    6000, // 6 seconds
    2000, // 2 seconds
};

static uint32_t PointBonusPerUpdate[4] =
{
    0,
    10,
    10,
    20,
};

// End BG Data
//////////////////////////////////////////////////////////////////////////////////////////

static uint32_t resourcesToGainBH = 260;
static uint32_t resourcesToGainBR = 160;

void BattleForGilneas::SpawnBuff(uint32_t x)
{
    uint32_t chosen_buffid = buffentries[x][Util::getRandomUInt(2)];
    auto gameobject_info = sMySQLStore.getGameObjectProperties(chosen_buffid);
    if (gameobject_info == nullptr)
        return;

    if (m_buffs[x] == nullptr)
    {
        m_buffs[x] = spawnGameObject(chosen_buffid, LocationVector(BuffCoordinates[x][0], BuffCoordinates[x][1], BuffCoordinates[x][2],
            BuffCoordinates[x][3]), 0, 114, 1);

        m_buffs[x]->setLocalRotation(0.f, 0.f, BuffRotations[x][0], BuffRotations[x][1]);
        m_buffs[x]->setState(GO_STATE_CLOSED);
        m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
        m_buffs[x]->setAnimationProgress(100);
        m_buffs[x]->PushToWorld(m_mapMgr);
    }
    else
    {
        if (m_buffs[x]->IsInWorld())
            m_buffs[x]->RemoveFromWorld(false);

        if (chosen_buffid != m_buffs[x]->getEntry())
        {
            m_buffs[x]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
            m_buffs[x]->setEntry(chosen_buffid);
            m_buffs[x]->SetGameObjectProperties(gameobject_info);
        }

        m_buffs[x]->PushToWorld(m_mapMgr);
    }
}

void BattleForGilneas::SpawnControlPoint(uint32_t Id, uint32_t Type)
{
    auto gameobject_info = sMySQLStore.getGameObjectProperties(ControlPointGoIds[Id][Type]);
    if (gameobject_info == nullptr)
        return;

    auto gi_aura = gameobject_info->raw.parameter_3 ? sMySQLStore.getGameObjectProperties(gameobject_info->raw.parameter_3) : nullptr;

    if (m_controlPoints[Id] == nullptr)
    {
        m_controlPoints[Id] = spawnGameObject(gameobject_info->entry, LocationVector(ControlPointCoordinates[Id][0], ControlPointCoordinates[Id][1],
            ControlPointCoordinates[Id][2], ControlPointCoordinates[Id][3]), 0, 35, 1.0f);

        m_controlPoints[Id]->setLocalRotation(0.f, 0.f, ControlPointRotations[Id][0], ControlPointRotations[Id][1]);
        m_controlPoints[Id]->setState(GO_STATE_CLOSED);
        m_controlPoints[Id]->setGoType(static_cast<uint8_t>(gameobject_info->type));
        m_controlPoints[Id]->setAnimationProgress(100);
        m_controlPoints[Id]->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
        m_controlPoints[Id]->setDisplayId(gameobject_info->display_id);

        switch (Type)
        {
            case BFG_SPAWN_TYPE_ALLIANCE_ASSAULT:
            case BFG_SPAWN_TYPE_ALLIANCE_CONTROLLED:
                m_controlPoints[Id]->SetFaction(2);
                break;

            case BFG_SPAWN_TYPE_HORDE_ASSAULT:
            case BFG_SPAWN_TYPE_HORDE_CONTROLLED:
                m_controlPoints[Id]->SetFaction(1);
                break;

            default:
                m_controlPoints[Id]->SetFaction(35);        // neutral
                break;
        }

        m_controlPoints[Id]->PushToWorld(m_mapMgr);
    }
    else
    {
        if (m_controlPoints[Id]->IsInWorld())
            m_controlPoints[Id]->RemoveFromWorld(false);

        m_controlPoints[Id]->SetNewGuid(m_mapMgr->generateGameobjectGuid());
        m_controlPoints[Id]->setEntry(gameobject_info->entry);
        m_controlPoints[Id]->setDisplayId(gameobject_info->display_id);
        m_controlPoints[Id]->setGoType(static_cast<uint8_t>(gameobject_info->type));

        switch (Type)
        {
            case BFG_SPAWN_TYPE_ALLIANCE_ASSAULT:
            case BFG_SPAWN_TYPE_ALLIANCE_CONTROLLED:
                m_controlPoints[Id]->SetFaction(2);
                break;

            case BFG_SPAWN_TYPE_HORDE_ASSAULT:
            case BFG_SPAWN_TYPE_HORDE_CONTROLLED:
                m_controlPoints[Id]->SetFaction(1);
                break;

            default:
                m_controlPoints[Id]->SetFaction(35);        // neutral
                break;
        }

        m_controlPoints[Id]->SetGameObjectProperties(gameobject_info);
        m_controlPoints[Id]->PushToWorld(m_mapMgr);
    }

    if (gi_aura == nullptr)
        return;

    // aura companion GO, same idiom as Arathi Basin's own control points - not separately
    // spawned here since no aura ring GO was independently confirmed for Gilneas' own banners.
}

void BattleForGilneas::OnCreate()
{
    // Alliance Gate (2-piece, same as the real map-761 data shows)
    GameObject* gate = spawnGameObject(205496, LocationVector(918.299f, 1336.49f, 20.455f, 2.82743f), 32, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.987688f, 0.156436f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(207177, LocationVector(918.391f, 1336.64f, 27.4252f, 2.84488f), 32, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.989015f, 0.147813f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    // Horde Gate (2-piece)
    gate = spawnGameObject(205495, LocationVector(1395.96f, 977.257f, -13.7897f, 6.26573f), 32, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.008728f, -0.999962f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(207178, LocationVector(1395.97f, 977.09f, 7.63597f, 6.27446f), 32, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.004363f, -0.999990f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    // spawn (default) control points
    SpawnControlPoint(BFG_CONTROL_POINT_LIGHTHOUSE, BFG_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(BFG_CONTROL_POINT_WATERWORKS, BFG_SPAWN_TYPE_NEUTRAL);
    SpawnControlPoint(BFG_CONTROL_POINT_MINE, BFG_SPAWN_TYPE_NEUTRAL);

    // spawn buffs
    SpawnBuff(BFG_CONTROL_POINT_LIGHTHOUSE);
    SpawnBuff(BFG_CONTROL_POINT_WATERWORKS);
    SpawnBuff(BFG_CONTROL_POINT_MINE);

    // spawn the h/a base spirit guides
    addSpiritGuide(spawnSpiritGuide(NoBaseGYLocations[0][0], NoBaseGYLocations[0][1], NoBaseGYLocations[0][2], 0.0f, 0));
    addSpiritGuide(spawnSpiritGuide(NoBaseGYLocations[1][0], NoBaseGYLocations[1][1], NoBaseGYLocations[1][2], 0.0f, 1));

    setWorldState(WORLDSTATE_BFG_MAX_SCORE, BFG_RESOURCES_WINVAL);
}

void BattleForGilneas::OnStart()
{
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (auto* player : m_players[i])
            player->removeAllAurasById(BattlegroundDef::PREPARATION);
    }

    for (auto* gate : m_gates)
    {
        gate->setFlags(GO_FLAG_TRIGGERED);
        gate->setState(GO_STATE_OPEN);
    }

    playSoundToAll(BattlegroundDef::BATTLEGROUND_BEGIN);

    m_hasStarted = true;
}

BattleForGilneas::BattleForGilneas(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    for (uint8_t i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
    }

    m_pvpData.clear();
    m_resurrectMap.clear();

    for (uint8_t i = 0; i < BFG_NUM_CONTROL_POINTS; ++i)
    {
        m_buffs[i] = nullptr;
        m_controlPoints[i] = nullptr;
        m_spiritGuides[i] = nullptr;
        m_basesAssaultedBy[i] = -1;
        m_basesOwnedBy[i] = -1;
        m_basesLastOwnedBy[i] = -1;
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        m_resources[i] = 0;
        m_capturedBases[i] = 0;
        m_lastHonorGainResources[i] = 0;
        m_lastRepGainResources[i] = 0;
        m_nearingVictory[i] = false;
    }

    m_lgroup = lgroup;

    for (uint8_t i = 0; i < BFG_NUM_CONTROL_POINTS; ++i)
    {
        DefFlag[i][0] = false;
        DefFlag[i][1] = true;
    }

    // Real Battle for Gilneas zone id (AreaTable.dbc entry "The Battle for Gilneas", MapID=761
    // confirmed matching).
    m_zoneId = 5449;
}

BattleForGilneas::~BattleForGilneas()
{
    for (uint8_t i = 0; i < BFG_NUM_CONTROL_POINTS; ++i)
    {
        if (m_buffs[i] != nullptr)
        {
            if (!m_buffs[i]->IsInWorld())
                delete m_buffs[i];
        }

        if (m_controlPoints[i] != nullptr)
        {
            if (!m_controlPoints[i]->IsInWorld())
            {
                delete m_controlPoints[i];
                m_controlPoints[i] = nullptr;
            }
        }

        if (m_spiritGuides[i])
        {
            if (!m_spiritGuides[i]->IsInWorld())
                delete m_spiritGuides[i];
        }
    }

    for (auto* gate : m_gates)
    {
        if (gate != nullptr && !gate->IsInWorld())
            delete gate;
    }

    m_resurrectMap.clear();
}

/*! Handles end of battleground rewards (marks etc)
 *  \param winningTeam Team that won the battleground
 *  \returns True if Battleground class should finish applying rewards, false if we handled it fully */
bool BattleForGilneas::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    // Reused from Arathi Basin's own reward spells - not independently confirmed as Gilneas'
    // own reward set.
    castSpellOnTeam(winningTeam, 43484);
    castSpellOnTeam(winningTeam, 69153);
    castSpellOnTeam(winningTeam, 69499);
    castSpellOnTeam(winningTeam, 69500);
    return true;
}

void BattleForGilneas::EventUpdateResources(uint32_t Team)
{
    uint32_t resource_fields[2] = { WORLDSTATE_BFG_ALLIANCE_RESOURCES, WORLDSTATE_BFG_HORDE_RESOURCES };

    uint32_t current_resources = m_resources[Team];
    uint32_t current_bases = m_capturedBases[Team];

    if (current_bases > BFG_NUM_CONTROL_POINTS)
        current_bases = BFG_NUM_CONTROL_POINTS;

    current_resources += (PointBonusPerUpdate[current_bases]);

    if (current_resources == m_resources[Team])
        return;

    if (current_resources > BFG_RESOURCES_WINVAL)
        current_resources = BFG_RESOURCES_WINVAL;

    m_resources[Team] = current_resources;

    // Honor only - no faction rep grant here, unlike Arathi Basin's own (The Defilers/League of
    // Arathor), since no Gilneas-specific reputation faction was independently confirmed.
    if ((current_resources - m_lastHonorGainResources[Team]) >= resourcesToGainBH)
    {
        uint32_t honorToAdd = m_honorPerKill;
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        for (auto* player : m_players[Team])
        {
            player->m_bgScore.BonusHonor += honorToAdd;
            HonorHandler::AddHonorPointsToPlayer(player, honorToAdd);
        }

        updatePvPData();
        m_lastHonorGainResources[Team] += resourcesToGainBH;
    }

    setWorldState(resource_fields[Team], current_resources);

    if (current_resources >= BFG_RESOURCES_WARNING_THRESHOLD && !m_nearingVictory[Team])
    {
        m_nearingVictory[Team] = true;
        sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, static_cast<uint64_t>(0), "The %s has gathered %u resources and is nearing victory!", Team ? "Horde" : "Alliance", current_resources);
        uint32_t sound = BattlegroundDef::ALLIANCE_BGALMOSTEND - Team;
        playSoundToAll(sound);
    }

    if (current_resources == BFG_RESOURCES_WINVAL)
    {
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        this->endBattleground(Team == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE);
    }
}

void BattleForGilneas::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;
    updatePvPData();
}

void BattleForGilneas::HookOnMount(Player* /*plr*/)
{
    // nothing in this BG
}

void BattleForGilneas::HookOnPlayerKill(Player* plr, Player* /*pVictim*/)
{
    plr->m_bgScore.KillingBlows++;
    updatePvPData();
}

void BattleForGilneas::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void BattleForGilneas::OnAddPlayer(Player* plr)
{
    if (!m_hasStarted && plr->IsInWorld())
    {
        plr->castSpell(plr, BattlegroundDef::PREPARATION, true);
        plr->m_bgScore.MiscData[BattlegroundDef::AB_BASES_ASSAULTED] = 0;
        plr->m_bgScore.MiscData[BattlegroundDef::AB_BASES_CAPTURED] = 0;
    }
    updatePvPData();
}

void BattleForGilneas::OnRemovePlayer(Player* plr)
{
    plr->removeAllAurasById(BattlegroundDef::PREPARATION);
}

void BattleForGilneas::HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/)
{
    // nothing - not a flag BG
}

void BattleForGilneas::HookFlagStand(Player* /*plr*/, GameObject* /*obj*/)
{
    // nothing - not a flag BG
}

// Not independently sourced (no explicit "start point" record found) - derived close to each
// side's real gate cluster.
LocationVector BattleForGilneas::GetStartingCoords(uint32_t Team)
{
    if (Team)
        return LocationVector(1396.0f, 995.0f, -14.0f, AscEmu::Math::PiF);

    return LocationVector(918.0f, 1320.0f, 20.0f, 0.0f);
}

void BattleForGilneas::HookOnAreaTrigger(Player* plr, uint32_t trigger)
{
    int32_t buffslot = -1;
    switch (trigger)
    {
        case 6265:            // waterworks buff area (real map-761 AreaTrigger.dbc entry)
            buffslot = BFG_CONTROL_POINT_WATERWORKS;
            break;
        case 6268:            // mine buff area
            buffslot = BFG_CONTROL_POINT_MINE;
            break;
        case 6269:            // lighthouse buff area
            buffslot = BFG_CONTROL_POINT_LIGHTHOUSE;
            break;
        case 6266:
        case 6267:
        case 6447:
        case 6448:
            // Real map-761 AreaTrigger.dbc entries not tied to a buff - boundary/encounter
            // triggers, no-op the same way Arathi Basin's own Trollbane Hall/Defiler's Den ones are.
            return;
        default:
            DLLLogDetail("Encountered unhandled areatrigger id {}", trigger);
            return;
    }

    if (plr->isDead())
        return;

    if (buffslot >= 0 && m_buffs[buffslot] && m_buffs[buffslot]->IsInWorld())
    {
        auto spellid = m_buffs[buffslot]->GetGameObjectProperties()->raw.parameter_3;
        m_buffs[buffslot]->RemoveFromWorld(false);

        sEventMgr.AddEvent(this, &BattleForGilneas::SpawnBuff, static_cast<uint32_t>(buffslot), EVENT_AB_RESPAWN_BUFF, BFG_BUFF_RESPAWN_TIME, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        const auto sp = sSpellMgr.getSpellInfo(spellid);
        if (sp)
        {
            Spell* pSpell = sSpellMgr.newSpell(plr, sp, true, nullptr);
            SpellCastTargets targets(plr->getGuid());
            pSpell->prepare(&targets);
        }
    }
}

bool BattleForGilneas::HookHandleRepop(Player* plr)
{
    LocationVector dest(NoBaseGYLocations[plr->getBgTeam()][0], NoBaseGYLocations[plr->getBgTeam()][1], NoBaseGYLocations[plr->getBgTeam()][2], 0.0f);
    float current_distance = 999999.0f;
    float dist;

    for (uint8_t i = 0; i < BFG_NUM_CONTROL_POINTS; ++i)
    {
        if (m_basesOwnedBy[i] == static_cast<int32_t>(plr->getBgTeam()))
        {
            dist = plr->GetPositionV()->distance2DSq({ GraveyardLocations[i][0], GraveyardLocations[i][1] });
            if (dist < current_distance)
            {
                current_distance = dist;
                dest.changeCoords({ GraveyardLocations[i][0], GraveyardLocations[i][1], GraveyardLocations[i][2] });
            }
        }
    }

    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

void BattleForGilneas::CaptureControlPoint(uint32_t Id, uint32_t Team)
{
    if (m_basesOwnedBy[Id] != -1)
        return;

    if (m_basesAssaultedBy[Id] != static_cast<int32_t>(Team))
        return;

    m_basesOwnedBy[Id] = Team;
    m_basesAssaultedBy[Id] = -1;
    m_basesLastOwnedBy[Id] = -1;

    if (m_spiritGuides[Id] != nullptr)
    {
        removeSpiritGuide(m_spiritGuides[Id]);
        m_spiritGuides[Id]->Despawn(0, 0);
    }

    m_spiritGuides[Id] = spawnSpiritGuide(GraveyardLocations[Id][0], GraveyardLocations[Id][1], GraveyardLocations[Id][2], 0.0f, Team);
    addSpiritGuide(m_spiritGuides[Id]);

    playSoundToAll(Team ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::ALLIANCE_CAPTURE);
    sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, 0, "The %s has taken the %s!", Team ? "Horde" : "Alliance", ControlPointNames[Id]);
    DefFlag[Id][0] = false;
    DefFlag[Id][1] = false;

    m_capturedBases[Team]++;
    setWorldState(Team ? WORLDSTATE_BFG_HORDE_OCCUPIED_BASES : WORLDSTATE_BFG_ALLIANCE_OCCUPIED_BASES, m_capturedBases[Team]);

    SpawnControlPoint(Id, Team ? BFG_SPAWN_TYPE_HORDE_CONTROLLED : BFG_SPAWN_TYPE_ALLIANCE_CONTROLLED);

    setWorldState(AssaultFields[Id][Team], 0);
    setWorldState(OwnedFields[Id][Team], 1);

    if (m_capturedBases[Team] == 1)
    {
        sEventMgr.AddEvent(this, &BattleForGilneas::EventUpdateResources, static_cast<uint32_t>(Team), EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Team, ResourceUpdateIntervals[1], 0,
            EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        event_ModifyTime(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Team, ResourceUpdateIntervals[m_capturedBases[Team]]);
    }
}

void BattleForGilneas::AssaultControlPoint(Player* pPlayer, uint32_t Id)
{
    if (!m_hasStarted)
    {
        sCheatLog.writefromsession(pPlayer->getSession(), "{} attempted to assault a Battle for Gilneas control point before the battleground started (battleground ID: {}).", pPlayer->getName(), this->m_id);
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, pPlayer->getGuid(), "%s attempted to assault a Battle for Gilneas control point before the battleground started and will be removed for cheating.", pPlayer->getName().c_str());
        removePlayer(pPlayer, false);
        pPlayer->kickFromServer(6000);
        return;
    }

    uint32_t Team = pPlayer->getBgTeam();
    uint32_t Owner;

    pPlayer->m_bgScore.MiscData[BattlegroundDef::AB_BASES_ASSAULTED]++;

    if (m_basesOwnedBy[Id] == -1 && m_basesAssaultedBy[Id] == -1)
        setWorldState(NeutralFields[Id], 0);

    if (m_basesOwnedBy[Id] != -1)
    {
        Owner = m_basesOwnedBy[Id];

        m_basesOwnedBy[Id] = -1;
        m_basesLastOwnedBy[Id] = Owner;

        if (m_spiritGuides[Id] != nullptr)
        {
            auto itr = m_resurrectMap.find(m_spiritGuides[Id]);
            if (itr != m_resurrectMap.end())
            {
                for (uint32_t guid : itr->second)
                {
                    Player* r_plr = m_mapMgr->getPlayer(guid);
                    if (r_plr != nullptr && r_plr->isDead())
                        HookHandleRepop(r_plr);
                }
            }
            m_resurrectMap.erase(itr);
            m_spiritGuides[Id]->Despawn(0, 0);
            m_spiritGuides[Id] = nullptr;
        }

        m_capturedBases[Owner] -= 1;
        setWorldState(Owner ? WORLDSTATE_BFG_HORDE_OCCUPIED_BASES : WORLDSTATE_BFG_ALLIANCE_OCCUPIED_BASES, m_capturedBases[Owner]);

        setWorldState(OwnedFields[Id][Owner], 0);

        if (m_capturedBases[Owner] == 0)
            this->event_RemoveEvents(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Owner);
        else
            this->event_ModifyTime(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Owner, ResourceUpdateIntervals[m_capturedBases[Owner]]);
    }

    if (m_basesAssaultedBy[Id] != -1)
    {
        Owner = m_basesAssaultedBy[Id];

        m_basesAssaultedBy[Id] = -1;
        setWorldState(AssaultFields[Id][Owner], 0);

        sEventMgr.RemoveEvents(this, EVENT_AB_CAPTURE_CP_1 + Id);
        if (m_basesLastOwnedBy[Id] == static_cast<int32_t>(Team))
        {
            m_basesAssaultedBy[Id] = static_cast<int32_t>(Team);
            CaptureControlPoint(Id, Team);
            return;
        }
    }

    m_basesAssaultedBy[Id] = Team;

    SpawnControlPoint(Id, Team ? BFG_SPAWN_TYPE_HORDE_ASSAULT : BFG_SPAWN_TYPE_ALLIANCE_ASSAULT);

    setWorldState(AssaultFields[Id][Team], 1);

    if (DefFlag[Id][0] && !DefFlag[Id][1])
    {
        DefFlag[Id][0] = false;
        sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, pPlayer->getGuid(), "%s defend %s", pPlayer->getName().c_str(), ControlPointNames[Id]);
        sEventMgr.AddEvent(this, &BattleForGilneas::CaptureControlPoint, Id, Team, EVENT_AB_CAPTURE_CP_1 + Id, 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        pPlayer->m_bgScore.MiscData[BattlegroundDef::AB_BASES_CAPTURED]++;
        updatePvPData();
    }
    else if (!DefFlag[Id][0] && !DefFlag[Id][1])
    {
        DefFlag[Id][0] = true;
        sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, pPlayer->getGuid(), "%s assault %s !", pPlayer->getName().c_str(), ControlPointNames[Id]);
        playSoundToAll(Team ? 8212 : 8174);
        sEventMgr.AddEvent(this, &BattleForGilneas::CaptureControlPoint, Id, Team, EVENT_AB_CAPTURE_CP_1 + Id, TimeVarsMs::Minute, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        pPlayer->m_bgScore.MiscData[BattlegroundDef::AB_BASES_ASSAULTED]++;
        updatePvPData();
    }
    else
    {
        DefFlag[Id][0] = true;
        sendChatMessage(Team ? CHAT_MSG_BG_EVENT_HORDE : CHAT_MSG_BG_EVENT_ALLIANCE, pPlayer->getGuid(), "%s claims the %s! If left unchallenged, the %s will control it in 1 minute!",
                                                                                                          pPlayer->getName().c_str(), ControlPointNames[Id], Team ? "Horde" : "Alliance");
        playSoundToAll(8192);
        sEventMgr.AddEvent(this, &BattleForGilneas::CaptureControlPoint, Id, Team, EVENT_AB_CAPTURE_CP_1 + Id, TimeVarsMs::Minute, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

bool BattleForGilneas::HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* /*pSpell*/)
{
    uint32_t cpid = 0;
    for (cpid = 0; cpid < BFG_NUM_CONTROL_POINTS; cpid++)
    {
        if (m_controlPoints[cpid] == nullptr)
            continue;
        if (m_controlPoints[cpid]->getGuid() == pGo->getGuid())
            break;
    }

    if (cpid == BFG_NUM_CONTROL_POINTS)
        return false;

    if (pPlayer->isStealthed() || pPlayer->isInvisible())
        return false;

    AssaultControlPoint(pPlayer, cpid);
    return true;
}

void BattleForGilneas::HookOnShadowSight()
{}
void BattleForGilneas::HookGenerateLoot(Player* /*plr*/, Object* /*pOCorpse*/)
{}

void BattleForGilneas::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void BattleForGilneas::HookOnFlagDrop(Player* /*plr*/)
{}

void BattleForGilneas::SetIsWeekend(bool isweekend)
{
    if (isweekend)
    {
        resourcesToGainBH = 160;
        resourcesToGainBR = 150;
    }
    else
    {
        resourcesToGainBH = 260;
        resourcesToGainBR = 160;
    }
}
