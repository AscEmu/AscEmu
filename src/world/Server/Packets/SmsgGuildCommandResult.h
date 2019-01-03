/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
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
            packet << command << message << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
