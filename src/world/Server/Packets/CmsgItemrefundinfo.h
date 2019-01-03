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
    class CmsgItemrefundinfo : public ManagedPacket
    {
#if VERSION_STRING > TBC
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid;
            return true;
        }
#endif
    };
}}
