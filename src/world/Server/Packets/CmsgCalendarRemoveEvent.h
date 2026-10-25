/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCalendarRemoveEvent : public ManagedPacket
    {
    public:
        uint32_t eventId = 0;

        CmsgCalendarRemoveEvent() :
            ManagedPacket(CMSG_CALENDAR_REMOVE_EVENT, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // planner invite id, not used by us
                packet.readSkip<uint64_t>();

                uint64_t id;
                packet >> id;
                eventId = static_cast<uint32_t>(id);
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata || m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // the event leads, moderator invite, club and flags follow and are not used by us
                uint64_t id;
                packet >> id;
                eventId = static_cast<uint32_t>(id);
                return true;
            }

            return false;
        }
    };
}
