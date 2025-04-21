/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "CommonTypes.hpp"
#include <Utilities/utf8.hpp>

class CachedCharacterInfo;
class Field;
class WorldPacket;

struct ArenaTeamPacketList
{
    uint64_t guid;
    uint8_t isLoggedIn;
    utf8_string name;
    uint32_t isLeader;
    uint8_t lastLevel;
    uint8_t cl;

    uint32_t playedWeek;
    uint32_t wonWeek;
    uint32_t playedSeason;
    uint32_t wonSeason;
    uint32_t rating;
};

struct ArenaTeamMember
{
    CachedCharacterInfo const* Info;
    uint32_t Played_ThisWeek;
    uint32_t Won_ThisWeek;
    uint32_t Played_ThisSeason;
    uint32_t Won_ThisSeason;
    uint32_t PersonalRating;
};

struct ArenaTeamEmblem
{
    uint32_t emblemStyle;
    uint32_t emblemColour;
    uint32_t borderStyle;
    uint32_t borderColour;
    uint32_t backgroundColour;
};

struct ArenaTeamStats
{
    uint32_t rating;
    uint32_t played_week;
    uint32_t won_week;
    uint32_t played_season;
    uint32_t won_season;
    uint32_t ranking;
};

class SERVER_DECL ArenaTeam
{
public:
    ArenaTeam(uint8_t type, uint32_t Id);
    ArenaTeam(Field* field);
    ~ArenaTeam();

    void saveToDB();

    void destroy();
    void sendPacket(WorldPacket* data) const;

    ArenaTeamMember* getMember(CachedCharacterInfo const* cachedCharInfo) const;
    ArenaTeamMember* getMemberByGuid(uint32_t lowGuid) const;
    bool addMember(CachedCharacterInfo const* cachedCharInfo);
    bool removeMember(CachedCharacterInfo const* cachedCharInfo);

    uint32_t getPlayersPerTeam() const;

    bool isMember(uint32_t lowGuid) const;

    void setLeader(CachedCharacterInfo const* cachedCharInfo);

    std::vector<ArenaTeamPacketList> getRoosterMembers() const;

    uint32_t m_id;
    uint8_t m_type;
    uint32_t m_leader;
    uint32_t m_slots;
    std::string m_name;
    uint32_t m_memberCount;

    std::unique_ptr<ArenaTeamMember[]> m_members;
    ArenaTeamEmblem m_emblem;
    ArenaTeamStats m_stats;

private:
    void _allocateSlots(uint16_t type);
};
