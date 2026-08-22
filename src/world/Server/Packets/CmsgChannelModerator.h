/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelModerator : public ManagedPacket
    {
    public:
        std::string name;
        std::string modName;

        CmsgChannelModerator() : CmsgChannelModerator("", "")
        {
        }

        CmsgChannelModerator(std::string name, std::string modName) :
            ManagedPacket(CMSG_CHANNEL_MODERATOR, 0),
            name(name),
            modName(modName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t channelLen = packet.readBits(8);
                const uint32_t nameLen = packet.readBits(7);

                modName = packet.readString(nameLen);
                name = packet.readString(channelLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> modName;
                return true;
            }

            return false;
        }
    };
}
