/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGmTicketStatusUpdate : public ManagedPacket
    {
    public:
        uint32_t status;

        SmsgGmTicketStatusUpdate() : SmsgGmTicketStatusUpdate(0)
        {
        }

        SmsgGmTicketStatusUpdate(uint32_t status) :
            ManagedPacket(SMSG_GM_TICKET_STATUS_UPDATE, 4),
            status(status)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << status;   // 3 = Survey

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
