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
    class CmsgGuildQuery : public ManagedPacket
    {
    public:
        uint32_t guildId;

        CmsgGuildQuery() : CmsgGuildQuery(0)
        {
        }

        CmsgGuildQuery(uint32_t guildId) :
            ManagedPacket(CMSG_GUILD_QUERY, 4),
            guildId(guildId)
        {
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guildId;
            return true;
        }
    };
}}
