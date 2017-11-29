/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Management/Item.h"
#include "Management/LFG/LFGMgr.h"
#include "Management/LFG/LFGGroupData.h"
#include "Management/LFG/LFGPlayerData.h"
#include "Management/ItemInterface.h"
#include "Server/MainServerDefines.h"

uint32 LfgDungeonTypes[MAX_DUNGEONS];

initialiseSingleton(LfgMgr);


LfgMgr::LfgMgr() : m_update(true), m_QueueTimer(0), m_lfgProposalId(1),
m_WaitTimeAvg(-1), m_WaitTimeTank(-1), m_WaitTimeHealer(-1), m_WaitTimeDps(-1),
m_NumWaitTimeAvg(0), m_NumWaitTimeTank(0), m_NumWaitTimeHealer(0), m_NumWaitTimeDps(0)
{
        m_update = true;
        if (m_update)
        {
#if VERSION_STRING != Cata
            // Initialize dungeon cache
            for (uint32 i = 0; i < sLFGDungeonStore.GetNumRows(); ++i)
            {
                DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(i);
                if (dungeon && dungeon->type != LFG_TYPE_ZONE)
                {
                    if (dungeon->type != LFG_TYPE_RANDOM)
                        m_CachedDungeonMap[dungeon->grouptype].insert(dungeon->ID);
                    m_CachedDungeonMap[0].insert(dungeon->ID);
                }
            }
#endif
        }
}

LfgMgr::~LfgMgr()
{
    for (LfgRewardMap::iterator itr = m_RewardMap.begin(); itr != m_RewardMap.end(); ++itr)
    {
        delete itr->second;
    }

    for (LfgQueueInfoMap::iterator it = m_QueueInfoMap.begin(); it != m_QueueInfoMap.end(); ++it)
    {
        delete it->second;
    }

    for (LfgProposalMap::iterator it = m_Proposals.begin(); it != m_Proposals.end(); ++it)
    {
        delete it->second;
    }

    for (LfgPlayerBootMap::iterator it = m_Boots.begin(); it != m_Boots.end(); ++it)
    {
        delete it->second;
    }

    for (LfgRoleCheckMap::iterator it = m_RoleChecks.begin(); it != m_RoleChecks.end(); ++it)
    {
        delete it->second;
    }
}

