/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TwinPeaks.hpp"

#include "Chat/ChatDefines.hpp"
#include "Management/HonorHandler.h"
#include "Management/WorldStates.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Server/Master.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Spell/SpellMgr.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Spell/Spell.hpp"
#include "Utilities/MathConstants.hpp"

TwinPeaks::TwinPeaks(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    // Real Twin Peaks zone id (AreaTable.dbc entry "Twin Peaks", MapID=726 confirmed matching).
    m_zoneId = 5031;
    m_scores[0] = m_scores[1] = 0;
    m_time_left = TP_TIME_LEFT;

    for (uint8_t i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
    }

    m_pvpData.clear();
    m_resurrectMap.clear();

    m_flagHolders[0] = m_flagHolders[1] = 0;
    m_lgroup = lgroup;

    // create the buffs
    for (uint8_t i = 0; i < 6; ++i)
        SpawnBuff(i);

    // team index 0 = Horde, 1 = Alliance, matching WarsongGulch's own convention
    m_homeFlags[0] = spawnGameObject(179831, LocationVector(1578.34f, 344.045f, 2.41841f, 2.79252f), 0, 210, 2.5f);
    m_homeFlags[0]->setState(GO_STATE_CLOSED);
    m_homeFlags[0]->setGoType(GAMEOBJECT_TYPE_FLAGSTAND);
    m_homeFlags[0]->setAnimationProgress(100);

    m_homeFlags[1] = spawnGameObject(179830, LocationVector(2117.64f, 191.682f, 44.052f, 6.02139f), 0, 1314, 2.5f);
    m_homeFlags[1]->setState(GO_STATE_CLOSED);
    m_homeFlags[1]->setGoType(GAMEOBJECT_TYPE_FLAGSTAND);
    m_homeFlags[1]->setAnimationProgress(100);

    // dropped flags - same generic reusable props Warsong Gulch uses, position set on drop
    m_dropFlags[1] = m_mapMgr->createGameObject(HORDE_FLAG_DROP);
    if (!m_dropFlags[1]->create(ALLIANCE_FLAG_DROP, m_mapMgr, 0, LocationVector(), QuaternionData(), GO_STATE_CLOSED))
        DLLLogDetail("TwinPeaks : Could not create dropped flag 1");

    m_dropFlags[0] = m_mapMgr->createGameObject(HORDE_FLAG_DROP);
    if (!m_dropFlags[0]->create(HORDE_FLAG_DROP, m_mapMgr, 0, LocationVector(), QuaternionData(), GO_STATE_CLOSED))
        DLLLogDetail("TwinPeaks : Could not create dropped flag 0");

    for (uint8_t i = 0; i < 2; ++i)
    {
        m_dropFlags[i]->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
        m_dropFlags[i]->setScale(2.5f);
    }
}

