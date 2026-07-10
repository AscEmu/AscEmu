/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildBankerActivate : public ManagedPacket
    {
    public:
        WoWGuid guid;
        bool full = false;  // since Cata

        CmsgGuildBankerActivate() : CmsgGuildBankerActivate(0)
        {
        }

        CmsgGuildBankerActivate(uint64_t guid) :
            ManagedPacket(CMSG_GUILD_BANKER_ACTIVATE, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.init(unpackedGuid);

            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                packet >> full;

            return true;
        }
    };
}
