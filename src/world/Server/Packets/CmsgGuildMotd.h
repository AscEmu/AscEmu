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
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 8.x and later allow longer messages
                packet.resetBitPos();
                message = packet.readString(packet.readBits(m_protocol.isWoD() || m_protocol.isLegion() ? 10 : 11));
                return true;
            }

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
