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
#include "Management/ArenaTeam.h"
#include "Server/MainServerDefines.h"
#include "Server/WorldSession.h"
#include "Chat/ChatHandler.hpp"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/SmsgArenaTeamStats.h"

using namespace AscEmu::Packets;

static const uint32 TeamCountToId[6] =
{
    0,                      // 0
    0,                      // 1
    0,                      // 2
    ARENA_TEAM_TYPE_2V2,    // 3
    ARENA_TEAM_TYPE_3V3,    // 4
    ARENA_TEAM_TYPE_5V5,    // 5
};

static const uint32 IdToTeamCount[6] =
{
    3,
    4,
    5,
    0,
    0,
    0,
};

ArenaTeam::ArenaTeam(uint8_t Type, uint32 Id)
{
    m_id = Id;
    m_type = Type;
    AllocateSlots(Type);
    m_leader = 0;
    m_emblem = { 0, 0, 0, 0, 0 };
    m_stats = { 1500, 0, 0, 0, 0, 0 };
}

ArenaTeam::ArenaTeam(Field* f)
{
    uint32 z = 0;

    m_id = f[z++].GetUInt32();
    m_type = f[z++].GetUInt8();
    m_leader = f[z++].GetUInt32();
    m_name = f[z++].GetString();
    m_emblem.emblemStyle = f[z++].GetUInt32();
    m_emblem.emblemColour = f[z++].GetUInt32();
    m_emblem.borderStyle = f[z++].GetUInt32();
    m_emblem.borderColour = f[z++].GetUInt32();
    m_emblem.backgroundColour = f[z++].GetUInt32();
    m_stats.rating = f[z++].GetUInt32();

    AllocateSlots(m_type);

    m_stats.played_week = 0;
    m_stats.played_season = 0;
    m_stats.won_season = 0;
    m_stats.won_week = 0;
    m_stats.ranking = 0;

    if (sscanf(f[z++].GetString(), "%u %u %u %u", &m_stats.played_week, &m_stats.won_week, &m_stats.played_season, &m_stats.won_season) != 3)
        return;

    m_stats.ranking = f[z++].GetUInt32();
    for (uint32 i = 0; i < m_slots; ++i)
    {
        uint32 guid;
        const char* data = f[z++].GetString();
        int ret = sscanf(data, "%u %u %u %u %u %u", &guid, &m_members[i].Played_ThisWeek, &m_members[i].Won_ThisWeek,
                         &m_members[i].Played_ThisSeason, &m_members[i].Won_ThisSeason, &m_members[i].PersonalRating);
        if (ret >= 5)
        {
            m_members[i].Info = sObjectMgr.GetPlayerInfo(guid);
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

void ArenaTeam::SendPacket(WorldPacket* data)
{
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        const auto playerInfo = m_members[i].Info;
        if (playerInfo && playerInfo->m_loggedInPlayer)
            playerInfo->m_loggedInPlayer->GetSession()->SendPacket(data);
    }
}

void ArenaTeam::Destroy()
{
    std::vector<PlayerInfo*> tokill;
    tokill.reserve(m_memberCount);

    char buffer[1024];
    snprintf(buffer, 1024, "The arena team, '%s', disbanded.", m_name.c_str());

    SendPacket(SmsgMessageChat(SystemMessagePacket(buffer)).serialise().get());

    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info)
            tokill.push_back(m_members[i].Info);
    }

    for (auto itr = tokill.begin(); itr != tokill.end(); ++itr)
        RemoveMember(*itr);

    sObjectMgr.RemoveArenaTeam(this);
    delete this;
}

bool ArenaTeam::AddMember(PlayerInfo* info)
{
    Player* player = info->m_loggedInPlayer;
    if (m_memberCount >= m_slots)
        return false;

    memset(&m_members[m_memberCount], 0, sizeof(ArenaTeamMember));
    m_members[m_memberCount].PersonalRating = 1500;
    m_members[m_memberCount++].Info = info;
    SaveToDB();

#if VERSION_STRING != Classic
    if (player)
    {
        player->setArenaTeamId(m_type, m_id);
        player->setArenaTeamMemberRank(m_type, 1);

        player->m_arenaTeams[m_type] = this;

        player->GetSession()->SystemMessage("You are now a member of the arena team, '%s'.", m_name.c_str());
    }
#endif

    return true;
}

bool ArenaTeam::RemoveMember(PlayerInfo* info)
{
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info == info)
        {
            /* memcpy all the blocks in front of him back (so we only loop O(members) instead of O(slots) */
            for (uint32 j = (i + 1); j < m_memberCount; ++j)
                memcpy(&m_members[j - 1], &m_members[j], sizeof(ArenaTeamMember));

            --m_memberCount;
            SaveToDB();

#if VERSION_STRING != Classic
            if (info->m_loggedInPlayer)
            {
                info->m_loggedInPlayer->setArenaTeamId(m_type, 0);
                info->m_loggedInPlayer->m_arenaTeams[m_type] = nullptr;
            }
#endif

            return true;
        }
    }

    return false;
}

//MIT
std::vector<ArenaTeamPacketList> ArenaTeam::getRoosterMembers() const
{
    std::vector<ArenaTeamPacketList> arenaTeamList{};

    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        if (const auto playerInfo = m_members[i].Info)
        {
            ArenaTeamPacketList arenaTeamListMember;

            arenaTeamListMember.guid = playerInfo->guid;
            arenaTeamListMember.isLoggedIn = playerInfo->m_loggedInPlayer ? 1 : 0;
            arenaTeamListMember.name = playerInfo->name;
            arenaTeamListMember.isLeader = m_members[i].Info->guid == m_leader ? 0 : 1;
            arenaTeamListMember.lastLevel = playerInfo->lastLevel;
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

void ArenaTeam::SaveToDB()
{
    std::stringstream ss;
    uint32 i;

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

bool ArenaTeam::isMember(uint32_t guid) const
{
    for (uint32_t i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info && m_members[i].Info->guid == guid)
            return true;
    }
    return false;
}

void ArenaTeam::SetLeader(PlayerInfo* info)
{
    char buffer[1024];
    snprintf(buffer, 1024, "%s is now the captain of the arena team, '%s'.", info->name, m_name.c_str());

    SendPacket(SmsgMessageChat(SystemMessagePacket(buffer)).serialise().get());

    const uint32 old_leader = m_leader;
    m_leader = info->guid;
    /* set the fields */
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
#if VERSION_STRING != Classic
        if (m_members[i].Info == info)        /* new leader */
        {
            if (m_members[i].Info->m_loggedInPlayer)
                m_members[i].Info->m_loggedInPlayer->setArenaTeamMemberRank(m_type, 0);
        }
        else if (m_members[i].Info->guid == old_leader)
        {
            if (m_members[i].Info->m_loggedInPlayer)
                m_members[i].Info->m_loggedInPlayer->setArenaTeamMemberRank(m_type, 1);
        }
#endif
    }
}

ArenaTeamMember* ArenaTeam::GetMember(PlayerInfo* info)
{
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info == info)
            return &m_members[i];
    }
    return nullptr;
}

ArenaTeamMember* ArenaTeam::GetMemberByGuid(uint32 guid)
{
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info && m_members[i].Info->guid == guid)
            return &m_members[i];
    }
    return nullptr;
}
