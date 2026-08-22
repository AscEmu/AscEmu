/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgReplaceGuildMaster : public ManagedPacket
    {
    public:
        CmsgReplaceGuildMaster() : ManagedPacket(CMSG_REPLACE_GUILD_MASTER, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket&) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                return true;

            return false;
        }
    };
}
