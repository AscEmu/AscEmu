/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "Storage/WDB/WDBStores.hpp"
#include "Macros/LFGMacros.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Objects/Item.hpp"
#include "Management/LFG/LFGMgr.hpp"

#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/LFG/LFGGroupData.hpp"
#include "Management/LFG/LFGPlayerData.hpp"
#include "Management/ItemInterface.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"

uint32_t LfgDungeonTypes[MAX_DUNGEONS];

LfgMgr& LfgMgr::getInstance()
{
    static LfgMgr mInstance;
    return mInstance;
}

void LfgMgr::initialize()
{
    m_update = true;
    m_QueueTimer = 0;
    m_lfgProposalId = 1;
    m_WaitTimeAvg = -1;
    m_WaitTimeTank = -1;
    m_WaitTimeHealer = -1;
    m_WaitTimeDps = -1;
    m_NumWaitTimeAvg = 0;
    m_NumWaitTimeTank = 0;
    m_NumWaitTimeHealer = 0;
    m_NumWaitTimeDps = 0;

#if VERSION_STRING < Cata
    // Initialize dungeon cache
    for (uint32_t i = 0; i < sLFGDungeonStore.getNumRows(); ++i)
    {
        WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(i);
        if (dungeon && dungeon->type != LFG_TYPE_ZONE)
        {
            if (dungeon->type != LFG_TYPE_RANDOM)
                m_CachedDungeonMap[dungeon->grouptype].insert(dungeon->ID);
            m_CachedDungeonMap[0].insert(dungeon->ID);
        }
    }
#endif
}

void LfgMgr::finalize()
{
    m_RewardMap.clear();
    m_QueueInfoMap.clear();
    m_Boots.clear();
    m_RoleChecks.clear();
    m_Proposals.clear();
}

