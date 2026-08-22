/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelSetOwner : public ManagedPacket
    {
    public:
        std::string name;
        std::string setName;

        CmsgChannelSetOwner() : CmsgChannelSetOwner("", "")
        {
        }

        CmsgChannelSetOwner(std::string name, std::string setName) :
            ManagedPacket(CMSG_CHANNEL_SET_OWNER, 0),
            name(name),
            setName(setName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t targetLen = packet.readBits(9);
                const uint32_t channelLen = packet.readBits(7);

                name = packet.readString(channelLen);
                setName = packet.readString(targetLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> setName;
                return true;
            }

            return false;
        }
    };
}
