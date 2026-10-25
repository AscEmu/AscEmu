/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the members of an arena team the event planner invites at once, 3.x and 4.x
    class CmsgCalendarArenaTeam : public ManagedPacket
    {
    public:
        uint32_t arenaTeamId = 0;

        CmsgCalendarArenaTeam() :
            ManagedPacket(CMSG_CALENDAR_ARENA_TEAM, 4)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> arenaTeamId;
            return true;
        }
    };
}