/// Load rewards for completing dungeons
void LfgMgr::LoadRewards()
{
    auto startTime = Util::TimeNow();

    for (LfgRewardMap::iterator itr = m_RewardMap.begin(); itr != m_RewardMap.end(); ++itr)
    {
        delete itr->second;
    }

    m_RewardMap.clear();

    // ORDER BY is very important for GetRandomDungeonReward!
    QueryResult* result = WorldDatabase.Query("SELECT dungeon_id, max_level, quest_id_1, money_var_1, xp_var_1, quest_id_2, money_var_2, xp_var_2 FROM lfg_dungeon_rewards ORDER BY dungeon_id, max_level ASC");
    if (result == nullptr)
    {
        LOG_ERROR("Loaded 0 lfg dungeon rewards.DB table `lfg_dungeon_rewards` is empty!\n");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 dungeonId = fields[0].GetUInt32();
        uint32 maxLevel = fields[1].GetUInt8();
        uint32 firstQuestId = fields[2].GetUInt32();
        uint32 firstMoneyVar = fields[3].GetUInt32();
        uint32 firstXPVar = fields[4].GetUInt32();
        uint32 otherQuestId = fields[5].GetUInt32();
        uint32 otherMoneyVar = fields[6].GetUInt32();
        uint32 otherXPVar = fields[7].GetUInt32();

#if VERSION_STRING != Cata
        if (!sLFGDungeonStore.LookupEntry(dungeonId))
        {
            LOG_DEBUG("Dungeon %u specified in table `lfg_dungeon_rewards` does not exist!", dungeonId);
            continue;
        }
#endif

        if (!maxLevel || maxLevel > 80)
        {
            LOG_DEBUG("Level %u specified for dungeon %u in table `lfg_dungeon_rewards` can never be reached!", maxLevel, dungeonId);
            maxLevel = 80;
        }

        if (firstQuestId && !sMySQLStore.getQuestProperties(firstQuestId))
        {
            LOG_DEBUG("First quest %u specified for dungeon %u in table `lfg_dungeon_rewards` does not exist!", firstQuestId, dungeonId);
            firstQuestId = 0;
        }

        if (otherQuestId && !sMySQLStore.getQuestProperties(otherQuestId))
        {
            LOG_DEBUG("Other quest %u specified for dungeon %u in table `lfg_dungeon_rewards` does not exist!", otherQuestId, dungeonId);
            otherQuestId = 0;
        }

#if VERSION_STRING != Cata
		//DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId);
#endif
        m_RewardMap.insert(LfgRewardMap::value_type(dungeonId, new LfgReward(maxLevel, firstQuestId, firstMoneyVar, firstXPVar, otherQuestId, otherMoneyVar, otherXPVar)));
        ++count;
    }
    while (result->NextRow());

    LogDetail("LFGMgr : Loaded %u lfg dungeon rewards in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
}

void LfgMgr::Update(uint32 diff)
{
    if (!m_update)
    {
        return;
    }

    m_update = false;
    time_t currTime = time(NULL);

    // Remove obsolete role checks
    for (LfgRoleCheckMap::iterator it = m_RoleChecks.begin(); it != m_RoleChecks.end();)
    {
        LfgRoleCheckMap::iterator itRoleCheck = it++;
        LfgRoleCheck* roleCheck = itRoleCheck->second;
        if (currTime < roleCheck->cancelTime)
        {
            continue;
        }

        roleCheck->state = LFG_ROLECHECK_MISSING_ROLE;

        for (LfgRolesMap::const_iterator itRoles = roleCheck->roles.begin(); itRoles != roleCheck->roles.end(); ++itRoles)
        {
            uint64 guid = itRoles->first;
            ClearState(guid);
            if (Player* player = objmgr.GetPlayer(GET_LOWGUID_PART(guid)))
            {
                player->GetSession()->SendLfgRoleCheckUpdate(roleCheck);

                if (itRoles->first == roleCheck->leader)
                {
                    player->GetSession()->SendLfgJoinResult(LfgJoinResultData(LFG_JOIN_FAILED, LFG_ROLECHECK_MISSING_ROLE));
                }
            }
        }
        delete roleCheck;
        m_RoleChecks.erase(itRoleCheck);
    }

    // Remove obsolete proposals
    for (LfgProposalMap::iterator it = m_Proposals.begin(); it != m_Proposals.end();)
    {
        LfgProposalMap::iterator itRemove = it++;
        if (itRemove->second->cancelTime < currTime)
        {
            RemoveProposal(itRemove, LFG_UPDATETYPE_PROPOSAL_FAILED);
        }
    }

    // Remove obsolete kicks
    for (LfgPlayerBootMap::iterator it = m_Boots.begin(); it != m_Boots.end();)
    {
        LfgPlayerBootMap::iterator itBoot = it++;
        LfgPlayerBoot* pBoot = itBoot->second;
        if (pBoot->cancelTime < currTime)
        {
            pBoot->inProgress = false;
            for (LfgAnswerMap::const_iterator itVotes = pBoot->votes.begin(); itVotes != pBoot->votes.end(); ++itVotes)
            {
                if (Player* plrg = objmgr.GetPlayer(GET_LOWGUID_PART(itVotes->first)))
                {
                    if (plrg->GetGUID() != pBoot->victim)
                    {
                        plrg->GetSession()->SendLfgBootPlayer(pBoot);
                    }
                }
            }

            delete pBoot;
            m_Boots.erase(itBoot);
        }
    }

    // Check if a proposal can be formed with the new groups being added
    for (LfgGuidListMap::iterator it = m_newToQueue.begin(); it != m_newToQueue.end(); ++it)
    {
        uint8 queueId = it->first;
        LfgGuidList& newToQueue = it->second;
        LfgGuidList& currentQueue = m_currentQueue[queueId];
        LfgGuidList firstNew;
        while (!newToQueue.empty())
        {
            uint64 frontguid = newToQueue.front();
            LOG_DEBUG("Update: QueueId %u: checking %u newToQueue(%u), currentQueue(%u)", queueId, frontguid, uint32(newToQueue.size()), uint32(currentQueue.size()));
            firstNew.push_back(frontguid);
            newToQueue.pop_front();

            LfgGuidList temporalList = currentQueue;
            if (LfgProposal* pProposal = FindNewGroups(firstNew, temporalList)) // Group found!
            {
                // Remove groups in the proposal from new and current queues (not from queue map)
                for (LfgGuidList::const_iterator itQueue = pProposal->queues.begin(); itQueue != pProposal->queues.end(); ++itQueue)
                {
                    currentQueue.remove(*itQueue);
                    newToQueue.remove(*itQueue);
                }

                m_Proposals[++m_lfgProposalId] = pProposal;

                uint64 guid = 0;
                for (LfgProposalPlayerMap::const_iterator itPlayers = pProposal->players.begin(); itPlayers != pProposal->players.end(); ++itPlayers)
                {
                    guid = itPlayers->first;
                    SetState(guid, LFG_STATE_PROPOSAL);
                    if (Player* player = objmgr.GetPlayer(GET_LOWGUID_PART(itPlayers->first)))
                    {
                        Group* grp = player->GetGroup();
                        if (grp)
                        {
                            uint64 gguid = grp->GetGUID();
                            SetState(gguid, LFG_STATE_PROPOSAL);
                            player->GetSession()->SendLfgUpdateParty(LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        }
                        else
                        {
                            player->GetSession()->SendLfgUpdatePlayer(LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        }

                        player->GetSession()->SendLfgUpdateProposal(m_lfgProposalId, pProposal);
                    }
                }

                if (pProposal->state == LFG_PROPOSAL_SUCCESS)
                {
                    UpdateProposal(m_lfgProposalId, guid, true);
                }
            }
            else
            {
                if (std::find(currentQueue.begin(), currentQueue.end(), frontguid) == currentQueue.end()) //already in queue?
                {
                    currentQueue.push_back(frontguid);         // Lfg group not found, add this group to the queue.
                }
            }

            firstNew.clear();
        }
    }

    // Update all players status queue info
    if (m_QueueTimer > LFG_QUEUEUPDATE_INTERVAL)
    {
        m_QueueTimer = 0;
        currTime = time(NULL);
        for (LfgQueueInfoMap::const_iterator itQueue = m_QueueInfoMap.begin(); itQueue != m_QueueInfoMap.end(); ++itQueue)
        {
            LfgQueueInfo* queue = itQueue->second;
            if (!queue)
            {
                LOG_DEBUG("Update: %u queued with null queue info!", itQueue->first);
                continue;
            }
            uint32 dungeonId = (*queue->dungeons.begin());
            uint32 queuedTime = uint32(currTime - queue->joinTime);
            uint8 role = ROLE_NONE;
            for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
            {
                role |= itPlayer->second;
            }

            role &= ~ROLE_LEADER;

            int32 waitTime = -1;
            switch (role)
            {
                case ROLE_NONE:                             // Should not happen - just in case
                    waitTime = -1;
                    break;
                case ROLE_TANK:
                    waitTime = m_WaitTimeTank;
                    break;
                case ROLE_HEALER:
                    waitTime = m_WaitTimeHealer;
                    break;
                case ROLE_DAMAGE:
                    waitTime = m_WaitTimeDps;
                    break;
                default:
                    waitTime = m_WaitTimeAvg;
                    break;
            }

            for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
            {
                if (Player* player = objmgr.GetPlayer(GET_LOWGUID_PART(itPlayer->first)))
                {
                    player->GetSession()->SendLfgQueueStatus(dungeonId, waitTime, m_WaitTimeAvg, m_WaitTimeTank, m_WaitTimeHealer, m_WaitTimeDps, queuedTime, queue->tanks, queue->healers, queue->dps);
                }
            }
        }
    }
    else
    {
        m_QueueTimer += diff;
    }

    m_update = true;
}

void LfgMgr::AddToQueue(uint64 guid, uint8 queueId)
{
    //queueId = 0; //allows horde and alli in same queque

    LfgGuidList& list = m_newToQueue[queueId];
    if (std::find(list.begin(), list.end(), guid) != list.end())
    {
        LOG_DEBUG("%u already in new queue. ignoring", guid);
    }
    else
    {
        list.push_back(guid);
        LOG_DEBUG("%u added to m_newToQueue (size: %u)", guid, uint32(list.size()));
    }
}

bool LfgMgr::RemoveFromQueue(uint64 guid)
{
    for (LfgGuidListMap::iterator it = m_currentQueue.begin(); it != m_currentQueue.end(); ++it)
    {
        it->second.remove(guid);
    }

    for (LfgGuidListMap::iterator it = m_newToQueue.begin(); it != m_newToQueue.end(); ++it)
    {
        it->second.remove(guid);
    }

    RemoveFromCompatibles(guid);

    LfgQueueInfoMap::iterator it = m_QueueInfoMap.find(guid);
    if (it != m_QueueInfoMap.end())
    {
        delete it->second;
        m_QueueInfoMap.erase(it);
        LOG_DEBUG("%u removed", guid);
        return true;
    }
    else
    {
        LOG_DEBUG("%u not in queue", guid);
        return false;
    }

}

void LfgMgr::InitializeLockedDungeons(Player* player)
{
    uint64 guid = player->GetGUID();
    uint8 level = static_cast<uint8>(player->getLevel());
    uint8 expansion = static_cast<uint8>(player->GetSession()->GetFlags());
    LfgDungeonSet dungeons = GetDungeonsByRandom(0);
    LfgLockMap lock;

#if VERSION_STRING != Cata
    for (LfgDungeonSet::const_iterator it = dungeons.begin(); it != dungeons.end(); ++it)
    {
        DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(*it);
        if (!dungeon) // should never happen - We provide a list from dbcLFGDungeon
        {
            continue;
        }

        LfgLockStatusType locktype = LFG_LOCKSTATUS_OK;
        if (dungeon->expansion > expansion)
        {
            locktype = LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
        }
        else if (dungeon->minlevel > level)
        {
            locktype = LFG_LOCKSTATUS_TOO_LOW_LEVEL;
        }
        else if (dungeon->maxlevel < level)
        {
            locktype = LFG_LOCKSTATUS_TOO_HIGH_LEVEL;
        }

        // \todo check for needed items/quests ...

        if (locktype != LFG_LOCKSTATUS_OK)
        {
            lock[dungeon->Entry()] = locktype;
        }
    }
    SetLockedDungeons(guid, lock);
#else
    if (expansion == 0) { return; }
    if (guid == 0) { return; }
    if (level == 0) { return; }
#endif
}

void LfgMgr::Join(Player* player, uint8 roles, const LfgDungeonSet& selectedDungeons, const std::string& comment)
{
    if (!player || !player->GetSession() || selectedDungeons.empty())
    {
        return;
    }

    Group* grp = player->GetGroup();
    uint64 guid = player->GetGUID();
    uint64 gguid = grp ? grp->GetGUID() : guid;
    LfgJoinResultData joinData;
    PlayerSet players;
    Player* plr = nullptr;
    uint32 rDungeonId = 0;
    bool isContinue = grp && grp->isLFGGroup() && GetState(gguid) != LFG_STATE_FINISHED_DUNGEON;
    LfgDungeonSet dungeons = selectedDungeons;

    // Do not allow to change dungeon in the middle of a current dungeon
    if (isContinue)
    {
        dungeons.clear();
        dungeons.insert(GetDungeon(gguid));
    }

    // Already in queue?
    LfgQueueInfoMap::iterator itQueue = m_QueueInfoMap.find(gguid);
    if (itQueue != m_QueueInfoMap.end())
    {
        LfgDungeonSet playerDungeons = GetSelectedDungeons(guid);
        if (playerDungeons == dungeons)                     // Joining the same dungeons -- Send OK
        {
            LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons, comment);
            player->GetSession()->SendLfgJoinResult(joinData); // Default value of joinData.result = LFG_JOIN_OK
            if (grp)
            {
                GroupMembersSet::iterator itx;
                for (itx = grp->GetSubGroup(0)->GetGroupMembersBegin(); itx != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
                {
                    plr = (*itx)->m_loggedInPlayer;
                }

                if (plr)
                {
                    plr->GetSession()->SendLfgUpdateParty(updateData);
                }

            }
            return;
        }
        else // Remove from queue and rejoin
        {
            RemoveFromQueue(gguid);
        }
    }

    // Check player or group member restrictions
    if (player->InBattleground() || player->InBattlegroundQueue())
    {
        joinData.result = LFG_JOIN_USING_BG_SYSTEM;
    }
    else if (player->HasAura(LFG_SPELL_DUNGEON_DESERTER))
    {
        joinData.result = LFG_JOIN_DESERTER;
    }
    else if (player->HasAura(LFG_SPELL_DUNGEON_COOLDOWN))
    {
        joinData.result = LFG_JOIN_RANDOM_COOLDOWN;
    }
    else if (dungeons.empty())
    {
        joinData.result = LFG_JOIN_NOT_MEET_REQS;
    }
    else if (grp)
    {
        if (grp->MemberCount() > 5)
        {
            joinData.result = LFG_JOIN_TOO_MUCH_MEMBERS;
        }
        else
        {
            uint8 memberCount = 0;
            GroupMembersSet::iterator itx;
            for (itx = grp->GetSubGroup(0)->GetGroupMembersBegin(); itx != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
            {
                if (Player* plrg = (*itx)->m_loggedInPlayer)
                {
                    if (joinData.result == LFG_JOIN_OK)
                    {
                        if (plrg->HasAura(LFG_SPELL_DUNGEON_DESERTER))
                        {
                            joinData.result = LFG_JOIN_PARTY_DESERTER;
                        }
                        else if (plrg->HasAura(LFG_SPELL_DUNGEON_COOLDOWN))
                        {
                            joinData.result = LFG_JOIN_PARTY_RANDOM_COOLDOWN;
                        }
                        else if (plrg->InBattleground() || plrg->InBattlegroundQueue())
                        {
                            joinData.result = LFG_JOIN_USING_BG_SYSTEM;
                        }

                        ++memberCount;
                        players.insert(plrg);
                    }
                }
            }
            if (memberCount != grp->MemberCount())
            {
                joinData.result = LFG_JOIN_DISCONNECTED;
            }
        }
    }
    else
    {
        players.insert(player);
    }

    // Check if all dungeons are valid
    bool isRaid = false;
    if (joinData.result == LFG_JOIN_OK)
    {
        LOG_DEBUG("JOIN QU LFG_JOIN_OK");
        bool isDungeon = false;
        for (LfgDungeonSet::const_iterator it = dungeons.begin(); it != dungeons.end() && joinData.result == LFG_JOIN_OK; ++it)
        {
            switch (GetDungeonType(*it))
            {
                case LFG_TYPE_RANDOM:
                {
                    if (dungeons.size() > 1)                            // Only allow 1 random dungeon
                    {
                        joinData.result = LFG_JOIN_DUNGEON_INVALID;
                    }
                    else
                    {
                        rDungeonId = (*dungeons.begin());
                    }
                }
                // No break on purpose (Random can only be dungeon or heroic dungeon)
                case LFG_TYPE_HEROIC:
                case LFG_TYPE_DUNGEON:
                {
                    if (isRaid)
                    {
                        joinData.result = LFG_JOIN_MIXED_RAID_DUNGEON;
                    }

                    isDungeon = true;
                } break;
                case LFG_TYPE_RAID:
                {
                    if (isDungeon)
                    {
                        joinData.result = LFG_JOIN_MIXED_RAID_DUNGEON;
                    }

                    isRaid = true;
                } break;
                default:
                {
                    joinData.result = LFG_JOIN_DUNGEON_INVALID;
                } break;
            }
        }

        // it could be changed
        if (joinData.result == LFG_JOIN_OK)
        {
            // Expand random dungeons and check restrictions
            if (rDungeonId)
            {
                dungeons = GetDungeonsByRandom(rDungeonId);
            }

            // if we have lockmap then there are no compatible dungeons
            GetCompatibleDungeons(dungeons, players, joinData.lockmap);
            if (dungeons.empty())
            {
                joinData.result = grp ? LFG_JOIN_PARTY_NOT_MEET_REQS : LFG_JOIN_NOT_MEET_REQS;
            }
        }
    }

    // Can't join. Send result
    if (joinData.result != LFG_JOIN_OK)
    {
        LOG_DEBUG("%u joining with %u members. result: %u   cant join return", guid, grp ? grp->MemberCount() : 1, joinData.result);
        if (!dungeons.empty())                             // Only should show lockmap when have no dungeons available
        {
            joinData.lockmap.clear();
        }

        player->GetSession()->SendLfgJoinResult(joinData);
        return;
    }

    // \todo Raid browser not supported yet
    if (isRaid)
    {
        LOG_DEBUG("%u trying to join raid browser and it's disabled.", guid);
        return;
    }

    SetComment(guid, comment);

    if (grp)        // Begin rolecheck
    {
        // Create new rolecheck
        LfgRoleCheck* roleCheck = new LfgRoleCheck();
        roleCheck->cancelTime = time_t(time(NULL)) + LFG_TIME_ROLECHECK;
        roleCheck->state = LFG_ROLECHECK_INITIALITING;
        roleCheck->leader = guid;
        roleCheck->dungeons = dungeons;
        roleCheck->rDungeonId = rDungeonId;
        m_RoleChecks[gguid] = roleCheck;

        if (rDungeonId)
        {
            dungeons.clear();
            dungeons.insert(rDungeonId);
        }

        SetState(gguid, LFG_STATE_ROLECHECK);
        // Send update to player
        LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_JOIN_PROPOSAL, dungeons, comment);
        GroupMembersSet::iterator itx;
        for (itx = grp->GetSubGroup(0)->GetGroupMembersBegin(); itx != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
        {
            if (Player* plrg = (*itx)->m_loggedInPlayer)
            {
                uint64 pguid = plrg->GetGUID();
                plrg->GetSession()->SendLfgUpdateParty(updateData);
                SetState(pguid, LFG_STATE_ROLECHECK);
                if (!isContinue)
                {
                    SetSelectedDungeons(pguid, dungeons);
                }

                roleCheck->roles[pguid] = 0;
            }
        }
        // Update leader role
        UpdateRoleCheck(gguid, guid, roles);
    }
    else        // Add player to queue
    {
        // Queue player
        LfgQueueInfo* pqInfo = new LfgQueueInfo();
        pqInfo->joinTime = time_t(time(NULL));
        pqInfo->roles[player->GetGUID()] = roles;
        pqInfo->dungeons = dungeons;
        if (roles & ROLE_TANK)
        {
            --pqInfo->tanks;
        }
        else if (roles & ROLE_HEALER)
        {
            --pqInfo->healers;
        }
        else
        {
            --pqInfo->dps;
        }

        m_QueueInfoMap[guid] = pqInfo;

        // Send update to player
        player->GetSession()->SendLfgJoinResult(joinData);
        player->GetSession()->SendLfgUpdatePlayer(LfgUpdateData(LFG_UPDATETYPE_JOIN_PROPOSAL, dungeons, comment));
        SetState(gguid, LFG_STATE_QUEUED);
        SetRoles(guid, roles);
        if (!isContinue)
        {
            if (rDungeonId)
            {
                dungeons.clear();
                dungeons.insert(rDungeonId);
            }

            SetSelectedDungeons(guid, dungeons);
        }
        AddToQueue(guid, uint8(player->GetTeam()));
    }
    LOG_DEBUG("%u joined with %u members. dungeons: %u", guid, grp ? grp->MemberCount() : 1, uint8(dungeons.size()));
}

void LfgMgr::Leave(Player* player, Group* grp /* = NULL*/)
{
    if (!player && !grp)
    {
        return;
    }

    uint64 guid = grp ? grp->GetGUID() : player->GetGUID();
    LfgState state = GetState(guid);

    LOG_DEBUG("%u", guid);
    switch (state)
    {
        case LFG_STATE_QUEUED:
        {
            RemoveFromQueue(guid);
            LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE);
            if (grp)
            {
                RestoreState(guid);
                GroupMembersSet::iterator itx;
                for (itx = grp->GetSubGroup(0)->GetGroupMembersBegin(); itx != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
                {
                    if (Player* plrg = (*itx)->m_loggedInPlayer)
                    {
                        plrg->GetSession()->SendLfgUpdateParty(updateData);
                        uint64 pguid = plrg->GetGUID();
                        ClearState(pguid);
                    }
                }
            }
            else
            {
                player->GetSession()->SendLfgUpdatePlayer(updateData);
                ClearState(guid);
            }
        }
        break;
        case LFG_STATE_ROLECHECK:
        {
            if (grp)
            {
                UpdateRoleCheck(guid);                     // No player to update role = LFG_ROLECHECK_ABORTED
            }
        } break;
        case LFG_STATE_PROPOSAL:
        {
            // Remove from Proposals
            LfgProposalMap::iterator it = m_Proposals.begin();
            while (it != m_Proposals.end())
            {
                LfgProposalPlayerMap::iterator itPlayer = it->second->players.find(player ? player->GetGUID() : grp->GetLeader()->guid);
                if (itPlayer != it->second->players.end())
                {
                    // Mark the player/leader of group who left as didn't accept the proposal
                    itPlayer->second->accept = LFG_ANSWER_DENY;
                    break;
                }
                ++it;
            }

            // Remove from queue - if proposal is found, RemoveProposal will call RemoveFromQueue
            if (it != m_Proposals.end())
            {
                RemoveProposal(it, LFG_UPDATETYPE_PROPOSAL_DECLINED);
            }
        } break;
        default:
            break;
    }
}

void LfgMgr::OfferContinue(Group* grp)
{
    if (grp)
    {
        uint64 gguid = grp->GetGUID();
        if (Player* leader = objmgr.GetPlayer(GET_LOWGUID_PART(grp->GetLeader()->guid)))
        {
            leader->GetSession()->SendLfgOfferContinue(GetDungeon(gguid, false));
        }

        LOG_DEBUG("player %u ", objmgr.GetPlayer(GET_LOWGUID_PART(grp->GetLeader()->guid)));
    }
}

LfgProposal* LfgMgr::FindNewGroups(LfgGuidList& check, LfgGuidList& all)
{
    LOG_DEBUG("(%s) - all(%s)", ConcatenateGuids(check).c_str(), ConcatenateGuids(all).c_str());

    LfgProposal* pProposal = nullptr;
    if (check.empty() || check.size() > 5 || !CheckCompatibility(check, pProposal))
    {
        return nullptr;
    }

    // Try to match with queued groups
    while (!pProposal && !all.empty())
    {
        check.push_back(all.front());
        all.pop_front();
        pProposal = FindNewGroups(check, all);
        check.pop_back();
    }
    return pProposal;
}

bool LfgMgr::CheckCompatibility(LfgGuidList check, LfgProposal*& pProposal)
{
    if (pProposal)                                         // Do not check anything if we already have a proposal
    {
        return false;
    }

    std::string strGuids = ConcatenateGuids(check);

    if (check.size() > 5 || check.empty())
    {
        LOG_DEBUG("(%s): Size wrong - Not compatibles", strGuids.c_str());
        return false;
    }

    if (check.size() == 1 && IS_PLAYER_GUID(check.front())) // Player joining dungeon... compatible
        return true;

    // Previously cached?
    LfgAnswer answer = GetCompatibles(strGuids);
    if (answer != LFG_ANSWER_PENDING)
    {
        LOG_DEBUG("(%s) compatibles (cached): %d", strGuids.c_str(), answer);
        return bool(answer ? true : false);
    }

    // Check all but new compatiblitity
    if (check.size() > 2)
    {
        uint64 frontGuid = check.front();
        check.pop_front();

        // Check all-but-new compatibilities (New, A, B, C, D) --> check(A, B, C, D)
        if (!CheckCompatibility(check, pProposal))          // Group not compatible
        {
            LOG_DEBUG("(%s) not compatibles (%s not compatibles)", strGuids.c_str(), ConcatenateGuids(check).c_str());
            SetCompatibles(strGuids, false);
            return false;
        }
        check.push_front(frontGuid);
        // all-but-new compatibles, now check with new
    }

    uint8 numPlayers = 0;
    uint8 numLfgGroups = 0;
    uint32 groupLowGuid = 0;
    LfgQueueInfoMap pqInfoMap;
    for (LfgGuidList::const_iterator it = check.begin(); it != check.end() && numLfgGroups < 2 && numPlayers <= 5; ++it)
    {
        uint64 guid = (*it);
        LfgQueueInfoMap::iterator itQueue = m_QueueInfoMap.find(guid);
        if (itQueue == m_QueueInfoMap.end() || GetState(guid) != LFG_STATE_QUEUED)
        {
            LOG_DEBUG("%u is not queued but listed as queued!", (*it));
            RemoveFromQueue(guid);
            return false;
        }
        pqInfoMap[guid] = itQueue->second;
        numPlayers += static_cast<uint8>(itQueue->second->roles.size());

        if (IS_GROUP(guid))
        {
            uint32 lowGuid = GET_LOWGUID_PART(guid);
            if (Group* grp = objmgr.GetGroupById(lowGuid))  //MAy Check these
            {
                if (grp->isLFGGroup())
                {
                    if (!numLfgGroups)
                    {
                        groupLowGuid = lowGuid;
                    }

                    ++numLfgGroups;
                }
            }
        }
    }

    if (check.size() == 1 && numPlayers != 5)   // Single group with less than MAXGROUPSIZE - Compatibles
    {
        return true;
    }

    // Do not match - groups already in a lfgDungeon or too much players
    if (numLfgGroups > 1 || numPlayers > 5)
    {
        SetCompatibles(strGuids, false);
        if (numLfgGroups > 1)
        {
            LOG_DEBUG("(%s) More than one Lfggroup (%u)", strGuids.c_str(), numLfgGroups);
        }
        else
        {
            LOG_DEBUG("(%s) Too much players (%u)", strGuids.c_str(), numPlayers);
        }

        return false;
    }

    // ----- Player checks -----
    LfgRolesMap rolesMap;
    uint64 leader = 0;
    for (LfgQueueInfoMap::const_iterator it = pqInfoMap.begin(); it != pqInfoMap.end(); ++it)
    {
        for (LfgRolesMap::const_iterator itRoles = it->second->roles.begin(); itRoles != it->second->roles.end(); ++itRoles)
        {
            // Assign new leader
            if (itRoles->second & ROLE_LEADER && (!leader || RandomUInt(1)))
            {
                leader = itRoles->first;
            }

            rolesMap[itRoles->first] = itRoles->second;
        }
    }

    if (rolesMap.size() != numPlayers)                     // Player in multiples queues!
        return false;

    PlayerSet players;
    for (LfgRolesMap::const_iterator it = rolesMap.begin(); it != rolesMap.end(); ++it)
    {
        Player* player = objmgr.GetPlayer(GET_LOWGUID_PART(it->first));
        if (!player)
        {
            LOG_DEBUG("(%s) Warning! %u offline! Marking as not compatibles!", strGuids.c_str(), it->first);
        }
        else
        {
            for (PlayerSet::const_iterator itPlayer = players.begin(); itPlayer != players.end() && player; ++itPlayer)
            {
                // Do not form a group with ignoring candidates
            }
            if (player)
            {
                players.insert(player);
            }
        }
    }

    // if we dont have the same ammount of players then we have self ignoring candidates or different faction groups
    // otherwise check if roles are compatible
    if (players.size() != numPlayers || !CheckGroupRoles(rolesMap))
    {
        if (players.size() == numPlayers)
        {
            LOG_DEBUG("(%s) Roles not compatible", strGuids.c_str());
        }

        SetCompatibles(strGuids, false);
        return false;
    }

    // ----- Selected Dungeon checks -----
    // Check if there are any compatible dungeon from the selected dungeons
    LfgDungeonSet compatibleDungeons;

    LfgQueueInfoMap::const_iterator itFirst = pqInfoMap.begin();
    for (LfgDungeonSet::const_iterator itDungeon = itFirst->second->dungeons.begin(); itDungeon != itFirst->second->dungeons.end(); ++itDungeon)
    {
        LfgQueueInfoMap::const_iterator itOther = itFirst;
        ++itOther;
        while (itOther != pqInfoMap.end() && itOther->second->dungeons.find(*itDungeon) != itOther->second->dungeons.end())
        {
            ++itOther;
        }

        if (itOther == pqInfoMap.end())
        {
            compatibleDungeons.insert(*itDungeon);
        }
    }
    LfgLockPartyMap lockMap;
    GetCompatibleDungeons(compatibleDungeons, players, lockMap);

    if (compatibleDungeons.empty())
    {
        SetCompatibles(strGuids, false);
        return false;
    }
    SetCompatibles(strGuids, true);

    // ----- Group is compatible, if we have MAXGROUPSIZE members then match is found
    if (numPlayers != 5)
    {
        LOG_DEBUG("(%s) Compatibles but not match. Players(%u)", strGuids.c_str(), numPlayers);
        uint8 Tanks_Needed = LFG_TANKS_NEEDED;
        uint8 Healers_Needed = LFG_HEALERS_NEEDED;
        uint8 Dps_Needed = LFG_DPS_NEEDED;
        for (LfgQueueInfoMap::const_iterator itQueue = pqInfoMap.begin(); itQueue != pqInfoMap.end(); ++itQueue)
        {
            LfgQueueInfo* queue = itQueue->second;
            for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
            {
                uint8 roles = itPlayer->second;
                if ((roles & ROLE_TANK) && Tanks_Needed > 0)
                    --Tanks_Needed;
                else if ((roles & ROLE_HEALER) && Healers_Needed > 0)
                    --Healers_Needed;
                else if ((roles & ROLE_DAMAGE) && Dps_Needed > 0)
                    --Dps_Needed;
            }
        }
        for (PlayerSet::const_iterator itPlayers = players.begin(); itPlayers != players.end(); ++itPlayers)
        {
            for (LfgQueueInfoMap::const_iterator itQueue = pqInfoMap.begin(); itQueue != pqInfoMap.end(); ++itQueue)
            {
                LfgQueueInfo* queue = itQueue->second;
                if (!queue)
                    continue;

                for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
                {
                    if (*itPlayers == objmgr.GetPlayer(GET_LOWGUID_PART(itPlayer->first)))
                    {
                        queue->tanks = Tanks_Needed;
                        queue->healers = Healers_Needed;
                        queue->dps = Dps_Needed;
                    }
                }
            }
        }
        return true;
    }
    LOG_DEBUG("(%s) MATCH! Group formed", strGuids.c_str());

    // GROUP FORMED!

    // Select a random dungeon from the compatible list
    // Create a new proposal
    pProposal = new LfgProposal(SelectRandomContainerElement(compatibleDungeons));
    pProposal->cancelTime = time_t(time(NULL)) + LFG_TIME_PROPOSAL;
    pProposal->state = LFG_PROPOSAL_INITIATING;
    pProposal->queues = check;
    pProposal->groupLowGuid = groupLowGuid;

    // Assign new roles to players and assign new leader
    PlayerSet::const_iterator itPlayers = players.begin();
    /*
    if (!leader)
    {
    uint8 pos = 1;
    for (uint8 i = 0; i < pos; ++i)
    ++itPlayers;
    leader = (*itPlayers)->GetGUID();
    }
    */
    pProposal->leader = leader;

    uint8 numAccept = 0;
    for (itPlayers = players.begin(); itPlayers != players.end(); ++itPlayers)
    {
        uint64 guid = (*itPlayers)->GetGUID();
        LfgProposalPlayer* ppPlayer = new LfgProposalPlayer();
        if (Group* grp = (*itPlayers)->GetGroup())
        {
            ppPlayer->groupLowGuid = grp->GetID();
            if (grp->isLFGGroup()) // Player from existing group, autoaccept
            {
                ppPlayer->accept = LFG_ANSWER_AGREE;
                ++numAccept;
            }
        }
        ppPlayer->role = rolesMap[guid];
        pProposal->players[guid] = ppPlayer;
    }
    if (numAccept == 5)
        pProposal->state = LFG_PROPOSAL_SUCCESS;

    return true;
}

void LfgMgr::UpdateRoleCheck(uint64 gguid, uint64 guid /* = 0 */, uint8 roles /* = ROLE_NONE */)
{
    if (!gguid)
        return;

    LfgRolesMap check_roles;
    LfgRoleCheckMap::iterator itRoleCheck = m_RoleChecks.find(gguid);
    if (itRoleCheck == m_RoleChecks.end())
        return;

    LfgRoleCheck* roleCheck = itRoleCheck->second;
    bool sendRoleChosen = roleCheck->state != LFG_ROLECHECK_DEFAULT && guid;

    if (!guid)
        roleCheck->state = LFG_ROLECHECK_ABORTED;
    else if (roles < ROLE_TANK)                            // Player selected no role.
        roleCheck->state = LFG_ROLECHECK_NO_ROLE;
    else
    {
        roleCheck->roles[guid] = roles;

        // Check if all players have selected a role
        LfgRolesMap::const_iterator itRoles = roleCheck->roles.begin();
        while (itRoles != roleCheck->roles.end() && itRoles->second != ROLE_NONE)
        {
            ++itRoles;
        }

        if (itRoles == roleCheck->roles.end())
        {
            // use temporal var to check roles, CheckGroupRoles modifies the roles
            check_roles = roleCheck->roles;
            roleCheck->state = CheckGroupRoles(check_roles) ? LFG_ROLECHECK_FINISHED : LFG_ROLECHECK_WRONG_ROLES;
        }
    }

    uint8 team = 0;
    LfgDungeonSet dungeons;
    if (roleCheck->rDungeonId)
        dungeons.insert(roleCheck->rDungeonId);
    else
        dungeons = roleCheck->dungeons;

    LfgJoinResultData joinData = LfgJoinResultData(LFG_JOIN_FAILED, roleCheck->state);
    for (LfgRolesMap::const_iterator it = roleCheck->roles.begin(); it != roleCheck->roles.end(); ++it)
    {
        uint64 pguid = it->first;
        Player* plrg = objmgr.GetPlayer(GET_LOWGUID_PART(pguid));
        if (!plrg)
        {
            if (roleCheck->state == LFG_ROLECHECK_FINISHED)
                SetState(pguid, LFG_STATE_QUEUED);
            else if (roleCheck->state != LFG_ROLECHECK_INITIALITING)
                ClearState(pguid);
            continue;
        }

        team = uint8(plrg->GetTeam());
        if (!sendRoleChosen)
            plrg->GetSession()->SendLfgRoleChosen(guid, roles);
        plrg->GetSession()->SendLfgRoleCheckUpdate(roleCheck);
        switch (roleCheck->state)
        {
            case LFG_ROLECHECK_INITIALITING:
                continue;
            case LFG_ROLECHECK_FINISHED:
                SetState(pguid, LFG_STATE_QUEUED);
                plrg->GetSession()->SendLfgUpdateParty(LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons, GetComment(pguid)));
                break;
            default:
                if (roleCheck->leader == pguid)
                    plrg->GetSession()->SendLfgJoinResult(joinData);
                plrg->GetSession()->SendLfgUpdateParty(LfgUpdateData(LFG_UPDATETYPE_ROLECHECK_FAILED));
                ClearState(pguid);
                break;
        }
    }

    if (roleCheck->state == LFG_ROLECHECK_FINISHED)
    {
        SetState(gguid, LFG_STATE_QUEUED);
        LfgQueueInfo* pqInfo = new LfgQueueInfo();
        pqInfo->joinTime = time_t(time(NULL));
        pqInfo->roles = roleCheck->roles;
        pqInfo->dungeons = roleCheck->dungeons;

        // Set queue roles needed - As we are using check_roles will not have more that 1 tank, 1 healer, 3 dps
        for (LfgRolesMap::const_iterator it = check_roles.begin(); it != check_roles.end(); ++it)
        {
            uint8 roles2 = it->second;
            if (roles2 & ROLE_TANK)
                --pqInfo->tanks;
            else if (roles2 & ROLE_HEALER)
                --pqInfo->healers;
            else
                --pqInfo->dps;
        }

        m_QueueInfoMap[gguid] = pqInfo;
        if (GetState(gguid) != LFG_STATE_NONE)
        {
            LfgGuidList& currentQueue = m_currentQueue[team];
            currentQueue.push_front(gguid);
        }
        AddToQueue(gguid, team);
    }

    if (roleCheck->state != LFG_ROLECHECK_INITIALITING)
    {
        if (roleCheck->state != LFG_ROLECHECK_FINISHED)
            RestoreState(gguid);
        delete roleCheck;
        m_RoleChecks.erase(itRoleCheck);
    }
}

void LfgMgr::RemoveFromCompatibles(uint64 guid)
{
    std::stringstream out;
    out << guid;
    std::string strGuid = out.str();

    LOG_DEBUG("Removing %u", guid);
    for (LfgCompatibleMap::iterator itNext = m_CompatibleMap.begin(); itNext != m_CompatibleMap.end();)
    {
        LfgCompatibleMap::iterator it = itNext++;
        if (it->first.find(strGuid) != std::string::npos)  // Found, remove it
            m_CompatibleMap.erase(it);
    }
}

void LfgMgr::SetCompatibles(std::string key, bool compatibles)
{
    m_CompatibleMap[key] = LfgAnswer(compatibles);
}

LfgAnswer LfgMgr::GetCompatibles(std::string key)
{
    LfgAnswer answer = LFG_ANSWER_PENDING;
    LfgCompatibleMap::iterator it = m_CompatibleMap.find(key);
    if (it != m_CompatibleMap.end())
        answer = it->second;

    return answer;
}

void LfgMgr::GetCompatibleDungeons(LfgDungeonSet& dungeons, const PlayerSet& players, LfgLockPartyMap& lockMap)
{
    lockMap.clear();
    for (PlayerSet::const_iterator it = players.begin(); it != players.end() && !dungeons.empty(); ++it)
    {
        uint64 guid = (*it)->GetGUID();
        LfgLockMap cachedLockMap = GetLockedDungeons(guid);
        for (LfgLockMap::const_iterator it2 = cachedLockMap.begin(); it2 != cachedLockMap.end() && !dungeons.empty(); ++it2)
        {
            uint32 dungeonId = (it2->first & 0x00FFFFFF); // Compare dungeon ids
            LfgDungeonSet::iterator itDungeon = dungeons.find(dungeonId);
            if (itDungeon != dungeons.end())
            {
                dungeons.erase(itDungeon);
                lockMap[guid][dungeonId] = it2->second;
            }
        }
    }
    if (!dungeons.empty())
        lockMap.clear();
}

bool LfgMgr::CheckGroupRoles(LfgRolesMap& groles, bool removeLeaderFlag /*= true*/)
{
    if (groles.empty())
        return false;

    uint8 damage = 0;
    uint8 tank = 0;
    uint8 healer = 0;

    if (removeLeaderFlag)
        for (LfgRolesMap::iterator it = groles.begin(); it != groles.end(); ++it)
            it->second &= ~ROLE_LEADER;

    for (LfgRolesMap::iterator it = groles.begin(); it != groles.end(); ++it)
    {
        if (it->second == ROLE_NONE)
            return false;

        if (it->second & ROLE_TANK)
        {
            if (it->second != ROLE_TANK)
            {
                it->second -= ROLE_TANK;
                if (CheckGroupRoles(groles, false))
                    return true;
                it->second += ROLE_TANK;
            }
            else if (tank == LFG_TANKS_NEEDED)
                return false;
            else
                tank++;
        }

        if (it->second & ROLE_HEALER)
        {
            if (it->second != ROLE_HEALER)
            {
                it->second -= ROLE_HEALER;
                if (CheckGroupRoles(groles, false))
                    return true;
                it->second += ROLE_HEALER;
            }
            else if (healer == LFG_HEALERS_NEEDED)
                return false;
            else
                healer++;
        }

        if (it->second & ROLE_DAMAGE)
        {
            if (it->second != ROLE_DAMAGE)
            {
                it->second -= ROLE_DAMAGE;
                if (CheckGroupRoles(groles, false))
                    return true;
                it->second += ROLE_DAMAGE;
            }
            else if (damage == LFG_DPS_NEEDED)
                return false;
            else
                damage++;
        }
    }
    return (tank + healer + damage) == uint8(groles.size());
}

void LfgMgr::UpdateProposal(uint32 proposalId, uint64 guid, bool accept)
{
    // Check if the proposal exists
    LfgProposalMap::iterator itProposal = m_Proposals.find(proposalId);
    if (itProposal == m_Proposals.end())
        return;
    LfgProposal* pProposal = itProposal->second;

    // Check if proposal have the current player
    LfgProposalPlayerMap::iterator itProposalPlayer = pProposal->players.find(guid);
    if (itProposalPlayer == pProposal->players.end())
        return;
    LfgProposalPlayer* ppPlayer = itProposalPlayer->second;

    ppPlayer->accept = LfgAnswer(accept);
    LOG_DEBUG("Player %u of proposal %u selected: %u", guid, proposalId, accept);
    if (!accept)
    {
        RemoveProposal(itProposal, LFG_UPDATETYPE_PROPOSAL_DECLINED);
        return;
    }

    LfgPlayerList players;
    LfgPlayerList playersToTeleport;

    // check if all have answered and reorder players (leader first)
    bool allAnswered = true;
    for (LfgProposalPlayerMap::const_iterator itPlayers = pProposal->players.begin(); itPlayers != pProposal->players.end(); ++itPlayers)
    {
        if (Player* player = objmgr.GetPlayer(GET_LOWGUID_PART(itPlayers->first)))
        {
            if (itPlayers->first == pProposal->leader)
                players.push_front(player);
            else
                players.push_back(player);

            // Only teleport new players
            Group* grp = player->GetGroup();
            uint64 gguid = grp ? grp->GetGUID() : 0;
            if (!gguid || !grp->isLFGGroup() || GetState(gguid) == LFG_STATE_FINISHED_DUNGEON)
                playersToTeleport.push_back(player);
        }

        if (itPlayers->second->accept != LFG_ANSWER_AGREE)   // No answer (-1) or not accepted (0)
            allAnswered = false;
    }

    if (!allAnswered)
    {
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
            (*it)->GetSession()->SendLfgUpdateProposal(proposalId, pProposal);
    }
    else
    {
        bool sendUpdate = pProposal->state != LFG_PROPOSAL_SUCCESS;
        pProposal->state = LFG_PROPOSAL_SUCCESS;
        time_t joinTime = time_t(time(NULL));
        std::map<uint64, int32> waitTimesMap;
        // Save wait times before redoing groups
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            LfgProposalPlayer* player = pProposal->players[(*it)->GetGUID()];
            uint32 lowgroupguid = (*it)->GetGroup() ? GET_LOWGUID_PART((*it)->GetGroup()->GetGUID()) : 0;
            if (player->groupLowGuid != lowgroupguid)
                LOG_DEBUG("%u group mismatch: actual (%u) - queued (%u)", (*it)->GetGUID(), lowgroupguid, player->groupLowGuid);

            uint64 guid2 = player->groupLowGuid ? MAKE_NEW_GUID(player->groupLowGuid, 0, HIGHGUID_TYPE_GROUP) : (*it)->GetGUID();
            LfgQueueInfoMap::iterator itQueue = m_QueueInfoMap.find(guid2);
            if (itQueue == m_QueueInfoMap.end())
            {
                LOG_DEBUG("Queue info for guid %u not found!", guid);
                waitTimesMap[(*it)->GetGUID()] = -1;
            }
            else
                waitTimesMap[(*it)->GetGUID()] = int32(joinTime - itQueue->second->joinTime);
        }

        // Create a new group (if needed)
        LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_GROUP_FOUND);
        Group* grp = pProposal->groupLowGuid ? objmgr.GetGroupById(pProposal->groupLowGuid) : NULL;
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = (*it);
            uint64 pguid = player->GetGUID();
            Group* group = player->GetGroup();
            if (sendUpdate)
                player->GetSession()->SendLfgUpdateProposal(proposalId, pProposal);
            if (group)
            {
                player->GetSession()->SendLfgUpdateParty(updateData);
                if (group != grp)
                    player->GetGroup()->Disband();
            }
            else
                player->GetSession()->SendLfgUpdatePlayer(updateData);

            if (!grp)
            {
                grp = new Group(true);
                grp->m_disbandOnNoMembers = false;
                grp->ExpandToLFG();

#if VERSION_STRING != Cata
                DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(pProposal->dungeonId);
                SetDungeon(grp->GetGUID(), dungeon->Entry());
#endif

                // uint32 low_gguid = grp->GetID();
                uint64 gguid = grp->GetGUID();
                SetState(gguid, LFG_STATE_PROPOSAL);
                grp->AddMember(player->getPlayerInfo());
                LOG_DEBUG("Add Player In Group %s", player->GetNameString());
            }
            else if (group != grp)
            {
                grp->AddMember(player->getPlayerInfo());
                LOG_DEBUG("Add Player In Group %s", player->GetNameString());
            }

            // Update timers
            uint8 role = GetRoles(pguid);
            role &= ~ROLE_LEADER;
            switch (role)
            {
                case ROLE_DAMAGE:
                {
                    uint32 old_number = m_NumWaitTimeDps++;
                    m_WaitTimeDps = int32((m_WaitTimeDps * old_number + waitTimesMap[player->GetGUID()]) / m_NumWaitTimeDps);
                    break;
                }
                case ROLE_HEALER:
                {
                    uint32 old_number = m_NumWaitTimeHealer++;
                    m_WaitTimeHealer = int32((m_WaitTimeHealer * old_number + waitTimesMap[player->GetGUID()]) / m_NumWaitTimeHealer);
                    break;
                }
                case ROLE_TANK:
                {
                    uint32 old_number = m_NumWaitTimeTank++;
                    m_WaitTimeTank = int32((m_WaitTimeTank * old_number + waitTimesMap[player->GetGUID()]) / m_NumWaitTimeTank);
                    break;
                }
                default:
                {
                    uint32 old_number = m_NumWaitTimeAvg++;
                    m_WaitTimeAvg = int32((m_WaitTimeAvg * old_number + waitTimesMap[player->GetGUID()]) / m_NumWaitTimeAvg);
                    break;
                }
            }

            m_teleport.push_back(pguid);
            if(Player* plr = objmgr.GetPlayer(static_cast<uint32>(pguid)))
				plr->SetRoles(pProposal->players[pguid]->role);
            SetState(pguid, LFG_STATE_DUNGEON);
        }

#if VERSION_STRING != Cata
        DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(pProposal->dungeonId);
#endif
        //Set Dungeon difficult incomplete :D

        if (grp == nullptr) // something went definitely wrong if we end up here... I'm sure it is just bad code design.
            return;

        uint64 gguid = grp->GetGUID();
#if VERSION_STRING != Cata
        SetDungeon(gguid, dungeon->ID);
#endif
        SetState(gguid, LFG_STATE_DUNGEON);
        //Maybe Save these :D

        // Remove players/groups from Queue
        for (LfgGuidList::const_iterator it = pProposal->queues.begin(); it != pProposal->queues.end(); ++it)
            RemoveFromQueue(*it);

        // Teleport Player
        for (LfgPlayerList::const_iterator it = playersToTeleport.begin(); it != playersToTeleport.end(); ++it)
            TeleportPlayer(*it, false);

        // Update group info
        if (grp)
            grp->Update();

        delete pProposal;
        m_Proposals.erase(itProposal);
    }
}

