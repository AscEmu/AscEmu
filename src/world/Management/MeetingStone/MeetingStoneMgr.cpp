/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MeetingStoneMgr.hpp"

#include <algorithm>

#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgLfgUpdate.h"
#include "Server/Packets/SmsgLfgUpdateQueued.h"
#include "Server/Packets/SmsgLfgLeaderIsLfm.h"
#include "Server/Packets/SmsgLfgPendingInvite.h"
#include "Server/Packets/SmsgLfgPendingMatch.h"
#include "Server/Packets/SmsgMeetingstoneComplete.h"
#include "Server/Packets/SmsgMeetingstoneInProgress.h"
#include "Server/Packets/SmsgMeetingstoneMemberAdded.h"
#include "Server/Packets/MsgLookingForGroup.h"
#include "WoWGuid.hpp"

using namespace AscEmu::Packets;

namespace
{
    Player* resolvePlayer(uint64_t guid)
    {
        return sObjectMgr.getPlayer(WoWGuid::getLowGuidFromRaw(guid));
    }
}

MeetingStoneQueue& MeetingStoneQueue::getInstance()
{
    static MeetingStoneQueue mInstance;
    return mInstance;
}

void MeetingStoneQueue::initialize()
{
    m_queuedPlayers.clear();
}

void MeetingStoneQueue::finalize()
{
    m_queuedPlayers.clear();
}

MeetingStonePlayerInfo const* MeetingStoneQueue::getInfo(uint64_t guid) const
{
    const auto itr = m_queuedPlayers.find(guid);
    return itr != m_queuedPlayers.end() ? &itr->second : nullptr;
}

void MeetingStoneQueue::setComment(uint64_t playerGuid, std::string const& comment)
{
    const auto itr = m_queuedPlayers.find(playerGuid);
    if (itr == m_queuedPlayers.end())
        return;

    itr->second.comment = comment;
}

void MeetingStoneQueue::setAutoFill(uint64_t playerGuid, bool state)
{
    const auto itr = m_queuedPlayers.find(playerGuid);
    if (itr == m_queuedPlayers.end())
        return;

    itr->second.autoFill = state;
    tryFill(playerGuid);
}

void MeetingStoneQueue::setAutoJoin(uint64_t playerGuid, bool state)
{
    const auto itr = m_queuedPlayers.find(playerGuid);
    if (itr == m_queuedPlayers.end())
        return;

    itr->second.autoJoin = state;
    tryJoin(playerGuid);
}

void MeetingStoneQueue::startLookingForMore(MeetingStonePlayerInfo info, uint64_t invokerGuid)
{
    const auto itr = m_queuedPlayers.find(info.leaderGuid);
    if (itr != m_queuedPlayers.end())
    {
        if (info.leaderGuid != invokerGuid)
        {
            if (auto* const invoker = resolvePlayer(invokerGuid))
            {
                SmsgLfgLeaderIsLfm managedPacket;
                invoker->getSession()->sendManagedPacket(managedPacket);
            }
        }

        sendLFGUpdate(info.leaderGuid, invokerGuid);
        return;
    }

    m_queuedPlayers.emplace(info.leaderGuid, std::move(info));

    groupUpdateUI(invokerGuid, false);
    groupUpdateQueueStatus(invokerGuid);

    tryFill(invokerGuid);
}

void MeetingStoneQueue::stopLookingForMore(uint64_t playerGuid)
{
    const auto itr = m_queuedPlayers.find(playerGuid);
    if (itr == m_queuedPlayers.end())
        return;

    m_queuedPlayers.erase(itr);
}

void MeetingStoneQueue::startLookingForGroup(MeetingStonePlayerInfo info, uint64_t invokerGuid)
{
    const auto itr = m_queuedPlayers.find(info.leaderGuid);
    if (itr != m_queuedPlayers.end())
    {
        if (info.leaderGuid != invokerGuid)
        {
            if (auto* const invoker = resolvePlayer(invokerGuid))
            {
                SmsgLfgLeaderIsLfm managedPacket;
                invoker->getSession()->sendManagedPacket(managedPacket);
            }
        }

        sendLFGUpdate(info.leaderGuid, invokerGuid);
        return;
    }

    const uint64_t leaderGuid = info.leaderGuid;
    m_queuedPlayers.emplace(leaderGuid, std::move(info));

    sendLFGUpdate(leaderGuid, invokerGuid);
    groupUpdateQueueStatus(leaderGuid);

    tryJoin(leaderGuid);
}

