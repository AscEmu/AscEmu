/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Objects/Item.hpp"
#include "Objects/Container.hpp"
#include "ItemProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Management/ItemInterface.h"
#include "Management/Loot/LootMgr.hpp"
#include "ObjectMgr.hpp"
#include "QuestMgr.h"
#include "Chat/ChatHandler.hpp"
#include "Logging/Logger.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/TradeData.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/SmsgInventoryChangeFailure.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

using namespace AscEmu::Packets;

// APGL End
// MIT Start

bool ItemInterface::hasItemForTotemCategory(uint32_t totemCategory)
{
    // If totem category is 0, the spell does not require any totems or tools
    if (totemCategory == 0)
        return true;

#if VERSION_STRING == Classic
    return false;
#else
    const auto spellTotemCategory = sTotemCategoryStore.lookupEntry(totemCategory);
    if (spellTotemCategory == nullptr)
        return false;

    // Helper lambda
    auto checkItem = [&](Item const* item) -> bool
    {
        if (item == nullptr)
            return false;
        // Item has no totem category
        if (item->getItemProperties()->TotemCategory == 0)
            return false;
        const auto itemTotemCategory = sTotemCategoryStore.lookupEntry(item->getItemProperties()->TotemCategory);
        // Item has invalid totem category
        if (itemTotemCategory == nullptr)
            return false;
        // Totem category types do not match
        if (spellTotemCategory->categoryType != itemTotemCategory->categoryType)
            return false;
        // Check if totem category masks match
        if (itemTotemCategory->categoryMask & spellTotemCategory->categoryMask)
            return true;
        return false;
    };

    for (int16_t i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (checkItem(GetInventoryItem(i)))
            return true;
    }

    for (int8_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        // Get bag from bag slot
        const auto container = GetContainer(i);
        if (container == nullptr)
            continue;
        // Loop through bag's content
        for (uint16_t j = 0; j < container->getSlotCount(); ++j)
        {
            if (checkItem(container->getItem(static_cast<int16_t>(j))))
                return true;
        }
    }

    for (int16_t i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (checkItem(GetInventoryItem(i)))
            return true;
    }

    for (int16_t i = INVENTORY_KEYRING_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (checkItem(GetInventoryItem(i)))
            return true;
    }

    return false;
#endif
}

bool ItemInterface::isItemInTradeWindow(Item const* item) const
{
    if (item == nullptr || item->getOwner() == nullptr)
        return false;

    if (item->getOwner()->getTradeData() == nullptr)
        return false;

    return item->getOwner()->getTradeData()->hasTradeItem(item->getGuid());
}

void ItemInterface::addTemporaryEnchantedItem(Item* item, EnchantmentSlot slot)
{
    std::unique_lock<std::mutex> guard(m_temporaryEnchantmentMutex);
    ItemEnchantmentDuration enchantment;
    enchantment.item = item;
    enchantment.slot = slot;
    enchantment.timeLeft = item->getEnchantmentDuration(slot);
    m_temporaryEnchantmentList.push_back(enchantment);
}

void ItemInterface::removeTemporaryEnchantedItem(Item* item)
{
    std::unique_lock<std::mutex> guard(m_temporaryEnchantmentMutex);
    for (auto itr = m_temporaryEnchantmentList.cbegin(); itr != m_temporaryEnchantmentList.cend();)
    {
        if ((*itr).item == item)
            itr = m_temporaryEnchantmentList.erase(itr);
        else
            ++itr;
    }
}

void ItemInterface::removeTemporaryEnchantedItem(Item* item, EnchantmentSlot slot)
{
    std::unique_lock<std::mutex> guard(m_temporaryEnchantmentMutex);
    for (auto itr = m_temporaryEnchantmentList.cbegin(); itr != m_temporaryEnchantmentList.cend();)
    {
        if ((*itr).item == item && (*itr).slot == slot)
            itr = m_temporaryEnchantmentList.erase(itr);
        else
            ++itr;
    }
}

void ItemInterface::sendEnchantDurations(Item const* forItem/* = nullptr*/)
{
    std::unique_lock<std::mutex> guard(m_temporaryEnchantmentMutex);
    for (const auto& itr : m_temporaryEnchantmentList)
    {
        if (forItem != nullptr && itr.item != forItem)
            continue;

        itr.item->setEnchantmentDuration(itr.slot, itr.timeLeft);
        itr.item->sendEnchantTimeUpdate(itr.slot, itr.timeLeft / 1000);
        itr.item->m_isDirty = true;
    }
}

void ItemInterface::updateEnchantDurations(uint32_t timePassed)
{
    std::unique_lock<std::mutex> guard(m_temporaryEnchantmentMutex);
    for (auto itr = m_temporaryEnchantmentList.begin(); itr != m_temporaryEnchantmentList.end();)
    {
        if (timePassed >= (*itr).timeLeft)
        {
            // Enchantment has expired
            (*itr).item->removeEnchantment((*itr).slot, true);
            itr = m_temporaryEnchantmentList.erase(itr);
        }
        else
        {
            (*itr).timeLeft -= timePassed;
            ++itr;
        }
    }
}

#if VERSION_STRING >= WotLK
void ItemInterface::updateSoulboundTradeItems()
{
    std::lock_guard<std::mutex> guard(m_soulboundTradeableMutex);
    if (m_soulboundTradeableList.empty())
        return;

    for (auto itemSoulbound = m_soulboundTradeableList.cbegin(); itemSoulbound != m_soulboundTradeableList.cend();)
    {
        if (!(*itemSoulbound)->m_isDirty)
        {
            if ((*itemSoulbound)->getOwner()->getGuid() != m_pOwner->getGuid())
            {
                itemSoulbound = m_soulboundTradeableList.erase(itemSoulbound);
                continue;
            }
            if ((*itemSoulbound)->checkSoulboundTradeExpire())
            {
                itemSoulbound = m_soulboundTradeableList.erase(itemSoulbound);
                continue;
            }
            ++itemSoulbound;
        }
        else
        {
            itemSoulbound = m_soulboundTradeableList.erase(itemSoulbound);
        }
    }
}

void ItemInterface::addTradeableItem(Item* item)
{
    std::lock_guard<std::mutex> guard(m_soulboundTradeableMutex);
    m_soulboundTradeableList.push_back(item);
}

void ItemInterface::removeTradeableItem(Item* item)
{
    std::lock_guard<std::mutex> guard(m_soulboundTradeableMutex);
    m_soulboundTradeableList.remove(item);
}
#endif

void ItemInterface::buildInventoryChangeError(Item const* srcItem, Item const* dstItem, uint8_t inventoryError, uint32_t srcItemId/* = 0*/)
{
    uint64_t srcGuid = 0;
    uint64_t destGuid = 0;
    uint32_t extraData = 0;
    auto sendExtraData = false;

    if (srcItem != nullptr)
        srcGuid = srcItem->getGuid();

    if (dstItem != nullptr)
        destGuid = dstItem->getGuid();

    const auto itemProperties = srcItem != nullptr ? srcItem->getItemProperties() : sMySQLStore.getItemProperties(srcItemId);
    switch (inventoryError)
    {
        case INV_ERR_YOU_MUST_REACH_LEVEL_N:
        case INV_ERR_PURCHASE_LEVEL_TOO_LOW:
        {
            extraData = itemProperties != nullptr ? itemProperties->RequiredLevel : 0;
            sendExtraData = true;
        } break;
        case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED:
        case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_SOCKETED_EXCEEDED:
        case INV_ERR_ITEM_MAX_LIMIT_CATEGORY_EQUIPPED_EXCEEDED:
        {
            extraData = itemProperties != nullptr ? itemProperties->ItemLimitCategory : 0;
            sendExtraData = true;
        } break;
        default:
            break;
    }

    m_pOwner->sendPacket(SmsgInventoryChangeFailure(inventoryError, srcGuid, destGuid, extraData, sendExtraData).serialise().get());
}

void ItemInterface::setOwnerInventoryItem(uint8_t slot, uint64_t guid)
{
    if (slot < INVENTORY_SLOT_BAG_END)
        m_pOwner->setInventorySlotItemGuid(slot, guid);
    else if (slot < INVENTORY_SLOT_ITEM_END)
        m_pOwner->setPackSlotItemGuid(slot - INVENTORY_SLOT_ITEM_START, guid);
    else if (slot < BANK_SLOT_ITEM_END)
        m_pOwner->setBankSlotItemGuid(slot - BANK_SLOT_ITEM_START, guid);
    else if (slot < BANK_SLOT_BAG_END)
        m_pOwner->setBankBagSlotItemGuid(slot - BANK_SLOT_BAG_START, guid);
    else if (slot < BUYBACK_SLOT_END)
        m_pOwner->setVendorBuybackSlot(slot - BUYBACK_SLOT_START, guid);
#if VERSION_STRING < Cata
    else if (slot < INVENTORY_KEYRING_END)
        m_pOwner->setKeyRingSlotItemGuid(slot - INVENTORY_KEYRING_START, guid);
#endif
#if VERSION_STRING == TBC
    else if (slot < (INVENTORY_KEYRING_END + WOWPLAYER_VANITY_PET_SLOT_COUNT))
        m_pOwner->setVanityPetSlotItemGuid(slot - INVENTORY_KEYRING_END, guid);
#elif VERSION_STRING == WotLK
    else if (slot < CURRENCYTOKEN_SLOT_END)
        m_pOwner->setCurrencyTokenSlotItemGuid(slot - CURRENCYTOKEN_SLOT_START, guid);
#endif
}

void ItemInterface::update(uint32_t timePassed)
{
    // Update enchantment durations
    updateEnchantDurations(timePassed);

#if VERSION_STRING >= WotLK
    // Retradeable soulbound items
    updateSoulboundTradeItems();
#endif

    // todo: add items with duration also here
}

// MIT End
// APGL Start

ItemInterface::ItemInterface(Player* pPlayer) : m_EquipmentSets(pPlayer->getGuidLow())
{
    m_pOwner = pPlayer;
    std::fill(m_pItems.begin(), m_pItems.end(), nullptr);
    std::fill(m_pBuyBack.begin(), m_pBuyBack.end(), nullptr);
    m_refundableitems.clear();

}

ItemInterface::~ItemInterface()
{
    // Free the memory explicitly here to prevent crash on logout -Appled
    std::fill(m_pItems.begin(), m_pItems.end(), nullptr);
    std::fill(m_pBuyBack.begin(), m_pBuyBack.end(), nullptr);
    this->m_refundableitems.clear();
}

uint32_t ItemInterface::m_CreateForPlayer(ByteBuffer* data)       // 100%
{
    uint32_t count = 0;

    if (m_pOwner)
    {
        for (uint8_t i = 0; i < MAX_INVENTORY_SLOT; ++i)
        {
            if (m_pItems[i])
            {
                if (m_pItems[i]->isContainer())
                {
                    count += m_pItems[i]->buildCreateUpdateBlockForPlayer(data, m_pOwner);

                    for (uint32_t e = 0; e < m_pItems[i]->getItemProperties()->ContainerSlots; ++e)
                    {
                        Item* pItem = static_cast<Container*>(m_pItems[i].get())->getItem(static_cast<int16_t>(e));
                        if (pItem)
                            count += pItem->buildCreateUpdateBlockForPlayer(data, m_pOwner);
                    }
                }
                else
                {
                    count += m_pItems[i]->buildCreateUpdateBlockForPlayer(data, m_pOwner);
                }
            }
        }
    }
    else
    {
        sLogger.failure("No Owner for ItemInterface::m_CreateForPlayer");
    }
    return count;
}

void ItemInterface::m_DestroyForPlayer()        // 100%
{
    if (m_pOwner)
    {
        for (uint8_t i = 0; i < MAX_INVENTORY_SLOT; ++i)
        {
            if (m_pItems[i])
            {
                if (m_pItems[i]->isContainer())
                {
                    for (uint32_t e = 0; e < m_pItems[i]->getItemProperties()->ContainerSlots; ++e)
                    {
                        Item* pItem = static_cast<Container*>(m_pItems[i].get())->getItem(static_cast<int16_t>(e));
                        if (pItem)
                        {
                            m_pOwner->sendDestroyObjectPacket(pItem->getGuid());
                        }
                    }
                    m_pOwner->sendDestroyObjectPacket(m_pItems[i]->getGuid());
                }
                else
                {
                    m_pOwner->sendDestroyObjectPacket(m_pItems[i]->getGuid());
                }
            }
        }
    }
    else
    {
        sLogger.failure("Tried to destroy owner item without an owner ItemInterface::m_DestroyForPlayer");
    }
}

