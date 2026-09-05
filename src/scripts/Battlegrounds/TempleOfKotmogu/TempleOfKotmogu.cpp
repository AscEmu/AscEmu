/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TempleOfKotmogu.hpp"

#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Management/HonorHandler.h"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellMgr.hpp"

namespace
{
    constexpr uint32_t KotmoguOrbEntries[KOTMOGU_NUM_ORBS] = { KOTMOGU_ORB_1_ENTRY, KOTMOGU_ORB_2_ENTRY, KOTMOGU_ORB_3_ENTRY, KOTMOGU_ORB_4_ENTRY };
    constexpr uint32_t KotmoguOrbPickupSpells[KOTMOGU_NUM_ORBS] = { KOTMOGU_SPELL_ORB_PICKED_UP_1, KOTMOGU_SPELL_ORB_PICKED_UP_2, KOTMOGU_SPELL_ORB_PICKED_UP_3, KOTMOGU_SPELL_ORB_PICKED_UP_4 };

    // Spawn coordinates.
    constexpr float KotmoguOrbCoords[KOTMOGU_NUM_ORBS][4] =
    {
        { 1716.78f, 1416.64f, 13.5709f, 1.57239f },
        { 1850.26f, 1416.77f, 13.5709f, 1.56061f },
        { 1850.29f, 1250.31f, 13.5708f, 4.70848f },
        { 1716.83f, 1249.93f, 13.5706f, 4.71397f },
    };
}

TempleOfKotmogu::TempleOfKotmogu(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    // Temple of Kotmogu zone id (AreaTable.dbc entry "Temple of Kotmogu", MapID=998 confirmed matching).
    m_zoneId = 6051;
    m_scores[0] = m_scores[1] = 0;
    m_orbsHeld[0] = m_orbsHeld[1] = 0;

    for (uint8_t i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
        m_doors[i] = nullptr;
    }

    for (uint8_t i = 0; i < KOTMOGU_NUM_ORBS; i++)
    {
        m_orbs[i] = nullptr;
        m_orbCarrier[i] = 0;
    }

    m_pvpData.clear();
    m_resurrectMap.clear();
}

TempleOfKotmogu::~TempleOfKotmogu()
{
}

void TempleOfKotmogu::OnCreate()
{
    // Door coordinates.
    m_doors[TEAM_ALLIANCE] = spawnGameObject(KOTMOGU_DOOR_ENTRY, LocationVector(1783.84f, 1100.66f, 20.60f, 1.625020f), 0, 0, 2.5f);
    m_doors[TEAM_ALLIANCE]->setAnimationProgress(100);
    m_doors[TEAM_ALLIANCE]->PushToWorld(m_mapMgr);

    m_doors[TEAM_HORDE] = spawnGameObject(KOTMOGU_DOOR_ENTRY, LocationVector(1780.15f, 1570.22f, 24.59f, 4.711630f), 0, 0, 2.5f);
    m_doors[TEAM_HORDE]->setAnimationProgress(100);
    m_doors[TEAM_HORDE]->PushToWorld(m_mapMgr);

    for (uint8_t i = 0; i < KOTMOGU_NUM_ORBS; i++)
    {
        m_orbs[i] = spawnGameObject(KotmoguOrbEntries[i],
            LocationVector(KotmoguOrbCoords[i][0], KotmoguOrbCoords[i][1], KotmoguOrbCoords[i][2], KotmoguOrbCoords[i][3]), 0, 0, 1.0f);
        m_orbs[i]->setState(GO_STATE_CLOSED);
        m_orbs[i]->setGoType(GAMEOBJECT_TYPE_FLAGSTAND);
        m_orbs[i]->setAnimationProgress(100);
        m_orbs[i]->PushToWorld(m_mapMgr);
    }

    addSpiritGuide(spawnSpiritGuide(1892.61f, 1151.69f, 14.7160f, 2.523528f, TEAM_ALLIANCE));
    addSpiritGuide(spawnSpiritGuide(1672.40f, 1524.10f, 16.7387f, 6.032206f, TEAM_HORDE));
}

void TempleOfKotmogu::OnStart()
{
    Battleground::OnStart();

    for (uint8_t i = 0; i < 2; i++)
        m_doors[i]->despawn(0, 0);

    setWorldState(WORLDSTATE_KOTMOGU_MAX_SCORE, KOTMOGU_MAX_TEAM_SCORE);
    setWorldState(WORLDSTATE_KOTMOGU_ALLIANCE_SCORE, m_scores[TEAM_ALLIANCE]);
    setWorldState(WORLDSTATE_KOTMOGU_HORDE_SCORE, m_scores[TEAM_HORDE]);
}

void TempleOfKotmogu::OnAddPlayer(Player* plr)
{
    plr->castSpell(plr, BattlegroundDef::PREPARATION, true);
    updatePvPData();
}

void TempleOfKotmogu::OnRemovePlayer(Player* plr)
{
    plr->removeAllAurasById(BattlegroundDef::PREPARATION);
}

