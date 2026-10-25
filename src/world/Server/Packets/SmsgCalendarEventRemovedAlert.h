/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>
#include <ctime>

namespace AscEmu::Packets
{
    // an event was deleted
    class SmsgCalendarEventRemovedAlert : public ManagedPacket
    {
    public:
        uint64_t eventId = 0;
        time_t date = 0;
        bool clearPending = false;

        SmsgCalendarEventRemovedAlert(uint64_t eventId, time_t date, bool clearPending) :
            ManagedPacket(SMSG_CALENDAR_EVENT_REMOVED_ALERT, 1 + 8 + 4),
            eventId(eventId),
            date(date),
            clearPending(clearPending)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Mop)
            {
                packet << eventId;
                CalendarWire::writeDate(packet, m_protocol, date);
                packet.writeBit(clearPending);
                packet.flushBits();
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet << uint8_t(clearPending ? 1 : 0);
                packet << eventId;
                packet.appendPackedTime(date);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
