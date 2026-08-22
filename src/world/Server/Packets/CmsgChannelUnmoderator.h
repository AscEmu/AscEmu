/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelUnmoderator : public ManagedPacket
    {
    public:
        std::string name;
        std::string unmodName;

        CmsgChannelUnmoderator() : CmsgChannelUnmoderator("", "")
        {
        }

        CmsgChannelUnmoderator(std::string name, std::string unmodName) :
            ManagedPacket(CMSG_CHANNEL_UNMODERATOR, 0),
            name(name),
            unmodName(unmodName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t targetLen = packet.readBits(7);
                const uint32_t channelLen = packet.readBits(8);

                name = packet.readString(channelLen);
                unmodName = packet.readString(targetLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> unmodName;
                return true;
            }

            return false;
        }
    };
}
