/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/LFG/LFGMgr.h"
#include "Common.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/CmsgSetLfgComment.h"
#include "Server/Packets/SmsgLfgUpdateSearch.h"
#include "Server/Packets/SmsgLfgOfferContinue.h"
#include "Server/Packets/SmsgLfgTeleportDenied.h"
#include "Server/Packets/CmsgLfgTeleport.h"
#include "Server/Packets/CmsgLfgSetBootVote.h"
#include "Server/Packets/CmsgLfgSetRoles.h"
#include "Server/Packets/CmsgLfgProposalResult.h"
#include "Server/Packets/SmsgLfgRoleChosen.h"
#include "Server/Packets/CmsgSearchLfgJoin.h"
#include "Server/Packets/CmsgSearchLfgLeave.h"

using namespace AscEmu::Packets;

void BuildPlayerLockDungeonBlock(WorldPacket& data, const LfgLockMap& lockMap)
{
    LogDebugFlag(LF_OPCODE, "BUILD PLAYER LOCK DUNGEON BLOCK");

    data << uint32_t(lockMap.size());
    for (const auto& lock : lockMap)
    {
        data << uint32_t(lock.first);
        data << uint32_t(lock.second);
    }
}

void BuildPartyLockDungeonBlock(WorldPacket& data, const LfgLockPartyMap& lockMap)
{
    LogDebugFlag(LF_OPCODE, "BUILD PARTY LOCK DUNGEON BLOCK");

    data << uint8_t(lockMap.size());
    for (const auto&  lock : lockMap)
    {
        data << uint64_t(lock.first);
        BuildPlayerLockDungeonBlock(data, lock.second);
    }
}

//not used cata
void WorldSession::sendLfgUpdateSearch(bool update)
{
#if VERSION_STRING > TBC
    SendPacket(SmsgLfgUpdateSearch(update).serialise().get());
#endif
}

//not used cata
void WorldSession::sendLfgDisabled()
{
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_DISABLED %u", _player->getGuid());

    WorldPacket data(SMSG_LFG_DISABLED, 0);
    SendPacket(&data);
}

void WorldSession::sendLfgOfferContinue(uint32_t dungeonEntry)
{
#if VERSION_STRING > TBC
    SendPacket(SmsgLfgOfferContinue(dungeonEntry).serialise().get());
#endif
}

void WorldSession::sendLfgTeleportError(uint8_t error)
{
#if VERSION_STRING > TBC
    SendPacket(SmsgLfgTeleportDenied(error).serialise().get());
#endif
}

void WorldSession::sendLfgJoinResult(const LfgJoinResultData& joinData)
{
#if VERSION_STRING > TBC
    uint32_t size = 0;
    for (auto lock : joinData.lockmap)
        size += 8 + 4 + uint32_t(lock.second.size()) * (4 + 4);

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_JOIN_RESULT %u heckResult: %u checkValue: %u", _player->getGuid(), joinData.result, joinData.state);

    WorldPacket data(SMSG_LFG_JOIN_RESULT, 4 + 4 + size);

    data << uint32_t(joinData.result);        // Check Result
    data << uint32_t(joinData.state);         // Check Value
    if (!joinData.lockmap.empty())
        BuildPartyLockDungeonBlock(data, joinData.lockmap);
    SendPacket(&data);
#endif
}

void WorldSession::sendLfgUpdatePlayer(const LfgUpdateData& updateData)
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

    uint64_t guid = _player->getGuid();
    uint8_t size = uint8_t(updateData.dungeons.size());

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_UPDATE_PLAYER %u updatetype: %u", guid, updateData.updateType);

    WorldPacket data(SMSG_LFG_UPDATE_PLAYER, 1 + 1 + (extrainfo ? 1 : 0) * (1 + 1 + 1 + 1 + size * 4 + updateData.comment.length()));

    data << uint8_t(updateData.updateType);       // Lfg Update type
    data << uint8_t(extrainfo);                   // Extra info
    if (extrainfo)
    {
        data << uint8_t(queued);                  // Join the queue
        data << uint8_t(0);                       // unk - Always 0
        data << uint8_t(0);                       // unk - Always 0
        data << uint8_t(size);
        if (size)
        {
            for (auto dungeonEntry : updateData.dungeons)
                data << uint32_t(dungeonEntry);
        }

        data << updateData.comment;
    }
    SendPacket(&data);
