/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelKick : public ManagedPacket
    {
    public:
        std::string name;
        std::string kickName;

        CmsgChannelKick() : CmsgChannelKick("", "")
        {
        }

        CmsgChannelKick(std::string name, std::string kickName) :
            ManagedPacket(CMSG_CHANNEL_KICK, 0),
            name(name),
            kickName(kickName)
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
                kickName = packet.readString(targetLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> kickName;
                return true;
            }

            return false;
        }
    };
}
