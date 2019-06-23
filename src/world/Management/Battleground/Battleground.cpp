/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"
#include "Units/Players/PlayerDefines.hpp"
#include "Management/HonorHandler.h"
#include "Management/Battleground/Battleground.h"
#include "Management/Arenas.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"
#include <Spell/Definitions/AuraInterruptFlags.h>
#include "Spell/Definitions/PowerType.h"
#include "Server/Packets/SmsgPlaySound.h"

uint32 CBattleground::GetId()
{
    return m_id;
}

uint32 CBattleground::GetLevelGroup()
{
    return m_levelGroup;
}

MapMgr* CBattleground::GetMapMgr()
{
    return m_mapMgr;
}

CBattleground::CBattleground(MapMgr* mgr, uint32 id, uint32 levelgroup, uint32 type) : m_mapMgr(mgr), m_id(id), m_type(type), m_levelGroup(levelgroup), m_invisGMs(0)
{
    m_nextPvPUpdateTime = 0;
    m_countdownStage = 0;
    m_ended = false;
    m_started = false;
    m_winningteam = 0;
    m_startTime = static_cast<uint32>(UNIXTIME);
    m_lastResurrect = static_cast<uint32>(UNIXTIME);
    m_zoneid = 0;
    sEventMgr.AddEvent(this, &CBattleground::EventResurrectPlayers, EVENT_BATTLEGROUND_QUEUE_UPDATE, 30000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    /* create raid groups */
    for (uint8 i = 0; i < 2; ++i)
    {
        m_groups[i] = new Group(true);
        m_groups[i]->m_disbandOnNoMembers = false;
        m_groups[i]->ExpandToRaid();
    }
    m_honorPerKill = HonorHandler::CalculateHonorPointsForKill(m_levelGroup * 10, m_levelGroup * 10);
}

CBattleground::~CBattleground()
{
    sEventMgr.RemoveEvents(this);
    for (uint8 i = 0; i < 2; ++i)
    {
        PlayerInfo* inf;
        for (uint32 j = 0; j < m_groups[i]->GetSubGroupCount(); ++j)
        {
            for (GroupMembersSet::iterator itr = m_groups[i]->GetSubGroup(j)->GetGroupMembersBegin(); itr != m_groups[i]->GetSubGroup(j)->GetGroupMembersEnd();)
            {
                inf = (*itr);
                ++itr;
                m_groups[i]->RemovePlayer(inf);
            }
        }
        delete m_groups[i];
    }

    m_resurrectMap.clear();
    m_players[0].clear();
    m_players[1].clear();
}

void CBattleground::UpdatePvPData()
{
    if (isArena(m_type))
    {
        if (!m_ended)
        {
            return;
        }
    }

    if (UNIXTIME >= m_nextPvPUpdateTime)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        WorldPacket data(10 * (m_players[0].size() + m_players[1].size()) + 50);
        BuildPvPUpdateDataPacket(&data);
        DistributePacketToAll(&data);

        m_nextPvPUpdateTime = UNIXTIME + 2;
    }
}

uint32 CBattleground::GetStartTime()
{
    return m_startTime;
}

uint32 CBattleground::GetType()
{
    return m_type;
}