#endif
}

void WorldSession::sendLfgUpdateParty(const LfgUpdateData& updateData)
{
#if VERSION_STRING > TBC
    bool isJoining = false;
    bool hasExtraInfo = false;
    bool isQueued = false;

    switch (updateData.updateType)
    {
        case LFG_UPDATETYPE_JOIN_PROPOSAL:
            hasExtraInfo = true;
            break;
        case LFG_UPDATETYPE_ADDED_TO_QUEUE:
            hasExtraInfo = true;
            isJoining = true;
            isQueued = true;
            break;
        case LFG_UPDATETYPE_CLEAR_LOCK_LIST:
            // join = true;  // todo: Sometimes queued and hasExtraInfo - Check ocurrences...
            isQueued = true;
            break;
        case LFG_UPDATETYPE_PROPOSAL_BEGIN:
            hasExtraInfo = true;
            isJoining = true;
            break;
        default:
            break;
    }

    uint8_t dungeonSize = uint8_t(updateData.dungeons.size());

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_UPDATE_PARTY %lld updatetype: %u", _player->getGuid(), updateData.updateType);

    WorldPacket data(SMSG_LFG_UPDATE_PARTY, 1 + 1 + (hasExtraInfo ? 1 : 0) * (1 + 1 + 1 + 1 + 1 + dungeonSize * 4 + updateData.comment.length()));

    data << uint8_t(updateData.updateType);         // Lfg Update type
    data << uint8_t(hasExtraInfo);                  // Extra info
    if (hasExtraInfo)
    {
        data << uint8_t(isJoining);                 // LFG Join
        data << uint8_t(isQueued);                  // Join the queue
        data << uint8_t(0);                         // unk - Always 0
        data << uint8_t(0);                         // unk - Always 0
        for (uint8_t i = 0; i < 3; ++i)
            data << uint8_t(0);                     // unk - Always 0

        data << uint8_t(dungeonSize);

        if (dungeonSize)
        {
            for (auto dungeonEntry : updateData.dungeons)
                data << uint32_t(dungeonEntry);
        }

        data << updateData.comment;
    }
    SendPacket(&data);
#endif
}

void WorldSession::sendLfgRoleChosen(uint64_t guid, uint8_t roles)
{
#if VERSION_STRING > TBC
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_ROLE_CHOSEN %lld guid: %lld roles: %u", _player->getGuid(), guid, roles);

    SendPacket(SmsgLfgRoleChosen(guid, roles > 0 ? 1 : 0, roles).serialise().get());
#endif
}

