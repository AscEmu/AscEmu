/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemEntry;
            return true;
        }
    };
}}
