/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfGuildGetRecruits : public ManagedPacket
    {
    public:
        uint32_t unkTime = 0;

        CmsgLfGuildGetRecruits() : ManagedPacket(CMSG_LF_GUILD_GET_RECRUITS, 4)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet >> unkTime;

                return true;
            }

            return false;
        }
    };
}
