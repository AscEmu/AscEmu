/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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
        uint8_t unknown;
        std::string channelName;
        std::vector<SmsgChannelListMembers> members;
        uint32_t membersCount;

        SmsgChannelList() : SmsgChannelList("", {})
        {
        }

        SmsgChannelList(std::string channelName, std::vector<SmsgChannelListMembers> members) :
            ManagedPacket(SMSG_CHANNEL_LIST, 1 + channelName.size() + 1 + members.size() * 9),
            unknown(1),
            channelName(channelName),
            members(members),
            membersCount(members.size())
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unknown << channelName << membersCount;
            for (const auto member : members)
                packet << member.guid << member.flags;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
