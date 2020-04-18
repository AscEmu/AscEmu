/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#ifndef ARENATEAMS_H
#define ARENATEAMS_H

#include "Units/Players/Player.h"

struct ArenaTeamMember
{
    PlayerInfo* Info;
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
        void AllocateSlots(uint16_t Type)
        {
            uint32_t Slots = 0;
            if (Type == ARENA_TEAM_TYPE_2V2)
                Slots = 4;
            else if (Type == ARENA_TEAM_TYPE_3V3)
                Slots = 6;
            else if (Type == ARENA_TEAM_TYPE_5V5)
                Slots = 10;
            ARCEMU_ASSERT(Slots > 0);
            m_members = new ArenaTeamMember[Slots];
            memset(m_members, 0, sizeof(ArenaTeamMember)*Slots);
            m_slots = Slots;
            m_memberCount = 0;
        }

    public:

        uint32_t m_id;
        uint16_t m_type;
        uint32_t m_leader;
        uint32_t m_slots;
        std::string m_name;
        uint32_t m_memberCount;
        ArenaTeamMember* m_members;

        ArenaTeamEmblem m_emblem;

        ArenaTeamStats m_stats;

        ArenaTeam(uint16_t Type, uint32_t Id);
        ArenaTeam(Field* f);
        ~ArenaTeam()
        {
            delete [] m_members;
        }

        void SendPacket(WorldPacket* data);
        void Roster(WorldPacket& data);
        void Inspect(WorldPacket& data);
        void Destroy();
        void SaveToDB();

        bool AddMember(PlayerInfo* info);
        bool RemoveMember(PlayerInfo* info);
        bool HasMember(uint32_t guid);
        void SetLeader(PlayerInfo* info);
        ArenaTeamMember* GetMember(PlayerInfo* info);
        ArenaTeamMember* GetMemberByGuid(uint32_t guid);

        uint32_t GetPlayersPerTeam()
        {
            switch(m_type)
            {
                case ARENA_TEAM_TYPE_2V2:
                    return 2;

                case ARENA_TEAM_TYPE_3V3:
                    return 3;

                case ARENA_TEAM_TYPE_5V5:
                    return 5;
            }

            // never reached
            return 2;
        }
};

#endif //ARENATEAMS_H
