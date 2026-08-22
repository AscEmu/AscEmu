/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgQueryGuildRewards : public ManagedPacket
    {
    public:
        CmsgQueryGuildRewards() : ManagedPacket(CMSG_QUERY_GUILD_REWARDS, 4)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet.readSkip<uint32_t>();

                return true;
            }

            return false;
        }
    };
}