/// Creates and adds a item that can be manipulated after
Item* ItemInterface::SafeAddItem(uint32_t ItemId, int8_t ContainerSlot, int16_t slot)
{
    ItemProperties const* pProto = sMySQLStore.getItemProperties(ItemId);
    if (!pProto)
        return nullptr;

    if (pProto->InventoryType == INVTYPE_BAG)
    {
        std::unique_ptr<Item> itemHolder = std::make_unique<Container>(HIGHGUID_TYPE_CONTAINER, sObjectMgr.generateLowGuid(HIGHGUID_TYPE_CONTAINER));
        static_cast<Container*>(itemHolder.get())->create(ItemId, m_pOwner);
        auto* pItem = itemHolder.get();
        const auto [result, _] = m_AddItem(std::move(itemHolder), ContainerSlot, slot);
        if (result)
        {
            return pItem;
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        auto itemHolder = std::make_unique<Item>();
        itemHolder->init(HIGHGUID_TYPE_ITEM, sObjectMgr.generateLowGuid(HIGHGUID_TYPE_ITEM));
        itemHolder->create(ItemId, m_pOwner);
        auto* pItem = itemHolder.get();
        const auto [result, _] = m_AddItem(std::move(itemHolder), ContainerSlot, slot);
        if (result)
        {
            return pItem;
        }
        else
        {
            return nullptr;
        }
    }
}

// Creates and adds a item that can be manipulated after
// Returns item in tuple with result if failed to add item, nullptr on success
std::tuple<AddItemResult, std::unique_ptr<Item>> ItemInterface::SafeAddItem(std::unique_ptr<Item> pItem, int8_t ContainerSlot, int16_t slot)
{
    return m_AddItem(std::move(pItem), ContainerSlot, slot);
}

// Adds items to player inventory, this includes all types of slots.
// Returns item in tuple with result if failed to add item, nullptr on success
std::tuple<AddItemResult, std::unique_ptr<Item>> ItemInterface::m_AddItem(std::unique_ptr<Item> itemHolder, int8_t ContainerSlot, int16_t slot)
{
    if (slot >= MAX_INVENTORY_SLOT)
    {
        sLogger.failure("ItemInterface::m_AddItem slot {} is invalid!", slot);
        return { ADD_ITEM_RESULT_ERROR, std::move(itemHolder) };
    }

    if (ContainerSlot >= MAX_INVENTORY_SLOT)
    {
        sLogger.failure("ItemInterface::m_AddItem containerSlot {} is invalid!", ContainerSlot);
        return { ADD_ITEM_RESULT_ERROR, std::move(itemHolder) };
    }

    if (itemHolder == nullptr || !itemHolder->getItemProperties() || slot < 0)
        return { ADD_ITEM_RESULT_ERROR, std::move(itemHolder) };

    itemHolder->m_isDirty = true;

    for (uint8_t i = 0; i < MAX_INVENTORY_SLOT; ++i)
    {
        Item * tempitem = m_pItems[i].get();
        if (tempitem != nullptr)
        {
            if (tempitem == itemHolder.get())
            {
                return { ADD_ITEM_RESULT_DUPLICATED, std::move(itemHolder) };
            }

            if (tempitem->isContainer())
            {
                uint32_t k = tempitem->getItemProperties()->ContainerSlots;
                for (uint16_t j = 0; j < k; ++j)
                {
                    if (static_cast<Container*>(tempitem)->getItem(j) == itemHolder.get())
                    {
                        return { ADD_ITEM_RESULT_DUPLICATED, std::move(itemHolder) };
                    }
                }
            }
        }
    }

    // Save raw ptr from itemHolder for later use
    auto* item = itemHolder.get();

    //case 1, item is from backpack container
    if (ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        if (GetInventoryItem(slot) != nullptr /*|| (slot == EQUIPMENT_SLOT_OFFHAND && !m_pOwner->HasSkillLine(118))*/)
        {
            //sLogger.failure("bugged inventory: {} {}", m_pOwner->GetName(), item->getGuid());
            SlotResult result = this->FindFreeInventorySlot(itemHolder->getItemProperties());

            // send message to player
            sChatHandler.BlueSystemMessage(m_pOwner->getSession(), "A duplicated item, `%s` was found in your inventory. We've attempted to add it to a free slot in your inventory, if there is none this will fail. It will be attempted again the next time you log on.",
                itemHolder->getItemProperties()->Name.c_str());
            if (result.Result == true)
            {
                // Found a new slot for that item.
                slot = result.Slot;
                ContainerSlot = result.ContainerSlot;
            }
            else
            {
                // don't leak memory!
                /*delete item;*/

                return { ADD_ITEM_RESULT_ERROR, std::move(itemHolder) };
            }
        }

        if (!GetInventoryItem(slot))        //slot is free, add item.
        {
            itemHolder->setOwner(m_pOwner);
            itemHolder->setContainerGuid(m_pOwner->getGuid());
            m_pItems[slot] = std::move(itemHolder);

            if (item->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP)
            {
                if (item->getItemProperties()->Flags & ITEM_FLAG_ACCOUNTBOUND)       // don't "Soulbind" account-bound items
                    item->addFlags(ITEM_FLAG_ACCOUNTBOUND);
                else
                    item->addFlags(ITEM_FLAG_SOULBOUND);
            }

            if (m_pOwner->IsInWorld() && !item->IsInWorld())
            {
                item->PushToWorld(m_pOwner->getWorldMap());
                ByteBuffer buf(2500);
                uint32_t count = item->buildCreateUpdateBlockForPlayer(&buf, m_pOwner);
                m_pOwner->getUpdateMgr().pushCreationData(&buf, count);
            }
            setOwnerInventoryItem(static_cast<uint8_t>(slot), item->getGuid());
        }
        else
        {
            return { ADD_ITEM_RESULT_ERROR, std::move(itemHolder) };
        }
    }
    else //case 2: item is from a bag container
    {
        if (GetInventoryItem(ContainerSlot) && GetInventoryItem(ContainerSlot)->isContainer() &&
            slot < (int32_t)GetInventoryItem(ContainerSlot)->getItemProperties()->ContainerSlots) //container exists
        {
            auto [result, returnedItem] = static_cast<Container*>(m_pItems[ContainerSlot].get())->addItem(slot, std::move(itemHolder));
            if (!result)
            {
                return { ADD_ITEM_RESULT_ERROR, std::move(returnedItem) };
            }
        }
        else
        {
            itemHolder->deleteFromDB(); //wpe dupefix ..we don't want it reappearing on the next relog now do we?
            return { ADD_ITEM_RESULT_ERROR, std::move(itemHolder) };
        }
    }

    if (slot < EQUIPMENT_SLOT_END && ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        m_pOwner->setVisibleItemFields(slot, item);
    }

    sendEnchantDurations(item);

    if (m_pOwner->IsInWorld() && slot < INVENTORY_SLOT_BAG_END && ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        m_pOwner->applyItemMods(item, slot, true);
    }

    if (slot >= CURRENCYTOKEN_SLOT_START && slot < CURRENCYTOKEN_SLOT_END)
    {
        m_pOwner->updateKnownCurrencies(item->getEntry(), true);
    }

    if (ContainerSlot == INVENTORY_SLOT_NOT_SET && slot == EQUIPMENT_SLOT_OFFHAND && item->getItemProperties()->Class == ITEM_CLASS_WEAPON)
    {
        /////////////////////////////////////////// Titan's grip stuff ////////////////////////////////////////////////////////////

        uint32_t subclass = item->getItemProperties()->SubClass;
        if (subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_AXE || subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_MACE || subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD)
        {
            m_pOwner->castSpell(m_pOwner, 49152, true);

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
    }

#if VERSION_STRING > TBC
    m_pOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM, item->getEntry(), 1, 0);
#endif
    ////////////////////////////////////////////////////// existingduration stuff /////////////////////////////////////////////////////
    if (item->getItemProperties()->ExistingDuration != 0)
    {
        if (item->getItemExpireTime() == 0)
        {
            item->setItemExpireTime(UNIXTIME + item->getItemProperties()->ExistingDuration);
            item->setDuration(item->getItemProperties()->ExistingDuration);
            sEventMgr.AddEvent(item, &Item::eventRemoveItem, EVENT_REMOVE_ITEM, item->getItemProperties()->ExistingDuration * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
        }
        else
        {
            item->setDuration(static_cast<uint32_t>(item->getItemExpireTime() - UNIXTIME));
            sEventMgr.AddEvent(item, &Item::eventRemoveItem, EVENT_REMOVE_ITEM, (item->getItemExpireTime() - UNIXTIME) * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
        }

        // if we are already in the world we will send the durationupdate now, so we can see the remaining duration in the client
        // otherwise we will send the updates in Player::Onpushtoworld anyways
        if (m_pOwner->IsInWorld())
            sEventMgr.AddEvent(item, &Item::sendDurationUpdate, EVENT_SEND_PACKET_TO_PLAYER_AFTER_LOGIN, 0, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    return { ADD_ITEM_RESULT_OK, nullptr };
}

/// Checks if the slot is a Bag slot
bool ItemInterface::IsBagSlot(int16_t slot)
{
    if ((slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
    {
        return true;
    }
    return false;
}

/// Removes the item safely and returns it back for usage
std::unique_ptr<Item> ItemInterface::SafeRemoveAndRetreiveItemFromSlot(int8_t ContainerSlot, int16_t slot, bool destroy)
{
    if (slot >= MAX_INVENTORY_SLOT)
    {
        sLogger.failure("ItemInterface::SafeRemoveAndRetreiveItemFromSlot slot {} is invalid!", slot);
        return nullptr;
    }

    if (ContainerSlot >= MAX_INVENTORY_SLOT)
    {
        sLogger.failure("ItemInterface::SafeRemoveAndRetreiveItemFromSlot containerSlot {} is invalid!", ContainerSlot);
        return nullptr;
    }

    std::unique_ptr<Item> pItem = nullptr;

    if (ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        auto* rawItem = GetInventoryItem(ContainerSlot, slot);

        if (rawItem == nullptr)
        {
            return nullptr;
        }

        if (rawItem->getItemProperties()->ContainerSlots > 0 && rawItem->isContainer() && static_cast<Container*>(rawItem)->hasItems())
        {
            // sounds weird? no. this will trigger a callstack display due to my other debug code.
            rawItem->deleteFromDB();
            return nullptr;
        }

        if (rawItem->getOwner() != m_pOwner)
            return nullptr;

        pItem = std::move(m_pItems[slot]);
        if (pItem->getOwner() == m_pOwner)
        {
            pItem->m_isDirty = true;

            setOwnerInventoryItem(static_cast<uint8_t>(slot), 0);

            sendEnchantDurations(pItem.get());

            if (slot < EQUIPMENT_SLOT_END)
            {
                m_pOwner->applyItemMods(pItem.get(), slot, false);
                m_pOwner->setVisibleItemFields(slot, nullptr);
            }
            else if (slot < INVENTORY_SLOT_BAG_END)
                m_pOwner->applyItemMods(pItem.get(), slot, false);

            sQuestMgr.onPlayerItemRemove(GetOwner(), pItem.get());

            if (destroy)
            {
                if (pItem->IsInWorld())
                {
                    pItem->removeFromWorld();
                }
                pItem->deleteFromDB();
            }
        }
    }
    else
    {
        Item* pContainer = GetInventoryItem(ContainerSlot);
        if (pContainer && pContainer->isContainer())
        {
            pItem = static_cast<Container*>(pContainer)->safeRemoveAndRetreiveItemFromSlot(slot, destroy);
        }
    }

    return pItem;
}

/// Removes the item safely by guid and returns it back for usage, supports full inventory
std::unique_ptr<Item> ItemInterface::SafeRemoveAndRetreiveItemByGuid(uint64_t guid, bool destroy)
{
    int16_t i = 0;

    for (i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for (i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
        else
        {
            if (item && item->isContainer())
            {
                for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                    if (item2 && item2->getGuid() == guid)
                    {
                        return static_cast<Container*>(item)->safeRemoveAndRetreiveItemFromSlot(static_cast<int16_t>(j), destroy);
                    }
                }
            }
        }
    }

    for (i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
    }

    for (i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeRemoveAndRetreiveItemFromSlot(INVENTORY_SLOT_NOT_SET, i, destroy);
        }
        else
        {
            if (item && item->isContainer())
            {
                for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                    if (item2 && item2->getGuid() == guid)
                    {
                        return static_cast<Container*>(item)->safeRemoveAndRetreiveItemFromSlot(static_cast<int16_t>(j), destroy);
                    }
                }
            }
        }
    }

    return nullptr;
}

/// Completely removes item from player
/// \return true if item removal was succefull
bool ItemInterface::SafeFullRemoveItemFromSlot(int8_t ContainerSlot, int16_t slot)
{
    if (slot >= MAX_INVENTORY_SLOT)
    {
        sLogger.failure("ItemInterface::SafeFullRemoveItemFromSlot slot {} is invalid!", slot);
        return false;
    }

    if (ContainerSlot >= MAX_INVENTORY_SLOT)
    {
        sLogger.failure("ItemInterface::SafeFullRemoveItemFromSlot containerSlot {} is invalid!", ContainerSlot);
        return false;
    }

    if (ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        Item* rawItem = GetInventoryItem(slot);

        if (rawItem == nullptr)
            return false;

        if (rawItem->getItemProperties()->ContainerSlots > 0 && rawItem->isContainer() && static_cast<Container*>(rawItem)->hasItems())
        {
            // sounds weird? no. this will trigger a callstack display due to my other debug code.
            rawItem->deleteFromDB();
            return false;
        }

        if (rawItem->getOwner() != m_pOwner)
            return false;

        std::unique_ptr<Item> pItem = std::move(m_pItems[slot]);
        // hacky crashfix
        if (pItem->getOwner() == m_pOwner)
        {
            pItem->m_isDirty = true;

            setOwnerInventoryItem(static_cast<uint8_t>(slot), 0);

            sendEnchantDurations(pItem.get());

            if (slot < EQUIPMENT_SLOT_END)
            {
                m_pOwner->applyItemMods(pItem.get(), slot, false);
                m_pOwner->setVisibleItemFields(slot, nullptr);
            }
            else if (slot < INVENTORY_SLOT_BAG_END)
                m_pOwner->applyItemMods(pItem.get(), slot, false);  //watch containers that give attackspeed and stuff ;)

            if (pItem->IsInWorld())
            {
                pItem->removeFromWorld();
            }

            pItem->deleteFromDB();

            sQuestMgr.onPlayerItemRemove(GetOwner(), pItem.get());

            //delete pItem;
            // We make it a garbage item, so when it's used for a spell, it gets deleted in the next Player update
            // otherwise we get a nice crash
            m_pOwner->addGarbageItem(std::move(pItem));
        }
    }
    else
    {
        Item* pContainer = GetInventoryItem(ContainerSlot);
        if (pContainer && pContainer->isContainer())
        {
            return static_cast<Container*>(pContainer)->safeFullRemoveItemFromSlot(slot);
        }
    }
    return true;
}

/// Removes the item safely by guid, supports full inventory
bool ItemInterface::SafeFullRemoveItemByGuid(uint64_t guid)
{
    int16_t i = 0;

    for (i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for (i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
        else
        {
            if (item && item->isContainer())
            {
                for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                    if (item2 && item2->getGuid() == guid)
                    {
                        return static_cast<Container*>(item)->safeFullRemoveItemFromSlot(static_cast<int16_t>(j));
                    }
                }
            }
        }
    }

    for (i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);

        if (item && item->getGuid() == guid)
        {
            return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for (i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->getGuid() == guid)
        {
            return this->SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
        }
        else
        {
            if (item && item->isContainer())
            {
                for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                    if (item2 && item2->getGuid() == guid)
                    {
                        return static_cast<Container*>(item)->safeFullRemoveItemFromSlot(static_cast<int16_t>(j));
                    }
                }
            }
        }
    }
    return false;
}

/// Gets a item from Inventory
Item* ItemInterface::GetInventoryItem(int16_t slot)
{
    if (slot < 0 || slot >= MAX_INVENTORY_SLOT)
        return nullptr;

    return m_pItems[slot].get();
}

/// Gets a Item from inventory or container
Item* ItemInterface::GetInventoryItem(int8_t ContainerSlot, int16_t slot)
{
    if (ContainerSlot <= INVENTORY_SLOT_NOT_SET)
    {
        if (slot < 0 || slot >= MAX_INVENTORY_SLOT)
            return nullptr;

        return m_pItems[slot].get();
    }
    else
    {
        if (IsBagSlot(ContainerSlot))
        {
            if (m_pItems[(int)ContainerSlot])
            {
                return static_cast<Container*>(m_pItems[ContainerSlot].get())->getItem(slot);
            }
        }
    }
    return nullptr;
}

Container* ItemInterface::GetContainer(int8_t containerSlot)
{
    if (!IsBagSlot(containerSlot))
        return nullptr;

    if (m_pItems[containerSlot] == nullptr)
        return nullptr;

    return static_cast<Container*>(m_pItems[containerSlot].get());
}

/// Checks for stacks that didn't reached max capacity
Item* ItemInterface::FindItemLessMax(uint32_t itemid, uint32_t cnt, bool IncBank)
{
    uint32_t i;
    for (i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item)
        {
            uint32_t itemMaxStack = (item->getOwner()->m_cheats.hasItemStackCheat) ? 0x7fffffff : item->getItemProperties()->MaxCount;
            if ((item->getEntry() == itemid && item->m_wrappedItemId == 0) && (itemMaxStack >= (item->getStackCount() + cnt)))
            {
                return item;
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item && item->isContainer())
        {
            for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                if (item2)
                {
                    uint32_t itemMaxStack = (item2->getOwner()->m_cheats.hasItemStackCheat) ? 0x7fffffff : item2->getItemProperties()->MaxCount;
                    if ((item2->getItemProperties()->ItemId == itemid && item2->m_wrappedItemId == 0) && (itemMaxStack >= (item2->getStackCount() + cnt)))
                    {
                        return item2;
                    }
                }
            }

        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item)
        {
            uint32_t itemMaxStack = (item->getOwner()->m_cheats.hasItemStackCheat) ? 0x7fffffff : item->getItemProperties()->MaxCount;
            if ((item->getEntry() == itemid && item->m_wrappedItemId == 0) && (itemMaxStack >= (item->getStackCount() + cnt)))
            {
                return item;
            }
        }
    }

    if (IncBank)
    {
        for (i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16_t>(i));
            if (item)
            {
                uint32_t itemMaxStack = (item->getOwner()->m_cheats.hasItemStackCheat) ? 0x7fffffff : item->getItemProperties()->MaxCount;
                if ((item->getEntry() == itemid && item->m_wrappedItemId == 0) && (itemMaxStack >= (item->getStackCount() + cnt)))
                {
                    return item;
                }
            }
        }

        for (i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16_t>(i));
            if (item && item->isContainer())
            {

                for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                    if (item2)
                    {
                        uint32_t itemMaxStack = (item2->getOwner()->m_cheats.hasItemStackCheat) ? 0x7fffffff : item2->getItemProperties()->MaxCount;
                        if ((item2->getItemProperties()->ItemId == itemid && item2->m_wrappedItemId == 0) && (itemMaxStack >= (item2->getStackCount() + cnt)))
                        {
                            return item2;
                        }
                    }
                }

            }
        }
    }

    return nullptr;
}