void CBattleground::BuildPvPUpdateDataPacket(WorldPacket* data)
{
    ARCEMU_ASSERT(data != NULL);

    data->Initialize(MSG_PVP_LOG_DATA);
    data->reserve(10 * (m_players[0].size() + m_players[1].size()) + 50);

    BGScore* bs;
    if (isArena(m_type))
    {
        if (!m_ended)
        {
            return;
        }

        *data << uint8(1);

        if (!Rated())
        {
            *data << uint32(0); //uint32(negative rating)
            *data << uint32(0); //uint32(positive rating)
            *data << uint32(0); //uint32(0)[<-this is the new field in 3.1]
            *data << uint8(0);  //name if available / which is a null-terminated string, and we send an uint8(0), so we provide a zero length name string
            *data << uint32(0);
            *data << uint32(0);
            *data << uint32(0);
            *data << uint8(0);
        }
        else
        {
            /* Grab some arena teams */
            ArenaTeam** teams = static_cast< Arena* >(this)->GetTeams();

            if (teams[0])
            {
                *data << uint32(0);
                *data << uint32(3000 + m_deltaRating[0]);
                *data << uint32(0);
                *data << uint8(0);
            }
            else
            {
                *data << uint32(0);
                *data << uint32(0);
                *data << uint32(0);
                *data << uint8(0);
            }

            if (teams[1])
            {
                *data << uint32(0);
                *data << uint32(3000 + m_deltaRating[1]);
                *data << uint32(0);
                *data << uint8(0);
            }
            else
            {
                *data << uint32(0);
                *data << uint32(0);
                *data << uint32(0);
                *data << uint8(0);
            }
        }

        *data << uint8(1);
        *data << uint8(m_winningteam);

        *data << uint32((m_players[0].size() + m_players[1].size()) - m_invisGMs);
        for (uint8 i = 0; i < 2; ++i)
        {
            for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
            {
                if ((*itr)->m_isGmInvisible)
                    continue;
                *data << (*itr)->getGuid();
                bs = &(*itr)->m_bgScore;
                *data << bs->KillingBlows;

                *data << uint8((*itr)->getBgTeam());

                *data << bs->DamageDone;
                *data << bs->HealingDone;
                *data << uint32(0);
            }
        }
    }
    else
    {
        *data << uint8(0);
        if (m_ended)
        {
            *data << uint8(1);
            *data << uint8(m_winningteam ? 0 : 1);
        }
        else
            *data << uint8(0);      // If the game has ended - this will be 1

        *data << uint32((m_players[0].size() + m_players[1].size()) - m_invisGMs);

        uint32 FieldCount = GetFieldCount(GetType());
        for (uint8 i = 0; i < 2; ++i)
        {
            for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
            {
                ARCEMU_ASSERT(*itr != NULL);
                if ((*itr)->m_isGmInvisible)
                    continue;
                *data << (*itr)->getGuid();
                bs = &(*itr)->m_bgScore;

                *data << bs->KillingBlows;
                *data << bs->HonorableKills;
                *data << bs->Deaths;
                *data << bs->BonusHonor;
                *data << bs->DamageDone;
                *data << bs->HealingDone;

                *data << FieldCount;
                for (uint32 x = 0; x < FieldCount; ++x)
                    *data << bs->MiscData[x];
            }
        }
    }

}

uint8 CBattleground::Rated()
{
    return 0;
}

void CBattleground::AddPlayer(Player* plr, uint32 team)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    /* This is called when the player is added, not when they port. So, they're essentially still queued, but not inside the bg yet */
    m_pendPlayers[team].insert(plr->getGuidLow());

    /* Send a packet telling them that they can enter */
    plr->m_pendingBattleground = this;
    BattlegroundManager.SendBattlefieldStatus(plr, BGSTATUS_READY, m_type, m_id, 80000, m_mapMgr->GetMapId(), Rated());        // You will be removed from the queue in 2 minutes.

    /* Add an event to remove them in 1 minute 20 seconds time. */
    sEventMgr.AddEvent(plr, &Player::RemoveFromBattlegroundQueue, EVENT_BATTLEGROUND_QUEUE_UPDATE, 80000, 1, 0);
}

void CBattleground::RemovePendingPlayer(Player* plr)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    m_pendPlayers[plr->getBgTeam()].erase(plr->getGuidLow());

    /* send a null bg update (so they don't join) */
    BattlegroundManager.SendBattlefieldStatus(plr, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
    plr->m_pendingBattleground = nullptr;
    plr->setBgTeam(plr->getTeam());
}

void CBattleground::OnPlayerPushed(Player* plr)
{
    if (plr->GetGroup() && !Rated())
        plr->GetGroup()->RemovePlayer(plr->getPlayerInfo());

    plr->ProcessPendingUpdates();

    if (plr->GetGroup() == nullptr)
    {
        if (plr->m_isGmInvisible == false)    //do not join invisible gm's into bg groups.
            m_groups[plr->getBgTeam()]->AddMember(plr->getPlayerInfo());
    }
}

