/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetCalendar Not handled");

    /* Get all events for the player */
    uint32_t guid = static_cast<uint32_t>(_player->getGuid());
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetCalendar CMSG_CALENDAR_GET_CALENDAR for guid %u", guid);

}

void WorldSession::handleCalendarComplain(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarComplain Not handled");
}

void WorldSession::handleCalendarGetNumPending(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetNumPending Not handled");

    WorldPacket data(SMSG_CALENDAR_SEND_NUM_PENDING, 4);
#if VERSION_STRING >= Cata
    data << uint32_t(0);  // num pending
#endif
    SendPacket(&data);
}

void WorldSession::handleCalendarAddEvent(WorldPacket& recvPacket)
{
    // Create an Event and save it to char db 
    LogDebugFlag(LF_OPCODE, "HandleCalendarAddEvent Not handled");

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
    LogDebugFlag(LF_OPCODE, "HandleCalendarAddEvent Playerguid: %u sends Calendarevent: Title: %s, Description: %s, Type: %u, Repeatable: %u, maxInvites: %u, dungeonId: %u, PackedTime: %u, unkPackedTime: %u, Flags: %u,",
        guid, title.c_str(), description.c_str(), type, repeatable, maxInvites, dungeonId, eventPackedTime, unkPackedTime, flags);

}

void WorldSession::handleCalendarGetEvent(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetEvent Not handled");
}

void WorldSession::handleCalendarGuildFilter(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarGuildFilter Not handled");
}

void WorldSession::handleCalendarArenaTeam(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarArenaTeam Not handled");
}

void WorldSession::handleCalendarUpdateEvent(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarUpdateEvent Not handled");
}

void WorldSession::handleCalendarRemoveEvent(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarRemoveEvent Not handled");
}

void WorldSession::handleCalendarCopyEvent(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarCopyEvent Not handled");
}

void WorldSession::handleCalendarEventInvite(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventInvite Not handled");
}

void WorldSession::handleCalendarEventRsvp(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventRsvp Not handled");
}

void WorldSession::handleCalendarEventRemoveInvite(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventRemoveInvite Not handled");
}

void WorldSession::handleCalendarEventStatus(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventStatus Not handled");
}

void WorldSession::handleCalendarEventModeratorStatus(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventModeratorStatus Not handled");
}
#endif
