/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Network/WorldPacket.hpp"

#include <cstdint>
#include <string>

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
        uint32_t oldestOpenTicketAge;

        SmsgGmTicketGetTicket() : SmsgGmTicketGetTicket(0, "", 0, 0, 0, "", 0)
        {
        }

        SmsgGmTicketGetTicket(uint32_t error, std::string message, uint8_t mapId , uint64_t ticketGuid, uint32_t ticketTimestamp, std::string comment, uint32_t oldestOpenTicketAge = 0) :
            ManagedPacket(SMSG_GMTICKET_GETTICKET, 0),
            error(error),
            message(message),
            mapId(mapId),
            ticketGuid(ticketGuid),
            ticketTimestamp(ticketTimestamp),
            comment(comment),
            oldestOpenTicketAge(oldestOpenTicketAge)
        {
        }

    protected:

        size_t expectedSize() const override { return 4 + 350 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // error doubles as the Mop status word: GMTCurrentTicketFound (6) == GMTICKET_STATUS_HASTEXT,
                // GMTNoCurrentTicket (10) == GMTICKET_STATUS_DEFAULT in the reference implementation.
                if (error == 6)
                {
                    packet.writeBit(1);                     // ticket exists
                    packet.writeBits(static_cast<uint32_t>(message.size()), 11);
                    packet.writeBits(0, 10);                 // waitTimeOverrideMessage size, always empty
                    packet.flushBits();

                    packet << static_cast<uint32_t>(ticketGuid);
                    packet << uint8_t(0);         // escalation status - not tracked
                    packet << uint8_t(0);         // opened-by-gm status - not tracked
                    packet << uint8_t(1);         // unk
                    packet << static_cast<uint32_t>(mapId);
                    packet.writeString(message);
                    packet << uint32_t(0);        // unk
                    packet << oldestOpenTicketAge;
                    packet << uint32_t(0);        // last ticket system change age - not tracked
                    packet << error;
                }
                else
                {
                    packet.writeBit(0);
                    packet.flushBits();
                    packet << error;
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << error;

                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    if (error == 6)                             // No current ticket
                        packet << message.c_str() << mapId;     // mapId is uint8_t, valid for cata/mop?
                }
                else
                {
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
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
