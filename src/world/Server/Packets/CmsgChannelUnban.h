/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelUnban : public ManagedPacket
    {
    public:
        std::string name;
        std::string unbanName;

        CmsgChannelUnban() : CmsgChannelUnban("", "")
        {
        }

        CmsgChannelUnban(std::string name, std::string unbanName) :
            ManagedPacket(CMSG_CHANNEL_UNBAN, 0),
            name(name),
            unbanName(unbanName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t channelLen = packet.readBits(7);
                const uint32_t targetLen = packet.readBits(8);

                unbanName = packet.readString(targetLen);
                name = packet.readString(channelLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> unbanName;
                return true;
            }

            return false;
        }
    };
}
