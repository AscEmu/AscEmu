/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class CmsgCalendarUpdateEvent : public ManagedPacket
    {
    public:
        uint32_t eventId = 0;
        uint32_t inviteId = 0;
        std::string title;
        std::string description;
        uint8_t type = 0;
        uint32_t dungeonId = 0;
        uint32_t eventPackedTime = 0;
        uint32_t flags = 0;

        CmsgCalendarUpdateEvent() :
            ManagedPacket(CMSG_CALENDAR_UPDATE_EVENT, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                uint32_t maxInvites;
                int32_t signedDungeonId;

                packet >> maxInvites >> signedDungeonId;
                packet.readPackedTime(eventPackedTime);
                packet >> flags >> type;

                dungeonId = static_cast<uint32_t>(signedDungeonId);

                WoWGuid eventGuid;
                WoWGuid inviteGuid;

                eventGuid[4] = packet.readBit();
                eventGuid[5] = packet.readBit();
                eventGuid[2] = packet.readBit();
                inviteGuid[4] = packet.readBit();
                eventGuid[7] = packet.readBit();
                eventGuid[0] = packet.readBit();
                inviteGuid[5] = packet.readBit();
                inviteGuid[3] = packet.readBit();
                eventGuid[6] = packet.readBit();
                eventGuid[1] = packet.readBit();
                inviteGuid[6] = packet.readBit();
                inviteGuid[2] = packet.readBit();
                inviteGuid[7] = packet.readBit();
                inviteGuid[1] = packet.readBit();
                inviteGuid[0] = packet.readBit();

                const uint32_t descriptionLength = static_cast<uint32_t>(packet.readBits(11));
                const uint32_t titleLength = static_cast<uint32_t>(packet.readBits(8));

                eventGuid[3] = packet.readBit();

                packet.readByteSeq(inviteGuid[6]);
                packet.readByteSeq(eventGuid[0]);
                packet.readByteSeq(inviteGuid[7]);
                packet.readByteSeq(inviteGuid[3]);
                packet.readByteSeq(eventGuid[6]);
                packet.readByteSeq(inviteGuid[1]);
                packet.readByteSeq(eventGuid[2]);

                title = packet.readString(titleLength);

                packet.readByteSeq(inviteGuid[5]);
                packet.readByteSeq(inviteGuid[4]);
                packet.readByteSeq(eventGuid[5]);
                packet.readByteSeq(eventGuid[3]);
                packet.readByteSeq(inviteGuid[0]);
                packet.readByteSeq(eventGuid[4]);

                description = packet.readString(descriptionLength);

                packet.readByteSeq(eventGuid[1]);
                packet.readByteSeq(inviteGuid[2]);
                packet.readByteSeq(eventGuid[7]);

                eventId = eventGuid.getGuidLowPart();
                inviteId = inviteGuid.getGuidLowPart();

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t id;
                uint64_t invId;
                uint8_t repetitionType;
                uint32_t maxInvites;
                int32_t signedDungeonId;
                uint32_t timeZoneTime;

                packet >> id >> invId >> title >> description >> type >> repetitionType >> maxInvites >> signedDungeonId;
                packet.readPackedTime(eventPackedTime);
                packet.readPackedTime(timeZoneTime);
                packet >> flags;

                eventId = static_cast<uint32_t>(id);
                inviteId = static_cast<uint32_t>(invId);
                dungeonId = static_cast<uint32_t>(signedDungeonId);

                return true;
            }

            return false;
        }
    };
}
