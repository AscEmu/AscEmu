/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#ifndef CALENDARMGR_H
#define CALENDARMGR_H

#include <vector>
#include <set>
#include <map>
#include <string>

#include "CommonTypes.hpp"

enum CalendarFlags
{
    CALENDAR_FLAG_ALL_ALLOWED = 0x001,
    CALENDAR_FLAG_INVITES_LOCKED = 0x010,
    CALENDAR_FLAG_WITHOUT_INVITES = 0x040,
    CALENDAR_FLAG_GUILD_EVENT = 0x400
};

enum CalendarEventType
{
    CALENDAR_TYPE_RAID = 0,
    CALENDAR_TYPE_DUNGEON = 1,
    CALENDAR_TYPE_PVP = 2,
    CALENDAR_TYPE_MEETING = 3,
    CALENDAR_TYPE_OTHER = 4
};

enum CalendarSendEventType
{
    CALENDAR_SENDTYPE_GET = 0,
    CALENDAR_SENDTYPE_ADD = 1,
    CALENDAR_SENDTYPE_COPY = 2
};

enum CalendarInviteStatus
{
    CALENDAR_STATUS_INVITED = 0,
    CALENDAR_STATUS_ACCEPTED = 1,
    CALENDAR_STATUS_DECLINED = 2,
    CALENDAR_STATUS_CONFIRMED = 3,
    CALENDAR_STATUS_OUT = 4,
    CALENDAR_STATUS_STANDBY = 5,
    CALENDAR_STATUS_SIGNED_UP = 6,
    CALENDAR_STATUS_NOT_SIGNED_UP = 7,
    CALENDAR_STATUS_TENTATIVE = 8,
    CALENDAR_STATUS_REMOVED = 9     // correct name?
};

struct CalendarEvent
{ 
    CalendarEvent(uint32_t p_entry = 0, uint32_t p_creator = 0, std::string p_title = "", std::string p_description = "", CalendarEventType p_type = CALENDAR_TYPE_RAID, uint32_t p_dungeon = 0, time_t p_date = 0, uint32_t p_flags = 0)
    {
        entry = p_entry;
        creator = p_creator;
        title = p_title;
        description = p_description;
        type = p_type;
        dungeon = p_dungeon;
        date = p_date;
        flags = p_flags;
    }

    ~CalendarEvent(){};

    uint32_t entry;               // entry of the calendar event (unique)
    uint32_t creator;             // id of the character
    std::string title;            // title of the calendar event
    std::string description;      // description of the event
    CalendarEventType type;       // the calendar type
    uint32_t dungeon;             // the dungeon id
    time_t date;                  // the date
    uint32_t flags;               // the flag
};

struct CalendarInvite
{
    CalendarInvite(uint32_t p_invite_id = 0, uint32_t p_event = 0, uint32_t p_invitee = 0, uint32_t p_sender = 0, CalendarInviteStatus p_status = CALENDAR_STATUS_REMOVED, time_t p_statustime = 0, uint32_t p_rank = 0, std::string p_text = "")
    {
        invite_id = p_invite_id;
        event = p_event;
        invitee = p_invitee;
        sender = p_sender;
        status = p_status;
        statustime = p_statustime;
        rank = p_rank;
        text = p_text;
    }

    ~CalendarInvite(){};

    uint32_t invite_id;           // entry of the calendar event (unique)
    uint32_t event;               // id of the character
    uint32_t invitee;
    uint32_t sender;
    CalendarInviteStatus status;
    time_t statustime;
    uint32_t rank;
    std::string text;

};
typedef std::vector<CalendarInvite*> CalendarInviteStore;
typedef std::set<CalendarEvent*> CalendarEventStore;
typedef std::map<uint64_t /* eventId */, CalendarInviteStore > CalendarEventInviteStore;

class CalendarMgr
{
    private:

        CalendarMgr() = default;
        ~CalendarMgr() = default;

    public:

        static CalendarMgr& getInstance();

        CalendarMgr(CalendarMgr&&) = delete;
        CalendarMgr(CalendarMgr const&) = delete;
        CalendarMgr& operator=(CalendarMgr&&) = delete;
        CalendarMgr& operator=(CalendarMgr const&) = delete;

        CalendarEventStore _events;
        CalendarEventInviteStore _invites;

        void LoadFromDB();
};

#define sCalendarMgr CalendarMgr::getInstance()

#endif // CALENDARMGR_H
