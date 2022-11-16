/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CalendarMgr.hpp"
#include "Server/MainServerDefines.h"
#include "Database/Database.h"
#include "Logging/Logger.hpp"

CalendarMgr& CalendarMgr::getInstance()
{
    static CalendarMgr mInstance;
    return mInstance;
}

void CalendarMgr::loadFromDB()
{
    sLogger.info("CalendarMgr : Start loading calendar_events");
    {
        const char* loadCalendarEvents = "SELECT entry, creator, title, description, type, dungeon, date, flags FROM calendar_events";
        bool success = false;
        QueryResult* result = CharacterDatabase.Query(&success, loadCalendarEvents);
        if (!success)
        {
            sLogger.failure("Query failed: %s", loadCalendarEvents);
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
                auto type = static_cast<CalendarEventType>(fields[4].GetUInt32());
                uint32_t dungeon = fields[5].GetUInt32();
                time_t date = fields[6].GetUInt32();
                uint32_t flags = fields[7].GetUInt32();

                CalendarEvent* calendarEvent = new CalendarEvent(static_cast<uint32_t>(entry), creator, title, description, type, dungeon, date, flags);
                m_events.insert(calendarEvent);

                sLogger.debug("Title %s loaded", calendarEvent->m_title.c_str()); // remove me ;-)

                ++count;
            }
            while (result->NextRow());
            delete result;

            sLogger.info("CalendarMgr : %u calendar events loaded from table calendar_events", count);
        }
    }

    sLogger.info("CalendarMgr : Start loading calendar_invites");
    {
        const char* loadCalendarInvites = "SELECT `id`, `event`, `invitee`, `sender`, `status`, `statustime`, `rank`, `text` FROM `calendar_invites`";
        bool success = false;
        QueryResult* result = CharacterDatabase.Query(&success, loadCalendarInvites);
        if (!success)
        {
            sLogger.failure("Query failed: %s", loadCalendarInvites);
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
                auto status = static_cast<CalendarInviteStatus>(fields[4].GetUInt32());
                time_t statustime = fields[5].GetUInt32();
                uint32_t rank = fields[6].GetUInt32();
                std::string text = fields[7].GetString();

                auto invite = new CalendarInvite(invite_id, event, invitee, sender, status, statustime, rank, text);
                m_invites[event].push_back(invite);

                ++count;
            }
            while (result->NextRow());
            delete result;
            sLogger.info("CalendarMgr : Loaded %u calendar invites", count);
        }
    }
}
