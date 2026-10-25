/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // an invite was removed from an event
    class SmsgCalendarEventInviteRemoved : public ManagedPacket
    {
    public:
        uint64_t eventId = 0;
        uint32_t inviteeGuid = 0;
        uint32_t flags = 0;

        SmsgCalendarEventInviteRemoved(uint64_t eventId, uint32_t inviteeGuid, uint32_t flags) :
            ManagedPacket(SMSG_CALENDAR_EVENT_INVITE_REMOVED, 9 + 8 + 4 + 1),
            eventId(eventId),
            inviteeGuid(inviteeGuid),
            flags(flags)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                CalendarWire::writePlayer(packet, m_protocol, inviteeGuid);
                packet << eventId;
                CalendarWire::writeFlags(packet, m_protocol, flags);
                packet.writeBit(true);                      // clear pending
                packet.flushBits();
                return true;
            }

            if (m_protocol.isMop())
            {
                const WoWGuid invitee = CalendarWire::playerGuid(inviteeGuid);

                packet.writeBit(invitee[6]);
                packet.writeBit(invitee[7]);
                packet.writeBit(invitee[3]);
                packet.writeBit(invitee[0]);
                packet.writeBit(invitee[2]);
                packet.writeBit(invitee[4]);
                packet.writeBit(invitee[1]);
                packet.writeBit(invitee[5]);
                packet.writeBit(true);                      // clear pending
                packet.flushBits();

                packet.writeByteSeq(invitee[0]);
                packet.writeByteSeq(invitee[4]);
                packet.writeByteSeq(invitee[3]);
                packet.writeByteSeq(invitee[5]);
                packet << eventId;
                packet.writeByteSeq(invitee[7]);
                packet.writeByteSeq(invitee[1]);
                packet.writeByteSeq(invitee[2]);
                packet << flags;
                packet.writeByteSeq(invitee[6]);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                CalendarWire::writePlayer(packet, m_protocol, inviteeGuid);
                packet << eventId << flags;
                packet << uint8_t(1);                       // clear pending
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