void CBattleground::SetIsWeekend(bool /*isweekend*/)
{

}

void CBattleground::PortPlayer(Player* plr, bool skip_teleport /* = false*/)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if (m_ended)
    {
        sChatHandler.SystemMessage(plr->GetSession(), plr->GetSession()->LocalizedWorldSrv(53));
        BattlegroundManager.SendBattlefieldStatus(plr, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
        plr->m_pendingBattleground = nullptr;
        return;
    }

    m_pendPlayers[plr->getBgTeam()].erase(plr->getGuidLow());
    if (m_players[plr->getBgTeam()].find(plr) != m_players[plr->getBgTeam()].end())
    {
        return;
    }

    plr->FullHPMP();
    plr->setTeam(plr->getBgTeam());
    if (plr->m_isGmInvisible == false)
    {
        //Do not let everyone know an invisible gm has joined.
        WorldPacket data(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
        data << plr->getGuid();
        DistributePacketToTeam(&data, plr->getBgTeam());
    }
    else
    {
        ++m_invisGMs;
    }
    m_players[plr->getBgTeam()].insert(plr);

    /* remove from any auto queue remove events */
    sEventMgr.RemoveEvents(plr, EVENT_BATTLEGROUND_QUEUE_UPDATE);

    if (!skip_teleport)
    {
        if (plr->IsInWorld())
            plr->RemoveFromWorld();
    }

    plr->m_pendingBattleground = nullptr;
    plr->m_bg = this;

    if (!plr->isPvpFlagSet())
        plr->setPvpFlag();

    plr->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_PVP_ENTER);

    /* Reset the score */
    memset(&plr->m_bgScore, 0, sizeof(BGScore));

    /* update pvp data */
    UpdatePvPData();

    /* add the player to the group */
    if (plr->GetGroup() && !Rated())
    {
        // remove them from their group
        plr->GetGroup()->RemovePlayer(plr->getPlayerInfo());
    }

    if (!m_countdownStage)
    {
        m_countdownStage = 1;
        sEventMgr.AddEvent(this, &CBattleground::EventCountdown, EVENT_BATTLEGROUND_COUNTDOWN, 30000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        sEventMgr.ModifyEventTimeLeft(this, EVENT_BATTLEGROUND_COUNTDOWN, 10000);
    }

    sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_CLOSE);

    if (!skip_teleport)
    {
        /* This is where we actually teleport the player to the battleground. */
        plr->SafeTeleport(m_mapMgr, GetStartingCoords(plr->getBgTeam()));
        BattlegroundManager.SendBattlefieldStatus(plr, BGSTATUS_TIME, m_type, m_id, static_cast<uint32>(UNIXTIME) - m_startTime, m_mapMgr->GetMapId(), Rated());     // Elapsed time is the last argument
    }
    else
    {
        /* If we are not ported, call this immediatelly, otherwise its called after teleportation in Player::OnPushToWorld */
        OnAddPlayer(plr);
    }
}

GameObject* CBattleground::SpawnGameObject(uint32 entry, uint32 MapId, float x, float y, float z, float o, uint32 flags, uint32 faction, float scale)
{
    GameObject* go = m_mapMgr->CreateGameObject(entry);

    ARCEMU_ASSERT(go != nullptr);

    //Zyres: CID 104108 coverity ignores the assert so double check this
    if (go != nullptr)
    {
        go->CreateFromProto(entry, MapId, x, y, z, o);

        go->SetFaction(faction);
        go->setScale(scale);
        go->setFlags(flags);
        go->SetPosition(x, y, z, o);
        go->SetInstanceID(m_mapMgr->GetInstanceID());
    }

    return go;
}

GameObject* CBattleground::SpawnGameObject(uint32 entry, LocationVector &v, uint32 flags, uint32 faction, float scale)
{
    return SpawnGameObject(entry, m_mapMgr->GetMapId(), v.x, v.y, v.z, v.o, flags, faction, scale);
}

