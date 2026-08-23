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

        CmsgBuyBackItem() : CmsgBuyBackItem(0, 0)
        {
        }

        CmsgBuyBackItem(uint64_t itemGuid, int32_t buybackSlot) :
            ManagedPacket(CMSG_BUY_BACK_ITEM, 8),
            itemGuid(itemGuid),
            buybackSlot(buybackSlot)
        {
        }

    protected:
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

            return false;
        }
    };
}