void WorldSession::sendLfgRoleCheckUpdate(const LfgRoleCheck* pRoleCheck)
{
#if VERSION_STRING > TBC

    ASSERT(pRoleCheck);

    LfgDungeonSet dungeons;
    if (pRoleCheck->rDungeonId)
        dungeons.insert(pRoleCheck->rDungeonId);
    else
        dungeons = pRoleCheck->dungeons;

    LogDebugFlag(LF_OPCODE, "Sent SMSG_LFG_ROLE_CHECK_UPDATE %lld", _player->getGuid());

    WorldPacket data(SMSG_LFG_ROLE_CHECK_UPDATE, 4 + 1 + 1 + dungeons.size() * 4 + 1 + pRoleCheck->roles.size() * (8 + 1 + 4 + 1));

    data << uint32_t(pRoleCheck->state);       // Check result
    data << uint8_t(pRoleCheck->state == LFG_ROLECHECK_INITIALITING);
    data << uint8_t(dungeons.size());          // Number of dungeons
#if VERSION_STRING < Cata
    if (!dungeons.empty())
    {
        for (auto dungeonEntry : dungeons)
        {
            auto dungeon = sLFGDungeonStore.LookupEntry(dungeonEntry);
            data << uint32_t(dungeon ? dungeon->Entry() : 0);
        }
    }
#endif

    data << uint8_t(pRoleCheck->roles.size());               // Players in group
    if (!pRoleCheck->roles.empty())
    {
        // Leader info MUST be sent 1st :S
        uint64_t guid = pRoleCheck->leader;
        uint8_t roles = pRoleCheck->roles.find(guid)->second;
        data << uint64_t(guid);                              // Guid
        data << uint8_t(roles > 0);                          // Ready
        data << uint32_t(roles);                             // Roles

        WoWGuid wowGuid;
        wowGuid.Init(guid);

        Player* player = objmgr.GetPlayer(wowGuid.getGuidLowPart());
        data << uint8_t(player ? player->getLevel() : 0);    // Level

        for (const auto rolePair : pRoleCheck->roles)
        {
            if (rolePair.first == pRoleCheck->leader)
                continue;

            WoWGuid guidItr;
            guidItr.Init(rolePair.first);

            guid = rolePair.first;
            roles = rolePair.second;
            data << uint64_t(guid);                          // Guid
            data << uint8_t(roles > 0);                      // Ready
            data << uint32_t(roles);                         // Roles

            player = objmgr.GetPlayer(guidItr.getGuidLowPart());
            data << uint8_t(player ? player->getLevel() : 0);     // Level
        }
    }
    SendPacket(&data);
#endif
}

void WorldSession::sendLfgQueueStatus(uint32_t dungeon, int32_t waitTime, int32_t avgWaitTime, int32_t waitTimeTanks, int32_t waitTimeHealer, int32_t waitTimeDps, uint32_t queuedTime, uint8_t tanks, uint8_t healers, uint8_t dps)
{
#if VERSION_STRING > TBC
    LogDebugFlag(LF_OPCODE, "SMSG_LFG_QUEUE_STATUS %u dungeon: %u - waitTime: %d - avgWaitTime: %d - waitTimeTanks: %d - waitTimeHealer: %d - waitTimeDps: %d - queuedTime: %u - tanks: %u - healers: %u - dps: %u", _player->getGuid(), dungeon, waitTime, avgWaitTime, waitTimeTanks, waitTimeHealer, waitTimeDps, queuedTime, tanks, healers, dps);

    WorldPacket data(SMSG_LFG_QUEUE_STATUS, 4 + 4 + 4 + 4 + 4 + 4 + 1 + 1 + 1 + 4);

    data << uint32_t(dungeon);                               // Dungeon
    data << int32_t(avgWaitTime);                            // Average Wait time
    data << int32_t(waitTime);                               // Wait Time
    data << int32_t(waitTimeTanks);                          // Wait Tanks
    data << int32_t(waitTimeHealer);                         // Wait Healers
    data << int32_t(waitTimeDps);                            // Wait Dps
    data << uint8_t(tanks);                                  // Tanks needed
    data << uint8_t(healers);                                // Healers needed
    data << uint8_t(dps);                                    // Dps needed
    data << uint32_t(queuedTime);                            // Player wait time in queue

    SendPacket(&data);
#endif
}

void WorldSession::sendLfgPlayerReward(uint32_t RandomDungeonEntry, uint32_t DungeonEntry, uint8_t done, const LfgReward* reward, QuestProperties const* qReward)
{
#if VERSION_STRING > TBC
    if (!RandomDungeonEntry || !DungeonEntry || !qReward)
        return;

    uint8_t itemNum = uint8_t(qReward->GetRewardItemCount());

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_PLAYER_REWARD %u rdungeonEntry: %u - sdungeonEntry: %u - done: %u", _player->getGuid(), RandomDungeonEntry, DungeonEntry, done);

    WorldPacket data(SMSG_LFG_PLAYER_REWARD, 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 1 + itemNum * (4 + 4 + 4));

    data << uint32_t(RandomDungeonEntry);                   // Random Dungeon Finished
    data << uint32_t(DungeonEntry);                         // Dungeon Finished
    data << uint8_t(done);
    data << uint32_t(1);
    data << uint32_t(qReward->reward_money);
    data << uint32_t(qReward->reward_xp);
    data << uint32_t(reward->reward[done].variableMoney);
    data << uint32_t(reward->reward[done].variableXP);
    data << uint8_t(itemNum);

    if (itemNum)
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            if (!qReward->reward_item[i])
                continue;

            auto itemProperties = sMySQLStore.getItemProperties(qReward->reward_item[i]);

            data << uint32_t(qReward->reward_item[i]);
            data << uint32_t(itemProperties ? itemProperties->DisplayInfoID : 0);
            data << uint32_t(qReward->reward_itemcount[i]);
        }
    }
    SendPacket(&data);
