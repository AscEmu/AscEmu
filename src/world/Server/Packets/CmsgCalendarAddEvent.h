/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct CalendarAddEventInvitee
    {
        WoWGuid guid;
        uint8_t status = 0;
        uint8_t rank = 0;
    };

    class CmsgCalendarAddEvent : public ManagedPacket
    {
    public:
        std::string title;
        std::string description;
        uint8_t type = 0;
        int32_t dungeonId = 0;
        uint32_t eventPackedTime = 0;
        uint32_t flags = 0;
        std::vector<CalendarAddEventInvitee> invitees;

        CmsgCalendarAddEvent() :
            ManagedPacket(CMSG_CALENDAR_ADD_EVENT, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                uint32_t maxInvites;

                packet >> maxInvites >> flags >> dungeonId;
                packet.readPackedTime(eventPackedTime);
                packet >> type;

                const uint32_t inviteeCount = static_cast<uint32_t>(packet.readBits(22));
                const uint32_t descriptionLength = static_cast<uint32_t>(packet.readBits(11));

                // This first invitee block is present on the wire (required to stay byte-aligned)
                // but is not something the reference server implementation acts on.
                std::vector<WoWGuid> unusedInvitees(inviteeCount);
                for (uint32_t i = 0; i < inviteeCount; ++i)
                {
                    unusedInvitees[i][7] = packet.readBit();
                    unusedInvitees[i][2] = packet.readBit();
                    unusedInvitees[i][6] = packet.readBit();
                    unusedInvitees[i][3] = packet.readBit();
                    unusedInvitees[i][5] = packet.readBit();
                    unusedInvitees[i][1] = packet.readBit();
                    unusedInvitees[i][0] = packet.readBit();
                    unusedInvitees[i][4] = packet.readBit();
                }

                const uint32_t titleLength = static_cast<uint32_t>(packet.readBits(8));

                for (uint32_t i = 0; i < inviteeCount; ++i)
                {
                    uint8_t unusedStatus;
                    uint8_t unusedRank;

                    packet.readByteSeq(unusedInvitees[i][4]);
                    packet.readByteSeq(unusedInvitees[i][2]);
                    packet.readByteSeq(unusedInvitees[i][3]);
                    packet.readByteSeq(unusedInvitees[i][1]);
                    packet.readByteSeq(unusedInvitees[i][0]);
                    packet.readByteSeq(unusedInvitees[i][6]);
                    packet.readByteSeq(unusedInvitees[i][7]);
                    packet >> unusedStatus;
                    packet.readByteSeq(unusedInvitees[i][5]);
                    packet >> unusedRank;
                }

                title = packet.readString(titleLength);
                description = packet.readString(descriptionLength);

                uint32_t inviteCount;
                packet >> inviteCount;

                invitees.resize(inviteCount);
                for (uint32_t i = 0; i < inviteCount; ++i)
                {
                    invitees[i].guid.init(packet.unpackGuid());
                    packet >> invitees[i].status >> invitees[i].rank;
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint8_t repeatable;
                uint32_t maxInvites;

                packet >> title >> description >> type >> repeatable >> maxInvites >> dungeonId;
                packet.readPackedTime(eventPackedTime);
                packet.readSkip<uint32_t>(); // unkPackedTime
                packet >> flags;

                uint32_t inviteCount;
                packet >> inviteCount;

                invitees.resize(inviteCount);
                for (uint32_t i = 0; i < inviteCount; ++i)
                {
                    uint64_t invitee = 0;
                    packet >> invitee;
                    invitees[i].guid.init(invitee);
                    packet >> invitees[i].status >> invitees[i].rank;
                }

                return true;
            }

            return false;
        }
    };
}