TwinPeaks::~TwinPeaks()
{
    // gates are always spawned, so mapmgr will clean them up
    for (uint8_t i = 0; i < 6; ++i)
    {
        if (m_buffs[i] && m_buffs[i]->IsInWorld() == false)
            delete m_buffs[i];
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        if (m_dropFlags[i] && m_dropFlags[i]->IsInWorld() == false)
            delete m_dropFlags[i];

        if (m_homeFlags[i] && m_homeFlags[i]->IsInWorld() == false)
            delete m_homeFlags[i];
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
bool TwinPeaks::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    castSpellOnTeam(winningTeam, 69158);
    castSpellOnTeam(winningTeam, 69496);
    castSpellOnTeam(winningTeam, 69497);
    castSpellOnTeam(winningTeam, 69498);
    return true;
}

void TwinPeaks::HookOnAreaTrigger(Player* plr, uint32_t id)
{
    int32_t buffslot = -1;
    switch (id)
    {
        case AREATRIGGER_TP_A_SPEED:
            buffslot = 0;
            break;
        case AREATRIGGER_TP_H_SPEED:
            buffslot = 1;
            break;
        case AREATRIGGER_TP_A_RESTORATION:
            buffslot = 2;
            break;
        case AREATRIGGER_TP_H_RESTORATION:
            buffslot = 3;
            break;
        case AREATRIGGER_TP_A_BERSERKING:
            buffslot = 4;
            break;
        case AREATRIGGER_TP_H_BERSERKING:
            buffslot = 5;
            break;
        case AREATRIGGER_TP_ENCOUNTER_01:
        case AREATRIGGER_TP_ENCOUNTER_02:
        case AREATRIGGER_TP_ENCOUNTER_03:
        case AREATRIGGER_TP_ENCOUNTER_04:
        case AREATRIGGER_TP_ENCOUNTER_05:
        case AREATRIGGER_TP_ENCOUNTER_06:
        case AREATRIGGER_TP_ENCOUNTER_07:
        case AREATRIGGER_TP_ENCOUNTER_08:
        case AREATRIGGER_TP_ENCOUNTER_09:
        case AREATRIGGER_TP_ENCOUNTER_10:
        case AREATRIGGER_TP_A_SPAWN:
        case AREATRIGGER_TP_H_SPAWN:
            break;
        default:
            DLLLogDetail("Encountered unhandled areatrigger id {}", id);
            return;
            break;
    }

    if (buffslot >= 0)
    {
        if (m_buffs[buffslot] != nullptr && m_buffs[buffslot]->IsInWorld())
        {
            SpellInfo const* sp = sSpellMgr.getSpellInfo(m_buffs[buffslot]->GetGameObjectProperties()->raw.parameter_3);
            Spell* s = sSpellMgr.newSpell(plr, sp, true, 0);
            SpellCastTargets targets(plr->getGuid());
            s->prepare(&targets);

            m_buffs[buffslot]->despawn(0, TP_BUFF_RESPAWN_TIME);
        }
        return;
    }

    if (((id == AREATRIGGER_TP_A_SPAWN && plr->isTeamAlliance()) || (id == AREATRIGGER_TP_H_SPAWN && plr->isTeamHorde())) && (plr->hasBgFlag() && m_flagHolders[plr->getTeam()] == plr->getGuidLow()))
    {
        if (m_flagHolders[plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE] != 0 || m_dropFlags[plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE]->IsInWorld())
        {
            // can't cap while flag dropped
            return;
        }
        float distance = plr->isTeamAlliance() ? plr->CalcDistance(2117.64f, 191.682f, 44.052f) : plr->CalcDistance(1578.34f, 344.045f, 2.41841f);
        if (distance > 50.0f)
        {
            // 50 yards from the spawn, gtfo hacker.
            sCheatLog.writefromsession(plr->getSession(), "{} attempted to capture the Twin Peaks flag from more than 50 yards away (distance: {:.2f} yards)", plr->getName(), distance);
            plr->getSession()->Disconnect();
            return;
        }

        m_flagHolders[plr->getTeam()] = 0;
        plr->setHasBgFlag(false);

        plr->removeAllAurasById(23333 + (plr->getTeam() * 2));

        plr->m_bgScore.MiscData[BattlegroundDef::WSG_FLAGS_CAPTURED]++;

        playSoundToAll(plr->isTeamHorde() ? BattlegroundDef::HORDE_SCORES : BattlegroundDef::ALLIANCE_SCORES);

        if (plr->isTeamHorde())
            sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "%s captured the Alliance flag!", plr->getName().c_str());
        else
            sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "%s captured the Horde flag!", plr->getName().c_str());

        setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 1);

        if (m_homeFlags[plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE]->IsInWorld())
            m_homeFlags[plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE]->RemoveFromWorld(false);

        sEventMgr.AddEvent(this, &TwinPeaks::EventReturnFlags, EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG, 20000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        // Bonus honor only - Twin Peaks doesn't have Warsong Gulch's own dedicated faction
        // reputation (Warsong Outriders/Silverwing Sentinels), so no rep grant here rather
        // than misattributing rep to an unrelated faction.
        uint32_t honorToAdd = 2 * m_honorPerKill;
        for (auto* player : m_players[plr->getTeam()])
        {
            player->m_bgScore.BonusHonor += honorToAdd;
            HonorHandler::AddHonorPointsToPlayer(player, honorToAdd);
        }

        m_scores[plr->getTeam()]++;
        if (m_scores[plr->getTeam()] == 3)
        {
            sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_CLOSE);
            sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            this->endBattleground(plr->getTeam());
        }

        setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_HORDE_SCORE : WORLDSTATE_WSG_ALLIANCE_SCORE, m_scores[plr->getTeam()]);

        updatePvPData();
    }
}

void TwinPeaks::EventReturnFlags()
{
    for (uint8_t x = 0; x < 2; x++)
    {
        if (m_homeFlags[x] != nullptr)
            m_homeFlags[x]->PushToWorld(m_mapMgr);
    }
    playSoundToAll(BattlegroundDef::FLAG_RESPAWN);
    sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The flags are now placed at their bases.");
}

