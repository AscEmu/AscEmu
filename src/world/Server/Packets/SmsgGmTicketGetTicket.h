/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class SmsgGmTicketGetTicket : public ManagedPacket
    {
    public:
        uint32_t error;
        std::string message;
        uint8_t mapId;
        uint64_t ticketGuid;
        uint32_t ticketTimestamp;
        std::string comment;

        SmsgGmTicketGetTicket() : SmsgGmTicketGetTicket(0, "", 0, 0, 0, "")
        {
        }

        SmsgGmTicketGetTicket(uint32_t error, std::string message, uint8_t mapId , uint64_t ticketGuid, uint32_t ticketTimestamp, std::string comment) :
            ManagedPacket(SMSG_GMTICKET_GETTICKET, 0),
            error(error),
            message(message),
            mapId(mapId),
            ticketGuid(ticketGuid),
            ticketTimestamp(ticketTimestamp),
            comment(comment)
        {
        }

    protected:

        size_t expectedSize() const override { return 4 + 350 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error;
#if VERSION_STRING < Cata
            if (error == 6)                             // No current ticket
                packet << message.c_str() << mapId;     // mapId is uint8_t, valid for cata/mop?

#else
            if (error == 6)
            {
                packet << uint32_t(ticketGuid);
                packet << message;
                packet << uint8_t(0);         // unk
                packet << float(ticketTimestamp);
                packet << float(0);           // unk
                packet << float(0);           // unk

                packet << uint8_t(2);         // escalate?
                packet << uint8_t(comment.empty() ? 0 : 1);

                std::string unkstring;
                packet << unkstring;
                packet << uint32_t(0);        // wait time
            }
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
