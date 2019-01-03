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
    class CmsgItemTextQuery : public ManagedPacket
    {
    public:
        uint64_t itemGuid;

        CmsgItemTextQuery() : CmsgItemTextQuery(0)
        {
        }

        CmsgItemTextQuery(uint64_t itemGuid) :
            ManagedPacket(CMSG_ITEM_TEXT_QUERY, 8),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid;
            return true;
        }
    };
}}
