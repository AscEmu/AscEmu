/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildInvite : public ManagedPacket
    {
    public:
        std::string name;

        CmsgGuildInvite() : CmsgGuildInvite("")
        {
        }

        CmsgGuildInvite(std::string name) :
            ManagedPacket(CMSG_GUILD_INVITE, 1),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 9.x and later: an optional value behind the name
                packet.resetBitPos();
                const uint32_t length = packet.readBits(9);
                const bool hasExtra = (m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater()) && packet.readBit();
                name = packet.readString(length);
                if (hasExtra)
                    packet.readSkip<int32_t>();
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> name;
                return true;
            }
            else if (m_protocol.isMop())
            {
                uint32_t nameLength = packet.readBits(9);
                name = packet.readString(nameLength);
                return true;
            }
            else if (m_protocol.isCata())
            {
                uint32_t nameLength = packet.readBits(7);
                name = packet.readString(nameLength);
                return true;
            }

            return false;
        }
    };
}
