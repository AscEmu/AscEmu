/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    // One visible loot slot (regular, quest, or FFA item). Populated by Player::sendLoot, which owns
    // the loot-method/round-robin/isAllowedForPlayer business rules that decide slotType and which
    // items are shown at all.
    struct LootSlotEntry
    {
        uint8_t slotIndex = 0;
        uint32_t itemId = 0;
        uint32_t count = 0;
        uint32_t displayId = 0;
        uint32_t randomField1 = 0; // randomSuffixFactor, or 0 if a random property is used instead
        uint32_t randomField2 = 0; // -randomSuffix.id, or randomProperty.ID, or 0 if neither
        uint8_t slotType = 0;
    };

    // One visible currency loot row. Own slot-index namespace, separate from LootSlotEntry::slotIndex.
    struct LootCurrencyEntry
    {
        uint8_t slotIndex = 0;
        uint32_t currencyId = 0;
        uint32_t count = 0;
    };

    class SmsgLootResponse : public ManagedPacket
    {
    public:
        uint64_t guid = 0;
        uint8_t lootType = 0;
        uint32_t gold = 0;
        std::vector<LootSlotEntry> slots;
        std::vector<LootCurrencyEntry> currencies;

        // The item-count byte the client sees can be higher than slots.size(): quest/FFA candidates
        // that fail the isAllowedForPlayer/is_looted check still bump the original counter even
        // though nothing gets written for them. Defaults to slots.size() when that quirk doesn't apply.
        uint32_t reportedItemCount = 0;

        SmsgLootResponse() : SmsgLootResponse(0, 0, 0, {}, {}, 0)
        {
        }

        SmsgLootResponse(uint64_t guid, uint8_t lootType, uint32_t gold, std::vector<LootSlotEntry> slots, std::vector<LootCurrencyEntry> currencies, uint32_t reportedItemCount) :
            ManagedPacket(SMSG_LOOT_RESPONSE, 0),
            guid(guid), lootType(lootType), gold(gold), slots(std::move(slots)), currencies(std::move(currencies)), reportedItemCount(reportedItemCount)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 15 + slots.size() * 20;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // AscEmu has no separate "loot session" guid; the container guid is reused for both
                // guid fields the client expects. Currency section not wired for Mop yet.
                const WoWGuid lootObjGuid(guid);
                const WoWGuid lootSessionGuid(guid);

                packet.writeBit(1); // Missing unk8
                packet.writeBit(0); // lootType present
                packet.writeBit(lootObjGuid[4]);

                const size_t currencyCountPos = packet.bitwpos();
                packet.writeBits(0, 20); // currency count, patched below (always 0)

                packet.writeBit(lootSessionGuid[2]);
                packet.writeBit(lootSessionGuid[3]);
                packet.writeBit(lootSessionGuid[7]);
                packet.writeBit(lootSessionGuid[1]);
                packet.writeBit(lootObjGuid[6]);
                packet.writeBit(lootObjGuid[7]);
                packet.writeBit(gold == 0);
                packet.writeBit(1); // isPersonalLooting
                packet.writeBit(1); // Missing unk8
                packet.writeBit(0); // isAoELooting - AscEmu has no AoE looting
                packet.writeBit(lootObjGuid[5]);
                packet.writeBit(lootSessionGuid[6]);

                const size_t itemCountPos = packet.bitwpos();
                packet.writeBits(0, 19); // item count, patched below

                packet.writeBit(lootSessionGuid[0]);

                for (const auto& slot : slots)
                {
                    packet.writeBits(slot.slotType, 3);
                    packet.writeBit(0); // canTradeToTapList
                    packet.writeBit(1); // no optional slot type byte
                    packet.writeBit(0); // slot is always present
                    packet.writeBits(3, 2);
                }

                packet.writeBit(lootObjGuid[1]);
                packet.writeBit(lootObjGuid[0]);

                // currency loop - not wired for Mop yet, always empty

                packet.writeBit(lootSessionGuid[5]);
                packet.writeBit(lootObjGuid[3]);
                packet.writeBit(lootSessionGuid[4]);
                packet.writeBit(0); // has loot method and threshold
                packet.writeBit(lootObjGuid[2]);
                packet.writeBits(0, 8); // LootMethod: FREE_FOR_ALL
                packet.writeBits(2, 2); // Threshold: ITEM_QUALITY_UNCOMMON

                packet.flushBits();

                packet.putBits(itemCountPos, slots.size(), 19);
                packet.putBits(currencyCountPos, 0, 20);

                ByteBuffer itemBuffer;
                for (const auto& slot : slots)
                {
                    itemBuffer << uint32_t(slot.randomField1);
                    itemBuffer << uint32_t(slot.count);
                    itemBuffer << uint32_t(slot.itemId);
                    itemBuffer << uint32_t(4); // situ size
                    itemBuffer << uint32_t(0); // situ, always 0
                    itemBuffer << uint32_t(slot.randomField2);
                    itemBuffer << uint8_t(slot.slotIndex);
                    itemBuffer << uint32_t(slot.displayId);
                }
                packet.append(itemBuffer);

                packet.writeByteSeq(lootSessionGuid[2]);

                if (gold)
                    packet << uint32_t(gold);

                packet.writeByteSeq(lootSessionGuid[7]);
                packet.writeByteSeq(lootObjGuid[5]);
                packet.writeByteSeq(lootSessionGuid[3]);
                packet.writeByteSeq(lootSessionGuid[4]);
                packet << uint8_t(lootType);
                packet.writeByteSeq(lootObjGuid[4]);
                packet.writeByteSeq(lootSessionGuid[5]);

                // currency data - not wired for Mop yet, always empty

                packet.writeByteSeq(lootObjGuid[2]);
                packet.writeByteSeq(lootObjGuid[3]);
                packet.writeByteSeq(lootSessionGuid[1]);
                packet.writeByteSeq(lootObjGuid[0]);
                packet.writeByteSeq(lootSessionGuid[0]);
                packet.writeByteSeq(lootObjGuid[6]);
                packet.writeByteSeq(lootObjGuid[7]);
                packet.writeByteSeq(lootObjGuid[1]);
                packet.writeByteSeq(lootSessionGuid[6]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint64_t(guid);
                packet << uint8_t(lootType);     //loot_type;

                packet << uint32_t(gold);        // gold
                packet << uint8_t(0);            //loot size reserve

                size_t currencyCountPos = 0;
                if (m_protocol.expansion >= WoW::Expansion::_Cata)
                {
                    currencyCountPos = packet.wpos();
                    packet << uint8_t(0);        // currency count reserve, patched below
                }

                for (const auto& slot : slots)
                {
                    packet << uint8_t(slot.slotIndex);
                    packet << uint32_t(slot.itemId);
                    packet << uint32_t(slot.count);  //nr of items of this type
                    packet << uint32_t(slot.displayId);
                    packet << uint32_t(slot.randomField1);
                    packet << uint32_t(slot.randomField2);
                    packet << uint8_t(slot.slotType);   // "still being rolled for" flag
                }

                // currency section: slotIndex(u8), currencyId(u32), count(u32) per entry
                if (m_protocol.expansion >= WoW::Expansion::_Cata)
                {
                    for (const auto& currency : currencies)
                    {
                        packet << uint8_t(currency.slotIndex);
                        packet << uint32_t(currency.currencyId);
                        packet << uint32_t(currency.count);
                    }

                    const size_t afterCurrencies = packet.wpos();
                    packet.wpos(currencyCountPos);
                    packet << uint8_t(currencies.size());
                    packet.wpos(afterCurrencies);
                }

                packet.wpos(13);
                packet << uint8_t(reportedItemCount);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
