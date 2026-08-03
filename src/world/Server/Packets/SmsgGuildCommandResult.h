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
    class SmsgGuildCommandResult : public ManagedPacket
    {
    public:
        uint32_t command;
        std::string message;
        uint32_t error;

        SmsgGuildCommandResult() : SmsgGuildCommandResult(0, "", 0)
        {
        }

        SmsgGuildCommandResult(uint32_t command, std::string message, uint32_t error) :
            ManagedPacket(SMSG_GUILD_COMMAND_RESULT, 0),
            command(command),
            message(message),
            error(error)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + message.size() + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << command << message << error;
            }
            else // Mop
            {
                packet << command << error;

                packet.writeBits(message.size(), 8);
                packet.flushBits();
                packet.writeString(message);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
