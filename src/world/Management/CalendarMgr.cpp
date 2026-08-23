/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CalendarMgr.hpp"
#include "Database/Database.hpp"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"

#if VERSION_STRING > TBC // sch: added in the 3.0.2 content patch

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
        auto result = CharacterDatabase.query(&success, loadCalendarEvents);
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
                Field* fields = result->fetch();

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

                if (static_cast<uint32_t>(entry) >= m_nextEventId)
                    m_nextEventId = static_cast<uint32_t>(entry) + 1;

                ++count;
            }
            while (result->nextRow());

            sLogger.info("CalendarMgr : {} calendar events loaded from table calendar_events", count);
        }
    }

    sLogger.info("CalendarMgr : Start loading calendar_invites");
    {
        const char* loadCalendarInvites = "SELECT `id`, `event`, `invitee`, `sender`, `status`, `statustime`, `rank`, `text` FROM `calendar_invites`";
        bool success = false;
        auto result = CharacterDatabase.query(&success, loadCalendarInvites);
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
                Field* fields = result->fetch();

                uint32_t invite_id = fields[0].asUint32();       // unique invite id
                uint32_t event = fields[1].asUint32();           // entry of the calendar event
                uint32_t invitee = fields[2].asUint32();         // player id
                uint32_t sender = fields[3].asUint32();          // player id
                auto status = static_cast<CalendarInviteStatus>(fields[4].asUint32());
                time_t statustime = fields[5].asUint32();
                uint32_t rank = fields[6].asUint32();
                std::string text = fields[7].asCString();

                m_invites[event].emplace_back(std::make_unique<CalendarInvite>(invite_id, event, invitee, sender, status, statustime, rank, text));

                if (invite_id >= m_nextInviteId)
                    m_nextInviteId = invite_id + 1;

                ++count;
            }
            while (result->nextRow());
            sLogger.info("CalendarMgr : Loaded {} calendar invites", count);
        }
    }
}

CalendarEvent* CalendarMgr::getEvent(uint32_t eventId)
{
    for (const auto& calendarEvent : m_events)
        if (calendarEvent->m_entry == eventId)
            return calendarEvent.get();

    return nullptr;
}

std::vector<CalendarEvent*> CalendarMgr::getPlayerEvents(uint32_t playerLowGuid)
{
    std::vector<CalendarEvent*> result;
    for (const auto& calendarEvent : m_events)
    {
        if (calendarEvent->m_creator == playerLowGuid)
        {
            result.push_back(calendarEvent.get());
            continue;
        }

        for (const auto& invite : m_invites[calendarEvent->m_entry])
        {
            if (invite->m_invitee == playerLowGuid)
            {
                result.push_back(calendarEvent.get());
                break;
            }
        }
    }

    return result;
}

CalendarInvite* CalendarMgr::getInvite(uint32_t eventId, uint32_t inviteId)
{
    for (const auto& invite : m_invites[eventId])
        if (invite->m_inviteId == inviteId)
            return invite.get();

    return nullptr;
}

std::vector<CalendarInvite*> CalendarMgr::getEventInvites(uint32_t eventId)
{
    std::vector<CalendarInvite*> result;
    for (const auto& invite : m_invites[eventId])
        result.push_back(invite.get());

    return result;
}

std::vector<CalendarInvite*> CalendarMgr::getPlayerInvites(uint32_t playerLowGuid)
{
    std::vector<CalendarInvite*> result;
    for (const auto& [eventId, inviteStore] : m_invites)
    {
        for (const auto& invite : inviteStore)
        {
            if (invite->m_invitee == playerLowGuid)
                result.push_back(invite.get());
        }
    }

    return result;
}

uint32_t CalendarMgr::getPlayerNumPending(uint32_t playerLowGuid)
{
    uint32_t pending = 0;
    for (const auto& invite : getPlayerInvites(playerLowGuid))
    {
        if (invite->m_status == CALENDAR_STATUS_INVITED || invite->m_status == CALENDAR_STATUS_TENTATIVE || invite->m_status == CALENDAR_STATUS_NOT_SIGNED_UP)
            ++pending;
    }

    return pending;
}

