/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfGuildBrowse : public ManagedPacket
    {
    public:
        uint32_t classRoles = 0;
        uint32_t availability = 0;
        uint32_t guildInterests = 0;
        uint32_t playerLevel = 0;

        CmsgLfGuildBrowse() : ManagedPacket(CMSG_LF_GUILD_BROWSE, 16)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet >> classRoles;
                packet >> availability;
                packet >> guildInterests;
                packet >> playerLevel;

                return true;
            }

            return false;
        }
    };
}
