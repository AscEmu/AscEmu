/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgCalendarEventRemoveInvite : public ManagedPacket
    {
    public:
        uint32_t inviteeGuid = 0;
        uint32_t inviteId = 0;
        uint32_t eventId = 0;

        CmsgCalendarEventRemoveInvite() :
            ManagedPacket(CMSG_CALENDAR_EVENT_REMOVE_INVITE, 1 + 8 + 8 + 8)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t invite;
            uint64_t id;

            if (m_protocol.isMop())
            {
                packet >> invite;
                packet.readSkip<uint64_t>();    // moderator invite
                packet >> id;

                WoWGuid invitee;
                invitee[6] = packet.readBit();
                invitee[3] = packet.readBit();
                invitee[2] = packet.readBit();
                invitee[4] = packet.readBit();
                invitee[5] = packet.readBit();
                invitee[7] = packet.readBit();
                invitee[0] = packet.readBit();
                invitee[1] = packet.readBit();

                packet.readByteSeq(invitee[0]);
                packet.readByteSeq(invitee[4]);
                packet.readByteSeq(invitee[7]);
                packet.readByteSeq(invitee[3]);
                packet.readByteSeq(invitee[5]);
                packet.readByteSeq(invitee[1]);
                packet.readByteSeq(invitee[2]);
                packet.readByteSeq(invitee[6]);

                inviteeGuid = invitee.getLowGuid();
            }
            else
            {
                inviteeGuid = CalendarWire::readPlayer(packet, m_protocol);
                packet >> invite;
                packet.readSkip<uint64_t>();    // moderator invite
                packet >> id;
            }

            inviteId = static_cast<uint32_t>(invite);
            eventId = static_cast<uint32_t>(id);
            return true;
        }
    };
}