/// Finds item ammount on inventory, banks not included
uint32_t ItemInterface::GetItemCount(uint32_t itemid, bool IncBank)
{
    uint32_t cnt = 0;
    uint32_t i;
    for (i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));

        if (item)
        {
            if (item->getEntry() == itemid && item->m_wrappedItemId == 0)
            {
                cnt += item->getStackCount() ? item->getStackCount() : 1;
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item && item->isContainer())
        {
            for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                if (item2)
                {
                    if (item2->getEntry() == itemid && item->m_wrappedItemId == 0)
                    {
                        cnt += item2->getStackCount() ? item2->getStackCount() : 1;
                    }
                }
            }

        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));

        if (item)
        {
            if (item->getItemProperties()->ItemId == itemid && item->m_wrappedItemId == 0)
            {
                cnt += item->getStackCount() ? item->getStackCount() : 1;
            }
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));

        if (item)
        {
            if (item->getItemProperties()->ItemId == itemid && item->m_wrappedItemId == 0)
            {
                cnt += item->getStackCount() ? item->getStackCount() : 1;
            }
        }
    }

    if (IncBank)
    {
        for (i = BANK_SLOT_ITEM_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16_t>(i));
            if (item)
            {
                if (item->getItemProperties()->ItemId == itemid && item->m_wrappedItemId == 0)
                {
                    cnt += item->getStackCount() ? item->getStackCount() : 1;
                }
            }
        }

        for (i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16_t>(i));
            if (item)
            {
                if (item->isContainer())
                {
                    for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                    {
                        Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                        if (item2)
                        {
                            if (item2->getItemProperties()->ItemId == itemid && item->m_wrappedItemId == 0)
                            {
                                cnt += item2->getStackCount() ? item2->getStackCount() : 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return cnt;
}

/// Removes a ammount of items from inventory
uint32_t ItemInterface::RemoveItemAmt(uint32_t id, uint32_t amt)
{
    uint32_t i;

    for (i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item)
        {
            if (item->getEntry() == id && item->m_wrappedItemId == 0)
            {
                if (item->getItemProperties()->ContainerSlots > 0 && item->isContainer() && ((Container*)item)->hasItems())
                {
                    // sounds weird? no. this will trigger a callstack display due to my other debug code.
                    item->deleteFromDB();
                    continue;
                }

                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                }
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item && item->isContainer())
        {
            for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = ((Container*)item)->getItem(static_cast<int16_t>(j));
                if (item2)
                {
                    if (item2->getItemProperties()->ItemId == id && item->m_wrappedItemId == 0)
                    {
                        if (item2->getStackCount() > amt)
                        {
                            item2->setStackCount(item2->getStackCount() - amt);
                            item2->m_isDirty = true;
                            sQuestMgr.onPlayerItemRemove(GetOwner(), item2);
                            return amt;
                        }
                        else if (item2->getStackCount() == amt)
                        {
                            bool result = SafeFullRemoveItemFromSlot(static_cast<int8_t>(i), static_cast<int16_t>(j));
                            if (result)
                            {
                                return amt;
                            }
                            else
                            {
                                return 0;
                            }
                        }
                        else
                        {
                            amt -= item2->getStackCount();
                            SafeFullRemoveItemFromSlot(static_cast<int8_t>(i), static_cast<int16_t>(j));

                        }
                    }
                }
            }
        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item)
        {
            if (item->getItemProperties()->ItemId == id && item->m_wrappedItemId == 0)
            {
                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                }
            }
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item)
        {
            if (item->getItemProperties()->ItemId == id && item->m_wrappedItemId == 0)
            {
                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                }
            }
        }
    }
    return 0;
}

uint32_t ItemInterface::RemoveItemAmt_ProtectPointer(uint32_t id, uint32_t amt, Item** pointer)
{
    //this code returns shit return value is fucked
    if (GetItemCount(id) < amt)
    {
        return 0;
    }
    uint32_t i;

    for (i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item)
        {
            if (item->getEntry() == id && item->m_wrappedItemId == 0)
            {
                if (item->getItemProperties()->ContainerSlots > 0 && item->isContainer() && ((Container*)item)->hasItems())
                {
                    // sounds weird? no. this will trigger a callstack display due to my other debug code.
                    item->deleteFromDB();
                    continue;
                }

                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    if (pointer != nullptr && *pointer == item)
                        *pointer = nullptr;

                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    if (pointer != nullptr && *pointer == item)
                        *pointer = nullptr;

                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                }
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item && item->isContainer())
        {
            for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                if (item2)
                {
                    if (item2->getItemProperties()->ItemId == id && item->m_wrappedItemId == 0)
                    {
                        if (item2->getStackCount() > amt)
                        {
                            item2->setStackCount(item2->getStackCount() - amt);
                            item2->m_isDirty = true;
                            sQuestMgr.onPlayerItemRemove(GetOwner(), item2);
                            return amt;
                        }
                        else if (item2->getStackCount() == amt)
                        {
                            if (pointer != nullptr && *pointer == item2)
                                *pointer = nullptr;

                            bool result = SafeFullRemoveItemFromSlot(static_cast<int8_t>(i), static_cast<int16_t>(j));
                            if (result)
                            {
                                return amt;
                            }
                            else
                            {
                                return 0;
                            }
                        }
                        else
                        {
                            if (pointer != nullptr && *pointer == item2)
                                *pointer = nullptr;

                            amt -= item2->getStackCount();
                            SafeFullRemoveItemFromSlot(static_cast<int8_t>(i), static_cast<int16_t>(j));
                        }
                    }
                }
            }
        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item)
        {
            if (item->getItemProperties()->ItemId == id && item->m_wrappedItemId == 0)
            {
                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    if (pointer != nullptr && *pointer == item)
                        *pointer = nullptr;

                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    if (pointer != nullptr && *pointer == item)
                        *pointer = nullptr;

                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                }
            }
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item)
        {
            if (item->getItemProperties()->ItemId == id && item->m_wrappedItemId == 0)
            {
                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    if (pointer != nullptr && *pointer == item)
                        *pointer = nullptr;

                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    if (pointer != nullptr && *pointer == item)
                        *pointer = nullptr;

                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                }
            }
        }
    }
    return 0;
}

/// Removes desired amount of items by guid
uint32_t ItemInterface::RemoveItemAmtByGuid(uint64_t guid, uint32_t amt)
{
    int16_t i;

    for (i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if (item->getGuid() == guid && item->m_wrappedItemId == 0)
            {
                if (item->getItemProperties()->ContainerSlots > 0 && item->isContainer() && static_cast<Container*>(item)->hasItems())
                {
                    // sounds weird? no. this will trigger a callstack display due to my other debug code.
                    item->deleteFromDB();
                    continue;
                }

                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    return amt;
                }
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->isContainer())
        {
            for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item)->getItem(static_cast<int16_t>(j));
                if (item2)
                {
                    if (item2->getGuid() == guid && item->m_wrappedItemId == 0)
                    {
                        if (item2->getStackCount() > amt)
                        {
                            item2->setStackCount(item2->getStackCount() - amt);
                            item2->m_isDirty = true;
                            sQuestMgr.onPlayerItemRemove(GetOwner(), item2);
                            return amt;
                        }
                        else if (item2->getStackCount() == amt)
                        {
                            bool result = SafeFullRemoveItemFromSlot(static_cast<int8_t>(i), static_cast<int16_t>(j));
                            if (result)
                            {
                                return amt;
                            }
                            else
                            {
                                return 0;
                            }
                        }
                        else
                        {
                            amt -= item2->getStackCount();
                            SafeFullRemoveItemFromSlot(static_cast<int8_t>(i), static_cast<int16_t>(j));
                            return amt;
                        }
                    }
                }
            }
        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if (item->getGuid() == guid && item->m_wrappedItemId == 0)
            {
                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    return amt;
                }
            }
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if (item->getGuid() == guid && item->m_wrappedItemId == 0)
            {
                if (item->getStackCount() > amt)
                {
                    item->setStackCount(item->getStackCount() - amt);
                    item->m_isDirty = true;
                    sQuestMgr.onPlayerItemRemove(GetOwner(), item);
                    return amt;
                }
                else if (item->getStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    if (result)
                    {
                        return amt;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    amt -= item->getStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    return amt;
                }
            }
        }
    }
    return 0;
}

void ItemInterface::RemoveAllConjured()
{
    for (uint32_t x = INVENTORY_SLOT_BAG_START; x < INVENTORY_SLOT_ITEM_END; ++x)
    {
        if (m_pItems[x] != nullptr)
        {
            if (IsBagSlot(static_cast<int16_t>(x)) && m_pItems[x]->isContainer())
            {
                Container* bag = static_cast<Container*>(m_pItems[x].get());

                for (uint32_t i = 0; i < bag->getItemProperties()->ContainerSlots; ++i)
                {
                    if (bag->getItem(static_cast<int16_t>(i)) != nullptr && bag->getItem(static_cast<int16_t>(i))->getItemProperties()->Flags & 2)
                        bag->safeFullRemoveItemFromSlot(static_cast<int16_t>(i));
                }
            }
            else
            {
                if (m_pItems[x]->getItemProperties()->Flags & 2)
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(x));
            }
        }
    }
}

/// Gets slot number by itemid, banks not included
int16_t ItemInterface::GetInventorySlotById(uint32_t ID)
{
    for (uint16_t i = 0; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getItemProperties()->ItemId == ID)
            {
                return i;
            }
        }
    }

    for (uint16_t i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getItemProperties()->ItemId == ID)
            {
                return i;
            }
        }
    }

    for (uint16_t i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getItemProperties()->ItemId == ID)
            {
                return i;
            }
        }
    }
    return ITEM_NO_SLOT_AVAILABLE;
}

/// Gets slot number by item guid, banks not included
int16_t ItemInterface::GetInventorySlotByGuid(uint64_t guid)
{
    for (uint16_t i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    for (uint16_t i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    for (uint16_t i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    return ITEM_NO_SLOT_AVAILABLE;      //was changed from 0 cuz 0 is the slot for head
}

int16_t ItemInterface::GetBagSlotByGuid(uint64_t guid)
{
    for (uint16_t i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    for (uint16_t i = INVENTORY_KEYRING_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    for (uint16_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] && m_pItems[i]->isContainer())
        {
            for (uint32_t j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
            {
                Item* inneritem = (static_cast<Container*>(m_pItems[i].get()))->getItem(static_cast<int16_t>(j));
                if (inneritem && inneritem->getGuid() == guid)
                    return i;
            }
        }
    }

    return ITEM_NO_SLOT_AVAILABLE; //was changed from 0 cuz 0 is the slot for head
}

/// Adds a Item to a free slot
std::tuple<AddItemResult, std::unique_ptr<Item>> ItemInterface::AddItemToFreeSlot(std::unique_ptr<Item> item)
{
    if (item == nullptr)
        return { ADD_ITEM_RESULT_ERROR, nullptr };

    if (item->getItemProperties() == nullptr)
        return { ADD_ITEM_RESULT_ERROR, nullptr };

    uint8_t i;
    Player* p = m_pOwner;
    uint32_t itemMaxStack = item->getItemProperties()->MaxCount;

    //detect special bag item
    if (item->getItemProperties()->BagFamily)
    {
        if (item->getItemProperties()->BagFamily & ITEM_TYPE_KEYRING || item->getItemProperties()->Class == ITEM_CLASS_KEY)
        {
            for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
            {
                if (m_pItems[i] == nullptr)
                {
                    auto [result3, returnedItem] = SafeAddItem(std::move(item), INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                    if (result3)
                    {
                        m_result.ContainerSlot = INVENTORY_SLOT_NOT_SET;
                        m_result.Slot = static_cast<int8_t>(i);
                        m_result.Result = true;
                        return { ADD_ITEM_RESULT_OK, nullptr };
                    }
                    else
                    {
                        item = std::move(returnedItem);
                    }
                }
            }
        }
        else if (item->getItemProperties()->BagFamily & ITEM_TYPE_CURRENCY)
        {
            for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
            {
                if (m_pItems[i])
                    itemMaxStack = (p->m_cheats.hasItemStackCheat) ? 0x7fffffff : m_pItems[i]->getItemProperties()->MaxCount;

                if (m_pItems[i] == nullptr)
                {
                    auto [result3, returnedItem] = SafeAddItem(std::move(item), INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
                    if (result3)
                    {
                        m_result.ContainerSlot = INVENTORY_SLOT_NOT_SET;
                        m_result.Slot = static_cast<int8_t>(i);
                        m_result.Result = true;
                        p->updateKnownCurrencies(m_pItems[i]->getEntry(), true);
                        return { ADD_ITEM_RESULT_OK, nullptr };
                    }
                    else
                    {
                        item = std::move(returnedItem);
                    }
                }
                else if (m_pItems[i]->getItemProperties()->ItemId == item->getItemProperties()->ItemId && itemMaxStack > 1 &&
                    m_pItems[i]->getStackCount() < itemMaxStack  &&
                    m_pItems[i]->getStackCount() + item->getStackCount() <= itemMaxStack)
                {
                    m_pItems[i]->setStackCount(m_pItems[i]->getStackCount() + item->getStackCount());
                    m_result.Slot = static_cast<int8_t>(i);
                    m_result.Result = true;
                    p->updateKnownCurrencies(m_pItems[i]->getEntry(), true);
                    return { ADD_ITEM_RESULT_OK, nullptr };
                }
            }
        }
        else
        {
            for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
            {
                if (m_pItems[i])
                {
                    if (m_pItems[i]->getItemProperties()->BagFamily & item->getItemProperties()->BagFamily)
                    {
                        if (m_pItems[i]->isContainer())
                        {
                            uint32_t r_slot;
                            auto [result2, returnedItem] = static_cast<Container*>(m_pItems[i].get())->addItemToFreeSlot(std::move(item), &r_slot);
                            if (result2)
                            {
                                m_result.ContainerSlot = static_cast<int8_t>(i);
                                m_result.Slot = static_cast<int8_t>(r_slot);
                                m_result.Result = true;
                                return { ADD_ITEM_RESULT_OK, nullptr };
                            }
                            else
                            {
                                item = std::move(returnedItem);
                            }
                        }
                    }
                }
            }
        }
    }

    //INVENTORY
    for (i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i])
            itemMaxStack = (p->m_cheats.hasItemStackCheat) ? 0x7fffffff : m_pItems[i]->getItemProperties()->MaxCount;
        if (m_pItems[i] == nullptr)
        {
            auto [result3, returnedItem] = SafeAddItem(std::move(item), INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(i));
            if (result3)
            {
                m_result.ContainerSlot = INVENTORY_SLOT_NOT_SET;
                m_result.Slot = static_cast<int8_t>(i);
                m_result.Result = true;
                return { ADD_ITEM_RESULT_OK, nullptr };
            }
            else
            {
                item = std::move(returnedItem);
            }
        }
        else if (m_pItems[i]->getItemProperties()->ItemId == item->getItemProperties()->ItemId &&
            itemMaxStack > 1 &&
            m_pItems[i]->getStackCount() < itemMaxStack  &&
            m_pItems[i]->getStackCount() + item->getStackCount() <= itemMaxStack)
        {
            m_pItems[i]->setStackCount(m_pItems[i]->getStackCount() + item->getStackCount());
            m_pItems[i]->m_isDirty = true;
            m_result.Slot = static_cast<int8_t>(i);
            m_result.Result = true;

            // delete the item because we added the stacks to another one
            item->deleteFromDB();
            // We make it a garbage item, so if it's used after calling this method, it gets deleted in the next Player update
            // otherwise we get a nice crash
            m_pOwner->addGarbageItem(std::move(item));

            return { ADD_ITEM_RESULT_OK, nullptr };
        }
    }

    //INVENTORY BAGS
    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr && m_pItems[i]->getItemProperties()->BagFamily == 0 && m_pItems[i]->isContainer()) //special bags ignored
        {
            for (uint32_t j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = (static_cast<Container*>(m_pItems[i].get()))->getItem(static_cast<int16_t>(j));
                if (item2)
                    itemMaxStack = (p->m_cheats.hasItemStackCheat) ? 0x7fffffff : item2->getItemProperties()->MaxCount;
                if (item2 == nullptr)
                {
                    auto [result3, returnedItem] = SafeAddItem(std::move(item), static_cast<int8_t>(i), static_cast<int16_t>(j));
                    if (result3)
                    {
                        m_result.ContainerSlot = static_cast<int8_t>(i);
                        m_result.Slot = static_cast<int8_t>(j);
                        m_result.Result = true;
                        return { ADD_ITEM_RESULT_OK, nullptr };
                    }
                    else
                    {
                        item = std::move(returnedItem);
                    }
                }
                else if (item2->getItemProperties()->ItemId == item->getItemProperties()->ItemId &&
                    itemMaxStack > 1 &&
                    item2->getStackCount() < itemMaxStack &&
                    item2->getStackCount() + item->getStackCount() <= itemMaxStack)
                {
                    item2->setStackCount(item2->getStackCount() + item->getStackCount());
                    item2->m_isDirty = true;
                    m_result.Slot = static_cast<int8_t>(i);
                    m_result.Result = true;

                    // delete the item because we added the stacks to another one
                    item->deleteFromDB();
                    // We make it a garbage item, so if it's used after calling this method, it gets deleted in the next Player update
                    // otherwise we get a nice crash
                    m_pOwner->addGarbageItem(std::move(item));

                    return { ADD_ITEM_RESULT_OK, nullptr };
                }
            }
        }
    }
    return { ADD_ITEM_RESULT_ERROR, std::move(item) };
}

/// Calculates inventory free slots, bag inventory slots not included
uint32_t ItemInterface::CalculateFreeSlots(ItemProperties const* proto)
{
    uint32_t count = 0;
    uint32_t i;

    if (proto)
    {
        if (proto->BagFamily)
        {
            if (proto->BagFamily & ITEM_TYPE_KEYRING || proto->Class == ITEM_CLASS_KEY)
            {
                for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
                {
                    if (m_pItems[i] == nullptr)
                    {
                        count++;
                    }
                }
            }
            else if (proto->BagFamily & ITEM_TYPE_CURRENCY)
            {
                for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
                {
                    if (m_pItems[i] == nullptr)
                    {
                        count++;
                    }
                }
            }
            else
            {
                for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                {
                    if (m_pItems[i] && m_pItems[i]->isContainer())
                    {
                        if (m_pItems[i]->getItemProperties()->BagFamily & proto->BagFamily)
                        {
                            int8_t slot = static_cast<Container*>(m_pItems[i].get())->findFreeSlot();
                            if (slot != ITEM_NO_SLOT_AVAILABLE)
                            {
                                count++;
                            }
                        }
                    }
                }
            }
        }
    }

    for (i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i] == nullptr)
        {
            count++;
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->isContainer() && !m_pItems[i]->getItemProperties()->BagFamily)
            {

                for (uint32_t j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = (static_cast<Container*>(m_pItems[i].get()))->getItem(static_cast<int16_t>(j));
                    if (item2 == nullptr)
                    {
                        count++;
                    }
                }
            }
        }
    }
    return count;
}

