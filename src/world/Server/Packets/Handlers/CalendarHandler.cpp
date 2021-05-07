/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "Log.hpp"
#include "Units/Players/Player.h"

#if VERSION_STRING > TBC

// \todo CalendarHandler
void WorldSession::handleCalendarGetCalendar(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarGetCalendar Not handled");

    /* Get all events for the player */
    uint32_t guid = static_cast<uint32_t>(_player->getGuid());
    sLogger.debug("HandleCalendarGetCalendar CMSG_CALENDAR_GET_CALENDAR for guid %u", guid);

}

void WorldSession::handleCalendarComplain(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarComplain Not handled");
}

void WorldSession::handleCalendarGetNumPending(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarGetNumPending Not handled");

    WorldPacket data(SMSG_CALENDAR_SEND_NUM_PENDING, 4);
#if VERSION_STRING >= Cata
    data << uint32_t(0);  // num pending
#endif
    SendPacket(&data);
}

void WorldSession::handleCalendarAddEvent(WorldPacket& recvPacket)
{
    // Create an Event and save it to char db 
    sLogger.debug("HandleCalendarAddEvent Not handled");

    uint32_t guid = static_cast<uint32_t>(_player->getGuid());

    std::string title;
    std::string description;
    uint8_t type;
    uint8_t repeatable;
    uint32_t maxInvites;
    int32_t dungeonId;
    uint32_t eventPackedTime;
    uint32_t unkPackedTime;
    uint32_t flags;

    recvPacket >> title;
    recvPacket >> description;
    recvPacket >> type;
    recvPacket >> repeatable;
    recvPacket >> maxInvites;
    recvPacket >> dungeonId;
    recvPacket.ReadPackedTime(eventPackedTime);
    recvPacket.ReadPackedTime(unkPackedTime);
    recvPacket >> flags;

    // \todo save it to db
    sLogger.debug("HandleCalendarAddEvent Playerguid: %u sends Calendarevent: Title: %s, Description: %s, Type: %u, Repeatable: %u, maxInvites: %u, dungeonId: %u, PackedTime: %u, unkPackedTime: %u, Flags: %u,",
        guid, title.c_str(), description.c_str(), type, repeatable, maxInvites, dungeonId, eventPackedTime, unkPackedTime, flags);

}

void WorldSession::handleCalendarGetEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarGetEvent Not handled");
}

void WorldSession::handleCalendarGuildFilter(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarGuildFilter Not handled");
}

void WorldSession::handleCalendarArenaTeam(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarArenaTeam Not handled");
}

void WorldSession::handleCalendarUpdateEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarUpdateEvent Not handled");
}

void WorldSession::handleCalendarRemoveEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarRemoveEvent Not handled");
}

void WorldSession::handleCalendarCopyEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarCopyEvent Not handled");
}

void WorldSession::handleCalendarEventInvite(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarEventInvite Not handled");
}

void WorldSession::handleCalendarEventRsvp(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarEventRsvp Not handled");
}

void WorldSession::handleCalendarEventRemoveInvite(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarEventRemoveInvite Not handled");
}

void WorldSession::handleCalendarEventStatus(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarEventStatus Not handled");
}

void WorldSession::handleCalendarEventModeratorStatus(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("HandleCalendarEventModeratorStatus Not handled");
}
#endif