/// Load rewards for completing dungeons
void LfgMgr::LoadRewards()
{
    auto startTime = Util::TimeNow();

    m_RewardMap.clear();

    // ORDER BY is very important for GetRandomDungeonReward!
    auto result = WorldDatabase.Query("SELECT dungeon_id, max_level, quest_id_1, money_var_1, xp_var_1, quest_id_2, money_var_2, xp_var_2 FROM lfg_dungeon_rewards ORDER BY dungeon_id, max_level ASC");
    if (result == nullptr)
    {
        sLogger.failure("Loaded 0 lfg dungeon rewards.DB table `lfg_dungeon_rewards` is empty!\n");
        return;
    }

    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32_t dungeonId = fields[0].asUint32();
        uint32_t maxLevel = fields[1].asUint8();
        uint32_t firstQuestId = fields[2].asUint32();
        uint32_t firstMoneyVar = fields[3].asUint32();
        uint32_t firstXPVar = fields[4].asUint32();
        uint32_t otherQuestId = fields[5].asUint32();
        uint32_t otherMoneyVar = fields[6].asUint32();
        uint32_t otherXPVar = fields[7].asUint32();

#if VERSION_STRING < Cata
        if (!sLFGDungeonStore.lookupEntry(dungeonId))
        {
            sLogger.debug("Dungeon {} specified in table `lfg_dungeon_rewards` does not exist!", dungeonId);
            continue;
        }
#endif

        if (!maxLevel || maxLevel > 80)
        {
            sLogger.debug("Level {} specified for dungeon {} in table `lfg_dungeon_rewards` can never be reached!", maxLevel, dungeonId);
            maxLevel = 80;
        }

        if (firstQuestId && !sMySQLStore.getQuestProperties(firstQuestId))
        {
            sLogger.debug("First quest {} specified for dungeon {} in table `lfg_dungeon_rewards` does not exist!", firstQuestId, dungeonId);
            firstQuestId = 0;
        }

        if (otherQuestId && !sMySQLStore.getQuestProperties(otherQuestId))
        {
            sLogger.debug("Other quest {} specified for dungeon {} in table `lfg_dungeon_rewards` does not exist!", otherQuestId, dungeonId);
            otherQuestId = 0;
        }

//#if VERSION_STRING < Cata
        //WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(dungeonId);
//#endif
        m_RewardMap.emplace(dungeonId, std::make_unique<LfgReward>(maxLevel, firstQuestId, firstMoneyVar, firstXPVar, otherQuestId, otherMoneyVar, otherXPVar));
        ++count;
    }
    while (result->NextRow());

    sLogger.info("LFGMgr : Loaded {} lfg dungeon rewards in {} ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void LfgMgr::Update(uint32_t diff)
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
        LfgRoleCheck* roleCheck = itRoleCheck->second.get();
        if (currTime < roleCheck->cancelTime)
        {
            continue;
        }

        roleCheck->state = LFG_ROLECHECK_MISSING_ROLE;

        for (LfgRolesMap::const_iterator itRoles = roleCheck->roles.begin(); itRoles != roleCheck->roles.end(); ++itRoles)
        {
            uint64_t guid = itRoles->first;
            ClearState(guid);

            WoWGuid wowGuid;
            wowGuid.Init(guid);

            if (Player* player = sObjectMgr.getPlayer(wowGuid.getGuidLowPart()))
            {
                player->getSession()->sendLfgRoleCheckUpdate(roleCheck);

                if (itRoles->first == roleCheck->leader)
                {
                    player->getSession()->sendLfgJoinResult(LfgJoinResultData(LFG_JOIN_FAILED, LFG_ROLECHECK_MISSING_ROLE));
                }
            }
        }
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
        LfgPlayerBoot* pBoot = itBoot->second.get();
        if (pBoot->cancelTime < currTime)
        {
            pBoot->inProgress = false;
            for (LfgAnswerMap::const_iterator itVotes = pBoot->votes.begin(); itVotes != pBoot->votes.end(); ++itVotes)
            {
                WoWGuid wowGuid;
                wowGuid.Init(itVotes->first);

                if (Player* plrg = sObjectMgr.getPlayer(wowGuid.getGuidLowPart()))
                {
                    if (plrg->getGuid() != pBoot->victim)
                    {
                        plrg->getSession()->sendLfgBootPlayer(pBoot);
                    }
                }
            }

            m_Boots.erase(itBoot);
        }
    }

    // Check if a proposal can be formed with the new groups being added
    for (LfgGuidListMap::iterator it = m_newToQueue.begin(); it != m_newToQueue.end(); ++it)
    {
        uint8_t queueId = it->first;
        LfgGuidList& newToQueue = it->second;
        LfgGuidList& currentQueue = m_currentQueue[queueId];
        LfgGuidList firstNew;
        while (!newToQueue.empty())
        {
            uint64_t frontguid = newToQueue.front();
            sLogger.debug("Update: QueueId {}: checking {} newToQueue({}), currentQueue({})", queueId, frontguid, uint32_t(newToQueue.size()), uint32_t(currentQueue.size()));
            firstNew.push_back(frontguid);
            newToQueue.pop_front();

            LfgGuidList temporalList = currentQueue;
            if (auto proposalHolder = FindNewGroups(firstNew, temporalList)) // Group found!
            {
                // Remove groups in the proposal from new and current queues (not from queue map)
                for (LfgGuidList::const_iterator itQueue = proposalHolder->queues.begin(); itQueue != proposalHolder->queues.end(); ++itQueue)
                {
                    currentQueue.remove(*itQueue);
                    newToQueue.remove(*itQueue);
                }

                auto* pProposal = proposalHolder.get();
                m_Proposals[++m_lfgProposalId] = std::move(proposalHolder);

                uint64_t guid = 0;
                for (LfgProposalPlayerMap::const_iterator itPlayers = pProposal->players.begin(); itPlayers != pProposal->players.end(); ++itPlayers)
                {
                    guid = itPlayers->first;
                    SetState(guid, LFG_STATE_PROPOSAL);

                    WoWGuid wowGuid;
                    wowGuid.Init(itPlayers->first);

                    if (Player* player = sObjectMgr.getPlayer(wowGuid.getGuidLowPart()))
                    {
                        if (auto group = player->getGroup())
                        {
                            uint64_t gguid = group->GetID();
                            SetState(gguid, LFG_STATE_PROPOSAL);
                            player->getSession()->sendLfgUpdateParty(LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        }
                        else
                        {
                            player->getSession()->sendLfgUpdatePlayer(LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid), GetComment(guid)));
                        }

                        player->getSession()->sendLfgUpdateProposal(m_lfgProposalId, pProposal);
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
            LfgQueueInfo* queue = itQueue->second.get();
            if (!queue)
            {
                sLogger.debug("Update: {} queued with null queue info!", itQueue->first);
                continue;
            }
            uint32_t dungeonId = (*queue->dungeons.begin());
            uint32_t queuedTime = uint32_t(currTime - queue->joinTime);
            uint8_t role = ROLE_NONE;
            for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
            {
                role |= itPlayer->second;
            }

            role &= ~ROLE_LEADER;

            int32_t waitTime = -1;
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
                WoWGuid wowGuid;
                wowGuid.Init(itPlayer->first);

                if (Player* player = sObjectMgr.getPlayer(wowGuid.getGuidLowPart()))
                {
                    player->getSession()->sendLfgQueueStatus(dungeonId, waitTime, m_WaitTimeAvg, m_WaitTimeTank, m_WaitTimeHealer, m_WaitTimeDps, queuedTime, queue->tanks, queue->healers, queue->dps);
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

void LfgMgr::AddToQueue(uint64_t guid, uint8_t queueId)
{
    //queueId = 0; //allows horde and alli in same queque

    LfgGuidList& list = m_newToQueue[queueId];
    if (std::find(list.begin(), list.end(), guid) != list.end())
    {
        sLogger.debug("{} already in new queue. ignoring", guid);
    }
    else
    {
        list.push_back(guid);
        sLogger.debug("{} added to m_newToQueue (size: {})", guid, uint32_t(list.size()));
    }
}

bool LfgMgr::RemoveFromQueue(uint64_t guid)
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
        m_QueueInfoMap.erase(it);
        sLogger.debug("{} removed", guid);
        return true;
    }
    else
    {
        sLogger.debug("{} not in queue", guid);
        return false;
    }

}

void LfgMgr::InitializeLockedDungeons(Player* player)
{
    uint64_t guid = player->getGuid();
    uint8_t level = player->getLevel();
    uint8_t expansion = player->getSession()->GetFlags();

#if VERSION_STRING < Cata
    LfgDungeonSet dungeons = GetDungeonsByRandom(0);
    LfgLockMap lock;

    for (LfgDungeonSet::const_iterator it = dungeons.begin(); it != dungeons.end(); ++it)
    {
        WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(*it);
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

void LfgMgr::Join(Player* player, uint8_t roles, const LfgDungeonSet& selectedDungeons, const std::string& comment)
{
    if (!player || !player->getSession() || selectedDungeons.empty())
        return;

    auto group = player->getGroup();
    uint64_t guid = player->getGuid();
    uint64_t gguid = group ? group->GetID() : guid;
    LfgJoinResultData joinData;
    PlayerSet players;
    uint32_t rDungeonId = 0;
    bool isContinue = group && group->isLFGGroup() && GetState(gguid) != LFG_STATE_FINISHED_DUNGEON;
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
            player->getSession()->sendLfgJoinResult(joinData); // Default value of joinData.result = LFG_JOIN_OK
            if (group)
            {
                for (const auto& itx : group->GetSubGroup(0)->getGroupMembers())
                {
                    if (Player* loggedInPlayer = sObjectMgr.getPlayer(itx->guid))
                        loggedInPlayer->getSession()->sendLfgUpdateParty(updateData);
                }
            }
            return;
        }

        RemoveFromQueue(gguid);
    }

    // Check player or group member restrictions
    if (player->hasQueuedBgInstanceId() || player->isQueuedForBg())
    {
        joinData.result = LFG_JOIN_USING_BG_SYSTEM;
    }
    else if (player->hasAurasWithId(LFG_SPELL_DUNGEON_DESERTER))
    {
        joinData.result = LFG_JOIN_DESERTER;
    }
    else if (player->hasAurasWithId(LFG_SPELL_DUNGEON_COOLDOWN))
    {
        joinData.result = LFG_JOIN_RANDOM_COOLDOWN;
    }
    else if (dungeons.empty())
    {
        joinData.result = LFG_JOIN_NOT_MEET_REQS;
    }
    else if (group)
    {
        if (group->MemberCount() > 5)
        {
            joinData.result = LFG_JOIN_TOO_MUCH_MEMBERS;
        }
        else
        {
            uint8_t memberCount = 0;
            for (const auto& itx : group->GetSubGroup(0)->getGroupMembers())
            {
                if (Player* plrg = sObjectMgr.getPlayer(itx->guid))
                {
                    if (joinData.result == LFG_JOIN_OK)
                    {
                        if (plrg->hasAurasWithId(LFG_SPELL_DUNGEON_DESERTER))
                            joinData.result = LFG_JOIN_PARTY_DESERTER;
                        else if (plrg->hasAurasWithId(LFG_SPELL_DUNGEON_COOLDOWN))
                            joinData.result = LFG_JOIN_PARTY_RANDOM_COOLDOWN;
                        else if (plrg->hasQueuedBgInstanceId() || plrg->isQueuedForBg())
                            joinData.result = LFG_JOIN_USING_BG_SYSTEM;

                        ++memberCount;
                        players.insert(plrg);
                    }
                }
            }
            if (memberCount != group->MemberCount())
                joinData.result = LFG_JOIN_DISCONNECTED;
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
        sLogger.debug("JOIN QU LFG_JOIN_OK");
        bool isDungeon = false;
        for (LfgDungeonSet::const_iterator it = dungeons.begin(); it != dungeons.end() && joinData.result == LFG_JOIN_OK; ++it)
        {
            switch (GetDungeonType(*it))
            {
                case LFG_TYPE_RANDOM:
                {
                    if (dungeons.size() > 1)                            // Only allow 1 random dungeon
                        joinData.result = LFG_JOIN_DUNGEON_INVALID;
                    else
                        rDungeonId = (*dungeons.begin());
                }
                // No break on purpose (Random can only be dungeon or heroic dungeon)
                case LFG_TYPE_HEROIC:
                case LFG_TYPE_DUNGEON:
                {
                    if (isRaid)
                        joinData.result = LFG_JOIN_MIXED_RAID_DUNGEON;

                    isDungeon = true;
                } break;
                case LFG_TYPE_RAID:
                {
                    if (isDungeon)
                        joinData.result = LFG_JOIN_MIXED_RAID_DUNGEON;

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
                dungeons = GetDungeonsByRandom(rDungeonId);

            // if we have lockmap then there are no compatible dungeons
            GetCompatibleDungeons(dungeons, players, joinData.lockmap);
            if (dungeons.empty())
            {
                joinData.result = group ? LFG_JOIN_PARTY_NOT_MEET_REQS : LFG_JOIN_NOT_MEET_REQS;
            }
        }
    }

    // Can't join. Send result
    if (joinData.result != LFG_JOIN_OK)
    {
        sLogger.debug("{} joining with {} members. result: {}   cant join return", guid, group ? group->MemberCount() : 1, joinData.result);
        if (!dungeons.empty())                             // Only should show lockmap when have no dungeons available
            joinData.lockmap.clear();

        player->getSession()->sendLfgJoinResult(joinData);
        return;
    }

    // \todo Raid browser not supported yet
    if (isRaid)
    {
        sLogger.debug("{} trying to join raid browser and it's disabled.", guid);
        return;
    }

    SetComment(guid, comment);

    if (group)        // Begin rolecheck
    {
        // Create new rolecheck
        auto roleCheck = std::make_unique<LfgRoleCheck>();
        roleCheck->cancelTime = time_t(time(nullptr)) + LFG_TIME_ROLECHECK;
        roleCheck->state = LFG_ROLECHECK_INITIALITING;
        roleCheck->leader = guid;
        roleCheck->dungeons = dungeons;
        roleCheck->rDungeonId = rDungeonId;
        m_RoleChecks.try_emplace(gguid, std::move(roleCheck));

        if (rDungeonId)
        {
            dungeons.clear();
            dungeons.insert(rDungeonId);
        }

        SetState(gguid, LFG_STATE_ROLECHECK);
        // Send update to player
        LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_JOIN_PROPOSAL, dungeons, comment);
        for (const auto& itx : group->GetSubGroup(0)->getGroupMembers())
        {
            if (Player* plrg = sObjectMgr.getPlayer(itx->guid))
            {
                uint64_t pguid = plrg->getGuid();
                plrg->getSession()->sendLfgUpdateParty(updateData);
                SetState(pguid, LFG_STATE_ROLECHECK);

                if (!isContinue)
                    SetSelectedDungeons(pguid, dungeons);

                roleCheck->roles[pguid] = 0;
            }
        }
        // Update leader role
        UpdateRoleCheck(gguid, guid, roles);
    }
    else        // Add player to queue
    {
        // Queue player
        auto pqInfo = std::make_unique<LfgQueueInfo>();
        pqInfo->joinTime = time_t(time(nullptr));
        pqInfo->roles[player->getGuid()] = roles;
        pqInfo->dungeons = dungeons;

        if (roles & ROLE_TANK)
            --pqInfo->tanks;
        else if (roles & ROLE_HEALER)
            --pqInfo->healers;
        else
            --pqInfo->dps;

        m_QueueInfoMap.try_emplace(guid, std::move(pqInfo));

        // Send update to player
        player->getSession()->sendLfgJoinResult(joinData);
        player->getSession()->sendLfgUpdatePlayer(LfgUpdateData(LFG_UPDATETYPE_JOIN_PROPOSAL, dungeons, comment));
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

        AddToQueue(guid, uint8_t(player->getTeam()));
    }

    sLogger.debug("{} joined with {} members. dungeons: {}", guid, group ? group->MemberCount() : 1, uint8_t(dungeons.size()));
}

void LfgMgr::Leave(Player* player, Group* _group /* = nullptr*/)
{
    if (!player && !_group)
        return;

    uint64_t guid = _group ? _group->GetID() : player->getGuid();
    LfgState state = GetState(guid);

    sLogger.debug("{}", guid);
    switch (state)
    {
        case LFG_STATE_QUEUED:
        {
            RemoveFromQueue(guid);
            LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE);

            if (_group)
            {
                RestoreState(guid);
                for (const auto& itx : _group->GetSubGroup(0)->getGroupMembers())
                {
                    if (Player* plrg = sObjectMgr.getPlayer(itx->guid))
                    {
                        plrg->getSession()->sendLfgUpdateParty(updateData);
                        uint64_t pguid = plrg->getGuid();
                        ClearState(pguid);
                    }
                }
            }
            else
            {
                player->getSession()->sendLfgUpdatePlayer(updateData);
                ClearState(guid);
            }
        }
        break;
        case LFG_STATE_ROLECHECK:
        {
            if (_group)
                UpdateRoleCheck(guid);                     // No player to update role = LFG_ROLECHECK_ABORTED

        } break;
        case LFG_STATE_PROPOSAL:
        {
            // Remove from Proposals
            LfgProposalMap::iterator it = m_Proposals.begin();
            while (it != m_Proposals.end())
            {
                LfgProposalPlayerMap::iterator itPlayer = it->second->players.find(player ? player->getGuid() : _group->GetLeader()->guid);
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
                RemoveProposal(it, LFG_UPDATETYPE_PROPOSAL_DECLINED);

        } break;
        default:
            break;
    }
}

void LfgMgr::OfferContinue(Group* grp)
{
    if (grp)
    {
        uint64_t gguid = grp->GetID();
        if (Player* leader = sObjectMgr.getPlayer(grp->GetLeader()->guid))
            leader->getSession()->sendLfgOfferContinue(GetDungeon(gguid, false));

        sLogger.debug("player {} ", fmt::ptr(sObjectMgr.getPlayer(grp->GetLeader()->guid)));
    }
}

std::unique_ptr<LfgProposal> LfgMgr::FindNewGroups(LfgGuidList& check, LfgGuidList& all)
{
    sLogger.debug("({}) - all({})", ConcatenateGuids(check), ConcatenateGuids(all));

    std::unique_ptr<LfgProposal> pProposal = nullptr;
    if (check.empty() || check.size() > 5 || !CheckCompatibility(check, pProposal))
        return nullptr;

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

bool LfgMgr::CheckCompatibility(LfgGuidList check, std::unique_ptr<LfgProposal>& pProposal)
{
    if (pProposal)                                         // Do not check anything if we already have a proposal
        return false;

    std::string strGuids = ConcatenateGuids(check);

    if (check.size() > 5 || check.empty())
    {
        sLogger.debug("({}): Size wrong - Not compatibles", strGuids);
        return false;
    }

    if (check.size() == 1)
    {
        WoWGuid wowGuid;
        wowGuid.Init(check.front());

        if (wowGuid.isPlayer())
            return true;
    }

    // Previously cached?
    LfgAnswer answer = GetCompatibles(strGuids);
    if (answer != LFG_ANSWER_PENDING)
    {
        sLogger.debug("({}) compatibles (cached): {}", strGuids, answer);
        return answer == LFG_ANSWER_AGREE ? true : false;
    }

    // Check all but new compatiblitity
    if (check.size() > 2)
    {
        uint64_t frontGuid = check.front();
        check.pop_front();

        // Check all-but-new compatibilities (New, A, B, C, D) --> check(A, B, C, D)
        if (!CheckCompatibility(check, pProposal))          // Group not compatible
        {
            sLogger.debug("({}) not compatibles ({} not compatibles)", strGuids, ConcatenateGuids(check));
            SetCompatibles(strGuids, false);
            return false;
        }
        check.push_front(frontGuid);
        // all-but-new compatibles, now check with new
    }

    uint8_t numPlayers = 0;
    uint8_t numLfgGroups = 0;
    uint32_t groupLowGuid = 0;
    LfgRawQueueInfoMap pqInfoMap;
    for (LfgGuidList::const_iterator it = check.begin(); it != check.end() && numLfgGroups < 2 && numPlayers <= 5; ++it)
    {
        uint64_t guid = (*it);
        LfgQueueInfoMap::iterator itQueue = m_QueueInfoMap.find(guid);
        if (itQueue == m_QueueInfoMap.end() || GetState(guid) != LFG_STATE_QUEUED)
        {
            sLogger.debug("{} is not queued but listed as queued!", (*it));
            RemoveFromQueue(guid);
            return false;
        }
        pqInfoMap[guid] = itQueue->second.get();
        numPlayers += static_cast<uint8_t>(itQueue->second->roles.size());

        WoWGuid wowGuid;
        wowGuid.Init(guid);

        if (wowGuid.isGroup())
        {
            uint32_t lowGuid = wowGuid.getGuidLowPart();
            if (auto grp = sObjectMgr.getGroupById(lowGuid))  //MAy Check these
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
            sLogger.debug("({}) More than one Lfggroup ({})", strGuids, numLfgGroups);
        }
        else
        {
            sLogger.debug("({}) Too much players ({})", strGuids, numPlayers);
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Player checks
    LfgRolesMap rolesMap;
    uint64_t leader = 0;
    for (LfgRawQueueInfoMap::const_iterator it = pqInfoMap.begin(); it != pqInfoMap.end(); ++it)
    {
        for (LfgRolesMap::const_iterator itRoles = it->second->roles.begin(); itRoles != it->second->roles.end(); ++itRoles)
        {
            // Assign new leader
            if (itRoles->second & ROLE_LEADER && (!leader || Util::getRandomUInt(1)))
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
        WoWGuid wowGuid;
        wowGuid.Init(it->first);

        if (Player* player = sObjectMgr.getPlayer(wowGuid.getGuidLowPart()))
        {
            //for (PlayerSet::const_iterator itPlayer = players.begin(); itPlayer != players.end() && player; ++itPlayer)
            //{
            //    // Do not form a group with ignoring candidates
            //}

            players.insert(player);
        }
        else
        {
            sLogger.debug("({}) Warning! {} offline! Marking as not compatibles!", strGuids, it->first);
        }
    }

    // if we dont have the same ammount of players then we have self ignoring candidates or different faction groups
    // otherwise check if roles are compatible
    if (players.size() != numPlayers || !CheckGroupRoles(rolesMap))
    {
        if (players.size() == numPlayers)
        {
            sLogger.debug("({}) Roles not compatible", strGuids);
        }

        SetCompatibles(strGuids, false);
        return false;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    // Selected Dungeon checks
    // Check if there are any compatible dungeon from the selected dungeons
    LfgDungeonSet compatibleDungeons;

    LfgRawQueueInfoMap::const_iterator itFirst = pqInfoMap.begin();
    for (LfgDungeonSet::const_iterator itDungeon = itFirst->second->dungeons.begin(); itDungeon != itFirst->second->dungeons.end(); ++itDungeon)
    {
        LfgRawQueueInfoMap::const_iterator itOther = itFirst;
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

    //////////////////////////////////////////////////////////////////////////////////////////
    // Group is compatible, if we have MAXGROUPSIZE members then match is found
    if (numPlayers != 5)
    {
        sLogger.debug("({}) Compatibles but not match. Players({})", strGuids, numPlayers);
        uint8_t Tanks_Needed = LFG_TANKS_NEEDED;
        uint8_t Healers_Needed = LFG_HEALERS_NEEDED;
        uint8_t Dps_Needed = LFG_DPS_NEEDED;
        for (LfgRawQueueInfoMap::const_iterator itQueue = pqInfoMap.begin(); itQueue != pqInfoMap.end(); ++itQueue)
        {
            LfgQueueInfo* queue = itQueue->second;
            for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
            {
                uint8_t roles = itPlayer->second;
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
            for (LfgRawQueueInfoMap::const_iterator itQueue = pqInfoMap.begin(); itQueue != pqInfoMap.end(); ++itQueue)
            {
                LfgQueueInfo* queue = itQueue->second;
                if (!queue)
                    continue;

                for (LfgRolesMap::const_iterator itPlayer = queue->roles.begin(); itPlayer != queue->roles.end(); ++itPlayer)
                {
                    WoWGuid wowGuid;
                    wowGuid.Init(itPlayer->first);

                    if (*itPlayers == sObjectMgr.getPlayer(wowGuid.getGuidLowPart()))
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
    sLogger.debug("({}) MATCH! Group formed", strGuids);

    // GROUP FORMED!

    // Select a random dungeon from the compatible list
    // Create a new proposal
    pProposal = std::make_unique<LfgProposal>(Util::selectRandomContainerElement(compatibleDungeons));
    pProposal->cancelTime = time_t(time(NULL)) + LFG_TIME_PROPOSAL;
    pProposal->state = LFG_PROPOSAL_INITIATING;
    pProposal->queues = check;
    pProposal->groupLowGuid = groupLowGuid;

    // Assign new roles to players and assign new leader
    PlayerSet::const_iterator itPlayers = players.begin();
    /*
    if (!leader)
    {
    uint8_t pos = 1;
    for (uint8_t i = 0; i < pos; ++i)
    ++itPlayers;
    leader = (*itPlayers)->GetGUID();
    }
    */
    pProposal->leader = leader;

    uint8_t numAccept = 0;
    for (itPlayers = players.begin(); itPlayers != players.end(); ++itPlayers)
    {
        uint64_t guid = (*itPlayers)->getGuid();
        auto ppPlayer = std::make_unique<LfgProposalPlayer>();
        if (auto grp = (*itPlayers)->getGroup())
        {
            ppPlayer->groupLowGuid = grp->GetID();
            if (grp->isLFGGroup()) // Player from existing group, autoaccept
            {
                ppPlayer->accept = LFG_ANSWER_AGREE;
                ++numAccept;
            }
        }
        ppPlayer->role = rolesMap[guid];
        pProposal->players.try_emplace(guid, std::move(ppPlayer));
    }
    if (numAccept == 5)
        pProposal->state = LFG_PROPOSAL_SUCCESS;

    return true;
}

void LfgMgr::UpdateRoleCheck(uint64_t gguid, uint64_t guid /* = 0 */, uint8_t roles /* = ROLE_NONE */)
{
    if (!gguid)
        return;

    LfgRolesMap check_roles;
    LfgRoleCheckMap::iterator itRoleCheck = m_RoleChecks.find(gguid);
    if (itRoleCheck == m_RoleChecks.end())
        return;

    LfgRoleCheck* roleCheck = itRoleCheck->second.get();
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

    uint8_t team = 0;
    LfgDungeonSet dungeons;
    if (roleCheck->rDungeonId)
        dungeons.insert(roleCheck->rDungeonId);
    else
        dungeons = roleCheck->dungeons;

    LfgJoinResultData joinData = LfgJoinResultData(LFG_JOIN_FAILED, roleCheck->state);
    for (LfgRolesMap::const_iterator it = roleCheck->roles.begin(); it != roleCheck->roles.end(); ++it)
    {
        uint64_t pguid = it->first;
        WoWGuid wowGuid;
        wowGuid.Init(it->first);

        Player* plrg = sObjectMgr.getPlayer(wowGuid.getGuidLowPart());
        if (!plrg)
        {
            if (roleCheck->state == LFG_ROLECHECK_FINISHED)
                SetState(pguid, LFG_STATE_QUEUED);
            else if (roleCheck->state != LFG_ROLECHECK_INITIALITING)
                ClearState(pguid);
            continue;
        }

        team = uint8_t(plrg->getTeam());
        if (!sendRoleChosen)
            plrg->getSession()->sendLfgRoleChosen(guid, roles);
        plrg->getSession()->sendLfgRoleCheckUpdate(roleCheck);
        switch (roleCheck->state)
        {
            case LFG_ROLECHECK_INITIALITING:
                continue;
            case LFG_ROLECHECK_FINISHED:
                SetState(pguid, LFG_STATE_QUEUED);
                plrg->getSession()->sendLfgUpdateParty(LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons, GetComment(pguid)));
                break;
            default:
                if (roleCheck->leader == pguid)
                    plrg->getSession()->sendLfgJoinResult(joinData);
                plrg->getSession()->sendLfgUpdateParty(LfgUpdateData(LFG_UPDATETYPE_ROLECHECK_FAILED));
                ClearState(pguid);
                break;
        }
    }

    if (roleCheck->state == LFG_ROLECHECK_FINISHED)
    {
        SetState(gguid, LFG_STATE_QUEUED);
        auto pqInfo = std::make_unique<LfgQueueInfo>();
        pqInfo->joinTime = time_t(time(NULL));
        pqInfo->roles = roleCheck->roles;
        pqInfo->dungeons = roleCheck->dungeons;

        // Set queue roles needed - As we are using check_roles will not have more that 1 tank, 1 healer, 3 dps
        for (LfgRolesMap::const_iterator it = check_roles.begin(); it != check_roles.end(); ++it)
        {
            uint8_t roles2 = it->second;
            if (roles2 & ROLE_TANK)
                --pqInfo->tanks;
            else if (roles2 & ROLE_HEALER)
                --pqInfo->healers;
            else
                --pqInfo->dps;
        }

        m_QueueInfoMap.try_emplace(gguid, std::move(pqInfo));
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
        m_RoleChecks.erase(itRoleCheck);
    }
}

void LfgMgr::RemoveFromCompatibles(uint64_t guid)
{
    std::stringstream out;
    out << guid;
    std::string strGuid = out.str();

    sLogger.debug("Removing {}", guid);
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
        uint64_t guid = (*it)->getGuid();
        LfgLockMap cachedLockMap = GetLockedDungeons(guid);
        for (LfgLockMap::const_iterator it2 = cachedLockMap.begin(); it2 != cachedLockMap.end() && !dungeons.empty(); ++it2)
        {
            uint32_t dungeonId = (it2->first & 0x00FFFFFF); // Compare dungeon ids
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

    uint8_t damage = 0;
    uint8_t tank = 0;
    uint8_t healer = 0;

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
    return (tank + healer + damage) == uint8_t(groles.size());
}

void LfgMgr::UpdateProposal(uint32_t proposalId, uint64_t guid, bool accept)
{
    // Check if the proposal exists
    LfgProposalMap::iterator itProposal = m_Proposals.find(proposalId);
    if (itProposal == m_Proposals.end())
        return;
    LfgProposal* pProposal = itProposal->second.get();

    // Check if proposal have the current player
    LfgProposalPlayerMap::iterator itProposalPlayer = pProposal->players.find(guid);
    if (itProposalPlayer == pProposal->players.end())
        return;
    LfgProposalPlayer* ppPlayer = itProposalPlayer->second.get();

    ppPlayer->accept = LfgAnswer(accept);
    sLogger.debug("Player {} of proposal {} selected: {}", guid, proposalId, accept);
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
        WoWGuid wowGuid;
        wowGuid.Init(itPlayers->first);

        if (Player* player = sObjectMgr.getPlayer(wowGuid.getGuidLowPart()))
        {
            if (itPlayers->first == pProposal->leader)
                players.push_front(player);
            else
                players.push_back(player);

            // Only teleport new players
            auto grp = player->getGroup();
            uint64_t gguid = grp ? grp->GetID() : 0;
            if (!gguid || !grp->isLFGGroup() || GetState(gguid) == LFG_STATE_FINISHED_DUNGEON)
                playersToTeleport.push_back(player);
        }

        if (itPlayers->second->accept != LFG_ANSWER_AGREE)   // No answer (-1) or not accepted (0)
            allAnswered = false;
    }

    if (!allAnswered)
    {
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
            (*it)->getSession()->sendLfgUpdateProposal(proposalId, pProposal);
    }
    else
    {
        bool sendUpdate = pProposal->state != LFG_PROPOSAL_SUCCESS;
        pProposal->state = LFG_PROPOSAL_SUCCESS;
        time_t joinTime = time_t(time(NULL));
        std::map<uint64_t, int32_t> waitTimesMap;
        // Save wait times before redoing groups
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            LfgProposalPlayer* player = pProposal->players[(*it)->getGuid()].get();

            WoWGuid wowGuid;
            if ((*it)->getGroup())
                wowGuid.Init((*it)->getGroup()->GetGUID());
            else
                wowGuid.Init(uint64_t(0));

            uint32_t lowgroupguid = (*it)->getGroup() ? wowGuid.getGuidLowPart() : 0;
            if (player->groupLowGuid != lowgroupguid)
                sLogger.debug("{} group mismatch: actual ({}) - queued ({})", (*it)->getGuid(), lowgroupguid, player->groupLowGuid);

            uint64_t guid2 = player->groupLowGuid ? WoWGuid(player->groupLowGuid, 0, HIGHGUID_TYPE_GROUP).getRawGuid() : (*it)->getGuid();
            LfgQueueInfoMap::iterator itQueue = m_QueueInfoMap.find(guid2);
            if (itQueue == m_QueueInfoMap.end())
            {
                sLogger.debug("Queue info for guid {} not found!", guid);
                waitTimesMap[(*it)->getGuid()] = -1;
            }
            else
                waitTimesMap[(*it)->getGuid()] = int32_t(joinTime - itQueue->second->joinTime);
        }

        // Create a new group (if needed)
        LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_GROUP_FOUND);
        auto grp = pProposal->groupLowGuid ? sObjectMgr.getGroupById(pProposal->groupLowGuid) : NULL;
        for (LfgPlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = (*it);
            uint64_t pguid = player->getGuid();
            auto group = player->getGroup();
            if (sendUpdate)
                player->getSession()->sendLfgUpdateProposal(proposalId, pProposal);
            if (group)
            {
                player->getSession()->sendLfgUpdateParty(updateData);
                if (group != grp)
                    player->getGroup()->Disband();
            }
            else
                player->getSession()->sendLfgUpdatePlayer(updateData);

            if (!grp)
            {
                // todo: where and when is lfg group deleted? -Appled
                grp = sObjectMgr.createGroup();
                grp->m_disbandOnNoMembers = false;
                grp->ExpandToLFG();

#if VERSION_STRING < Cata
                WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(pProposal->dungeonId);
                SetDungeon(grp->GetID(), dungeon->Entry());
#endif

                // uint32_t low_gguid = grp->GetID();
                uint64_t gguid = grp->GetID();
                SetState(gguid, LFG_STATE_PROPOSAL);
                grp->AddMember(player->getPlayerInfo());
                sLogger.debug("Add Player In Group {}", player->getName());
            }
            else if (group != grp)
            {
                grp->AddMember(player->getPlayerInfo());
                sLogger.debug("Add Player In Group {}", player->getName());
            }

            // Update timers
            uint8_t role = GetRoles(pguid);
            role &= ~ROLE_LEADER;
            switch (role)
            {
                case ROLE_DAMAGE:
                {
                    uint32_t old_number = m_NumWaitTimeDps++;
                    m_WaitTimeDps = int32_t((m_WaitTimeDps * old_number + waitTimesMap[player->getGuid()]) / m_NumWaitTimeDps);
                    break;
                }
                case ROLE_HEALER:
                {
                    uint32_t old_number = m_NumWaitTimeHealer++;
                    m_WaitTimeHealer = int32_t((m_WaitTimeHealer * old_number + waitTimesMap[player->getGuid()]) / m_NumWaitTimeHealer);
                    break;
                }
                case ROLE_TANK:
                {
                    uint32_t old_number = m_NumWaitTimeTank++;
                    m_WaitTimeTank = int32_t((m_WaitTimeTank * old_number + waitTimesMap[player->getGuid()]) / m_NumWaitTimeTank);
                    break;
                }
                default:
                {
                    uint32_t old_number = m_NumWaitTimeAvg++;
                    m_WaitTimeAvg = int32_t((m_WaitTimeAvg * old_number + waitTimesMap[player->getGuid()]) / m_NumWaitTimeAvg);
                    break;
                }
            }

            m_teleport.push_back(pguid);
            if(Player* plr = sObjectMgr.getPlayer(static_cast<uint32_t>(pguid)))
                plr->setRoles(pProposal->players[pguid]->role);
            SetState(pguid, LFG_STATE_DUNGEON);
        }

#if VERSION_STRING < Cata
        WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(pProposal->dungeonId);
#endif
        //Set Dungeon difficult incomplete :D

        if (grp == nullptr) // something went definitely wrong if we end up here... I'm sure it is just bad code design.
            return;

        uint64_t gguid = grp->GetID();
#if VERSION_STRING < Cata
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
        grp->Update();

        m_Proposals.erase(itProposal);
    }
}

void LfgMgr::RemoveProposal(LfgProposalMap::iterator itProposal, LfgUpdateType type)
{
    LfgProposal* pProposal = itProposal->second.get();
    pProposal->state = LFG_PROPOSAL_FAILED;

    sLogger.debug("Proposal {}, state FAILED, UpdateType {}", itProposal->first, type);
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

        uint64_t guid = it->second->groupLowGuid ? WoWGuid(it->second->groupLowGuid, 0, HIGHGUID_TYPE_GROUP).getRawGuid() : it->first;
        // Player didn't accept or still pending when no secs left
        if (it->second->accept == LFG_ANSWER_DENY || type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        {
            it->second->accept = LFG_ANSWER_DENY;
            toRemove.insert(guid);
        }
    }

    uint8_t team = 0;
    // Notify players
    for (LfgProposalPlayerMap::const_iterator it = pProposal->players.begin(); it != pProposal->players.end(); ++it)
    {
        WoWGuid wowGuid;
        wowGuid.Init(it->first);

        Player* player = sObjectMgr.getPlayer(wowGuid.getGuidLowPart());
        if (!player)
            continue;

        team = uint8_t(player->getTeam());
        player->getSession()->sendLfgUpdateProposal(itProposal->first, pProposal);

        auto grp = player->getGroup();
        uint64_t guid = player->getGuid();
        uint64_t gguid = it->second->groupLowGuid ? WoWGuid(it->second->groupLowGuid, 0, HIGHGUID_TYPE_GROUP).getRawGuid() : guid;

        if (toRemove.find(gguid) != toRemove.end())         // Didn't accept or in same group that someone that didn't accept
        {
            LfgUpdateData updateData;
            if (it->second->accept == LFG_ANSWER_DENY)
            {
                updateData.updateType = type;
                sLogger.debug("{} didn't accept. Removing from queue and compatible cache", guid);
            }
            else
            {
                updateData.updateType = LFG_UPDATETYPE_REMOVED_FROM_QUEUE;
                sLogger.debug("{} in same group that someone that didn't accept. Removing from queue and compatible cache", guid);
            }
            ClearState(guid);
            if (grp)
            {
                RestoreState(gguid);
                player->getSession()->sendLfgUpdateParty(updateData);
            }
            else
                player->getSession()->sendLfgUpdatePlayer(updateData);
        }
        else
        {
            sLogger.debug("Readding {} to queue.", guid);
            SetState(guid, LFG_STATE_QUEUED);
            if (grp)
            {
                SetState(gguid, LFG_STATE_QUEUED);
                player->getSession()->sendLfgUpdateParty(LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid), GetComment(guid)));
            }
            else
                player->getSession()->sendLfgUpdatePlayer(LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid), GetComment(guid)));
        }
    }

    // Remove players/groups from queue
    for (LfgGuidSet::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        uint64_t guid = *it;
        RemoveFromQueue(guid);
        pProposal->queues.remove(guid);
    }

    // Readd to queue
    for (LfgGuidList::const_iterator it = pProposal->queues.begin(); it != pProposal->queues.end(); ++it)
    {
        uint64_t guid = *it;
        LfgGuidList& currentQueue = m_currentQueue[team];
        currentQueue.push_front(guid);         //Add GUID for high priority
        AddToQueue(guid, team);                //We have to add each GUID in newQueue to check for a new groups
    }

    m_Proposals.erase(itProposal);
}

void LfgMgr::InitBoot(Group* grp, uint64_t kicker, uint64_t victim, std::string reason)
{
    if (!grp)
        return;
    uint64_t gguid = grp->GetID();
    SetState(gguid, LFG_STATE_BOOT);

    auto pBoot = std::make_unique<LfgPlayerBoot>();
    pBoot->inProgress = true;
    pBoot->cancelTime = time_t(time(NULL)) + LFG_TIME_BOOT;
    pBoot->reason = reason;
    pBoot->victim = victim;
    pBoot->votedNeeded = GetVotesNeeded(gguid);
    PlayerSet players;

    // Set votes
    for (const auto itx : grp->GetSubGroup(0)->getGroupMembers())
    {
        if (Player* plrg = sObjectMgr.getPlayer(itx->guid))
        {
            uint64_t guid = plrg->getGuid();
            SetState(guid, LFG_STATE_BOOT);

            if (guid == victim)
            {
                pBoot->votes[victim] = LFG_ANSWER_DENY;    // Victim auto vote NO
            }
            else if (guid == kicker)
            {
                pBoot->votes[kicker] = LFG_ANSWER_AGREE;   // Kicker auto vote YES
            }
            else
            {
                pBoot->votes[guid] = LFG_ANSWER_PENDING;   // Other members need to vote
                players.insert(plrg);
            }
        }
    }

    // Notify players
    for (PlayerSet::const_iterator it = players.begin(); it != players.end(); ++it)
        (*it)->getSession()->sendLfgBootPlayer(pBoot.get());

    m_Boots.try_emplace(grp->GetID(), std::move(pBoot));
}

void LfgMgr::UpdateBoot(Player* player, bool accept)
{
    if (!player)
        return;

    auto grp = player->getGroup();
    if (!grp)
        return;

    uint32_t bootId = grp->GetID();
    uint64_t guid = player->getGuid();

    LfgPlayerBootMap::iterator itBoot = m_Boots.find(bootId);
    if (itBoot == m_Boots.end())
        return;

    LfgPlayerBoot* pBoot = itBoot->second.get();
    if (!pBoot)
        return;

    if (pBoot->votes[guid] != LFG_ANSWER_PENDING)          // Cheat check: Player can't vote twice
        return;

    pBoot->votes[guid] = LfgAnswer(accept);

    uint8_t votesNum = 0;
    uint8_t agreeNum = 0;
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

    if (agreeNum == pBoot->votedNeeded ||                                   // Vote passed
        votesNum == pBoot->votes.size() ||                                  // All voted but not passed
        (pBoot->votes.size() - votesNum + agreeNum) < pBoot->votedNeeded)   // Vote didnt passed
    {
        // Send update info to all players
        pBoot->inProgress = false;
        for (LfgAnswerMap::const_iterator itVotes = pBoot->votes.begin(); itVotes != pBoot->votes.end(); ++itVotes)
        {
            uint64_t pguid = itVotes->first;

            WoWGuid wowGuid;
            wowGuid.Init(itVotes->first);

            if (pguid != pBoot->victim)
            {
                SetState(pguid, LFG_STATE_DUNGEON);
                if (Player* plrg = sObjectMgr.getPlayer(wowGuid.getGuidLowPart()))
                {
                    plrg->getSession()->sendLfgBootPlayer(pBoot);
                }
            }
        }

        uint64_t gguid = grp->GetGUID();
        SetState(gguid, LFG_STATE_DUNGEON);
        if (agreeNum == pBoot->votedNeeded)                // Vote passed - Kick player
        {
            /* incomplete :D
            Player::RemoveFromGroup(grp, pBoot->victim);
            if (Player* victim = sObjectMgr.GetPlayer(GET_LOWGUID_PART(pBoot->victim)))
            {
            TeleportPlayer(victim, true, false);
            setState(pBoot->victim, LFG_STATE_NONE);
            }
            OfferContinue(grp);
            DecreaseKicksLeft(gguid);
            */
        }
        m_Boots.erase(itBoot);
    }
}

void LfgMgr::TeleportPlayer(Player* player, bool out, bool fromOpcode /*= false*/)
{
    sLogger.debug("{} is being teleported {}", player->getGuid(), out ? "out" : "in");
    if (out)
    {
        player->removeAllAurasById(LFG_SPELL_LUCK_OF_THE_DRAW);
        player->safeTeleport(player->getBGEntryMapId(), player->getBGEntryInstanceId(), player->getBGEntryPosition());
        return;
    }

    // TODO Add support for LFG_TELEPORTERROR_FATIGUE
    LfgTeleportError error = LFG_TELEPORTERROR_OK;
    auto grp = player->getGroup();

    if (!grp || !grp->isLFGGroup())                        // should never happen, but just in case...
        error = LFG_TELEPORTERROR_INVALID_LOCATION;
    else if (!player->isAlive())
        error = LFG_TELEPORTERROR_PLAYER_DEAD;
    else
    {
#if VERSION_STRING < Cata
        uint64_t gguid = grp->GetID();
        WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(GetDungeon(gguid));

        if (!dungeon)
            error = LFG_TELEPORTERROR_INVALID_LOCATION;
        else if (player->GetMapId() != uint32_t(dungeon->map))  // Do not teleport players in dungeon to the entrance
        {
            uint32_t mapid = 0;
            LocationVector location = { 0, 0, 0, 0 };

            if (!fromOpcode)
            {
                // Select a player inside to be teleported to
                for (const auto itx : grp->GetSubGroup(0)->getGroupMembers())
                {
                    Player* plrg = sObjectMgr.getPlayer(itx->guid);
                    if (plrg && plrg != player && plrg->GetMapId() == uint32_t(dungeon->map))
                    {
                        mapid = plrg->GetMapId();
                        location.x = plrg->GetPositionX();
                        location.y = plrg->GetPositionY();
                        location.z = plrg->GetPositionZ();
                        location.o = plrg->GetOrientation();
                    }
                }
            }

            if (!mapid)
            {
                MySQLStructure::AreaTrigger const* areaTrigger = sMySQLStore.getMapEntranceTrigger(dungeon->map);
                if (areaTrigger == nullptr)
                {
                    sLogger.debug("Failed to teleport {}: No areatrigger found for map: {} difficulty: {}", player->getGuid(), dungeon->map, dungeon->difficulty);
                    error = LFG_TELEPORTERROR_INVALID_LOCATION;
                }
                else
                {
                    mapid = areaTrigger->mapId;
                    location.x = areaTrigger->x;
                    location.y = areaTrigger->y;
                    location.z = areaTrigger->z;
                    location.o = areaTrigger->o;
                }
            }

            if (error == LFG_TELEPORTERROR_OK)
            {
                player->setBGEntryPoint(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), player->GetMapId(), player->GetInstanceID());
                player->dismount();
                if (!player->safeTeleport(mapid, 0, location))
                    error = LFG_TELEPORTERROR_INVALID_LOCATION;
            }
        }
#endif
    }

    if (error != LFG_TELEPORTERROR_OK)
        player->getSession()->sendLfgTeleportError(uint8_t(error));

    if (fromOpcode) { return;  }
}

void LfgMgr::RewardDungeonDoneFor(const uint32_t dungeonId, Player* player)
{
#if VERSION_STRING < Cata
    auto group = player->getGroup();
    if (!group || !group->isLFGGroup())
    {
        sLogger.debug("{} is not in a group or not a LFGGroup. Ignoring", player->getGuid());
        return;
    }

    uint64_t guid = player->getGuid();
    uint64_t gguid = player->getGroup()->GetID();
    uint32_t gDungeonId = GetDungeon(gguid, true);
    if (gDungeonId != dungeonId)
    {
        sLogger.debug("{} Finished dungeon {} but group queued for {}. Ignoring", guid, dungeonId, gDungeonId);
        return;
    }

    if (GetState(guid) == LFG_STATE_FINISHED_DUNGEON)
    {
        sLogger.debug("{} Already rewarded player. Ignoring", guid);
        return;
    }

    // Mark dungeon as finished
    SetState(gguid, LFG_STATE_FINISHED_DUNGEON);

    // Clear player related lfg stuff
    uint32_t rDungeonId = (*GetSelectedDungeons(guid).begin());
    ClearState(guid);
    SetState(guid, LFG_STATE_FINISHED_DUNGEON);

    
    // Give rewards only if its a random or seasonal dungeon
    WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(rDungeonId);
    if (!dungeon || (dungeon->type != LFG_TYPE_RANDOM /*add seasonal checks*/))
    {
        sLogger.debug("{} dungeon {} is not random nor seasonal", guid, rDungeonId);
        return;
    }

#if VERSION_STRING > TBC
    // Update achievements
    if (dungeon->difficulty == 1) // Heroic
        player->getAchievementMgr()->updateAchievementCriteria(player, 13029, 1); // Done LFG Dungeon with random Players
#endif

    LfgReward const* reward = GetRandomDungeonReward(rDungeonId, static_cast<uint8_t>(player->getLevel()));
    if (!reward)
        return;

    uint8_t index = 0;
    QuestProperties const* qReward = sMySQLStore.getQuestProperties(reward->reward[index].questId);
    if (!qReward)
        return;

    // if we can take the quest, means that we haven't done this kind of "run", IE: First Heroic Random of Day.
    if (!player->hasQuestInFinishedDailies(qReward->id) || !player->hasQuestFinished(qReward->id))
    {
        sQuestMgr.BuildQuestComplete(player, qReward);
        player->addQuestToFinished(qReward->id);

        // Reputation reward
        for (uint8_t z = 0; z < 6; z++)
        {
            if (qReward->reward_repfaction[z])
            {
                int32_t amt = 0;
                uint32_t fact = qReward->reward_repfaction[z];
                if (qReward->reward_repvalue[z])
                {
                    amt = qReward->reward_repvalue[z];
                }
                if (qReward->reward_replimit && (player->getFactionStanding(fact) >= (int32_t)qReward->reward_replimit))
                {
                    continue;
                }
                amt = Util::float2int32(amt * worldConfig.getFloatRate(RATE_QUESTREPUTATION));
                player->modFactionStanding(fact, amt);
            }
        }
        // Static Item reward
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (qReward->reward_item[i])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qReward->reward_item[i]);
                if (!proto)
                {
                    sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qReward->reward_item[i], qReward->id);
                }
                else
                {
                    auto item_add = player->getItemInterface()->FindItemLessMax(qReward->reward_item[i], qReward->reward_itemcount[i], false);
                    if (!item_add)
                    {
                        auto slotresult = player->getItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            player->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = sObjectMgr.createItem(qReward->reward_item[i], player);
                            if (item)
                            {
                                item->setStackCount(uint32_t(qReward->reward_itemcount[i]));
                                // TODO: if add fails, should item be sent in mail? now it's destroyed
                                player->getItemInterface()->SafeAddItem(std::move(item), slotresult.ContainerSlot, slotresult.Slot);
                            }
                        }
                    }
                    else
                    {
                        item_add->setStackCount(item_add->getStackCount() + qReward->reward_itemcount[i]);
                        item_add->m_isDirty = true;
                    }
                }
            }
        }

        // if daily then append to finished dailies
        if (qReward->is_repeatable == DEFINE_QUEST_REPEATABLE_DAILY)
            player->addQuestIdToFinishedDailies(qReward->id);
#if VERSION_STRING > TBC
        player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT, 1, 0, 0);
#endif
        if (qReward->reward_money > 0)
        {
            // Money reward
            // Check they don't have more than the max gold
            if (worldConfig.player.isGoldCapEnabled && (player->getCoinage() + qReward->reward_money) <= worldConfig.player.limitGoldAmount)
            {
                player->modCoinage(qReward->reward_money);
            }
#if VERSION_STRING > TBC
            player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD, qReward->reward_money, 0, 0);
#endif
        }
#if VERSION_STRING > TBC
        player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE, qReward->zone_id, 0, 0);
        player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, qReward->id, 0, 0);
#endif
    }
    else
    {
        index = 1;
        qReward = sMySQLStore.getQuestProperties(reward->reward[index].questId);
        if (!qReward)
            return;

        sQuestMgr.BuildQuestComplete(player, qReward);
        player->addQuestToFinished(qReward->id);

        // Reputation reward
        for (uint8_t z = 0; z < 6; z++)
        {
            if (qReward->reward_repfaction[z])
            {
                int32_t amt = 0;
                uint32_t fact = qReward->reward_repfaction[z];
                if (qReward->reward_repvalue[z])
                {
                    amt = qReward->reward_repvalue[z];
                }
                if (qReward->reward_replimit && (player->getFactionStanding(fact) >= (int32_t)qReward->reward_replimit))
                {
                    continue;
                }
                amt = Util::float2int32(amt * worldConfig.getFloatRate(RATE_QUESTREPUTATION));
                player->modFactionStanding(fact, amt);
            }
        }
        // Static Item reward
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (qReward->reward_item[i])
            {
                ItemProperties const* proto = sMySQLStore.getItemProperties(qReward->reward_item[i]);
                if (!proto)
                {
                    sLogger.failure("Invalid item prototype in quest reward! ID {}, quest {}", qReward->reward_item[i], qReward->id);
                }
                else
                {
                    auto item_add = player->getItemInterface()->FindItemLessMax(qReward->reward_item[i], qReward->reward_itemcount[i], false);
                    if (!item_add)
                    {
                        auto slotresult = player->getItemInterface()->FindFreeInventorySlot(proto);
                        if (!slotresult.Result)
                        {
                            player->getItemInterface()->buildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                        }
                        else
                        {
                            auto item = sObjectMgr.createItem(qReward->reward_item[i], player);
                            if (item)
                            {
                                item->setStackCount(uint32_t(qReward->reward_itemcount[i]));
                                // TODO: if add fails, should item be sent in mail? now it's destroyed
                                player->getItemInterface()->SafeAddItem(std::move(item), slotresult.ContainerSlot, slotresult.Slot);
                            }
                        }
                    }
                    else
                    {
                        item_add->setStackCount(item_add->getStackCount() + qReward->reward_itemcount[i]);
                        item_add->m_isDirty = true;
                    }
                }
            }
        }

        // if daily then append to finished dailies
        if (qReward->is_repeatable == DEFINE_QUEST_REPEATABLE_DAILY)
            player->addQuestIdToFinishedDailies(qReward->id);

