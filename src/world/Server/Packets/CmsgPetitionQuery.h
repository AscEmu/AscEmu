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
    class CmsgPetitionQuery : public ManagedPacket
    {
    public:
        uint32_t charterId;
        uint64_t itemGuid;

        CmsgPetitionQuery() : CmsgPetitionQuery(0, 0)
        {
        }

        CmsgPetitionQuery(uint32_t charterId, uint64_t itemGuid) :
            ManagedPacket(CMSG_PETITION_QUERY, 12),
            charterId(charterId),
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
            packet >> charterId >> itemGuid;
            return true;
        }
    };
}}
