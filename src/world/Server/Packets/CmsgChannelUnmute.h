/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelUnmute : public ManagedPacket
    {
    public:
        std::string name;
        std::string unmuteName;

        CmsgChannelUnmute() : CmsgChannelUnmute("", "")
        {
        }

        CmsgChannelUnmute(std::string name, std::string unmuteName) :
            ManagedPacket(CMSG_CHANNEL_UNMUTE, 0),
            name(name),
            unmuteName(unmuteName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t targetLen = packet.readBits(8);
                const uint32_t channelLen = packet.readBits(7);

                unmuteName = packet.readString(targetLen);
                name = packet.readString(channelLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> unmuteName;
                return true;
            }

            return false;
        }
    };
}