#endif
}

void WorldSession::sendLfgBootPlayer(const LfgPlayerBoot* pBoot)
{
#if VERSION_STRING > TBC
    uint64_t guid = _player->getGuid();
    const LfgAnswer playerVote = pBoot->votes.find(guid)->second;
    uint8_t votesNum = 0;
    uint8_t agreeNum = 0;
    const uint32_t secsleft = uint8_t((pBoot->cancelTime - time(nullptr)) / 1000);

    for (const auto vote : pBoot->votes)
    {
        if (vote.second != LFG_ANSWER_PENDING)
        {
            ++votesNum;
            if (vote.second == LFG_ANSWER_AGREE)
                ++agreeNum;
        }
    }

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_BOOT_PROPOSAL_UPDATE %u inProgress: %u - didVote: %u - agree: %u - victim: %u votes: %u - agrees: %u - left: %u - needed: %u - reason %s",
        guid, uint8_t(pBoot->inProgress), uint8_t(playerVote != LFG_ANSWER_PENDING), uint8_t(playerVote == LFG_ANSWER_AGREE),
        pBoot->victim, votesNum, agreeNum, secsleft, pBoot->votedNeeded, pBoot->reason.c_str());

    WorldPacket data(SMSG_LFG_BOOT_PROPOSAL_UPDATE, 1 + 1 + 1 + 8 + 4 + 4 + 4 + 4 + pBoot->reason.length());

    data << uint8_t(pBoot->inProgress);                      // Vote in progress
    data << uint8_t(playerVote != LFG_ANSWER_PENDING);       // Did Vote
    data << uint8_t(playerVote == LFG_ANSWER_AGREE);         // Agree
    data << uint64_t(pBoot->victim);                         // Victim GUID
    data << uint32_t(votesNum);                              // Total Votes
    data << uint32_t(agreeNum);                              // Agree Count
    data << uint32_t(secsleft);                              // Time Left
    data << uint32_t(pBoot->votedNeeded);                    // Needed Votes
    data << pBoot->reason.c_str();                           // Kick reason

    SendPacket(&data);
#endif
}

void WorldSession::sendLfgUpdateProposal(uint32_t proposalId, const LfgProposal* pProp)
{
#if VERSION_STRING > TBC
    if (!pProp)
        return;

    uint64_t guid = _player->getGuid();
    auto itPlayer = pProp->players.find(guid);
    if (itPlayer == pProp->players.end())
        return;

    LfgProposalPlayer* ppPlayer = itPlayer->second;
    const uint32_t pLowGroupGuid = ppPlayer->groupLowGuid;
    const uint32_t dLowGuid = pProp->groupLowGuid;
    uint32_t dungeonId = pProp->dungeonId;
    bool isSameDungeon = false;
    bool isContinue = false;

    Group* grp = dLowGuid ? objmgr.GetGroupById(dLowGuid) : nullptr;
    const uint32_t completedEncounters = 0;
    if (grp)
    {
        uint64_t gguid = grp->GetGUID();
        isContinue = grp->isLFGGroup() && sLfgMgr.GetState(gguid) != LFG_STATE_FINISHED_DUNGEON;
        isSameDungeon = _player->GetGroup() == grp && isContinue;
    }

    LogDebugFlag(LF_OPCODE, "SMSG_LFG_PROPOSAL_UPDATE %u state: %u", _player->getGuid(), pProp->state);
    WorldPacket data(SMSG_LFG_PROPOSAL_UPDATE, 4 + 1 + 4 + 4 + 1 + 1 + pProp->players.size() * (4 + 1 + 1 + 1 + 1 + 1));

    if (!isContinue)                                       // Only show proposal dungeon if it's continue
    {
        LfgDungeonSet playerDungeons = sLfgMgr.GetSelectedDungeons(guid);
        if (playerDungeons.size() == 1)
            dungeonId = (*playerDungeons.begin());
    }

#if VERSION_STRING < Cata
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
                if (groupMember && groupMember->GetMapId() == uint32_t(dungeon->map))
                {
                    /* todo fix this
                    if (InstanceScript* instance = groupMember->GetInstanceScript())
                    completedEncounters = instance->GetCompletedEncounterMask();
                    */
                    break;
                }
            }
        }
    }
