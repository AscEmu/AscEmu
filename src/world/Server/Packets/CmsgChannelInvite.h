/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelInvite : public ManagedPacket
    {
    public:
        std::string name;
        std::string inviteName;

        CmsgChannelInvite() : CmsgChannelInvite("", "")
        {
        }

        CmsgChannelInvite(std::string name, std::string inviteName) :
            ManagedPacket(CMSG_CHANNEL_INVITE, 0),
            name(name),
            inviteName(inviteName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t targetLen = packet.readBits(7);
                const uint32_t channelLen = packet.readBits(8);

                inviteName = packet.readString(targetLen);
                name = packet.readString(channelLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> inviteName;
                return true;
            }

            return false;
        }
    };
}
