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

#ifndef ARENATEAMS_H
#define ARENATEAMS_H

#include "Objects/Units/Players/Player.h"

//MIT
class CachedCharacterInfo;

struct ArenaTeamPacketList
{
    uint64_t guid;
    uint8_t isLoggedIn;
    std::string name;
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
    CachedCharacterInfo* Info;
    uint32 Played_ThisWeek;
    uint32 Won_ThisWeek;
    uint32 Played_ThisSeason;
    uint32 Won_ThisSeason;
    uint32 PersonalRating;
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

        ArenaTeam(uint8_t Type, uint32 Id);
        ArenaTeam(Field* f);
        ~ArenaTeam()
        {
            delete [] m_members;
        }

    
        void SaveToDB();

        void Destroy();

        void SendPacket(WorldPacket* data);
        

        bool AddMember(CachedCharacterInfo* info);
        bool RemoveMember(CachedCharacterInfo* info);

        bool isMember(uint32_t guid) const;

        void SetLeader(CachedCharacterInfo* info);
        ArenaTeamMember* GetMember(CachedCharacterInfo* info);
        ArenaTeamMember* GetMemberByGuid(uint32 guid);

        uint32 GetPlayersPerTeam()
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

        //MIT
        std::vector<ArenaTeamPacketList> getRoosterMembers() const;

        uint32 m_id;
        uint8_t m_type;
        uint32 m_leader;
        uint32 m_slots;
        std::string m_name;
        uint32 m_memberCount;
        ArenaTeamMember* m_members;

        ArenaTeamEmblem m_emblem;

        ArenaTeamStats m_stats;

    private:

        void AllocateSlots(uint16 Type)
        {
            uint32 Slots = 0;
            if (Type == ARENA_TEAM_TYPE_2V2)
                Slots = 4;
            else if (Type == ARENA_TEAM_TYPE_3V3)
                Slots = 6;
            else if (Type == ARENA_TEAM_TYPE_5V5)
                Slots = 10;

            if (Slots == 0)
            {
                sLogger.failure("Tried to allocate Slot 0 in ArenaTeam::AllocateSlots");
                return;
            }

            m_members = new ArenaTeamMember[Slots];
            memset(m_members, 0, sizeof(ArenaTeamMember)*Slots);
            m_slots = Slots;
            m_memberCount = 0;
        }
};

#endif //ARENATEAMS_H
