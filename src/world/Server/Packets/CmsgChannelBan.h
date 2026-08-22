/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelBan : public ManagedPacket
    {
    public:
        std::string name;
        std::string banName;

        CmsgChannelBan() : CmsgChannelBan("", "")
        {
        }

        CmsgChannelBan(std::string name, std::string banName) :
            ManagedPacket(CMSG_CHANNEL_BAN, 0),
            name(name),
            banName(banName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t channelLen = packet.readBits(8);
                const uint32_t targetLen = packet.readBits(7);

                banName = packet.readString(targetLen);
                name = packet.readString(channelLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> banName;
                return true;
            }

            return false;
        }
    };
}
