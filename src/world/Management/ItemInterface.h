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

#pragma once

#include "EquipmentSetMgr.h"
#include "Objects/ItemDefines.hpp"
#include "CommonTypes.hpp"
#include "Macros/ItemMacros.hpp"

#include <cstdint>
#include <list>
#include <mutex>
#include <vector>

class Creature;
struct ItemProperties;

struct VoidStorageItem
{
    VoidStorageItem()
    {
        itemId = 0;
        itemEntry = 0;
        creatorGuid = 0;
        itemRandomPropertyId = 0;
        itemSuffixFactor = 0;
    }

    VoidStorageItem(uint64_t id, uint32_t entry, uint32_t creator, uint32_t randomPropertyId, uint32_t suffixFactor)
    {
        itemId = id;
        itemEntry = entry;
        creatorGuid = creator;
        itemRandomPropertyId = randomPropertyId;
        itemSuffixFactor = suffixFactor;
    }

    uint64_t itemId;
    uint32_t itemEntry;
    uint32_t creatorGuid;
    uint32_t itemRandomPropertyId;
    uint32_t itemSuffixFactor;
};


struct SlotResult
{
    SlotResult() { ContainerSlot = -1; Slot = -1; Result = false; }
    int8_t ContainerSlot;
    int8_t Slot;
    bool Result;
};

class Item;
class Container;
class Player;
class UpdateData;
class ByteBuffer;
class EquipmentSetMgr;


//////////////////////////////////////////////////////////////////////////////////////////
// RefundableMap
// Contains refundable item data.
//
//Key:     uint64_t GUID     - GUID of the item
//
// \param time_t buytime    - time of purchase in Unixtime
// \param uint32_t costid     - extendedcostID of the cost
//
//////////////////////////////////////////////////////////////////////////////////////////
typedef std::map<uint64_t, std::pair<time_t, uint32_t>> RefundableMap;

// APGL End
// MIT Start
struct ItemEnchantmentDuration
{
    Item* item = nullptr;
    EnchantmentSlot slot = TEMP_ENCHANTMENT_SLOT;
    uint32_t timeLeft = 0;
};

typedef std::list<Item*> ItemList;
typedef std::vector<ItemEnchantmentDuration> ItemEnchantmentDurationList;

class SERVER_DECL ItemInterface
{
public:
    bool hasItemForTotemCategory(uint32_t totemCategory);
    bool isItemInTradeWindow(Item const* item) const;

    void addTemporaryEnchantedItem(Item* item, EnchantmentSlot slot);
    void removeTemporaryEnchantedItem(Item* item);
    void removeTemporaryEnchantedItem(Item* item, EnchantmentSlot slot);
    void sendEnchantDurations(Item const* forItem = nullptr);
    void updateEnchantDurations(uint32_t timePassed);

#if VERSION_STRING >= WotLK
    // Soulbound Tradeable
    void updateSoulboundTradeItems();
    void addTradeableItem(Item* item);
    void removeTradeableItem(Item* item);
#endif

    // Inventory error report
    void buildInventoryChangeError(Item const* srcItem, Item const* dstItem, uint8_t inventoryError, uint32_t srcItemId = 0);

    void setOwnerInventoryItem(uint8_t slot, uint64_t guid);

    void update(uint32_t timePassed);

private:
    ItemEnchantmentDurationList m_temporaryEnchantmentList;
    std::mutex m_temporaryEnchantmentMutex;

#if VERSION_STRING >= WotLK
    ItemList m_soulboundTradeableList;
    std::mutex m_soulboundTradeableMutex;
#endif

    // MIT End
    // APGL Start

        SlotResult m_result;
        Player* m_pOwner;
        std::array<std::unique_ptr<Item>, MAX_INVENTORY_SLOT> m_pItems;
        std::array<std::unique_ptr<Item>, MAX_BUYBACK_SLOT> m_pBuyBack;

        RefundableMap m_refundableitems;

        // Returns item in tuple with result if failed to add item, nullptr on success
        std::tuple<AddItemResult, std::unique_ptr<Item>> m_AddItem(std::unique_ptr<Item> itemHolder, int8_t ContainerSlot, int16_t slot);

