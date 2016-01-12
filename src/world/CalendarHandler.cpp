/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"

/// \todo CalendarHandler
void WorldSession::HandleCalendarGetCalendar(WorldPacket& /*recv_data*/)
{
    Log.Debug("HandleCalendarGetCalendar", "Not handled");

    /* Get all events for the player */
    uint32 guid = _player->GetGUID();
    Log.Debug("HandleCalendarGetCalendar", "CMSG_CALENDAR_GET_CALENDAR for guid %u", guid);

}

void WorldSession::HandleCalendarComplain(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarComplain", "Not handled");
}

void WorldSession::HandleCalendarGetNumPending(WorldPacket& /*recv_data*/)
{
    Log.Debug("HandleCalendarGetNumPending", "Not handled");

    WorldPacket data(SMSG_CALENDAR_SEND_NUM_PENDING, 4);
    SendPacket(&data);
}

void WorldSession::HandleCalendarAddEvent(WorldPacket& recv_data)
{
    // Create an Event and save it to char db 
    Log.Debug("HandleCalendarAddEvent", "Not handled");

    uint32 guid = _player->GetGUID();

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
    Log.Debug("HandleCalendarAddEvent", "Playerguid: %u sends Calendarevent: Title: %s, Description: %s, Type: %u, Repeatable: %u, maxInvites: %u, dungeonId: %u, PackedTime: %u, unkPackedTime: %u, Flags: %u,", 
        guid, title.c_str(), description.c_str(), type, repeatable, maxInvites, dungeonId, eventPackedTime, unkPackedTime, flags);

}

void WorldSession::HandleCalendarGetEvent(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarGetEvent", "Not handled");
}

void WorldSession::HandleCalendarGuildFilter(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarGuildFilter", "Not handled");
}

void WorldSession::HandleCalendarArenaTeam(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarArenaTeam", "Not handled");
}

void WorldSession::HandleCalendarUpdateEvent(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarUpdateEvent", "Not handled");
}

void WorldSession::HandleCalendarRemoveEvent(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarRemoveEvent", "Not handled");
}

void WorldSession::HandleCalendarCopyEvent(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarCopyEvent", "Not handled");
}

void WorldSession::HandleCalendarEventInvite(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarEventInvite", "Not handled");
}

void WorldSession::HandleCalendarEventRsvp(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarEventRsvp", "Not handled");
}

void WorldSession::HandleCalendarEventRemoveInvite(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarEventRemoveInvite", "Not handled");
}

void WorldSession::HandleCalendarEventStatus(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarEventStatus", "Not handled");
}

void WorldSession::HandleCalendarEventModeratorStatus(WorldPacket& recv_data)
{
    Log.Debug("HandleCalendarEventModeratorStatus", "Not handled");
}
