/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Maps/InstanceMgr.hpp"
#include "Utilities/Util.hpp"
#include "Management/CalendarMgr.hpp"
#include "Server/Packets/SmsgCalendarSendNumPending.h"
#include "Server/Packets/SmsgCalendarRaidLockoutRemoved.h"
#include "Server/Packets/SmsgCalendarRaidLockoutUpdated.h"
#include "Server/Packets/SmsgCalendarSendCalendar.h"
#include "Server/Packets/SmsgCalendarSendEvent.h"
#include "Server/Packets/SmsgCalendarCommandResult.h"
#include "Server/Packets/CmsgCalendarGetEvent.h"
#include "Server/Packets/CmsgCalendarAddEvent.h"
#include "Server/Packets/CmsgCalendarUpdateEvent.h"
#include "Server/Packets/CmsgCalendarRemoveEvent.h"

using namespace AscEmu::Packets;

void WorldSession::handleCalendarGetCalendar(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_GET_CALENDAR.");

    const uint32_t lowGuid = _player->getGuidLow();

    std::vector<CalendarInviteEntry> invites;
    for (const auto* invite : sCalendarMgr.getPlayerInvites(lowGuid))
        invites.push_back({ invite, sCalendarMgr.getEvent(invite->m_event) });

    SmsgCalendarSendCalendar managedPacket(std::move(invites), sCalendarMgr.getPlayerEvents(lowGuid), _player->getGuildId());
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarComplain(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarComplain Not handled.");
}

void WorldSession::handleCalendarGetNumPending(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_GET_NUM_PENDING.");

    SmsgCalendarSendNumPending managedPacket(sCalendarMgr.getPlayerNumPending(_player->getGuidLow()));
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarAddEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_ADD_EVENT.");

    CmsgCalendarAddEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const uint32_t lowGuid = _player->getGuidLow();

    auto newEvent = std::make_unique<CalendarEvent>(sCalendarMgr.generateEventId(), lowGuid, srlPacket.title, srlPacket.description,
        static_cast<CalendarEventType>(srlPacket.type), static_cast<uint32_t>(srlPacket.dungeonId), time_t(srlPacket.eventPackedTime), srlPacket.flags);

    if (newEvent->isGuildEvent())
        newEvent->m_guildId = _player->getGuildId();

    CalendarEvent* calendarEvent = sCalendarMgr.addEvent(std::move(newEvent));

    for (const auto& invitee : srlPacket.invitees)
    {
        auto invite = std::make_unique<CalendarInvite>(sCalendarMgr.generateInviteId(), calendarEvent->m_entry, invitee.guid.getCounter(),
            lowGuid, static_cast<CalendarInviteStatus>(invitee.status), time_t(946684800), invitee.rank, "");
        sCalendarMgr.addInvite(calendarEvent->m_entry, std::move(invite));
    }

    SmsgCalendarSendEvent managedPacket(CALENDAR_SENDTYPE_ADD, calendarEvent, sCalendarMgr.getEventInvites(calendarEvent->m_entry));
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarGetEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_GET_EVENT.");

    CmsgCalendarGetEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId))
    {
        SmsgCalendarSendEvent managedPacket(CALENDAR_SENDTYPE_GET, calendarEvent, sCalendarMgr.getEventInvites(srlPacket.eventId));
        sendManagedPacket(managedPacket);
    }
    else
    {
        SmsgCalendarCommandResult managedPacket(CALENDAR_ERROR_EVENT_INVALID);
        sendManagedPacket(managedPacket);
    }
}

void WorldSession::handleCalendarGuildFilter(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarGuildFilter Not handled.");
}

void WorldSession::handleCalendarArenaTeam(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarArenaTeam Not handled.");
}

void WorldSession::handleCalendarUpdateEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_UPDATE_EVENT.");

    CmsgCalendarUpdateEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        SmsgCalendarCommandResult managedPacket(CALENDAR_ERROR_EVENT_INVALID);
        sendManagedPacket(managedPacket);
        return;
    }

    if (calendarEvent->m_creator != _player->getGuidLow())
    {
        SmsgCalendarCommandResult managedPacket(CALENDAR_ERROR_PERMISSIONS);
        sendManagedPacket(managedPacket);
        return;
    }

    calendarEvent->m_type = static_cast<CalendarEventType>(srlPacket.type);
    calendarEvent->m_flags = srlPacket.flags;
    calendarEvent->m_date = time_t(srlPacket.eventPackedTime);
    calendarEvent->m_dungeon = srlPacket.dungeonId;
    calendarEvent->m_title = srlPacket.title;
    calendarEvent->m_description = srlPacket.description;

    sCalendarMgr.updateEvent(calendarEvent);

    SmsgCalendarCommandResult managedPacket(CALENDAR_OK);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarRemoveEvent(WorldPacket& recvPacket)
{
    sLogger.debugOpcode("Received CMSG_CALENDAR_REMOVE_EVENT.");

    CmsgCalendarRemoveEvent srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    CalendarEvent* calendarEvent = sCalendarMgr.getEvent(srlPacket.eventId);
    if (calendarEvent == nullptr)
    {
        SmsgCalendarCommandResult managedPacket(CALENDAR_ERROR_EVENT_INVALID);
        sendManagedPacket(managedPacket);
        return;
    }

    if (calendarEvent->m_creator != _player->getGuidLow())
    {
        SmsgCalendarCommandResult managedPacket(CALENDAR_ERROR_DELETE_CREATOR_FAILED);
        sendManagedPacket(managedPacket);
        return;
    }

    sCalendarMgr.removeEvent(srlPacket.eventId);

    SmsgCalendarCommandResult managedPacket(CALENDAR_OK);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleCalendarCopyEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarCopyEvent Not handled.");
}

void WorldSession::handleCalendarEventInvite(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarEventInvite Not handled.");
}

void WorldSession::handleCalendarEventRsvp(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarEventRsvp Not handled.");
}

void WorldSession::handleCalendarEventRemoveInvite(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarEventRemoveInvite Not handled.");
}

void WorldSession::handleCalendarEventStatus(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarEventStatus Not handled.");
}

void WorldSession::handleCalendarEventModeratorStatus(WorldPacket& /*recvPacket*/)
{
    sLogger.debugOpcode("HandleCalendarEventModeratorStatus Not handled.");
}

void WorldSession::sendCalendarRaidLockout(InstanceSaved const* save, bool add)
{
    sLogger.debugOpcode("SMSG_CALENDAR_RAID_LOCKOUT_ADDED/REMOVED.");
    const auto now = Util::getTimeNow();
    time_t currTime = now;

    SmsgCalendarRaidLockoutRemoved managedPacket(add, currTime, uint32_t(save->getMapId()), uint32_t(save->getDifficulty()), uint32_t(save->getResetTime() - currTime), uint64_t(save->getInstanceId()));
    sendManagedPacket(managedPacket);
}

void WorldSession::sendCalendarRaidLockoutUpdated(InstanceSaved const* save)
{
    if (!save)
        return;

    sLogger.debugOpcode("SMSG_CALENDAR_RAID_LOCKOUT_UPDATED [{}] Map: {}, Difficulty {}.", _player->getGuid(), save->getMapId(), save->getDifficulty());

    const auto now = Util::getTimeNow();
    time_t currTime = now;

    SmsgCalendarRaidLockoutUpdated managedPacket(currTime, uint32_t(save->getMapId()), uint32_t(save->getDifficulty()), uint32_t(save->getResetTime() - currTime));
    sendManagedPacket(managedPacket);
}

