/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
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
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t id;
                packet >> id;
                eventId = static_cast<uint32_t>(id);

                // remaining flags/invite id are not used by us

                return true;
            }

            return false;
        }
    };
}
