/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgBuyItem : public ManagedPacket
    {
    public:
        // Which of the handlers built this packet - the wire format below the shared
        // leading GUID differs between them and does not otherwise map cleanly onto a
        // single set of fields.
        enum class Variant : uint8_t
        {
            Vendor,         // handleBuyItemOpcode (regular vendor purchase)
            BuyBack,        // handleBuyBackOpcode
            BuyItemInSlot   // handleBuyItemInSlotOpcode
        };

        uint64_t sellerGuid;
        // cata specific
        uint32_t time;

        uint32_t itemEntry;
        uint32_t purchasedAmount;

        // BuyBack / BuyItemInSlot (post-Cata) specific fields
        uint32_t slotNumber = 0;
        int32_t signedAmount = 0;
        uint32_t extraAmount = 0;

        Variant variant = Variant::Vendor;

        SmsgBuyItem() : SmsgBuyItem(0, 0, 0, 0)
        {
        }

        // Vendor purchase (handleBuyItemOpcode)
        SmsgBuyItem(uint64_t sellerGuid, uint32_t time, uint32_t itemEntry, uint32_t purchasedAmount) :
            ManagedPacket(SMSG_BUY_ITEM, 1),
            sellerGuid(sellerGuid),
            time(time),
            itemEntry(itemEntry),
            purchasedAmount(purchasedAmount)
        {
        }

        // BuyBack (handleBuyBackOpcode) / BuyItemInSlot (handleBuyItemInSlotOpcode)
        SmsgBuyItem(Variant variant, uint64_t itemGuid, uint32_t time, uint32_t itemEntry, uint32_t purchasedAmount,
            uint32_t slotNumber, int32_t signedAmount, uint32_t extraAmount) :
            ManagedPacket(SMSG_BUY_ITEM, 1),
            sellerGuid(itemGuid),
            time(time),
            itemEntry(itemEntry),
            purchasedAmount(purchasedAmount),
            slotNumber(slotNumber),
            signedAmount(signedAmount),
            extraAmount(extraAmount),
            variant(variant)
        {
        }

    protected:
        size_t expectedSize() const override { return 24; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (variant == Variant::Vendor)
            {
                if (m_protocol.expansion < WoW::Expansion::_Mop)
                {
                    packet << sellerGuid;

                    if (m_protocol.expansion > WoW::Expansion::_TBC)
                        packet << time;

                    packet << itemEntry << purchasedAmount;
                }
                else if (m_protocol.isMop())
                {
                    WoWGuid selGuid = sellerGuid;

                    packet.writeBit(selGuid[3]);
                    packet.writeBit(selGuid[4]);
                    packet.writeBit(selGuid[7]);
                    packet.writeBit(selGuid[6]);
                    packet.writeBit(selGuid[0]);
                    packet.writeBit(selGuid[2]);
                    packet.writeBit(selGuid[1]);
                    packet.writeBit(selGuid[5]);

                    packet.writeByteSeq(selGuid[6]);
                    packet.writeByteSeq(selGuid[7]);

                    packet << purchasedAmount;

                    packet.writeByteSeq(selGuid[1]);
                    packet.writeByteSeq(selGuid[3]);
                    packet.writeByteSeq(selGuid[5]);
                    packet.writeByteSeq(selGuid[2]);

                    packet << int32_t(-1);

                    packet.writeByteSeq(selGuid[0]);
                    packet.writeByteSeq(selGuid[4]);

                    packet << itemEntry;
                }
                else
                {
                    return false;
                }
            }
            else // BuyBack / BuyItemInSlot - neither raw call site special-cased Mop, both
                 // simply took the ">= Cata" branch below for any post-Cata build.
            {
                packet << sellerGuid;

                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    packet << time;
                    packet << itemEntry;
                    packet << purchasedAmount;
                }
                else if (m_protocol.expansion <= WoW::Expansion::_Mop)
                {
                    packet << slotNumber;
                    packet << signedAmount;
                    packet << purchasedAmount;

                    if (variant == Variant::BuyBack)
                        packet << extraAmount;
                }
                else
                {
                    return false;
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
