/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "SilvershardMines.hpp"

#include <cmath>

#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Management/HonorHandler.h"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"

namespace
{
    struct SilvershardSegment
    {
        float const (*nodes)[3];
        uint32_t count;
    };

    constexpr int32_t SilvershardAllianceControlStates[SILVERSHARD_NUM_MINES] =
    {
        WORLDSTATE_SILVERSHARD_ALLIANCE_CONTROLS_SOUTH,
        WORLDSTATE_SILVERSHARD_ALLIANCE_CONTROLS_NORTH,
        WORLDSTATE_SILVERSHARD_ALLIANCE_CONTROLS_EAST
    };

    constexpr int32_t SilvershardHordeControlStates[SILVERSHARD_NUM_MINES] =
    {
        WORLDSTATE_SILVERSHARD_HORDE_CONTROLS_SOUTH,
        WORLDSTATE_SILVERSHARD_HORDE_CONTROLS_NORTH,
        WORLDSTATE_SILVERSHARD_HORDE_CONTROLS_EAST
    };

    // Global dummy-spell handler for the real track-switch spellclick spell (see
    // SilvershardMinesDefinitions.hpp - npc 60283 -> spell 124491). Registered once in Setup.cpp;
    // dispatches to whichever SilvershardMines instance the clicking player is actually in.
    bool SilvershardTrackSwitchDummySpell(uint8_t /*effectIndex*/, Spell* pSpell)
    {
        Player* caster = pSpell->getPlayerCaster();
        if (!caster)
            return false;

        Unit* target = pSpell->getUnitTarget();
        if (!target || !target->isCreature())
            return false;

        Battleground* bg = caster->getBattleground();
        if (!bg)
            return false;

        if (auto* silvershard = dynamic_cast<SilvershardMines*>(bg))
            silvershard->OnTrackSwitchClicked(static_cast<Creature*>(target));

        return true;
    }
}

SilvershardMines::SilvershardMines(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    // Silvershard Mines zone id (AreaTable.dbc entry "Silvershard Mines", MapID=727 confirmed matching).
    m_zoneId = 6126;
    m_scores[0] = m_scores[1] = 0;
    m_eastTrackState = SILVERSHARD_TRACK_CLOSED;
    m_northTrackState = SILVERSHARD_TRACK_CLOSED;
    m_trackSwitchEast = nullptr;
    m_trackSwitchNorth = nullptr;

    for (uint8_t i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
    }

    for (uint8_t i = 0; i < SILVERSHARD_NUM_GATES; i++)
        m_gates[i] = nullptr;

    for (uint8_t i = 0; i < SILVERSHARD_NUM_MINES; i++)
    {
        m_carts[i] = nullptr;
        m_cartWaypointIndex[i] = 0;
        m_cartControlledBy[i] = -1;
        m_cartSegment[i] = 0;
    }

    m_pvpData.clear();
    m_resurrectMap.clear();
}

SilvershardMines::~SilvershardMines()
{
}

void SilvershardMines::OnCreate()
{
    for (uint8_t i = 0; i < SILVERSHARD_NUM_GATES; i++)
    {
        uint32_t const entries[SILVERSHARD_NUM_GATES] = { SILVERSHARD_GATE_1_ENTRY, SILVERSHARD_GATE_2_ENTRY, SILVERSHARD_GATE_3_ENTRY, SILVERSHARD_GATE_4_ENTRY };
        float const o = SilvershardGateCoords[i][3];
        m_gates[i] = spawnGameObject(entries[i], LocationVector(SilvershardGateCoords[i][0], SilvershardGateCoords[i][1], SilvershardGateCoords[i][2], o), 0, 0, 1.0f);
        m_gates[i]->setLocalRotation(0.f, 0.f, std::sin(o / 2.0f), std::cos(o / 2.0f));
        m_gates[i]->setAnimationProgress(100);
        m_gates[i]->PushToWorld(m_mapMgr);
    }

    for (uint8_t i = 0; i < SILVERSHARD_NUM_MINES; i++)
    {
        m_carts[i] = spawnCreature(SILVERSHARD_MINE_CART_ENTRY,
            SilvershardCartSpawn[i][0], SilvershardCartSpawn[i][1], SilvershardCartSpawn[i][2], SilvershardCartSpawn[i][3]);
        if (m_carts[i])
            m_carts[i]->castSpell(m_carts[i], SILVERSHARD_SPELL_CONTROL_NEUTRAL, true);
    }

    m_trackSwitchEast = spawnCreature(SILVERSHARD_TRACK_SWITCH_ENTRY,
        SilvershardTrackSwitchEast[0], SilvershardTrackSwitchEast[1], SilvershardTrackSwitchEast[2], SilvershardTrackSwitchEast[3]);
    m_trackSwitchNorth = spawnCreature(SILVERSHARD_TRACK_SWITCH_ENTRY,
        SilvershardTrackSwitchNorth[0], SilvershardTrackSwitchNorth[1], SilvershardTrackSwitchNorth[2], SilvershardTrackSwitchNorth[3]);
}

