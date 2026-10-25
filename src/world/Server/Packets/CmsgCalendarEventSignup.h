/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // a member signs up for a guild event
    class CmsgCalendarEventSignup : public ManagedPacket
    {
    public:
        uint32_t eventId = 0;
        bool tentative = false;

        CmsgCalendarEventSignup() :
            ManagedPacket(CMSG_CALENDAR_EVENT_SIGNUP, 8 + 1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t id;
            packet >> id;
            eventId = static_cast<uint32_t>(id);

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint8_t value;
                packet >> value;
                tentative = value != 0;
                return true;
            }

            // 8.x and later: the club of the event
            if (m_protocol.expansion >= WoW::Expansion::_BfA)
                packet.readSkip<uint64_t>();

            tentative = packet.readBit();
            return true;
        }
    };
}