Creature* CBattleground::SpawnCreature(uint32 entry, float x, float y, float z, float o, uint32 faction)
{
    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(entry);
    if (cp == nullptr)
    {
        LOG_ERROR("tried to push a invalid creature with entry %u!", entry);
        return nullptr;
    }

    Creature* c = m_mapMgr->CreateCreature(entry);

    ARCEMU_ASSERT(c != nullptr);

    c->Load(cp, x, y, z, o);

    if (faction != 0)
        c->SetFaction(faction);

    c->PushToWorld(m_mapMgr);
    return c;
}

Creature* CBattleground::SpawnCreature(uint32 entry, LocationVector &v, uint32 faction)
{
    return SpawnCreature(entry, v.x, v.y, v.z, v.o, faction);
}

void CBattleground::AddInvisGM()
{
    ++m_invisGMs;
}

void CBattleground::RemoveInvisGM()
{
    --m_invisGMs;
}

std::recursive_mutex& CBattleground::GetMutex()
{
    return m_mutex;
}

/*!
 * Starts the current battleground
 * \sa CBattleground::EndBattleground */
void CBattleground::StartBattleground()
{
    this->OnStart();
}

/*!
 * Ends the current battleground
 * \param winningTeam PlayerTeam that won the battleground
 * \sa CBattleground::StartBattleground
 * \todo Move reward calculations to seperate functions */
void CBattleground::EndBattleground(PlayerTeam winningTeam)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    this->m_ended = true;
    this->m_winningteam = winningTeam;
    this->m_nextPvPUpdateTime = 0;

    auto losingTeam = winningTeam == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE;

    /* If we still need to handle reward calculations */
    if (this->HandleFinishBattlegroundRewardCalculation(winningTeam))
    {
        for (auto winningPlr : m_players[winningTeam])
        {
            if (winningPlr->QueuedForRbg())
            {
                winningPlr->ApplyRandomBattlegroundReward(true);
                winningPlr->SetHasWonRbgToday(true);
            }

            winningPlr->SaveToDB(false);
        }
        for (auto losingPlr : m_players[losingTeam])
        {
            if (losingPlr->QueuedForRbg())
            {
                losingPlr->ApplyRandomBattlegroundReward(false);
            }

            losingPlr->SaveToDB(false);
        }

        this->AddHonorToTeam(winningTeam, 3 * 185);
        this->AddHonorToTeam(losingTeam, 1 * 185);
    }

    this->PlaySoundToAll(winningTeam == TEAM_ALLIANCE ? SOUND_ALLIANCEWINS : SOUND_HORDEWINS);

    this->UpdatePvPData();
}

/*! \returns True if battleground has started
 *  \sa CBattleground::HasEnded */
bool CBattleground::HasStarted()
{
    return this->m_started;
}

/*! \returns True if battleground has ended
 *  \sa CBattleground::HasStarted */
bool CBattleground::HasEnded()
{
    return this->m_ended;
}

void CBattleground::AddHonorToTeam(uint32 team, uint32 amount)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    for (std::set< Player* >::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
    {
        Player* p = *itr;
        HonorHandler::AddHonorPointsToPlayer(p, amount);
    }
}

void CBattleground::CastSpellOnTeam(uint32 team, uint32 spell)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    for (std::set< Player* >::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
    {
        Player* p = *itr;
        p->castSpell(p, spell, false);
    }
}

void CBattleground::RemoveAuraFromTeam(uint32 team, uint32 aura)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    for (std::set< Player* >::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
    {
        Player* p = *itr;
        p->RemoveAura(aura);
    }
}

void CBattleground::SendChatMessage(uint32 Type, uint64 Guid, const char* Format, ...)
{
    char msg[500];
    va_list ap;
    va_start(ap, Format);
    vsnprintf(msg, 500, Format, ap);
    va_end(ap);
    WorldPacket* data = sChatHandler.FillMessageData(Type, 0, msg, Guid, 0);
    DistributePacketToAll(data);
    delete data;
}

/*! \returns True if CBattleground should handle calculations, false if calculations were handled completely
 *  \param winningTeam PlayerTeam of the team that won the battleground */
bool CBattleground::HandleFinishBattlegroundRewardCalculation(PlayerTeam /*winningTeam*/)
{
    return true;
}

void CBattleground::HookOnPlayerResurrect(Player* /*player*/)
{
}

