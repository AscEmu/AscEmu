/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgDefenseMessage : public ManagedPacket
    {
    public:
        uint32_t zoneId;
        std::string message;
        uint32_t messageSize;

        SmsgDefenseMessage() : SmsgDefenseMessage(0, "")
        {
        }

        SmsgDefenseMessage(uint32_t zoneId, const std::string message) :
            ManagedPacket(SMSG_DEFENSE_MESSAGE, 4 + 4 + message.size()),
            zoneId(zoneId), message(message), messageSize(static_cast<uint32_t>(message.size()))
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << zoneId << messageSize << message;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
