/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Utilities/utf8String.hpp"

namespace AscEmu::Packets
{
    class SmsgGroupDecline : public ManagedPacket
    {
    public:
        utf8_string name;

        SmsgGroupDecline() : SmsgGroupDecline("")
        {
        }

        SmsgGroupDecline(std::string name) :
            ManagedPacket(SMSG_GROUP_DECLINE, 100),
            name(name)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << name;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
