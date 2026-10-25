/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    // an event with all its invites, answer to a request, a new or a copied event
    class SmsgCalendarSendEvent : public ManagedPacket
    {
    public:
        uint8_t sendType = 0;
        CalendarEventData calendarEvent;
        std::vector<CalendarInviteData> invites;

        SmsgCalendarSendEvent(uint8_t sendType, CalendarEventData calendarEvent, std::vector<CalendarInviteData> invites) :
            ManagedPacket(SMSG_CALENDAR_SEND_EVENT, 0),
            sendType(sendType),
            calendarEvent(std::move(calendarEvent)),
            invites(std::move(invites))
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
                return writeModern(packet);

            if (m_protocol.isMop())
                return writeMop(packet);

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << sendType;
                CalendarWire::writePlayer(packet, m_protocol, calendarEvent.ownerGuid);
                packet << calendarEvent.eventId;
                packet << calendarEvent.title;
                packet << calendarEvent.description;
                packet << calendarEvent.type;
                packet << uint8_t(0);                       // never repeats
                packet << uint32_t(CALENDAR_MAX_INVITES);
                packet << calendarEvent.textureId;
                packet << calendarEvent.flags;
                packet.appendPackedTime(calendarEvent.date);
                packet.appendPackedTime(calendarEvent.date); // lock date in 3.x, time zone date in 4.x

                if (m_protocol.isCata())
                    packet << CalendarWire::guildGuid(calendarEvent.guildId).getRawGuid();
                else
                    packet << calendarEvent.guildId;

                packet << uint32_t(invites.size());
                for (const auto& invite : invites)
                {
                    CalendarWire::writePlayer(packet, m_protocol, invite.inviteeGuid);
                    packet << invite.level << invite.status << invite.rank;
                    packet << uint8_t(invite.guildMember ? 1 : 0);
                    packet << invite.inviteId;
                    packet.appendPackedTime(invite.responseTime);
                    packet << invite.notes;
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }

    private:
        void writeInvite(WorldPacket& packet, CalendarInviteData const& invite) const
        {
            CalendarWire::writePlayer(packet, m_protocol, invite.inviteeGuid);
            packet << invite.inviteId << invite.level << invite.status << invite.rank;
            packet << uint8_t(invite.guildMember ? 1 : 0);
            CalendarWire::writeDate(packet, m_protocol, invite.responseTime);
            packet.writeBits(invite.notes.size(), 8);
            CalendarWire::writeRaw(packet, invite.notes);
        }

        bool writeModern(WorldPacket& packet)
        {
            packet << sendType;
            CalendarWire::writePlayer(packet, m_protocol, calendarEvent.ownerGuid);
            packet << calendarEvent.eventId << calendarEvent.type << calendarEvent.textureId;
            CalendarWire::writeFlags(packet, m_protocol, calendarEvent.flags);
            CalendarWire::writeDate(packet, m_protocol, calendarEvent.date);

            // the event is never locked
            if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
                CalendarWire::writeUnsetDate(packet, m_protocol);
            else
                packet << uint32_t(0);

            CalendarWire::writeEventGuild(packet, m_protocol, calendarEvent.guildId);
            packet << uint32_t(invites.size());

            // 7.x to 11.x announce the text lengths before the invites, 6.x and 12.x after them
            const bool lengthsFirst = m_protocol.expansion >= WoW::Expansion::_Legion && m_protocol.expansion < WoW::Expansion::_Midnight;
            if (lengthsFirst)
            {
                packet.writeBits(calendarEvent.title.size(), 8);
                packet.writeBits(calendarEvent.description.size(), 11);
                packet.flushBits();
            }

            for (const auto& invite : invites)
                writeInvite(packet, invite);

            if (!lengthsFirst)
            {
                packet.writeBits(calendarEvent.title.size(), 8);
                packet.writeBits(calendarEvent.description.size(), 11);
            }

            CalendarWire::writeRaw(packet, calendarEvent.title);
            CalendarWire::writeRaw(packet, calendarEvent.description);
            return true;
        }

        bool writeMop(WorldPacket& packet)
        {
            const WoWGuid owner = CalendarWire::playerGuid(calendarEvent.ownerGuid);
            const WoWGuid guild = CalendarWire::guildGuid(calendarEvent.guildId);

            packet.writeBits(invites.size(), 20);

            ByteBuffer inviteData;
            for (const auto& invite : invites)
            {
                const WoWGuid invitee = CalendarWire::playerGuid(invite.inviteeGuid);

                packet.writeBit(invitee[1]);
                packet.writeBit(invitee[2]);
                packet.writeBit(invitee[0]);
                packet.writeBit(invitee[7]);
                packet.writeBit(invitee[3]);
                packet.writeBit(invitee[5]);
                packet.writeBit(invitee[6]);
                packet.writeBits(invite.notes.size(), 8);
                packet.writeBit(invitee[4]);

                if (invite.responseTime != 0)
                    inviteData.appendPackedTime(invite.responseTime);
                else
                    inviteData << uint32_t(0);

                inviteData.writeByteSeq(invitee[5]);
                inviteData << uint8_t(invite.guildMember ? 1 : 0);
                inviteData.writeByteSeq(invitee[1]);
                inviteData.writeByteSeq(invitee[2]);
                inviteData.writeByteSeq(invitee[6]);
                CalendarWire::writeRaw(inviteData, invite.notes);
                inviteData << invite.level;
                inviteData.writeByteSeq(invitee[7]);
                inviteData << invite.rank;
                inviteData << invite.inviteId;
                inviteData.writeByteSeq(invitee[0]);
                inviteData.writeByteSeq(invitee[3]);
                inviteData.writeByteSeq(invitee[4]);
                inviteData << invite.status;
            }

            packet.writeBits(calendarEvent.title.size(), 8);
            packet.writeBit(owner[2]);
            packet.writeBit(owner[0]);
            packet.writeBit(guild[4]);
            packet.writeBit(guild[5]);
            packet.writeBit(owner[1]);
            packet.writeBit(owner[5]);
            packet.writeBit(owner[3]);
            packet.writeBit(guild[6]);
            packet.writeBits(calendarEvent.description.size(), 11);
            packet.writeBit(guild[1]);
            packet.writeBit(guild[7]);
            packet.writeBit(owner[6]);
            packet.writeBit(guild[2]);
            packet.writeBit(guild[0]);
            packet.writeBit(owner[4]);
            packet.writeBit(guild[3]);
            packet.writeBit(owner[7]);
            packet.flushBits();

            packet.append(inviteData);
            packet.writeByteSeq(owner[0]);
            packet.writeByteSeq(owner[5]);
            packet << calendarEvent.flags;
            packet.writeByteSeq(guild[1]);
            packet.writeByteSeq(owner[7]);
            packet.writeByteSeq(owner[3]);
            packet.appendPackedTime(calendarEvent.date);
            packet.writeByteSeq(guild[6]);
            packet.writeByteSeq(owner[4]);
            packet << calendarEvent.textureId;
            packet << uint32_t(0);                          // lock date
            packet.writeByteSeq(guild[4]);
            packet << calendarEvent.type;
            CalendarWire::writeRaw(packet, calendarEvent.title);
            packet.writeByteSeq(owner[6]);
            packet.writeByteSeq(guild[3]);
            packet.writeByteSeq(owner[2]);
            packet.writeByteSeq(guild[7]);
            packet.writeByteSeq(owner[1]);
            CalendarWire::writeRaw(packet, calendarEvent.description);
            packet.writeByteSeq(guild[2]);
            packet.writeByteSeq(guild[5]);
            packet.writeByteSeq(guild[0]);
            packet << calendarEvent.eventId;
            packet << sendType;
            return true;
        }
    };
}
