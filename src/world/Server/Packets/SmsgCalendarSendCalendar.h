/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#if VERSION_STRING > TBC

#include "ManagedPacket.h"
#include "Management/CalendarMgr.hpp"
#include "Management/Guild/Guild.hpp"
#include "Management/Guild/GuildMgr.hpp"
#include "Utilities/Util.hpp"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgCalendarSendCalendar : public ManagedPacket
    {
    public:
        std::vector<CalendarInvite*> invites;
        std::vector<CalendarEvent*> events;
        uint32_t requestingPlayerGuildId = 0;

        SmsgCalendarSendCalendar(std::vector<CalendarInvite*> invites, std::vector<CalendarEvent*> events, uint32_t requestingPlayerGuildId) :
            ManagedPacket(SMSG_CALENDAR_SEND_CALENDAR, 0),
            invites(std::move(invites)),
            events(std::move(events)),
            requestingPlayerGuildId(requestingPlayerGuildId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            const uint32_t currTime = static_cast<uint32_t>(::Util::getTimeNow());

            if (m_protocol.isMop())
            {
                // No raid-lockout or holiday overlay data - out of scope, sent as empty lists.
                packet.writeBits(0, 20);   // raid lockout count placeholder
                packet.writeBits(0, 16);   // holiday count
                packet.writeBits(0, 20);   // bound instance count

                packet.writeBits(invites.size(), 19);
                ByteBuffer invitesInfoBuffer;
                for (const auto* invite : invites)
                {
                    const WoWGuid senderGuid(uint64_t(invite->m_sender));

                    packet.writeBit(senderGuid[1]);
                    packet.writeBit(senderGuid[2]);
                    packet.writeBit(senderGuid[6]);
                    packet.writeBit(senderGuid[7]);
                    packet.writeBit(senderGuid[3]);
                    packet.writeBit(senderGuid[0]);
                    packet.writeBit(senderGuid[4]);
                    packet.writeBit(senderGuid[5]);

                    invitesInfoBuffer.writeByteSeq(senderGuid[2]);
                    invitesInfoBuffer << uint64_t(invite->m_inviteId);
                    invitesInfoBuffer << uint8_t(invite->m_status);
                    invitesInfoBuffer.writeByteSeq(senderGuid[6]);
                    invitesInfoBuffer.writeByteSeq(senderGuid[3]);
                    invitesInfoBuffer.writeByteSeq(senderGuid[4]);
                    invitesInfoBuffer.writeByteSeq(senderGuid[1]);
                    invitesInfoBuffer.writeByteSeq(senderGuid[0]);
                    invitesInfoBuffer << uint64_t(invite->m_event);
                    invitesInfoBuffer.writeByteSeq(senderGuid[7]);
                    invitesInfoBuffer.writeByteSeq(senderGuid[5]);
                    invitesInfoBuffer << uint8_t(invite->m_rank);

                    const auto* linkedEvent = sCalendarMgr.getEvent(invite->m_event);
                    invitesInfoBuffer << uint8_t(linkedEvent != nullptr && linkedEvent->isGuildEvent() && linkedEvent->m_guildId == requestingPlayerGuildId ? 1 : 0);
                }

                packet.writeBits(events.size(), 19);
                ByteBuffer eventsInfoBuffer;
                for (const auto* calendarEvent : events)
                {
                    const WoWGuid creatorGuid(uint64_t(calendarEvent->m_creator));
                    Guild* guild = calendarEvent->isGuildEvent() ? sGuildMgr.getGuildById(calendarEvent->m_guildId) : nullptr;
                    const WoWGuid guildGuid(guild != nullptr ? guild->getGUID() : uint64_t(0));

                    packet.writeBit(creatorGuid[2]);
                    packet.writeBit(guildGuid[1]);
                    packet.writeBit(guildGuid[7]);
                    packet.writeBit(creatorGuid[4]);
                    packet.writeBit(guildGuid[5]);
                    packet.writeBit(guildGuid[6]);
                    packet.writeBit(guildGuid[3]);
                    packet.writeBit(guildGuid[4]);
                    packet.writeBit(creatorGuid[7]);
                    packet.writeBits(calendarEvent->m_title.size(), 8);
                    packet.writeBit(creatorGuid[1]);
                    packet.writeBit(guildGuid[2]);
                    packet.writeBit(guildGuid[0]);
                    packet.writeBit(creatorGuid[0]);
                    packet.writeBit(creatorGuid[3]);
                    packet.writeBit(creatorGuid[6]);
                    packet.writeBit(creatorGuid[5]);

                    eventsInfoBuffer.writeByteSeq(creatorGuid[5]);
                    eventsInfoBuffer.writeByteSeq(guildGuid[3]);

                    if (calendarEvent->m_title.length())
                        eventsInfoBuffer.append(reinterpret_cast<const uint8_t*>(calendarEvent->m_title.c_str()), calendarEvent->m_title.length() + 1);
                    else
                        eventsInfoBuffer << uint8_t(0);

                    eventsInfoBuffer.writeByteSeq(guildGuid[7]);
                    eventsInfoBuffer << int32_t(calendarEvent->m_dungeon);
                    eventsInfoBuffer.writeByteSeq(creatorGuid[0]);
                    eventsInfoBuffer.writeByteSeq(creatorGuid[4]);
                    eventsInfoBuffer.writeByteSeq(guildGuid[2]);
                    eventsInfoBuffer.writeByteSeq(creatorGuid[7]);
                    eventsInfoBuffer.writeByteSeq(creatorGuid[2]);
                    eventsInfoBuffer.appendPackedTime(calendarEvent->m_date);
                    eventsInfoBuffer.writeByteSeq(creatorGuid[3]);
                    eventsInfoBuffer.writeByteSeq(creatorGuid[1]);
                    eventsInfoBuffer.writeByteSeq(guildGuid[6]);
                    eventsInfoBuffer.writeByteSeq(guildGuid[1]);
                    eventsInfoBuffer.writeByteSeq(creatorGuid[6]);
                    eventsInfoBuffer << uint32_t(calendarEvent->m_flags);
                    eventsInfoBuffer.writeByteSeq(guildGuid[4]);
                    eventsInfoBuffer.writeByteSeq(guildGuid[5]);
                    eventsInfoBuffer.writeByteSeq(guildGuid[0]);
                    eventsInfoBuffer << uint64_t(calendarEvent->m_entry);
                    eventsInfoBuffer << uint8_t(calendarEvent->m_type);
                }

                packet.flushBits();
                packet.append(eventsInfoBuffer);
                packet.append(invitesInfoBuffer);
                packet.appendPackedTime(currTime); // zone time

                packet << uint32_t(1135753200); // constant date, unknown
                packet << uint32_t(currTime);   // server time

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint32_t(invites.size());
                for (const auto* invite : invites)
                {
                    packet << uint64_t(invite->m_event);
                    packet << uint64_t(invite->m_inviteId);
                    packet << uint8_t(invite->m_status);
                    packet << uint8_t(invite->m_rank);

                    if (const auto* linkedEvent = sCalendarMgr.getEvent(invite->m_event))
                    {
                        packet << uint8_t(linkedEvent->isGuildEvent());
                        packet << WoWGuid(uint64_t(linkedEvent->m_creator));
                    }
                    else
                    {
                        packet << uint8_t(0);
                        packet << WoWGuid(uint64_t(invite->m_sender));
                    }
                }

                packet << uint32_t(events.size());
                for (const auto* calendarEvent : events)
                {
                    packet << uint64_t(calendarEvent->m_entry);
                    packet << calendarEvent->m_title;
                    packet << uint32_t(calendarEvent->m_type);
                    packet.appendPackedTime(calendarEvent->m_date);
                    packet << uint32_t(calendarEvent->m_flags);
                    packet << int32_t(calendarEvent->m_dungeon);

                    if (m_protocol.isCata())
                    {
                        Guild* guild = calendarEvent->isGuildEvent() ? sGuildMgr.getGuildById(calendarEvent->m_guildId) : nullptr;
                        packet << uint64_t(guild != nullptr ? guild->getGUID() : uint64_t(0));
                    }

                    packet << WoWGuid(uint64_t(calendarEvent->m_creator));
                }

                packet << uint32_t(currTime); // server time
                packet.appendPackedTime(currTime); // zone time

                packet << uint32_t(0); // bound instance count
                packet << uint32_t(1135753200); // constant date, unknown
                packet << uint32_t(0); // raid reset count
                packet << uint32_t(0); // holiday count

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}

#endif
