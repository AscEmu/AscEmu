/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <string>

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
    CalendarEvent(uint32_t p_entry = 0, uint32_t p_creator = 0, std::string p_title = "", std::string p_description = "", CalendarEventType p_type = CALENDAR_TYPE_RAID, uint32_t p_dungeon = 0, time_t p_date = 0, uint32_t p_flags = 0) :
        m_entry(p_entry), m_creator(p_creator), m_title(std::move(p_title)), m_description(std::move(p_description)), m_type(p_type), m_dungeon(p_dungeon), m_date(p_date), m_flags(p_flags)
    {
    }

    ~CalendarEvent() = default;

    uint32_t m_entry;               // entry of the calendar event (unique)
    uint32_t m_creator;             // id of the character
    std::string m_title;            // title of the calendar event
    std::string m_description;      // description of the event
    CalendarEventType m_type;       // the calendar type
    uint32_t m_dungeon;             // the dungeon id
    time_t m_date;                  // the date
    uint32_t m_flags;               // the flag
};

struct CalendarInvite
{
    CalendarInvite(uint32_t p_invite_id = 0, uint32_t p_event = 0, uint32_t p_invitee = 0, uint32_t p_sender = 0, CalendarInviteStatus p_status = CALENDAR_STATUS_REMOVED, time_t p_statustime = 0, uint32_t p_rank = 0, std::string p_text = "") :
        m_inviteId(p_invite_id), m_event(p_event), m_invitee(p_invitee), m_sender(p_sender), m_status(p_status), m_statusTime(p_statustime), m_rank(p_rank), m_text(std::move(p_text))
    {
    }

    ~CalendarInvite() = default;

    uint32_t m_inviteId;            // entry of the calendar event (unique)
    uint32_t m_event;               // id of the character
    uint32_t m_invitee;
    uint32_t m_sender;
    CalendarInviteStatus m_status;
    time_t m_statusTime;
    uint32_t m_rank;
    std::string m_text;
};

typedef std::vector<std::unique_ptr<CalendarInvite>> CalendarInviteStore;
typedef std::set<std::unique_ptr<CalendarEvent>> CalendarEventStore;
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

    CalendarEventStore m_events;
    CalendarEventInviteStore m_invites;

    void loadFromDB();
};

#define sCalendarMgr CalendarMgr::getInstance()
