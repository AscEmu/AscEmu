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
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCalendarSendEvent : public ManagedPacket
    {
    public:
        uint8_t sendType = 0;
        const CalendarEvent* calendarEvent = nullptr;
        std::vector<CalendarInvite*> invites;

        SmsgCalendarSendEvent(uint8_t sendType, const CalendarEvent* calendarEvent, std::vector<CalendarInvite*> invites) :
            ManagedPacket(SMSG_CALENDAR_SEND_EVENT, 0),
            sendType(sendType),
            calendarEvent(calendarEvent),
            invites(std::move(invites))
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (calendarEvent == nullptr)
                return false;

            const WoWGuid creatorGuid(uint64_t(calendarEvent->m_creator));
            Guild* guild = calendarEvent->isGuildEvent() ? sGuildMgr.getGuildById(calendarEvent->m_guildId) : nullptr;
            const WoWGuid guildGuid(guild != nullptr ? guild->getGUID() : uint64_t(0));

            if (m_protocol.isMop())
            {
                packet.writeBits(invites.size(), 20);

                ByteBuffer inviteeData;
                for (const auto* invite : invites)
                {
                    const WoWGuid inviteeGuid(uint64_t(invite->m_invitee));

                    packet.writeBit(inviteeGuid[1]);
                    packet.writeBit(inviteeGuid[2]);
                    packet.writeBit(inviteeGuid[0]);
                    packet.writeBit(inviteeGuid[7]);
                    packet.writeBit(inviteeGuid[3]);
                    packet.writeBit(inviteeGuid[5]);
                    packet.writeBit(inviteeGuid[6]);
                    packet.writeBits(invite->m_text.size(), 8);
                    packet.writeBit(inviteeGuid[4]);

                    if (invite->m_statusTime)
                        inviteeData.appendPackedTime(invite->m_statusTime);
                    else
                        inviteeData << uint32_t(0);

                    inviteeData.writeByteSeq(inviteeGuid[5]);
                    inviteeData << uint8_t(calendarEvent->isGuildEvent() ? 1 : 0);
                    inviteeData.writeByteSeq(inviteeGuid[1]);
                    inviteeData.writeByteSeq(inviteeGuid[2]);
                    inviteeData.writeByteSeq(inviteeGuid[6]);

                    if (invite->m_text.length())
                        inviteeData.append(reinterpret_cast<const uint8_t*>(invite->m_text.c_str()), invite->m_text.length() + 1);
                    else
                        inviteeData << uint8_t(0);

                    inviteeData << uint8_t(0); // invitee level, not tracked
                    inviteeData.writeByteSeq(inviteeGuid[7]);
                    inviteeData << uint8_t(invite->m_rank);
                    inviteeData << uint64_t(invite->m_inviteId);
                    inviteeData.writeByteSeq(inviteeGuid[0]);
                    inviteeData.writeByteSeq(inviteeGuid[3]);
                    inviteeData.writeByteSeq(inviteeGuid[4]);
                    inviteeData << uint8_t(invite->m_status);
                }

                packet.writeBits(calendarEvent->m_title.size(), 8);
                packet.writeBit(creatorGuid[2]);
                packet.writeBit(creatorGuid[0]);
                packet.writeBit(guildGuid[4]);
                packet.writeBit(guildGuid[5]);
                packet.writeBit(creatorGuid[1]);
                packet.writeBit(creatorGuid[5]);
                packet.writeBit(creatorGuid[3]);
                packet.writeBit(guildGuid[6]);
                packet.writeBits(calendarEvent->m_description.size(), 11);
                packet.writeBit(guildGuid[1]);
                packet.writeBit(guildGuid[7]);
                packet.writeBit(creatorGuid[6]);
                packet.writeBit(guildGuid[2]);
                packet.writeBit(guildGuid[0]);
                packet.writeBit(creatorGuid[4]);
                packet.writeBit(guildGuid[3]);
                packet.writeBit(creatorGuid[7]);
                packet.flushBits();

                packet.append(inviteeData);
                packet.writeByteSeq(creatorGuid[0]);
                packet.writeByteSeq(creatorGuid[5]);
                packet << uint32_t(calendarEvent->m_flags);
                packet.writeByteSeq(guildGuid[1]);
                packet.writeByteSeq(creatorGuid[7]);
                packet.writeByteSeq(creatorGuid[3]);
                packet.appendPackedTime(calendarEvent->m_date);
                packet.writeByteSeq(guildGuid[6]);
                packet.writeByteSeq(creatorGuid[4]);
                packet.writeByteSeq(creatorGuid[6]);
                packet << int32_t(calendarEvent->m_dungeon);
                packet << uint32_t(0); // lock date, always 0
                packet.writeByteSeq(guildGuid[4]);
                packet << uint8_t(calendarEvent->m_type);

                if (calendarEvent->m_title.length())
                    packet.append(reinterpret_cast<const uint8_t*>(calendarEvent->m_title.c_str()), calendarEvent->m_title.length() + 1);
                else
                    packet << uint8_t(0);

                packet.writeByteSeq(guildGuid[3]);
                packet.writeByteSeq(creatorGuid[2]);
                packet.writeByteSeq(guildGuid[7]);
                packet.writeByteSeq(creatorGuid[1]);

                if (calendarEvent->m_description.length())
                    packet.append(reinterpret_cast<const uint8_t*>(calendarEvent->m_description.c_str()), calendarEvent->m_description.length() + 1);
                else
                    packet << uint8_t(0);

                packet.writeByteSeq(guildGuid[2]);
                packet.writeByteSeq(guildGuid[5]);
                packet.writeByteSeq(guildGuid[0]);
                packet << uint64_t(calendarEvent->m_entry);
                packet << uint8_t(sendType);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint8_t(sendType);
                packet << creatorGuid;
                packet << uint64_t(calendarEvent->m_entry);
                packet << calendarEvent->m_title;
                packet << calendarEvent->m_description;
                packet << uint8_t(calendarEvent->m_type);
                packet << uint8_t(0); // repeatable
                packet << uint32_t(100); // max invites
                packet << int32_t(calendarEvent->m_dungeon);
                packet << uint32_t(calendarEvent->m_flags);
                packet.appendPackedTime(calendarEvent->m_date);       // event time
                packet.appendPackedTime(calendarEvent->m_date);       // time zone time, unknown - reuse event time

                if (m_protocol.isCata())
                    packet << uint64_t(guild != nullptr ? guild->getGUID() : uint64_t(0));
                else
                    packet << uint32_t(calendarEvent->m_guildId);

                packet << uint32_t(invites.size());
                for (const auto* invite : invites)
                {
                    const WoWGuid inviteeGuid(uint64_t(invite->m_invitee));

                    packet << inviteeGuid;
                    packet << uint8_t(0); // invitee level, not tracked
                    packet << uint8_t(invite->m_status);
                    packet << uint8_t(invite->m_rank);
                    packet << uint8_t(calendarEvent->isGuildEvent() ? 1 : 0);
                    packet << uint64_t(invite->m_inviteId);
                    packet.appendPackedTime(invite->m_statusTime);
                    packet << invite->m_text;
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}

#endif