void MeetingStoneQueue::stopLookingForGroup(uint64_t leaderGuid, uint64_t /*playerGuid*/)
{
    const auto itr = m_queuedPlayers.find(leaderGuid);
    if (itr != m_queuedPlayers.end())
        m_queuedPlayers.erase(itr);

    groupUpdateUI(leaderGuid, false);
}

void MeetingStoneQueue::setLfgSlot(uint64_t leaderGuid, uint8_t slot, uint16_t entry, uint16_t type)
{
    if (slot >= MEETINGSTONE_MAX_LFG_SLOTS)
        return;

    const auto itr = m_queuedPlayers.find(leaderGuid);
    if (itr == m_queuedPlayers.end())
        return;

    auto& info = itr->second;
    info.group[slot].set(entry, type);

    const bool stillLfg = info.isLFG();

    groupUpdateUI(leaderGuid, false);

    if (!stillLfg)
        m_queuedPlayers.erase(leaderGuid);
}

void MeetingStoneQueue::setLfmData(uint64_t leaderGuid, uint16_t entry, uint16_t type)
{
    const auto itr = m_queuedPlayers.find(leaderGuid);
    if (itr == m_queuedPlayers.end())
        return;

    auto& info = itr->second;
    info.more.set(entry, type);

    const bool stillLfm = info.isLFM();

    groupUpdateUI(leaderGuid, false);

    if (!stillLfm)
        m_queuedPlayers.erase(leaderGuid);
}

void MeetingStoneQueue::tryJoin(uint64_t playerGuid)
{
    const auto itr = m_queuedPlayers.find(playerGuid);
    if (itr == m_queuedPlayers.end() || !itr->second.autoJoin)
        return;

    auto& info = itr->second;
    bool attempted = false;

    for (auto& [_, otherInfo] : m_queuedPlayers)
    {
        if (!otherInfo.autoFill)
            continue;

        if (!info.isLFG(otherInfo, true))
            continue;

        if (addMember(otherInfo, info, otherInfo.more.entry))
        {
            attempted = false;
            break;
        }

        attempted = true;
    }

    if (attempted)
    {
        if (auto* const player = resolvePlayer(playerGuid))
        {
            SmsgMeetingstoneInProgress managedPacket;
            player->getSession()->sendManagedPacket(managedPacket);
        }
    }
}

void MeetingStoneQueue::tryFill(uint64_t leaderGuid)
{
    const auto itr = m_queuedPlayers.find(leaderGuid);
    if (itr == m_queuedPlayers.end() || !itr->second.autoFill)
        return;

    auto& info = itr->second;
    bool attempted = false;

    for (auto& [_, otherInfo] : m_queuedPlayers)
    {
        if (!otherInfo.autoJoin)
            continue;

        if (!otherInfo.isLFG(info, true))
            continue;

        if (!addMember(info, otherInfo, info.more.entry))
        {
            attempted = true;
            break;
        }
    }

    if (attempted)
    {
        if (auto* const leader = resolvePlayer(leaderGuid))
        {
            SmsgMeetingstoneInProgress managedPacket;
            leader->getSession()->sendManagedPacket(managedPacket);
        }
    }
}

bool MeetingStoneQueue::addMember(MeetingStonePlayerInfo& leaderInfo, MeetingStonePlayerInfo& joinerInfo, uint32_t entry)
{
    const uint64_t leaderGuid = leaderInfo.leaderGuid;
    const uint64_t joinerGuid = joinerInfo.leaderGuid;

    leaderInfo.pendingMembers.push_back(joinerGuid);

    joinerInfo.pendingTransfer = true;
    joinerInfo.pendingLeaderGuid = leaderGuid;
    joinerInfo.pendingEntry = entry;

    auto* const joiner = resolvePlayer(joinerGuid);
    auto* const leader = resolvePlayer(leaderGuid);

    if (leader != nullptr && leader->getGroup() != nullptr && leader->getGroup()->IsFull())
    {
        removePendingJoin(leaderGuid, joinerGuid);
        return false;
    }

    if (joiner != nullptr)
    {
        SmsgLfgPendingInvite managedPacket(entry);
        joiner->getSession()->sendManagedPacket(managedPacket);
    }

    if (leader != nullptr)
    {
        SmsgLfgPendingMatch managedPacket(entry);
        leader->getSession()->sendManagedPacket(managedPacket);
    }

    return true;
}

