/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Players/PlayerDefines.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

// TBC-only "Looking For Group" matchmaking (predates the WotLK+ Dungeon Finder, which is
// implemented separately in Management/LFG/LFGMgr.hpp - do not confuse the two or reuse their
// names). Players queue at a Meeting Stone as either "looking for group" (LFG, up to 3 dungeon
// slots) or "looking for more" (LFM, a group leader with an opening); with auto-join/auto-fill
// enabled the server matches compatible LFG/LFM entries automatically.
//
// Verified independently for both Classic and TBC (not assumed identical): Classic's meeting
// stones use a completely different, simpler group+area-id queueing mechanic with no per-player
// LFG/LFM slot selection at all - it is not implemented in AscEmu. This whole system is
// registered for TBC only (WorldSession.cpp, #if VERSION_STRING == TBC).
static constexpr uint8_t MEETINGSTONE_MAX_LFG_SLOTS = 3;

enum class MeetingStoneLfgType : uint16_t
{
    None           = 0,
    Dungeon        = 1,
    Raid           = 2,
    Quest          = 3,
    Zone           = 4,
    HeroicDungeon  = 5
};

// Mirrors real client's SMSG_MEETINGSTONE_SETQUEUE status byte
enum class MeetingStoneQueueStatus : uint8_t
{
    LeftQueue        = 0,
    InQueue          = 1,
    Unknown          = 2,
    OtherMemberLeft  = 3,
    KickedFromQueue  = 4,
    JoinedGroup      = 5
};

struct MeetingStoneSlot
{
    uint16_t entry = 0;
    uint16_t type = 0;

    [[nodiscard]] bool empty() const { return entry == 0 || type == 0; }
    void clear() { entry = 0; }
    bool set(uint16_t _entry, uint16_t _type) { entry = _entry; type = _type; return !empty(); }
    [[nodiscard]] bool is(uint16_t _entry, uint16_t _type) const { return entry == _entry && type == _type; }

    [[nodiscard]] bool isAuto() const
    {
        return entry != 0 && (type == static_cast<uint16_t>(MeetingStoneLfgType::Dungeon) || type == static_cast<uint16_t>(MeetingStoneLfgType::HeroicDungeon));
    }
};

struct MeetingStoneGroupMember
{
    uint64_t guid = 0;
    uint32_t level = 0;
};

// One entry per queued player, keyed by that player's own guid (a solo LFG player is its own
// "leader" entry until matched into a real group, exactly like the reference implementation).
struct MeetingStonePlayerInfo
{
    void clear()
    {
        more.clear();
        for (auto& slot : group)
            slot.clear();
    }

    [[nodiscard]] bool isAutoFill() const { return more.isAuto(); }

    [[nodiscard]] bool isAutoJoin() const
    {
        for (auto const& slot : group)
            if (slot.isAuto())
                return true;
        return false;
    }
    [[nodiscard]] bool isEmpty() const { return !isLFM() && !isLFG(); }

    [[nodiscard]] bool isLFG() const
    {
        for (auto const& slot : group)
            if (!slot.empty())
                return true;
        return false;
    }

    [[nodiscard]] bool isLFG(uint16_t entry, uint16_t type, bool autoOnly) const
    {
        for (auto const& slot : group)
            if (slot.is(entry, type) && (!autoOnly || slot.isAuto()))
                return true;
        return false;
    }

    [[nodiscard]] bool isLFG(MeetingStonePlayerInfo const& other, bool autoOnly) const { return isLFG(other.more.entry, other.more.type, autoOnly); }
    [[nodiscard]] bool isLFM() const { return !more.empty(); }
    [[nodiscard]] bool isLFM(uint16_t entry, uint16_t type) const { return more.is(entry, type); }

    MeetingStoneSlot group[MEETINGSTONE_MAX_LFG_SLOTS];
    MeetingStoneSlot more;
    std::string comment;
    PlayerTeam team = TEAM_ALLIANCE;
    bool isLeader = true;
    bool full = false;
    uint32_t level = 0;
    uint32_t zoneId = 0;
    bool status = false;
    uint64_t leaderGuid = 0;

    bool autoFill = false;
    bool autoJoin = false;

    bool pendingTransfer = false;
    uint64_t pendingLeaderGuid = 0;
    uint32_t pendingEntry = 0;

    std::vector<MeetingStoneGroupMember> members;
    std::vector<uint64_t> pendingMembers;
};

class MeetingStoneQueue
{
private:
    MeetingStoneQueue() = default;
    ~MeetingStoneQueue() = default;

public:
    static MeetingStoneQueue& getInstance();
    void initialize();
    void finalize();

    MeetingStoneQueue(MeetingStoneQueue&&) = delete;
    MeetingStoneQueue(MeetingStoneQueue const&) = delete;
    MeetingStoneQueue& operator=(MeetingStoneQueue&&) = delete;
    MeetingStoneQueue& operator=(MeetingStoneQueue const&) = delete;

    void setComment(uint64_t playerGuid, std::string const& comment);
    void setAutoFill(uint64_t playerGuid, bool state);
    void setAutoJoin(uint64_t playerGuid, bool state);

    void startLookingForMore(MeetingStonePlayerInfo info, uint64_t invokerGuid);
    void stopLookingForMore(uint64_t playerGuid);

    void startLookingForGroup(MeetingStonePlayerInfo info, uint64_t invokerGuid);
    void stopLookingForGroup(uint64_t leaderGuid, uint64_t playerGuid);
    void setLfgSlot(uint64_t leaderGuid, uint8_t slot, uint16_t entry, uint16_t type);
    void setLfmData(uint64_t leaderGuid, uint16_t entry, uint16_t type);

    void handlePendingJoin(uint64_t playerGuid);
    void handleDeclinePendingJoin(uint64_t playerGuid);

    void sendListQueryResponse(uint64_t playerGuid, PlayerTeam playerTeam, uint32_t type, uint32_t entry) const;

    MeetingStonePlayerInfo const* getInfo(uint64_t guid) const;

private:
    void tryJoin(uint64_t playerGuid);
    void tryFill(uint64_t leaderGuid);
    bool addMember(MeetingStonePlayerInfo& leaderInfo, MeetingStonePlayerInfo& joinerInfo, uint32_t entry);
    void removePendingJoin(uint64_t leaderGuid, uint64_t playerGuid);
    void pendingJoinSuccess(uint64_t leaderGuid, uint64_t playerGuid, bool full);

    void sendLFGUpdate(uint64_t leaderGuid, uint64_t playerGuid) const;
    void groupUpdate(uint64_t leaderGuid, bool completed);
    bool groupUpdateQueueStatus(uint64_t leaderGuid);
    void groupUpdateUI(uint64_t leaderGuid, bool completed);

    std::map<uint64_t, MeetingStonePlayerInfo> m_queuedPlayers;
};

#define sMeetingStoneQueue MeetingStoneQueue::getInstance()
