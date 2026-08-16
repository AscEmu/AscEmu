/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ItemInterface.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Objects/Units/Players/TradeData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgTradeStatusExtended : public ManagedPacket
    {
    public:
        TradeData* tradeData;
        bool tradeState;

        SmsgTradeStatusExtended() : SmsgTradeStatusExtended(nullptr, true)
        {
        }

        SmsgTradeStatusExtended(TradeData* tradeData, bool tradeState) :
            ManagedPacket(SMSG_TRADE_STATUS_EXTENDED, 21 + (TRADE_SLOT_COUNT * 73)),
            tradeData(tradeData),
            tradeState(tradeState)
        {
        }

    protected:
        size_t expectedSize() const override { return 21 + (TRADE_SLOT_COUNT * 73); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (tradeData == nullptr)
                return false;

            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << uint8_t(tradeState ? 1 : 0);
                packet << uint32_t(0); // unk
                packet << uint32_t(TRADE_SLOT_COUNT);
                packet << uint32_t(TRADE_SLOT_COUNT);
                packet << uint32_t(tradeData->getTradeMoney());
                packet << uint32_t(tradeData->getSpell()); // This is the spell which is casted on the lowest item

                uint8_t itemCount = 0;
                for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
                {
                    packet << uint8_t(i);

                    const auto item = tradeData->getTradeItem(TradeSlots(i));
                    if (item == nullptr)
                    {
                        // Need to send empty fields, otherwise slots get messed up
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint64_t(0);
                        packet << uint32_t(0);
                        for (uint8_t ench = SOCK_ENCHANTMENT_SLOT1; ench < BONUS_ENCHANTMENT_SLOT; ++ench)
                            packet << uint32_t(0);
                        packet << uint64_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        continue;
                    }

                    ++itemCount;
                    if (const auto itemProperties = item->getItemProperties())
                    {
                        packet << uint32_t(itemProperties->ItemId);
                        packet << uint32_t(itemProperties->DisplayInfoID);
                        packet << uint32_t(item->getStackCount());
                        packet << uint32_t(item->hasFlags(ITEM_FLAG_WRAPPED) ? 1 : 0);

                        // Enchantment stuff
                        packet << uint64_t(item->getGiftCreatorGuid());
                        packet << uint32_t(item->getEnchantmentId(PERM_ENCHANTMENT_SLOT));
                        for (uint8_t ench = SOCK_ENCHANTMENT_SLOT1; ench < BONUS_ENCHANTMENT_SLOT; ++ench)
                            packet << uint32_t(item->getEnchantmentId(ench));

                        packet << uint64_t(item->getCreatorGuid()); // Item creator
                        packet << uint32_t(item->getSpellCharges(0)); // Spell charges
                        packet << uint32_t(item->getPropertySeed());
                        packet << uint32_t(item->getRandomPropertiesId());
                        packet << uint32_t(itemProperties->LockId);
                        packet << uint32_t(item->getMaxDurability());
                        packet << uint32_t(item->getDurability());
                    }
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Mop)
            {
                packet << uint32_t(0); // unk
                packet << uint32_t(0); // unk
                packet << uint64_t(tradeData->getTradeMoney());
                packet << uint32_t(tradeData->getSpell());
                packet << uint32_t(TRADE_SLOT_COUNT);
                packet << uint32_t(0); // unk
                packet << uint8_t(tradeState ? 1 : 0);
                packet << uint32_t(TRADE_SLOT_COUNT);

                uint8_t count = 0;
                for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
                {
                    if (tradeData->getTradeItem(TradeSlots(i)) != nullptr)
                        ++count;
                }

                packet.writeBits(count, 22);

                for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
                {
                    if (Item* item = tradeData->getTradeItem(TradeSlots(i)))
                    {
                        WoWGuid creatorGuid = item->getCreatorGuid();
                        WoWGuid giftCreatorGuid = item->getGiftCreatorGuid();

                        packet.writeBit(giftCreatorGuid[7]);
                        packet.writeBit(giftCreatorGuid[1]);
                        bool notWrapped = packet.writeBit(!item->hasFlags(ITEM_FLAG_WRAPPED)); //wrapped
                        packet.writeBit(giftCreatorGuid[3]);

                        if (notWrapped)
                        {
                            packet.writeBit(creatorGuid[7]);
                            packet.writeBit(creatorGuid[1]);
                            packet.writeBit(creatorGuid[4]);
                            packet.writeBit(creatorGuid[6]);
                            packet.writeBit(creatorGuid[2]);
                            packet.writeBit(creatorGuid[3]);
                            packet.writeBit(creatorGuid[5]);
                            packet.writeBit(item->getItemProperties()->LockId != 0);
                            packet.writeBit(creatorGuid[0]);
                        }
                        packet.writeBit(giftCreatorGuid[6]);
                        packet.writeBit(giftCreatorGuid[4]);
                        packet.writeBit(giftCreatorGuid[2]);
                        packet.writeBit(giftCreatorGuid[0]);
                        packet.writeBit(giftCreatorGuid[5]);
                    }
                }

                packet.flushBits();

                for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
                {
                    if (Item* item = tradeData->getTradeItem(TradeSlots(i)))
                    {
                        WoWGuid creatorGuid = item->getCreatorGuid();
                        WoWGuid giftCreatorGuid = item->getGiftCreatorGuid();

                        if (!item->hasFlags(ITEM_FLAG_WRAPPED))
                        {
                            packet.writeByteSeq(creatorGuid[1]);

                            packet << uint32_t(item->getEnchantmentId(PERM_ENCHANTMENT_SLOT));
                            for (uint8_t enchant_slot = SOCK_ENCHANTMENT_SLOT1; enchant_slot < BONUS_ENCHANTMENT_SLOT; ++enchant_slot)
                            {
                                packet << uint32_t(item->getEnchantmentId(enchant_slot));
                            }

                            packet << uint32_t(item->getMaxDurability());

                            packet.writeByteSeq(creatorGuid[6]);
                            packet.writeByteSeq(creatorGuid[2]);
                            packet.writeByteSeq(creatorGuid[7]);
                            packet.writeByteSeq(creatorGuid[4]);

                            packet << uint32_t(item->getEnchantmentId(REFORGE_ENCHANTMENT_SLOT));
                            packet << uint32_t(item->getDurability());
                            packet << uint32_t(item->getRandomPropertiesId());

                            packet.writeByteSeq(creatorGuid[3]);

                            packet << uint32_t(0); // unk

                            packet.writeByteSeq(creatorGuid[0]);

                            packet << uint32_t(item->getSpellCharges(0));
                            packet << uint32_t(item->getPropertySeed());

                            packet.writeByteSeq(creatorGuid[5]);
                        }

                        packet.writeByteSeq(giftCreatorGuid[6]);
                        packet.writeByteSeq(giftCreatorGuid[1]);
                        packet.writeByteSeq(giftCreatorGuid[7]);
                        packet.writeByteSeq(giftCreatorGuid[4]);

                        packet << uint32_t(item->getItemProperties()->ItemId);

                        packet.writeByteSeq(giftCreatorGuid[0]);

                        packet << uint32_t(item->getStackCount());

                        packet.writeByteSeq(giftCreatorGuid[5]);

                        packet << uint8_t(i); // slot

                        packet.writeByteSeq(giftCreatorGuid[2]);
                        packet.writeByteSeq(giftCreatorGuid[3]);
                    }
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