void CBattleground::HookOnUnitDied(Unit* /*victim*/)
{
}

void CBattleground::DistributePacketToAll(WorldPacket* packet)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    for (uint8 i = 0; i < 2; ++i)
    {
        for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
            if ((*itr) && (*itr)->GetSession())
                (*itr)->GetSession()->SendPacket(packet);
    }
}

void CBattleground::DistributePacketToTeam(WorldPacket* packet, uint32 Team)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    for (std::set<Player*>::iterator itr = m_players[Team].begin(); itr != m_players[Team].end(); ++itr)
    {
        if ((*itr) && (*itr)->GetSession())
            (*itr)->GetSession()->SendPacket(packet);
    }
}

void CBattleground::PlaySoundToAll(uint32 Sound)
{
    DistributePacketToAll(AscEmu::Packets::SmsgPlaySound(Sound).serialise().get());
}

void CBattleground::PlaySoundToTeam(uint32 Team, uint32 Sound)
{
    DistributePacketToTeam(AscEmu::Packets::SmsgPlaySound(Sound).serialise().get(), Team);
}

void CBattleground::RemovePlayer(Player* plr, bool logout)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    WorldPacket data(SMSG_BATTLEGROUND_PLAYER_LEFT, 30);
    data << plr->getGuid();
    if (plr->m_isGmInvisible == false)
    {
        //Don't show invisible gm's leaving the game.
        DistributePacketToAll(&data);
    }
    else
    {
        --m_invisGMs;
    }

    // Call subclassed virtual method
    OnRemovePlayer(plr);

    // Clean-up
    plr->m_bg = nullptr;
    plr->FullHPMP();
    m_players[plr->getBgTeam()].erase(plr);
    memset(&plr->m_bgScore, 0, sizeof(BGScore));

    /* are we in the group? */
    if (plr->GetGroup() == m_groups[plr->getBgTeam()])
        plr->GetGroup()->RemovePlayer(plr->getPlayerInfo());

    // reset team
    plr->resetTeam();

    /* revive the player if he is dead */
    if (!plr->isAlive())
    {
        plr->setHealth(plr->getMaxHealth());
        plr->ResurrectPlayer();
    }

    /* remove buffs */
    plr->RemoveAura(32727); // Arena preparation
    plr->RemoveAura(44521); // BG preparation
    plr->RemoveAura(44535);
    plr->RemoveAura(21074);

    plr->setMoveRoot(false);

    /* teleport out */
    if (!logout)
    {
        if (!m_ended)
        {
            if(!plr->GetSession()->HasGMPermissions())
                plr->castSpell(plr, BG_DESERTER, true);
        }

        if (!IS_INSTANCE(plr->m_bgEntryPointMap))
        {
            LocationVector vec(plr->m_bgEntryPointX, plr->m_bgEntryPointY, plr->m_bgEntryPointZ, plr->m_bgEntryPointO);
            plr->SafeTeleport(plr->m_bgEntryPointMap, plr->m_bgEntryPointInstance, vec);
        }
        else
        {
            LocationVector vec(plr->GetBindPositionX(), plr->GetBindPositionY(), plr->GetBindPositionZ());
            plr->SafeTeleport(plr->GetBindMapId(), 0, vec);
        }

        BattlegroundManager.SendBattlefieldStatus(plr, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
    }

    if (/*!m_ended && */m_players[0].size() == 0 && m_players[1].size() == 0)
    {
        /* create an inactive event */
        sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_CLOSE);                  // 10mins
        //sEventMgr.AddEvent(this, &CBattleground::Close, EVENT_BATTLEGROUND_CLOSE, 600000, 1,0); //this is BS..appears to be        the cause if the battleground crashes.
        this->Close();
    }

    plr->setBgTeam(plr->getTeam());
}

void CBattleground::SendPVPData(Player* plr)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    WorldPacket data(10 * (m_players[0].size() + m_players[1].size()) + 50);
    BuildPvPUpdateDataPacket(&data);
    plr->GetSession()->SendPacket(&data);
}

void CBattleground::EventCreate()
{
    OnCreate();
}

uint32 CBattleground::GetNameID()
{
    return 34;
}

