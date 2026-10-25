/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>
#include <ctime>

namespace AscEmu::Packets
{
    // an event was changed, originalDate is its date before the change
    class SmsgCalendarEventUpdatedAlert : public ManagedPacket
    {
    public:
        CalendarEventData calendarEvent;
        time_t originalDate = 0;
        bool clearPending = false;

        SmsgCalendarEventUpdatedAlert(CalendarEventData calendarEvent, time_t originalDate, bool clearPending) :
            ManagedPacket(SMSG_CALENDAR_EVENT_UPDATED_ALERT, 0),
            calendarEvent(std::move(calendarEvent)),
            originalDate(originalDate),
            clearPending(clearPending)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 8.x and later name the club of the event first
                if (m_protocol.expansion >= WoW::Expansion::_BfA)
                    packet << uint64_t(calendarEvent.guildId);

                packet << calendarEvent.eventId;
                CalendarWire::writeDate(packet, m_protocol, originalDate);
                CalendarWire::writeDate(packet, m_protocol, calendarEvent.date);

                // the event is never locked
                if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
                    CalendarWire::writeUnsetDate(packet, m_protocol);
                else
                    packet << uint32_t(0);

                CalendarWire::writeFlags(packet, m_protocol, calendarEvent.flags);
                packet << uint32_t(calendarEvent.textureId) << calendarEvent.type;
                packet.writeBits(calendarEvent.title.size(), 8);
                packet.writeBits(calendarEvent.description.size(), 11);
                packet.writeBit(clearPending);
                CalendarWire::writeRaw(packet, calendarEvent.title);
                CalendarWire::writeRaw(packet, calendarEvent.description);
                return true;
            }

            if (m_protocol.isMop())
            {
                packet << calendarEvent.flags << calendarEvent.type;
                packet << uint32_t(0);                      // lock date
                packet.appendPackedTime(originalDate);
                packet << calendarEvent.eventId;
                packet.appendPackedTime(calendarEvent.date);
                packet << calendarEvent.textureId;
                packet.writeBit(clearPending);
                packet.writeBits(calendarEvent.description.size(), 11);
                packet.writeBits(calendarEvent.title.size(), 8);
                CalendarWire::writeRaw(packet, calendarEvent.title);
                CalendarWire::writeRaw(packet, calendarEvent.description);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint8_t(clearPending ? 1 : 0);
                packet << calendarEvent.eventId;
                packet.appendPackedTime(originalDate);
                packet << calendarEvent.flags;
                packet.appendPackedTime(calendarEvent.date);
                packet << calendarEvent.type;
                packet << calendarEvent.textureId;
                packet << calendarEvent.title;
                packet << calendarEvent.description;
                packet << uint8_t(0);                       // never repeats
                packet << uint32_t(CALENDAR_MAX_INVITES);
                packet << uint32_t(0);                      // lock date
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
