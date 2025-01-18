/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    struct SmsgChannelListMembers
    {
        uint64_t guid;
        uint8_t flags;
    };

    class SmsgChannelList : public ManagedPacket
    {
    public:
        bool chatQuery;
        utf8_string channelName;
        uint8_t channelFlags;
        std::vector<SmsgChannelListMembers> members;
        uint32_t membersCount;

        SmsgChannelList() : SmsgChannelList(false, "", 0, {})
        {
        }

        SmsgChannelList(bool chatQuery, std::string channelName, uint8_t channelFlags, std::vector<SmsgChannelListMembers> members) :
            ManagedPacket(SMSG_CHANNEL_LIST, 1 + channelName.size() + 1 + members.size() * 9),
            chatQuery(chatQuery),
            channelName(channelName),
            channelFlags(channelFlags),
            members(members),
            membersCount(static_cast<uint32_t>(members.size()))
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING >= TBC
            if (chatQuery)
                packet << uint8_t(0);
            else
                packet << uint8_t(1);
#endif

            packet << channelName << channelFlags << membersCount;
            for (const auto member : members)
                packet << member.guid << member.flags;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
