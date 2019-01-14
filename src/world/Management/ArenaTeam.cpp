/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

ArenaTeam::ArenaTeam(uint16 Type, uint32 Id)
{
    m_id = Id;
    m_type = Type;
    AllocateSlots(Type);
    m_leader = 0;
    m_emblemStyle = 0;
    m_emblemColour = 0;
    m_borderColour = 0;
    m_borderStyle = 0;
    m_backgroundColour = 0;
    m_stat_rating = 1500;
    m_stat_gamesplayedweek = 0;
    m_stat_gamesplayedseason = 0;
    m_stat_gameswonseason = 0;
    m_stat_gameswonweek = 0;
    m_stat_ranking = 0;
}

ArenaTeam::ArenaTeam(Field* f)
{
    uint32 z = 0, i, guid;
    const char* data;
    int ret;

    m_id = f[z++].GetUInt32();
    m_type = f[z++].GetUInt16();
    m_leader = f[z++].GetUInt32();
    m_name = f[z++].GetString();
    m_emblemStyle = f[z++].GetUInt32();
    m_emblemColour = f[z++].GetUInt32();
    m_borderStyle = f[z++].GetUInt32();
    m_borderColour = f[z++].GetUInt32();
    m_backgroundColour = f[z++].GetUInt32();
    m_stat_rating = f[z++].GetUInt32();
    AllocateSlots(m_type);

    m_stat_gamesplayedweek = 0;
    m_stat_gamesplayedseason = 0;
    m_stat_gameswonseason = 0;
    m_stat_gameswonweek = 0;
    m_stat_ranking = 0;
    if (sscanf(f[z++].GetString(), "%u %u %u %u", &m_stat_gamesplayedweek, &m_stat_gameswonweek, &m_stat_gamesplayedseason, &m_stat_gameswonseason) != 3)
        return;

    m_stat_ranking = f[z++].GetUInt32();
    for (i = 0; i < m_slots; ++i)
    {
        data = f[z++].GetString();
        ret = sscanf(data, "%u %u %u %u %u %u", &guid, &m_members[i].Played_ThisWeek, &m_members[i].Won_ThisWeek,
                     &m_members[i].Played_ThisSeason, &m_members[i].Won_ThisSeason, &m_members[i].PersonalRating);
        if (ret >= 5)
        {
            m_members[i].Info = objmgr.GetPlayerInfo(guid);
            if (m_members[i].Info)
                ++m_memberCount;
            if (ret == 5)
            {
                // In case PersonalRating is not in the string just set the rating to the team rating
                m_members[i].PersonalRating = m_stat_rating;
            }
        }
        else
            m_members[i].Info = NULL;
    }
}

void ArenaTeam::SendPacket(WorldPacket* data)
{
    PlayerInfo* info;
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        info = m_members[i].Info;
        if (info && info->m_loggedInPlayer)
            info->m_loggedInPlayer->GetSession()->SendPacket(data);
    }
}

void ArenaTeam::Destroy()
{
    char buffer[1024];
    WorldPacket* data;
    std::vector<PlayerInfo*> tokill;
    uint32 i;
    tokill.reserve(m_memberCount);
    snprintf(buffer, 1024, "The arena team, '%s', disbanded.", m_name.c_str());
    data = sChatHandler.FillSystemMessageData(buffer);
    SendPacket(data);
    delete data;

    for (i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info)
            tokill.push_back(m_members[i].Info);
    }

    for (std::vector<PlayerInfo*>::iterator itr = tokill.begin(); itr != tokill.end(); ++itr)
    {
        RemoveMember(*itr);
    }

    objmgr.RemoveArenaTeam(this);
    delete this;
}