#endif

    data << uint32_t(dungeonId);                             // Dungeon
    data << uint8_t(pProp->state);                           // Result state
    data << uint32_t(proposalId);                            // Internal Proposal ID
    data << uint32_t(completedEncounters);                   // Bosses killed
    data << uint8_t(isSameDungeon);                          // Silent (show client window)
    data << uint8_t(pProp->players.size());                  // Group size

    for (itPlayer = pProp->players.begin(); itPlayer != pProp->players.end(); ++itPlayer)
    {
        ppPlayer = itPlayer->second;
        data << uint32_t(ppPlayer->role);                    // Role
        data << uint8_t(itPlayer->first == guid);            // Self player
        if (!ppPlayer->groupLowGuid)                       // Player not it a group
        {
            data << uint8_t(0);                              // Not in dungeon
            data << uint8_t(0);                              // Not same group
        }
        else
        {
            data << uint8_t(ppPlayer->groupLowGuid == dLowGuid);  // In dungeon (silent)
            data << uint8_t(ppPlayer->groupLowGuid == pLowGroupGuid); // Same Group than player
        }
        data << uint8_t(ppPlayer->accept != LFG_ANSWER_PENDING); // Answered
        data << uint8_t(ppPlayer->accept == LFG_ANSWER_AGREE); // Accepted
    }
    SendPacket(&data);
#endif
}

void WorldSession::handleLfgSetCommentOpcode(WorldPacket& recvPacket)
{
    CmsgSetLfgComment srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SET_LFG_COMMENT playerGuid: %lld, comment: %s", _player->getGuid(), srlPacket.comment.c_str());

    sLfgMgr.SetComment(_player->getGuid(), srlPacket.comment);
}

#if VERSION_STRING >= Cata
void WorldSession::handleLfgLockInfoOpcode(WorldPacket& recvPacket)
{
    const bool requestFromPlayer = recvPacket.readBit();

    LogDebugFlag(LF_OPCODE, "Received CMSG_LFG_LOCK_INFO_REQUEST from %s", requestFromPlayer ? "player" : "group");

    //\todo handle player lock info and group lock info here
}
#endif

#if VERSION_STRING > TBC
void WorldSession::handleLfgJoinOpcode(WorldPacket& recvPacket)
{
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_JOIN");

    if (_player->GetGroup() && _player->GetGroup()->GetLeader()->guid != _player->getGuid() 
        && (_player->GetGroup()->MemberCount() == 5 || !_player->GetGroup()->isLFGGroup()))
    {
        LogDebug("handleLfgJoinOpcode : Unable to JoinQueue");

        recvPacket.clear();
        return;
    }

    uint8_t numDungeons;
    uint32_t dungeon;
    uint32_t roles;

    recvPacket >> roles;
    recvPacket.read<uint16_t>();        // uint8_t (always 0) - uint8_t (always 0)
    recvPacket >> numDungeons;

    if (!numDungeons)
    {
        LogDebugFlag(LF_OPCODE, "CMSG_LFG_JOIN no dungeons selected", _player->getGuid());
        recvPacket.clear();
        return;
    }

    LfgDungeonSet newDungeons;
    for (int8_t i = 0; i < numDungeons; ++i)
    {
        recvPacket >> dungeon;
        newDungeons.insert(dungeon & 0x00FFFFFF);       // remove the type from the dungeon entry
    }

    recvPacket.read<uint32_t>();                        // for 0..uint8_t (always 3) { uint8_t (always 0) }

    std::string comment;
    recvPacket >> comment;
    LogDebugFlag(LF_OPCODE, "CMSG_LFG_JOIN: %s, roles: %u, Dungeons: %u, Comment: %s", _player->getName().c_str(), roles, uint8_t(newDungeons.size()), comment.c_str());
    sLfgMgr.Join(_player, uint8_t(roles), newDungeons, comment);
}