void LfgMgr::RemoveProposal(LfgProposalMap::iterator itProposal, LfgUpdateType type)
{
    LfgProposal* pProposal = itProposal->second;
    pProposal->state = LFG_PROPOSAL_FAILED;

    LOG_DEBUG("Proposal %u, state FAILED, UpdateType %u", itProposal->first, type);
    // Mark all people that didn't answered as no accept
    if (type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        for (LfgProposalPlayerMap::const_iterator it = pProposal->players.begin(); it != pProposal->players.end(); ++it)
            if (it->second->accept == LFG_ANSWER_PENDING)
                it->second->accept = LFG_ANSWER_DENY;

    // Mark players/groups to be removed
    LfgGuidSet toRemove;
    for (LfgProposalPlayerMap::const_iterator it = pProposal->players.begin(); it != pProposal->players.end(); ++it)
    {
        if (it->second->accept == LFG_ANSWER_AGREE)
            continue;

        uint64 guid = it->second->groupLowGuid ? MAKE_NEW_GUID(it->second->groupLowGuid, 0, HIGHGUID_TYPE_GROUP) : it->first;
        // Player didn't accept or still pending when no secs left
        if (it->second->accept == LFG_ANSWER_DENY || type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        {
            it->second->accept = LFG_ANSWER_DENY;
            toRemove.insert(guid);
        }
    }

    uint8 team = 0;
    // Notify players
    for (LfgProposalPlayerMap::const_iterator it = pProposal->players.begin(); it != pProposal->players.end(); ++it)
    {
        Player* player = objmgr.GetPlayer(GET_LOWGUID_PART(it->first));
        if (!player)
            continue;

        team = uint8(player->GetTeam());
        player->GetSession()->SendLfgUpdateProposal(itProposal->first, pProposal);

        Group* grp = player->GetGroup();
        uint64 guid = player->GetGUID();
        uint64 gguid = it->second->groupLowGuid ? MAKE_NEW_GUID(it->second->groupLowGuid, 0, HIGHGUID_TYPE_GROUP) : guid;

        if (toRemove.find(gguid) != toRemove.end())         // Didn't accept or in same group that someone that didn't accept
        {
            LfgUpdateData updateData;
            if (it->second->accept == LFG_ANSWER_DENY)
            {
                updateData.updateType = type;
                LOG_DEBUG("%u didn't accept. Removing from queue and compatible cache", guid);
            }
            else
            {
                updateData.updateType = LFG_UPDATETYPE_REMOVED_FROM_QUEUE;
                LOG_DEBUG("%u in same group that someone that didn't accept. Removing from queue and compatible cache", guid);
            }
            ClearState(guid);
            if (grp)
            {
                RestoreState(gguid);
                player->GetSession()->SendLfgUpdateParty(updateData);
            }
            else
                player->GetSession()->SendLfgUpdatePlayer(updateData);
        }
        else
        {
            LOG_DEBUG("Readding %u to queue.", guid);
            SetState(guid, LFG_STATE_QUEUED);
            if (grp)
            {
                SetState(gguid, LFG_STATE_QUEUED);
                player->GetSession()->SendLfgUpdateParty(LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid), GetComment(guid)));
            }
            else
                player->GetSession()->SendLfgUpdatePlayer(LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid), GetComment(guid)));
        }
    }

    // Remove players/groups from queue
    for (LfgGuidSet::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        uint64 guid = *it;
        RemoveFromQueue(guid);
        pProposal->queues.remove(guid);
    }

    // Readd to queue
    for (LfgGuidList::const_iterator it = pProposal->queues.begin(); it != pProposal->queues.end(); ++it)
    {
        uint64 guid = *it;
        LfgGuidList& currentQueue = m_currentQueue[team];
        currentQueue.push_front(guid);         //Add GUID for high priority
        AddToQueue(guid, team);                //We have to add each GUID in newQueue to check for a new groups
    }

    delete pProposal;
    m_Proposals.erase(itProposal);

}

