/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCalendarCopyEvent : public ManagedPacket
    {
    public:
        uint32_t eventId = 0;
        uint32_t eventTime = 0;

        CmsgCalendarCopyEvent() :
            ManagedPacket(CMSG_CALENDAR_COPY_EVENT, 8 + 8 + 4)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t id;
            packet >> id;
            packet.readSkip<uint64_t>();        // moderator invite

            // 8.x and later: the club of the event
            if (m_protocol.expansion >= WoW::Expansion::_BfA)
                packet.readSkip<uint64_t>();

            eventTime = packet.readPackedTime();
            eventId = static_cast<uint32_t>(id);
            return true;
        }
    };
}
