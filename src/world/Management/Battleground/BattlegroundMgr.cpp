/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "Management/Battleground/Battleground.h"
#include "Management/Battleground/BattlegroundMgr.h"
#include "Management/Arenas.h"
#include "Management/ArenaTeam.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreator.h"
#include "Chat/ChatHandler.hpp"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/SmsgArenaError.h"
#include "Server/Packets/CmsgBattlemasterJoin.h"
#include "Server/Packets/SmsgGroupJoinedBattleground.h"
#include "Server/Packets/SmsgBattlefieldStatus.h"
#include "Storage/WorldStrings.h"

using namespace AscEmu::Packets;

CBattlegroundManager& CBattlegroundManager::getInstance()
{
    static CBattlegroundManager mInstance;
    return mInstance;
}

void CBattlegroundManager::initialize()
{
    // Yes we will be running from WorldRunnable
    m_holder = sEventMgr.GetEventHolder(WORLD_INSTANCE);

    sEventMgr.AddEvent(this, &CBattlegroundManager::EventQueueUpdate, EVENT_BATTLEGROUND_QUEUE_UPDATE, 15000, 0, 0);

    for (uint32_t i = 0; i < BATTLEGROUND_NUM_TYPES; ++i)
    {
        m_instances[i].clear();
        m_maxBattlegroundId[i] = 0;
    }

    // These battlegrounds will be available in Random Battleground queue
    avalibleInRandom.push_back(BATTLEGROUND_ALTERAC_VALLEY);
    avalibleInRandom.push_back(BATTLEGROUND_WARSONG_GULCH);
    avalibleInRandom.push_back(BATTLEGROUND_ARATHI_BASIN);
    avalibleInRandom.push_back(BATTLEGROUND_EYE_OF_THE_STORM);
    avalibleInRandom.push_back(BATTLEGROUND_STRAND_OF_THE_ANCIENT);
    avalibleInRandom.push_back(BATTLEGROUND_ISLE_OF_CONQUEST);
}

void CBattlegroundManager::RegisterBgFactory(uint32 map, BattlegroundFactoryMethod method)
{
    const std::map< uint32, BattlegroundFactoryMethod >::iterator itr = bgFactories.find(map);
    if (itr != bgFactories.end())
        return;

    bgFactories[map] = method;
}

void CBattlegroundManager::RegisterArenaFactory(uint32 map, ArenaFactoryMethod method)
{
    const std::vector< uint32 >::iterator itr = std::find(arenaMaps.begin(), arenaMaps.end(), map);
    if (itr != arenaMaps.end())
        return;
    arenaMaps.push_back(map);
    arenaFactories.push_back(method);
}

void CBattlegroundManager::RegisterMapForBgType(uint32 type, uint32 map)
{
    const std::map< uint32, uint32 >::iterator itr = bgMaps.find(type);
    if (itr != bgMaps.end())
        return;

    bgMaps[type] = map;
}

void CBattlegroundManager::HandleBattlegroundListPacket(WorldSession* m_session, uint32 BattlegroundType, uint8 from)
{
#if VERSION_STRING == Cata
    //todo: correct packet 
    WorldPacket data(SMSG_BATTLEFIELD_LIST, 18);

    ObjectGuid guid;

    // Send 0 instead of GUID when using the BG UI instead of Battlemaster
    if (from == 0)
        guid = m_session->GetPlayer()->getGuid();
    else
        guid = 0;

    data << uint32_t(0);
    data << uint32_t(0);
    data << uint32_t(0);
    data << uint32_t(BattlegroundType);
    data << uint32_t(0);
    data << uint32_t(0);
    data << uint32_t(0);
    data << uint8(0);                                      // unk
    data << uint8(0);                                      // unk

    // new

    // Rewards
    data << uint8(0);                                      // 3.3.3 hasWin
    data << uint32(0);                                     // 3.3.3 winHonor
    data << uint32(0);                                     // 3.3.3 winArena
    data << uint32(0);                                     // 3.3.3 lossHonor

    uint8 isRandom = BattlegroundType == BATTLEGROUND_RANDOM;
    data << uint8(isRandom);                               // 3.3.3 isRandom

    // Random bgs
    if (isRandom == 1)
    {
        auto hasWonRbgToday = m_session->GetPlayer()->HasWonRbgToday();
        uint32 honorPointsForWinning, honorPointsForLosing, arenaPointsForWinning, arenaPointsForLosing;

        m_session->GetPlayer()->FillRandomBattlegroundReward(true, honorPointsForWinning, arenaPointsForWinning);
        m_session->GetPlayer()->FillRandomBattlegroundReward(false, honorPointsForLosing, arenaPointsForLosing);

        // rewards
        data << uint8(hasWonRbgToday);
        data << uint32(honorPointsForWinning);
        data << uint32(arenaPointsForWinning);
        data << uint32(honorPointsForLosing);
    }

    if (isArena(BattlegroundType))
    {
        data << uint32(0);
        m_session->SendPacket(&data);
        return;
    }

    if (BattlegroundType >= BATTLEGROUND_NUM_TYPES)     //VLack: Nasty hackers might try to abuse this packet to crash us...
        return;

    uint32 Count = 0;
    size_t pos = data.wpos();

    data << uint32(0);      // Count

    // Append the battlegrounds
    m_instanceLock.Acquire();
    for (std::map<uint32, CBattleground*>::iterator itr = m_instances[BattlegroundType].begin(); itr != m_instances[BattlegroundType].end(); ++itr)
    {
        if (itr->second->CanPlayerJoin(m_session->GetPlayer(), BattlegroundType) && !itr->second->HasEnded())
        {
            data << uint32(itr->first);
            ++Count;
        }
    }
    m_instanceLock.Release();

    data.put< uint32 >(pos, Count);

    m_session->SendPacket(&data);

#endif
#if VERSION_STRING == WotLK
    WorldPacket data(SMSG_BATTLEFIELD_LIST, 18);

    // Send 0 instead of GUID when using the BG UI instead of Battlemaster
    if (from == 0)
        data << uint64(m_session->GetPlayer()->getGuid());
    else
        data << uint64(0);

    data << from;
    data << uint32(BattlegroundType);   // typeid
    data << uint8(0);                                      // unk
    data << uint8(0);                                      // unk

    // Rewards
    data << uint8(0);                                      // 3.3.3 hasWin
    data << uint32(0);                                     // 3.3.3 winHonor
    data << uint32(0);                                     // 3.3.3 winArena
    data << uint32(0);                                     // 3.3.3 lossHonor

    uint8 isRandom = BattlegroundType == BATTLEGROUND_RANDOM;
    data << uint8(isRandom);                               // 3.3.3 isRandom

    // Random bgs
    if (isRandom == 1)
    {
        auto hasWonRbgToday = m_session->GetPlayer()->HasWonRbgToday();
        uint32 honorPointsForWinning, honorPointsForLosing, arenaPointsForWinning, arenaPointsForLosing;

        m_session->GetPlayer()->FillRandomBattlegroundReward(true, honorPointsForWinning, arenaPointsForWinning);
        m_session->GetPlayer()->FillRandomBattlegroundReward(false, honorPointsForLosing, arenaPointsForLosing);

        // rewards
        data << uint8(hasWonRbgToday);
        data << uint32(honorPointsForWinning);
        data << uint32(arenaPointsForWinning);
        data << uint32(honorPointsForLosing);
    }

    if (isArena(BattlegroundType))
    {
        data << uint32(0);
        m_session->SendPacket(&data);
        return;
    }

    if (BattlegroundType >= BATTLEGROUND_NUM_TYPES)     //VLack: Nasty hackers might try to abuse this packet to crash us...
        return;

    uint32 Count = 0;
    size_t pos = data.wpos();

    data << uint32(0);      // Count

    // Append the battlegrounds
    m_instanceLock.Acquire();
    for (std::map<uint32, CBattleground*>::iterator itr = m_instances[BattlegroundType].begin(); itr != m_instances[BattlegroundType].end(); ++itr)
    {
        if (itr->second->CanPlayerJoin(m_session->GetPlayer(), BattlegroundType) && !itr->second->HasEnded())
        {
            data << uint32(itr->first);
            ++Count;
        }
    }
    m_instanceLock.Release();

    data.put< uint32 >(pos, Count);

    m_session->SendPacket(&data);
#endif
}

