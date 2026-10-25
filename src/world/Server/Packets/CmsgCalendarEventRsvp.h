/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the answer of a player to the invite of an event
    class CmsgCalendarEventRsvp : public ManagedPacket
    {
    public:
        uint32_t eventId = 0;
        uint32_t inviteId = 0;
        uint8_t status = 0;

        CmsgCalendarEventRsvp() :
            ManagedPacket(CMSG_CALENDAR_EVENT_RSVP, 8 + 8 + 1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t id;
            uint64_t invite;
            packet >> id >> invite;

            // 4.x sends the status as 32 bit value
            if (m_protocol.isCata())
            {
                uint32_t longStatus;
                packet >> longStatus;
                status = static_cast<uint8_t>(longStatus);
            }
            else
            {
                packet >> status;
            }

            eventId = static_cast<uint32_t>(id);
            inviteId = static_cast<uint32_t>(invite);
            return true;
        }
    };
}