void TwinPeaks::HookOnFlagDrop(Player* plr)
{
    if (!plr->hasBgFlag() || m_dropFlags[plr->getTeam()]->IsInWorld())
        return;

    m_dropFlags[plr->getTeam()]->SetPosition(plr->GetPosition());
    m_dropFlags[plr->getTeam()]->PushToWorld(m_mapMgr);

    m_flagHolders[plr->getTeam()] = 0;
    plr->setHasBgFlag(false);
    plr->removeAllAurasById(23333 + (plr->getTeam() * 2));

    setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 1);

    const auto teamId = plr->getTeam();
    const uint32_t eventId = static_cast<uint32_t>(EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG) + static_cast<uint32_t>(teamId);
    sEventMgr.AddEvent(this, &TwinPeaks::ReturnFlag, teamId, eventId, 5000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    playSoundToAll(BattlegroundDef::FLAG_RETURNED);

    if (plr->isTeamHorde())
        sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "The Alliance flag was dropped by %s!", plr->getName().c_str());
    else
        sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "The Horde flag was dropped by %s!", plr->getName().c_str());
}

void TwinPeaks::HookFlagDrop(Player* plr, GameObject* obj)
{
    // picking up a dropped flag
    if (m_dropFlags[plr->getTeam()] != obj)
    {
        // are we returning it?
        if ((obj->getEntry() == ALLIANCE_FLAG_DROP && plr->isTeamAlliance()) || (obj->getEntry() == HORDE_FLAG_DROP && plr->isTeamHorde()))
        {
            uint32_t x = plr->getTeam() ? TEAM_ALLIANCE : TEAM_HORDE;
            sEventMgr.RemoveEvents(this, static_cast<uint8_t>(EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG) + static_cast<uint8_t>(plr->isTeamHorde() ? TEAM_ALLIANCE : TEAM_HORDE));

            if (m_dropFlags[x]->IsInWorld())
                m_dropFlags[x]->RemoveFromWorld(false);

            if (m_homeFlags[x]->IsInWorld() == false)
                m_homeFlags[x]->PushToWorld(m_mapMgr);

            plr->m_bgScore.MiscData[BattlegroundDef::WSG_FLAGS_RETURNED]++;
            updatePvPData();

            playSoundToAll(BattlegroundDef::FLAG_RETURNED);

            if (plr->isTeamHorde())
                sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "The Horde flag was returned to its base by %s!", plr->getName().c_str());
            else
                sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "The Alliance flag was returned to its base by %s!", plr->getName().c_str());

            setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 1);
        }
        return;
    }

    auto itr = plr->m_forcedReactions.find(1059);
    if (itr != plr->m_forcedReactions.end())
    {
        return;
    }

    if (plr->isTeamAlliance())
        sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG);
    else
        sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_WSG_AUTO_RETURN_FLAG + 1);

    if (m_dropFlags[plr->getTeam()]->IsInWorld())
        m_dropFlags[plr->getTeam()]->RemoveFromWorld(false);

    m_flagHolders[plr->getTeam()] = plr->getGuidLow();
    plr->setHasBgFlag(true);

    // Same reguid workaround Warsong Gulch documents - respawning the same guid after a
    // despawn/respawn cycle silently fails to show client-side.
    m_dropFlags[plr->getTeam()]->SetNewGuid(m_mapMgr->generateGameobjectGuid());

    SpellInfo const* pSp = sSpellMgr.getSpellInfo(23333 + (plr->getTeam() * 2));
    Spell* sp = sSpellMgr.newSpell(plr, pSp, true, 0);
    SpellCastTargets targets(plr->getGuid());
    sp->prepare(&targets);
    setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 2);
    playSoundToAll(plr->isTeamHorde() ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::ALLIANCE_CAPTURE);

    if (plr->isTeamHorde())
        sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "The Alliance's flag has been taken by %s !", plr->getName().c_str());
    else
        sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "The Horde's flag has been taken by %s !", plr->getName().c_str());
}

void TwinPeaks::ReturnFlag(PlayerTeam team)
{
    if (m_dropFlags[team]->IsInWorld())
        m_dropFlags[team]->RemoveFromWorld(false);

    if (!m_homeFlags[team]->IsInWorld())
        m_homeFlags[team]->PushToWorld(m_mapMgr);

    playSoundToAll(BattlegroundDef::FLAG_RESPAWN);

    if (team)
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The Alliance flag was returned to its base!");
    else
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The Horde flag was returned to its base!");
}

