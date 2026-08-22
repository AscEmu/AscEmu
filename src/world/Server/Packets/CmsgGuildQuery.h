/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildQuery : public ManagedPacket
    {
    public:
        uint32_t guildId = 0;

        uint64_t guildId64 = 0;
        uint64_t playerGuid = 0;

        CmsgGuildQuery() : CmsgGuildQuery(0, 0)
        {
        }

        CmsgGuildQuery(uint32_t guildId, uint64_t playerGuid) :
            ManagedPacket(CMSG_GUILD_QUERY, 16),
            guildId(guildId),
            playerGuid(playerGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> guildId;
                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_WotLK)
            {
                packet >> guildId64;
                guildId = static_cast<uint32_t>(guildId64);
                packet >> playerGuid;
                return true;
            }

            return false;
        }
    };
}
