/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "AEVersion.hpp"
#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgItemTextQuery : public ManagedPacket
    {
    public:
#if VERSION_STRING > TBC
        uint64_t itemGuid;
#else
        uint32_t itemTextId;
#endif

        CmsgItemTextQuery() : CmsgItemTextQuery(0)
        {
        }

#if VERSION_STRING > TBC
        CmsgItemTextQuery(uint64_t itemGuid) :
            ManagedPacket(CMSG_ITEM_TEXT_QUERY, 8),
            itemGuid(itemGuid)
        {
        }
#else
        CmsgItemTextQuery(uint32_t itemTextId) :
            ManagedPacket(CMSG_ITEM_TEXT_QUERY, 4),
            itemTextId(itemTextId)
        {
        }
#endif

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING > TBC
            packet >> itemGuid;
#else
            packet >> itemTextId;
#endif
            return true;
        }
    };
}
