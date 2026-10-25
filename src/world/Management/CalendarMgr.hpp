/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#include <cstdint>
#include <ctime>
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
    CALENDAR_TYPE_OTHER = 4,
    CALENDAR_TYPE_HEROIC = 5        // Cata and MoP
};

enum CalendarModerationRank : uint8_t
{
    CALENDAR_RANK_PLAYER = 0,
    CALENDAR_RANK_MODERATOR = 1,
    CALENDAR_RANK_OWNER = 2
};

enum CalendarLimits : uint32_t
{
    CALENDAR_MAX_EVENTS = 30,       // own events without guild
    CALENDAR_MAX_GUILD_EVENTS = 100,
    CALENDAR_MAX_INVITES = 100
};

// the answer time of an invite nobody answered yet: 2000-01-01
constexpr time_t CALENDAR_DEFAULT_RESPONSE_TIME = 946684800;

enum CalendarSendEventType
{
    CALENDAR_SENDTYPE_GET = 0,
    CALENDAR_SENDTYPE_ADD = 1,
    CALENDAR_SENDTYPE_COPY = 2
};

enum CalendarError
{
    CALENDAR_OK                                 = 0,
    CALENDAR_ERROR_GUILD_EVENTS_EXCEEDED        = 1,
    CALENDAR_ERROR_EVENTS_EXCEEDED              = 2,
    CALENDAR_ERROR_SELF_INVITES_EXCEEDED        = 3,
    CALENDAR_ERROR_OTHER_INVITES_EXCEEDED       = 4,    // with the name
    CALENDAR_ERROR_PERMISSIONS                  = 5,
    CALENDAR_ERROR_EVENT_INVALID                = 6,
    CALENDAR_ERROR_NOT_INVITED                  = 7,
    CALENDAR_ERROR_INTERNAL                     = 8,
    CALENDAR_ERROR_GUILD_PLAYER_NOT_IN_GUILD    = 9,
    CALENDAR_ERROR_ALREADY_INVITED_TO_EVENT_S   = 10,   // with the name
    CALENDAR_ERROR_PLAYER_NOT_FOUND             = 11,
    CALENDAR_ERROR_NOT_ALLIED                   = 12,
    CALENDAR_ERROR_IGNORING_YOU_S               = 13,   // with the name
    CALENDAR_ERROR_INVITES_EXCEEDED             = 14,
    CALENDAR_ERROR_INVALID_DATE                 = 16,
    CALENDAR_ERROR_INVALID_TIME                 = 17,
    CALENDAR_ERROR_NEEDS_TITLE                  = 19,
    CALENDAR_ERROR_EVENT_PASSED                 = 20,
    CALENDAR_ERROR_EVENT_LOCKED                 = 21,
    CALENDAR_ERROR_DELETE_CREATOR_FAILED        = 22,
    CALENDAR_ERROR_SYSTEM_DISABLED              = 24,
    CALENDAR_ERROR_RESTRICTED_ACCOUNT           = 25,
    CALENDAR_ERROR_ARENA_EVENTS_EXCEEDED        = 26,
    CALENDAR_ERROR_RESTRICTED_LEVEL             = 27,
    CALENDAR_ERROR_USER_SQUELCHED               = 28,
    CALENDAR_ERROR_NO_INVITE                    = 29,
    CALENDAR_ERROR_EVENT_WRONG_SERVER           = 36,
    CALENDAR_ERROR_INVITE_WRONG_SERVER          = 37,
    CALENDAR_ERROR_NO_GUILD_INVITES             = 38,
    CALENDAR_ERROR_INVALID_SIGNUP               = 39,
    CALENDAR_ERROR_NO_MODERATOR                 = 40
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
    CalendarEvent(uint32_t p_entry = 0, uint32_t p_creator = 0, std::string p_title = "", std::string p_description = "", CalendarEventType p_type = CALENDAR_TYPE_RAID, uint32_t p_dungeon = 0, time_t p_date = 0, uint32_t p_flags = 0, uint32_t p_guildId = 0) :
        m_entry(p_entry), m_creator(p_creator), m_title(std::move(p_title)), m_description(std::move(p_description)), m_type(p_type), m_dungeon(p_dungeon), m_date(p_date), m_flags(p_flags), m_guildId(p_guildId)
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
    uint32_t m_guildId;             // guild id, only meaningful when CALENDAR_FLAG_GUILD_EVENT is set

    bool isGuildEvent() const { return (m_flags & CALENDAR_FLAG_GUILD_EVENT) != 0; }
    // a guild announcement has no invites, every member of the guild sees it
    bool isGuildAnnouncement() const { return (m_flags & CALENDAR_FLAG_WITHOUT_INVITES) != 0; }
    bool isLocked() const { return (m_flags & CALENDAR_FLAG_INVITES_LOCKED) != 0; }
    bool belongsToGuild() const { return isGuildEvent() || isGuildAnnouncement(); }
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

    CalendarEvent* getEvent(uint32_t eventId);
    std::vector<CalendarEvent*> getPlayerEvents(uint32_t playerLowGuid);

    CalendarInvite* getInvite(uint32_t eventId, uint32_t inviteId);
    CalendarInvite* getInviteOfPlayer(uint32_t eventId, uint32_t playerLowGuid);
    std::vector<CalendarInvite*> getEventInvites(uint32_t eventId);
    std::vector<CalendarInvite*> getPlayerInvites(uint32_t playerLowGuid);
    uint32_t getPlayerNumPending(uint32_t playerLowGuid);

    // the guild of a guild event or announcement, taken from the owner when it is not known yet
    uint32_t getEventGuildId(CalendarEvent* calendarEvent);
    // the events of the calendar of a player: own, invited and those of the guild
    std::vector<CalendarEvent*> getCalendarEvents(uint32_t playerLowGuid, uint32_t guildId);
    uint32_t getOwnedEventCount(uint32_t ownerLowGuid);
    uint32_t getGuildEventCount(uint32_t guildId);

    CalendarEvent* addEvent(std::unique_ptr<CalendarEvent> calendarEvent);
    void updateEvent(CalendarEvent* calendarEvent);
    void removeEvent(uint32_t eventId);

    CalendarInvite* addInvite(uint32_t eventId, std::unique_ptr<CalendarInvite> invite);
    void updateInvite(CalendarInvite* invite);
    void removeInvite(uint32_t eventId, uint32_t inviteId);

    uint32_t generateEventId();
    uint32_t generateInviteId();

private:
    uint32_t m_nextEventId = 1;
    uint32_t m_nextInviteId = 1;
};

#define sCalendarMgr CalendarMgr::getInstance()

