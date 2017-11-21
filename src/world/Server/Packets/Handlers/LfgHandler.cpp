/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "Management/LFG/LFGMgr.h"
#include "Common.hpp"
#include "Storage/MySQLDataStore.hpp"

void BuildPlayerLockDungeonBlock(WorldPacket& data, const LfgLockMap& lock)
{
    LogDebugFlag(LF_OPCODE, "BUILD PLAYER LOCK DUNGEON BLOCK");
    data << uint32(lock.size());                           // Size of lock dungeons
    for (LfgLockMap::const_iterator it = lock.begin(); it != lock.end(); ++it)
    {
        data << uint32(it->first);                         // Dungeon entry (id + type)
        data << uint32(it->second);                        // Lock status
    }
}

void BuildPartyLockDungeonBlock(WorldPacket& data, const LfgLockPartyMap& lockMap)
{
    LogDebugFlag(LF_OPCODE, "BUILD PARTY LOCK DUNGEON BLOCK");
    data << uint8(lockMap.size());
    for (LfgLockPartyMap::const_iterator it = lockMap.begin(); it != lockMap.end(); ++it)
    {
        data << uint64(it->first);                         // Player guid
        BuildPlayerLockDungeonBlock(data, it->second);
    }
}


void WorldSession::HandleLfgSetCommentOpcode(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "CMSG_SET_LFG_COMMENT");

    std::string comment;
    recv_data >> comment;
    uint64 guid = GetPlayer()->GetGUID();
    LogDebugFlag(LF_OPCODE, "LfgHandler CMSG_SET_LFG_COMMENT %u, comment: %s", guid, comment.c_str());

    sLfgMgr.SetComment(guid, comment);
}

#if VERSION_STRING > TBC
void WorldSession::HandleLfgJoinOpcode(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_JOIN");

    if ((GetPlayer()->GetGroup() && GetPlayer()->GetGroup()->GetLeader()->guid != GetPlayer()->GetGUID() && (GetPlayer()->GetGroup()->MemberCount() == 5 || !GetPlayer()->GetGroup()->isLFGGroup())))
    {
        LogDebug("HandleLfgJoinOpcode : Unable to JoinQueue");

        recv_data.clear();
        return;
    }

    uint8 numDungeons;
    uint32 dungeon;
    uint32 roles;

    recv_data >> roles;
    recv_data.read<uint16>();                        // uint8 (always 0) - uint8 (always 0)
    recv_data >> numDungeons;

    if (!numDungeons)
    {
        LogDebugFlag(LF_OPCODE, "CMSG_LFG_JOIN no dungeons selected", GetPlayer()->GetGUID());
        recv_data.clear();
        return;
    }

    LfgDungeonSet newDungeons;
    for (int8 i = 0; i < numDungeons; ++i)
    {
        recv_data >> dungeon;
        newDungeons.insert((dungeon & 0x00FFFFFF));       // remove the type from the dungeon entry
    }

    recv_data.read<uint32>();                        // for 0..uint8 (always 3) { uint8 (always 0) }

    std::string comment;
    recv_data >> comment;
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_JOIN: %s, roles: %u, Dungeons: %u, Comment: %s", GetPlayer()->GetName(), roles, uint8(newDungeons.size()), comment.c_str());
    sLfgMgr.Join(GetPlayer(), uint8(roles), newDungeons, comment);
}

void WorldSession::HandleLfgLeaveOpcode(WorldPacket& /*recvData*/)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_LEAVE");

    Group* grp = GetPlayer()->GetGroup();

    LogDebugFlag(LF_OPCODE, "CMSG_LFG_LEAVE %u in group: %u", GetPlayer()->GetGUID(), grp ? 1 : 0);

    // Check cheating - only leader can leave the queue
    if (!grp || grp->GetLeader()->guid == GetPlayer()->GetGUID())
        sLfgMgr.Leave(GetPlayer(), grp);
}

void WorldSession::HandleLfrSearchOpcode(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "CMSG_SEARCH_LFG_JOIN");
    uint32 entry;                                          // Raid id to search
    recv_data >> entry;
    LogDebugFlag(LF_OPCODE, "CMSG_SEARCH_LFG_JOIN %u dungeon entry: %u", GetPlayer()->GetGUID(), entry);
    //SendLfrUpdateListOpcode(entry);
}

