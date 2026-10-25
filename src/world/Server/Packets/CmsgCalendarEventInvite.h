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
    class CmsgCalendarEventInvite : public ManagedPacket
    {
    public:
        uint32_t eventId = 0;
        std::string name;
        bool isPreInvite = false;           // the event is still in the planner, it has no id yet
        bool isSignUp = false;              // the planner prepares a guild sign up

        CmsgCalendarEventInvite() :
            ManagedPacket(CMSG_CALENDAR_EVENT_INVITE, 8 + 8)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t id;
            packet >> id;
            packet.readSkip<uint64_t>();        // moderator invite
            eventId = static_cast<uint32_t>(id);

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint8_t preInvite;
                uint8_t signUp;
                packet >> name >> preInvite >> signUp;
                isPreInvite = preInvite != 0;
                isSignUp = signUp != 0;
                return true;
            }

            if (m_protocol.isMop())
            {
                isPreInvite = packet.readBit();
                const uint32_t nameLength = packet.readBits(9);
                isSignUp = packet.readBit();
                name = packet.readString(nameLength);
                return true;
            }

            // 8.x and later: the club of the event
            if (m_protocol.expansion >= WoW::Expansion::_BfA)
                packet.readSkip<uint64_t>();

            const uint32_t nameLength = packet.readBits(9);
            if (m_protocol.expansion >= WoW::Expansion::_Midnight)
            {
                // 12.x sends the sign up bit first
                isSignUp = packet.readBit();
                isPreInvite = packet.readBit();
            }
            else
            {
                isPreInvite = packet.readBit();
                isSignUp = packet.readBit();
            }

            // no event has the id 0, the planner has not saved it yet
            if (eventId == 0)
                isPreInvite = true;

            packet.resetBitPos();
            name = packet.readString(nameLength);
            return true;
        }
    };
}
