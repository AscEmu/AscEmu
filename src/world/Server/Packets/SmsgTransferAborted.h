/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgTransferAborted : public ManagedPacket
    {
    public:
        uint32_t newMapId;
        uint32_t status;
        
        SmsgTransferAborted() : SmsgTransferAborted(0, 0)
        {
        }

        SmsgTransferAborted(uint32_t newMapId, uint32_t status) :
            ManagedPacket(SMSG_TRANSFER_ABORTED, 8),
            newMapId(newMapId),
            status(status)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << newMapId << status;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