void WorldSession::HandleLfrLeaveOpcode(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "CMSG_SEARCH_LFG_LEAVE");
    uint32 dungeonId;                                      // Raid id queue to leave
    recv_data >> dungeonId;
    LogDebugFlag(LF_OPCODE, "CMSG_SEARCH_LFG_LEAVE %u dungeonId: %u", GetPlayer()->GetGUID(), dungeonId);
    //sLFGMgr->LeaveLfr(GetPlayer(), dungeonId);
}

void WorldSession::HandleLfgProposalResultOpcode(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_PROPOSAL_RESULT");
    uint32 lfgGroupID;                                     // Internal lfgGroupID
    bool accept;                                           // Accept to join?
    recv_data >> lfgGroupID;
    recv_data >> accept;

    LogDebugFlag(LF_OPCODE, "CMSG_LFG_PROPOSAL_RESULT %u proposal: %u accept: %u", GetPlayer()->GetGUID(), lfgGroupID, accept ? 1 : 0);
    sLfgMgr.UpdateProposal(lfgGroupID, GetPlayer()->GetGUID(), accept);
}

void WorldSession::HandleLfgSetRolesOpcode(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_SET_ROLES");
    uint8 roles;
    recv_data >> roles;                                    // Player Group Roles
    uint64 guid = GetPlayer()->GetGUID();
    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
    {
        LogDebugFlag(LF_OPCODE, "CMSG_LFG_SET_ROLES %u Not in group", guid);
        return;
    }
    uint64 gguid = grp->GetGUID();// NEED TO ADD Bind group to guids
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_SET_ROLES: Group %u, Player %u, Roles: %u", gguid, guid, roles);
    sLfgMgr.UpdateRoleCheck(gguid, guid, roles);
}

void WorldSession::HandleLfgSetBootVoteOpcode(WorldPacket& recv_data)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_SET_BOOT_VOTE");
    bool agree;                                            // Agree to kick player
    recv_data >> agree;

    LogDebugFlag(LF_OPCODE, "CMSG_LFG_SET_BOOT_VOTE %u agree: %u", GetPlayer()->GetGUID(), agree ? 1 : 0);
    sLfgMgr.UpdateBoot(GetPlayer(), agree);
}

void WorldSession::HandleLfgPlayerLockInfoRequestOpcode(WorldPacket& /*recvData*/)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFD_PLAYER_LOCK_INFO_REQUEST");
    uint64 guid = GetPlayer()->GetGUID();
    LogDebugFlag(LF_OPCODE, "CMSG_LFD_PLAYER_LOCK_INFO_REQUEST %u", guid);

    // Get Random dungeons that can be done at a certain level and expansion
    // FIXME - Should return seasonals (when not disabled)
    LfgDungeonSet randomDungeons;
    uint8 level = static_cast<uint8>(GetPlayer()->getLevel());

#if VERSION_STRING != Cata
    uint8 expansion = static_cast<uint8>(GetPlayer()->GetSession()->GetFlags());
	for (uint32 i = 0; i < sLFGDungeonStore.GetNumRows(); ++i)
    {
        DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(i);
        if (dungeon && dungeon->type == LFG_TYPE_RANDOM && dungeon->expansion <= expansion && dungeon->minlevel <= level && level <= dungeon->maxlevel)
            randomDungeons.insert(dungeon->Entry());
 
    }