/// Finds a free slot on the backpack
int8_t ItemInterface::FindFreeBackPackSlot()
{
    //search for backpack slots
    for (int8_t i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (!item)
        {
            return i;
        }
    }

    return ITEM_NO_SLOT_AVAILABLE;      //no slots available
}

uint8_t ItemInterface::FindFreeBackPackSlotMax()
{
    //search for backpack slots
    uint8_t slotfree = 0;
    for (int8_t i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (!item) slotfree++;
    }
    return slotfree;
}

/// Converts bank bags slot ids into player bank byte slots(0-5)
uint8_t ItemInterface::GetInternalBankSlotFromPlayer(int8_t islot)
{
    switch (islot)
    {
        case BANK_SLOT_BAG_1:
        {
            return 1;
        }
        case BANK_SLOT_BAG_2:
        {
            return 2;
        }
        case BANK_SLOT_BAG_3:
        {
            return 3;
        }
        case BANK_SLOT_BAG_4:
        {
            return 4;
        }
        case BANK_SLOT_BAG_5:
        {
            return 5;
        }
        case BANK_SLOT_BAG_6:
        {
            return 6;
        }
        case BANK_SLOT_BAG_7:
        {
            return 7;
        }
        default:
            return 8;
    }
}

/// Checks if the item can be equipped on a specific slot this will check unique-equipped gems as well
int8_t ItemInterface::CanEquipItemInSlot2(int8_t DstInvSlot, int8_t slot, Item* item, bool ignore_combat /* = false */, bool skip_2h_check /* = false */)
{
    ItemProperties const* proto = item->getItemProperties();

    if (int8_t ret = CanEquipItemInSlot(DstInvSlot, slot, proto, ignore_combat, skip_2h_check))
        return ret;

#if VERSION_STRING > Classic
    if ((slot < INVENTORY_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET))
    {
        for (uint8_t count = 0; count < item->getSocketSlotCount(); count++)
        {
            const auto ei = item->getEnchantment(static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT1 + count));

            if (ei && ei->Enchantment->GemEntry)       //huh ? Gem without entry ?
            {
                ItemProperties const* ip = sMySQLStore.getItemProperties(ei->Enchantment->GemEntry);

                if (ip)             //maybe gem got removed from db due to update ?
                {
                    if (ip->Flags & ITEM_FLAG_UNIQUE_EQUIP && IsEquipped(ip->ItemId))
                    {
                        return INV_ERR_CANT_CARRY_MORE_OF_THIS;
                    }

#if VERSION_STRING > TBC
                    if (ip->ItemLimitCategory > 0)
                    {
                        uint32_t LimitId = ip->ItemLimitCategory;
                        auto item_limit_category = sItemLimitCategoryStore.lookupEntry(LimitId);
                        if (item_limit_category)
                        {
                            uint32_t gemCount = 0;
                            if ((item_limit_category->equippedFlag & ILFLAG_EQUIP_ONLY  && slot < EQUIPMENT_SLOT_END) || (!(item_limit_category->equippedFlag & ILFLAG_EQUIP_ONLY) && slot > EQUIPMENT_SLOT_END))
                                gemCount = item->countGemsWithLimitId(item_limit_category->Id);

                            uint32_t gCount = GetEquippedCountByItemLimit(item_limit_category->Id);
                            if ((gCount + gemCount) > item_limit_category->maxAmount)
                                return INV_ERR_CANT_CARRY_MORE_OF_THIS;
                        }
                    }
#endif
                }
            }
        }
    }
#endif

    return 0;
}

