/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the invited player (or the guild of a guild event) learns about the invite
    class SmsgCalendarEventInviteAlert : public ManagedPacket
    {
    public:
        CalendarEventData calendarEvent;
        CalendarInviteData invite;

        SmsgCalendarEventInviteAlert(CalendarEventData calendarEvent, CalendarInviteData invite) :
            ManagedPacket(SMSG_CALENDAR_EVENT_INVITE_ALERT, 0),
            calendarEvent(std::move(calendarEvent)),
            invite(std::move(invite))
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet << calendarEvent.eventId;
                CalendarWire::writeDate(packet, m_protocol, calendarEvent.date);
                CalendarWire::writeFlags(packet, m_protocol, calendarEvent.flags);
                packet << calendarEvent.type << calendarEvent.textureId;
                CalendarWire::writeEventGuild(packet, m_protocol, calendarEvent.guildId);
                packet << invite.inviteId << invite.status << invite.rank;
                CalendarWire::writePlayer(packet, m_protocol, invite.senderGuid);
                CalendarWire::writePlayer(packet, m_protocol, calendarEvent.ownerGuid);
                packet.writeBits(calendarEvent.title.size(), 8);

                // 11.x and later: one more flag, never set
                if (m_protocol.expansion >= WoW::Expansion::_TheWarWithin)
                    packet.writeBit(false);

                CalendarWire::writeRaw(packet, calendarEvent.title);
                return true;
            }

            if (m_protocol.isMop())
                return writeMop(packet);

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << calendarEvent.eventId;
                packet << calendarEvent.title;
                packet.appendPackedTime(calendarEvent.date);
                packet << calendarEvent.flags;
                packet << uint32_t(calendarEvent.type);
                packet << calendarEvent.textureId;
                packet << invite.inviteId;

                // 4.x names the guild of the event
                if (m_protocol.isCata())
                    packet << CalendarWire::guildGuid(calendarEvent.guildId).getRawGuid();

                packet << invite.status << invite.rank;
                CalendarWire::writePlayer(packet, m_protocol, calendarEvent.ownerGuid);
                CalendarWire::writePlayer(packet, m_protocol, invite.senderGuid);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }

    private:
        bool writeMop(WorldPacket& packet)
        {
            const WoWGuid owner = CalendarWire::playerGuid(calendarEvent.ownerGuid);
            const WoWGuid sender = CalendarWire::playerGuid(invite.senderGuid);
            const WoWGuid guild = CalendarWire::guildGuid(calendarEvent.guildId);

            packet << calendarEvent.eventId << calendarEvent.textureId << calendarEvent.type << invite.inviteId << calendarEvent.flags;
            packet << invite.status;
            packet.appendPackedTime(calendarEvent.date);
            packet << invite.rank;

            packet.writeBit(guild[7]);
            packet.writeBit(owner[6]);
            packet.writeBit(guild[4]);
            packet.writeBit(guild[0]);
            packet.writeBit(owner[3]);
            packet.writeBit(sender[1]);
            packet.writeBit(guild[5]);
            packet.writeBit(owner[2]);
            packet.writeBit(owner[0]);
            packet.writeBit(guild[1]);
            packet.writeBit(owner[5]);
            packet.writeBit(guild[6]);
            packet.writeBit(guild[3]);
            packet.writeBit(sender[6]);
            packet.writeBit(sender[2]);
            packet.writeBit(owner[4]);
            packet.writeBit(owner[7]);
            packet.writeBit(sender[4]);
            packet.writeBit(owner[1]);
            packet.writeBit(sender[3]);
            packet.writeBit(guild[2]);
            packet.writeBit(sender[0]);
            packet.writeBits(calendarEvent.title.size(), 8);
            packet.writeBit(sender[5]);
            packet.writeBit(sender[7]);
            packet.flushBits();

            packet.writeByteSeq(owner[5]);
            packet.writeByteSeq(guild[6]);
            packet.writeByteSeq(guild[0]);
            packet.writeByteSeq(owner[6]);
            packet.writeByteSeq(sender[5]);
            packet.writeByteSeq(sender[4]);
            packet.writeString(calendarEvent.title);
            packet.writeByteSeq(owner[0]);
            packet.writeByteSeq(owner[1]);
            packet.writeByteSeq(guild[2]);
            packet.writeByteSeq(guild[7]);
            packet.writeByteSeq(sender[6]);
            packet.writeByteSeq(guild[1]);
            packet.writeByteSeq(owner[2]);
            packet.writeByteSeq(sender[3]);
            packet.writeByteSeq(sender[7]);
            packet.writeByteSeq(sender[0]);
            packet.writeByteSeq(owner[3]);
            packet.writeByteSeq(sender[2]);
            packet.writeByteSeq(owner[7]);
            packet.writeByteSeq(guild[5]);
            packet.writeByteSeq(guild[4]);
            packet.writeByteSeq(guild[3]);
            packet.writeByteSeq(sender[1]);
            packet.writeByteSeq(owner[4]);
            return true;
        }
    };
}
