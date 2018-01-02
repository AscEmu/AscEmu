/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "Singleton.h"
#include <map>

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
    CalendarEvent(uint32 p_entry = 0, uint32 p_creator = 0, std::string p_title = "", std::string p_description = "", CalendarEventType p_type = CALENDAR_TYPE_RAID, uint32 p_dungeon = 0, time_t p_date = 0, uint32 p_flags = 0)
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

    uint32 entry;               // entry of the calendar event (unique)
    uint32 creator;             // id of the character
    std::string title;          // title of the calendar event
    std::string description;    // description of the event
    CalendarEventType type;     // the calendar type
    uint32 dungeon;             // the dungeon id
    time_t date;                // the date
    uint32 flags;               // the flag
};

struct CalendarInvite
{
    CalendarInvite(uint32 p_invite_id = 0, uint32 p_event = 0, uint32 p_invitee = 0, uint32 p_sender = 0, CalendarInviteStatus p_status = CALENDAR_STATUS_REMOVED, time_t p_statustime = 0, uint32 p_rank = 0, std::string p_text = "")
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

    uint32 invite_id;               // entry of the calendar event (unique)
    uint32 event;             // id of the character
    uint32 invitee;
    uint32 sender;
    CalendarInviteStatus status;
    time_t statustime;
    uint32 rank;
    std::string text;

};
typedef std::vector<CalendarInvite*> CalendarInviteStore;
typedef std::set<CalendarEvent*> CalendarEventStore;
typedef std::map<uint64 /* eventId */, CalendarInviteStore > CalendarEventInviteStore;

class CalendarMgr : public Singleton< CalendarMgr >
{
    public:

        CalendarMgr();
        ~CalendarMgr();

        CalendarEventStore _events;
        CalendarEventInviteStore _invites;

        void LoadFromDB();

};

#define sCalendarMgr CalendarMgr::getSingleton()

#endif // CALENDARMGR_H
