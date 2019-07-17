/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgSearchLfgLeave : public ManagedPacket
    {
#if VERSION_STRING > TBC
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> entry;
            return true;
        }
#endif
    };
}
