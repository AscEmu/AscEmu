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
        LocationVector oldMapPosition;
        bool isTransport;
        uint32_t transportId;
        uint32_t oldMapId;
        
        SmsgTransferPending() : SmsgTransferPending(0, {0, 0, 0, 0}, false, 0, 0)
        {
        }

        SmsgTransferPending(uint32_t newMapId, LocationVector oldMapPosition = {0, 0, 0, 0}, bool isTransport = false, uint32_t transportId = 0, uint32_t oldMapId = 0) :
            ManagedPacket(SMSG_TRANSFER_PENDING, 17),
            newMapId(newMapId),
            oldMapPosition(oldMapPosition),
            isTransport(isTransport),
            transportId(transportId),
            oldMapId(oldMapId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isForever() ? 17 : 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                packet << int32_t(newMapId);
                packet << oldMapPosition.x << oldMapPosition.y << oldMapPosition.z;
                packet.writeBit(isTransport);
                packet.writeBit(0); // TransferSpellID
                packet.writeBit(0); // TaxiPathID
                packet.flushBits();

                if (isTransport)
                    packet << transportId << int32_t(oldMapId);

                return true;
            }

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