void CBattlegroundManager::HandleBattlegroundJoin(WorldSession* m_session, WorldPacket& pck)
{
    Player* plr = m_session->GetPlayer();
    const uint32 pguid = plr->getGuidLow();
    const uint32 lgroup = GetLevelGrouping(plr->getLevel());

    CmsgBattlemasterJoin srlPacket;
    if (!srlPacket.deserialise(pck))
        return;

    if (srlPacket.bgType == BATTLEGROUND_RANDOM)
        plr->SetQueuedForRbg(true);
    else
        plr->SetQueuedForRbg(false);

    if (srlPacket.bgType >= BATTLEGROUND_NUM_TYPES || srlPacket.bgType == 0 || bgMaps.find(srlPacket.bgType) == bgMaps.end() && srlPacket.bgType != BATTLEGROUND_RANDOM)
    {
        sCheatLog.writefromsession(m_session, "tried to crash the server by joining battleground that does not exist (0)");
        m_session->Disconnect();
        return;
    }

    if (srlPacket.instanceId)
    {
        // We haven't picked the first instance. This means we've specified an instance to join
        m_instanceLock.Acquire();
        const auto itr = m_instances[srlPacket.bgType].find(srlPacket.instanceId);
        if (itr == m_instances[srlPacket.bgType].end())
        {
            sChatHandler.SystemMessage(m_session, m_session->LocalizedWorldSrv(ServerString::SS_JOIN_INVALID_INSTANCE));
            m_instanceLock.Release();
            return;
        }

        m_instanceLock.Release();
    }

    // Queue him!
    m_queueLock.Acquire();
    m_queuedPlayers[srlPacket.bgType][lgroup].push_back(pguid);
    LogNotice("BattlegroundManager : Player %u is now in battleground queue for instance %u", m_session->GetPlayer()->getGuidLow(), (srlPacket.instanceId + 1));

    plr->m_bgIsQueued = true;
    plr->m_bgQueueInstanceId = srlPacket.instanceId;
    plr->m_bgQueueType = srlPacket.bgType;

    plr->setBGEntryPoint(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetOrientation(), plr->GetMapId(), plr->GetInstanceID());

    SendBattlefieldStatus(plr, BGSTATUS_INQUEUE, srlPacket.bgType, srlPacket.instanceId, 0, bgMaps[srlPacket.bgType], 0);

    m_queueLock.Release();
}

void ErasePlayerFromList(uint32 guid, std::list<uint32>* l)
{
    for (std::list<uint32>::iterator itr = l->begin(); itr != l->end(); ++itr)
    {
        if (*itr == guid)
        {
            l->erase(itr);
            return;
        }
    }
}

uint8 GetBattlegroundCaption(BattleGroundTypes bgType)
{
    switch (bgType)
    {
        case BATTLEGROUND_ALTERAC_VALLEY:
            return 38;
        case BATTLEGROUND_WARSONG_GULCH:
            return 39;
        case BATTLEGROUND_ARATHI_BASIN:
            return 40;
        case BATTLEGROUND_ARENA_2V2:
            return 41;
        case BATTLEGROUND_ARENA_3V3:
            return 42;
        case BATTLEGROUND_ARENA_5V5:
            return 43;
        case BATTLEGROUND_EYE_OF_THE_STORM:
            return 44;
        case BATTLEGROUND_STRAND_OF_THE_ANCIENT:
            return 34;
        default:
            return 45;
    }
}

