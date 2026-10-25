/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

namespace AscEmu::Packets
{
    // the client may close the waiting state of its last calendar action
    class SmsgCalendarClearPendingAction : public ManagedPacket
    {
    public:
        SmsgCalendarClearPendingAction() :
            ManagedPacket(SMSG_CALENDAR_CLEAR_PENDING_ACTION, 0)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override { return true; }
        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