#if VERSION_STRING > TBC
        player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT, 1, 0, 0);
#endif
        if (qReward->reward_money > 0)
        {
            // Money reward
            // Check they don't have more than the max gold
            if (worldConfig.player.isGoldCapEnabled && (player->getCoinage() + qReward->reward_money) <= worldConfig.player.limitGoldAmount)
            {
                player->modCoinage(qReward->reward_money);
            }
#if VERSION_STRING > TBC
            player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD, qReward->reward_money, 0, 0);
#endif
        }
#if VERSION_STRING > TBC
        player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE, qReward->zone_id, 0, 0);
        player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST, qReward->id, 0, 0);
#endif
    }

    // Give rewards
    //Log.Debug("LfgMgr", "LfgMgr::RewardDungeonDoneFor: %u done dungeon %u, %s previously done.", player->GetGUID(), GetDungeon(gguid), index > 0 ? " " : " not");
    sLogger.debug("{} done dungeon {}, previously done.", player->getGuid(), GetDungeon(gguid));
    player->getSession()->sendLfgPlayerReward(dungeon->Entry(), GetDungeon(gguid, false), index, reward, qReward);
#else
    if (player == nullptr ||dungeonId == 0) { return; }
#endif
}

const LfgDungeonSet& LfgMgr::GetDungeonsByRandom(uint32_t randomdungeon)
{
#if VERSION_STRING < Cata
    WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(randomdungeon);
    uint32_t groupType = dungeon ? dungeon->grouptype : 0;
    return m_CachedDungeonMap[groupType];
#else
    if (randomdungeon == 0) { return m_CachedDungeonMap[0]; }
    return m_CachedDungeonMap[0];
#endif
}