void TwinPeaks::HookFlagStand(Player* plr, GameObject* obj)
{
    if (!m_hasStarted)
    {
        sCheatLog.writefromsession(plr->getSession(), "{} attempted to pick up the Twin Peaks flag before the battleground started (ID: {}).", plr->getName(), this->m_id);
        sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, plr->getGuid(), "%s attempted to pick up the Twin Peaks flag before the battleground started and will be removed for cheating.", plr->getName().c_str());
        this->removePlayer(plr, false);
        plr->kickFromServer(6000);
        return;
    }

    if (m_flagHolders[plr->getTeam()] || m_homeFlags[plr->getTeam()] != obj || m_dropFlags[plr->getTeam()]->IsInWorld())
    {
        // cheater!
        return;
    }

    auto itr = plr->m_forcedReactions.find(1059);
    if (itr != plr->m_forcedReactions.end())
    {
        return;
    }

    SpellInfo const* pSp = sSpellMgr.getSpellInfo(23333 + (plr->getTeam() * 2));
    Spell* sp = sSpellMgr.newSpell(plr, pSp, true, 0);
    SpellCastTargets targets(plr->getGuid());
    sp->prepare(&targets);

    plr->setHasBgFlag(true);
    m_flagHolders[plr->getTeam()] = plr->getGuidLow();
    if (m_homeFlags[plr->getTeam()]->IsInWorld())
        m_homeFlags[plr->getTeam()]->RemoveFromWorld(false);

    playSoundToAll(plr->isTeamHorde() ? BattlegroundDef::HORDE_CAPTURE : BattlegroundDef::ALLIANCE_CAPTURE);
    setWorldState(plr->isTeamHorde() ? WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY : WORLDSTATE_WSG_HORDE_FLAG_DISPLAY, 2);
    if (plr->isTeamHorde())
        sendChatMessage(CHAT_MSG_BG_EVENT_HORDE, plr->getGuid(), "The Alliance's flag has been taken by %s !", plr->getName().c_str());
    else
        sendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, plr->getGuid(), "The Horde's flag has been taken by %s !", plr->getName().c_str());
}

void TwinPeaks::HookOnPlayerKill(Player* plr, Player* /*pVictim*/)
{
    plr->m_bgScore.KillingBlows++;
    updatePvPData();
}

void TwinPeaks::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void TwinPeaks::OnAddPlayer(Player* plr)
{
    if (!m_hasStarted && plr->IsInWorld())
    {
        plr->castSpell(plr, BattlegroundDef::PREPARATION, true);
        plr->m_bgScore.MiscData[BattlegroundDef::WSG_FLAGS_CAPTURED] = 0;
        plr->m_bgScore.MiscData[BattlegroundDef::WSG_FLAGS_RETURNED] = 0;
    }
    updatePvPData();
}

void TwinPeaks::OnRemovePlayer(Player* plr)
{
    if (plr->hasBgFlag())
        HookOnMount(plr);

    plr->removeAllAurasById(BattlegroundDef::PREPARATION);
}

// Not independently sourced (no explicit "start point" record found) - derived as a short
// offset from each side's real flag stand position, the same relationship Warsong Gulch's own
// (sourced) starting coords have to its own flag stands.
LocationVector TwinPeaks::GetStartingCoords(uint32_t Team)
{
    if (Team)        // Horde
        return LocationVector(1560.0f, 345.0f, 2.0f, AscEmu::Math::PiF);
    else            // Alliance
        return LocationVector(2135.0f, 190.0f, 44.0f, 0.0f);
}

void TwinPeaks::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;

    if (plr->hasBgFlag())
        plr->removeAllAurasById(23333 + (plr->getTeam() * 2));

    updatePvPData();
}

void TwinPeaks::HookOnMount(Player* plr)
{
    if (m_flagHolders[plr->getTeam()] == plr->getGuidLow())
        HookOnFlagDrop(plr);
}

// Same as GetStartingCoords - not independently sourced, derived close to each side's real
// flag stand the same way Warsong Gulch's own repop coords sit close to its flag stands.
bool TwinPeaks::HookHandleRepop(Player* plr)
{
    LocationVector dest;
    if (plr->isTeamHorde())
        dest.changeCoords({ 1560.0f, 345.0f, 2.0f, AscEmu::Math::PiF });
    else
        dest.changeCoords({ 2135.0f, 190.0f, 44.0f, 0.0f });
    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest);
    return true;
}

