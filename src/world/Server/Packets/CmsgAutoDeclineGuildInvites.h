/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgAutoDeclineGuildInvites : public ManagedPacket
    {
    public:
        bool enable = false;

        CmsgAutoDeclineGuildInvites() : ManagedPacket(CMSG_AUTO_DECLINE_GUILD_INVITES, 1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                enable = packet.readBit();

                return true;
            }
            else if (m_protocol.isCata())
            {
                uint8_t enableByte = 0;
                packet >> enableByte;
                enable = enableByte != 0;

                return true;
            }

            return false;
        }
    };
}
