/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CalendarMgr.hpp"
#include "Database/Database.h"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"

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
        auto result = CharacterDatabase.Query(&success, loadCalendarEvents);
        if (!success)
        {
            sLogger.failure("Query failed: {}", loadCalendarEvents);
            return;
        }
        if (result)
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint64_t entry = fields[0].asUint32();
                uint32_t creator = fields[1].asUint32();
                std::string title = fields[2].asCString();
                std::string description = fields[3].asCString();
                auto type = static_cast<CalendarEventType>(fields[4].asUint32());
                uint32_t dungeon = fields[5].asUint32();
                time_t date = fields[6].asUint32();
                uint32_t flags = fields[7].asUint32();

                const auto [eventItr, _] = m_events.emplace(std::make_unique<CalendarEvent>(static_cast<uint32_t>(entry), creator, title, description, type, dungeon, date, flags));

                sLogger.debug("Title {} loaded", eventItr->get()->m_title); // remove me ;-)

                ++count;
            }
            while (result->NextRow());

            sLogger.info("CalendarMgr : {} calendar events loaded from table calendar_events", count);
        }
    }

    sLogger.info("CalendarMgr : Start loading calendar_invites");
    {
        const char* loadCalendarInvites = "SELECT `id`, `event`, `invitee`, `sender`, `status`, `statustime`, `rank`, `text` FROM `calendar_invites`";
        bool success = false;
        auto result = CharacterDatabase.Query(&success, loadCalendarInvites);
        if (!success)
        {
            sLogger.failure("Query failed: {}", loadCalendarInvites);
            return;
        }
        if (result)
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();

                uint32_t invite_id = fields[0].asUint32();       // unique invite id
                uint32_t event = fields[1].asUint32();           // entry of the calendar event
                uint32_t invitee = fields[2].asUint32();         // player id
                uint32_t sender = fields[3].asUint32();          // player id
                auto status = static_cast<CalendarInviteStatus>(fields[4].asUint32());
                time_t statustime = fields[5].asUint32();
                uint32_t rank = fields[6].asUint32();
                std::string text = fields[7].asCString();

                m_invites[event].emplace_back(std::make_unique<CalendarInvite>(invite_id, event, invitee, sender, status, statustime, rank, text));

                ++count;
            }
            while (result->NextRow());
            sLogger.info("CalendarMgr : Loaded {} calendar invites", count);
        }
    }
}
