/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ItemInstanceHelpers.h"
#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgBuyFailed : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t itemId;
        uint8_t error;

        SmsgBuyFailed() : SmsgBuyFailed(0, 0, 0)
        {
        }

        SmsgBuyFailed(uint64_t guid, uint32_t itemId, uint8_t error) :
            ManagedPacket(SMSG_BUY_FAILED, 13),
            guid(guid),
            itemId(itemId),
            error(error)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x+: vendor, item and the reason, 11.x+ as 32 bit
                writeClientGuid(packet, m_protocol, m_receiverMapId, guid);
                packet << uint32_t(itemId);
                if (m_protocol.isTheWarWithinOrLater())
                    packet << int32_t(error);
                else
                    packet << uint8_t(error);
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << guid << itemId << error;
            }
            else // Mop
            {
                WoWGuid vendorGuid = guid;

                packet.writeBit(vendorGuid[6]);
                packet.writeBit(vendorGuid[3]);
                packet.writeBit(vendorGuid[1]);
                packet.writeBit(vendorGuid[2]);
                packet.writeBit(vendorGuid[4]);
                packet.writeBit(vendorGuid[5]);
                packet.writeBit(vendorGuid[0]);
                packet.writeBit(vendorGuid[7]);

                packet << error;

                packet.writeByteSeq(vendorGuid[2]);
                packet.writeByteSeq(vendorGuid[7]);

                packet << itemId;

                packet.writeByteSeq(vendorGuid[4]);
                packet.writeByteSeq(vendorGuid[5]);
                packet.writeByteSeq(vendorGuid[1]);
                packet.writeByteSeq(vendorGuid[3]);
                packet.writeByteSeq(vendorGuid[6]);
                packet.writeByteSeq(vendorGuid[0]);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