bool ArenaTeam::AddMember(PlayerInfo* info)
{
    Player* plr = info->m_loggedInPlayer;
    if (m_memberCount >= m_slots)
    {
        return false;
    }

    memset(&m_members[m_memberCount], 0, sizeof(ArenaTeamMember));
    m_members[m_memberCount].PersonalRating = 1500;
    m_members[m_memberCount++].Info = info;
    SaveToDB();

#if VERSION_STRING != Classic
    if (plr)
    {
        uint16_t base_field = (m_type * 7) + PLAYER_FIELD_ARENA_TEAM_INFO_1_1;
        plr->setUInt32Value(base_field, m_id);
        plr->setUInt32Value(base_field + 1, m_leader);

        plr->m_arenaTeams[m_type] = this;
        plr->GetSession()->SystemMessage("You are now a member of the arena team, '%s'.", m_name.c_str());
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
            {
                memcpy(&m_members[j - 1], &m_members[j], sizeof(ArenaTeamMember));
            }

            --m_memberCount;
            SaveToDB();

#if VERSION_STRING != Classic
            if (info->m_loggedInPlayer)
            {
                info->m_loggedInPlayer->setUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (m_type * 7), 0);
                info->m_loggedInPlayer->m_arenaTeams[m_type] = 0;
            }
#endif

            return true;
        }
    }

    return false;
}

void ArenaTeam::Stat(WorldPacket& data)
{
    data.Initialize(SMSG_ARENA_TEAM_STATS);
    data << m_id;
    data << m_stat_rating;
    data << m_stat_gamesplayedweek;
    data << m_stat_gameswonweek;
    data << m_stat_gamesplayedseason;
    data << m_stat_gameswonseason;
    data << m_stat_ranking;
}

void ArenaTeam::Query(WorldPacket& data)
{
    data.Initialize(SMSG_ARENA_TEAM_QUERY_RESPONSE);
    data << m_id;
    data << m_name;
    data << GetPlayersPerTeam();
    data << m_emblemColour;
    data << m_emblemStyle;
    data << m_borderColour;
    data << m_borderStyle;
    data << m_backgroundColour;
}

void ArenaTeam::Roster(WorldPacket& data)
{
    data.Initialize(SMSG_ARENA_TEAM_ROSTER);
    data.reserve(m_memberCount * 81 + 9);
    data << m_id;
    data << uint8(0); // 3.0.8
    data << m_memberCount;
    data << GetPlayersPerTeam();

    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        PlayerInfo* info = m_members[i].Info;
        ///\todo figure out why this became null
        if (info != NULL)
        {
            data << uint64(info->guid);
            data << uint8((info->m_loggedInPlayer != NULL) ? 1 : 0);
            data << info->name;
            data << uint32(m_members[i].Info->guid == m_leader ? 0 : 1);  // Unk
            data << uint8(info->lastLevel);
            data << uint8(info->cl);
            data << m_members[i].Played_ThisWeek;
            data << m_members[i].Won_ThisWeek;
            data << m_members[i].Played_ThisSeason;
            data << m_members[i].Won_ThisSeason;
            data << m_members[i].PersonalRating;
        }
    }
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
        << m_emblemStyle << ","
        << m_emblemColour << ","
        << m_borderStyle << ","
        << m_borderColour << ","
        << m_backgroundColour << ","
        << m_stat_rating << ",'"
        << m_stat_gamesplayedweek << " " << m_stat_gameswonweek << " "
        << m_stat_gamesplayedseason << " " << m_stat_gameswonseason << "',"
        << m_stat_ranking;

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

bool ArenaTeam::HasMember(uint32 guid)
{
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info && m_members[i].Info->guid == guid)
            return true;
    }
    return false;
}

void ArenaTeam::SetLeader(PlayerInfo* info)
{
    uint32 old_leader = m_leader;
    char buffer[1024];
    WorldPacket* data;
    snprintf(buffer, 1024, "%s is now the captain of the arena team, '%s'.", info->name, m_name.c_str());
    data = sChatHandler.FillSystemMessageData(buffer);
    m_leader = info->guid;
    SendPacket(data);
    delete data;

    /* set the fields */
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
#if VERSION_STRING != Classic
        if (m_members[i].Info == info)        /* new leader */
        {
            if (m_members[i].Info->m_loggedInPlayer)
                m_members[i].Info->m_loggedInPlayer->setUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (m_type * 7) + 1, 0);
        }
        else if (m_members[i].Info->guid == old_leader)
        {
            if (m_members[i].Info->m_loggedInPlayer)
                m_members[i].Info->m_loggedInPlayer->setUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (m_type * 7) + 1, 1);
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
    return NULL;
}

ArenaTeamMember* ArenaTeam::GetMemberByGuid(uint32 guid)
{
    for (uint32 i = 0; i < m_memberCount; ++i)
    {
        if (m_members[i].Info && m_members[i].Info->guid == guid)
            return &m_members[i];
    }
    return NULL;
}
