/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // a moderator sets the status (or with moderatorRank the rank) of an invite
    class CmsgCalendarEventStatus : public ManagedPacket
    {
    public:
        uint32_t inviteeGuid = 0;
        uint32_t eventId = 0;
        uint32_t inviteId = 0;
        uint8_t value = 0;                  // invite status or moderation rank

        explicit CmsgCalendarEventStatus(bool moderatorRank = false) :
            ManagedPacket(moderatorRank ? CMSG_CALENDAR_EVENT_MODERATOR_STATUS : CMSG_CALENDAR_EVENT_STATUS, 1 + 8 + 8 + 8 + 1),
            m_moderatorRank(moderatorRank)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t id;
            uint64_t invite;

            if (m_protocol.isMop())
                return readMop(packet);

            inviteeGuid = CalendarWire::readPlayer(packet, m_protocol);
            packet >> id >> invite;
            packet.readSkip<uint64_t>();        // moderator invite
            packet >> value;

            eventId = static_cast<uint32_t>(id);
            inviteId = static_cast<uint32_t>(invite);
            return true;
        }

    private:
        bool m_moderatorRank;

        bool readMop(WorldPacket& packet)
        {
            uint64_t id;
            uint64_t invite;
            WoWGuid invitee;

            if (m_moderatorRank)
            {
                packet >> value >> id;
                packet.readSkip<uint64_t>();    // moderator invite
                packet >> invite;

                invitee[6] = packet.readBit();
                invitee[5] = packet.readBit();
                invitee[1] = packet.readBit();
                invitee[3] = packet.readBit();
                invitee[4] = packet.readBit();
                invitee[7] = packet.readBit();
                invitee[0] = packet.readBit();
                invitee[2] = packet.readBit();

                packet.readByteSeq(invitee[7]);
                packet.readByteSeq(invitee[5]);
                packet.readByteSeq(invitee[0]);
                packet.readByteSeq(invitee[4]);
                packet.readByteSeq(invitee[1]);
                packet.readByteSeq(invitee[3]);
                packet.readByteSeq(invitee[2]);
                packet.readByteSeq(invitee[6]);
            }
            else
            {
                packet.readSkip<uint64_t>();    // moderator invite
                packet >> id >> invite >> value;

                invitee[4] = packet.readBit();
                invitee[3] = packet.readBit();
                invitee[7] = packet.readBit();
                invitee[6] = packet.readBit();
                invitee[2] = packet.readBit();
                invitee[0] = packet.readBit();
                invitee[5] = packet.readBit();
                invitee[1] = packet.readBit();

                packet.readByteSeq(invitee[7]);
                packet.readByteSeq(invitee[5]);
                packet.readByteSeq(invitee[2]);
                packet.readByteSeq(invitee[1]);
                packet.readByteSeq(invitee[4]);
                packet.readByteSeq(invitee[6]);
                packet.readByteSeq(invitee[0]);
                packet.readByteSeq(invitee[3]);
            }

            inviteeGuid = invitee.getLowGuid();
            eventId = static_cast<uint32_t>(id);
            inviteId = static_cast<uint32_t>(invite);
            return true;
        }
    };
}
