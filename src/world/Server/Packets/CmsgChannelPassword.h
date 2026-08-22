/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgChannelPassword : public ManagedPacket
    {
    public:
        std::string name;
        std::string password;

        CmsgChannelPassword() : CmsgChannelPassword("", "")
        {
        }

        CmsgChannelPassword(std::string name, std::string password) :
            ManagedPacket(CMSG_CHANNEL_PASSWORD, 0),
            name(name),
            password(password)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const uint32_t nameLen = packet.readBits(8);
                const uint32_t passLen = packet.readBits(7);

                name = packet.readString(nameLen);
                password = packet.readString(passLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> name >> password;
                return true;
            }

            return false;
        }
    };
}