void LfgMgr::InitBoot(Group* grp, uint64 kicker, uint64 victim, std::string reason)
{
    if (!grp)
        return;
    uint64 gguid = grp->GetGUID();
    SetState(gguid, LFG_STATE_BOOT);

    LfgPlayerBoot* pBoot = new LfgPlayerBoot();
    pBoot->inProgress = true;
    pBoot->cancelTime = time_t(time(NULL)) + LFG_TIME_BOOT;
    pBoot->reason = reason;
    pBoot->victim = victim;
    pBoot->votedNeeded = GetVotesNeeded(gguid);
    PlayerSet players;

    // Set votes
    GroupMembersSet::iterator itx;
    for (itx = grp->GetSubGroup(0)->GetGroupMembersBegin(); itx != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
    {
        if (Player* plrg = (*itx)->m_loggedInPlayer)
        {
            uint64 guid = plrg->GetGUID();
            SetState(guid, LFG_STATE_BOOT);
            if (guid == victim)
                pBoot->votes[victim] = LFG_ANSWER_DENY;    // Victim auto vote NO
            else if (guid == kicker)
                pBoot->votes[kicker] = LFG_ANSWER_AGREE;   // Kicker auto vote YES
            else
            {
                pBoot->votes[guid] = LFG_ANSWER_PENDING;   // Other members need to vote
                players.insert(plrg);
            }
        }
    }

    // Notify players
    for (PlayerSet::const_iterator it = players.begin(); it != players.end(); ++it)
        (*it)->GetSession()->SendLfgBootPlayer(pBoot);

    m_Boots[grp->GetID()] = pBoot;
}

void LfgMgr::UpdateBoot(Player* player, bool accept)
{
    Group* grp = player ? player->GetGroup() : NULL;
    if (!grp)
        return;

    uint32 bootId = grp->GetID();
    uint64 guid = player->GetGUID();

    LfgPlayerBootMap::iterator itBoot = m_Boots.find(bootId);
    if (itBoot == m_Boots.end())
        return;

    LfgPlayerBoot* pBoot = itBoot->second;
    if (!pBoot)
        return;

    if (pBoot->votes[guid] != LFG_ANSWER_PENDING)          // Cheat check: Player can't vote twice
        return;

    pBoot->votes[guid] = LfgAnswer(accept);

    uint8 votesNum = 0;
    uint8 agreeNum = 0;
    for (LfgAnswerMap::const_iterator itVotes = pBoot->votes.begin(); itVotes != pBoot->votes.end(); ++itVotes)
    {
        if (itVotes->second != LFG_ANSWER_PENDING)
        {
            ++votesNum;
            if (itVotes->second == LFG_ANSWER_AGREE)
            {
                ++agreeNum;
            }
        }
    }

    if (agreeNum == pBoot->votedNeeded ||                  // Vote passed
        votesNum == pBoot->votes.size() ||                 // All voted but not passed
        (pBoot->votes.size() - votesNum + agreeNum) < pBoot->votedNeeded) // Vote didnt passed
    {
        // Send update info to all players
        pBoot->inProgress = false;
        for (LfgAnswerMap::const_iterator itVotes = pBoot->votes.begin(); itVotes != pBoot->votes.end(); ++itVotes)
        {
            uint64 pguid = itVotes->first;
            if (pguid != pBoot->victim)
            {
                SetState(pguid, LFG_STATE_DUNGEON);
                if (Player* plrg = objmgr.GetPlayer(GET_LOWGUID_PART(pguid)))
                {
                    plrg->GetSession()->SendLfgBootPlayer(pBoot);
                }
            }
        }

        uint64 gguid = grp->GetGUID();
        SetState(gguid, LFG_STATE_DUNGEON);
        if (agreeNum == pBoot->votedNeeded)                // Vote passed - Kick player
        {
            /* incomplete :D
            Player::RemoveFromGroup(grp, pBoot->victim);
            if (Player* victim = objmgr.GetPlayer(GET_LOWGUID_PART(pBoot->victim)))
            {
            TeleportPlayer(victim, true, false);
            SetState(pBoot->victim, LFG_STATE_NONE);
            }
            OfferContinue(grp);
            DecreaseKicksLeft(gguid);
            */
        }
        delete pBoot;
        m_Boots.erase(itBoot);
    }
}

void LfgMgr::TeleportPlayer(Player* player, bool out, bool fromOpcode /*= false*/)
{
    LOG_DEBUG("%u is being teleported %s", player->GetGUID(), out ? "out" : "in");
    if (out)
    {
        player->RemoveAura(LFG_SPELL_LUCK_OF_THE_DRAW);
        player->SafeTeleport(player->m_bgEntryPointMap, player->m_bgEntryPointInstance, player->m_bgEntryPointX, player->m_bgEntryPointY, player->m_bgEntryPointZ, player->m_bgEntryPointO);
        return;
    }

    // TODO Add support for LFG_TELEPORTERROR_FATIGUE
    LfgTeleportError error = LFG_TELEPORTERROR_OK;
    Group* grp = player->GetGroup();

    if (!grp || !grp->isLFGGroup())                        // should never happen, but just in case...
        error = LFG_TELEPORTERROR_INVALID_LOCATION;
    else if (!player->isAlive())
        error = LFG_TELEPORTERROR_PLAYER_DEAD;
    else
    {
#if VERSION_STRING != Cata
        uint64 gguid = grp->GetGUID();
        DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(GetDungeon(gguid));

        if (!dungeon)
            error = LFG_TELEPORTERROR_INVALID_LOCATION;
        else if (player->GetMapId() != uint32(dungeon->map))  // Do not teleport players in dungeon to the entrance
        {
            uint32 mapid = 0;
            float x = 0;
            float y = 0;
            float z = 0;
            float orientation = 0;

            if (!fromOpcode)
            {
                // Select a player inside to be teleported to
                GroupMembersSet::iterator itx;
                for (itx = grp->GetSubGroup(0)->GetGroupMembersBegin(); itx != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
                {
                    Player* plrg = (*itx)->m_loggedInPlayer;
                    if (plrg && plrg != player && plrg->GetMapId() == uint32(dungeon->map))
                    {
                        mapid = plrg->GetMapId();
                        x = plrg->GetPositionX();
                        y = plrg->GetPositionY();
                        z = plrg->GetPositionZ();
                        orientation = plrg->GetOrientation();
                    }
                }
            }

            if (!mapid)
            {
                MySQLStructure::AreaTrigger const* areaTrigger = sMySQLStore.getMapEntranceTrigger(dungeon->map);
                if (areaTrigger == nullptr)
                {
                    LOG_DEBUG("Failed to teleport %u: No areatrigger found for map: %u difficulty: %u", player->GetGUID(), dungeon->map, dungeon->difficulty);
                    error = LFG_TELEPORTERROR_INVALID_LOCATION;
                }
                else
                {
                    mapid = areaTrigger->mapId;
                    x = areaTrigger->x;
                    y = areaTrigger->y;
                    z = areaTrigger->z;
                    orientation = areaTrigger->o;
                }
            }

            if (error == LFG_TELEPORTERROR_OK)
            {
                player->SetBattlegroundEntryPoint();
                player->Dismount();
                if (!player->SafeTeleport(mapid, 0, x, y, z, orientation))
                    error = LFG_TELEPORTERROR_INVALID_LOCATION;
            }
        }
#endif
    }

    if (error != LFG_TELEPORTERROR_OK)
        player->GetSession()->SendLfgTeleportError(uint8(error));

    if (fromOpcode) { return;  }
}

void LfgMgr::RewardDungeonDoneFor(const uint32 dungeonId, Player* player)
{
#if VERSION_STRING != Cata
    Group* group = player->GetGroup();
    if (!group || !group->isLFGGroup())
    {
        LOG_DEBUG("%u is not in a group or not a LFGGroup. Ignoring", player->GetGUID());
        return;
    }

    uint64 guid = player->GetGUID();
    uint64 gguid = player->GetGroup()->GetGUID();
    uint32 gDungeonId = GetDungeon(gguid, true);
    if (gDungeonId != dungeonId)
    {
        LOG_DEBUG("%u Finished dungeon %u but group queued for %u. Ignoring", guid, dungeonId, gDungeonId);
        return;
    }

    if (GetState(guid) == LFG_STATE_FINISHED_DUNGEON)
    {
        LOG_DEBUG("%u Already rewarded player. Ignoring", guid);
        return;
    }

    // Mark dungeon as finished
    SetState(gguid, LFG_STATE_FINISHED_DUNGEON);

    // Clear player related lfg stuff
    uint32 rDungeonId = (*GetSelectedDungeons(guid).begin());
    ClearState(guid);
    SetState(guid, LFG_STATE_FINISHED_DUNGEON);

    
    // Give rewards only if its a random or seasonal dungeon
    DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(rDungeonId);
    if (!dungeon || (dungeon->type != LFG_TYPE_RANDOM /*add seasonal checks*/))
    {
        LOG_DEBUG("%u dungeon %u is not random nor seasonal", guid, rDungeonId);
        return;
    }

#if VERSION_STRING > TBC
    // Update achievements
    if (dungeon->difficulty == 1) // Heroic
        player->GetAchievementMgr().UpdateAchievementCriteria(player, 13029, 1); // Done LFG Dungeon with random Players
#endif

    LfgReward const* reward = GetRandomDungeonReward(rDungeonId, static_cast<uint8_t>(player->getLevel()));
    if (!reward)
        return;

    uint8 index = 0;
    QuestProperties const* qReward = sMySQLStore.getQuestProperties(reward->reward[index].questId);
    if (!qReward)
        return;

    // if we can take the quest, means that we haven't done this kind of "run", IE: First Heroic Random of Day.
    if (!player->HasFinishedDaily(qReward->id) || !player->HasFinishedQuest(qReward->id))
    {
        sQuestMgr.BuildQuestComplete(player, qReward);
        player->AddToFinishedQuests(qReward->id);

        // Reputation reward
        for (uint8 z = 0; z < 6; z++)
        {
            if (qReward->reward_repfaction[z])
            {
                int32 amt = 0;
                uint32 fact = qReward->reward_repfaction[z];
                if (qReward->reward_repvalue[z])
                {
                    amt = qReward->reward_repvalue[z];
                }
                if (qReward->reward_replimit && (player->GetStanding(fact) >= (int32)qReward->reward_replimit))
                {
                    continue;
                }
                amt = float2int32(amt * worldConfig.getFloatRate(RATE_QUESTREPUTATION));
                player->ModStanding(fact, amt);
            }
        }
        // Static Item reward
        for (uint8 i = 0; i < 4; ++i)
        {
            if (qReward->reward_item[i])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qReward->reward_item[i]);
                if (!proto)
                {
                    LOG_ERROR("Invalid item prototype in quest reward! ID %d, quest %d", qReward->reward_item[i], qReward->id);
                }
                else
                {
                    auto item_add = player->GetItemInterface()->FindItemLessMax(qReward->reward_item[i], qReward->reward_itemcount[i], false);
                    if (!item_add)
                    {
                        auto slotresult = player->GetItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = objmgr.CreateItem(qReward->reward_item[i], player);
                            if (item)
                            {
                                item->SetStackCount(uint32(qReward->reward_itemcount[i]));
                                if (!player->GetItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
                                {
                                    item->DeleteMe();
                                }
                            }
                        }
                    }
                    else
                    {
                        item_add->SetStackCount(item_add->GetStackCount() + qReward->reward_itemcount[i]);
                        item_add->m_isDirty = true;
                    }
                }
            }
        }

        // if daily then append to finished dailies
        if (qReward->is_repeatable == arcemu_QUEST_REPEATABLE_DAILY)
            player->PushToFinishedDailies(qReward->id);
#if VERSION_STRING > TBC
        player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT, 1, 0, 0);
#endif
        if (qReward->reward_money > 0)
        {
            // Money reward
            // Check they don't have more than the max gold
            if (worldConfig.player.isGoldCapEnabled && (player->GetGold() + qReward->reward_money) <= worldConfig.player.limitGoldAmount)
            {
                player->ModGold(qReward->reward_money);
            }
#if VERSION_STRING > TBC
            player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD, qReward->reward_money, 0, 0);
#endif
        }
