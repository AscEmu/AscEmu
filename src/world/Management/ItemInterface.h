/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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
#include "ItemPrototype.h"
#include "Common.hpp"

class Creature;

const uint8_t INVALID_BACKPACK_SLOT = 0xFF;

struct SlotResult
{
    SlotResult() { ContainerSlot = -1, Slot = -1, Result = false; }
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

// sanity checking
enum AddItemResult
{
    ADD_ITEM_RESULT_ERROR           = 0,
    ADD_ITEM_RESULT_OK              = 1,
    ADD_ITEM_RESULT_DUPLICATED      = 2
};

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

class SERVER_DECL ItemInterface
{
    private:

        SlotResult m_result;
        Player* m_pOwner;
        Item* m_pItems[MAX_INVENTORY_SLOT];
        Item* m_pBuyBack[MAX_BUYBACK_SLOT];

        RefundableMap m_refundableitems;

        AddItemResult m_AddItem(Item* item, int8_t ContainerSlot, int16_t slot);

    public:
        // APGL End
        // MIT Start

        bool hasItemForTotemCategory(uint32_t totemCategory);
        bool isItemInTradeWindow(Item const* item) const;

        // Inventory error report
        void buildInventoryChangeError(Item const* srcItem, Item const* dstItem, uint8_t inventoryError, uint32_t srcItemId = 0);

        // MIT End
        // APGL Start

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
        AddItemResult SafeAddItem(Item* pItem, int8_t ContainerSlot, int16_t slot);
        Item* SafeRemoveAndRetreiveItemFromSlot(int8_t ContainerSlot, int16_t slot, bool destroy);  // doesn't destroy item from memory
        Item* SafeRemoveAndRetreiveItemByGuid(uint64_t guid, bool destroy);
        bool SafeFullRemoveItemFromSlot(int8_t ContainerSlot, int16_t slot);                        // destroys item fully
        bool SafeFullRemoveItemByGuid(uint64_t guid);                                               // destroys item fully
        AddItemResult AddItemToFreeSlot(Item* item);
        AddItemResult AddItemToFreeBankSlot(Item* item);

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

        uint8_t GetInternalBankSlotFromPlayer(int8_t islot); // converts inventory slots into 0-x numbers

        // buyback stuff
        inline Item* GetBuyBack(int32_t slot)
        {
            if (slot >= 0 && slot < MAX_BUYBACK_SLOT)
                return m_pBuyBack[slot];
            else
                return nullptr;
        }
        void AddBuyBackItem(Item* it, uint32_t price);
        void RemoveBuyBackItem(uint32_t index);
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
            ARCEMU_ASSERT(!m_searchInProgress);
            m_atEnd = false;
            m_searchInProgress = true;
            m_container = nullptr;
            m_currentItem = nullptr;
            m_slot = 0;
            Increment();
        }

        void EndSearch()
        {
            // nothing here either
            ARCEMU_ASSERT(m_searchInProgress);
            m_atEnd = true;
            m_searchInProgress = false;
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
