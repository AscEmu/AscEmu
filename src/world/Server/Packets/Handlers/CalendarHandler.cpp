/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Maps/InstanceMgr.hpp"
#include "Utilities/Util.hpp"

#if VERSION_STRING > TBC

// \todo CalendarHandler
void WorldSession::handleCalendarGetCalendar(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarGetCalendar Not handled");

    /* Get all events for the player */
    uint32_t guid = static_cast<uint32_t>(_player->getGuid());
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarGetCalendar CMSG_CALENDAR_GET_CALENDAR for guid {}", guid);

}

void WorldSession::handleCalendarComplain(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarComplain Not handled");
}

void WorldSession::handleCalendarGetNumPending(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarGetNumPending Not handled");

    WorldPacket data(SMSG_CALENDAR_SEND_NUM_PENDING, 4);
#if VERSION_STRING >= Cata
    data << uint32_t(0);  // num pending
#endif
    SendPacket(&data);
}

void WorldSession::handleCalendarAddEvent(WorldPacket& recvPacket)
{
    // Create an Event and save it to char db 
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarAddEvent Not handled");

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
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarAddEvent Playerguid: {} sends Calendarevent: Title: {}, Description: {}, Type: {}, Repeatable: {}, maxInvites: {}, dungeonId: {}, PackedTime: {}, unkPackedTime: {}, Flags: {},",
        guid, title, description, type, repeatable, maxInvites, dungeonId, eventPackedTime, unkPackedTime, flags);

}

void WorldSession::handleCalendarGetEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarGetEvent Not handled");
}

void WorldSession::handleCalendarGuildFilter(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarGuildFilter Not handled");
}

void WorldSession::handleCalendarArenaTeam(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarArenaTeam Not handled");
}

void WorldSession::handleCalendarUpdateEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarUpdateEvent Not handled");
}

void WorldSession::handleCalendarRemoveEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarRemoveEvent Not handled");
}

void WorldSession::handleCalendarCopyEvent(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarCopyEvent Not handled");
}

void WorldSession::handleCalendarEventInvite(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarEventInvite Not handled");
}

void WorldSession::handleCalendarEventRsvp(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarEventRsvp Not handled");
}

void WorldSession::handleCalendarEventRemoveInvite(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarEventRemoveInvite Not handled");
}

void WorldSession::handleCalendarEventStatus(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarEventStatus Not handled");
}

void WorldSession::handleCalendarEventModeratorStatus(WorldPacket& /*recvPacket*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "HandleCalendarEventModeratorStatus Not handled");
}

void WorldSession::sendCalendarRaidLockout(InstanceSaved const* save, bool add)
{
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "SMSG_CALENDAR_RAID_LOCKOUT_ADDED/REMOVED");
    const auto now = Util::getTimeNow();
    time_t currTime = now;

    WorldPacket data(SMSG_CALENDAR_RAID_LOCKOUT_REMOVED, (4) + 4 + 4 + 4 + 8);
    if (add)
    {
        data.SetOpcode(SMSG_CALENDAR_RAID_LOCKOUT_ADDED);
        data.appendPackedTime(currTime);
    }

    data << uint32_t(save->getMapId());
    data << uint32_t(save->getDifficulty());
    data << uint32_t(save->getResetTime() - currTime);
    data << uint64_t(save->getInstanceId());
    SendPacket(&data);
}

void WorldSession::sendCalendarRaidLockoutUpdated(InstanceSaved const* save)
{
    if (!save)
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "SMSG_CALENDAR_RAID_LOCKOUT_UPDATED [{}] Map: {}, Difficulty {}", _player->getGuid(), save->getMapId(), save->getDifficulty());

    const auto now = Util::getTimeNow();
    time_t currTime = now;

    WorldPacket data(SMSG_CALENDAR_RAID_LOCKOUT_UPDATED, 4 + 4 + 4 + 4 + 8);
    data.appendPackedTime(currTime);
    data << uint32_t(save->getMapId());
    data << uint32_t(save->getDifficulty());
    data << uint32_t(0); // Amount of seconds that has changed to the reset time
    data << uint32_t(save->getResetTime() - currTime);
    SendPacket(&data);
}
#endif