#if VERSION_STRING > TBC
        player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE, qReward->zone_id, 0, 0);
        player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, qReward->id, 0, 0);
#endif
    }
    else
    {
        index = 1;
        qReward = sMySQLStore.getQuestProperties(reward->reward[index].questId);
        if (!qReward)
            return;

        sQuestMgr.BuildQuestComplete(player, qReward);
        player->AddToFinishedQuests(qReward->id);

        // Reputation reward
        for (uint8 z = 0; z < 6; z++)
        {
            if (qReward->reward_repfaction[z])
            {
                int32 amt = 0;
                uint32 fact = qReward->reward_repfaction[z];
                if (qReward->reward_repvalue[z])
                {
                    amt = qReward->reward_repvalue[z];
                }
                if (qReward->reward_replimit && (player->GetStanding(fact) >= (int32)qReward->reward_replimit))
                {
                    continue;
                }
                amt = float2int32(amt * worldConfig.getFloatRate(RATE_QUESTREPUTATION));
                player->ModStanding(fact, amt);
            }
        }
        // Static Item reward
        for (uint8 i = 0; i < 4; ++i)
        {
            if (qReward->reward_item[i])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qReward->reward_item[i]);
                if (!proto)
                {
                    LOG_ERROR("Invalid item prototype in quest reward! ID %d, quest %d", qReward->reward_item[i], qReward->id);
                }
                else
                {
                    auto item_add = player->GetItemInterface()->FindItemLessMax(qReward->reward_item[i], qReward->reward_itemcount[i], false);
                    if (!item_add)
                    {
                        auto slotresult = player->GetItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            player->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = objmgr.CreateItem(qReward->reward_item[i], player);
                            if (item)
                            {
                                item->SetStackCount(uint32(qReward->reward_itemcount[i]));
                                if (!player->GetItemInterface()->SafeAddItem(item, slotresult.ContainerSlot, slotresult.Slot))
                                {
                                    item->DeleteMe();
                                }
                            }
                        }
                    }
                    else
                    {
                        item_add->SetStackCount(item_add->GetStackCount() + qReward->reward_itemcount[i]);
                        item_add->m_isDirty = true;
                    }
                }
            }
        }

        // if daily then append to finished dailies
        if (qReward->is_repeatable == arcemu_QUEST_REPEATABLE_DAILY)
            player->PushToFinishedDailies(qReward->id);

