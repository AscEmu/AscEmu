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
    // the invited player learns that the status of his invite changed
    class SmsgCalendarEventInviteStatusAlert : public ManagedPacket
    {
    public:
        uint64_t eventId = 0;
        time_t date = 0;
        uint32_t flags = 0;
        uint8_t status = 0;

        SmsgCalendarEventInviteStatusAlert(uint64_t eventId, time_t date, uint32_t flags, uint8_t status) :
            ManagedPacket(SMSG_CALENDAR_EVENT_INVITE_STATUS_ALERT, 8 + 4 + 4 + 1),
            eventId(eventId),
            date(date),
            flags(flags),
            status(status)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet << eventId << flags << status;
                packet.appendPackedTime(date);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet << eventId;
                CalendarWire::writeDate(packet, m_protocol, date);
                if (m_protocol.expansion >= WoW::Expansion::_WoD)
                    CalendarWire::writeFlags(packet, m_protocol, flags);
                else
                    packet << flags;
                packet << status;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
