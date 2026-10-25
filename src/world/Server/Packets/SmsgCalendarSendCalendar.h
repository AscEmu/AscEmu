/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"
#include "Utilities/Util.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    // the calendar of a player: his invites, the events he sees and the raids he is saved to
    class SmsgCalendarSendCalendar : public ManagedPacket
    {
    public:
        std::vector<CalendarInviteData> invites;
        std::vector<CalendarEventData> events;
        std::vector<CalendarLockoutData> lockouts;
        std::vector<CalendarRaidResetData> raidResets;
        std::vector<CalendarHolidayData> holidays;

        SmsgCalendarSendCalendar(std::vector<CalendarInviteData> invites, std::vector<CalendarEventData> events, std::vector<CalendarLockoutData> lockouts,
            std::vector<CalendarRaidResetData> raidResets = {}, std::vector<CalendarHolidayData> holidays = {}) :
            ManagedPacket(SMSG_CALENDAR_SEND_CALENDAR, 0),
            invites(std::move(invites)),
            events(std::move(events)),
            lockouts(std::move(lockouts)),
            raidResets(std::move(raidResets)),
            holidays(std::move(holidays))
        {
        }

    protected:
        // the start of the raid reset rhythm the clients count from: 2005-12-28 07:00
        static constexpr uint32_t RaidResetOrigin = 1135753200;

        bool internalSerialise(WorldPacket& packet) override
        {
            const time_t now = ::Util::getTimeNow();

            if (m_protocol.expansion >= WoW::Expansion::_Legion)
                return writeCounted(packet, now);

            if (m_protocol.isWoD())
            {
                packet << uint32_t(now);
                CalendarWire::writeDate(packet, m_protocol, now);
                packet << RaidResetOrigin;
                packet << uint32_t(invites.size());
                packet << uint32_t(events.size());
                packet << uint32_t(lockouts.size());
                packet << uint32_t(raidResets.size());

                for (const auto& invite : invites)
                    writeInvite(packet, invite);
                for (const auto& calendarEvent : events)
                    writeEvent(packet, calendarEvent);
                for (const auto& lockout : lockouts)
                    writeLockout(packet, lockout);
                writeRaidResets(packet);
                return true;
            }

            if (m_protocol.isMop())
                return writeMop(packet, now);

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint32_t(invites.size());
                for (const auto& invite : invites)
                {
                    packet << invite.eventId << invite.inviteId << invite.status << invite.rank;

                    // 4.x names the owner of an existing event, 3.x always the sender
                    if (m_protocol.isCata())
                    {
                        packet << uint8_t(invite.ownerGuid != 0 && invite.guildEvent ? 1 : 0);
                        CalendarWire::writePlayer(packet, m_protocol, invite.ownerGuid != 0 ? invite.ownerGuid : invite.senderGuid);
                    }
                    else
                    {
                        packet << uint8_t(invite.guildMember ? 1 : 0);
                        CalendarWire::writePlayer(packet, m_protocol, invite.senderGuid);
                    }
                }

                packet << uint32_t(events.size());
                for (const auto& calendarEvent : events)
                {
                    packet << calendarEvent.eventId;
                    packet << calendarEvent.title;
                    packet << uint32_t(calendarEvent.type);
                    packet.appendPackedTime(calendarEvent.date);
                    packet << calendarEvent.flags;
                    packet << calendarEvent.textureId;
                    if (m_protocol.isCata())
                        packet << CalendarWire::guildGuid(calendarEvent.guildId).getRawGuid();
                    CalendarWire::writePlayer(packet, m_protocol, calendarEvent.ownerGuid);
                }

                packet << uint32_t(now);
                packet.appendPackedTime(now);

                packet << uint32_t(lockouts.size());
                for (const auto& lockout : lockouts)
                    packet << lockout.mapId << lockout.difficulty << uint32_t(lockout.secondsLeft) << lockout.instanceId;

                packet << RaidResetOrigin;
                packet << uint32_t(raidResets.size());
                writeRaidResets(packet);

                packet << uint32_t(holidays.size());
                for (const auto& holiday : holidays)
                {
                    packet << holiday.id << holiday.region << holiday.looping << holiday.priority << holiday.filterType;
                    for (const uint32_t date : holiday.dates)
                        packet << date;
                    for (const uint32_t duration : holiday.durations)
                        packet << duration;
                    for (const uint32_t flag : holiday.flags)
                        packet << flag;
                    packet << holiday.texture;
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }

    private:
        // 7.x and later: server time and the three counts lead, the order of the lists changes with 10.x and 12.x
        bool writeCounted(WorldPacket& packet, time_t now)
        {
            CalendarWire::writeDate(packet, m_protocol, now);
            packet << uint32_t(invites.size());
            packet << uint32_t(events.size());
            packet << uint32_t(lockouts.size());

            if (m_protocol.expansion >= WoW::Expansion::_Midnight)
            {
                for (const auto& invite : invites)
                    writeInvite(packet, invite);
                for (const auto& calendarEvent : events)
                    writeEvent(packet, calendarEvent);
                for (const auto& lockout : lockouts)
                    writeLockout(packet, lockout);
            }
            else if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
            {
                for (const auto& lockout : lockouts)
                    writeLockout(packet, lockout);
                for (const auto& invite : invites)
                    writeInvite(packet, invite);
                for (const auto& calendarEvent : events)
                    writeEvent(packet, calendarEvent);
            }
            else
            {
                for (const auto& invite : invites)
                    writeInvite(packet, invite);
                for (const auto& lockout : lockouts)
                    writeLockout(packet, lockout);
                for (const auto& calendarEvent : events)
                    writeEvent(packet, calendarEvent);
            }
            return true;
        }

        void writeRaidResets(WorldPacket& packet) const
        {
            for (const auto& reset : raidResets)
                packet << int32_t(reset.mapId) << reset.secondsLeft << int32_t(0);
        }

        void writeInvite(WorldPacket& packet, CalendarInviteData const& invite) const
        {
            packet << invite.eventId << invite.inviteId << invite.status << invite.rank;
            packet << uint8_t(invite.guildMember ? 1 : 0);
            CalendarWire::writePlayer(packet, m_protocol, invite.senderGuid);

            // 10.x and later: friend and guild restrictions are never ignored
            if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
            {
                packet.writeBit(false);
                packet.flushBits();
            }
        }

        void writeEvent(WorldPacket& packet, CalendarEventData const& calendarEvent) const
        {
            packet << calendarEvent.eventId << calendarEvent.type;
            CalendarWire::writeDate(packet, m_protocol, calendarEvent.date);
            CalendarWire::writeFlags(packet, m_protocol, calendarEvent.flags);
            packet << calendarEvent.textureId;
            CalendarWire::writeEventGuild(packet, m_protocol, calendarEvent.guildId);
            CalendarWire::writePlayer(packet, m_protocol, calendarEvent.ownerGuid);
            packet.writeBits(calendarEvent.title.size(), 8);
            CalendarWire::writeRaw(packet, calendarEvent.title);
        }

        void writeLockout(WorldPacket& packet, CalendarLockoutData const& lockout) const
        {
            // 12.x: map and difficulty lead, the instance closes the entry
            if (m_protocol.expansion >= WoW::Expansion::_Midnight)
            {
                packet << int32_t(lockout.mapId) << int16_t(lockout.difficulty) << lockout.secondsLeft << lockout.instanceId;
                return;
            }

            packet << lockout.instanceId << int32_t(lockout.mapId) << lockout.difficulty << lockout.secondsLeft;
        }

        bool writeMop(WorldPacket& packet, time_t now)
        {
            packet.writeBits(raidResets.size(), 20);
            packet.writeBits(0, 16);                        // holidays
            packet.writeBits(lockouts.size(), 20);

            ByteBuffer lockoutBuffer;
            for (const auto& lockout : lockouts)
            {
                const WoWGuid instance(lockout.instanceId);
                packet.writeBit(instance[6]);
                packet.writeBit(instance[7]);
                packet.writeBit(instance[2]);
                packet.writeBit(instance[1]);
                packet.writeBit(instance[5]);
                packet.writeBit(instance[4]);
                packet.writeBit(instance[0]);
                packet.writeBit(instance[3]);

                lockoutBuffer << lockout.difficulty;
                lockoutBuffer.writeByteSeq(instance[3]);
                lockoutBuffer.writeByteSeq(instance[0]);
                lockoutBuffer.writeByteSeq(instance[1]);
                lockoutBuffer.writeByteSeq(instance[5]);
                lockoutBuffer << uint32_t(lockout.secondsLeft);
                lockoutBuffer << lockout.mapId;
                lockoutBuffer.writeByteSeq(instance[2]);
                lockoutBuffer.writeByteSeq(instance[7]);
                lockoutBuffer.writeByteSeq(instance[6]);
                lockoutBuffer.writeByteSeq(instance[4]);
            }

            packet.writeBits(invites.size(), 19);

            ByteBuffer invitesBuffer;
            for (const auto& invite : invites)
            {
                const WoWGuid sender = CalendarWire::playerGuid(invite.senderGuid);
                packet.writeBit(sender[1]);
                packet.writeBit(sender[2]);
                packet.writeBit(sender[6]);
                packet.writeBit(sender[7]);
                packet.writeBit(sender[3]);
                packet.writeBit(sender[0]);
                packet.writeBit(sender[4]);
                packet.writeBit(sender[5]);

                invitesBuffer.writeByteSeq(sender[2]);
                invitesBuffer << invite.inviteId;
                invitesBuffer << invite.status;
                invitesBuffer.writeByteSeq(sender[6]);
                invitesBuffer.writeByteSeq(sender[3]);
                invitesBuffer.writeByteSeq(sender[4]);
                invitesBuffer.writeByteSeq(sender[1]);
                invitesBuffer.writeByteSeq(sender[0]);
                invitesBuffer << invite.eventId;
                invitesBuffer.writeByteSeq(sender[7]);
                invitesBuffer.writeByteSeq(sender[5]);
                invitesBuffer << invite.rank;
                invitesBuffer << uint8_t(invite.guildMember ? 1 : 0);
            }

            packet.writeBits(events.size(), 19);

            ByteBuffer eventsBuffer;
            for (const auto& calendarEvent : events)
            {
                const WoWGuid owner = CalendarWire::playerGuid(calendarEvent.ownerGuid);
                const WoWGuid guild = CalendarWire::guildGuid(calendarEvent.guildId);

                packet.writeBit(owner[2]);
                packet.writeBit(guild[1]);
                packet.writeBit(guild[7]);
                packet.writeBit(owner[4]);
                packet.writeBit(guild[5]);
                packet.writeBit(guild[6]);
                packet.writeBit(guild[3]);
                packet.writeBit(guild[4]);
                packet.writeBit(owner[7]);
                packet.writeBits(calendarEvent.title.size(), 8);
                packet.writeBit(owner[1]);
                packet.writeBit(guild[2]);
                packet.writeBit(guild[0]);
                packet.writeBit(owner[0]);
                packet.writeBit(owner[3]);
                packet.writeBit(owner[6]);
                packet.writeBit(owner[5]);

                eventsBuffer.writeByteSeq(owner[5]);
                eventsBuffer.writeByteSeq(guild[3]);
                CalendarWire::writeRaw(eventsBuffer, calendarEvent.title);
                eventsBuffer.writeByteSeq(guild[7]);
                eventsBuffer << calendarEvent.textureId;
                eventsBuffer.writeByteSeq(owner[0]);
                eventsBuffer.writeByteSeq(owner[4]);
                eventsBuffer.writeByteSeq(guild[2]);
                eventsBuffer.writeByteSeq(owner[7]);
                eventsBuffer.writeByteSeq(owner[2]);
                eventsBuffer.appendPackedTime(calendarEvent.date);
                eventsBuffer.writeByteSeq(owner[3]);
                eventsBuffer.writeByteSeq(owner[1]);
                eventsBuffer.writeByteSeq(guild[6]);
                eventsBuffer.writeByteSeq(guild[1]);
                eventsBuffer.writeByteSeq(owner[6]);
                eventsBuffer << calendarEvent.flags;
                eventsBuffer.writeByteSeq(guild[4]);
                eventsBuffer.writeByteSeq(guild[5]);
                eventsBuffer.writeByteSeq(guild[0]);
                eventsBuffer << calendarEvent.eventId;
                eventsBuffer << calendarEvent.type;
            }

            packet.flushBits();
            packet.append(eventsBuffer);
            packet.append(lockoutBuffer);
            packet.append(invitesBuffer);
            packet.appendPackedTime(now);                   // zone time
            writeRaidResets(packet);
            packet << RaidResetOrigin;
            packet << uint32_t(now);                        // server time
            return true;
        }
    };
}
