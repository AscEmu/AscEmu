/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/ArenaTeam.hpp"

#include <sstream>

#include "Logging/Logger.hpp"
#include "Server/WorldSession.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Packets/SmsgMessageChat.h"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Server/DatabaseDefinition.hpp"

using namespace AscEmu::Packets;

static const uint32_t TeamCountToId[6] =
{
    0,                      // 0
    0,                      // 1
    0,                      // 2
    ARENA_TEAM_TYPE_2V2,    // 3
    ARENA_TEAM_TYPE_3V3,    // 4
    ARENA_TEAM_TYPE_5V5,    // 5
};

static const uint32_t IdToTeamCount[6] =
{
    3,
    4,
    5,
    0,
    0,
    0,
};

ArenaTeam::ArenaTeam(uint8_t type, uint32_t Id)
{
    m_id = Id;
    m_type = type;
    _allocateSlots(type);
    m_leader = 0;
    m_emblem = { 0, 0, 0, 0, 0 };
    m_stats = { 1500, 0, 0, 0, 0, 0 };
}

ArenaTeam::ArenaTeam(Field* field)
{
    uint32_t z = 0;

    m_id = field[z++].asUint32();
    m_type = field[z++].asUint8();
    m_leader = field[z++].asUint32();
    m_name = field[z++].asCString();
    m_emblem.emblemStyle = field[z++].asUint32();
    m_emblem.emblemColour = field[z++].asUint32();
    m_emblem.borderStyle = field[z++].asUint32();
    m_emblem.borderColour = field[z++].asUint32();
    m_emblem.backgroundColour = field[z++].asUint32();
    m_stats.rating = field[z++].asUint32();

    _allocateSlots(m_type);

    m_stats.played_week = 0;
    m_stats.played_season = 0;
    m_stats.won_season = 0;
    m_stats.won_week = 0;
    m_stats.ranking = 0;

    if (sscanf(field[z++].asCString(), "%u %u %u %u", &m_stats.played_week, &m_stats.won_week, &m_stats.played_season, &m_stats.won_season) != 3)
        return;

    m_stats.ranking = field[z++].asUint32();
    for (uint32_t i = 0; i < m_slots; ++i)
    {
        uint32_t guid;
        const char* data = field[z++].asCString();
        int ret = sscanf(data, "%u %u %u %u %u %u", &guid, &m_members[i].Played_ThisWeek, &m_members[i].Won_ThisWeek,
                         &m_members[i].Played_ThisSeason, &m_members[i].Won_ThisSeason, &m_members[i].PersonalRating);
        if (ret >= 5)
        {
            m_members[i].Info = sObjectMgr.getCachedCharacterInfo(guid);
            if (m_members[i].Info)
                ++m_memberCount;

            if (ret == 5)
            {
                // In case PersonalRating is not in the string just set the rating to the team rating
                m_members[i].PersonalRating = m_stats.rating;
            }
        }
        else
        {
            m_members[i].Info = nullptr;
        }
    }
}

ArenaTeam::~ArenaTeam() = default;

void ArenaTeam::saveToDB()
{
    std::stringstream ss;
    uint32_t i;

    ss << "DELETE FROM arenateams WHERE id = ";
    ss << m_id;
    ss << ";";

    CharacterDatabase.ExecuteNA(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO arenateams VALUES("
        << m_id << ","
        << m_type << ","
        << m_leader << ",'"
        << m_name << "',"
        << m_emblem.emblemStyle << ","
        << m_emblem.emblemColour << ","
        << m_emblem.borderStyle << ","
        << m_emblem.borderColour << ","
        << m_emblem.backgroundColour << ","
        << m_stats.rating << ",'"
        << m_stats.played_week << " " << m_stats.won_week << " "
        << m_stats.played_season << " " << m_stats.won_season << "',"
        << m_stats.ranking;

    for (i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info)
        {
            ss << ",'" << m_members[i].Info->guid << " " << m_members[i].Played_ThisWeek << " "
                << m_members[i].Won_ThisWeek << " " << m_members[i].Played_ThisSeason << " "
                << m_members[i].Won_ThisSeason << " " << m_members[i].PersonalRating << "'";
        }
        else
        {
            ss << ",'0 0 0 0 0 0'";
        }
    }

    for (; i < 10; ++i)
    {
        ss << ",'0 0 0 0 0 0'";
    }

    ss << ")";
    CharacterDatabase.Execute(ss.str().c_str());
}

void ArenaTeam::destroy()
{
    std::vector<CachedCharacterInfo const*> toDestroyMembers;
    toDestroyMembers.reserve(m_memberCount);

    char buffer[1024];
    snprintf(buffer, 1024, "The arena team, '%s', disbanded.", m_name.c_str());

    sendPacket(SmsgMessageChat(SystemMessagePacket(buffer)).serialise().get());

    for (uint32_t i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info)
            toDestroyMembers.push_back(m_members[i].Info);
    }

    for (auto& itr : toDestroyMembers)
        removeMember(itr);

    // TODO: arena team is not removed from db? -Appled
    sObjectMgr.removeArenaTeam(this);
}

void ArenaTeam::sendPacket(WorldPacket* data) const
{
    for (uint32_t i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info)
        {
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(m_members[i].Info->guid))
                loggedInPlayer->getSession()->SendPacket(data);
        }
    }
}

ArenaTeamMember* ArenaTeam::getMember(CachedCharacterInfo const* cachedCharInfo) const
{
    for (uint32_t i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info == cachedCharInfo)
            return &m_members[i];
    }
    return nullptr;
}

