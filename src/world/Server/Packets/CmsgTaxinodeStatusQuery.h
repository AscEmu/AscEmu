/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgTaxinodeStatusQuery : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgTaxinodeStatusQuery() : CmsgTaxinodeStatusQuery(0)
        {
        }

        CmsgTaxinodeStatusQuery(int64_t guid) :
            ManagedPacket(CMSG_TAXINODE_STATUS_QUERY, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;
            return true;
        }
    };
}}
