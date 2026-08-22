/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> message;
                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_WotLK)
            {
                const uint32_t motdLength = packet.readBits(11);
                message = packet.readString(motdLength);
                return true;
            }

            return false;
        }
    };
}
