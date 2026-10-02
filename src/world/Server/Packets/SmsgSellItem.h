/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>
#include <utility>

namespace AscEmu::Packets
{
    class SmsgSellItem : public ManagedPacket
    {
    public:
        WoWGuid vendorGuid;
        WoWGuid itemGuid;
        std::vector<WoWGuid> itemGuids;
        uint8_t error;
        
        SmsgSellItem() : SmsgSellItem(WoWGuid(), WoWGuid(), 0)
        {
        }

        SmsgSellItem(WoWGuid vendorGuid, WoWGuid itemGuid, uint8_t error) :
            ManagedPacket(SMSG_SELL_ITEM, 8 + 8 + 1),
            vendorGuid(vendorGuid),
            itemGuid(itemGuid),
            error(error)
        {
            if (itemGuid)
                itemGuids.push_back(itemGuid);
        }

        SmsgSellItem(WoWGuid vendorGuid, std::vector<WoWGuid> itemGuids, uint8_t error) :
            ManagedPacket(SMSG_SELL_ITEM, 17),
            vendorGuid(vendorGuid),
            itemGuid(itemGuids.empty() ? WoWGuid() : itemGuids.front()),
            itemGuids(std::move(itemGuids)),
            error(error)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 8 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                const WoWGuid modernVendor = WoWGuid::createModernFromLegacy(vendorGuid.getRawGuid(), m_protocol.realmId);
                const auto packedVendor = modernVendor.packModern();
                packet.append(packedVendor.data(), packedVendor.size());
                packet << uint32_t(itemGuids.size()) << int32_t(error);
                for (WoWGuid const& guid : itemGuids)
                {
                    const WoWGuid modernItem = WoWGuid::createModernFromLegacy(guid.getRawGuid(), m_protocol.realmId);
                    const auto packedItem = modernItem.packModern();
                    packet.append(packedItem.data(), packedItem.size());
                }
                return true;
            }
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << vendorGuid.getRawGuid() << itemGuid.getRawGuid() << error;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(itemGuid[2]);

                packet.writeBit(vendorGuid[4]);

                packet.writeBit(itemGuid[5]);
                packet.writeBit(itemGuid[4]);

                packet.writeBit(vendorGuid[3]);
                packet.writeBit(vendorGuid[5]);

                packet.writeBit(itemGuid[3]);

                packet.writeBit(vendorGuid[6]);
                packet.writeBit(vendorGuid[0]);
                packet.writeBit(vendorGuid[2]);

                packet.writeBit(itemGuid[1]);
                packet.writeBit(itemGuid[7]);

                packet.writeBit(vendorGuid[1]);

                packet.writeBit(itemGuid[0]);
                packet.writeBit(itemGuid[6]);

                packet.writeBit(vendorGuid[7]);

                packet.writeByteSeq(itemGuid[4]);
                packet.writeByteSeq(itemGuid[1]);

                packet << uint8_t(error);

                packet.writeByteSeq(itemGuid[2]);

                packet.writeByteSeq(vendorGuid[4]);
                packet.writeByteSeq(vendorGuid[0]);
                packet.writeByteSeq(vendorGuid[5]);
                packet.writeByteSeq(vendorGuid[2]);

                packet.writeByteSeq(itemGuid[0]);

                packet.writeByteSeq(vendorGuid[3]);

                packet.writeByteSeq(itemGuid[5]);
                packet.writeByteSeq(itemGuid[6]);
                packet.writeByteSeq(itemGuid[7]);

                packet.writeByteSeq(vendorGuid[6]);
                packet.writeByteSeq(vendorGuid[1]);

                packet.writeByteSeq(itemGuid[3]);

                packet.writeByteSeq(vendorGuid[7]);
                return true;
            }
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