    public:
        Arcemu::EquipmentSetMgr m_EquipmentSets;
        friend class ItemIterator;
        ItemInterface(Player* pPlayer);
        ~ItemInterface();

        Player* GetOwner() { return m_pOwner; }
        bool IsBagSlot(int16_t slot);

        uint32_t m_CreateForPlayer(ByteBuffer* data);
        void m_DestroyForPlayer();

        void mLoadItemsFromDatabase(QueryResult* result);
        void mSaveItemsToDatabase(bool first, QueryBuffer* buf);

        Item* GetInventoryItem(int16_t slot);
        Item* GetInventoryItem(int8_t ContainerSlot, int16_t slot);
        Container* GetContainer(int8_t containerSlot);
        int16_t GetInventorySlotById(uint32_t ID);
        int16_t GetInventorySlotByGuid(uint64_t guid);
        int16_t GetBagSlotByGuid(uint64_t guid);

        Item* SafeAddItem(uint32_t ItemId, int8_t ContainerSlot, int16_t slot);
        // Returns item in tuple with result if failed to add item, nullptr on success
        std::tuple<AddItemResult, std::unique_ptr<Item>> SafeAddItem(std::unique_ptr<Item> pItem, int8_t ContainerSlot, int16_t slot);
        std::unique_ptr<Item> SafeRemoveAndRetreiveItemFromSlot(int8_t ContainerSlot, int16_t slot, bool destroy);  // doesn't destroy item from memory
        std::unique_ptr<Item> SafeRemoveAndRetreiveItemByGuid(uint64_t guid, bool destroy);
        bool SafeFullRemoveItemFromSlot(int8_t ContainerSlot, int16_t slot);                        // destroys item fully
        bool SafeFullRemoveItemByGuid(uint64_t guid);                                             // destroys item fully
        // Returns item in tuple with result if failed to add item, nullptr on success
        std::tuple<AddItemResult, std::unique_ptr<Item>> AddItemToFreeSlot(std::unique_ptr<Item> item);
        // Returns item in tuple with result if failed to add item, nullptr on success
        std::tuple<AddItemResult, std::unique_ptr<Item>> AddItemToFreeBankSlot(std::unique_ptr<Item> itemHolder);

        Item* FindItemLessMax(uint32_t itemid, uint32_t cnt, bool IncBank);
        uint32_t GetItemCount(uint32_t itemid, bool IncBank = false);
        uint32_t RemoveItemAmt(uint32_t id, uint32_t amt);
        uint32_t RemoveItemAmt_ProtectPointer(uint32_t id, uint32_t amt, Item** pointer);
        uint32_t RemoveItemAmtByGuid(uint64_t guid, uint32_t amt);
        void RemoveAllConjured();
        void BuyItem(ItemProperties const* item, uint32_t total_amount, Creature* pVendor);

        uint32_t CalculateFreeSlots(ItemProperties const* proto);
        void ReduceItemDurability();

        uint8_t LastSearchItemBagSlot() { return m_result.ContainerSlot; }
        uint8_t LastSearchItemSlot() { return m_result.Slot; }
        SlotResult* LastSearchResult() { return &m_result; }

        //Searching functions
        SlotResult FindFreeInventorySlot(ItemProperties const* proto);
        SlotResult FindFreeBankSlot(ItemProperties const* proto);
        SlotResult FindAmmoBag();
        int8_t FindFreeBackPackSlot();
        uint8_t FindFreeBackPackSlotMax();
        int8_t FindFreeKeyringSlot();
        int16_t FindFreeCurrencySlot();
        int8_t FindSpecialBag(Item* item);


        int8_t CanEquipItemInSlot(int8_t DstInvSlot, int8_t slot, ItemProperties const* item, bool ignore_combat = false, bool skip_2h_check = false);
        int8_t CanEquipItemInSlot2(int8_t DstInvSlot, int8_t slot, Item* item, bool ignore_combat = false, bool skip_2h_check = false);
        int8_t CanReceiveItem(ItemProperties const* item, uint32_t amount);
        int8_t CanAffordItem(ItemProperties const* item, uint32_t amount, Creature* pVendor);
        int8_t GetItemSlotByType(uint32_t type);
        Item* GetItemByGUID(uint64_t itemGuid);

