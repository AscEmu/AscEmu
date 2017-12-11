/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "Log.hpp"
#include "Units/Players/Player.h"

//\todo Rewrite for cata - after this all functions are copied from wotlk
/// \todo CalendarHandler
void WorldSession::HandleCalendarGetCalendar(WorldPacket& /*recv_data*/)
{
    LogDebugFlag(LF_OPCODE, "HandleCalendarGetCalendar Not handled");

    /* Get all events for the player */
    uint32 guid = static_cast<uint32>(_player->GetGUID());
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
    data << uint32(0);  // num pending
    SendPacket(&data);
}

void WorldSession::HandleCalendarAddEvent(WorldPacket& recvData)
{
    // Create an Event and save it to char db 
    LogDebugFlag(LF_OPCODE, "HandleCalendarAddEvent Not handled");

    uint32 guid = static_cast<uint32>(_player->GetGUID());

    std::string title;
    std::string description;
    uint8 type;
    uint8 repeatable;
    uint32 maxInvites;
    int32 dungeonId;
    uint32 eventPackedTime;
    uint32 unkPackedTime;
    uint32 flags;

    recvData >> title;
    recvData >> description;
    recvData >> type;
    recvData >> repeatable;
    recvData >> maxInvites;
    recvData >> dungeonId;
    recvData.ReadPackedTime(eventPackedTime);
    recvData.ReadPackedTime(unkPackedTime);
    recvData >> flags;

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