uint32_t CalendarMgr::generateEventId()
{
    return m_nextEventId++;
}

uint32_t CalendarMgr::generateInviteId()
{
    return m_nextInviteId++;
}

CalendarEvent* CalendarMgr::addEvent(std::unique_ptr<CalendarEvent> calendarEvent)
{
    CalendarEvent* rawPtr = calendarEvent.get();
    m_events.emplace(std::move(calendarEvent));

    CharacterDatabase.execute("INSERT INTO calendar_events (entry, creator, title, description, type, dungeon, date, flags) VALUES (%u, %u, '%s', '%s', %u, %d, %u, %u)",
        rawPtr->m_entry, rawPtr->m_creator, CharacterDatabase.escapeString(rawPtr->m_title).c_str(), CharacterDatabase.escapeString(rawPtr->m_description).c_str(),
        static_cast<uint32_t>(rawPtr->m_type), rawPtr->m_dungeon, static_cast<uint32_t>(rawPtr->m_date), rawPtr->m_flags);

    return rawPtr;
}

void CalendarMgr::updateEvent(CalendarEvent* calendarEvent)
{
    if (calendarEvent == nullptr)
        return;

    CharacterDatabase.execute("UPDATE calendar_events SET title = '%s', description = '%s', type = %u, dungeon = %d, date = %u, flags = %u WHERE entry = %u",
        CharacterDatabase.escapeString(calendarEvent->m_title).c_str(), CharacterDatabase.escapeString(calendarEvent->m_description).c_str(),
        static_cast<uint32_t>(calendarEvent->m_type), calendarEvent->m_dungeon, static_cast<uint32_t>(calendarEvent->m_date), calendarEvent->m_flags, calendarEvent->m_entry);
}

void CalendarMgr::removeEvent(uint32_t eventId)
{
    for (auto& invite : m_invites[eventId])
        CharacterDatabase.execute("DELETE FROM calendar_invites WHERE id = %u", invite->m_inviteId);

    m_invites.erase(eventId);

    for (auto itr = m_events.begin(); itr != m_events.end(); ++itr)
    {
        if ((*itr)->m_entry == eventId)
        {
            m_events.erase(itr);
            break;
        }
    }

    CharacterDatabase.execute("DELETE FROM calendar_events WHERE entry = %u", eventId);
}

CalendarInvite* CalendarMgr::addInvite(uint32_t eventId, std::unique_ptr<CalendarInvite> invite)
{
    CalendarInvite* rawPtr = invite.get();
    m_invites[eventId].emplace_back(std::move(invite));

    CharacterDatabase.execute("INSERT INTO calendar_invites (id, event, invitee, sender, status, statustime, rank, text) VALUES (%u, %u, %u, %u, %u, %u, %u, '%s')",
        rawPtr->m_inviteId, rawPtr->m_event, rawPtr->m_invitee, rawPtr->m_sender, static_cast<uint32_t>(rawPtr->m_status),
        static_cast<uint32_t>(rawPtr->m_statusTime), rawPtr->m_rank, CharacterDatabase.escapeString(rawPtr->m_text).c_str());

    return rawPtr;
}

void CalendarMgr::updateInvite(CalendarInvite* invite)
{
    if (invite == nullptr)
        return;

    CharacterDatabase.execute("UPDATE calendar_invites SET status = %u, statustime = %u, rank = %u WHERE id = %u",
        static_cast<uint32_t>(invite->m_status), static_cast<uint32_t>(invite->m_statusTime), invite->m_rank, invite->m_inviteId);
}

void CalendarMgr::removeInvite(uint32_t eventId, uint32_t inviteId)
{
    auto& inviteStore = m_invites[eventId];
    for (auto itr = inviteStore.begin(); itr != inviteStore.end(); ++itr)
    {
        if ((*itr)->m_inviteId == inviteId)
        {
            inviteStore.erase(itr);
            break;
        }
    }

    CharacterDatabase.execute("DELETE FROM calendar_invites WHERE id = %u", inviteId);
}
#endif