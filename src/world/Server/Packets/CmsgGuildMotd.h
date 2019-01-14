/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet >> message;
#else
            const uint32_t motdLength = packet.readBits(11);
            message = packet.ReadString(motdLength);
#endif
            return true;
        }
    };
}}
