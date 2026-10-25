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
    // 3.x: the invited player learns the new text of the invite
    class SmsgCalendarEventInviteNotesAlert : public ManagedPacket
    {
    public:
        uint64_t eventId = 0;
        std::string notes;

        SmsgCalendarEventInviteNotesAlert(uint64_t eventId, std::string notes) :
            ManagedPacket(SMSG_CALENDAR_EVENT_INVITE_NOTES_ALERT, 8 + notes.size() + 1),
            eventId(eventId),
            notes(std::move(notes))
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion != WoW::Expansion::_WotLK)
                return false;

            packet << eventId << notes;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
