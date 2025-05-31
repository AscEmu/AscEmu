/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/ItemInterface.h"
#include "Management/Arenas.hpp"

#include "Logging/Logger.hpp"
#include "Management/ArenaTeam.hpp"
#include "Management/WorldStates.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Spell/SpellAura.hpp"
#include "Objects/GameObject.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Spell/SpellMgr.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/Script/HookInterface.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Utilities/Narrow.hpp"

const uint32_t ARENA_PREPARATION = 32727;

// const uint32_t GREEN_TEAM = 0;
// const uint32_t GOLD_TEAM = 1;

Arena::Arena(WorldMap* _worldMap, uint32_t _id, uint32_t _levelGroup, uint32_t _arenaType, uint32_t _playersPerSide) : Battleground(_worldMap, _id, _levelGroup, _arenaType)
{
    for (uint8_t i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
    }

    m_pvpData.clear();
    m_resurrectMap.clear();

    m_hasStarted = false;
    m_playerCountPerTeam = _playersPerSide;

    switch (_arenaType)
    {
    case BattlegroundDef::TYPE_ARENA_5V5:
        m_arenateamtype = 2;
        break;
    case BattlegroundDef::TYPE_ARENA_3V3:
        m_arenateamtype = 1;
        break;
    case BattlegroundDef::TYPE_ARENA_2V2:
        m_arenateamtype = 0;
        break;
    default:
        m_arenateamtype = 0;
        break;
    }

    switch (m_mapMgr->getBaseMap()->getMapId())
    {
    case 559:
        m_zoneId = 3698;
        break;
    case 562:
        m_zoneId = 3702;
        break;
    case 572:
        m_zoneId = 3968;
        break;
    case 617:
        m_zoneId = 4378;
        break;
    case 618:
        m_zoneId = 4408;
        break;
    default:
        break;
    }
}

Arena::~Arena()
{
    for (uint8_t i = 0; i < 2; ++i)
    {
        // buffs may not be spawned, so delete them if they're not
        if (m_buffs[i] && m_buffs[i]->IsInWorld() == false)
            delete m_buffs[i];
    }

    for (std::set<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        if ((*itr) != NULL)
        {
            if (!(*itr)->IsInWorld())
                delete(*itr);
        }
    }
}

// \todo Rewrite this function entirely
bool Arena::HandleFinishBattlegroundRewardCalculation(PlayerTeam _winningTeam)
{
    // update arena team stats
    if (rated_match)
    {
        m_deltaRating[0] = m_deltaRating[1] = 0;
        for (uint8_t i = 0; i < 2; ++i)
        {
            uint8_t j = i ? 0 : 1; // opposing side
            bool outcome;

            if (m_teams[i] == NULL || m_teams[j] == NULL)
                continue;

            outcome = (i == _winningTeam);
            if (outcome)
            {
                m_teams[i]->m_stats.won_season++;
                m_teams[i]->m_stats.won_week++;
            }

            m_deltaRating[i] = CalcDeltaRating(m_teams[i]->m_stats.rating, m_teams[j]->m_stats.rating, outcome);
            m_teams[i]->m_stats.rating += m_deltaRating[i];
            if (static_cast<int32_t>(m_teams[i]->m_stats.rating) < 0) m_teams[i]->m_stats.rating = 0;

            for (std::set<uint32_t>::iterator itr = m_players2[i].begin(); itr != m_players2[i].end(); ++itr)
            {
                const auto info = sObjectMgr.getCachedCharacterInfo(*itr);
                if (info)
                {
                    ArenaTeamMember* tp = m_teams[i]->getMember(info);

                    if (tp != NULL)
                    {
                        tp->PersonalRating += CalcDeltaRating(tp->PersonalRating, m_teams[j]->m_stats.rating, outcome);
                        if (static_cast<int32_t>(tp->PersonalRating) < 0)
                            tp->PersonalRating = 0;

                        if (outcome)
                        {
                            ++(tp->Won_ThisWeek);
                            ++(tp->Won_ThisSeason);
                        }
                    }
                }
            }

            m_teams[i]->saveToDB();
        }
    }

    sObjectMgr.updateArenaTeamRankings();

    m_nextPvPUpdateTime = 0;
    updatePvPData();
    playSoundToAll(_winningTeam ? BattlegroundDef::ALLIANCEWINS : BattlegroundDef::HORDEWINS);

    sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_CLOSE);
    sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    for (uint8_t i = 0; i < 2; i++)
    {
        bool victorious = (i == _winningTeam);
        std::set<Player*>::iterator itr = m_players[i].begin();
        for (; itr != m_players[i].end(); ++itr)
        {
            Player* plr = *itr;
            if (plr != NULL)
            {
                sHookInterface.OnArenaFinish(plr, plr->getArenaTeam(m_arenateamtype), victorious, rated_match);
                plr->resetAllCooldowns();
            }
        }
    }

    /* Prevent honor being given to people in arena */
    return false;
}