void WorldSession::handleLfgLeaveOpcode(WorldPacket& /*recvPacket*/)
{
    Group* grp = _player->GetGroup();

    LogDebugFlag(LF_OPCODE, "Received CMSG_LFG_LEAVE %lld in group: %u", _player->getGuid(), grp ? 1 : 0);

    // Check cheating - only leader can leave the queue
    if (!grp || grp->GetLeader()->guid == _player->getGuid())
        sLfgMgr.Leave(_player, grp);
}

void WorldSession::handleLfgSearchOpcode(WorldPacket& recvPacket)
{
    CmsgSearchLfgJoin srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SEARCH_LFG_JOIN for guid %lld dungeon entry: %u",
        _player->getGuid(), srlPacket.entry);
}

void WorldSession::handleLfgSearchLeaveOpcode(WorldPacket& recvPacket)
{
    CmsgSearchLfgLeave srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_SEARCH_LFG_LEAVE for guid %lld dungeonId: %u",
        _player->getGuid(), srlPacket.entry);
}

void WorldSession::handleLfgProposalResultOpcode(WorldPacket& recvPacket)
{
    CmsgLfgProposalResult srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_LFG_PROPOSAL_RESULT guid %lld proposal: %u accept: %u", 
        _player->getGuid(), srlPacket.lfgGroupId, srlPacket.accept ? 1 : 0);

    sLfgMgr.UpdateProposal(srlPacket.lfgGroupId, _player->getGuid(), srlPacket.accept);
}

void WorldSession::handleLfgSetRolesOpcode(WorldPacket& recvPacket)
{
    CmsgLfgSetRoles srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (Group* grp = _player->GetGroup())
    {
        LogDebugFlag(LF_OPCODE, "Received CMSG_LFG_SET_ROLES: Group %lld, Player %lld, Roles: %u",
            grp->GetGUID(), _player->getGuid(), srlPacket.roles);

        sLfgMgr.UpdateRoleCheck(grp->GetGUID(), _player->getGuid(), srlPacket.roles);
    }
}

void WorldSession::handleLfgSetBootVoteOpcode(WorldPacket& recvPacket)
{
    CmsgLfgSetBootVote srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_LFG_SET_BOOT_VOTE %lld agree: %u",
        _player->getGuid(), srlPacket.voteFor ? 1 : 0);

    sLfgMgr.UpdateBoot(_player, srlPacket.voteFor);
}