/// Checks if the item can be equipped on a specific slot
int8_t ItemInterface::CanEquipItemInSlot(int8_t DstInvSlot, int8_t slot, ItemProperties const* proto, bool ignore_combat /* = false */, bool skip_2h_check /* = false */)
{
    if (proto == nullptr)
        return INV_ERR_ITEMS_CANT_BE_SWAPPED;

    uint32_t type = proto->InventoryType;

    if (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END && DstInvSlot == -1)
        if (proto->ContainerSlots == 0)
            return INV_ERR_ITEMS_CANT_BE_SWAPPED;

    if ((slot < INVENTORY_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET))
    {
        if (!ignore_combat && m_pOwner->getCombatHandler().isInCombat() && (slot < EQUIPMENT_SLOT_MAINHAND || slot > EQUIPMENT_SLOT_RANGED))
            return INV_ERR_CANT_DO_IN_COMBAT;

        if (IsEquipped(proto->ItemId) && (proto->Unique || proto->Flags & ITEM_FLAG_UNIQUE_EQUIP))
            return INV_ERR_CANT_CARRY_MORE_OF_THIS;

        // Check to see if we have the correct race
        if (!(proto->AllowableRace & m_pOwner->getRaceMask()))
            return INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;

        // Check to see if we have the correct class
        if (!(proto->AllowableClass & m_pOwner->getClassMask()))
            return INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM2;

        // Check to see if we have the reqs for that reputation
        if (proto->RequiredFaction)
        {
            Standing current_standing = Player::getReputationRankFromStanding(m_pOwner->getFactionStanding(proto->RequiredFaction));
            if (current_standing < (Standing)proto->RequiredFactionStanding)       // Not enough rep rankage..
                return INV_ERR_ITEM_REPUTATION_NOT_ENOUGH;
        }

        // Check to see if we have the correct level.
        if (proto->RequiredLevel > m_pOwner->getLevel())
            return INV_ERR_YOU_MUST_REACH_LEVEL_N;

        if (proto->Class == 4)
        {
            uint32_t bogus_subclass = 0;
            uint32_t playerlevel = 0;
            // scaling heirloom items
            if (proto->ScalingStatsEntry != 0 && proto->ScalingStatsFlag != 0)
            {
                playerlevel = m_pOwner->getLevel();

                if (playerlevel < 40 && proto->SubClass >= 3)
                    bogus_subclass = proto->SubClass - 1; // mail items become leather, plate items become mail below lvl40
                else bogus_subclass = proto->SubClass;
            }
            else
                bogus_subclass = proto->SubClass;

            if (!(m_pOwner->getArmorProficiency() & (((uint32_t)(1)) << bogus_subclass)))
                return INV_ERR_NO_REQUIRED_PROFICIENCY;

        }
        else if (proto->Class == 2)
        {
            if (!(m_pOwner->getWeaponProficiency() & (((uint32_t)(1)) << proto->SubClass)))
                return INV_ERR_NO_REQUIRED_PROFICIENCY;
        }

        if (proto->RequiredSkill)
            if (proto->RequiredSkillRank > m_pOwner->getSkillLineCurrent(proto->RequiredSkill, true))
                return INV_ERR_SKILL_ISNT_HIGH_ENOUGH;

        if (proto->RequiredSkillSubRank)
            if (!m_pOwner->hasSpell(proto->RequiredSkillSubRank))
                return INV_ERR_NO_REQUIRED_PROFICIENCY;

        // You are dead !
        if (m_pOwner->getDeathState() != ALIVE)
            return INV_ERR_YOU_ARE_DEAD;
    }

    switch (uint8_t(slot))        //CURRENCYTOKEN_SLOT_ are over 128
    {
        case EQUIPMENT_SLOT_HEAD:
        {
            if (type == INVTYPE_HEAD)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_NECK:
        {
            if (type == INVTYPE_NECK)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_SHOULDERS:
        {
            if (type == INVTYPE_SHOULDERS)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_BODY:
        {
            if (type == INVTYPE_BODY)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_CHEST:
        {
            if (type == INVTYPE_CHEST || type == INVTYPE_ROBE)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_WAIST:
        {
            if (type == INVTYPE_WAIST)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_LEGS:
        {
            if (type == INVTYPE_LEGS)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_FEET:
        {
            if (type == INVTYPE_FEET)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_WRISTS:
        {
            if (type == INVTYPE_WRISTS)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_HANDS:
        {
            if (type == INVTYPE_HANDS)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_FINGER1:
        case EQUIPMENT_SLOT_FINGER2:
        {
            if (type == INVTYPE_FINGER)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_TRINKET1:
        case EQUIPMENT_SLOT_TRINKET2:
        {
            if (type == INVTYPE_TRINKET)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_BACK:
        {
            if (type == INVTYPE_CLOAK)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_MAINHAND:
        {
            if (type == INVTYPE_WEAPON || type == INVTYPE_WEAPONMAINHAND ||
                (type == INVTYPE_2HWEAPON && (!GetInventoryItem(EQUIPMENT_SLOT_OFFHAND) || skip_2h_check || m_pOwner->canDualWield2H())))
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_OFFHAND:
        {
            if ((type == INVTYPE_2HWEAPON || type == INVTYPE_SHIELD) && m_pOwner->canDualWield2H())
            {
                return 0;
            }

            if (type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND)
            {
                Item* mainweapon = GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (mainweapon)     //item exists
                {
                    if (mainweapon->getItemProperties()->InventoryType != INVTYPE_2HWEAPON)
                    {
                        if (m_pOwner->hasSkillLine(SKILL_DUAL_WIELD))
                            return 0;
                        else
                            return INV_ERR_CANT_DUAL_WIELD;
                    }
                    else
                    {
                        if (!skip_2h_check)
                            return INV_ERR_CANT_EQUIP_WITH_TWOHANDED;
                        else
                            return 0;
                    }
                }
                else
                {
                    if (m_pOwner->hasSkillLine(SKILL_DUAL_WIELD))
                        return 0;
                    else
                        return INV_ERR_CANT_DUAL_WIELD;
                }
            }
            else if (type == INVTYPE_SHIELD || type == INVTYPE_HOLDABLE)
            {
                Item* mainweapon = GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (mainweapon)     //item exists
                {
                    if (mainweapon->getItemProperties()->InventoryType != INVTYPE_2HWEAPON)
                    {
                        return 0;
                    }
                    else
                    {
                        if (!skip_2h_check)
                            return INV_ERR_CANT_EQUIP_WITH_TWOHANDED;
                        else
                            return 0;
                    }
                }
                else
                {
                    return 0;
                }
            }
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_RANGED:
        {
            if (type == INVTYPE_RANGED || type == INVTYPE_THROWN || type == INVTYPE_RANGEDRIGHT || type == INVTYPE_RELIC)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;  // 6;
        }
        case EQUIPMENT_SLOT_TABARD:
        {
            if (type == INVTYPE_TABARD)
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;  // 6;
        }
        case BANK_SLOT_BAG_1:
        case BANK_SLOT_BAG_2:
        case BANK_SLOT_BAG_3:
        case BANK_SLOT_BAG_4:
        case BANK_SLOT_BAG_5:
        case BANK_SLOT_BAG_6:
        case BANK_SLOT_BAG_7:
        {
            if (!GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot))        //check if player got that slot.
            {
                const uint8_t slots = m_pOwner->getBankSlots();
                const uint8_t islot = GetInternalBankSlotFromPlayer(slot);
                if (slots < islot)
                {
                    return INV_ERR_MUST_PURCHASE_THAT_BAG_SLOT;
                }

                if (type == INVTYPE_BAG)                                //in case bank slot exists, check if player can put the item there
                {
                    return 0;
                }
                else
                {
                    return INV_ERR_NOT_A_BAG;
                }
            }
            else
            {
                if (GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot)->getItemProperties()->BagFamily)
                {
                    if ((IsBagSlot(slot) && DstInvSlot == INVENTORY_SLOT_NOT_SET))
                    {
                        if (proto->InventoryType == INVTYPE_BAG)
                        {
                            return 0;
                        }
                    }

                    if (proto->BagFamily & GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot)->getItemProperties()->BagFamily)
                    {
                        return 0;
                    }
                    else
                    {
                        return INV_ERR_ITEM_DOESNT_GO_INTO_BAG;
                    }
                }
                else
                {
                    return 0;
                }
            }
        }
        case INVENTORY_SLOT_BAG_1:
        case INVENTORY_SLOT_BAG_2:
        case INVENTORY_SLOT_BAG_3:
        case INVENTORY_SLOT_BAG_4:
        {
            //this chunk of code will limit you to equip only 1 Ammo Bag. Later i found out that this is not blizzlike so i will remove it when it's blizzlike
            //we are trying to equip an Ammo Bag
            if (proto->Class == ITEM_CLASS_QUIVER)
            {
                //check if we already have an AB equipped
                FindAmmoBag();
                //we do have ammo bag but we are not swapping them then we send error
                if (m_result.Slot != ITEM_NO_SLOT_AVAILABLE && m_result.Slot != slot)
                {
                    return INV_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH;
                }
            }
            if (GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot))
            {
                if (GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot)->getItemProperties()->BagFamily)
                {
                    if ((IsBagSlot(slot) && DstInvSlot == INVENTORY_SLOT_NOT_SET))
                    {
                        if (proto->InventoryType == INVTYPE_BAG)
                        {
                            return 0;
                        }
                    }

                    if (proto->BagFamily & GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot)->getItemProperties()->BagFamily)
                    {
                        return 0;
                    }
                    else
                    {
                        return INV_ERR_ITEM_DOESNT_GO_INTO_BAG;
                    }
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                if (type == INVTYPE_BAG)
                {
                    return 0;
                }
                else
                {
                    return INV_ERR_NOT_A_BAG;
                }
            }
        }
        case INVENTORY_SLOT_ITEM_1:
        case INVENTORY_SLOT_ITEM_2:
        case INVENTORY_SLOT_ITEM_3:
        case INVENTORY_SLOT_ITEM_4:
        case INVENTORY_SLOT_ITEM_5:
        case INVENTORY_SLOT_ITEM_6:
        case INVENTORY_SLOT_ITEM_7:
        case INVENTORY_SLOT_ITEM_8:
        case INVENTORY_SLOT_ITEM_9:
        case INVENTORY_SLOT_ITEM_10:
        case INVENTORY_SLOT_ITEM_11:
        case INVENTORY_SLOT_ITEM_12:
        case INVENTORY_SLOT_ITEM_13:
        case INVENTORY_SLOT_ITEM_14:
        case INVENTORY_SLOT_ITEM_15:
        case INVENTORY_SLOT_ITEM_16:
        {
            return 0;
        }
        case INVENTORY_KEYRING_1:
        case INVENTORY_KEYRING_2:
        case INVENTORY_KEYRING_3:
        case INVENTORY_KEYRING_4:
        case INVENTORY_KEYRING_5:
        case INVENTORY_KEYRING_6:
        case INVENTORY_KEYRING_7:
        case INVENTORY_KEYRING_8:
        case INVENTORY_KEYRING_9:
        case INVENTORY_KEYRING_10:
        case INVENTORY_KEYRING_11:
        case INVENTORY_KEYRING_12:
        case INVENTORY_KEYRING_13:
        case INVENTORY_KEYRING_14:
        case INVENTORY_KEYRING_15:
        case INVENTORY_KEYRING_16:
        case INVENTORY_KEYRING_17:
        case INVENTORY_KEYRING_18:
        case INVENTORY_KEYRING_19:
        case INVENTORY_KEYRING_20:
        case INVENTORY_KEYRING_21:
        case INVENTORY_KEYRING_22:
        case INVENTORY_KEYRING_23:
        case INVENTORY_KEYRING_24:
        case INVENTORY_KEYRING_25:
        case INVENTORY_KEYRING_26:
        case INVENTORY_KEYRING_27:
        case INVENTORY_KEYRING_28:
        case INVENTORY_KEYRING_29:
        case INVENTORY_KEYRING_30:
        case INVENTORY_KEYRING_31:
        case INVENTORY_KEYRING_32:
        {
            if (proto->BagFamily & ITEM_TYPE_KEYRING || proto->Class == ITEM_CLASS_KEY)
            {
                return 0;
            }
            else
            {
                return INV_ERR_ITEM_DOESNT_GO_INTO_BAG;
            }
        }
        case CURRENCYTOKEN_SLOT_1:
        case CURRENCYTOKEN_SLOT_2:
        case CURRENCYTOKEN_SLOT_3:
        case CURRENCYTOKEN_SLOT_4:
        case CURRENCYTOKEN_SLOT_5:
        case CURRENCYTOKEN_SLOT_6:
        case CURRENCYTOKEN_SLOT_7:
        case CURRENCYTOKEN_SLOT_8:
        case CURRENCYTOKEN_SLOT_9:
        case CURRENCYTOKEN_SLOT_10:
        case CURRENCYTOKEN_SLOT_11:
        case CURRENCYTOKEN_SLOT_12:
        case CURRENCYTOKEN_SLOT_13:
        case CURRENCYTOKEN_SLOT_14:
        case CURRENCYTOKEN_SLOT_15:
        case CURRENCYTOKEN_SLOT_16:
        case CURRENCYTOKEN_SLOT_17:
        case CURRENCYTOKEN_SLOT_18:
        case CURRENCYTOKEN_SLOT_19:
        case CURRENCYTOKEN_SLOT_20:
        case CURRENCYTOKEN_SLOT_21:
        case CURRENCYTOKEN_SLOT_22:
        case CURRENCYTOKEN_SLOT_23:
        case CURRENCYTOKEN_SLOT_24:
        case CURRENCYTOKEN_SLOT_25:
        case CURRENCYTOKEN_SLOT_26:
        case CURRENCYTOKEN_SLOT_27:
        case CURRENCYTOKEN_SLOT_28:
        case CURRENCYTOKEN_SLOT_29:
        case CURRENCYTOKEN_SLOT_30:
        case CURRENCYTOKEN_SLOT_31:
        case CURRENCYTOKEN_SLOT_32:
        {
            if (proto->BagFamily & ITEM_TYPE_CURRENCY)
            {
                return 0;
            }
            else
            {
                return INV_ERR_ITEM_DOESNT_GO_INTO_BAG;
            }
        }
        default:
            return 0;
    }
}

/// Checks if player can receive the item
int8_t ItemInterface::CanReceiveItem(ItemProperties const* item, uint32_t amount)
{
    if (!item)
    {
        return INV_ERR_OK;
    }

    if (item->Unique)
    {
        uint32_t count = GetItemCount(item->ItemId, true);
        if (count == item->Unique || ((count + amount) > item->Unique))
        {
            return INV_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }

#if VERSION_STRING > TBC
    if (item->ItemLimitCategory > 0)
    {
        auto item_limit_category = sItemLimitCategoryStore.lookupEntry(item->ItemLimitCategory);
        if (item_limit_category && !(item_limit_category->equippedFlag & ILFLAG_EQUIP_ONLY))
        {
            uint32_t count = GetItemCountByLimitId(item_limit_category->Id, false);
            if (count >= item_limit_category->maxAmount || ((count + amount) > item_limit_category->maxAmount))
                return INV_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED;
        }
    }
#endif

    return INV_ERR_OK;
}

void ItemInterface::BuyItem(ItemProperties const* item, uint32_t total_amount, Creature* pVendor)
{
    if (item->BuyPrice)
    {
        uint32_t factionStanding = m_pOwner->getFactionStandingRank(pVendor->m_factionTemplate->Faction);
        uint32_t itemprice = item->getBuyPriceForItem(total_amount, factionStanding);
        if (!m_pOwner->hasEnoughCoinage(itemprice))
            m_pOwner->setCoinage(0);
        else
            m_pOwner->modCoinage(-(int32_t)itemprice);
    }
    auto item_extended_cost = pVendor->GetItemExtendedCostByItemId(item->ItemId);
    if (item_extended_cost != nullptr)
    {
        for (uint8_t i = 0; i < 5; ++i)
        {
            if (item_extended_cost->item[i])
                m_pOwner->getItemInterface()->RemoveItemAmt(item_extended_cost->item[i], total_amount * item_extended_cost->count[i]);
        }

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
        if (m_pOwner->getHonorCurrency() >= (item_extended_cost->honor_points * total_amount))
        {
            m_pOwner->modHonorCurrency(-int32_t((item_extended_cost->honor_points * total_amount)));
            m_pOwner->removeArenaPoints(item_extended_cost->honor_points * total_amount, true);
        }

        if (m_pOwner->getArenaCurrency() >= item_extended_cost->arena_points * total_amount)
        {
            m_pOwner->modArenaCurrency(-int32_t(item_extended_cost->arena_points * total_amount));
            m_pOwner->removeArenaPoints(item_extended_cost->arena_points * total_amount, true);
        }
#endif
#endif
    }
}

int8_t ItemInterface::CanAffordItem(ItemProperties const* item, uint32_t amount, Creature* pVendor)
{
    auto item_extended_cost = pVendor->GetItemExtendedCostByItemId(item->ItemId);
    if (item_extended_cost != nullptr)
    {
        for (uint8_t i = 0; i < 5; ++i)
        {
            if (item_extended_cost->item[i])
            {
                if (m_pOwner->getItemInterface()->GetItemCount(item_extended_cost->item[i], false) < (item_extended_cost->count[i] * amount))
                    return INV_ERR_VENDOR_MISSING_TURNINS;
            }
        }

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
        if (m_pOwner->getHonorCurrency() < (item_extended_cost->honor_points * amount))
            return INV_ERR_NOT_ENOUGH_HONOR_POINTS;

        if (m_pOwner->getArenaCurrency() < item_extended_cost->arena_points * amount)
            return INV_ERR_NOT_ENOUGH_ARENA_POINTS;
#endif
#endif
        if (m_pOwner->getMaxPersonalRating() < item_extended_cost->personalrating)
            return INV_ERR_PERSONAL_ARENA_RATING_TOO_LOW;
    }

    if (item->BuyPrice)
    {
        uint32_t factionStanding = m_pOwner->getFactionStandingRank(pVendor->m_factionTemplate->Faction);
        uint32_t price = item->getBuyPriceForItem(amount, factionStanding) * amount;
        if (!m_pOwner->hasEnoughCoinage(price))
        {
            return INV_ERR_NOT_ENOUGH_MONEY;
        }
    }

    if (item->RequiredFaction)
    {
        WDB::Structures::FactionEntry const* factdbc = sFactionStore.lookupEntry(item->RequiredFaction);
        if (!factdbc || factdbc->RepListId < 0)
            return INV_ERR_OK;

        if (m_pOwner->getReputationRankFromStanding(m_pOwner->getFactionStanding(item->RequiredFaction)) < (int32_t)item->RequiredFactionStanding)
        {
            return INV_ERR_ITEM_REPUTATION_NOT_ENOUGH;
        }
    }
    return INV_ERR_OK;
}

// Gets the Item slot by item type
int8_t ItemInterface::GetItemSlotByType(uint32_t type)
{
    switch (type)
    {
        case INVTYPE_NON_EQUIP:
            return ITEM_NO_SLOT_AVAILABLE;
        case INVTYPE_HEAD:
            return EQUIPMENT_SLOT_HEAD;
        case INVTYPE_NECK:
            return EQUIPMENT_SLOT_NECK;
        case INVTYPE_SHOULDERS:
            return EQUIPMENT_SLOT_SHOULDERS;
        case INVTYPE_BODY:
            return EQUIPMENT_SLOT_BODY;
        case INVTYPE_CHEST:
            return EQUIPMENT_SLOT_CHEST;
        case INVTYPE_ROBE: // ???
            return EQUIPMENT_SLOT_CHEST;
        case INVTYPE_WAIST:
            return EQUIPMENT_SLOT_WAIST;
        case INVTYPE_LEGS:
            return EQUIPMENT_SLOT_LEGS;
        case INVTYPE_FEET:
            return EQUIPMENT_SLOT_FEET;
        case INVTYPE_WRISTS:
            return EQUIPMENT_SLOT_WRISTS;
        case INVTYPE_HANDS:
            return EQUIPMENT_SLOT_HANDS;
        case INVTYPE_FINGER:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_FINGER1))
                return EQUIPMENT_SLOT_FINGER1;
            if (!GetInventoryItem(EQUIPMENT_SLOT_FINGER2))
                return EQUIPMENT_SLOT_FINGER2;
            
            return EQUIPMENT_SLOT_FINGER1;          //auto equips always in finger 1
        }
        case INVTYPE_TRINKET:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_TRINKET1))
                return EQUIPMENT_SLOT_TRINKET1;
            if (!GetInventoryItem(EQUIPMENT_SLOT_TRINKET2))
                return EQUIPMENT_SLOT_TRINKET2;
            
            return EQUIPMENT_SLOT_TRINKET1;         //auto equips always on trinket 1
        }
        case INVTYPE_CLOAK:
            return EQUIPMENT_SLOT_BACK;
        case INVTYPE_SHIELD:
            return EQUIPMENT_SLOT_OFFHAND;
        case INVTYPE_RANGED:
            return EQUIPMENT_SLOT_RANGED;
        case INVTYPE_WEAPON:
        case INVTYPE_2HWEAPON:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
                return EQUIPMENT_SLOT_MAINHAND;
            if (!GetInventoryItem(EQUIPMENT_SLOT_OFFHAND))
                return EQUIPMENT_SLOT_OFFHAND;
            
             return EQUIPMENT_SLOT_MAINHAND;
        }
        case INVTYPE_TABARD:
            return EQUIPMENT_SLOT_TABARD;
        case INVTYPE_WEAPONMAINHAND:
            return EQUIPMENT_SLOT_MAINHAND;
        case INVTYPE_WEAPONOFFHAND:
            return EQUIPMENT_SLOT_OFFHAND;
        case INVTYPE_HOLDABLE:
            return EQUIPMENT_SLOT_OFFHAND;
        case INVTYPE_THROWN:
            return EQUIPMENT_SLOT_RANGED;   // ?
        case INVTYPE_RANGEDRIGHT:
            return EQUIPMENT_SLOT_RANGED;   // ?
        case INVTYPE_RELIC:
            return EQUIPMENT_SLOT_RANGED;
        case INVTYPE_BAG:
        {
            for (int8_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            {
                if (!GetInventoryItem(i))
                    return i;
            }
            return ITEM_NO_SLOT_AVAILABLE;      //bags are not supposed to be auto-equipped when slots are not free
        }
        default:
            return ITEM_NO_SLOT_AVAILABLE;
    }
}

/// Gets a Item by guid
Item* ItemInterface::GetItemByGUID(uint64_t Guid)
{
    uint32_t i;

    //EQUIPMENT
    for (i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getGuid() == Guid)
            {
                m_result.ContainerSlot = static_cast<int8_t>(INVALID_BACKPACK_SLOT);//not a containerslot. In 1.8 client marked wrong slot like this
                m_result.Slot = static_cast<int8_t>(i);
                return m_pItems[i].get();
            }
        }
    }

    //INVENTORY BAGS
    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr && m_pItems[i]->isContainer())
        {
            if (m_pItems[i]->getGuid() == Guid)
            {
                m_result.ContainerSlot = static_cast<int8_t>(INVALID_BACKPACK_SLOT);
                m_result.Slot = static_cast<int8_t>(i);
                return m_pItems[i].get();
            }

            for (uint32_t j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = (static_cast<Container*>(m_pItems[i].get()))->getItem(static_cast<int16_t>(j));
                if (item2)
                {
                    if (item2->getGuid() == Guid)
                    {
                        m_result.ContainerSlot = static_cast<int8_t>(i);
                        m_result.Slot = static_cast<int8_t>(j);
                        return item2;
                    }
                }
            }
        }
    }

    //INVENTORY
    for (i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getGuid() == Guid)
            {
                m_result.ContainerSlot = static_cast<int8_t>(INVALID_BACKPACK_SLOT);
                m_result.Slot = static_cast<int8_t>(i);
                return m_pItems[i].get();
            }
        }
    }

    //Keyring
    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getGuid() == Guid)
            {
                m_result.ContainerSlot = static_cast<int8_t>(INVALID_BACKPACK_SLOT);
                m_result.Slot = static_cast<int8_t>(i);
                return m_pItems[i].get();
            }
        }
    }

    //Currency
    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getGuid() == Guid)
            {
                m_result.ContainerSlot = static_cast<int8_t>(INVALID_BACKPACK_SLOT);
                m_result.Slot = static_cast<int8_t>(i);
                return m_pItems[i].get();
            }
        }
    }
    return nullptr;
}

void ItemInterface::EmptyBuyBack()
{
    for (uint8_t j = 0; j < MAX_BUYBACK_SLOT; ++j)
    {
        if (m_pBuyBack[j] != nullptr)
        {
            m_pOwner->sendDestroyObjectPacket(m_pBuyBack[j]->getGuid());
            m_pBuyBack[j]->deleteFromDB();

            if (m_pBuyBack[j]->isContainer())
            {
                if (m_pBuyBack[j]->IsInWorld())
                    m_pBuyBack[j]->removeFromWorld();
            }
            else
            {
                if (m_pBuyBack[j]->IsInWorld())
                    m_pBuyBack[j]->removeFromWorld();
            }

            m_pOwner->setVendorBuybackSlot(j, 0);
            m_pOwner->setBuybackPriceSlot(j, 0);
            m_pOwner->setBuybackTimestampSlot(j, 0);
            m_pBuyBack[j] = nullptr;
        }
        else
            break;
    }
}