int32 CBattleground::event_GetInstanceID()
{
    return m_mapMgr->GetInstanceID();
}

void CBattleground::EventCountdown()
{
    if (m_countdownStage == 1)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_countdownStage = 2;

        for (uint8 i = 0; i < 2; ++i)
        {
            for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
                if ((*itr) && (*itr)->GetSession())
                {
                    (*itr)->GetSession()->SystemMessage((*itr)->GetSession()->LocalizedWorldSrv(46), (*itr)->GetSession()->LocalizedWorldSrv(GetNameID()));
                }
        }

        // SendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "One minute until the battle for %s begins!", GetName());
    }
    else if (m_countdownStage == 2)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        m_countdownStage = 3;

        for (uint8 i = 0; i < 2; ++i)
        {
            for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
                if ((*itr) && (*itr)->GetSession())
                {
                    (*itr)->GetSession()->SystemMessage((*itr)->GetSession()->LocalizedWorldSrv(47), (*itr)->GetSession()->LocalizedWorldSrv(GetNameID()));
                }
        }

        //SendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "Thirty seconds until the battle for %s begins!", GetName());
    }
    else if (m_countdownStage == 3)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        m_countdownStage = 4;

        for (uint8 i = 0; i < 2; ++i)
        {
            for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
                if ((*itr) && (*itr)->GetSession())
                {
                    (*itr)->GetSession()->SystemMessage((*itr)->GetSession()->LocalizedWorldSrv(48), (*itr)->GetSession()->LocalizedWorldSrv(GetNameID()));
                }
        }

        //SendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "Fifteen seconds until the battle for %s begins!", GetName());
        sEventMgr.ModifyEventTime(this, EVENT_BATTLEGROUND_COUNTDOWN, 150);
        sEventMgr.ModifyEventTimeLeft(this, EVENT_BATTLEGROUND_COUNTDOWN, 15000);
    }
    else
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        for (uint8 i = 0; i < 2; ++i)
        {
            for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
                if ((*itr) && (*itr)->GetSession())
                {
                    (*itr)->GetSession()->SystemMessage((*itr)->GetSession()->LocalizedWorldSrv(49), (*itr)->GetSession()->LocalizedWorldSrv(GetNameID()));
                }
        }
        //SendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, "The battle for %s has begun!", GetName());
        sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_COUNTDOWN);
        this->StartBattleground();
    }
}

void CBattleground::OnStart()
{

}

void CBattleground::SetWorldState(uint32 Index, uint32 Value)
{
    if (m_zoneid == 0)
        return;

    m_mapMgr->GetWorldStatesHandler().SetWorldStateForZone(m_zoneid, 0, Index, Value);
}

void CBattleground::Close()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    /* remove all players from the battleground */
    m_ended = true;
    for (uint8 i = 0; i < 2; ++i)
    {
        std::set<Player*>::iterator itr;
        std::set<uint32>::iterator it2;
        uint32 guid;
        Player* plr;
        for (itr = m_players[i].begin(); itr != m_players[i].end();)
        {
            plr = *itr;
            ++itr;
            RemovePlayer(plr, false);
        }

        for (it2 = m_pendPlayers[i].begin(); it2 != m_pendPlayers[i].end();)
        {
            guid = *it2;
            ++it2;
            plr = objmgr.GetPlayer(guid);

            if (plr)
                RemovePendingPlayer(plr);
            else
                m_pendPlayers[i].erase(guid);
        }
    }

    /* call the virtual on close for cleanup etc */
    OnClose();

    /* shut down the map thread. this will delete the battleground from the current context. */
    m_mapMgr->SetThreadState(THREADSTATE_TERMINATE);
}

void CBattleground::OnClose()
{

}