        void SwapItemSlots(int8_t srcslot, int8_t dstslot);

        uint8_t GetInternalBankSlotFromPlayer(int8_t islot);         // converts inventory slots into 0-x numbers

        // buyback stuff
        inline Item* GetBuyBack(int32_t slot)
        {
            if (slot >= 0 && slot < MAX_BUYBACK_SLOT)
                return m_pBuyBack[slot].get();
            else
                return nullptr;
        }
        void AddBuyBackItem(std::unique_ptr<Item> it, uint32_t price);
        std::unique_ptr<Item> RemoveBuyBackItem(uint8_t index);
        void EmptyBuyBack();
        bool IsEquipped(uint32_t itemid);

        void CheckAreaItems();

        uint32_t GetItemCountByLimitId(uint32_t LimitId, bool IncBank);
        uint32_t GetEquippedCountByItemLimit(uint32_t LimitId);

        void HandleItemDurations();

        // Refundable item stuff start
        void AddRefundable(uint64_t GUID, uint32_t extendedcost);
        void AddRefundable(uint64_t GUID, uint32_t extendedcost, time_t buytime);
        void AddRefundable(Item* item, uint32_t extendedcost, time_t buytime);
        void RemoveRefundable(uint64_t GUID);
        std::pair<time_t, uint32_t> LookupRefundable(uint64_t GUID);
        // Refundable item stuff end

    public:
        //////////////////////////////////////////////////////////////////////////////////////////
        // bool AddItemById(uint32_t itemid, uint32_t count, int32_t randomprop)
        // Adds item(s) to a Player
        //
        // \param uint32_t itemid     -  ID of the item(s) to add
        // \param uint32_t count      -  Amount of items to add
        // \param int32_t randomprop  -  Random prop or suffix id for the items to add.
        //
        // \note If positive it's prop, if negative it's suffix, if 0 it's autogenerated.
        //
        // \return true on success, false on failure.
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        bool AddItemById(uint32_t itemid, uint32_t count, int32_t randomprop);


        //////////////////////////////////////////////////////////////////////////////////////////
        // void SwapItems(int8_t DstInvSlot, int8_t DstSlot, int8_t SrcInvSlot, int8_t SrcSlot)
        // Exchanges items A and B
        //
        // \param int8_t DstInvSlot  -  Item A's bag inventory slot (-1 if it's an equipment slot)
        // \param int8_t DstSlot     -  Item A's slotid within that bag
        // \param int8_t SrcInvSlot  -  Item B's bag inventory slot (-1 if it's an equipment slot)
        // \param int8_t SrcSlot     -  Item B's slotid within that bag
        //
        // \return true on success, false on failure.
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        bool SwapItems(int8_t DstInvSlot, int8_t DstSlot, int8_t SrcInvSlot, int8_t SrcSlot);

        void removeLootableItems();
};

class ItemIterator
{
    bool m_atEnd;
    bool m_searchInProgress;
    uint32_t m_slot;
    uint32_t m_containerSlot;
    Container* m_container;
    Item* m_currentItem;
    ItemInterface* m_target;

    public:
        ItemIterator(ItemInterface* target) : m_atEnd(false), m_searchInProgress(false), m_slot(0), m_containerSlot(0), m_container(nullptr), m_currentItem(nullptr), m_target(target) {}
        ~ItemIterator() { if (m_searchInProgress) { EndSearch(); } }

        void BeginSearch()
        {
            // iteminterface doesn't use mutexes, maybe it should :P
            if (!m_searchInProgress)
            {
                m_atEnd = false;
                m_searchInProgress = true;
                m_container = nullptr;
                m_currentItem = nullptr;
                m_slot = 0;
                Increment();
            }
        }

        void EndSearch()
        {
            // nothing here either
            if (m_searchInProgress)
            {
                m_atEnd = true;
                m_searchInProgress = false;
            }
        }

        Item* operator*() const
        {
            return m_currentItem;
        }

        Item* operator->() const
        {
            return m_currentItem;
        }

        void Increment();

        inline Item* Grab() { return m_currentItem; }
        inline bool End() { return m_atEnd; }
};