void TwinPeaks::SpawnBuff(uint32_t x)
{
    switch (x)
    {
        case 0: // Speed - Alliance
            m_buffs[x] = spawnGameObject(179871, LocationVector(2175.87f, 226.622f, 43.7629f, 2.60053f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.963629f, 0.267243f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 1: // Speed - Horde
            m_buffs[x] = spawnGameObject(179899, LocationVector(1544.55f, 303.852f, 0.692371f, 6.26573f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.008728f, -0.999962f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 2: // Restoration - Alliance
            m_buffs[x] = spawnGameObject(179904, LocationVector(1951.18f, 383.795f, -10.5257f, 4.06662f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.894934f, -0.446199f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 3: // Restoration - Horde
            m_buffs[x] = spawnGameObject(179906, LocationVector(1754.16f, 242.125f, -14.1316f, 1.15192f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.544640f, 0.838670f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 4: // Berserking - Alliance
            m_buffs[x] = spawnGameObject(179905, LocationVector(1932.83f, 226.792f, -17.0598f, 2.44346f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.939692f, 0.342021f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
        case 5: // Berserking - Horde
            m_buffs[x] = spawnGameObject(179907, LocationVector(1737.57f, 435.845f, -8.08634f, 5.51524f), 0, 114, 1);
            m_buffs[x]->setLocalRotation(0.f, 0.f, 0.374607f, -0.927184f);
            m_buffs[x]->setState(GO_STATE_CLOSED);
            m_buffs[x]->setGoType(GAMEOBJECT_TYPE_TRAP);
            m_buffs[x]->setAnimationProgress(100);
            break;
    }
}

void TwinPeaks::OnCreate()
{
    for (uint8_t i = 0; i < 6; ++i)
    {
        if (!m_buffs[i]->IsInWorld())
            m_buffs[i]->PushToWorld(m_mapMgr);
    }

    // Alliance Gates (Dwarven Gate 01-03)
    GameObject* gate = spawnGameObject(206653, LocationVector(2135.52f, 218.926f, 43.6095f, 5.75086f), 33, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.263031f, -0.964787f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(206654, LocationVector(2156.0f, 219.206f, 43.6256f, 2.60926f), 33, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.964786f, 0.263035f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(206655, LocationVector(2118.09f, 154.675f, 43.5709f, 2.60926f), 33, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.964786f, 0.263035f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    // Horde Gates
    gate = spawnGameObject(208205, LocationVector(1556.66f, 314.713f, 1.589f, 6.17847f), 32, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.052334f, -0.998630f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(208206, LocationVector(1574.61f, 321.242f, 1.58989f, 6.17847f), 32, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.052334f, -0.998630f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(208207, LocationVector(1558.09f, 372.765f, 1.72373f, 6.17847f), 32, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.052334f, -0.998630f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    gate = spawnGameObject(203710, LocationVector(1558.62f, 379.16f, -6.40967f, 4.60767f), 32, 114, 1.0f);
    gate->setLocalRotation(0.f, 0.f, 0.743145f, -0.669131f);
    gate->setAnimationProgress(100);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    setWorldState(WORLDSTATE_WSG_MAX_SCORE, 3);

    // Not independently sourced - same relationship to the real flag stands as
    // GetStartingCoords/HookHandleRepop above.
    addSpiritGuide(spawnSpiritGuide(2135.0f, 190.0f, 44.0f, 0.0f, 0));
    addSpiritGuide(spawnSpiritGuide(1560.0f, 345.0f, 2.0f, AscEmu::Math::PiF, 1));
}

void TwinPeaks::OnStart()
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

    DespawnGates(5000);

    for (uint8_t i = 0; i < 2; ++i)
    {
        if (!m_homeFlags[i]->IsInWorld())
            m_homeFlags[i]->PushToWorld(m_mapMgr);
    }

    playSoundToAll(BattlegroundDef::BATTLEGROUND_BEGIN);
    sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The flags are now placed at their bases.");

    sEventMgr.AddEvent(this, &TwinPeaks::TimeLeft, EVENT_UNK, 60000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    m_hasStarted = true;
}

void TwinPeaks::TimeLeft()
{
    --m_time_left;
    setWorldState(WORLDSTATE_WSG_TIME_LEFT, m_time_left);
}

void TwinPeaks::HookOnShadowSight()
{}

void TwinPeaks::HookGenerateLoot(Player* /*plr*/, Object* /*pOCorpse*/)
{}

void TwinPeaks::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void TwinPeaks::SetIsWeekend(bool isweekend)
{
    m_isWeekend = isweekend;
}

void TwinPeaks::DespawnGates(uint32_t delay)
{
    for (auto* gate : m_gates)
        gate->despawn(delay, 0);

    m_gates.clear();
}