#if VERSION_STRING > TBC
        player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT, 1, 0, 0);
#endif
        if (qReward->reward_money > 0)
        {
            // Money reward
            // Check they don't have more than the max gold
            if (worldConfig.player.isGoldCapEnabled && (player->GetGold() + qReward->reward_money) <= worldConfig.player.limitGoldAmount)
            {
                player->ModGold(qReward->reward_money);
            }
#if VERSION_STRING > TBC
            player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD, qReward->reward_money, 0, 0);
#endif
        }
#if VERSION_STRING > TBC
        player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE, qReward->zone_id, 0, 0);
        player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, qReward->id, 0, 0);
#endif
    }

    // Give rewards
    //Log.Debug("LfgMgr", "LfgMgr::RewardDungeonDoneFor: %u done dungeon %u, %s previously done.", player->GetGUID(), GetDungeon(gguid), index > 0 ? " " : " not");
    LOG_DEBUG("%u done dungeon %u, previously done.", player->GetGUID(), GetDungeon(gguid));
    player->GetSession()->SendLfgPlayerReward(dungeon->Entry(), GetDungeon(gguid, false), index, reward, qReward);
#else
    if (player == nullptr ||dungeonId == 0) { return; }
#endif
}

const LfgDungeonSet& LfgMgr::GetDungeonsByRandom(uint32 randomdungeon)
{
#if VERSION_STRING != Cata
    DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(randomdungeon);
    uint32 groupType = dungeon ? dungeon->grouptype : 0;
    return m_CachedDungeonMap[groupType];
#else
    if (randomdungeon == 0) { return m_CachedDungeonMap[0]; }
    return m_CachedDungeonMap[0];
#endif
}

