/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <string>

namespace AscEmu::Packets
{
    class CmsgLfGuildSetGuildPost : public ManagedPacket
    {
    public:
        uint32_t classRoles = 0;
        uint32_t availability = 0;
        uint32_t guildInterests = 0;
        uint32_t level = 0;
        bool listed = false;
        std::string comment;

        CmsgLfGuildSetGuildPost() : ManagedPacket(CMSG_LF_GUILD_SET_GUILD_POST, 16)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet >> level;
                packet >> availability;
                packet >> guildInterests;
                packet >> classRoles;

                const uint32_t length = packet.readBits(11);
                listed = packet.readBit();
                comment = packet.readString(length);

                return true;
            }

            return false;
        }
    };
}
