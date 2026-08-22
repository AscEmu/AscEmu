/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelOwner : public ManagedPacket
    {
    public:
        std::string name;

        CmsgChannelOwner() : CmsgChannelOwner("")
        {
        }

        CmsgChannelOwner(std::string name) :
            ManagedPacket(CMSG_CHANNEL_OWNER, 0),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t nameLen = packet.readBits(8);
                name = packet.readString(nameLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name;
                return true;
            }

            return false;
        }
    };
}