Creature* CBattleground::SpawnSpiritGuide(float x, float y, float z, float o, uint32 horde)
{
    if (horde > 1)
        horde = 1;

    CreatureProperties const* pInfo = sMySQLStore.getCreatureProperties(13116 + horde);
    if (pInfo == nullptr)
    {
        return nullptr;
    }

    Creature* pCreature = m_mapMgr->CreateCreature(pInfo->Id);

    pCreature->Create(m_mapMgr->GetMapId(), x, y, z, o);

    pCreature->setEntry(13116 + horde);
    pCreature->setScale(1.0f);

    pCreature->setMaxHealth(10000);
    pCreature->setMaxPower(POWER_TYPE_MANA, 4868);
    pCreature->setMaxPower(POWER_TYPE_FOCUS, 200);
    pCreature->setMaxPower(POWER_TYPE_HAPPINESS, 2000000);

    pCreature->setHealth(100000);
    pCreature->setPower(POWER_TYPE_MANA, 4868);
    pCreature->setPower(POWER_TYPE_FOCUS, 200);
    pCreature->setPower(POWER_TYPE_HAPPINESS, 2000000);

    pCreature->setLevel(60);
    pCreature->SetFaction(84 - horde);

    pCreature->setRace(0);
    pCreature->setClass(2);
    pCreature->setGender(1);
    pCreature->setPowerType(0);

    pCreature->setVirtualItemSlotId(MELEE, 22802);

    pCreature->setUnitFlags(UNIT_FLAG_PLUS_MOB | UNIT_FLAG_IGNORE_PLAYER_COMBAT | UNIT_FLAG_IGNORE_PLAYER_NPC | UNIT_FLAG_PVP); // 4928

    pCreature->setBaseAttackTime(MELEE, 2000);
    pCreature->setBaseAttackTime(OFFHAND, 2000);
    pCreature->setBoundingRadius(0.208f);
    pCreature->setCombatReach(1.5f);

    pCreature->setDisplayId(13337 + horde);
    pCreature->setNativeDisplayId(13337 + horde);

    pCreature->setChannelSpellId(22011);
    pCreature->setModCastSpeed(1.0f);

    pCreature->setNpcFlags(UNIT_NPC_FLAG_SPIRITGUIDE);
    pCreature->setSheathType(SHEATH_STATE_MELEE);
    pCreature->setPvpFlags(U_FIELD_BYTES_FLAG_AURAS);

    pCreature->DisableAI();

    pCreature->SetCreatureProperties(sMySQLStore.getCreatureProperties(pInfo->Id));

    pCreature->PushToWorld(m_mapMgr);
    return pCreature;
}

Creature* CBattleground::SpawnSpiritGuide(LocationVector &v, uint32 faction)
{
    return SpawnSpiritGuide(v.x, v.y, v.z, v.o, faction);
}

uint32 CBattleground::GetLastResurrect()
{
    return m_lastResurrect;
}

void CBattleground::QueuePlayerForResurrect(Player* plr, Creature* spirit_healer)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::map<Creature*, std::set<uint32> >::iterator itr = m_resurrectMap.find(spirit_healer);
    if (itr != m_resurrectMap.end())
        itr->second.insert(plr->getGuidLow());
    plr->m_areaSpiritHealer_guid = spirit_healer->getGuid();
}

void CBattleground::RemovePlayerFromResurrect(Player* plr, Creature* spirit_healer)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::map<Creature*, std::set<uint32> >::iterator itr = m_resurrectMap.find(spirit_healer);
    if (itr != m_resurrectMap.end())
        itr->second.erase(plr->getGuidLow());
    plr->m_areaSpiritHealer_guid = 0;
}

void CBattleground::AddSpiritGuide(Creature* pCreature)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::map<Creature*, std::set<uint32> >::iterator itr = m_resurrectMap.find(pCreature);
    if (itr == m_resurrectMap.end())
    {
        std::set<uint32> ti;
        m_resurrectMap.insert(make_pair(pCreature, ti));
    }
}

void CBattleground::RemoveSpiritGuide(Creature* pCreature)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    m_resurrectMap.erase(pCreature);
}

