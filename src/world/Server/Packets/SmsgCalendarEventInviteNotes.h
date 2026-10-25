/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    // 3.x: the text of an invite changed
    class SmsgCalendarEventInviteNotes : public ManagedPacket
    {
    public:
        uint32_t inviteeGuid = 0;
        uint64_t eventId = 0;
        std::string notes;
        bool clearPending = false;

        SmsgCalendarEventInviteNotes(uint32_t inviteeGuid, uint64_t eventId, std::string notes, bool clearPending) :
            ManagedPacket(SMSG_CALENDAR_EVENT_INVITE_NOTES, 9 + 8 + notes.size() + 1 + 1),
            inviteeGuid(inviteeGuid),
            eventId(eventId),
            notes(std::move(notes)),
            clearPending(clearPending)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion != WoW::Expansion::_WotLK)
                return false;

            CalendarWire::writePlayer(packet, m_protocol, inviteeGuid);
            packet << eventId << notes << uint8_t(clearPending ? 1 : 0);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