void TempleOfKotmogu::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;

    // Blizzard rule: a player can carry only one orb at a time - dropping it on death respawns
    // it immediately at its own pad.
    if (plr->hasBgFlag())
    {
        for (uint8_t i = 0; i < KOTMOGU_NUM_ORBS; i++)
        {
            if (m_orbCarrier[i] == plr->getGuid())
            {
                plr->removeAllAurasById(KotmoguOrbPickupSpells[i]);
                ReturnOrb(i);

                if (m_orbsHeld[plr->getTeam()] > 0)
                    m_orbsHeld[plr->getTeam()]--;
                EventUpdateResources(plr->getTeam());
                break;
            }
        }
        plr->setHasBgFlag(false);
    }

    updatePvPData();
}

void TempleOfKotmogu::ReturnOrb(uint32_t orbIndex)
{
    m_orbCarrier[orbIndex] = 0;
    playSoundToAll(KOTMOGU_SOUND_ORB);
    m_orbs[orbIndex]->PushToWorld(m_mapMgr);
}

bool TempleOfKotmogu::HookHandleRepop(Player* plr)
{
    // Per-team graveyard coordinates, sourced from the local Mop client's own WorldSafeLocs.dbc
    // ("Valley of Power - BG - Graveyard - Horde/Alliance (Rectangle)").
    LocationVector dest = plr->getTeam()
        ? LocationVector(1675.798584f, 1522.343750f, 16.763117f, 0.0f)
        : LocationVector(1889.371582f, 1156.541626f, 15.076625f, 0.0f);
    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

void TempleOfKotmogu::HookOnMount(Player* /*plr*/)
{
}

void TempleOfKotmogu::HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/)
{
}

void TempleOfKotmogu::HookFlagStand(Player* plr, GameObject* obj)
{
    if (plr->hasBgFlag())
        return;

    for (uint8_t i = 0; i < KOTMOGU_NUM_ORBS; i++)
    {
        if (m_orbs[i] != obj || m_orbCarrier[i] != 0)
            continue;

        SpellInfo const* sp = sSpellMgr.getSpellInfo(KotmoguOrbPickupSpells[i]);
        Spell* s = sSpellMgr.newSpell(plr, sp, true, 0);
        SpellCastTargets targets(plr->getGuid());
        s->prepare(&targets);

        plr->setHasBgFlag(true);
        m_orbCarrier[i] = plr->getGuid();
        m_orbs[i]->RemoveFromWorld(false);

        m_orbsHeld[plr->getTeam()]++;
        EventUpdateResources(plr->getTeam());

        playSoundToAll(KOTMOGU_SOUND_ORB);
        break;
    }
}

void TempleOfKotmogu::HookOnFlagDrop(Player* /*plr*/)
{
}

void TempleOfKotmogu::HookOnPlayerKill(Player* plr, Player* pVictim)
{
    plr->m_bgScore.KillingBlows++;

    // Killing an orb carrier awards the killer's team bonus points (real Blizzard mechanic).
    if (pVictim->hasBgFlag())
    {
        m_scores[plr->getTeam()] += KOTMOGU_PLAYER_KILL_POINTS;
        setWorldState(plr->isTeamHorde() ? WORLDSTATE_KOTMOGU_HORDE_SCORE : WORLDSTATE_KOTMOGU_ALLIANCE_SCORE, m_scores[plr->getTeam()]);
    }

    updatePvPData();
}

void TempleOfKotmogu::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void TempleOfKotmogu::HookOnAreaTrigger(Player* /*plr*/, uint32_t /*id*/)
{
}

void TempleOfKotmogu::HookOnShadowSight()
{
}

void TempleOfKotmogu::HookGenerateLoot(Player* /*plr*/, Object* /*pCorpse*/)
{
}

void TempleOfKotmogu::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{
}

LocationVector TempleOfKotmogu::GetStartingCoords(uint32_t Team)
{
    // Starting coordinates, sourced from the local Mop client's own WorldSafeLocs.dbc
    // ("Valley of Power - BG - Start - Horde/Alliance"). Orientation is an estimate (facing
    // toward the map center) - not carried by WorldSafeLocs.dbc.
    if (Team)       // Horde
        return LocationVector(1780.423584f, 1598.723999f, 33.782162f, 4.0f);
    else            // Alliance
        return LocationVector(1784.265625f, 1077.633667f, 28.878117f, 2.0f);
}

void TempleOfKotmogu::EventUpdateResources(uint32_t Team)
{
    const uint32_t heldOrbs = m_orbsHeld[Team] > KOTMOGU_NUM_ORBS ? KOTMOGU_NUM_ORBS : m_orbsHeld[Team];

    if (heldOrbs == 0)
    {
        event_RemoveEvents(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Team);
        return;
    }

    m_scores[Team] += KotmoguTickPoints[heldOrbs];
    if (m_scores[Team] > KOTMOGU_MAX_TEAM_SCORE)
        m_scores[Team] = KOTMOGU_MAX_TEAM_SCORE;

    setWorldState(Team ? WORLDSTATE_KOTMOGU_HORDE_SCORE : WORLDSTATE_KOTMOGU_ALLIANCE_SCORE, m_scores[Team]);

    if (m_scores[Team] >= KOTMOGU_MAX_TEAM_SCORE)
    {
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        this->endBattleground(Team == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE);
        return;
    }

    if (event_HasEvent(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Team))
        event_ModifyTime(EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Team, KotmoguTickIntervals[heldOrbs]);
    else
        sEventMgr.AddEvent(this, &TempleOfKotmogu::EventUpdateResources, Team, EVENT_AB_RESOURCES_UPDATE_TEAM_0 + Team,
            KotmoguTickIntervals[heldOrbs], 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}