void MeetingStoneQueue::handlePendingJoin(uint64_t playerGuid)
{
    const auto itr = m_queuedPlayers.find(playerGuid);
    if (itr == m_queuedPlayers.end() || !itr->second.pendingTransfer)
        return;

    auto& info = itr->second;
    const uint64_t leaderGuid = info.pendingLeaderGuid;

    auto* const player = resolvePlayer(playerGuid);
    auto* const leader = resolvePlayer(leaderGuid);
    if (player == nullptr || leader == nullptr)
    {
        sLogger.failure("MeetingStoneQueue::handlePendingJoin: offline player or leader for guid {} {}", playerGuid, leaderGuid);
        removePendingJoin(leaderGuid, playerGuid);
        return;
    }

    Group* grp = leader->getGroup();

    if (Group* const currentGroup = player->getGroup(); currentGroup != nullptr && currentGroup != grp)
        currentGroup->RemovePlayer(player->getPlayerInfo());

    bool created = false;
    if (grp == nullptr)
    {
        grp = sObjectMgr.createGroup();
        if (grp == nullptr)
        {
            removePendingJoin(leaderGuid, playerGuid);
            return;
        }

        created = true;
        grp->AddMember(leader->getPlayerInfo());
    }

    if (player->getGroup() != grp)
    {
        if (!grp->AddMember(player->getPlayerInfo()))
        {
            removePendingJoin(leaderGuid, playerGuid);
            return;
        }
    }

    if (created)
    {
        SmsgMeetingstoneComplete completePacket;
        leader->getSession()->sendManagedPacket(completePacket);
    }

    SmsgMeetingstoneMemberAdded memberAddedPacket(player->getGuid());
    if (auto const serialisedPacket = memberAddedPacket.serialise())
        grp->SendPacketToAll(serialisedPacket.get());

    SmsgMeetingstoneComplete completePacket;
    player->getSession()->sendManagedPacket(completePacket);

    const bool full = grp->IsFull();
    pendingJoinSuccess(leaderGuid, playerGuid, full);
}

void MeetingStoneQueue::handleDeclinePendingJoin(uint64_t playerGuid)
{
    const auto itr = m_queuedPlayers.find(playerGuid);
    if (itr == m_queuedPlayers.end() || !itr->second.pendingTransfer)
        return;

    removePendingJoin(itr->second.pendingLeaderGuid, playerGuid);
}

void MeetingStoneQueue::removePendingJoin(uint64_t leaderGuid, uint64_t playerGuid)
{
    if (const auto leaderItr = m_queuedPlayers.find(leaderGuid); leaderItr != m_queuedPlayers.end())
    {
        auto& members = leaderItr->second.pendingMembers;
        members.erase(std::remove(members.begin(), members.end(), playerGuid), members.end());
    }

    if (const auto playerItr = m_queuedPlayers.find(playerGuid); playerItr != m_queuedPlayers.end())
        playerItr->second.pendingTransfer = false;
}

void MeetingStoneQueue::pendingJoinSuccess(uint64_t leaderGuid, uint64_t playerGuid, bool full)
{
    const auto leaderItr = m_queuedPlayers.find(leaderGuid);
    const auto playerItr = m_queuedPlayers.find(playerGuid);

    if (leaderItr != m_queuedPlayers.end())
    {
        auto& members = leaderItr->second.pendingMembers;
        members.erase(std::remove(members.begin(), members.end(), playerGuid), members.end());

        if (playerItr != m_queuedPlayers.end())
        {
            MeetingStoneGroupMember member;
            member.guid = playerGuid;
            member.level = playerItr->second.level;
            leaderItr->second.members.push_back(member);

            m_queuedPlayers.erase(playerItr);
        }
    }
    else if (playerItr != m_queuedPlayers.end())
    {
        playerItr->second.pendingTransfer = false;
    }

    if (full && leaderItr != m_queuedPlayers.end())
        m_queuedPlayers.erase(leaderItr);

    groupUpdate(leaderGuid, full);
}

void MeetingStoneQueue::sendLFGUpdate(uint64_t leaderGuid, uint64_t playerGuid) const
{
    auto* const player = resolvePlayer(playerGuid);
    if (player == nullptr)
        return;

    const auto itr = m_queuedPlayers.find(leaderGuid);
    bool queued = itr != m_queuedPlayers.end();
    bool lfg = false;
    bool lfm = false;
    uint32_t data = 0;

    if (queued)
    {
        auto const& info = itr->second;
        lfm = info.isLFM();
        lfg = info.isLFG();
        data = uint32_t(info.more.entry | (info.more.type << 24));
    }

    SmsgLfgUpdate managedPacket(queued, lfg, lfm, data);
    player->getSession()->sendManagedPacket(managedPacket);
}