void ItemInterface::AddBuyBackItem(std::unique_ptr<Item> it, uint32_t price)
{
    // If all slots are used, first set buyback item gets removed and new one gets added on last position
    if ((m_pBuyBack[MAX_BUYBACK_SLOT - 1] != nullptr) && (m_pOwner->getVendorBuybackSlot(MAX_BUYBACK_SLOT - 1) != 0))
    {
        if (m_pBuyBack[0] != nullptr)
        {
            m_pOwner->sendDestroyObjectPacket(m_pBuyBack[0]->getGuid());
            m_pBuyBack[0]->deleteFromDB();

            if (m_pBuyBack[0]->isContainer())
            {
                if (m_pBuyBack[0]->IsInWorld())
                    m_pBuyBack[0]->removeFromWorld();
            }
            else
            {
                if (m_pBuyBack[0]->IsInWorld())
                    m_pBuyBack[0]->removeFromWorld();
            }

            m_pBuyBack[0] = nullptr;
        }

        for (uint8_t j = 0; j < MAX_BUYBACK_SLOT - 1; ++j)
        {
            m_pOwner->setVendorBuybackSlot(j, m_pOwner->getVendorBuybackSlot(j + 1));
            m_pOwner->setBuybackPriceSlot(j, m_pOwner->getBuybackPriceSlot(j + 1));
            m_pOwner->setBuybackTimestampSlot(j, m_pOwner->getBuybackTimestampSlot(j + 1));
            m_pBuyBack[j] = std::move(m_pBuyBack[j + 1]);
        }
        m_pBuyBack[MAX_BUYBACK_SLOT - 1] = std::move(it);

        m_pOwner->setVendorBuybackSlot(MAX_BUYBACK_SLOT - 1, m_pBuyBack[MAX_BUYBACK_SLOT - 1]->getGuid());
        m_pOwner->setBuybackPriceSlot(MAX_BUYBACK_SLOT - 1, price);
        m_pOwner->setBuybackTimestampSlot(MAX_BUYBACK_SLOT - 1, static_cast<uint32_t>(UNIXTIME));
    }
    else
    {
        for (uint8_t i = 0; i < MAX_BUYBACK_SLOT - 1; ++i) //at least 1 slot is empty
        {
            if (m_pOwner->getVendorBuybackSlot(i) == 0 || m_pBuyBack[i] == nullptr)
            {
                sLogger.info("setting buybackslot {}", i / 2);
                m_pBuyBack[i] = std::move(it);

                // TODO: there are actually 24 slots -Appled
                // arcemu: m_pOwner->SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2 * j), m_pOwner->GetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ((j + 1) * 2)));
                m_pOwner->setVendorBuybackSlot(i, m_pBuyBack[i]->getGuid());
                m_pOwner->setBuybackPriceSlot(i, price);
                m_pOwner->setBuybackTimestampSlot(i, static_cast<uint32_t>(UNIXTIME));
                return;
            }
        }
    }
}

std::unique_ptr<Item> ItemInterface::RemoveBuyBackItem(uint8_t index)
{
    if (index >= MAX_BUYBACK_SLOT)
        return nullptr;

    uint8_t j = 0;
    std::unique_ptr<Item> itemHolder = std::move(m_pBuyBack[index]);
    for (j = index; j < MAX_BUYBACK_SLOT - 1; ++j)
    {
        if (m_pOwner->getVendorBuybackSlot(j) != 0)
        {
            m_pOwner->setVendorBuybackSlot(j, m_pOwner->getVendorBuybackSlot(j + 1));
            m_pOwner->setBuybackPriceSlot(j, m_pOwner->getBuybackPriceSlot(j + 1));
            m_pOwner->setBuybackTimestampSlot(j, m_pOwner->getBuybackTimestampSlot(j + 1));

            if (m_pBuyBack[j + 1] != nullptr && m_pOwner->getVendorBuybackSlot(j + 1) != 0)
            {
                m_pBuyBack[j] = std::move(m_pBuyBack[j + 1]);
            }
            else
            {
                m_pBuyBack[j] = nullptr;

                sLogger.info("nulling {}", j);
            }
        }
        else
            return itemHolder;
    }

    j = 11;
    m_pOwner->setVendorBuybackSlot(j, 0);
    m_pOwner->setBuybackPriceSlot(j, 0);
    m_pOwner->setBuybackTimestampSlot(j, 0);

    return itemHolder;
}

/// Swap inventory slots
void ItemInterface::SwapItemSlots(int8_t srcslot, int8_t dstslot)
{
    // srcslot and dstslot are int... NULL might not be an int depending on arch where it is compiled
    if (srcslot >= INVENTORY_KEYRING_END || srcslot < 0)
        return;

    if (dstslot >= INVENTORY_KEYRING_END || dstslot < 0)
        return;

    Item* SrcItem = GetInventoryItem(srcslot);
    Item* DstItem = GetInventoryItem(dstslot);

    sLogger.debug("ItemInterface::SwapItemSlots({}, {});", srcslot, dstslot);
    //Item * temp = GetInventoryItem(srcslot);
    //if (temp)
    //    sLogger.debug("Source item: {} (inventoryType={}, realslot={});" , temp->GetProto()->Name1 , temp->GetProto()->InventoryType , GetItemSlotByType(temp->GetProto()->InventoryType));
    //    temp = GetInventoryItem(dstslot);
    //if (temp)
    //    sLogger.debug("Destination: Item: {} (inventoryType={}, realslot={});" , temp->GetProto()->Name1 , temp->GetProto()->InventoryType , GetItemSlotByType(temp->GetProto()->InventoryType));
    //else
    //    sLogger.debug("Destination: Empty");

    // don't stack equipped items (even with hasItemStackCheat), just swap them
    uint32_t srcItemMaxStack, dstItemMaxStack;
    if (SrcItem != nullptr)
    {
        if (srcslot < INVENTORY_SLOT_BAG_END || !(SrcItem->getOwner()->m_cheats.hasItemStackCheat))
        {
            srcItemMaxStack = SrcItem->getItemProperties()->MaxCount;
        }
        else
        {
            srcItemMaxStack = 0x7fffffff;
        }
    }
    else
    {
        srcItemMaxStack = 0;
    }
    if (DstItem != nullptr)
    {
        if (dstslot < INVENTORY_SLOT_BAG_END || !(DstItem->getOwner()->m_cheats.hasItemStackCheat))
        {
            dstItemMaxStack = DstItem->getItemProperties()->MaxCount;
        }
        else
        {
            dstItemMaxStack = 0x7fffffff;
        }
    }
    else
    {
        dstItemMaxStack = 0;
    }

    if (SrcItem != nullptr && DstItem != nullptr && SrcItem->getEntry() == DstItem->getEntry() && srcItemMaxStack > 1 && SrcItem->m_wrappedItemId == 0 && DstItem->m_wrappedItemId == 0)
    {
        uint32_t total = SrcItem->getStackCount() + DstItem->getStackCount();
        if (total <= dstItemMaxStack)
        {
            DstItem->modStackCount(SrcItem->getStackCount());
            SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, srcslot);
            DstItem->m_isDirty = true;
            return;
        }
        else
        {
            if (DstItem->getStackCount() == dstItemMaxStack)
            {

            }
            else
            {
                int32_t delta = dstItemMaxStack - DstItem->getStackCount();
                DstItem->setStackCount(dstItemMaxStack);
                SrcItem->modStackCount(-delta);
                SrcItem->m_isDirty = true;
                DstItem->m_isDirty = true;
                return;
            }
        }
    }

    //src item was equipped previously
    if (srcslot < INVENTORY_SLOT_BAG_END)
    {
        if (m_pItems[(int)srcslot] != nullptr)
            m_pOwner->applyItemMods(m_pItems[srcslot].get(), srcslot, false);
    }

    //dst item was equipped previously
    if (dstslot < INVENTORY_SLOT_BAG_END)
    {
        if (m_pItems[(int)dstslot] != nullptr)
            m_pOwner->applyItemMods(m_pItems[dstslot].get(), dstslot, false);
    }

    //sLogger.debug("Putting items into slots...");

    std::unique_ptr<Item> tmpDstItemHolder = std::move(m_pItems[dstslot]);
    m_pItems[dstslot] = std::move(m_pItems[srcslot]);

    // Moving a bag with items to a empty bagslot
    if (tmpDstItemHolder == nullptr && SrcItem != nullptr && SrcItem->isContainer())
    {
        std::unique_ptr<Item> tSrcItem;

        for (uint32_t Slot = 0; Slot < SrcItem->getItemProperties()->ContainerSlots; ++Slot)
        {
            tSrcItem = m_pOwner->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srcslot, static_cast<int16_t>(Slot), false);
            if (tSrcItem != nullptr)
            {
                const auto [result, returnedItem] = m_pOwner->getItemInterface()->SafeAddItem(std::move(tSrcItem), dstslot, static_cast<int16_t>(Slot));
                if (!result)
                {
                    // TODO: if add failed, should item be sent in mail to player? now it's destroyed -Appled
                    sLogger.failure("Error while adding item {} to player {}", returnedItem->getEntry(), m_pOwner->getName());
                    return;
                }
            }
        }
    }

    m_pItems[srcslot] = std::move(tmpDstItemHolder);

    // swapping 2 bags filled with items
    if (DstItem != nullptr && SrcItem != nullptr && SrcItem->isContainer() && DstItem->isContainer())
    {
        std::unique_ptr<Item> tDstItem;
        std::unique_ptr<Item> tSrcItem;
        uint32_t TotalSlots = 0;

        // Determine the max amount of slots to swap
        if (SrcItem->getItemProperties()->ContainerSlots > DstItem->getItemProperties()->ContainerSlots)
            TotalSlots = SrcItem->getItemProperties()->ContainerSlots;
        else
            TotalSlots = DstItem->getItemProperties()->ContainerSlots;

        // swap items in the bags
        for (uint32_t Slot = 0; Slot < TotalSlots; ++Slot)
        {
            tSrcItem = m_pOwner->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srcslot, static_cast<int16_t>(Slot), false);
            tDstItem = m_pOwner->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(dstslot, static_cast<int16_t>(Slot), false);

            // TODO: if add failed, should items be sent in mail to player? now they are destroyed -Appled
            if (tSrcItem != nullptr)
                (static_cast<Container*>(DstItem))->addItem(static_cast<int16_t>(Slot), std::move(tSrcItem));
            if (tDstItem != nullptr)
                (static_cast<Container*>(SrcItem))->addItem(static_cast<int16_t>(Slot), std::move(tDstItem));
        }
    }

    if (DstItem != nullptr)
        DstItem->m_isDirty = true;
    if (SrcItem != nullptr)
        SrcItem->m_isDirty = true;

    if (m_pItems[(int)dstslot] != nullptr)
    {
        //sLogger.debug("(SrcItem) PLAYER_FIELD_INV_SLOT_HEAD + {} is now {}" , dstslot , m_pItems[(int)dstslot]->getGuid());
        setOwnerInventoryItem(static_cast<uint8_t>(dstslot), m_pItems[(int)dstslot]->getGuid());
    }
    else
    {
        //sLogger.debug("(SrcItem) PLAYER_FIELD_INV_SLOT_HEAD + {} is now 0" , dstslot);
        setOwnerInventoryItem(static_cast<uint8_t>(dstslot), 0);
    }

    if (m_pItems[(int)srcslot] != nullptr)
    {
        //sLogger.debug("(DstItem) PLAYER_FIELD_INV_SLOT_HEAD + {} is now {}" , dstslot , m_pItems[(int)srcslot]->getGuid());
        setOwnerInventoryItem(static_cast<uint8_t>(srcslot), m_pItems[(int)srcslot]->getGuid());
    }
    else
    {
        //sLogger.debug("(DstItem) PLAYER_FIELD_INV_SLOT_HEAD + {} is now 0" , dstslot);
        setOwnerInventoryItem(static_cast<uint8_t>(srcslot), 0);
    }

    if (srcslot < INVENTORY_SLOT_BAG_END)    // source item is equipped
    {
        if (m_pItems[srcslot])   // dstitem goes into here.
        {
            // Bags aren't considered "visible".
            if (srcslot < EQUIPMENT_SLOT_END)
            {
                m_pOwner->setVisibleItemFields(static_cast<uint32_t>(srcslot), m_pItems[srcslot].get());
            }

            // handle bind on equip
            if (m_pItems[srcslot]->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
                m_pItems[srcslot]->addFlags(ITEM_FLAG_SOULBOUND);
        }
        else
        {
            // Bags aren't considered "visible".
            if (srcslot < EQUIPMENT_SLOT_END)
            {
                m_pOwner->setVisibleItemFields(static_cast<uint32_t>(srcslot), nullptr);
            }
        }
    }

    if (dstslot < INVENTORY_SLOT_BAG_END)     // source item is inside inventory
    {
        if (m_pItems[(int)dstslot] != nullptr)   // srcitem goes into here.
        {
            // Bags aren't considered "visible".
            if (dstslot < EQUIPMENT_SLOT_END)
            {
                m_pOwner->setVisibleItemFields(static_cast<uint32_t>(dstslot), m_pItems[dstslot].get());
            }

            // handle bind on equip
            if (m_pItems[(int)dstslot]->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
                m_pItems[(int)dstslot]->addFlags(ITEM_FLAG_SOULBOUND);

        }
        else
        {

            // bags aren't considered visible
            if (dstslot < EQUIPMENT_SLOT_END)
            {
                m_pOwner->setVisibleItemFields(static_cast<uint32_t>(dstslot), nullptr);
            }
        }
    }
    // handle dual wield
    if (dstslot == EQUIPMENT_SLOT_OFFHAND || srcslot == EQUIPMENT_SLOT_OFFHAND)
    {
        if (m_pItems[EQUIPMENT_SLOT_OFFHAND] != nullptr && m_pItems[EQUIPMENT_SLOT_OFFHAND]->getItemProperties()->Class == ITEM_CLASS_WEAPON)
        {
            /////////////////////////////////////////// Titan's grip stuff ////////////////////////////////////////////////////////////
            uint32_t subclass = m_pItems[EQUIPMENT_SLOT_OFFHAND]->getItemProperties()->SubClass;
            if (subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_AXE || subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_MACE || subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD)
            {
                m_pOwner->castSpell(m_pOwner, 49152, true);
            }
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
    }

    // Update enchantment durations
    if (m_pItems[(int)srcslot] != nullptr)
        sendEnchantDurations(m_pItems[(int)srcslot].get());
    if (m_pItems[(int)dstslot] != nullptr)
        sendEnchantDurations(m_pItems[(int)dstslot].get());

    //src item is equipped now
    if (srcslot < INVENTORY_SLOT_BAG_END)
    {
        if (m_pItems[(int)srcslot] != nullptr)
            m_pOwner->applyItemMods(m_pItems[srcslot].get(), srcslot, true);
        else if (srcslot == EQUIPMENT_SLOT_MAINHAND || srcslot == EQUIPMENT_SLOT_OFFHAND)
            m_pOwner->calculateDamage();
    }

    //dst item is equipped now
    if (dstslot < INVENTORY_SLOT_BAG_END)
    {
        if (m_pItems[(int)dstslot] != nullptr)
            m_pOwner->applyItemMods(m_pItems[dstslot].get(), dstslot, true);
        else if (dstslot == EQUIPMENT_SLOT_MAINHAND || dstslot == EQUIPMENT_SLOT_OFFHAND)
            m_pOwner->calculateDamage();
    }

    //Recalculate Expertise (for Weapon specs)
    m_pOwner->calcExpertise();
}

/// Item Loading
void ItemInterface::mLoadItemsFromDatabase(QueryResult* result)
{
    int8_t containerslot, slot;

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            containerslot = fields[13].asInt8();
            slot = fields[14].asInt8();

            ItemProperties const* proto = sMySQLStore.getItemProperties(fields[2].asUint32());
            if (proto != nullptr)
            {
                std::unique_ptr<Item> item;
                if (proto->InventoryType == INVTYPE_BAG)
                {
                    item = std::make_unique<Container>(HIGHGUID_TYPE_CONTAINER, fields[1].asUint32());
                    static_cast<Container*>(item.get())->loadFromDB(fields);
                }
                else
                {
                    item = std::make_unique<Item>();
                    item->init(HIGHGUID_TYPE_ITEM, fields[1].asUint32());
                    item->loadFromDB(fields, m_pOwner, false);
                }

                // if we encounter an item that expired, we remove it from db
                if (item->getItemExpireTime() > 0 && UNIXTIME > item->getItemExpireTime())
                {
                    item->deleteFromDB();
                    continue;
                }

                auto* itemPtr = item.get();
                const auto [addResult, _] = SafeAddItem(std::move(item), containerslot, slot);
                if (addResult)
                    itemPtr->m_isDirty = false;
            }
        } while (result->NextRow());
    }
}