ArenaTeamMember* ArenaTeam::getMemberByGuid(uint32_t lowGuid) const
{
    for (uint32_t i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info && m_members[i].Info->guid == lowGuid)
            return &m_members[i];
    }
    return nullptr;
}

bool ArenaTeam::addMember(CachedCharacterInfo const* cachedCharInfo)
{
    if (!cachedCharInfo)
        return false;

    if (Player* loggedInPlayer = sObjectMgr.getPlayer(cachedCharInfo->guid))
    {
        if (m_memberCount >= m_slots)
            return false;

        memset(&m_members[m_memberCount], 0, sizeof(ArenaTeamMember));
        m_members[m_memberCount].PersonalRating = 1500;
        m_members[m_memberCount++].Info = cachedCharInfo;
        saveToDB();

#if VERSION_STRING != Classic
        loggedInPlayer->setArenaTeamId(m_type, m_id);
        loggedInPlayer->setArenaTeamMemberRank(m_type, 1);
#endif
    }

    return true;
}

bool ArenaTeam::removeMember(CachedCharacterInfo const* cachedCharInfo)
{
    if (!cachedCharInfo)
        return false;

    for (uint32_t i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info == cachedCharInfo)
        {
            /* memcpy all the blocks in front of him back (so we only loop O(members) instead of O(slots) */
            for (uint32_t j = (i + 1); j < m_memberCount; ++j)
                memcpy(&m_members[j - 1], &m_members[j], sizeof(ArenaTeamMember));

            --m_memberCount;
            saveToDB();

#if VERSION_STRING != Classic
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(cachedCharInfo->guid))
            {
                loggedInPlayer->setArenaTeamId(m_type, 0);
                loggedInPlayer->setArenaTeam(m_type, nullptr);
            }
#endif

            return true;
        }
    }

    return false;
}

uint32_t ArenaTeam::getPlayersPerTeam() const
{
    switch (m_type)
    {
    case ARENA_TEAM_TYPE_2V2:
        return 2;

    case ARENA_TEAM_TYPE_3V3:
        return 3;

    case ARENA_TEAM_TYPE_5V5:
        return 5;

    default:
        return 2;
    }
}

bool ArenaTeam::isMember(uint32_t lowGuid) const
{
    for (uint32_t i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info && m_members[i].Info->guid == lowGuid)
            return true;
    }
    return false;
}

void ArenaTeam::setLeader(CachedCharacterInfo const* cachedCharInfo)
{
    if (cachedCharInfo)
    {
        char buffer[1024];
        snprintf(buffer, 1024, "%s is now the captain of the arena team, '%s'.", cachedCharInfo->name.c_str(), m_name.c_str());

        sendPacket(SmsgMessageChat(SystemMessagePacket(buffer)).serialise().get());

        const uint32_t old_leader = m_leader;
        m_leader = cachedCharInfo->guid;

#if VERSION_STRING != Classic
        for (uint32_t i = 0; i < m_memberCount; ++i)
        {
            if (m_members[i].Info)
            {
                if (Player* loggedInPlayer = sObjectMgr.getPlayer(m_members[i].Info->guid))
                {
                    if (m_members[i].Info == cachedCharInfo)
                        loggedInPlayer->setArenaTeamMemberRank(m_type, 0);
                    else if (m_members[i].Info->guid == old_leader)
                        loggedInPlayer->setArenaTeamMemberRank(m_type, 1);
                }
            }
        }
#endif
    }
}

std::vector<ArenaTeamPacketList> ArenaTeam::getRoosterMembers() const
{
    std::vector<ArenaTeamPacketList> arenaTeamList{};

    for (uint32_t i = 0; i < m_memberCount; ++i)
    {
        if (const auto playerInfo = m_members[i].Info)
        {
            ArenaTeamPacketList arenaTeamListMember;

            arenaTeamListMember.guid = playerInfo->guid;
            arenaTeamListMember.isLoggedIn = sObjectMgr.getPlayer(playerInfo->guid) ? 1 : 0;
            arenaTeamListMember.name = playerInfo->name;
            arenaTeamListMember.isLeader = m_members[i].Info->guid == m_leader ? 0 : 1;
            arenaTeamListMember.lastLevel = static_cast<uint8_t>(playerInfo->lastLevel);
            arenaTeamListMember.cl = playerInfo->cl;
            arenaTeamListMember.playedWeek = m_members[i].Played_ThisWeek;
            arenaTeamListMember.wonWeek = m_members[i].Won_ThisWeek;
            arenaTeamListMember.playedSeason = m_members[i].Played_ThisSeason;
            arenaTeamListMember.wonSeason = m_members[i].Won_ThisSeason;
            arenaTeamListMember.rating = m_members[i].PersonalRating;

            arenaTeamList.push_back(arenaTeamListMember);
        }
    }

    return arenaTeamList;
}

void ArenaTeam::_allocateSlots(uint16_t type)
{
    uint32_t Slots = 0;
    if (type == ARENA_TEAM_TYPE_2V2)
        Slots = 4;
    else if (type == ARENA_TEAM_TYPE_3V3)
        Slots = 6;
    else if (type == ARENA_TEAM_TYPE_5V5)
        Slots = 10;

    if (Slots == 0)
    {
        sLogger.failure("Tried to allocate Slot 0 in ArenaTeam::AllocateSlots");
        return;
    }

    m_members = std::make_unique<ArenaTeamMember[]>(Slots);
    std::fill(m_members.get(), m_members.get() + Slots, ArenaTeamMember());
    m_slots = Slots;
    m_memberCount = 0;
}
