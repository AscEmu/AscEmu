/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGuildQueryNews : public ManagedPacket
    {
    public:
        CmsgGuildQueryNews() : ManagedPacket(CMSG_GUILD_QUERY_NEWS, 4)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.readSkip<uint32_t>();

                return true;
            }
            else if (m_protocol.isMop())
            {
                // Mop's client sends no payload for this opcode; nothing to read.
                return true;
            }

            return false;
        }
    };
}