void Arena::OnAddPlayer(Player* _player)
{
    if (_player == NULL)
        return;

    _player->m_deathVision = true;

    // remove all buffs (exclude talents, include flasks)
    for (uint16_t x = AuraSlots::REMOVABLE_SLOT_START; x < AuraSlots::REMOVABLE_SLOT_END; x++)
    {
        if (auto* const aur = _player->getAuraWithAuraSlot(x))
        {
            if (!aur->getSpellInfo()->getDurationIndex() && aur->getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD)
                continue;

            aur->removeAura();
        }
    }
    // On arena start all conjured items are removed
    _player->getItemInterface()->RemoveAllConjured();
    // On arena start remove all temp enchants
    _player->removeTempItemEnchantsOnArena();

    // Before the arena starts all your cooldowns are reset
    if (!m_hasStarted && _player->IsInWorld())
        _player->resetAllCooldowns();

    // if (_player->m_isGmInvisible == false)
    // Make sure the player isn't a GM an isn't invisible (monitoring?)
    if (!_player->m_isGmInvisible)
    {
        if (!m_hasStarted && _player->IsInWorld())
            _player->castSpell(_player, ARENA_PREPARATION, true);

        m_playersCount[_player->getTeam()]++;
        UpdatePlayerCounts();
    }
    // If they're still queued for the arena, remove them from the queue
    if (_player->isQueuedForBg())
        _player->setIsQueuedForBg(false);

    // Add the green/gold team flag
    auto aura = sSpellMgr.newAura(sSpellMgr.getSpellInfo((_player->getInitialTeam()) ? 35775 - _player->getBgTeam() : 32725 - _player->getBgTeam()), -1, _player, _player, true);
    _player->addAura(std::move(aura));

    _player->setFfaPvpFlag();

    m_playersAlive.insert(_player->getGuidLow());
}

void Arena::OnRemovePlayer(Player* _player)
{
    // remove arena readiness buff
    _player->m_deathVision = false;

    _player->removeAllAuras();

    // Player has left arena, call HookOnPlayerDeath as if he died
    HookOnPlayerDeath(_player);

    _player->removeAllAurasById(_player->getInitialTeam() ? 35775 - _player->getBgTeam() : 32725 - _player->getBgTeam());
    _player->removeFfaPvpFlag();

    // Reset all their cooldowns and restore their HP/Mana/Energy to max
    _player->resetAllCooldowns();
    _player->setFullHealthMana();
}

void Arena::HookOnPlayerKill(Player* _player, Player* _playerVictim)
{
    if (!m_hasStarted)
    {
        _player->die(nullptr, 0, 0); //cheater.
        return;
    }

    if (_playerVictim->isPlayer())
    {
        _player->m_bgScore.KillingBlows++;
    }
}

void Arena::HookOnHK(Player* _player)
{
    _player->m_bgScore.HonorableKills++;
}

void Arena::HookOnPlayerDeath(Player* _player)
{
    if (_player)
    {
        if (_player->m_isGmInvisible == true)
            return;

        if (m_playersAlive.find(_player->getGuidLow()) != m_playersAlive.end())
        {
            m_playersCount[_player->getTeam()]--;
            UpdatePlayerCounts();
            m_playersAlive.erase(_player->getGuidLow());
        }
    }
    else
    {
        sLogger.failure("Tried to call Arena::HookOnPlayerDeath with nullptr player pointer");
    }
}

void Arena::OnCreate()
{
    // push gates into world
    for (std::set<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
        (*itr)->PushToWorld(m_mapMgr);
}

void Arena::HookOnShadowSight()
{}

void Arena::OnStart()
{
    // remove arena readiness buff
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            Player* plr = *itr;
            plr->removeAllAurasById(ARENA_PREPARATION);
            m_players2[i].insert(plr->getGuidLow());

            // update arena team stats
            if (rated_match && plr->isInArenaTeam(m_arenateamtype))
            {
                m_teams[i] = plr->getArenaTeam(m_arenateamtype);
                ArenaTeamMember* tp = m_teams[i]->getMember(plr->getPlayerInfo());
                if (tp != NULL)
                {
                    tp->Played_ThisWeek++;
                    tp->Played_ThisSeason++;
                }
            }
        }
    }

    for (uint8_t i = 0; i < 2; i++)
    {
        if (m_teams[i] == NULL)
            continue;

        m_teams[i]->m_stats.played_season++;
        m_teams[i]->m_stats.played_week++;
        m_teams[i]->saveToDB();
    }

    // open gates
    for (std::set<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        (*itr)->setFlags(GO_FLAG_TRIGGERED);
        (*itr)->setState(GO_STATE_CLOSED);
    }

    m_hasStarted = true;

    // Incase all players left
    UpdatePlayerCounts();

    // WHEEEE
    playSoundToAll(BattlegroundDef::BATTLEGROUND_BEGIN);

    sEventMgr.RemoveEvents(this, EVENT_ARENA_SHADOW_SIGHT);
    sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::HookOnShadowSight, EVENT_ARENA_SHADOW_SIGHT, 90000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

