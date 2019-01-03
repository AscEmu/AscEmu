/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGmTicketGetTicket : public ManagedPacket
    {
    public:
        uint32_t error;
        std::string message;
        uint8_t mapId;

        SmsgGmTicketGetTicket() : SmsgGmTicketGetTicket(0, "", 0)
        {
        }

        SmsgGmTicketGetTicket(uint32_t error, std::string message, uint8_t mapId) :
            ManagedPacket(SMSG_GMTICKET_GETTICKET, 0),
            error(error),
            message(message),
            mapId(mapId)
        {
        }

    protected:

        size_t expectedSize() const override { return 4 + 350 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error;
            if (error == 6)                             // No current ticket
                packet << message.c_str() << mapId;     // mapId is uint8_t, valid for cata/mop?
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
