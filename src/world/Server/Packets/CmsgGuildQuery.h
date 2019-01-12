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
    class CmsgGuildQuery : public ManagedPacket
    {
    public:
#if VERSION_STRING < Cata
        uint32_t guildId;

        CmsgGuildQuery() : CmsgGuildQuery(0)
        {
        }

        CmsgGuildQuery(uint32_t guildId) :
            ManagedPacket(CMSG_GUILD_QUERY, 4),
            guildId(guildId)
        {
        }
#else
        uint64_t guildId;
        uint64_t playerGuid;

        CmsgGuildQuery() : CmsgGuildQuery(0, 0)
        {
        }

        CmsgGuildQuery(uint32_t guildId, uint64_t playerGuid) :
            ManagedPacket(CMSG_GUILD_QUERY, 16),
            guildId(guildId),
            playerGuid(playerGuid)
        {
        }
#endif

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guildId;
#if VERSION_STRING >= Cata
            packet >> playerGuid;
#endif
            return true;
        }
    };
}}
