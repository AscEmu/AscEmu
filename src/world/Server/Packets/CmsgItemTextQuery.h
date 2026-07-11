/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgItemTextQuery : public ManagedPacket
    {
    public:
        uint64_t itemGuid = 0;  // since WotLK
        uint32_t itemTextId = 0; // before WotLK

        CmsgItemTextQuery() : CmsgItemTextQuery(0)
        {
        }

        CmsgItemTextQuery(uint32_t itemTextId) :
            ManagedPacket(CMSG_ITEM_TEXT_QUERY, 4),
            itemTextId(itemTextId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet >> itemGuid;
            }
            else
            {
                packet >> itemTextId;
            }

            return true;
        }
    };
}
