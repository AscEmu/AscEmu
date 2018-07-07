/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildMotd : public ManagedPacket
    {
    public:
        std::string message;

        CmsgGuildMotd() : CmsgGuildMotd("")
        {
        }

        CmsgGuildMotd(std::string message) :
            ManagedPacket(CMSG_GUILD_MOTD, 0),
            message(message)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> message;
            return true;
        }
    };
}}
