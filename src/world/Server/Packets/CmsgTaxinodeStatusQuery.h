/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTaxinodeStatusQuery : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgTaxinodeStatusQuery() : CmsgTaxinodeStatusQuery(0)
        {
        }

        CmsgTaxinodeStatusQuery(uint64_t guid) :
            ManagedPacket(CMSG_TAXINODE_STATUS_QUERY, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;
            return true;
        }
    };
}