void SilvershardMines::OnStart()
{
    Battleground::OnStart();

    for (uint8_t i = 0; i < SILVERSHARD_NUM_GATES; i++)
        m_gates[i]->despawn(0, 0);

    setWorldState(WORLDSTATE_SILVERSHARD_ALLIANCE_SCORE, m_scores[TEAM_ALLIANCE]);
    setWorldState(WORLDSTATE_SILVERSHARD_HORDE_SCORE, m_scores[TEAM_HORDE]);
    setWorldState(WORLDSTATE_SILVERSHARD_EASTERN_TRACK_SWITCH, m_eastTrackState);
    setWorldState(WORLDSTATE_SILVERSHARD_NORTHERN_TRACK_SWITCH, m_northTrackState);

    for (uint8_t i = 0; i < SILVERSHARD_NUM_MINES; i++)
    {
        setWorldState(SilvershardAllianceControlStates[i], 0);
        setWorldState(SilvershardHordeControlStates[i], 0);

        sEventMgr.AddEvent(this, &SilvershardMines::EventUpdateCart, static_cast<uint32_t>(i),
            EVENT_AB_RESOURCES_UPDATE_TEAM_0 + i, SILVERSHARD_CART_STEP_INTERVAL, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void SilvershardMines::OnAddPlayer(Player* plr)
{
    plr->castSpell(plr, BattlegroundDef::PREPARATION, true);
    updatePvPData();
}

void SilvershardMines::OnRemovePlayer(Player* plr)
{
    plr->removeAllAurasById(BattlegroundDef::PREPARATION);
}

void SilvershardMines::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;
    updatePvPData();
}

bool SilvershardMines::HookHandleRepop(Player* plr)
{
    LocationVector dest = GetStartingCoords(plr->getTeam());
    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

void SilvershardMines::HookOnMount(Player* /*plr*/)
{
}

void SilvershardMines::HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/)
{
}

void SilvershardMines::HookFlagStand(Player* /*plr*/, GameObject* /*obj*/)
{
}

void SilvershardMines::HookOnFlagDrop(Player* /*plr*/)
{
}

void SilvershardMines::HookOnPlayerKill(Player* plr, Player* /*pVictim*/)
{
    plr->m_bgScore.KillingBlows++;
    updatePvPData();
}

void SilvershardMines::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void SilvershardMines::HookOnAreaTrigger(Player* /*plr*/, uint32_t /*id*/)
{
}

void SilvershardMines::HookOnShadowSight()
{
}

void SilvershardMines::HookGenerateLoot(Player* /*plr*/, Object* /*pCorpse*/)
{
}

void SilvershardMines::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{
}

LocationVector SilvershardMines::GetStartingCoords(uint32_t Team)
{
    // Starting coordinates, sourced from the local Mop client's own WorldSafeLocs.dbc
    // ("STV Diamond Mine - BG - Start - Horde/Alliance"). Orientation is an estimate (facing
    // toward the map center) - not carried by WorldSafeLocs.dbc.
    if (Team)       // Horde
        return LocationVector(630.170166f, 230.173614f, 328.838196f, 5.0f);
    else            // Alliance
        return LocationVector(851.574646f, 135.140625f, 328.838196f, 2.0f);
}

void SilvershardMines::OnTrackSwitchClicked(Creature* trackSwitch)
{
    int32_t worldStateId;
    uint32_t* state;
    char const* label;

    if (trackSwitch == m_trackSwitchEast)
    {
        worldStateId = WORLDSTATE_SILVERSHARD_EASTERN_TRACK_SWITCH;
        state = &m_eastTrackState;
        label = "Eastern";
    }
    else if (trackSwitch == m_trackSwitchNorth)
    {
        worldStateId = WORLDSTATE_SILVERSHARD_NORTHERN_TRACK_SWITCH;
        state = &m_northTrackState;
        label = "Northern";
    }
    else
    {
        return;
    }

    *state = (*state == SILVERSHARD_TRACK_CLOSED) ? SILVERSHARD_TRACK_OPEN : SILVERSHARD_TRACK_CLOSED;
    setWorldState(worldStateId, *state);
    playSoundToAll(BattlegroundDef::FLAG_RETURNED);
    sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The %s Crossroads tracks have been switched!", label);
}

void SilvershardMines::CaptureCart(uint32_t mineIndex, uint32_t Team)
{
    m_scores[Team] += SILVERSHARD_SCORE_CAPTURE;
    if (m_scores[Team] > SILVERSHARD_SCORE_MAX)
        m_scores[Team] = SILVERSHARD_SCORE_MAX;

    setWorldState(Team ? WORLDSTATE_SILVERSHARD_HORDE_SCORE : WORLDSTATE_SILVERSHARD_ALLIANCE_SCORE, m_scores[Team]);

    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    for (auto* player : m_players[Team])
    {
        player->m_bgScore.BonusHonor += m_honorPerKill;
        HonorHandler::AddHonorPointsToPlayer(player, m_honorPerKill);
    }

    if (m_scores[Team] >= SILVERSHARD_SCORE_MAX)
    {
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        this->endBattleground(Team == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE);
        return;
    }

    ResetCart(mineIndex);
}

void SilvershardMines::ResetCart(uint32_t mineIndex)
{
    m_cartWaypointIndex[mineIndex] = 0;
    m_cartSegment[mineIndex] = 0;
    m_cartControlledBy[mineIndex] = -1;

    setWorldState(SilvershardAllianceControlStates[mineIndex], 0);
    setWorldState(SilvershardHordeControlStates[mineIndex], 0);

    if (Creature* cart = m_carts[mineIndex])
    {
        LocationVector const spawn(SilvershardCartSpawn[mineIndex][0], SilvershardCartSpawn[mineIndex][1],
            SilvershardCartSpawn[mineIndex][2], SilvershardCartSpawn[mineIndex][3]);
        cart->SetPosition(spawn);

        // EventUpdateCart only re-casts the control visual when newController differs from
        // m_cartControlledBy - since that was just reset to -1 above, a capture (which leaves the
        // cart still showing the previous team's banner) would never clear it without this.
        cart->removeAllAurasById(SILVERSHARD_SPELL_CONTROL_NEUTRAL);
        cart->removeAllAurasById(SILVERSHARD_SPELL_CONTROL_ALLIANCE);
        cart->removeAllAurasById(SILVERSHARD_SPELL_CONTROL_HORDE);
        cart->castSpell(cart, SILVERSHARD_SPELL_CONTROL_NEUTRAL, true);
    }
}

void SilvershardMines::EventUpdateCart(uint32_t mineIndex)
{
    Creature* cart = m_carts[mineIndex];
    if (!cart)
        return;

    // Control is resolved by an explicit nearby-player faction-majority scan (see
    // SilvershardMinesDefinitions.hpp, this differs from the capture-point gameobject
    // approach), scanning around the cart's current position rather than a fixed point.
    uint32_t allianceCount = 0;
    uint32_t hordeCount = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        for (auto* player : m_players[TEAM_ALLIANCE])
            if (player->CalcDistance(cart->GetPositionX(), cart->GetPositionY(), cart->GetPositionZ()) <= SILVERSHARD_CONTROL_RADIUS)
                allianceCount++;
        for (auto* player : m_players[TEAM_HORDE])
            if (player->CalcDistance(cart->GetPositionX(), cart->GetPositionY(), cart->GetPositionZ()) <= SILVERSHARD_CONTROL_RADIUS)
                hordeCount++;
    }

    int32_t const newController = allianceCount > hordeCount ? static_cast<int32_t>(TEAM_ALLIANCE)
        : hordeCount > allianceCount ? static_cast<int32_t>(TEAM_HORDE)
        : -1;

    if (newController != m_cartControlledBy[mineIndex])
    {
        m_cartControlledBy[mineIndex] = newController;
        setWorldState(SilvershardAllianceControlStates[mineIndex], newController == TEAM_ALLIANCE ? 1 : 0);
        setWorldState(SilvershardHordeControlStates[mineIndex], newController == TEAM_HORDE ? 1 : 0);
        playSoundToAll(newController == TEAM_ALLIANCE ? BattlegroundDef::ALLIANCE_CAPTURE : newController == TEAM_HORDE ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::FLAG_RETURNED);

        cart->removeAllAurasById(SILVERSHARD_SPELL_CONTROL_NEUTRAL);
        cart->removeAllAurasById(SILVERSHARD_SPELL_CONTROL_ALLIANCE);
        cart->removeAllAurasById(SILVERSHARD_SPELL_CONTROL_HORDE);
        cart->castSpell(cart, newController == TEAM_ALLIANCE ? SILVERSHARD_SPELL_CONTROL_ALLIANCE
            : newController == TEAM_HORDE ? SILVERSHARD_SPELL_CONTROL_HORDE : SILVERSHARD_SPELL_CONTROL_NEUTRAL, true);
    }

    if (newController == -1)
        return; // contested or empty - cart holds position

    // Resolve which segment array the cart is currently walking, following the track-switch
    // state at each mine's crossroads (South has no junction and only ever uses segment 0). A
    // lambda rather than a one-shot lookup, since taking the crossroads below can change which
    // segment is "current" mid-tick and the nodes/count must be re-resolved afterward.
    auto resolveSegment = [&]() -> SilvershardSegment
    {
        switch (mineIndex)
        {
            case SILVERSHARD_MINE_SOUTH:
                return { SilvershardSouthRoute, 25 };
            case SILVERSHARD_MINE_NORTH:
                if (m_cartSegment[mineIndex] == 0)
                    return { SilvershardNorthBase, 14 };
                return m_northTrackState == SILVERSHARD_TRACK_OPEN
                    ? SilvershardSegment{ SilvershardNorthEastFork, 39 }
                    : SilvershardSegment{ SilvershardNorthWestFork, 23 };
            case SILVERSHARD_MINE_EAST:
                if (m_cartSegment[mineIndex] == 0)
                    return { SilvershardEastBase, 10 };
                return m_eastTrackState == SILVERSHARD_TRACK_OPEN
                    ? SilvershardSegment{ SilvershardEastSouthFork, 14 }
                    : SilvershardSegment{ SilvershardEastNorthFork, 32 };
            default:
                return {};
        }
    };

    uint32_t& idx = m_cartWaypointIndex[mineIndex];
    SilvershardSegment segment = resolveSegment();

    if (idx >= segment.count)
    {
        if (mineIndex != SILVERSHARD_MINE_SOUTH && m_cartSegment[mineIndex] == 0)
        {
            // Reached the crossroads - move onto whichever fork the track switch currently points
            // to, and re-resolve so the rest of this tick uses the fork's own nodes/count.
            m_cartSegment[mineIndex] = 1;
            idx = 0;
            segment = resolveSegment();
        }
        else
        {
            CaptureCart(mineIndex, static_cast<uint32_t>(newController));
            return;
        }
    }

    if (cart->CalcDistance(segment.nodes[idx][0], segment.nodes[idx][1], segment.nodes[idx][2]) <= 2.0f)
    {
        idx++;
        if (idx >= segment.count)
        {
            // Arrived at the end of this segment. A base segment just hands off to its fork next
            // tick (handled by the crossroads branch above); a final segment means the depot was
            // reached - capture now.
            if (mineIndex == SILVERSHARD_MINE_SOUTH || m_cartSegment[mineIndex] != 0)
                CaptureCart(mineIndex, static_cast<uint32_t>(newController));
            return;
        }
    }

    cart->getMovementManager()->movePoint(idx, segment.nodes[idx][0], segment.nodes[idx][1], segment.nodes[idx][2], false);
}

void SetupSilvershardMines(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(SILVERSHARD_SPELL_TRACK_SWITCH_CLICK, &SilvershardTrackSwitchDummySpell);
}