void WorldSession::handleLfgPlayerLockInfoRequestOpcode(WorldPacket& /*recvPacket*/)
{
    uint64_t guid = _player->getGuid();

    LogDebugFlag(LF_OPCODE, "Received CMSG_LFD_PLAYER_LOCK_INFO_REQUEST %lld", guid);

    // Get Random dungeons that can be done at a certain level and expansion
    // todo FIXME - Should return seasonals (when not disabled)
    LfgDungeonSet randomDungeons;
    const uint8_t level = static_cast<uint8_t>(_player->getLevel());

#if VERSION_STRING < Cata
    uint8_t expansion = static_cast<uint8_t>(_player->GetSession()->GetFlags());
    for (uint32_t i = 0; i < sLFGDungeonStore.GetNumRows(); ++i)
    {
        DBC::Structures::LFGDungeonEntry const* dungeon = sLFGDungeonStore.LookupEntry(i);
        if (dungeon && dungeon->type == LFG_TYPE_RANDOM && dungeon->expansion <= expansion && dungeon->minlevel <= level && level <= dungeon->maxlevel)
            randomDungeons.insert(dungeon->Entry());

    }
#endif

    // Get player locked Dungeons
    LfgLockMap lock = sLfgMgr.GetLockedDungeons(guid);
    const uint32_t rsize = uint32_t(randomDungeons.size());
    const uint32_t lsize = uint32_t(lock.size());

    LogDebugFlag(LF_OPCODE, "Received SMSG_LFG_PLAYER_INFO %lld", guid);
    WorldPacket data(SMSG_LFG_PLAYER_INFO, 1 + rsize * (4 + 1 + 4 + 4 + 4 + 4 + 1 + 4 + 4 + 4) + 4 + lsize * (1 + 4 + 4 + 4 + 4 + 1 + 4 + 4 + 4));

    data << uint8_t(randomDungeons.size());                 // Random Dungeon count
    for (auto randomDungeon : randomDungeons)
    {
        data << uint32_t(randomDungeon);                    // Dungeon Entry (id + type)
        LfgReward const* reward = sLfgMgr.GetRandomDungeonReward(randomDungeon, level);
        QuestProperties const* qRew = nullptr;
        uint8_t done = 0;
        if (reward)
        {
            qRew = sMySQLStore.getQuestProperties(reward->reward[0].questId);
            if (qRew)
            {
                done = _player->HasFinishedQuest(qRew->id);
                if (done)
                    qRew = sMySQLStore.getQuestProperties(reward->reward[1].questId);
            }
        }
        if (qRew)
        {
            data << uint8_t(done);
            data << uint32_t(qRew->reward_money);
            data << uint32_t(qRew->reward_xp);
            data << uint32_t(reward->reward[done].variableMoney);
            data << uint32_t(reward->reward[done].variableXP);
            // todo FIXME Linux: error: cast from const uint32_t* {aka const unsigned int*} to uint8_t {aka unsigned char} loses precision 
            // can someone check this now ?

            data << uint8_t(qRew->GetRewardItemCount());
            for (uint8_t i = 0; i < 4; ++i)
                if (qRew->reward_item[i] != 0)
                {
                    ItemProperties const* item = sMySQLStore.getItemProperties(qRew->reward_item[i]);
                    data << uint32_t(qRew->reward_item[i]);
                    data << uint32_t(item ? item->DisplayInfoID : 0);
                    data << uint32_t(qRew->reward_itemcount[i]);
                }
        }
        else
        {
            data << uint8_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint32_t(0);
            data << uint8_t(0);
        }
    }
    BuildPlayerLockDungeonBlock(data, lock);
    SendPacket(&data);
}

void WorldSession::handleLfgTeleportOpcode(WorldPacket& recvPacket)
{
    CmsgLfgTeleport srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_LFG_TELEPORT guid %lld out: %u", _player->getGuid(), srlPacket.teleportOut ? 1 : 0);
    sLfgMgr.TeleportPlayer(_player, srlPacket.teleportOut, true);
}

void WorldSession::handleLfgPartyLockInfoRequestOpcode(WorldPacket& /*recvPacket*/)
{
    uint64_t guid = _player->getGuid();

    LogDebugFlag(LF_OPCODE, "Received CMSG_LFD_PARTY_LOCK_INFO_REQUEST guid %lld", guid);

    Group* grp = _player->GetGroup();
    if (!grp)
        return;

    // Get the locked dungeons of the other party members
    LfgLockPartyMap lockMap;
    for (auto groupPlayerInfo : grp->GetSubGroup(0)->getGroupMembers())
    {
        Player* plrg = groupPlayerInfo->m_loggedInPlayer;
        if (!plrg)
            continue;

        uint64_t pguid = plrg->getGuid();
        if (pguid == guid)
            continue;

        lockMap[pguid] = sLfgMgr.GetLockedDungeons(pguid);
    }

    uint32_t size = 0;
    for (auto lock : lockMap)
        size += 8 + 4 + uint32_t(lock.second.size()) * (4 + 4);

    LogDebugFlag(LF_OPCODE, "Sent SMSG_LFG_PARTY_INFO %lld", guid);

    WorldPacket data(SMSG_LFG_PARTY_INFO, 1 + size);
    BuildPartyLockDungeonBlock(data, lockMap);
    SendPacket(&data);
}
#endif

