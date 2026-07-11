/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgItemNameQuery : public ManagedPacket
    {
    public:
        uint32_t itemEntry;

        CmsgItemNameQuery() : CmsgItemNameQuery(0)
        {
        }

        CmsgItemNameQuery(uint32_t itemEntry) :
            ManagedPacket(CMSG_ITEM_NAME_QUERY, 4),
            itemEntry(itemEntry)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemEntry;
            return true;
        }
    };
}