uint32_t LfgMgr::GetLFGDungeon(uint32_t id)
{
    if (WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(id))
        if (dungeon->ID)
            return dungeon->ID;

    return 0;
}

LfgReward const* LfgMgr::GetRandomDungeonReward(uint32_t dungeon, uint8_t level)
{
    sLogger.debug("Get Reward dungeon id = {} level = {}", dungeon, level);
    LfgReward const* rew = NULL;
    LfgRewardMapBounds bounds = m_RewardMap.equal_range(dungeon & 0x00FFFFFF);
    for (LfgRewardMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        rew = itr->second.get();
        // ordered properly at loading
        if (itr->second->maxLevel >= level)
            break;
    }

    return rew;
}

LfgType LfgMgr::GetDungeonType(uint32_t dungeonId)
{
#if VERSION_STRING < Cata
    WDB::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.lookupEntry(dungeonId);
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

LfgState LfgMgr::GetState(uint64_t guid)
{
    sLogger.debug("{}", guid);
    if (HIGHGUID_TYPE_GROUP == guid)
        return m_Groups[guid].GetState();
    else
        return m_Players[guid].GetState();
}

uint32_t LfgMgr::GetDungeon(uint64_t guid, bool asId /*= true*/)
{
    sLogger.debug("{} asId: {}", guid, asId);
    return m_Groups[guid].GetDungeon(asId);
}

uint8_t LfgMgr::GetRoles(uint64_t guid)
{
    sLogger.debug("{}", guid);
    return m_Players[guid].GetRoles();
}

const std::string& LfgMgr::GetComment(uint64_t guid)
{
    sLogger.debug("{}", guid);
    return m_Players[guid].GetComment();
}

bool LfgMgr::IsTeleported(uint64_t pguid)
{
    if (std::find(m_teleport.begin(), m_teleport.end(), pguid) != m_teleport.end())
    {
        m_teleport.remove(pguid);
        return true;
    }
    return false;
}

const LfgDungeonSet& LfgMgr::GetSelectedDungeons(uint64_t guid)
{
    sLogger.debug("{}", guid);
    return m_Players[guid].GetSelectedDungeons();
}

const LfgLockMap& LfgMgr::GetLockedDungeons(uint64_t guid)
{
    sLogger.debug("{}", guid);
    return m_Players[guid].GetLockedDungeons();
}

uint8_t LfgMgr::GetKicksLeft(uint64_t guid)
{
    sLogger.debug("{}", guid);
    return m_Groups[guid].GetKicksLeft();
}

uint8_t LfgMgr::GetVotesNeeded(uint64_t guid)
{
    sLogger.debug("{}", guid);
    return m_Groups[guid].GetVotesNeeded();
}

void LfgMgr::RestoreState(uint64_t guid)
{
    sLogger.debug("{}", guid);
    m_Groups[guid].RestoreState();
}

void LfgMgr::ClearState(uint64_t guid)
{
    sLogger.debug("{}", guid);
    m_Players[guid].ClearState();
}

void LfgMgr::SetState(uint64_t guid, LfgState state)
{
    sLogger.debug("{} state {}", guid, state);

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    if (wowGuid.isGroup())
        m_Groups[guid].SetState(state);
    else
        m_Players[guid].SetState(state);

}

void LfgMgr::SetDungeon(uint64_t guid, uint32_t dungeon)
{
    sLogger.debug("{} dungeon {}", guid, dungeon);
    m_Groups[guid].SetDungeon(dungeon);
}

void LfgMgr::SetRoles(uint64_t guid, uint8_t roles)
{
    sLogger.debug("{} roles: {}", guid, roles);
    m_Players[guid].SetRoles(roles);
}

void LfgMgr::SetComment(uint64_t guid, const std::string& comment)
{
    sLogger.debug("{} comment: {}", guid, comment);
    m_Players[guid].SetComment(comment);
}

void LfgMgr::SetSelectedDungeons(uint64_t guid, const LfgDungeonSet& dungeons)
{
    sLogger.debug("{}", guid);
    m_Players[guid].SetSelectedDungeons(dungeons);
}

void LfgMgr::SetLockedDungeons(uint64_t guid, const LfgLockMap& lock)
{
    sLogger.debug("{}", guid);
    m_Players[guid].SetLockedDungeons(lock);
}

void LfgMgr::DecreaseKicksLeft(uint64_t guid)
{
    sLogger.debug("{}", guid);
    m_Groups[guid].DecreaseKicksLeft();
}

void LfgMgr::RemovePlayerData(uint64_t guid)
{
    sLogger.debug("{}", guid);
    LfgPlayerDataMap::iterator it = m_Players.find(guid);
    if (it != m_Players.end())
        m_Players.erase(it);
}

void LfgMgr::RemoveGroupData(uint64_t guid)
{
    sLogger.debug("{}", guid);
    LfgGroupDataMap::iterator it = m_Groups.find(guid);
    if (it != m_Groups.end())
        m_Groups.erase(it);
}
