/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildInfoText : public ManagedPacket
    {
    public:
        std::string text;

        CmsgGuildInfoText() : CmsgGuildInfoText("")
        {
        }

        CmsgGuildInfoText(std::string text) :
            ManagedPacket(CMSG_GUILD_INFO_TEXT, 1),
            text(text)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> text;
                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_WotLK)
            {
                const uint32_t length = static_cast<uint32_t>(packet.readBits(12));
                text = packet.readString(length);
                return true;
            }

            return false;
        }
    };
}