LfgReward const* LfgMgr::GetRandomDungeonReward(uint32 dungeon, uint8 level)
{
	LOG_DEBUG("Get Reward dungeon id = %u level = %u", dungeon, level);
    LfgReward const* rew = NULL;
    LfgRewardMapBounds bounds = m_RewardMap.equal_range(dungeon & 0x00FFFFFF);
    for (LfgRewardMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        rew = itr->second;
        // ordered properly at loading
        if (itr->second->maxLevel >= level)
            break;
    }

    return rew;
}

LfgType LfgMgr::GetDungeonType(uint32 dungeonId)
{
#if VERSION_STRING != Cata
    DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId);
    if (!dungeon)
        return LFG_TYPE_NONE;

    return LfgType(dungeon->type);
#else
    if (dungeonId == 0) { return LfgType(0); }
    return LfgType(0);
#endif
}

std::string LfgMgr::ConcatenateGuids(LfgGuidList check)
{
    if (check.empty())
        return "";

    std::ostringstream o;
    LfgGuidList::const_iterator it = check.begin();
    o << (*it);
    for (++it; it != check.end(); ++it)
        o << '|' << (*it);
    return o.str();
}

LfgState LfgMgr::GetState(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    if (HIGHGUID_TYPE_GROUP == guid)
        return m_Groups[guid].GetState();
    else
        return m_Players[guid].GetState();
}

