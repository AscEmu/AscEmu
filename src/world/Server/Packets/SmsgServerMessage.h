/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgServerMessage : public ManagedPacket
    {
    public:
        uint32_t type;
        std::string text;

        SmsgServerMessage() : SmsgServerMessage(0, "")
        {
        }

        SmsgServerMessage(uint32_t type, std::string text = "") :
            ManagedPacket(SMSG_SERVER_MESSAGE, 4 + 46),
            type(type),
            text(text)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << type;

            if (type <= 3)   // > msg type
                packet << text.c_str();

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
