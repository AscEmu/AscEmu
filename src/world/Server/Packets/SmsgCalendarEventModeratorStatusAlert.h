/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the moderation rank of an invite changed
    class SmsgCalendarEventModeratorStatusAlert : public ManagedPacket
    {
    public:
        uint64_t eventId = 0;
        uint32_t inviteeGuid = 0;
        uint8_t rank = 0;

        SmsgCalendarEventModeratorStatusAlert(uint64_t eventId, uint32_t inviteeGuid, uint8_t rank) :
            ManagedPacket(SMSG_CALENDAR_EVENT_MODERATOR_STATUS_ALERT, 9 + 8 + 1 + 1),
            eventId(eventId),
            inviteeGuid(inviteeGuid),
            rank(rank)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                CalendarWire::writePlayer(packet, m_protocol, inviteeGuid);
                packet << eventId << rank;
                packet.writeBit(true);                      // clear pending
                packet.flushBits();
                return true;
            }

            if (m_protocol.isMop())
            {
                const WoWGuid invitee = CalendarWire::playerGuid(inviteeGuid);

                packet.writeBit(invitee[3]);
                packet.writeBit(invitee[7]);
                packet.writeBit(invitee[2]);
                packet.writeBit(invitee[4]);
                packet.writeBit(invitee[1]);
                packet.writeBit(invitee[6]);
                packet.writeBit(invitee[5]);
                packet.writeBit(true);                      // clear pending
                packet.writeBit(invitee[0]);
                packet.flushBits();

                packet.writeByteSeq(invitee[5]);
                packet.writeByteSeq(invitee[4]);
                packet.writeByteSeq(invitee[6]);
                packet.writeByteSeq(invitee[1]);
                packet.writeByteSeq(invitee[7]);
                packet.writeByteSeq(invitee[2]);
                packet.writeByteSeq(invitee[3]);
                packet << rank << eventId;
                packet.writeByteSeq(invitee[0]);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                CalendarWire::writePlayer(packet, m_protocol, inviteeGuid);
                packet << eventId << rank;
                packet << uint8_t(1);                       // clear pending
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
