/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSearchLfgLeave : public ManagedPacket
    {
    public:
        uint32_t entry;

        CmsgSearchLfgLeave() : CmsgSearchLfgLeave(0)
        {
        }

        CmsgSearchLfgLeave(uint32_t entry) :
            ManagedPacket(CMSG_SEARCH_LFG_LEAVE, 4),
            entry(entry)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_Cata)
                return false;

            packet >> entry;
            return true;
        }
    };
}
