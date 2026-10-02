/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgBuyBackItem : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        int32_t buybackSlot;
        bool isForeverPacket = false;

        CmsgBuyBackItem() : CmsgBuyBackItem(0, 0)
        {
        }

        CmsgBuyBackItem(uint64_t itemGuid, int32_t buybackSlot) :
            ManagedPacket(CMSG_BUY_BACK_ITEM, 8),
            itemGuid(itemGuid),
            buybackSlot(buybackSlot)
        {
        }

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 5; // int32 buybackSlot + packed guid mask byte
            else if (m_protocol.isForever())
                return 5; // packed modern vendor guid + uint32 buyback slot
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> itemGuid >> buybackSlot;
                return true;
            }
            else if (m_protocol.isMop())
            {
                // Mop no longer sends the item guid - the handler must look it up via the slot instead
                packet >> buybackSlot;

                WoWGuid vendorGuid;
                vendorGuid[2] = packet.readBit();
                vendorGuid[3] = packet.readBit();
                vendorGuid[0] = packet.readBit();
                vendorGuid[4] = packet.readBit();
                vendorGuid[1] = packet.readBit();
                vendorGuid[7] = packet.readBit();
                vendorGuid[5] = packet.readBit();
                vendorGuid[6] = packet.readBit();

                packet.readByteSeq(vendorGuid[0]);
                packet.readByteSeq(vendorGuid[6]);
                packet.readByteSeq(vendorGuid[1]);
                packet.readByteSeq(vendorGuid[7]);
                packet.readByteSeq(vendorGuid[5]);
                packet.readByteSeq(vendorGuid[2]);
                packet.readByteSeq(vendorGuid[3]);
                packet.readByteSeq(vendorGuid[4]);
                return true;
            }
            else if (m_protocol.isForever())
            {
                WoWGuid modernVendorGuid;
                std::size_t consumed = 0;
                if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), modernVendorGuid, consumed))
                    return false;
                if (packet.remaining() < consumed + sizeof(uint32_t))
                    return false;

                packet.rpos(packet.rpos() + consumed);
                itemGuid = modernVendorGuid.toLegacyRaw();
                uint32_t rawBuybackSlot = 0;
                packet >> rawBuybackSlot;
                buybackSlot = static_cast<int32_t>(rawBuybackSlot);
                isForeverPacket = true;
                return packet.remaining() == 0;
            }

            return false;
        }
    };
}
