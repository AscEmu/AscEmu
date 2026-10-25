/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // a player was added to the invites of an event (or of the planner, then without event and invite id)
    class SmsgCalendarEventInvite : public ManagedPacket
    {
    public:
        CalendarInviteData invite;
        bool clearPending = false;

        SmsgCalendarEventInvite(CalendarInviteData invite, bool clearPending) :
            ManagedPacket(SMSG_CALENDAR_EVENT_INVITE, 0),
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
                packet << invite.eventId << invite.inviteId << invite.level << invite.status;
                packet << uint8_t(invite.guildEvent ? 1 : 0);
                CalendarWire::writeDate(packet, m_protocol, invite.responseTime);
                packet.writeBit(clearPending);
                packet.flushBits();
                return true;
            }

            const bool answered = invite.responseTime != 0 && invite.responseTime != CALENDAR_DEFAULT_RESPONSE_TIME;

            if (m_protocol.isMop())
            {
                const WoWGuid invitee = CalendarWire::playerGuid(invite.inviteeGuid);

                packet << uint8_t(invite.senderGuid == invite.inviteeGuid ? 1 : 0);
                packet << invite.status << invite.inviteId << invite.level << invite.eventId;

                packet.writeBit(invitee[6]);
                packet.writeBit(invitee[4]);
                packet.writeBit(invitee[1]);
                packet.writeBit(invitee[3]);
                packet.writeBit(invitee[7]);
                packet.writeBit(invitee[0]);
                packet.writeBit(invitee[2]);
                packet.writeBit(invitee[5]);
                packet.writeBit(invite.responseTime == 0);
                packet.writeBit(clearPending);
                packet.flushBits();

                packet.writeByteSeq(invitee[7]);
                packet.writeByteSeq(invitee[0]);
                packet.writeByteSeq(invitee[5]);
                if (invite.responseTime != 0)
                    packet.appendPackedTime(invite.responseTime);
                packet.writeByteSeq(invitee[2]);
                packet.writeByteSeq(invitee[3]);
                packet.writeByteSeq(invitee[4]);
                packet.writeByteSeq(invitee[1]);
                packet.writeByteSeq(invitee[6]);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                CalendarWire::writePlayer(packet, m_protocol, invite.inviteeGuid);
                packet << invite.eventId << invite.inviteId << invite.level << invite.status;

                // the answer time only follows when the invite was answered
                packet << uint8_t(answered ? 1 : 0);
                if (answered)
                    packet.appendPackedTime(invite.responseTime);

                packet << uint8_t(invite.senderGuid != invite.inviteeGuid ? 1 : 0);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
