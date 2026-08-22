/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelMute : public ManagedPacket
    {
    public:
        std::string name;
        std::string muteName;

        CmsgChannelMute() : CmsgChannelMute("", "")
        {
        }

        CmsgChannelMute(std::string name, std::string muteName) :
            ManagedPacket(CMSG_CHANNEL_MUTE, 0),
            name(name),
            muteName(muteName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t channelLen = packet.readBits(8);
                const uint32_t targetLen = packet.readBits(7);

                name = packet.readString(channelLen);
                muteName = packet.readString(targetLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> muteName;
                return true;
            }

            return false;
        }
    };
}
