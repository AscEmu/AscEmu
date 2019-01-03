/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmgsChannelMemberCount : public ManagedPacket
    {
    public:
        std::string channel_name;
        uint8_t flags;
        uint32_t member_count;

        SmgsChannelMemberCount() : SmgsChannelMemberCount("", 0, 0)
        {
        }

        SmgsChannelMemberCount(std::string channel_name, uint8_t flags, uint32_t member_count) :
            ManagedPacket(SMSG_CHANNEL_MEMBER_COUNT, 0),
            channel_name(channel_name),
            flags(flags),
            member_count(member_count)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << channel_name << flags << member_count;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> channel_name >> flags >> member_count;
            return true;
        }
    };
}}
