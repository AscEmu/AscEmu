/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct CalendarAddEventInvitee
    {
        uint32_t guid = 0;              // low guid of the invited player
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
        uint32_t eventTime = 0;
        uint32_t flags = 0;
        std::vector<CalendarAddEventInvitee> invitees;

        CmsgCalendarAddEvent() :
            ManagedPacket(CMSG_CALENDAR_ADD_EVENT, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_BfA)
                return readClubLayout(packet);

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x and 7.x: the string lengths lead, the invites follow the strings
                const uint32_t titleLength = packet.readBits(8);
                const uint32_t descriptionLength = packet.readBits(11);
                packet.resetBitPos();

                packet >> type >> dungeonId;
                eventTime = packet.readPackedTime();
                packet >> flags;

                uint32_t inviteCount;
                packet >> inviteCount;
                if (inviteCount > CALENDAR_MAX_INVITES)
                    return false;

                title = packet.readString(titleLength);
                description = packet.readString(descriptionLength);

                invitees.resize(inviteCount);
                for (auto& invitee : invitees)
                {
                    invitee.guid = CalendarWire::readPlayer(packet, m_protocol);
                    packet >> invitee.status >> invitee.rank;
                }

                uint32_t maxSize;
                packet >> maxSize;
                return true;
            }

            if (m_protocol.isMop())
            {
                uint32_t maxInvites;
                packet >> maxInvites >> flags >> dungeonId;
                eventTime = packet.readPackedTime();
                packet >> type;

                const uint32_t inviteCount = packet.readBits(22);
                const uint32_t descriptionLength = packet.readBits(11);
                if (inviteCount > CALENDAR_MAX_INVITES)
                    return false;

                std::vector<WoWGuid> guids(inviteCount);
                for (auto& guid : guids)
                {
                    guid[7] = packet.readBit();
                    guid[2] = packet.readBit();
                    guid[6] = packet.readBit();
                    guid[3] = packet.readBit();
                    guid[5] = packet.readBit();
                    guid[1] = packet.readBit();
                    guid[0] = packet.readBit();
                    guid[4] = packet.readBit();
                }

                const uint32_t titleLength = packet.readBits(8);

                invitees.resize(inviteCount);
                for (uint32_t i = 0; i < inviteCount; ++i)
                {
                    packet.readByteSeq(guids[i][4]);
                    packet.readByteSeq(guids[i][2]);
                    packet.readByteSeq(guids[i][3]);
                    packet.readByteSeq(guids[i][1]);
                    packet.readByteSeq(guids[i][0]);
                    packet.readByteSeq(guids[i][6]);
                    packet.readByteSeq(guids[i][7]);
                    packet >> invitees[i].status;
                    packet.readByteSeq(guids[i][5]);
                    packet >> invitees[i].rank;
                    invitees[i].guid = guids[i].getLowGuid();
                }

                title = packet.readString(titleLength);
                description = packet.readString(descriptionLength);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint8_t repeatable;
                uint32_t maxInvites;
                packet >> title >> description >> type >> repeatable >> maxInvites >> dungeonId;
                eventTime = packet.readPackedTime();
                packet.readSkip<uint32_t>();        // lock date / time zone date
                packet >> flags;

                // 4.x: a guild announcement carries no invites
                if (m_protocol.isCata() && (flags & CALENDAR_FLAG_WITHOUT_INVITES) != 0)
                    return true;

                uint32_t inviteCount;
                packet >> inviteCount;
                if (inviteCount > CALENDAR_MAX_INVITES)
                    return false;

                invitees.resize(inviteCount);
                for (auto& invitee : invitees)
                {
                    invitee.guid = CalendarWire::readPlayer(packet, m_protocol);
                    packet >> invitee.status >> invitee.rank;
                }
                return true;
            }

            return false;
        }

    private:
        // 8.x and later: club of the event first, every invite carries three optional fields
        bool readClubLayout(WorldPacket& packet)
        {
            uint64_t clubId;
            packet >> clubId >> type >> dungeonId;
            eventTime = packet.readPackedTime();

            // 12.x: the flags shrink to 16 bit and the string lengths follow the invites
            const bool lengthsAfterInvites = m_protocol.expansion >= WoW::Expansion::_Midnight;
            if (lengthsAfterInvites)
            {
                uint16_t shortFlags;
                packet >> shortFlags;
                flags = shortFlags;
            }
            else
            {
                packet >> flags;
            }

            uint32_t inviteCount;
            packet >> inviteCount;
            if (inviteCount > CALENDAR_MAX_INVITES)
                return false;

            uint32_t titleLength = 0;
            uint32_t descriptionLength = 0;
            if (!lengthsAfterInvites)
            {
                titleLength = packet.readBits(8);
                descriptionLength = packet.readBits(11);
            }

            invitees.resize(inviteCount);
            for (auto& invitee : invitees)
            {
                packet.resetBitPos();
                invitee.guid = CalendarWire::readPlayer(packet, m_protocol);
                packet >> invitee.status >> invitee.rank;

                packet.resetBitPos();
                const bool hasAccount = packet.readBit();
                const bool hasRealm = packet.readBit();
                const bool hasCommunity = packet.readBit();

                if (hasAccount)
                {
                    WoWGuid128 account;
                    packet >> account;
                }
                if (hasRealm)
                    packet.readSkip<uint64_t>();
                if (hasCommunity)
                    packet.readSkip<uint64_t>();
            }

            if (lengthsAfterInvites)
            {
                // the lengths start a new byte after the invite flags
                packet.resetBitPos();
                titleLength = packet.readBits(8);
                descriptionLength = packet.readBits(11);
            }

            packet.resetBitPos();
            title = packet.readString(titleLength);
            description = packet.readString(descriptionLength);

            uint32_t maxSize;
            packet >> maxSize;
            return true;
        }
    };
}
