/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCalendarGetEvent : public ManagedPacket
    {
    public:
        uint32_t eventId = 0;

        CmsgCalendarGetEvent() :
            ManagedPacket(CMSG_CALENDAR_GET_EVENT, 8)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t id;
            packet >> id;
            eventId = static_cast<uint32_t>(id);
            return true;
        }
    };
}