#endif

    // Get player locked Dungeons
    LfgLockMap lock = sLfgMgr.GetLockedDungeons(guid);
    uint32 rsize = uint32(randomDungeons.size());
    uint32 lsize = uint32(lock.size());

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_PLAYER_INFO %u", guid);
    WorldPacket data(SMSG_LFG_PLAYER_INFO, 1 + rsize * (4 + 1 + 4 + 4 + 4 + 4 + 1 + 4 + 4 + 4) + 4 + lsize * (1 + 4 + 4 + 4 + 4 + 1 + 4 + 4 + 4));

    data << uint8(randomDungeons.size());                  // Random Dungeon count
    for (LfgDungeonSet::const_iterator it = randomDungeons.begin(); it != randomDungeons.end(); ++it)
    {
        data << uint32(*it);                               // Dungeon Entry (id + type)
        LfgReward const* reward = sLfgMgr.GetRandomDungeonReward(*it, level);
        QuestProperties const* qRew = nullptr;
        uint8 done = 0;
        if (reward)
        {
            qRew = sMySQLStore.getQuestProperties(reward->reward[0].questId);
            if (qRew)
            {
                done = GetPlayer()->HasFinishedQuest(qRew->id);
                if (done)
                    qRew = sMySQLStore.getQuestProperties(reward->reward[1].questId);
            }
        }
        if (qRew)
        {
            data << uint8(done);
            data << uint32(qRew->reward_money);
            data << uint32(qRew->reward_xp);
            data << uint32(reward->reward[done].variableMoney);
            data << uint32(reward->reward[done].variableXP);
            //\todo FIXME Linux: error: cast from const uint32* {aka const unsigned int*} to uint8 {aka unsigned char} loses precision 
            // can someone check this now ?
            data << uint8(qRew->GetRewardItemCount());
            for (uint8 i = 0; i < 4; ++i)
                if (qRew->reward_item[i] != 0)
                {
                    ItemProperties const* item = sMySQLStore.getItemProperties(qRew->reward_item[i]);
                    data << uint32(qRew->reward_item[i]);
                    data << uint32(item ? item->DisplayInfoID : 0);
                    data << uint32(qRew->reward_itemcount[i]);
                }
        }
        else
        {
            data << uint8(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0);
            data << uint8(0);
        }
    }
    BuildPlayerLockDungeonBlock(data, lock);
    SendPacket(&data);
}

void WorldSession::HandleLfgTeleportOpcode(WorldPacket& recvData)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_TELEPORT");
    bool out;
    recvData >> out;

    LogDebugFlag(LF_OPCODE, "CMSG_LFG_TELEPORT %u out: %u", GetPlayer()->GetGUID(), out ? 1 : 0);
    sLfgMgr.TeleportPlayer(GetPlayer(), out, true);
}

void WorldSession::HandleLfgPartyLockInfoRequestOpcode(WorldPacket& /*recvData*/)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFD_PARTY_LOCK_INFO_REQUEST");

    uint64 guid = GetPlayer()->GetGUID();
    LogDebugFlag(LF_OPCODE, "CMSG_LFD_PARTY_LOCK_INFO_REQUEST %u", guid);

    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
        return;

    // Get the locked dungeons of the other party members
    LfgLockPartyMap lockMap;
    GroupMembersSet::iterator itx;
    for (itx = grp->GetSubGroup(0)->GetGroupMembersBegin(); itx != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
    {
        Player* plrg = (*itx)->m_loggedInPlayer;
        if (!plrg)
            continue;

        uint64 pguid = plrg->GetGUID();
        if (pguid == guid)
            continue;

        lockMap[pguid] = sLfgMgr.GetLockedDungeons(pguid);
    }

    uint32 size = 0;
    for (LfgLockPartyMap::const_iterator it = lockMap.begin(); it != lockMap.end(); ++it)
        size += 8 + 4 + uint32(it->second.size()) * (4 + 4);

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_PARTY_INFO %u", guid);
    WorldPacket data(SMSG_LFG_PARTY_INFO, 1 + size);
    BuildPartyLockDungeonBlock(data, lockMap);
    SendPacket(&data);
}
#endif