void MeetingStoneQueue::groupUpdate(uint64_t leaderGuid, bool completed)
{
    groupUpdateQueueStatus(leaderGuid);
    groupUpdateUI(leaderGuid, completed);
}

bool MeetingStoneQueue::groupUpdateQueueStatus(uint64_t leaderGuid)
{
    const auto itr = m_queuedPlayers.find(leaderGuid);
    if (itr == m_queuedPlayers.end())
        return false;

    auto& info = itr->second;

    const bool autojoin = info.isAutoJoin();
    const bool autofill = info.isAutoFill();
    const bool status = (autojoin && info.members.empty()) || (autofill && !info.members.empty());

    if (info.status == status)
        return false;

    info.status = status;

    const bool queued = true;
    const bool lfm = info.isLFM();

    SmsgLfgUpdateQueued queuedPacket(queued);

    if (auto* const leader = resolvePlayer(leaderGuid))
    {
        leader->getSession()->sendManagedPacket(queuedPacket);
        if (lfm)
            leader->sendMeetingStoneSetQueuePacket(info.more.entry, static_cast<uint8_t>(MeetingStoneQueueStatus::InQueue));
    }

    for (auto const& member : info.members)
    {
        if (auto* const memberPlayer = resolvePlayer(member.guid))
        {
            memberPlayer->getSession()->sendManagedPacket(queuedPacket);
            if (lfm)
                memberPlayer->sendMeetingStoneSetQueuePacket(info.more.entry, static_cast<uint8_t>(MeetingStoneQueueStatus::InQueue));
        }
    }

    return true;
}

void MeetingStoneQueue::groupUpdateUI(uint64_t leaderGuid, bool completed)
{
    std::vector<uint64_t> memberGuids;

    if (const auto itr = m_queuedPlayers.find(leaderGuid); itr != m_queuedPlayers.end())
        for (auto const& member : itr->second.members)
            memberGuids.push_back(member.guid);

    auto notify = [completed](Player* player)
    {
        if (player == nullptr)
            return;

        if (completed)
        {
            SmsgMeetingstoneComplete managedPacket;
            player->getSession()->sendManagedPacket(managedPacket);
        }
        else
        {
            SmsgLfgUpdateQueued managedPacket(false);
            player->getSession()->sendManagedPacket(managedPacket);
        }
    };

    notify(resolvePlayer(leaderGuid));

    for (auto const memberGuid : memberGuids)
        notify(resolvePlayer(memberGuid));
}

void MeetingStoneQueue::sendListQueryResponse(uint64_t playerGuid, PlayerTeam playerTeam, uint32_t type, uint32_t entry) const
{
    auto* const player = resolvePlayer(playerGuid);
    if (player == nullptr)
        return;

    std::vector<MsgLookingForGroupEntry> entries;
    entries.reserve(m_queuedPlayers.size());

    for (auto const& [leaderGuid, info] : m_queuedPlayers)
    {
        if (info.team != playerTeam)
            continue;

        if (!info.isLeader || info.full)
            continue;

        MsgLookingForGroupEntry entry_;
        entry_.guid = leaderGuid;
        entry_.level = info.level;
        entry_.zoneId = info.zoneId;
        entry_.isLFM = info.isLFM(static_cast<uint16_t>(entry), static_cast<uint16_t>(type));

        if (entry_.isLFM)
        {
            entry_.lfmData = uint32_t(info.more.entry | (info.more.type << 24));
        }
        else
        {
            for (uint8_t i = 0; i < MEETINGSTONE_MAX_LFG_SLOTS; ++i)
                entry_.lfgSlots[i] = uint32_t(info.group[i].entry | (info.group[i].type << 24));
        }

        entry_.comment = info.comment;

        for (auto const& member : info.members)
        {
            if (member.guid != leaderGuid)
                entry_.members.emplace_back(member.guid, member.level);
        }

        entries.push_back(std::move(entry_));
    }

    MsgLookingForGroup managedPacket(type, entry, std::move(entries));
    player->getSession()->sendManagedPacket(managedPacket);
}