/// Item saving
void ItemInterface::mSaveItemsToDatabase(bool first, QueryBuffer* buf)
{
    // Make sure durations of temporary enchanted items are saved to db
    sendEnchantDurations();

    int16_t x;

    for (x = EQUIPMENT_SLOT_START; x < CURRENCYTOKEN_SLOT_END; ++x)
    {
        if (GetInventoryItem(x) != nullptr)
        {
            if (IsBagSlot(x) && GetInventoryItem(x)->isContainer())
            {
                static_cast<Container*>(GetInventoryItem(x))->saveToDB(static_cast<int8_t>(x), first, buf);
            }
            else
            {
                GetInventoryItem(x)->saveToDB(INVENTORY_SLOT_NOT_SET, static_cast<int8_t>(x), first, buf);
            }
        }
    }
}

std::tuple<AddItemResult, std::unique_ptr<Item>> ItemInterface::AddItemToFreeBankSlot(std::unique_ptr<Item> itemHolder)
{
    //special items first
    for (uint32_t i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getItemProperties()->BagFamily & itemHolder->getItemProperties()->BagFamily)
            {
                if (m_pItems[i]->isContainer())
                {
                    auto [result, returnedItem] = static_cast<Container*>(m_pItems[i].get())->addItemToFreeSlot(std::move(itemHolder), NULL);
                    if (result)
                        return { ADD_ITEM_RESULT_OK, nullptr };
                    else
                        itemHolder = std::move(returnedItem);
                }
            }
        }
    }

    for (int16_t i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i] == nullptr)
        {
            return SafeAddItem(std::move(itemHolder), INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for (uint32_t i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr && m_pItems[i]->getItemProperties()->BagFamily == 0 && m_pItems[i]->isContainer())   //special bags ignored
        {
            for (uint32_t j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(m_pItems[i].get())->getItem(static_cast<int16_t>(j));
                if (item2 == nullptr)
                {
                    return SafeAddItem(std::move(itemHolder), static_cast<int8_t>(i), static_cast<int16_t>(j));
                }
            }
        }
    }
    return { ADD_ITEM_RESULT_ERROR, std::move(itemHolder) };
}

int8_t ItemInterface::FindSpecialBag(Item* item)
{
    for (uint8_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getItemProperties()->BagFamily & item->getItemProperties()->BagFamily)
            {
                return static_cast<int8_t>(i);
            }
        }
    }
    return ITEM_NO_SLOT_AVAILABLE;
}

int8_t ItemInterface::FindFreeKeyringSlot()
{
    for (uint8_t i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        if (m_pItems[i] == nullptr)
        {
            return static_cast<int8_t>(i);
        }
    }
    return ITEM_NO_SLOT_AVAILABLE;
}

int16_t ItemInterface::FindFreeCurrencySlot()
{
    for (uint16_t i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (m_pItems[i] == nullptr)
        {
            return static_cast<int16_t>(i);
        }
    }
    return ITEM_NO_SLOT_AVAILABLE;
}

SlotResult ItemInterface::FindFreeInventorySlot(ItemProperties const* proto)
{
    //special item
    //special slots will be ignored of item is not set
    if (proto != nullptr)
    {
        //sLogger.debug("ItemInterface::FindFreeInventorySlot called for item {}" , proto->Name1);
        if (proto->BagFamily)
        {
            if (proto->BagFamily & ITEM_TYPE_KEYRING || proto->Class == ITEM_CLASS_KEY)
            {
                for (uint32_t i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
                {
                    if (m_pItems[i] == nullptr)
                    {
                        m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
                        m_result.Slot = static_cast<int8_t>(i);
                        m_result.Result = true;
                        return m_result;
                    }
                }
            }
            else if (proto->BagFamily & ITEM_TYPE_CURRENCY)
            {
                for (uint32_t i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
                {
                    if (m_pItems[i] == nullptr)
                    {
                        m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
                        m_result.Slot = static_cast<int8_t>(i);
                        m_result.Result = true;
                        return m_result;
                    }
                }
            }
            else
            {
                for (uint32_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                {
                    if (m_pItems[i] != nullptr && m_pItems[i]->isContainer())
                    {
                        if (m_pItems[i]->getItemProperties()->BagFamily & proto->BagFamily)
                        {
                            int32_t slot = static_cast<Container*>(m_pItems[i].get())->findFreeSlot();
                            if (slot != ITEM_NO_SLOT_AVAILABLE)
                            {
                                m_result.ContainerSlot = static_cast<int8_t>(i);
                                m_result.Slot = static_cast<int8_t>(slot);
                                m_result.Result = true;
                                return m_result;
                            }
                        }
                    }
                }
            }
        }
    }

    //backpack
    for (uint32_t i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item == nullptr)
        {
            m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            m_result.Slot = static_cast<int8_t>(i);
            m_result.Result = true;
            return m_result;
        }
    }

    //bags
    for (uint32_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item != nullptr)
        {
            if (item->isContainer() && !item->getItemProperties()->BagFamily)
            {
                int32_t slot = static_cast<Container*>(m_pItems[i].get())->findFreeSlot();
                if (slot != ITEM_NO_SLOT_AVAILABLE)
                {
                    m_result.ContainerSlot = static_cast<int8_t>(i);
                    m_result.Slot = static_cast<int8_t>(slot);
                    m_result.Result = true;
                    return m_result;
                }
            }
        }
    }

    m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
    m_result.Slot = ITEM_NO_SLOT_AVAILABLE;
    m_result.Result = false;

    return m_result;
}

SlotResult ItemInterface::FindFreeBankSlot(ItemProperties const* proto)
{
    //special item
    //special slots will be ignored of item is not set
    if (proto != nullptr)
    {
        if (proto->BagFamily)
        {
            for (uint32_t i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
            {
                if (m_pItems[i] != nullptr && m_pItems[i]->isContainer())
                {
                    if (m_pItems[i]->getItemProperties()->BagFamily & proto->BagFamily)
                    {
                        int32_t slot = static_cast<Container*>(m_pItems[i].get())->findFreeSlot();
                        if (slot != ITEM_NO_SLOT_AVAILABLE)
                        {
                            m_result.ContainerSlot = static_cast<int8_t>(i);
                            m_result.Slot = static_cast<int8_t>(slot);
                            m_result.Result = true;
                            return m_result;
                        }
                    }
                }
            }
        }
    }

    //backpack
    for (uint32_t i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item == nullptr)
        {
            m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            m_result.Slot = static_cast<int8_t>(i);
            m_result.Result = true;
            return m_result;
        }
    }

    //bags
    for (uint32_t i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item != nullptr)
        {
            if (item->isContainer() && !item->getItemProperties()->BagFamily)
            {
                int32_t slot = static_cast<Container*>(m_pItems[i].get())->findFreeSlot();
                if (slot != ITEM_NO_SLOT_AVAILABLE)
                {
                    m_result.ContainerSlot = static_cast<int8_t>(i);
                    m_result.Slot = static_cast<int8_t>(slot);
                    m_result.Result = true;
                    return m_result;
                }
            }
        }
    }

    m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
    m_result.Slot = ITEM_NO_SLOT_AVAILABLE;
    m_result.Result = false;

    return m_result;
}

SlotResult ItemInterface::FindAmmoBag()
{
    for (uint32_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (m_pItems[i] != nullptr && m_pItems[i]->isAmmoBag())
        {
            m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            m_result.Slot = static_cast<int8_t>(i);
            m_result.Result = true;
            return m_result;
        }

    m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
    m_result.Slot = ITEM_NO_SLOT_AVAILABLE;
    m_result.Result = false;

    return m_result;
}

void ItemInterface::ReduceItemDurability()
{
    uint32_t f = Util::getRandomUInt(100);
    if (f <= 10)   //10% chance to loose 1 dur from a random valid item.
    {
        int32_t slot = static_cast<int32_t>(Util::getRandomUInt(EQUIPMENT_SLOT_END));
        Item* pItem = GetInventoryItem(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(slot));
        if (pItem != nullptr)
        {
            if (pItem->getDurability() && pItem->getMaxDurability())
            {
                pItem->setDurability(pItem->getMaxDurability() - 1);
                pItem->m_isDirty = true;
                //check final durability
                if (!pItem->getDurability())   //no dur left
                {
                    m_pOwner->applyItemMods(pItem, static_cast<int16_t>(slot), false, true);

                }
            }
        }
    }
}

bool ItemInterface::IsEquipped(uint32_t itemid)
{
    for (uint32_t x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        Item* it = m_pItems[x].get();

        if (it != nullptr)
        {
            if (it->getItemProperties()->ItemId == itemid)
                return true;

#if VERSION_STRING > Classic
            // check gems as well
            for (uint8_t count = 0; count < it->getSocketSlotCount(); count++)
            {
                const auto ei = it->getEnchantment(static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT1 + count));

                if (ei && ei->Enchantment)
                {
                    ItemProperties const* ip = sMySQLStore.getItemProperties(ei->Enchantment->GemEntry);
                    if (ip && ip->ItemId == itemid)
                        return true;
                }
            }
#endif
        }
    }
    return false;
}

void ItemInterface::CheckAreaItems()
{
    for (uint32_t x = EQUIPMENT_SLOT_START; x < INVENTORY_SLOT_ITEM_END; ++x)
    {
        if (m_pItems[x] != nullptr)
        {
            if (IsBagSlot(static_cast<int16_t>(x)) && m_pItems[x]->isContainer())
            {
                Container* bag = static_cast<Container*>(m_pItems[x].get());

                for (uint32_t i = 0; i < bag->getItemProperties()->ContainerSlots; ++i)
                {
                    if (bag->getItem(static_cast<int16_t>(i)) != nullptr && bag->getItem(static_cast<int16_t>(i))->getItemProperties()->MapID && bag->getItem(static_cast<int16_t>(i))->getItemProperties()->MapID != GetOwner()->GetMapId())
                        bag->safeFullRemoveItemFromSlot(static_cast<int16_t>(i));
                }
            }
            else
            {
                if (m_pItems[x]->getItemProperties()->MapID && m_pItems[x]->getItemProperties()->MapID != GetOwner()->GetMapId())
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16_t>(x));
            }
        }
    }
}

uint32_t ItemInterface::GetEquippedCountByItemLimit(uint32_t LimitId)
{
    uint32_t count = 0;
#if VERSION_STRING > Classic
    for (uint32_t x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        Item* it = m_pItems[x].get();

        if (it != nullptr)
        {
            for (uint8_t socketcount = 0; socketcount < it->getSocketSlotCount(); socketcount++)
            {
                const auto ei = it->getEnchantment(static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT1 + count));

                if (ei && ei->Enchantment)
                {
                    ItemProperties const* ip = sMySQLStore.getItemProperties(ei->Enchantment->GemEntry);
                    if (ip && ip->ItemLimitCategory == LimitId)
                        count++;
                }
            }
        }
    }
#endif
    return count;
}

uint32_t ItemInterface::GetItemCountByLimitId(uint32_t LimitId, bool IncBank)
{
    uint32_t cnt = 0;

    for (uint8_t i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item != nullptr)
        {
            if (item->getItemProperties()->ItemLimitCategory == LimitId
                && item->m_wrappedItemId == 0)
            {
                cnt += item->getStackCount() ? item->getStackCount() : 1;
            }
        }
    }

    for (uint8_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item && item->isContainer())
        {
            for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = (static_cast<Container*>(item))->getItem(static_cast<int16_t>(j));
                if (item2 != nullptr)
                {
                    if (item2->getItemProperties()->ItemLimitCategory == LimitId
                        && item2->m_wrappedItemId == 0)
                    {
                        cnt += item2->getStackCount() ? item2->getStackCount() : 1;
                    }
                }
            }
        }
    }

    for (uint8_t i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item != nullptr)
        {
            if (item->getItemProperties()->ItemLimitCategory == LimitId
                && item->m_wrappedItemId == 0)
            {
                cnt += item->getStackCount() ? item->getStackCount() : 1;
            }
        }
    }

    for (uint8_t i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16_t>(i));
        if (item != nullptr)
        {
            if (item->getItemProperties()->ItemLimitCategory == LimitId
                && item->m_wrappedItemId == 0)
            {
                cnt += item->getStackCount() ? item->getStackCount() : 1;
            }
        }
    }

    if (IncBank)
    {
        for (uint8_t i = BANK_SLOT_ITEM_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16_t>(i));
            if (item != nullptr)
            {
                if (item->getItemProperties()->ItemLimitCategory == LimitId
                    && item->m_wrappedItemId == 0)
                {
                    cnt += item->getStackCount() ? item->getStackCount() : 1;
                }
            }
        }

        for (uint8_t i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16_t>(i));
            if (item != nullptr)
            {
                if (item->isContainer())
                {
                    for (uint32_t j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                    {
                        Item* item2 = (static_cast<Container*>(item))->getItem(static_cast<int16_t>(j));
                        if (item2 != nullptr)
                        {
                            if (item2->getItemProperties()->ItemLimitCategory == LimitId
                                && item2->m_wrappedItemId == 0)
                            {
                                cnt += item2->getStackCount() ? item2->getStackCount() : 1;
                            }
                        }
                    }
                }
            }
        }
    }
    cnt += GetEquippedCountByItemLimit(LimitId);

    return cnt;
}

