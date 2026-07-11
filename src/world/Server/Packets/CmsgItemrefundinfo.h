/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgItemrefundinfo : public ManagedPacket
    {
    public:
        uint64_t itemGuid;

        CmsgItemrefundinfo() : CmsgItemrefundinfo(0)
        {
        }

        CmsgItemrefundinfo(uint64_t itemGuid) :
            ManagedPacket(CMSG_ITEMREFUNDINFO, 8),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet >> itemGuid;
                return true;
            }

            return false;
        }
    };
}