void CBattlegroundManager::HandleGetBattlegroundQueueCommand(WorldSession* m_session)
{
    std::stringstream ss;

    m_queueLock.Acquire();

    bool foundSomething = false;

    for (uint32_t i = 0; i < BATTLEGROUND_NUM_TYPES; ++i)
    {
        for (uint8 j = 0; j < MAX_LEVEL_GROUP; ++j)
        {
            if (m_queuedPlayers[i][j].empty())
                continue;

            foundSomething = true;

            ss << m_session->LocalizedWorldSrv(GetBattlegroundCaption(static_cast<BattleGroundTypes>(i)));

            switch (j)
            {
                case 0:
                    ss << " (<10)";
                    break;
                case 1:
                    ss << " (<20)";
                    break;
                case 2:
                    ss << " (<30)";
                    break;
                case 3:
                    ss << " (<40)";
                    break;
                case 4:
                    ss << " (<50)";
                    break;
                case 5:
                    ss << " (<60)";
                    break;
                case 6:
                    ss << " (<70)";
                    break;
                case 7:
                    ss << " (<80)";
                    break;
            }

            ss << ": ";

            ss << static_cast<uint32>(m_queuedPlayers[i][j].size()) << " players queued";

            if (!isArena(i))
            {
                int ally = 0, horde = 0;

                for (std::list<uint32>::iterator it3 = m_queuedPlayers[i][j].begin(); it3 != m_queuedPlayers[i][j].end();)
                {
                    std::list<uint32>::iterator it4 = it3++;
                    Player* plr = sObjectMgr.GetPlayer(*it4);
                    if (!plr || GetLevelGrouping(plr->getLevel()) != j)
                        continue;

                    if (plr->isTeamAlliance())
                        ally++;
                    else
                        horde++;
                }

                ss << " (Alliance: " << ally << " Horde: " << horde;
                if (static_cast<int>(m_queuedPlayers[i][j].size()) > (ally + horde))
                    ss << " Unknown: " << (static_cast<int>(m_queuedPlayers[i][j].size()) - ally - horde);
                ss << ")";
            }

            m_session->SystemMessage(ss.str().c_str());
            ss.rdbuf()->str("");
        }

        if (isArena(i))
        {
            if (!m_queuedGroups[i].empty())
            {
                foundSomething = true;

                ss << m_session->LocalizedWorldSrv(GetBattlegroundCaption(static_cast<BattleGroundTypes>(i))) << " (rated): ";
                ss << static_cast<uint32>(m_queuedGroups[i].size()) << " groups queued";

                m_session->SystemMessage(ss.str().c_str());
                ss.rdbuf()->str("");
            }
        }
    }

    m_queueLock.Release();

    if (!foundSomething)
        m_session->SystemMessage("There's nobody queued.");
}

void CBattlegroundManager::EventQueueUpdate()
{
    this->EventQueueUpdate(false);
}