/// Look for items with limited duration and send the remaining time to the client
void ItemInterface::HandleItemDurations()
{

    for (uint16_t i = EQUIPMENT_SLOT_START; i <= CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item1 = this->GetInventoryItem(static_cast<int16_t>(i));
        Item* realitem = nullptr;

        if (item1 != nullptr && item1->isContainer())
        {

            for (uint32_t j = 0; j < item1->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = dynamic_cast<Container*>(item1)->getItem(static_cast<int16_t>(j));

                if (item2 != nullptr && item2->getItemProperties()->ExistingDuration > 0)
                    realitem = item2;
            }

        }
        else
        {
            if (item1 != nullptr)
                realitem = item1;
        }

        if (realitem != nullptr)
            sEventMgr.AddEvent(realitem, &Item::sendDurationUpdate, EVENT_SEND_PACKET_TO_PLAYER_AFTER_LOGIN, 0, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

/// Inserts a new entry into the RefundableMap. This should be called when purchasing the item
void ItemInterface::AddRefundable(uint64_t GUID, uint32_t extendedcost)
{
    std::pair< time_t, uint32_t > RefundableEntry;
    std::pair< uint64_t, std::pair< time_t, uint32_t > > insertpair;

    Item* item = this->GetItemByGUID(GUID);
    if (item == nullptr)
        return;

    uint32_t* played = this->GetOwner()->getPlayedTime();

    RefundableEntry.first = played[1];               // time of purchase in playedtime
    RefundableEntry.second = extendedcost;          // extendedcost

    insertpair.first = GUID;
    insertpair.second = RefundableEntry;

    this->m_refundableitems.insert(insertpair);

    sEventMgr.AddEvent(item, &Item::removeFromRefundableMap, EVENT_REMOVE_ITEM_FROM_REFUNDABLE_MAP, (UNIXTIME + 60 * 60 * 2), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void ItemInterface::AddRefundable(uint64_t GUID, uint32_t extendedcost, time_t buytime)
{
    std::pair< time_t, uint32_t > RefundableEntry;
    std::pair< uint64_t, std::pair< time_t, uint32_t > > insertpair;

    Item* item = this->GetItemByGUID(GUID);
    if (item == nullptr)
        return;

    RefundableEntry.first = buytime;               // time of purchase in playedtime
    RefundableEntry.second = extendedcost;      // extendedcost

    insertpair.first = GUID;
    insertpair.second = RefundableEntry;

    this->m_refundableitems.insert(insertpair);

    sEventMgr.AddEvent(item, &Item::removeFromRefundableMap, EVENT_REMOVE_ITEM_FROM_REFUNDABLE_MAP, buytime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void ItemInterface::AddRefundable(Item* item, uint32_t extendedcost, time_t buytime)
{
    std::pair< time_t, uint32_t > RefundableEntry;
    std::pair< uint64_t, std::pair< time_t, uint32_t > > insertpair;

    if (item == nullptr)
        return;

    RefundableEntry.first = buytime;      // time of purchase in playedtime
    RefundableEntry.second = extendedcost; // extendedcost

    insertpair.first = item->getGuid();
    insertpair.second = RefundableEntry;

    this->m_refundableitems.insert(insertpair);

    sEventMgr.AddEvent(item, &Item::removeFromRefundableMap, EVENT_REMOVE_ITEM_FROM_REFUNDABLE_MAP, (buytime + 60 * 60 * 2), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

/// Removes an entry from the RefundableMap
void ItemInterface::RemoveRefundable(uint64_t GUID)
{
    this->m_refundableitems.erase(GUID);
}

/// Looks up an item in the RefundableMap, and returns the data
std::pair<time_t, uint32_t> ItemInterface::LookupRefundable(uint64_t GUID)
{
    std::pair<time_t, uint32_t> RefundableEntry;
    RefundableEntry.first = 0;          // time of purchase in playedtime
    RefundableEntry.second = 0;         // extendedcost

    RefundableMap::iterator itr = this->m_refundableitems.find(GUID);
    if (itr != this->m_refundableitems.end())
    {
        RefundableEntry.first = itr->second.first;
        RefundableEntry.second = itr->second.second;
    }

    return RefundableEntry;
}

bool ItemInterface::AddItemById(uint32_t itemid, uint32_t count, int32_t randomprop)
{
    if (count == 0)
        return false;

    Player* chr = GetOwner();

    if (!chr)
        return false;

    ItemProperties const* it = sMySQLStore.getItemProperties(itemid);
    if (it == nullptr)
        return false;

    int8_t error = CanReceiveItem(it, count);
    if (error != 0)
    {
        return false;
    }

    uint32_t maxStack = chr->m_cheats.hasItemStackCheat ? 0x7fffffff : it->MaxCount;
    uint32_t toadd;
    bool freeslots = true;

    while (count > 0 && freeslots)
    {
        if (count < maxStack)
        {
            // find existing item with free stack
            Item* free_stack_item = FindItemLessMax(itemid, count, false);
            if (free_stack_item != nullptr)
            {
                // increase stack by new amount
                free_stack_item->modStackCount(static_cast<int32_t>(count));
                free_stack_item->m_isDirty = true;

                sQuestMgr.OnPlayerItemPickup(m_pOwner, free_stack_item);

                return true;
            }
        }

        // create new item
        auto itemHolder = sObjectMgr.createItem(itemid, chr);
        if (itemHolder == nullptr)
            return false;

        if (it->Bonding == ITEM_BIND_ON_PICKUP)
        {
            if (it->Flags & ITEM_FLAG_ACCOUNTBOUND)   // don't "Soulbind" account-bound items
                itemHolder->addFlags(ITEM_FLAG_ACCOUNTBOUND);
            else
                itemHolder->addFlags(ITEM_FLAG_SOULBOUND);
        }

        // Let's try to autogenerate randomprop / randomsuffix
        if (randomprop == 0)
        {

            if ((it->RandomPropId != 0) && (it->RandomSuffixId != 0))
            {
                sLogger.failure("Item {} ({}) has both RandomPropId and RandomSuffixId.", itemid, it->Name);
            }

            if (it->RandomPropId != 0)
            {
                auto item_random_properties = sLootMgr.getRandomProperties(it);

                if (item_random_properties != nullptr)
                {
                    randomprop = static_cast<int32_t>(item_random_properties->ID);
                }
                else
                {
                    sLogger.failure("Item {} ({}) has unknown RandomPropId {}", itemid, it->Name, it->RandomPropId);
                }
            }

            if (it->RandomSuffixId != 0)
            {
                auto item_random_suffix = sLootMgr.getRandomSuffix(it);

                if (item_random_suffix != nullptr)
                {
                    randomprop = -1 * static_cast<int32_t>(item_random_suffix->id);
                }
                else
                {
                    sLogger.failure("Item {} ({}) has unknown RandomSuffixId {}", itemid, it->Name, it->RandomSuffixId);
                }
            }
        }

        if (randomprop != 0)
        {
            if (randomprop < 0)
                itemHolder->setRandomSuffix(static_cast<uint32_t>(-randomprop));
            else
                itemHolder->setRandomPropertiesId(static_cast<uint32_t>(randomprop));

            itemHolder->applyRandomProperties(false);
        }

        if (maxStack != 0)
        {
            toadd = count > maxStack ? maxStack : count;
        }
        else
        {
            toadd = count;
        }

        itemHolder->setStackCount(toadd);

        auto* item = itemHolder.get();
        const auto [res, _] = AddItemToFreeSlot(std::move(itemHolder));
        if (res == ADD_ITEM_RESULT_OK)
        {
            SlotResult* lr = LastSearchResult();

            chr->sendItemPushResultPacket(false, true, false, lr->ContainerSlot, lr->Slot, toadd, item->getEntry(), item->getPropertySeed(), item->getRandomPropertiesId(), item->getStackCount());
#if VERSION_STRING > TBC
            chr->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, itemid, 1, 0);
#endif
            sQuestMgr.OnPlayerItemPickup(m_pOwner, item);
            count -= toadd;
        }
        else
        {
            freeslots = false;
            chr->getSession()->SendNotification("No free slots were found in your inventory!");
        }
    }
    return freeslots;
}


bool ItemInterface::SwapItems(int8_t DstInvSlot, int8_t DstSlot, int8_t SrcInvSlot, int8_t SrcSlot)
{
    Item* SrcItem = nullptr;
    Item* DstItem = nullptr;
    bool adderror = false;
    int8_t error;

    if (DstInvSlot == SrcSlot && SrcInvSlot == -1)   // player trying to add self container to self container slots
    {
        buildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEMS_CANT_BE_SWAPPED);
        return false;
    }

    if ((DstInvSlot <= 0 && DstSlot < 0) || DstInvSlot < -1)
        return false;

    if ((SrcInvSlot <= 0 && SrcSlot < 0) || SrcInvSlot < -1)
        return false;

    SrcItem = GetInventoryItem(SrcInvSlot, SrcSlot);
    if (!SrcItem)
        return false;

    DstItem = GetInventoryItem(DstInvSlot, DstSlot);

    if (DstItem)
    {
        //check if it will go to equipment slot
        if (SrcInvSlot == INVENTORY_SLOT_NOT_SET)  //not bag
        {
            if (DstItem->isContainer())
            {
                if (static_cast<Container*>(DstItem)->hasItems())
                {
                    if (!IsBagSlot(SrcSlot))
                    {
                        buildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                        return false;
                    }
                }
            }

            if (SrcSlot < INVENTORY_KEYRING_END)
            {
                if ((error = CanEquipItemInSlot2(SrcInvSlot, SrcSlot, DstItem)) != 0)
                {
                    buildInventoryChangeError(SrcItem, DstItem, static_cast<uint8_t>(error));
                    return false;
                }
            }
        }
        else
        {
            if (DstItem->isContainer())
            {
                if (static_cast<Container*>(DstItem)->hasItems())
                {
                    buildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return false;
                }
            }

            if ((error = CanEquipItemInSlot2(SrcInvSlot, SrcInvSlot, DstItem)) != 0)
            {
                buildInventoryChangeError(SrcItem, DstItem, static_cast<uint8_t>(error));
                return false;
            }
        }
    }

    if (DstInvSlot == INVENTORY_SLOT_NOT_SET) //not bag
    {
        if (SrcItem->isContainer())
        {
            if (static_cast<Container*>(SrcItem)->hasItems())
            {
                if (!IsBagSlot(DstSlot))
                {
                    buildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return false;
                }
            }
        }

        if (DstSlot < INVENTORY_KEYRING_END)
        {
            if ((error = CanEquipItemInSlot2(DstInvSlot, DstSlot, SrcItem)) != 0)
            {
                buildInventoryChangeError(SrcItem, DstItem, static_cast<uint8_t>(error));
                return false;
            }
        }
    }
    else
    {
        if (SrcItem->isContainer())
        {
            if (static_cast<Container*>(SrcItem)->hasItems())
            {
                buildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                return false;
            }
        }

        if ((error = CanEquipItemInSlot2(DstInvSlot, DstInvSlot, SrcItem)) != 0)
        {
            buildInventoryChangeError(SrcItem, DstItem, static_cast<uint8_t>(error));
            return false;
        }
    }

    if (DstSlot < INVENTORY_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET)   //equip - bags can be soulbound too
    {
        if (SrcItem->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
            SrcItem->addFlags(ITEM_FLAG_SOULBOUND);

#if VERSION_STRING > TBC
        m_pOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, SrcItem->getItemProperties()->ItemId, 0, 0);

        if (DstSlot < INVENTORY_SLOT_BAG_START) // check Superior/Epic achievement
        {
            // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
            // "213" value not found in achievement or criteria entries, have to hard-code it here? :(
            // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
            // "187" value not found in achievement or criteria entries, have to hard-code it here? :(
            if ((SrcItem->getItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && SrcItem->getItemProperties()->ItemLevel >= 187) ||
                (SrcItem->getItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && SrcItem->getItemProperties()->ItemLevel >= 213))
                m_pOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, DstSlot, SrcItem->getItemProperties()->Quality, 0);
        }
#endif
    }

    if (DstItem && SrcSlot < INVENTORY_SLOT_BAG_END && SrcInvSlot == INVENTORY_SLOT_NOT_SET)   //equip - make sure to soulbind items swapped from equip slot to bag slot
    {
        if (DstItem->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
            DstItem->addFlags(ITEM_FLAG_SOULBOUND);
#if VERSION_STRING > TBC
        m_pOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, DstItem->getItemProperties()->ItemId, 0, 0);
        if (SrcSlot < INVENTORY_SLOT_BAG_START) // check Superior/Epic achievement
        {
            if ((DstItem->getItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && DstItem->getItemProperties()->ItemLevel >= 187) ||
                (DstItem->getItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && DstItem->getItemProperties()->ItemLevel >= 213))
                m_pOwner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, SrcSlot, DstItem->getItemProperties()->Quality, 0);
        }
#endif
    }

    if (SrcInvSlot == DstInvSlot)  //in 1 bag
    {
        if (SrcInvSlot == INVENTORY_SLOT_NOT_SET)   //in backpack
        {
            SwapItemSlots(SrcSlot, DstSlot);
        }
        else//in bag
        {
            static_cast<Container*>(GetInventoryItem(SrcInvSlot))->swapItems(SrcSlot, DstSlot);
        }
    }
    else
    {
        //Check for stacking
        uint32_t srcItemMaxStack = (SrcItem->getOwner()->m_cheats.hasItemStackCheat) ? 0x7fffffff : SrcItem->getItemProperties()->MaxCount;
        uint32_t dstItemMaxStack = (DstItem) ? ((DstItem->getOwner()->m_cheats.hasItemStackCheat) ? 0x7fffffff : DstItem->getItemProperties()->MaxCount) : 0;
        if (DstItem && SrcItem->getEntry() == DstItem->getEntry() && srcItemMaxStack > 1 && SrcItem->m_wrappedItemId == 0 && DstItem->m_wrappedItemId == 0)
        {
            uint32_t total = SrcItem->getStackCount() + DstItem->getStackCount();
            if (total <= dstItemMaxStack)
            {
                DstItem->modStackCount(SrcItem->getStackCount());
                DstItem->m_isDirty = true;
                bool result = SafeFullRemoveItemFromSlot(SrcInvSlot, SrcSlot);
                if (!result)
                {
                    buildInventoryChangeError(SrcItem, DstItem, INV_ERR_ITEM_CANT_STACK);
                }
                return false;
            }
            else
            {
                if (DstItem->getStackCount() == dstItemMaxStack)
                {

                }
                else
                {
                    int32_t delta = dstItemMaxStack - DstItem->getStackCount();
                    DstItem->setStackCount(dstItemMaxStack);
                    SrcItem->modStackCount(-delta);
                    SrcItem->m_isDirty = true;
                    DstItem->m_isDirty = true;
                    return false;
                }
            }
        }

        auto srcItemHolder = SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot, SrcSlot, false);
        auto dstItemHolder = DstItem != nullptr ? SafeRemoveAndRetreiveItemFromSlot(DstInvSlot, DstSlot, false) : nullptr;

        if (srcItemHolder)
        {
            const auto [result, returnedItem] = SafeAddItem(std::move(srcItemHolder), DstInvSlot, DstSlot);
            if (!result)
            {
                // TODO: if add failed, should item be sent in mail to player? now it's destroyed -Appled
                sLogger.failure("HandleSwapItem: Error while adding item to dstslot");
                returnedItem->deleteFromDB();
                adderror = true;
            }
        }

        if (dstItemHolder)
        {
            const auto [result, returnedItem] = SafeAddItem(std::move(dstItemHolder), SrcInvSlot, SrcSlot);
            if (!result)
            {
                // TODO: if add failed, should item be sent in mail to player? now it's destroyed -Appled
                sLogger.failure("HandleSwapItem: Error while adding item to srcslot");
                returnedItem->deleteFromDB();
                adderror = true;
            }
        }
    }

    //Recalculate Expertise (for Weapon specs)
    m_pOwner->calcExpertise();

    if (adderror)
        return false;
    else
        return true;
}

void ItemInterface::removeLootableItems()
{
    for (uint8_t i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item == nullptr)
            continue;

        if (item->m_loot != nullptr)
            SafeFullRemoveItemFromSlot(-1, i);
    }

    for (uint8_t i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item == nullptr)
            continue;

        if (item->m_loot != nullptr)
            SafeFullRemoveItemFromSlot(-1, i);
    }

    for (uint8_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Container* container = dynamic_cast<Container*>(GetInventoryItem(i));
        if (container == nullptr)
            continue;

        uint8_t s = static_cast<uint8_t>(container->getSlotCount());
        for (uint8_t j = 0; j < s; j++)
        {
            Item* item = container->getItem(j);
            if (item == nullptr)
                continue;

            if (item->m_loot != nullptr)
                container->safeFullRemoveItemFromSlot(j);
        }
    }

    for (uint8_t i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        Container* container = dynamic_cast<Container*>(GetInventoryItem(i));
        if (container == nullptr)
            continue;

        uint8_t s = static_cast<uint8_t>(container->getSlotCount());
        for (uint8_t j = 0; j < s; ++j)
        {
            Item* item = container->getItem(j);
            if (item == nullptr)
                continue;

            if (item->m_loot != nullptr)
                container->safeFullRemoveItemFromSlot(j);
        }
    }
}

void ItemIterator::Increment()
{
    if (!m_searchInProgress)
        BeginSearch();

    /// check: are we currently inside a container?
    if (m_container != nullptr)
    {
        /// loop the container.
        for (; m_containerSlot < m_container->getItemProperties()->ContainerSlots; ++m_containerSlot)
        {
            m_currentItem = m_container->getItem(static_cast<int16_t>(m_containerSlot));
            if (m_currentItem != nullptr)
            {
                ++m_containerSlot;      /// increment the counter so we don't get the same item again

                return;
            }
        }

        m_container = nullptr;             /// unset this
    }

    for (; m_slot < MAX_INVENTORY_SLOT; ++m_slot)
    {
        if (m_target->m_pItems[m_slot])
        {
            if (m_target->m_pItems[m_slot]->isContainer())
            {
                m_container = static_cast<Container*>(m_target->m_pItems[m_slot].get());       /// we are a container :O lets look inside the box!
                m_containerSlot = 0;
                m_currentItem = nullptr;        /// clear the pointer up. so we can tell if we found an item or not
                ++m_slot;                       /// increment m_slot so we don't search this container again

                Increment();                    /// call increment() recursively. this will search the container.

                return;                         /// jump out so we're not wasting cycles and skipping items
            }


            m_currentItem = m_target->m_pItems[m_slot].get();   /// we're not a container, just a regular item. Set the pointer
            ++m_slot;                                           /// increment the slot counter so we don't do the same item again

            return;             /// jump out
        }
    }

    /// if we're here we've searched all items.
    m_atEnd = true;
    m_currentItem = nullptr;
}
