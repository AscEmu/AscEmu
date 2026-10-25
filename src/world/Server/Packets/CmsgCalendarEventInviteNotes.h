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
    // 3.x: the text of an invite changes
    class CmsgCalendarEventInviteNotes : public ManagedPacket
    {
    public:
        uint32_t inviteeGuid = 0;
        uint64_t id = 0;                    // event or invite of the invited player
        std::string notes;

        CmsgCalendarEventInviteNotes() :
            ManagedPacket(CMSG_CALENDAR_EVENT_INVITE_NOTES, 9 + 8 + 1 + 1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion != WoW::Expansion::_WotLK)
                return false;

            uint8_t unused;
            inviteeGuid = CalendarWire::readPlayer(packet, m_protocol);
            packet >> id >> notes >> unused;
            return true;
        }
    };
}
