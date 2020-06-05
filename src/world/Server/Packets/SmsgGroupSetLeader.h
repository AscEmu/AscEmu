/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGroupSetLeader : public ManagedPacket
    {
    public:
        std::string name;

        SmsgGroupSetLeader() : SmsgGroupSetLeader("")
        {
        }

        SmsgGroupSetLeader(std::string name) :
            ManagedPacket(SMSG_GROUP_SET_LEADER, name.size() + 1),
            name(name)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            packet << name;
#else
            packet << uint8_t(0);
            packet.writeBits(uint8_t(name.size()), 6);
            packet.flushBits();
            packet.WriteString(name);
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
