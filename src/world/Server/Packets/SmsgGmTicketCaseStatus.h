/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <ctime>

namespace AscEmu::Packets
{
    class SmsgGmTicketCaseStatus : public ManagedPacket
    {
    public:
        std::time_t updateTime;
        std::time_t oldestTicketTime;

        SmsgGmTicketCaseStatus() : SmsgGmTicketCaseStatus(0, 0)
        {
        }

        SmsgGmTicketCaseStatus(std::time_t updateTime, std::time_t oldestTicketTime) :
            ManagedPacket(SMSG_GM_TICKET_CASE_STATUS, 0),
            updateTime(updateTime),
            oldestTicketTime(oldestTicketTime)
        {
        }

    protected:
        size_t expectedSize() const override { return 11; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBits(0, 20); // case count, always empty - client only reads the timestamps
                packet.flushBits();
                packet.appendPackedTime(updateTime);
                packet.appendPackedTime(oldestTicketTime);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
