/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "Log.hpp"
#include "Units/Players/Player.h"

#if VERSION_STRING > TBC

/// \todo CalendarHandler
void WorldSession::HandleCalendarGetCalendar(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetCalendar Not handled");

    /* Get all events for the player */
    uint32 guid = static_cast<uint32>(_player->getGuid());
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetCalendar CMSG_CALENDAR_GET_CALENDAR for guid %u", guid);

}

void WorldSession::HandleCalendarComplain(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarComplain Not handled");
}

void WorldSession::HandleCalendarGetNumPending(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetNumPending Not handled");

    WorldPacket data(SMSG_CALENDAR_SEND_NUM_PENDING, 4);
#if VERSION_STRING == Cata
    data << uint32(0);  // num pending
#endif
    SendPacket(&data);
}

void WorldSession::HandleCalendarAddEvent(WorldPacket& recv_data)
{
    // Create an Event and save it to char db 
    LogDebugFlag(LF_OPCODE, "HandleCalendarAddEvent Not handled");

    uint32 guid = static_cast<uint32>(_player->getGuid());

    std::string title;
    std::string description;
    uint8 type;
    uint8 repeatable;
    uint32 maxInvites;
    int32 dungeonId;
    uint32 eventPackedTime;
    uint32 unkPackedTime;
    uint32 flags;

    recv_data >> title;
    recv_data >> description;
    recv_data >> type;
    recv_data >> repeatable;
    recv_data >> maxInvites;
    recv_data >> dungeonId;
    recv_data.ReadPackedTime(eventPackedTime);
    recv_data.ReadPackedTime(unkPackedTime);
    recv_data >> flags;

    /// \todo save it to db
    LogDebugFlag(LF_OPCODE, "HandleCalendarAddEvent Playerguid: %u sends Calendarevent: Title: %s, Description: %s, Type: %u, Repeatable: %u, maxInvites: %u, dungeonId: %u, PackedTime: %u, unkPackedTime: %u, Flags: %u,",
        guid, title.c_str(), description.c_str(), type, repeatable, maxInvites, dungeonId, eventPackedTime, unkPackedTime, flags);

}

void WorldSession::HandleCalendarGetEvent(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetEvent Not handled");
}

void WorldSession::HandleCalendarGuildFilter(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarGuildFilter Not handled");
}

void WorldSession::HandleCalendarArenaTeam(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarArenaTeam Not handled");
}

void WorldSession::HandleCalendarUpdateEvent(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarUpdateEvent Not handled");
}

void WorldSession::HandleCalendarRemoveEvent(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarRemoveEvent Not handled");
}

void WorldSession::HandleCalendarCopyEvent(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarCopyEvent Not handled");
}

void WorldSession::HandleCalendarEventInvite(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventInvite Not handled");
}

void WorldSession::HandleCalendarEventRsvp(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventRsvp Not handled");
}

void WorldSession::HandleCalendarEventRemoveInvite(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventRemoveInvite Not handled");
}

void WorldSession::HandleCalendarEventStatus(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventStatus Not handled");
}

void WorldSession::HandleCalendarEventModeratorStatus(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarEventModeratorStatus Not handled");
}
#endif
