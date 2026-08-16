/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCalendarSendNumPending : public ManagedPacket
    {
    public:
        uint32_t numPending;

        SmsgCalendarSendNumPending() : SmsgCalendarSendNumPending(0)
        {
        }

        SmsgCalendarSendNumPending(uint32_t numPending) :
            ManagedPacket(SMSG_CALENDAR_SEND_NUM_PENDING, 4),
            numPending(numPending)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                packet << numPending;    // num pending

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
