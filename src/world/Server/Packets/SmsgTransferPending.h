/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgTransferPending : public ManagedPacket
    {
    public:
        uint32_t newMapId;
        bool isTransport;
        uint32_t transportId;
        uint32_t oldMapId;
        
        SmsgTransferPending() : SmsgTransferPending(0, false, 0, 0)
        {
        }

        SmsgTransferPending(uint32_t newMapId, bool isTransport = false, uint32_t transportId = 0, uint32_t oldMapId = 0) :
            ManagedPacket(SMSG_TRANSFER_PENDING, 4),
            newMapId(newMapId),
            isTransport(isTransport),
            transportId(transportId),
            oldMapId(oldMapId)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet.writeBit(0);
                packet.writeBit(0);
            }
            packet << newMapId;

            if (isTransport)
                packet << transportId << oldMapId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