void WorldSession::SendLfgJoinResult(const LfgJoinResultData& joinData)
{
#if VERSION_STRING > TBC
    uint32 size = 0;
    for (LfgLockPartyMap::const_iterator it = joinData.lockmap.begin(); it != joinData.lockmap.end(); ++it)
        size += 8 + 4 + uint32(it->second.size()) * (4 + 4);

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_JOIN_RESULT %u heckResult: %u checkValue: %u", GetPlayer()->GetGUID(), joinData.result, joinData.state);

    WorldPacket data(SMSG_LFG_JOIN_RESULT, 4 + 4 + size);

    data << uint32(joinData.result);        // Check Result
    data << uint32(joinData.state);         // Check Value
    if (!joinData.lockmap.empty())
        BuildPartyLockDungeonBlock(data, joinData.lockmap);
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgUpdatePlayer(const LfgUpdateData& updateData)
{
#if VERSION_STRING > TBC
    bool queued = false;
    bool extrainfo = false;

    switch (updateData.updateType)
    {
        case LFG_UPDATETYPE_JOIN_PROPOSAL:
        case LFG_UPDATETYPE_ADDED_TO_QUEUE:
            queued = true;
            extrainfo = true;
            break;
            //case LFG_UPDATETYPE_CLEAR_LOCK_LIST: // TODO: Sometimes has extrainfo - Check ocurrences...
        case LFG_UPDATETYPE_PROPOSAL_BEGIN:
            extrainfo = true;
            break;
        default:
            break;
    }

    uint64 guid = GetPlayer()->GetGUID();
    uint8 size = uint8(updateData.dungeons.size());

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_UPDATE_PLAYER %u updatetype: %u", guid, updateData.updateType);

    WorldPacket data(SMSG_LFG_UPDATE_PLAYER, 1 + 1 + (extrainfo ? 1 : 0) * (1 + 1 + 1 + 1 + size * 4 + updateData.comment.length()));

    data << uint8(updateData.updateType);       // Lfg Update type
    data << uint8(extrainfo);                   // Extra info
    if (extrainfo)
    {
        data << uint8(queued);                  // Join the queue
        data << uint8(0);                       // unk - Always 0
        data << uint8(0);                       // unk - Always 0
        data << uint8(size);
        if (size)
            for (LfgDungeonSet::const_iterator it = updateData.dungeons.begin(); it != updateData.dungeons.end(); ++it)
                data << uint32(*it);
        data << updateData.comment;
    }
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgUpdateParty(const LfgUpdateData& updateData)
{
#if VERSION_STRING > TBC
    bool join = false;
    bool extrainfo = false;
    bool queued = false;

    switch (updateData.updateType)
    {
        case LFG_UPDATETYPE_JOIN_PROPOSAL:
            extrainfo = true;
            break;
        case LFG_UPDATETYPE_ADDED_TO_QUEUE:
            extrainfo = true;
            join = true;
            queued = true;
            break;
        case LFG_UPDATETYPE_CLEAR_LOCK_LIST:
            // join = true;  // TODO: Sometimes queued and extrainfo - Check ocurrences...
            queued = true;
            break;
        case LFG_UPDATETYPE_PROPOSAL_BEGIN:
            extrainfo = true;
            join = true;
            break;
        default:
            break;
    }

    uint64 guid = GetPlayer()->GetGUID();
    uint8 size = uint8(updateData.dungeons.size());

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_UPDATE_PARTY %u updatetype: %u", guid, updateData.updateType);

    WorldPacket data(SMSG_LFG_UPDATE_PARTY, 1 + 1 + (extrainfo ? 1 : 0) * (1 + 1 + 1 + 1 + 1 + size * 4 + updateData.comment.length()));

    data << uint8(updateData.updateType);                 // Lfg Update type
    data << uint8(extrainfo);                             // Extra info
    if (extrainfo)
    {
        data << uint8(join);                              // LFG Join
        data << uint8(queued);                            // Join the queue
        data << uint8(0);                                 // unk - Always 0
        data << uint8(0);                                 // unk - Always 0
        for (uint8 i = 0; i < 3; ++i)
            data << uint8(0);                             // unk - Always 0

        data << uint8(size);
        if (size)
            for (LfgDungeonSet::const_iterator it = updateData.dungeons.begin(); it != updateData.dungeons.end(); ++it)
                data << uint32(*it);
        data << updateData.comment;
    }
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgRoleChosen(uint64 guid, uint8 roles)
{
#if VERSION_STRING > TBC
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_ROLE_CHOSEN %u guid: %u roles: %u", GetPlayer()->GetGUID(), guid, roles);

    WorldPacket data(SMSG_LFG_ROLE_CHOSEN, 8 + 1 + 4);

    data << uint64(guid);                                  // Guid
    data << uint8(roles > 0);                              // Ready
    data << uint32(roles);                                 // Roles
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgRoleCheckUpdate(const LfgRoleCheck* pRoleCheck)
{
#if VERSION_STRING > TBC
    ASSERT(pRoleCheck);
    LfgDungeonSet dungeons;
    if (pRoleCheck->rDungeonId)
        dungeons.insert(pRoleCheck->rDungeonId);
    else
        dungeons = pRoleCheck->dungeons;

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_ROLE_CHECK_UPDATE %u", GetPlayer()->GetGUID());

    WorldPacket data(SMSG_LFG_ROLE_CHECK_UPDATE, 4 + 1 + 1 + dungeons.size() * 4 + 1 + pRoleCheck->roles.size() * (8 + 1 + 4 + 1));

    data << uint32(pRoleCheck->state);                     // Check result
    data << uint8(pRoleCheck->state == LFG_ROLECHECK_INITIALITING);
    data << uint8(dungeons.size());                        // Number of dungeons
#if VERSION_STRING != Cata
    if (!dungeons.empty())
    {
        for (LfgDungeonSet::iterator it = dungeons.begin(); it != dungeons.end(); ++it)
        {
            DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(*it);
            data << uint32(dungeon ? dungeon->Entry() : 0); // Dungeon
        }
    }
#endif

    data << uint8(pRoleCheck->roles.size());               // Players in group
    if (!pRoleCheck->roles.empty())
    {
        // Leader info MUST be sent 1st :S
        uint64 guid = pRoleCheck->leader;
        uint8 roles = pRoleCheck->roles.find(guid)->second;
        data << uint64(guid);                              // Guid
        data << uint8(roles > 0);                          // Ready
        data << uint32(roles);                             // Roles

        Player* player = objmgr.GetPlayer(GET_LOWGUID_PART(guid));
        data << uint8(player ? player->getLevel() : 0);    // Level

        for (LfgRolesMap::const_iterator it = pRoleCheck->roles.begin(); it != pRoleCheck->roles.end(); ++it)
        {
            if (it->first == pRoleCheck->leader)
                continue;

            guid = it->first;
            roles = it->second;
            data << uint64(guid);                          // Guid
            data << uint8(roles > 0);                      // Ready
            data << uint32(roles);                         // Roles

            player = objmgr.GetPlayer(GET_LOWGUID_PART(guid));
            data << uint8(player ? player->getLevel() : 0);     // Level
        }
    }
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgQueueStatus(uint32 dungeon, int32 waitTime, int32 avgWaitTime, int32 waitTimeTanks, int32 waitTimeHealer, int32 waitTimeDps, uint32 queuedTime, uint8 tanks, uint8 healers, uint8 dps)
{
#if VERSION_STRING > TBC
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_QUEUE_STATUS %u dungeon: %u - waitTime: %d - avgWaitTime: %d - waitTimeTanks: %d - waitTimeHealer: %d - waitTimeDps: %d - queuedTime: %u - tanks: %u - healers: %u - dps: %u", GetPlayer()->GetGUID(), dungeon, waitTime, avgWaitTime, waitTimeTanks, waitTimeHealer, waitTimeDps, queuedTime, tanks, healers, dps);

    WorldPacket data(SMSG_LFG_QUEUE_STATUS, 4 + 4 + 4 + 4 + 4 + 4 + 1 + 1 + 1 + 4);

    data << uint32(dungeon);                               // Dungeon
    data << int32(avgWaitTime);                            // Average Wait time
    data << int32(waitTime);                               // Wait Time
    data << int32(waitTimeTanks);                          // Wait Tanks
    data << int32(waitTimeHealer);                         // Wait Healers
    data << int32(waitTimeDps);                            // Wait Dps
    data << uint8(tanks);                                  // Tanks needed
    data << uint8(healers);                                // Healers needed
    data << uint8(dps);                                    // Dps needed
    data << uint32(queuedTime);                            // Player wait time in queue
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgPlayerReward(uint32 RandomDungeonEntry, uint32 DungeonEntry, uint8 done, const LfgReward* reward, QuestProperties const* qReward)
{
#if VERSION_STRING > TBC
    if (!RandomDungeonEntry || !DungeonEntry || !qReward)
        return;

    uint8 itemNum = uint8(qReward->GetRewardItemCount());

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_PLAYER_REWARD %u rdungeonEntry: %u - sdungeonEntry: %u - done: %u", GetPlayer()->GetGUID(), RandomDungeonEntry, DungeonEntry, done);

    WorldPacket data(SMSG_LFG_PLAYER_REWARD, 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 1 + itemNum * (4 + 4 + 4));

    data << uint32(RandomDungeonEntry);                         // Random Dungeon Finished
    data << uint32(DungeonEntry);                         // Dungeon Finished
    data << uint8(done);
    data << uint32(1);
    data << uint32(qReward->reward_money);
    data << uint32(qReward->reward_xp);
    data << uint32(reward->reward[done].variableMoney);
    data << uint32(reward->reward[done].variableXP);
    data << uint8(itemNum);

    if (itemNum)
    {
        for (uint8 i = 0; i < 4; ++i)
        {
            if (!qReward->reward_item[i])
                continue;

            ItemProperties const* iProto = sMySQLStore.getItemProperties(qReward->reward_item[i]);

            data << uint32(qReward->reward_item[i]);
            data << uint32(iProto ? iProto->DisplayInfoID : 0);
            data << uint32(qReward->reward_itemcount[i]);
        }
    }
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgBootPlayer(const LfgPlayerBoot* pBoot)
{
#if VERSION_STRING > TBC
    uint64 guid = GetPlayer()->GetGUID();
    LfgAnswer playerVote = pBoot->votes.find(guid)->second;
    uint8 votesNum = 0;
    uint8 agreeNum = 0;
    uint32 secsleft = uint8((pBoot->cancelTime - time(NULL)) / 1000);

    for (LfgAnswerMap::const_iterator it = pBoot->votes.begin(); it != pBoot->votes.end(); ++it)
    {
        if (it->second != LFG_ANSWER_PENDING)
        {
            ++votesNum;
            if (it->second == LFG_ANSWER_AGREE)
                ++agreeNum;
        }
    }

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_BOOT_PROPOSAL_UPDATE %u inProgress: %u - didVote: %u - agree: %u - victim: %u votes: %u - agrees: %u - left: %u - needed: %u - reason %s",
           guid, uint8(pBoot->inProgress), uint8(playerVote != LFG_ANSWER_PENDING), uint8(playerVote == LFG_ANSWER_AGREE), pBoot->victim, votesNum, agreeNum, secsleft, pBoot->votedNeeded, pBoot->reason.c_str());
    
    WorldPacket data(SMSG_LFG_BOOT_PROPOSAL_UPDATE, 1 + 1 + 1 + 8 + 4 + 4 + 4 + 4 + pBoot->reason.length());

    data << uint8(pBoot->inProgress);                      // Vote in progress
    data << uint8(playerVote != LFG_ANSWER_PENDING);       // Did Vote
    data << uint8(playerVote == LFG_ANSWER_AGREE);         // Agree
    data << uint64(pBoot->victim);                         // Victim GUID
    data << uint32(votesNum);                              // Total Votes
    data << uint32(agreeNum);                              // Agree Count
    data << uint32(secsleft);                              // Time Left
    data << uint32(pBoot->votedNeeded);                    // Needed Votes
    data << pBoot->reason.c_str();                         // Kick reason
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgUpdateProposal(uint32 proposalId, const LfgProposal* pProp)
{
#if VERSION_STRING > TBC
    if (!pProp)
        return;

    uint64 guid = GetPlayer()->GetGUID();
    LfgProposalPlayerMap::const_iterator itPlayer = pProp->players.find(guid);
    if (itPlayer == pProp->players.end())                  // Player MUST be in the proposal
        return;

    LfgProposalPlayer* ppPlayer = itPlayer->second;
    uint32 pLowGroupGuid = ppPlayer->groupLowGuid;
    uint32 dLowGuid = pProp->groupLowGuid;
    uint32 dungeonId = pProp->dungeonId;
    bool isSameDungeon = false;
    bool isContinue = false;

    Group* grp = dLowGuid ? objmgr.GetGroupById(dLowGuid) : NULL; ///fiiiix
    uint32 completedEncounters = 0;
    if (grp)
    {
        uint64 gguid = grp->GetGUID();
        isContinue = grp->isLFGGroup() && sLfgMgr.GetState(gguid) != LFG_STATE_FINISHED_DUNGEON;
        isSameDungeon = GetPlayer()->GetGroup() == grp && isContinue;
    }

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_PROPOSAL_UPDATE %u state: %u", GetPlayer()->GetGUID(), pProp->state);
    WorldPacket data(SMSG_LFG_PROPOSAL_UPDATE, 4 + 1 + 4 + 4 + 1 + 1 + pProp->players.size() * (4 + 1 + 1 + 1 + 1 + 1));

    if (!isContinue)                                       // Only show proposal dungeon if it's continue
    {
        LfgDungeonSet playerDungeons = sLfgMgr.GetSelectedDungeons(guid);
        if (playerDungeons.size() == 1)
            dungeonId = (*playerDungeons.begin());
    }

#if VERSION_STRING != Cata
    if (DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(dungeonId))
    {
        dungeonId = dungeon->Entry();

        // Select a player inside to be get completed encounters from
        if (grp)
        {
            GroupMembersSet::iterator itx;
            for (itx = grp->GetSubGroup(0)->GetGroupMembersBegin(); itx != grp->GetSubGroup(0)->GetGroupMembersEnd(); ++itx)
            {
                Player* groupMember = (*itx)->m_loggedInPlayer;
                if (groupMember && groupMember->GetMapId() == uint32(dungeon->map))
                {
                    /* fiiiix
                    if (InstanceScript* instance = groupMember->GetInstanceScript())
                    completedEncounters = instance->GetCompletedEncounterMask();
                    */
                    break;
                }
            }
        }
    }
#endif

    data << uint32(dungeonId);                             // Dungeon
    data << uint8(pProp->state);                           // Result state
    data << uint32(proposalId);                            // Internal Proposal ID
    data << uint32(completedEncounters);                   // Bosses killed
    data << uint8(isSameDungeon);                          // Silent (show client window)
    data << uint8(pProp->players.size());                  // Group size

    for (itPlayer = pProp->players.begin(); itPlayer != pProp->players.end(); ++itPlayer)
    {
        ppPlayer = itPlayer->second;
        data << uint32(ppPlayer->role);                    // Role
        data << uint8(itPlayer->first == guid);            // Self player
        if (!ppPlayer->groupLowGuid)                       // Player not it a group
        {
            data << uint8(0);                              // Not in dungeon
            data << uint8(0);                              // Not same group
        }
        else
        {
            data << uint8(ppPlayer->groupLowGuid == dLowGuid);  // In dungeon (silent)
            data << uint8(ppPlayer->groupLowGuid == pLowGroupGuid); // Same Group than player
        }
        data << uint8(ppPlayer->accept != LFG_ANSWER_PENDING); // Answered
        data << uint8(ppPlayer->accept == LFG_ANSWER_AGREE); // Accepted
    }
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgUpdateSearch(bool update)
{
#if VERSION_STRING > TBC
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_UPDATE_SEARCH %u update: %u", GetPlayer()->GetGUID(), update ? 1 : 0);

    WorldPacket data(SMSG_LFG_UPDATE_SEARCH, 1);

    data << uint8(update);                                 // In Lfg Queue?
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgDisabled()
{
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_DISABLED %u", GetPlayer()->GetGUID());

    WorldPacket data(SMSG_LFG_DISABLED, 0);

    SendPacket(&data);
}

void WorldSession::SendLfgOfferContinue(uint32 dungeonEntry)
{
#if VERSION_STRING > TBC
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_OFFER_CONTINUE %u dungeon entry: %u", GetPlayer()->GetGUID(), dungeonEntry);

    WorldPacket data(SMSG_LFG_OFFER_CONTINUE, 4);

    data << uint32(dungeonEntry);
    SendPacket(&data);
#endif
}

void WorldSession::SendLfgTeleportError(uint8 err)
{
#if VERSION_STRING > TBC
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_TELEPORT_DENIED %u reason: %u", GetPlayer()->GetGUID(), err);

    WorldPacket data(SMSG_LFG_TELEPORT_DENIED, 4);

    data << uint32(err);                                   // Error
    SendPacket(&data);
#endif
}
