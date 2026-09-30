/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSellItem : public ManagedPacket
    {
    public:
        WoWGuid vendorGuid;
        WoWGuid itemGuid;
        int32_t amount;

        CmsgSellItem() : CmsgSellItem(0, 0, 0)
        {
        }

        CmsgSellItem(uint64_t vendorGuid, uint64_t itemGuid, int8_t amount) :
            ManagedPacket(CMSG_SELL_ITEM, 17),
            vendorGuid(vendorGuid),
            itemGuid(itemGuid),
            amount(amount)
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
            if (m_protocol.isForever()) return 8; // two packed modern GUID masks + uint32 amount
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 6; // int32 amount + 2 bytes holding the bits of both packed guids
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                auto readModernGuid = [&packet](WoWGuid& guid) { WoWGuid modernGuid; std::size_t consumed = 0; if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), modernGuid, consumed)) return false; packet.rpos(packet.rpos() + consumed); guid.init(modernGuid.toLegacyRaw()); return true; };
                if (!readModernGuid(vendorGuid) || !readModernGuid(itemGuid) || packet.remaining() != sizeof(uint32_t)) return false;
                uint32_t modernAmount = 0;
                packet >> modernAmount;
                amount = static_cast<int32_t>(modernAmount);
                return !packet.hadReadFailure() && packet.remaining() == 0;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t rawGuid;
                uint64_t rawItemGuid;
                int8_t amountByte = 0;
                packet >> rawGuid >> rawItemGuid >> amountByte;
                vendorGuid.init(rawGuid);
                itemGuid.init(rawItemGuid);
                amount = amountByte;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> amount;

                itemGuid[4] = packet.readBit();
                itemGuid[3] = packet.readBit();
                itemGuid[7] = packet.readBit();
                vendorGuid[6] = packet.readBit();
                vendorGuid[5] = packet.readBit();
                vendorGuid[1] = packet.readBit();
                itemGuid[5] = packet.readBit();
                itemGuid[2] = packet.readBit();
                itemGuid[1] = packet.readBit();
                vendorGuid[2] = packet.readBit();
                itemGuid[6] = packet.readBit();
                vendorGuid[4] = packet.readBit();
                vendorGuid[0] = packet.readBit();
                vendorGuid[7] = packet.readBit();
                vendorGuid[3] = packet.readBit();
                itemGuid[0] = packet.readBit();

                packet.readByteSeq(vendorGuid[6]);
                packet.readByteSeq(vendorGuid[3]);
                packet.readByteSeq(vendorGuid[1]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(vendorGuid[2]);
                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(itemGuid[5]);
                packet.readByteSeq(vendorGuid[7]);
                packet.readByteSeq(itemGuid[2]);
                packet.readByteSeq(vendorGuid[0]);
                packet.readByteSeq(vendorGuid[5]);
                packet.readByteSeq(itemGuid[3]);
                packet.readByteSeq(itemGuid[6]);
                packet.readByteSeq(vendorGuid[4]);
                packet.readByteSeq(itemGuid[4]);
                packet.readByteSeq(itemGuid[0]);
                return true;
            }

            return false;
        }
    };
}
