/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the status of an invite changed
    class SmsgCalendarEventStatus : public ManagedPacket
    {
    public:
        CalendarEventData calendarEvent;
        CalendarInviteData invite;
        bool clearPending = false;

        SmsgCalendarEventStatus(CalendarEventData calendarEvent, CalendarInviteData invite, bool clearPending) :
            ManagedPacket(SMSG_CALENDAR_EVENT_STATUS, 0),
            calendarEvent(std::move(calendarEvent)),
            invite(std::move(invite)),
            clearPending(clearPending)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                CalendarWire::writePlayer(packet, m_protocol, invite.inviteeGuid);
                packet << calendarEvent.eventId;
                CalendarWire::writeDate(packet, m_protocol, calendarEvent.date);
                CalendarWire::writeFlags(packet, m_protocol, calendarEvent.flags);
                packet << invite.status;
                CalendarWire::writeDate(packet, m_protocol, invite.responseTime);
                packet.writeBit(clearPending);
                packet.flushBits();
                return true;
            }

            if (m_protocol.isMop())
            {
                const WoWGuid invitee = CalendarWire::playerGuid(invite.inviteeGuid);

                packet.writeBit(invitee[5]);
                packet.writeBit(invitee[0]);
                packet.writeBit(clearPending);
                packet.writeBit(invitee[2]);
                packet.writeBit(invitee[1]);
                packet.writeBit(invitee[4]);
                packet.writeBit(invitee[6]);
                packet.writeBit(invitee[7]);
                packet.writeBit(invitee[3]);
                packet.flushBits();

                packet.writeByteSeq(invitee[1]);
                packet.writeByteSeq(invitee[6]);
                packet.writeByteSeq(invitee[7]);
                if (invite.responseTime != 0)
                    packet.appendPackedTime(invite.responseTime);
                else
                    packet << uint32_t(0);
                packet.writeByteSeq(invitee[2]);
                packet.writeByteSeq(invitee[4]);
                packet << calendarEvent.eventId;
                packet.writeByteSeq(invitee[0]);
                packet.writeByteSeq(invitee[3]);
                packet.writeByteSeq(invitee[5]);
                packet << invite.status;
                packet << calendarEvent.flags;
                packet.appendPackedTime(calendarEvent.date);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                CalendarWire::writePlayer(packet, m_protocol, invite.inviteeGuid);
                packet << calendarEvent.eventId;
                packet.appendPackedTime(calendarEvent.date);
                packet << calendarEvent.flags;
                packet << invite.status;

                // 3.x: clear the pending answer, 4.x: the rank of the invite
                if (m_protocol.isCata())
                    packet << invite.rank;
                else
                    packet << uint8_t(clearPending ? 1 : 0);

                packet.appendPackedTime(invite.responseTime);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