uint32 CBattlegroundManager::GetArenaGroupQInfo(Group* group, int type, uint32* avgRating)
{
    uint32 count = 0;
    uint32 rating = 0;

    if (group == nullptr || group->GetLeader() == nullptr)
        return 0;

    Player* plr = group->GetLeader()->m_loggedInPlayer;
    if (plr == nullptr)
        return 0;

    ArenaTeam* team = plr->m_arenaTeams[type - BATTLEGROUND_ARENA_2V2];
    if (team == nullptr)
        return 0;

    for (GroupMembersSet::iterator itx = group->GetSubGroup(0)->GetGroupMembersBegin(); itx != group->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
    {
        plr = (*itx)->m_loggedInPlayer;
        if (plr)
        {
            if (team == plr->m_arenaTeams[type - BATTLEGROUND_ARENA_2V2])
            {
                ArenaTeamMember* atm = team->GetMemberByGuid(plr->getGuidLow());
                if (atm)
                {
                    rating += atm->PersonalRating;
                    count++;
                }
            }
        }
    }

    *avgRating = count > 0 ? rating / count : 0;

    return team->m_id;
}

void CBattlegroundManager::AddGroupToArena(CBattleground* bg, Group* group, uint32 nteam)
{
    if (group == nullptr || group->GetLeader() == nullptr)
        return;

    Player* plr = group->GetLeader()->m_loggedInPlayer;
    if (plr == nullptr)
        return;

    ArenaTeam* team = plr->m_arenaTeams[bg->GetType() - BATTLEGROUND_ARENA_2V2];
    if (team == nullptr)
        return;

    for (GroupMembersSet::iterator itx = group->GetSubGroup(0)->GetGroupMembersBegin(); itx != group->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
    {
        plr = (*itx)->m_loggedInPlayer;
        if (plr && team == plr->m_arenaTeams[bg->GetType() - BATTLEGROUND_ARENA_2V2])
        {
            if (bg->HasFreeSlots(nteam, bg->GetType()))
            {
                bg->AddPlayer(plr, nteam);
                plr->setTeam(nteam);
            }
        }
    }
}

int CBattlegroundManager::CreateArenaType(int type, Group* group1, Group* group2)
{
    Arena* ar = dynamic_cast<Arena*>(CreateInstance(type, LEVEL_GROUP_70));
    if (ar == nullptr)
    {
        LOG_ERROR("%s (%u): Couldn't create Arena Instance", __FILE__, __LINE__);
        m_queueLock.Release();
        m_instanceLock.Release();
        return -1;
    }
    ar->rated_match = true;

    AddGroupToArena(ar, group1, TEAM_ALLIANCE);
    AddGroupToArena(ar, group2, TEAM_HORDE);

    return 0;
}

void CBattlegroundManager::AddPlayerToBg(CBattleground* bg, std::deque<uint32> *playerVec, uint32 i, uint32 j)
{
    uint32 plrguid = *playerVec->begin();
    playerVec->pop_front();

    if (Player* plr = sObjectMgr.GetPlayer(plrguid))
    {
        if (bg->CanPlayerJoin(plr, bg->GetType()))
        {
            bg->AddPlayer(plr, plr->getTeam());
            ErasePlayerFromList(plr->getGuidLow(), &m_queuedPlayers[i][j]);
        }
        else
        {
            // Put again the player in the queue
            playerVec->push_back(plrguid);
        }
    }
    else
    {
        ErasePlayerFromList(plrguid, &m_queuedPlayers[i][j]);
    }
}

void CBattlegroundManager::AddPlayerToBgTeam(CBattleground* bg, std::deque<uint32> *playerVec, uint32 i, uint32 j, int Team)
{
    if (bg->HasFreeSlots(Team, bg->GetType()))
    {
        uint32 plrguid = *playerVec->begin();
        playerVec->pop_front();

        if (Player* plr = sObjectMgr.GetPlayer(plrguid))
        {
            plr->setBgTeam(Team);
            bg->AddPlayer(plr, Team);
        }
        ErasePlayerFromList(plrguid, &m_queuedPlayers[i][j]);
    }
}

void CBattlegroundManager::EventQueueUpdate(bool forceStart)
{
    std::deque<uint32> tempPlayerVec[2];

    Player* plr;
    CBattleground* bg;

    std::list<uint32>::iterator it3, it4;
    std::map<uint32, CBattleground*>::iterator iitr;

    Arena* arena;

    int32 team;
    uint32 plrguid;
    uint32 factionMap[MAX_PLAYER_TEAMS];
    uint32 count;

    std::queue<uint32> teams[MAX_PLAYER_TEAMS];

    m_queueLock.Acquire();
    m_instanceLock.Acquire();

    for (uint32_t i = 0; i < BATTLEGROUND_NUM_TYPES; ++i)
    {
        for (uint8 j = 0; j < MAX_LEVEL_GROUP; ++j)
        {
            if (m_queuedPlayers[i][j].empty())
                continue;

            tempPlayerVec[0].clear();
            tempPlayerVec[1].clear();

            // We try to add the players who queued for a specific Bg/Arena instance to
            // the Bg/Arena where they queued to, and add the rest to another list
            for (it3 = m_queuedPlayers[i][j].begin(); it3 != m_queuedPlayers[i][j].end();)
            {
                it4 = it3++;
                plrguid = *it4;
                plr = sObjectMgr.GetPlayer(plrguid);

                // Player has left the game or switched level group since queuing (by leveling for example)
                if (!plr || GetLevelGrouping(plr->getLevel()) != j)
                {
                    m_queuedPlayers[i][j].erase(it4);
                    continue;
                }

                // queued to a specific instance id?
                if (plr->m_bgQueueInstanceId != 0)
                {
                    iitr = m_instances[i].find(plr->m_bgQueueInstanceId);
                    if (iitr == m_instances[i].end())
                    {
                        // queue no longer valid, since instance has closed since queuing
                        plr->GetSession()->SystemMessage(plr->GetSession()->LocalizedWorldSrv(ServerString::SS_QUEUE_BG_INSTANCE_ID_NO_VALID_DELETED), plr->m_bgQueueInstanceId);
                        plr->m_bgIsQueued = false;
                        plr->m_bgQueueType = 0;
                        plr->m_bgQueueInstanceId = 0;
                        m_queuedPlayers[i][j].erase(it4);
                        continue;
                    }

                    // can we join the specified Bg instance?
                    bg = iitr->second;
                    if (bg->CanPlayerJoin(plr, bg->GetType()))
                    {
                        bg->AddPlayer(plr, plr->getTeam());
                        m_queuedPlayers[i][j].erase(it4);
                    }
                }
                else
                {
                    if (isArena(i))
                        tempPlayerVec[plr->getTeam()].push_back(plrguid);
                    else if (!plr->HasAura(BG_DESERTER))
                        tempPlayerVec[plr->getTeam()].push_back(plrguid);
                }
            }


            /// Now that we have a list of players who didn't queue for a specific instance
            /// try to add them to a Bg/Arena that is already under way
            std::vector<uint32> tryJoinVec;
            if (i == BATTLEGROUND_RANDOM)
            {
                tryJoinVec = avalibleInRandom;
            }
            else
            {
                tryJoinVec.push_back(i);
            }

            for (uint32 bgIndex = 0; bgIndex < tryJoinVec.size(); bgIndex++)
            {
                uint32 tmpJoinBgType = tryJoinVec[bgIndex];

                for (iitr = m_instances[tmpJoinBgType].begin(); iitr != m_instances[tmpJoinBgType].end(); ++iitr)
                {
                    if (iitr->second->HasEnded() || iitr->second->GetLevelGroup() != j)
                        continue;

                    if (isArena(i))
                    {
                        arena = dynamic_cast<Arena*>(iitr->second);
                        if (arena->Rated())
                            continue;

                        factionMap[0] = arena->GetTeamFaction(0);
                        factionMap[1] = arena->GetTeamFaction(1);

                        team = arena->GetFreeTeam();
                        while ((team >= 0) && (!tempPlayerVec[factionMap[team]].empty()))
                        {
                            plrguid = *tempPlayerVec[factionMap[team]].begin();
                            tempPlayerVec[factionMap[team]].pop_front();
                            plr = sObjectMgr.GetPlayer(plrguid);
                            if (plr)
                            {
                                plr->setBgTeam(team);
                                arena->AddPlayer(plr, team);
                                team = arena->GetFreeTeam();
                            }
                            ErasePlayerFromList(plrguid, &m_queuedPlayers[i][j]);
                        }
                    }
                    else
                    {
                        bg = iitr->second;
                        int size = static_cast<int>(std::min(tempPlayerVec[0].size(), tempPlayerVec[1].size()));
                        for (int counter = 0; (counter < size) && (bg->IsFull() == false); counter++)
                        {
                            AddPlayerToBgTeam(bg, &tempPlayerVec[0], i, j, 0);
                            AddPlayerToBgTeam(bg, &tempPlayerVec[1], i, j, 1);
                        }

                        while (!tempPlayerVec[0].empty() && bg->HasFreeSlots(0, bg->GetType()))
                        {
                            AddPlayerToBgTeam(bg, &tempPlayerVec[0], i, j, 0);
                        }
                        while (!tempPlayerVec[1].empty() && bg->HasFreeSlots(1, bg->GetType()))
                        {
                            AddPlayerToBgTeam(bg, &tempPlayerVec[1], i, j, 1);
                        }
                    }
                }
            }

            // Now that that we added everyone we could to a running Bg/Arena
            // We shall see if we can start a new one!
            if (isArena(i))
            {
                // enough players to start a round?
                uint32 minPlayers = sBattlegroundManager.GetMinimumPlayers(i);
                if (!forceStart && ((tempPlayerVec[0].size() + tempPlayerVec[1].size()) < (minPlayers * 2)))
                    continue;

                if (CanCreateInstance(i, j))
                {
                    arena = dynamic_cast<Arena*>(CreateInstance(i, j));
                    if (arena == nullptr)
                    {
                        LOG_ERROR("%s (%u): Couldn't create Arena Instance", __FILE__, __LINE__);
                        m_queueLock.Release();
                        m_instanceLock.Release();
                        return;
                    } // No alliance in the queue
                    if (tempPlayerVec[0].empty())
                    {
                        count = GetMaximumPlayers(i) * 2;
                        while ((count > 0) && (!tempPlayerVec[1].empty()))
                        {
                            if (teams[0].size() > teams[1].size())
                                teams[1].push(tempPlayerVec[1].front());
                            else
                                teams[0].push(tempPlayerVec[1].front());
                            tempPlayerVec[1].pop_front();
                            count--;
                        }
                    }
                    else // No horde in the queue
                    {
                        if (tempPlayerVec[1].empty())
                        {
                            count = GetMaximumPlayers(i) * 2;
                            while ((count > 0) && (!tempPlayerVec[0].empty()))
                            {
                                if (teams[0].size() > teams[1].size())
                                    teams[1].push(tempPlayerVec[0].front());
                                else
                                    teams[0].push(tempPlayerVec[0].front());
                                tempPlayerVec[0].pop_front();
                                count--;
                            }
                        }
                        else // There are both alliance and horde players in the queue
                        {
                            count = GetMaximumPlayers(i);
                            while ((count > 0) && (!tempPlayerVec[0].empty()) && (!tempPlayerVec[1].empty()))
                            {
                                teams[0].push(tempPlayerVec[0].front());
                                teams[1].push(tempPlayerVec[1].front());
                                tempPlayerVec[0].pop_front();
                                tempPlayerVec[1].pop_front();
                                count--;
                            }
                        }
                    }

                    // Now we just need to add the players to the Arena instance
                    while (!teams[0].empty())
                    {
                        for (uint32 localeTeam = 0; localeTeam < 2; localeTeam++)
                        {
                            plrguid = teams[localeTeam].front();
                            teams[localeTeam].pop();
                            plr = sObjectMgr.GetPlayer(plrguid);
                            if (plr == nullptr)
                                continue;

                            plr->setBgTeam(localeTeam);
                            arena->AddPlayer(plr, plr->getBgTeam());
                            // remove from the main queue (painful!)
                            ErasePlayerFromList(plr->getGuidLow(), &m_queuedPlayers[i][j]);
                        }
                    }
                }
            }
            else
            {
                uint32 bgToStart = i;
                if (i == BATTLEGROUND_RANDOM)
                {
                    if (!forceStart)
                    {
                        std::vector<uint32> bgPossible;
                        for (uint32 bgIndex = 0; bgIndex < avalibleInRandom.size(); bgIndex++)
                        {
                            uint32 tmpJoinBgType = avalibleInRandom[bgIndex];

                            uint32 minPlayers = sBattlegroundManager.GetMinimumPlayers(tmpJoinBgType);
                            if ((tempPlayerVec[0].size() >= minPlayers && tempPlayerVec[1].size() >= minPlayers))
                            {
                                bgPossible.push_back(tmpJoinBgType);
                            }
                        }

                        if (!bgPossible.empty())
                        {
                            uint32 num = Util::getRandomUInt(0, static_cast<uint32>(bgPossible.size() - 1));
                            bgToStart = bgPossible[num];
                        }
                    }
                    else
                    {
                        uint32 num = Util::getRandomUInt(0, static_cast<uint32>(avalibleInRandom.size() - 1));
                        bgToStart = avalibleInRandom[num];
                    }
                }


                uint32 minPlayers = sBattlegroundManager.GetMinimumPlayers(bgToStart);
                if (forceStart || ((tempPlayerVec[0].size() >= minPlayers && tempPlayerVec[1].size() >= minPlayers) && bgToStart != BATTLEGROUND_RANDOM))
                {
                    if (CanCreateInstance(bgToStart, j))
                    {
                        bg = CreateInstance(bgToStart, j);
                        if (bg == nullptr)
                        {
                            m_queueLock.Release();
                            m_instanceLock.Release();
                            return;
                        }

                        // push as many as possible in
                        if (forceStart)
                        {
                            for (uint8 k = 0; k < 2; ++k)
                            {
                                while (!tempPlayerVec[k].empty() && bg->HasFreeSlots(k, bg->GetType()))
                                {
                                    AddPlayerToBgTeam(bg, &tempPlayerVec[k], i, j, k);
                                }
                            }
                        }
                        else
                        {
                            int size = static_cast<int>(std::min(tempPlayerVec[0].size(), tempPlayerVec[1].size()));
                            for (int counter = 0; (counter < size) && (bg->IsFull() == false); counter++)
                            {
                                AddPlayerToBgTeam(bg, &tempPlayerVec[0], i, j, 0);
                                AddPlayerToBgTeam(bg, &tempPlayerVec[1], i, j, 1);
                            }
                        }
                    }
                }
            }
        }
    }

    // Handle paired arena team joining
    Group* group1, *group2;
    uint32 teamids[2] = { 0, 0 };
    uint32 avgRating[2] = { 0, 0 };
    uint32 n;
    std::list<uint32>::iterator itz;
    for (uint8 i = BATTLEGROUND_ARENA_2V2; i <= BATTLEGROUND_ARENA_5V5; ++i)
    {
        if (!forceStart && m_queuedGroups[i].size() < 2)      // got enough to have an arena battle ;P
        {
            continue;
        }

        for (uint32 j = 0; j < static_cast<uint32>(m_queuedGroups[i].size()); j++)
        {
            group1 = group2 = nullptr;
            n = Util::getRandomUInt(static_cast<uint32>(m_queuedGroups[i].size())) - 1;
            for (itz = m_queuedGroups[i].begin(); itz != m_queuedGroups[i].end() && n > 0; ++itz)
                --n;

            if (itz == m_queuedGroups[i].end())
                itz = m_queuedGroups[i].begin();

            if (itz == m_queuedGroups[i].end())
            {
                LOG_ERROR("Internal error at %s:%u", __FILE__, __LINE__);
                m_queueLock.Release();
                m_instanceLock.Release();
                return;
            }

            group1 = sObjectMgr.GetGroupById(*itz);
            if (group1 == nullptr)
            {
                continue;
            }

            if (forceStart && m_queuedGroups[i].size() == 1)
            {
                if (CreateArenaType(i, group1, nullptr) == -1)
                    return;

                m_queuedGroups[i].remove(group1->GetID());
                continue;
            }

            teamids[0] = GetArenaGroupQInfo(group1, i, &avgRating[0]);

            std::list<uint32> possibleGroups;
            for (itz = m_queuedGroups[i].begin(); itz != m_queuedGroups[i].end(); ++itz)
            {
                group2 = sObjectMgr.GetGroupById(*itz);
                if (group2)
                {
                    teamids[1] = GetArenaGroupQInfo(group2, i, &avgRating[1]);
                    uint32 delta = abs(static_cast<int32>(avgRating[0]) - static_cast<int32>(avgRating[1]));
                    if (teamids[0] != teamids[1] && delta <= worldConfig.rate.arenaQueueDiff)
                    {
                        possibleGroups.push_back(group2->GetID());
                    }
                }
            }

            if (!possibleGroups.empty())
            {
                n = Util::getRandomUInt(static_cast<uint32>(possibleGroups.size())) - 1;
                for (itz = possibleGroups.begin(); itz != possibleGroups.end() && n > 0; ++itz)
                    --n;

                if (itz == possibleGroups.end())
                    itz = possibleGroups.begin();

                if (itz == possibleGroups.end())
                {
                    LOG_ERROR("Internal error at %s:%u", __FILE__, __LINE__);
                    m_queueLock.Release();
                    m_instanceLock.Release();
                    return;
                }

                group2 = sObjectMgr.GetGroupById(*itz);
                if (group2)
                {
                    if (CreateArenaType(i, group1, group2) == -1) return;
                    m_queuedGroups[i].remove(group1->GetID());
                    m_queuedGroups[i].remove(group2->GetID());
                }
            }
        }
    }

    m_queueLock.Release();
    m_instanceLock.Release();
}

void CBattlegroundManager::RemovePlayerFromQueues(Player* plr)
{
    ARCEMU_ASSERT(plr->m_bgQueueType < BATTLEGROUND_NUM_TYPES);

    m_queueLock.Acquire();

    sEventMgr.RemoveEvents(plr, EVENT_BATTLEGROUND_QUEUE_UPDATE);

    uint32 lgroup = GetLevelGrouping(plr->getLevel());

    std::list<uint32>::iterator itr = m_queuedPlayers[plr->m_bgQueueType][lgroup].begin();
    while (itr != m_queuedPlayers[plr->m_bgQueueType][lgroup].end())
    {
        if ((*itr) == plr->getGuidLow())
        {
            LOG_DEBUG("Removing player %u from queue instance %u type %u", plr->getGuidLow(), plr->m_bgQueueInstanceId, plr->m_bgQueueType);
            m_queuedPlayers[plr->m_bgQueueType][lgroup].erase(itr);
            break;
        }

        ++itr;
    }

    plr->m_bgIsQueued = false;
    plr->setBgTeam(plr->getTeam());
    plr->m_pendingBattleground = nullptr;
    SendBattlefieldStatus(plr, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
    m_queueLock.Release();

    if (Group* group = plr->getGroup())
    {
        LOG_DEBUG("Player %u removed whilst in a group. Removing players group %u from queue", plr->getGuidLow(), group->GetID());
        RemoveGroupFromQueues(group);
    }
}

void CBattlegroundManager::RemoveGroupFromQueues(Group* grp)
{
    m_queueLock.Acquire();
    for (uint32 i = BATTLEGROUND_ARENA_2V2; i < BATTLEGROUND_ARENA_5V5 + 1; ++i)
    {
        for (std::list<uint32>::iterator itr = m_queuedGroups[i].begin(); itr != m_queuedGroups[i].end();)
        {
            if ((*itr) == grp->GetID())
                itr = m_queuedGroups[i].erase(itr);
            else
                ++itr;
        }
    }

    for (GroupMembersSet::iterator itr = grp->GetSubGroup(0)->GetGroupMembersBegin(); itr != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itr)
        if ((*itr)->m_loggedInPlayer)
            SendBattlefieldStatus((*itr)->m_loggedInPlayer, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);

    m_queueLock.Release();
}


bool CBattlegroundManager::CanCreateInstance(uint32 /*Type*/, uint32 /*LevelGroup*/)
{
    /*uint32 lc = 0;
    for (map<uint32, CBattleground*>::iterator itr = m_instances[Type].begin(); itr != m_instances[Type].end(); ++itr)
    {
    if (itr->second->GetLevelGroup() == LevelGroup)
    {
    lc++;
    if (lc >= MAXIMUM_BATTLEGROUNDS_PER_LEVEL_GROUP)
    return false;
    }
    }*/

    return true;
}

/// Returns the minimum number of players (Only valid for battlegrounds)
uint32 CBattlegroundManager::GetMinimumPlayers(uint32 dbcIndex)
{
    switch (dbcIndex)
    {
        case BATTLEGROUND_ALTERAC_VALLEY:
            return worldConfig.bg.minPlayerCountAlteracValley;
        case BATTLEGROUND_WARSONG_GULCH:
            return worldConfig.bg.minPlayerCountWarsongGulch;
        case BATTLEGROUND_ARATHI_BASIN:
            return worldConfig.bg.minPlayerCountArathiBasin;
        case BATTLEGROUND_EYE_OF_THE_STORM:
            return worldConfig.bg.minPlayerCountEyeOfTheStorm;
        case BATTLEGROUND_ARENA_2V2:
            return worldConfig.arena.minPlayerCount2V2;
        case BATTLEGROUND_ARENA_3V3:
            return worldConfig.arena.minPlayerCount3V3;
        case BATTLEGROUND_ARENA_5V5:
            return worldConfig.arena.minPlayerCount5V5;
        case BATTLEGROUND_STRAND_OF_THE_ANCIENT:
            return worldConfig.bg.minPlayerCountStrandOfTheAncients;
        case BATTLEGROUND_ISLE_OF_CONQUEST:
            return worldConfig.bg.minPlayerCountIsleOfConquest;
        default:
            return 1;
    }
}

/// Returns the maximum number of players (Only valid for battlegrounds)
uint32 CBattlegroundManager::GetMaximumPlayers(uint32 dbcIndex)
{
    switch (dbcIndex)
    {
        case BATTLEGROUND_ALTERAC_VALLEY:
            return worldConfig.bg.maxPlayerCountAlteracValley;
        case BATTLEGROUND_WARSONG_GULCH:
            return worldConfig.bg.maxPlayerCountWarsongGulch;
        case BATTLEGROUND_ARATHI_BASIN:
            return worldConfig.bg.maxPlayerCountArathiBasin;
        case BATTLEGROUND_EYE_OF_THE_STORM:
            return worldConfig.bg.maxPlayerCountEyeOfTheStorm;
        case BATTLEGROUND_ARENA_2V2:
            return worldConfig.arena.maxPlayerCount2V2;
        case BATTLEGROUND_ARENA_3V3:
            return worldConfig.arena.maxPlayerCount3V3;
        case BATTLEGROUND_ARENA_5V5:
            return worldConfig.arena.maxPlayerCount5V5;
        case BATTLEGROUND_STRAND_OF_THE_ANCIENT:
            return worldConfig.bg.maxPlayerCountStrandOfTheAncients;
        case BATTLEGROUND_ISLE_OF_CONQUEST:
            return worldConfig.bg.maxPlayerCountIsleOfConquest;
        default:
            return 1;
    }
}


CBattleground* CBattlegroundManager::CreateInstance(uint32 Type, uint32 LevelGroup)
{
    if (bgMaps.find(Type) == bgMaps.end())
    {
        if (!isArena(Type))
        {
            LOG_ERROR("BattlegroundManager", "No map Id is registered for Battleground type %u", Type);
            return nullptr;
        }
    }

    BattlegroundFactoryMethod cfunc = nullptr;

    if (!isArena(Type))
        if (bgFactories.find(bgMaps[Type]) != bgFactories.end())
            cfunc = bgFactories[bgMaps[Type]];

    MapMgr* mgr = nullptr;
    CBattleground* bg;
    bool isWeekend = false;
    struct tm tm;
    uint32 iid;
    time_t t;
    int n;

    if (isArena(Type))
    {
        // arenas follow a different procedure.
        uint32 arenaMapCount = static_cast<uint32>(arenaMaps.size());
        if (arenaMapCount == 0)
        {
            LOG_ERROR("BattlegroundManager", "There are no Arenas registered. Cannot create Arena.");
            return nullptr;
        }

        uint32 index = Util::getRandomUInt(arenaMapCount - 1);
        uint32 mapid = arenaMaps[index];
        ArenaFactoryMethod arenaFactory = arenaFactories[index];

        mgr = sInstanceMgr.CreateBattlegroundInstance(mapid);
        if (mgr == nullptr)
        {
            LOG_ERROR("call failed for map %u, type %u, level group %u", mapid, Type, LevelGroup);
            return nullptr;      // Shouldn't happen
        }

        uint32 players_per_side = GetMaximumPlayers(Type);

        iid = ++m_maxBattlegroundId[Type];
        bg = arenaFactory(mgr, iid, LevelGroup, Type, players_per_side);
        mgr->m_battleground = bg;
        LogNotice("BattlegroundManager : Created arena battleground type %u for level group %u on map %u.", Type, LevelGroup, mapid);
        sEventMgr.AddEvent(bg, &CBattleground::EventCreate, EVENT_BATTLEGROUND_QUEUE_UPDATE, 1, 1, 0);
        m_instanceLock.Acquire();
        m_instances[Type].insert(std::make_pair(iid, bg));
        m_instanceLock.Release();
        return bg;
    }

    if (cfunc == nullptr)
    {
        LOG_ERROR("Could not find CreateBattlegroundFunc pointer for type %u level group %u", Type, LevelGroup);
        return nullptr;
    }

    t = time(nullptr);
#ifdef WIN32
    //    localtime_s(&tm, &t);
    //zack : some luv for vs2k3 compiler
    tm = *localtime(&t);
#else
    localtime_r(&t, &tm);
#endif

    switch (Type)
    {
        case BATTLEGROUND_WARSONG_GULCH:
            n = 0;
            break;
        case BATTLEGROUND_ARATHI_BASIN:
            n = 1;
            break;
        case BATTLEGROUND_EYE_OF_THE_STORM:
            n = 2;
            break;
        case BATTLEGROUND_STRAND_OF_THE_ANCIENT:
            n = 3;
            break;
        case BATTLEGROUND_ISLE_OF_CONQUEST:
            n = 4;
            break;
        default:
            n = 0;
            break;
    }
    if (((tm.tm_yday / 7) % 4) == n)
    {
        // Set weekend from Thursday night at midnight until Tuesday morning
        isWeekend = tm.tm_wday >= 5 || tm.tm_wday < 2;
    }

    // Create Map Manager
    mgr = sInstanceMgr.CreateBattlegroundInstance(bgMaps[Type]);
    if (mgr == nullptr)
    {
        LOG_ERROR("call failed for map %u, type %u, level group %u", bgMaps[Type], Type, LevelGroup);
        return nullptr;      // Shouldn't happen
    }

    // Call the create function
    iid = ++m_maxBattlegroundId[Type];
    bg = cfunc(mgr, iid, LevelGroup, Type);
    bg->SetIsWeekend(isWeekend);
    mgr->m_battleground = bg;
    sEventMgr.AddEvent(bg, &CBattleground::EventCreate, EVENT_BATTLEGROUND_QUEUE_UPDATE, 1, 1, 0);
    LogNotice("BattlegroundManager : Created battleground type %u for level group %u.", Type, LevelGroup);

    m_instanceLock.Acquire();
    m_instances[Type].insert(std::make_pair(iid, bg));
    m_instanceLock.Release();

    return bg;
}

void CBattlegroundManager::DeleteBattleground(CBattleground* bg)
{
    uint32 i = bg->GetType();
    uint32 j = bg->GetLevelGroup();

    m_instanceLock.Acquire();
    m_queueLock.Acquire();
    m_instances[i].erase(bg->GetId());

    // erase any queued players
    std::list<uint32>::iterator itr = m_queuedPlayers[i][j].begin();
    for (; itr != m_queuedPlayers[i][j].end();)
    {
        std::list<uint32>::iterator it2 = itr++;

        if (Player* plr = sObjectMgr.GetPlayer(*it2))
        {
            if (plr->m_bgQueueInstanceId == bg->GetId())
            {
                sChatHandler.SystemMessage(plr->GetSession(), plr->GetSession()->LocalizedWorldSrv(ServerString::SS_QUEUE_BG_INSTANCE_ID_NO_VALID_LONGER_EXISTS), bg->GetId());
                SendBattlefieldStatus(plr, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
                plr->m_bgIsQueued = false;
                m_queuedPlayers[i][j].erase(it2);
            }
        }
        else
        {
            m_queuedPlayers[i][j].erase(it2);
            continue;
        }
    }

    m_queueLock.Release();
    m_instanceLock.Release();

    //LOG_DETAIL("Deleting battleground from queue %u, instance %u", bg->GetType(), bg->GetId());
    delete bg;
}

void CBattlegroundManager::SendBattlefieldStatus(Player* plr, BattleGroundStatus Status, uint32 Type, uint32 InstanceID, uint32 Time, uint32 MapId, uint8 RatedMatch)
{
    plr->SendPacket(SmsgBattlefieldStatus(plr->GetNewGUID(), Status, Type, InstanceID, Time, MapId, RatedMatch, isArena(Type)).serialise().get());
}

void CBattlegroundManager::HandleArenaJoin(WorldSession* m_session, uint32 BattlegroundType, uint8 as_group, uint8 rated_match)
{
    uint32 pguid = m_session->GetPlayer()->getGuidLow();
    uint32 lgroup = GetLevelGrouping(m_session->GetPlayer()->getLevel());
    if (as_group && m_session->GetPlayer()->getGroup() == nullptr)
        return;

    Group* pGroup = m_session->GetPlayer()->getGroup();
    if (as_group)
    {
        if (pGroup->GetSubGroupCount() != 1)
        {
            m_session->SystemMessage(m_session->LocalizedWorldSrv(ServerString::SS_SORRY_RAID_GROUPS_JOINING_BG_ARE_UNSUPPORTED));
            return;
        }
        if (pGroup->GetLeader() != m_session->GetPlayer()->getPlayerInfo())
        {
            m_session->SystemMessage(m_session->LocalizedWorldSrv(ServerString::SS_MUST_BE_PARTY_LEADER_TO_ADD_GROUP_AN_ARENA));
            return;
        }

        GroupMembersSet::iterator itx;
        if (!rated_match)
        {
            // add all players normally.. bleh ;P
            pGroup->Lock();
            for (itx = pGroup->GetSubGroup(0)->GetGroupMembersBegin(); itx != pGroup->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
            {
                if ((*itx)->m_loggedInPlayer && !(*itx)->m_loggedInPlayer->m_bgIsQueued && !(*itx)->m_loggedInPlayer->m_bg)
                    HandleArenaJoin((*itx)->m_loggedInPlayer->GetSession(), BattlegroundType, 0, 0);
            }
            pGroup->Unlock();
            return;
        }

        // make sure all players are 70
        uint32 maxplayers;
        uint32 type = BattlegroundType - BATTLEGROUND_ARENA_2V2;
        switch (BattlegroundType)
        {
        case BATTLEGROUND_ARENA_3V3:
            maxplayers = 3;
            break;

        case BATTLEGROUND_ARENA_5V5:
            maxplayers = 5;
            break;

        case BATTLEGROUND_ARENA_2V2:
        default:
            maxplayers = 2;
            break;
        }

        if (pGroup->GetLeader()->m_loggedInPlayer && pGroup->GetLeader()->m_loggedInPlayer->m_arenaTeams[type] == nullptr)
        {
            m_session->SendPacket(SmsgArenaError(0, static_cast<uint8_t>(maxplayers)).serialise().get());
            return;
        }

        pGroup->Lock();
        for (itx = pGroup->GetSubGroup(0)->GetGroupMembersBegin(); itx != pGroup->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
        {
            if (maxplayers == 0)
            {
                m_session->SystemMessage(m_session->LocalizedWorldSrv(ServerString::SS_TOO_MANY_PLAYERS_PARTY_TO_JOIN_OF_ARENA));
                pGroup->Unlock();
                return;
            }

            if ((*itx)->lastLevel < PLAYER_ARENA_MIN_LEVEL)
            {
                m_session->SystemMessage(m_session->LocalizedWorldSrv(ServerString::SS_SORRY_SOME_OF_PARTY_MEMBERS_ARE_NOT_LVL_70));
                pGroup->Unlock();
                return;
            }

            if ((*itx)->m_loggedInPlayer)
            {
                if ((*itx)->m_loggedInPlayer->m_bg || (*itx)->m_loggedInPlayer->m_bgIsQueued)
                {
                    m_session->SystemMessage(m_session->LocalizedWorldSrv(ServerString::SS_ONE_OR_MORE_OF_PARTY_MEMBERS_ARE_ALREADY_QUEUED_OR_INSIDE_BG));
                    pGroup->Unlock();
                    return;
                };
                if ((*itx)->m_loggedInPlayer->m_arenaTeams[type] != pGroup->GetLeader()->m_loggedInPlayer->m_arenaTeams[type])
                {
                    m_session->SystemMessage(m_session->LocalizedWorldSrv(ServerString::SS_ONE_OR_MORE_OF_YOUR_PARTY_MEMBERS_ARE_NOT_MEMBERS_OF_YOUR_TEAM));
                    pGroup->Unlock();
                    return;
                }

                --maxplayers;
            }
        }

        for (itx = pGroup->GetSubGroup(0)->GetGroupMembersBegin(); itx != pGroup->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
        {
            if ((*itx)->m_loggedInPlayer)
            {
                SendBattlefieldStatus((*itx)->m_loggedInPlayer, BGSTATUS_INQUEUE, BattlegroundType, 0, 0, 0, 1);
                (*itx)->m_loggedInPlayer->m_bgIsQueued = true;
                (*itx)->m_loggedInPlayer->m_bgQueueInstanceId = 0;
                (*itx)->m_loggedInPlayer->m_bgQueueType = BattlegroundType;
                //\todo error/bgtype missing, always send all arenas (from legacy)
                (*itx)->m_loggedInPlayer->GetSession()->SendPacket(SmsgGroupJoinedBattleground(6).serialise().get());
                (*itx)->m_loggedInPlayer->setBGEntryPoint((*itx)->m_loggedInPlayer->GetPositionX(), (*itx)->m_loggedInPlayer->GetPositionY(), (*itx)->m_loggedInPlayer->GetPositionZ(),
                    (*itx)->m_loggedInPlayer->GetOrientation(), (*itx)->m_loggedInPlayer->GetMapId(), (*itx)->m_loggedInPlayer->GetInstanceID());
            }
        }

        pGroup->Unlock();

        m_queueLock.Acquire();
        m_queuedGroups[BattlegroundType].push_back(pGroup->GetID());
        m_queueLock.Release();
        LogNotice("BattlegroundMgr : Group %u is now in battleground queue for arena type %u", pGroup->GetID(), BattlegroundType);

        // send the battleground status packet

        return;
    }


    // Queue him!
    m_queueLock.Acquire();
    m_queuedPlayers[BattlegroundType][lgroup].push_back(pguid);
    LogNotice("BattlegroundMgr : Player %u is now in battleground queue for {Arena %u}", m_session->GetPlayer()->getGuidLow(), BattlegroundType);

    // send the battleground status packet
    SendBattlefieldStatus(m_session->GetPlayer(), BGSTATUS_INQUEUE, BattlegroundType, 0, 0, 0, 0);
    m_session->GetPlayer()->m_bgIsQueued = true;
    m_session->GetPlayer()->m_bgQueueInstanceId = 0;
    m_session->GetPlayer()->m_bgQueueType = BattlegroundType;

    m_session->GetPlayer()->setBGEntryPoint(m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ(), m_session->GetPlayer()->GetOrientation(),
        m_session->GetPlayer()->GetMapId(), m_session->GetPlayer()->GetInstanceID());

    m_queueLock.Release();
}