void CBattleground::EventResurrectPlayers()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    Player* plr;
    std::set<uint32>::iterator itr;
    std::map<Creature*, std::set<uint32> >::iterator i;
    WorldPacket data(50);
    for (i = m_resurrectMap.begin(); i != m_resurrectMap.end(); ++i)
    {
        for (itr = i->second.begin(); itr != i->second.end(); ++itr)
        {
            plr = m_mapMgr->GetPlayer(*itr);
            if (plr && plr->isDead())
            {
                data.Initialize(SMSG_SPELL_START);
                data << plr->GetNewGUID();
                data << plr->GetNewGUID();
                data << uint32(RESURRECT_SPELL);
                data << uint8(0);
                data << uint16(0);
                data << uint32(0);
                data << uint16(2);
                data << plr->getGuid();
                plr->SendMessageToSet(&data, true);

                data.Initialize(SMSG_SPELL_GO);
                data << plr->GetNewGUID();
                data << plr->GetNewGUID();
                data << uint32(RESURRECT_SPELL);
                data << uint8(0);
                data << uint8(1);
                data << uint8(1);
                data << plr->getGuid();
                data << uint8(0);
                data << uint16(2);
                data << plr->getGuid();
                plr->SendMessageToSet(&data, true);

                plr->ResurrectPlayer();
                plr->setHealth(plr->getMaxHealth());
                plr->setPower(POWER_TYPE_MANA, plr->getMaxPower(POWER_TYPE_MANA));
                plr->setPower(POWER_TYPE_ENERGY, plr->getMaxPower(POWER_TYPE_ENERGY));
                plr->castSpell(plr, BG_REVIVE_PREPARATION, true);
            }
        }
        i->second.clear();
    }
    m_lastResurrect = static_cast<uint32>(UNIXTIME);
}

bool CBattleground::CanPlayerJoin(Player* plr, uint32 type)
{
    return HasFreeSlots(plr->getBgTeam(), type) && (GetLevelGrouping(plr->getLevel()) == GetLevelGroup()) && (!plr->HasAura(BG_DESERTER));
}

bool CBattleground::CreateCorpse(Player* /*plr*/)
{
    return true;
}

bool CBattleground::HookSlowLockOpen(GameObject* /*pGo*/, Player* /*pPlayer*/, Spell* /*pSpell*/)
{
    return false;
}

bool CBattleground::HookQuickLockOpen(GameObject* /*go*/, Player* /*player*/, Spell* /*spell*/)
{
    return false;
}

void CBattleground::QueueAtNearestSpiritGuide(Player* plr, Creature* old)
{
    float dd;
    float dist = 999999.0f;
    Creature* cl = nullptr;
    std::set<uint32> *closest = nullptr;
    m_lock.Acquire();
    std::map<Creature*, std::set<uint32> >::iterator itr = m_resurrectMap.begin();
    for (; itr != m_resurrectMap.end(); ++itr)
    {
        if (itr->first == old)
            continue;

        dd = plr->GetDistance2dSq(itr->first) < dist;
        if (dd < dist)
        {
            cl = itr->first;
            closest = &itr->second;
            dist = dd;
        }
    }

    if (closest != nullptr)
    {
        closest->insert(plr->getGuidLow());
        plr->m_areaSpiritHealer_guid = cl->getGuid();
        plr->castSpell(plr, 2584, true);
    }

    m_lock.Release();
}

uint64 CBattleground::GetFlagHolderGUID(uint32 /*faction*/) const
{
    return 0;
}

uint32 CBattleground::GetFreeSlots(uint32 t, uint32 type)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    uint32 maxPlayers = BattlegroundManager.GetMaximumPlayers(type);

    size_t s = maxPlayers - m_players[t].size() - m_pendPlayers[t].size();
    return static_cast<uint32>(s);
}

bool CBattleground::HasFreeSlots(uint32 Team, uint32 type)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    bool res;
    uint32 maxPlayers = BattlegroundManager.GetMaximumPlayers(type);

    if (isArena(type))
    {
        res = (static_cast<uint32>(m_players[Team].size()) + m_pendPlayers[Team].size() < maxPlayers);
    }
    else
    {
        uint32 size[2];
        size[0] = uint32(m_players[0].size() + m_pendPlayers[0].size());
        size[1] = uint32(m_players[1].size() + m_pendPlayers[1].size());
        res = (size[Team] < maxPlayers) && ((static_cast<int>(size[Team]) - static_cast<int>(size[1 - Team])) <= 0);
    }
    return res;
}