uint32 LfgMgr::GetDungeon(uint64 guid, bool asId /*= true*/)
{
    LOG_DEBUG("%u asId: %u", guid, asId);
    return m_Groups[guid].GetDungeon(asId);
}

uint8 LfgMgr::GetRoles(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    return m_Players[guid].GetRoles();
}

const std::string& LfgMgr::GetComment(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    return m_Players[guid].GetComment();
}

bool LfgMgr::IsTeleported(uint64 pguid)
{
    if (std::find(m_teleport.begin(), m_teleport.end(), pguid) != m_teleport.end())
    {
        m_teleport.remove(pguid);
        return true;
    }
    return false;
}

const LfgDungeonSet& LfgMgr::GetSelectedDungeons(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    return m_Players[guid].GetSelectedDungeons();
}

const LfgLockMap& LfgMgr::GetLockedDungeons(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    return m_Players[guid].GetLockedDungeons();
}

uint8 LfgMgr::GetKicksLeft(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    return m_Groups[guid].GetKicksLeft();
}

uint8 LfgMgr::GetVotesNeeded(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    return m_Groups[guid].GetVotesNeeded();
}

void LfgMgr::RestoreState(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    m_Groups[guid].RestoreState();
}

void LfgMgr::ClearState(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    m_Players[guid].ClearState();
}

void LfgMgr::SetState(uint64 guid, LfgState state)
{
    LOG_DEBUG("%u state %u", guid, state);
    if (IS_GROUP(guid))
        m_Groups[guid].SetState(state);
    else
        m_Players[guid].SetState(state);

}

void LfgMgr::SetDungeon(uint64 guid, uint32 dungeon)
{
    LOG_DEBUG("%u dungeon %u", guid, dungeon);
    m_Groups[guid].SetDungeon(dungeon);
}

void LfgMgr::SetRoles(uint64 guid, uint8 roles)
{
    LOG_DEBUG("%u roles: %u", guid, roles);
    m_Players[guid].SetRoles(roles);
}

void LfgMgr::SetComment(uint64 guid, const std::string& comment)
{
    LOG_DEBUG("%u comment: %s", guid, comment.c_str());
    m_Players[guid].SetComment(comment);
}

void LfgMgr::SetSelectedDungeons(uint64 guid, const LfgDungeonSet& dungeons)
{
    LOG_DEBUG("%u", guid);
    m_Players[guid].SetSelectedDungeons(dungeons);
}

void LfgMgr::SetLockedDungeons(uint64 guid, const LfgLockMap& lock)
{
    LOG_DEBUG("%u", guid);
    m_Players[guid].SetLockedDungeons(lock);
}

void LfgMgr::DecreaseKicksLeft(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    m_Groups[guid].DecreaseKicksLeft();
}

void LfgMgr::RemovePlayerData(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    LfgPlayerDataMap::iterator it = m_Players.find(guid);
    if (it != m_Players.end())
        m_Players.erase(it);
}

void LfgMgr::RemoveGroupData(uint64 guid)
{
    LOG_DEBUG("%u", guid);
    LfgGroupDataMap::iterator it = m_Groups.find(guid);
    if (it != m_Groups.end())
        m_Groups.erase(it);
}