bool Arena::CanPlayerJoin(Player* _player, uint32_t _type)
{
    if (m_hasStarted)
        return false;

    return Battleground::CanPlayerJoin(_player, _type);
}

bool Arena::CreateCorpse(Player* /*plr*/) { return false; }

void Arena::UpdatePlayerCounts()
{
    if (this->hasEnded())
        return;

    setWorldState(WORLDSTATE_ARENA__GREEN_PLAYER_COUNT, m_playersCount[0]);
    setWorldState(WORLDSTATE_ARENA__GOLD_PLAYER_COUNT, m_playersCount[1]);

    if (!m_hasStarted)
        return;

    //    return;

    if (m_playersCount[TEAM_HORDE] == 0)
        this->endBattleground(TEAM_ALLIANCE);
    else if (m_playersCount[TEAM_ALLIANCE] == 0)
        this->endBattleground(TEAM_HORDE);
}

uint32_t Arena::CalcDeltaRating(uint32_t _oldRating, uint32_t _opponentRating, bool _outcome)
{
    double power = (int)(_opponentRating - _oldRating) / 400.0f;
    double divisor = pow(10.0, power);
    divisor += 1.0;

    double winChance = 1.0 / divisor;

    // New Rating Calculation via Elo
    // New Rating = Old Rating + K * (_outcome - Expected Win Chance)
    // _outcome = 1 for a win and 0 for a loss (0.5 for a draw ... but we cant have that)
    // K is the maximum possible change
    // Through investigation, K was estimated to be 32 (same as chess)
    double multiplier = (_outcome ? 1.0 : 0.0) - winChance;
    return Util::long2int32(32.0 * multiplier);
}

uint32_t Arena::GetTeamFaction(uint32_t _teamId)
{
    std::set< Player* >::iterator itr = m_players[_teamId].begin();
    Player* p = *itr;
    return p->getTeam();
}

uint8_t Arena::Rated() { return rated_match; }
uint8_t Arena::GetArenaTeamType() const { return m_arenateamtype; }
ArenaTeam** Arena::GetTeams() { return m_teams; }

LocationVector Arena::GetStartingCoords(uint32_t /*Team*/)
{
    return LocationVector(0, 0, 0, 0);
}

uint32_t Arena::GetNameID() { return 50; }

bool Arena::HookHandleRepop(Player* /*plr*/)
{
    return false;
}

void Arena::HookOnAreaTrigger(Player* _player, uint32_t id)
{
    int32_t buffslot = -1;

    if (_player)
    {
        switch (id)
        {
        case 4536:
        case 4538:
        case 4696:
            buffslot = 0;
            break;
        case 4537:
        case 4539:
        case 4697:
            buffslot = 1;
            break;
        default:
            break;
        }

        if (buffslot >= 0)
        {
            if (m_buffs[buffslot] != NULL && m_buffs[buffslot]->IsInWorld())
            {
                // apply the buff
                SpellInfo const* sp = sSpellMgr.getSpellInfo(m_buffs[buffslot]->GetGameObjectProperties()->raw.parameter_3);
                if (sp == nullptr)
                {
                    sLogger.failure("Arena::HookOnAreaTrigger: tried to use invalid spell {} for gameobject {}", m_buffs[buffslot]->GetGameObjectProperties()->raw.parameter_3, m_buffs[buffslot]->GetGameObjectProperties()->entry);
                }

                Spell* s = sSpellMgr.newSpell(_player, sp, true, 0);
                SpellCastTargets targets(_player->getGuid());
                s->prepare(&targets);

                // despawn the gameobject (not delete!)
                m_buffs[buffslot]->despawn(0, 30 /*BUFF_RESPAWN_TIME*/);
            }
        }
    }
    else
    {
        sLogger.failure("Tried to call Arena::HookOnAreaTrigger with nullptr player pointer");
    }
}

int32_t Arena::GetFreeTeam() const
{
    size_t c0 = m_players[0].size() + m_pendPlayers[0].size();
    size_t c1 = m_players[1].size() + m_pendPlayers[1].size();
    if (m_hasStarted)
        return -1;

    // Check if there is free room, if yes, return team with less members
    return ((c0 + c1 >= m_playerCountPerTeam * 2) ? -1 : (c0 > c1));
}

void Arena::HookGenerateLoot(Player* /*plr*/, Object* /*pCorpse*/) // Not Used
{}

void Arena::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void Arena::HookOnFlagDrop(Player* /*plr*/)
{}
