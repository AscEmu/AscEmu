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

#include "StdAfx.h"
#include "CalendarMgr.h"
#include "Server/MainServerDefines.h"
#include "Log.hpp"
#include "Database/Database.h"

CalendarMgr& CalendarMgr::getInstance()
{
    static CalendarMgr mInstance;
    return mInstance;
}

void CalendarMgr::LoadFromDB()
{
    LogNotice("CalendarMgr : Start loading calendar_events");
    {
        const char* loadCalendarEvents = "SELECT entry, creator, title, description, type, dungeon, date, flags FROM calendar_events";
        bool success = false;
        QueryResult* result = CharacterDatabase.Query(&success, loadCalendarEvents);
        if (!success)
        {
            LOG_ERROR("Query failed: %s", loadCalendarEvents);
            return;
        }
        if (result)
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint64_t entry = fields[0].GetUInt32();
                uint32_t creator = fields[1].GetUInt32();
                std::string title = fields[2].GetString();
                std::string description = fields[3].GetString();
                CalendarEventType type = CalendarEventType(fields[4].GetUInt32());
                uint32_t dungeon = fields[5].GetUInt32();
                time_t date = fields[6].GetUInt32();
                uint32_t flags = fields[7].GetUInt32();

                CalendarEvent* calendarEvent = new CalendarEvent(static_cast<uint32_t>(entry), creator, title, description, type, dungeon, time_t(date), flags);
                _events.insert(calendarEvent);

                LOG_DEBUG("Title %s loaded", calendarEvent->title.c_str()); // remove me ;-)

                ++count;
            }
            while (result->NextRow());
            delete result;

            LogDetail("CalendarMgr : %u calendar events loaded from table calendar_events", count);
        }
    }

    LogNotice("CalendarMgr : Start loading calendar_invites");
    {
        const char* loadCalendarInvites = "SELECT `id`, `event`, `invitee`, `sender`, `status`, `statustime`, `rank`, `text` FROM `calendar_invites`";
        bool success = false;
        QueryResult* result = CharacterDatabase.Query(&success, loadCalendarInvites);
        if (!success)
        {
            LOG_ERROR("Query failed: %s", loadCalendarInvites);
            return;
        }
        if (result)
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32_t invite_id = fields[0].GetUInt32();       // unique invite id
                uint32_t event = fields[1].GetUInt32();           // entry of the calendar event
                uint32_t invitee = fields[2].GetUInt32();         // player id
                uint32_t sender = fields[3].GetUInt32();          // player id
                CalendarInviteStatus status = CalendarInviteStatus(fields[4].GetUInt32());
                time_t statustime = fields[5].GetUInt32();
                uint32_t rank = fields[6].GetUInt32();
                std::string text = fields[7].GetString();

                CalendarInvite* invite = new CalendarInvite(invite_id, event, invitee, sender, status, time_t(statustime), rank, text);
                _invites[event].push_back(invite);

                ++count;
            }
            while (result->NextRow());
            delete result;
            LogDetail("CalendarMgr : Loaded %u calendar invites", count);
        }
    }
}
