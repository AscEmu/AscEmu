/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
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
#include "ItemProperties.hpp"
#include "Objects/ItemDefines.hpp"
#include "CommonTypes.hpp"

#include <cstdint>
#include <list>
#include <mutex>

class Creature;

const uint8 INVALID_BACKPACK_SLOT = 0xFF;

#define VOID_STORAGE_UNLOCK       100*GOLD
#define VOID_STORAGE_STORE_ITEM   25*GOLD
#define VOID_STORAGE_MAX_DEPOSIT  9
#define VOID_STORAGE_MAX_WITHDRAW 9
#define VOID_STORAGE_MAX_SLOT     80

enum VoidTransferError
{
    VOID_TRANSFER_ERROR_NO_ERROR                = 0,
    VOID_TRANSFER_ERROR_INTERNAL_ERROR_1        = 1,
    VOID_TRANSFER_ERROR_INTERNAL_ERROR_2        = 2,
    VOID_TRANSFER_ERROR_FULL                    = 3,
    VOID_TRANSFER_ERROR_INTERNAL_ERROR_3        = 4,
    VOID_TRANSFER_ERROR_INTERNAL_ERROR_4        = 5,
    VOID_TRANSFER_ERROR_NOT_ENOUGH_MONEY        = 6,
    VOID_TRANSFER_ERROR_INVENTORY_FULL          = 7,
    VOID_TRANSFER_ERROR_INTERNAL_ERROR_5        = 8,
    VOID_TRANSFER_ERROR_TRANSFER_UNKNOWN        = 9,
};

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

enum EquipmentSlots : uint8_t                                      // 19 slots
{
    EQUIPMENT_SLOT_START        = 0,
    EQUIPMENT_SLOT_HEAD         = 0,
    EQUIPMENT_SLOT_NECK         = 1,
    EQUIPMENT_SLOT_SHOULDERS    = 2,
    EQUIPMENT_SLOT_BODY         = 3,
    EQUIPMENT_SLOT_CHEST        = 4,
    EQUIPMENT_SLOT_WAIST        = 5,
    EQUIPMENT_SLOT_LEGS         = 6,
    EQUIPMENT_SLOT_FEET         = 7,
    EQUIPMENT_SLOT_WRISTS       = 8,
    EQUIPMENT_SLOT_HANDS        = 9,
    EQUIPMENT_SLOT_FINGER1      = 10,
    EQUIPMENT_SLOT_FINGER2      = 11,
    EQUIPMENT_SLOT_TRINKET1     = 12,
    EQUIPMENT_SLOT_TRINKET2     = 13,
    EQUIPMENT_SLOT_BACK         = 14,
    EQUIPMENT_SLOT_MAINHAND     = 15,
    EQUIPMENT_SLOT_OFFHAND      = 16,
    EQUIPMENT_SLOT_RANGED       = 17,
    EQUIPMENT_SLOT_TABARD       = 18,
    EQUIPMENT_SLOT_END          = 19
};

enum InventorySlots                                         // 4 slots
{
    INVENTORY_SLOT_BAG_START    = 19,
    INVENTORY_SLOT_BAG_1        = 19,
    INVENTORY_SLOT_BAG_2        = 20,
    INVENTORY_SLOT_BAG_3        = 21,
    INVENTORY_SLOT_BAG_4        = 22,
    INVENTORY_SLOT_BAG_END      = 23
};

enum InventoryPackSlots                                     // 16 slots
{
    INVENTORY_SLOT_ITEM_START   = 23,
    INVENTORY_SLOT_ITEM_1       = 23,
    INVENTORY_SLOT_ITEM_2       = 24,
    INVENTORY_SLOT_ITEM_3       = 25,
    INVENTORY_SLOT_ITEM_4       = 26,
    INVENTORY_SLOT_ITEM_5       = 27,
    INVENTORY_SLOT_ITEM_6       = 28,
    INVENTORY_SLOT_ITEM_7       = 29,
    INVENTORY_SLOT_ITEM_8       = 30,
    INVENTORY_SLOT_ITEM_9       = 31,
    INVENTORY_SLOT_ITEM_10      = 32,
    INVENTORY_SLOT_ITEM_11      = 33,
    INVENTORY_SLOT_ITEM_12      = 34,
    INVENTORY_SLOT_ITEM_13      = 35,
    INVENTORY_SLOT_ITEM_14      = 36,
    INVENTORY_SLOT_ITEM_15      = 37,
    INVENTORY_SLOT_ITEM_16      = 38,
    INVENTORY_SLOT_ITEM_END     = 39
};

enum BankItemSlots                                          // 28 slots
{
    BANK_SLOT_ITEM_START        = 39,
    BANK_SLOT_ITEM_1            = 39,
    BANK_SLOT_ITEM_2            = 40,
    BANK_SLOT_ITEM_3            = 41,
    BANK_SLOT_ITEM_4            = 42,
    BANK_SLOT_ITEM_5            = 43,
    BANK_SLOT_ITEM_6            = 44,
    BANK_SLOT_ITEM_7            = 45,
    BANK_SLOT_ITEM_8            = 46,
    BANK_SLOT_ITEM_9            = 47,
    BANK_SLOT_ITEM_10           = 48,
    BANK_SLOT_ITEM_11           = 49,
    BANK_SLOT_ITEM_12           = 50,
    BANK_SLOT_ITEM_13           = 51,
    BANK_SLOT_ITEM_14           = 52,
    BANK_SLOT_ITEM_15           = 53,
    BANK_SLOT_ITEM_16           = 54,
    BANK_SLOT_ITEM_17           = 55,
    BANK_SLOT_ITEM_18           = 56,
    BANK_SLOT_ITEM_19           = 57,
    BANK_SLOT_ITEM_20           = 58,
    BANK_SLOT_ITEM_21           = 59,
    BANK_SLOT_ITEM_22           = 60,
    BANK_SLOT_ITEM_23           = 61,
    BANK_SLOT_ITEM_24           = 62,
    BANK_SLOT_ITEM_25           = 63,
    BANK_SLOT_ITEM_26           = 64,
    BANK_SLOT_ITEM_27           = 65,
    BANK_SLOT_ITEM_28           = 66,
    BANK_SLOT_ITEM_END          = 67
};

enum BankBagSlots                                           // 7 slots
{
    BANK_SLOT_BAG_START         = 67,
    BANK_SLOT_BAG_1             = 67,
    BANK_SLOT_BAG_2             = 68,
    BANK_SLOT_BAG_3             = 69,
    BANK_SLOT_BAG_4             = 70,
    BANK_SLOT_BAG_5             = 71,
    BANK_SLOT_BAG_6             = 72,
    BANK_SLOT_BAG_7             = 73,
    BANK_SLOT_BAG_END           = 74
};

enum BuyBackSlots                                           // 12 slots
{
    // stored in m_buybackitems
    BUYBACK_SLOT_START          = 74,
    BUYBACK_SLOT_1              = 74,
    BUYBACK_SLOT_2              = 75,
    BUYBACK_SLOT_3              = 76,
    BUYBACK_SLOT_4              = 77,
    BUYBACK_SLOT_5              = 78,
    BUYBACK_SLOT_6              = 79,
    BUYBACK_SLOT_7              = 80,
    BUYBACK_SLOT_8              = 81,
    BUYBACK_SLOT_9              = 82,
    BUYBACK_SLOT_10             = 83,
    BUYBACK_SLOT_11             = 84,
    BUYBACK_SLOT_12             = 85,
    BUYBACK_SLOT_END            = 86,
    MAX_BUYBACK_SLOT            = 13
};

struct SlotResult
{
    SlotResult() { ContainerSlot = -1, Slot = -1, Result = false; }
    int8 ContainerSlot;
    int8 Slot;
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
//Key:     uint64 GUID     - GUID of the item
//
// \param time_t buytime    - time of purchase in Unixtime
// \param uint32 costid     - extendedcostID of the cost
//
//////////////////////////////////////////////////////////////////////////////////////////
typedef std::map<uint64, std::pair<time_t, uint32>> RefundableMap;

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
        Item* m_pItems[MAX_INVENTORY_SLOT];
        Item* m_pBuyBack[MAX_BUYBACK_SLOT];

        RefundableMap m_refundableitems;

        AddItemResult m_AddItem(Item* item, int8 ContainerSlot, int16 slot);

    public:

        Arcemu::EquipmentSetMgr m_EquipmentSets;
        friend class ItemIterator;
        ItemInterface(Player* pPlayer);
        ~ItemInterface();

        Player* GetOwner() { return m_pOwner; }
        bool IsBagSlot(int16 slot);

        uint32 m_CreateForPlayer(ByteBuffer* data);
        void m_DestroyForPlayer();

        void mLoadItemsFromDatabase(QueryResult* result);
        void mSaveItemsToDatabase(bool first, QueryBuffer* buf);

        Item* GetInventoryItem(int16 slot);
        Item* GetInventoryItem(int8 ContainerSlot, int16 slot);
        Container* GetContainer(int8 containerSlot);
        int16 GetInventorySlotById(uint32 ID);
        int16 GetInventorySlotByGuid(uint64 guid);
        int16 GetBagSlotByGuid(uint64 guid);

        Item* SafeAddItem(uint32 ItemId, int8 ContainerSlot, int16 slot);
        AddItemResult SafeAddItem(Item* pItem, int8 ContainerSlot, int16 slot);
        Item* SafeRemoveAndRetreiveItemFromSlot(int8 ContainerSlot, int16 slot, bool destroy);  // doesn't destroy item from memory
        Item* SafeRemoveAndRetreiveItemByGuid(uint64 guid, bool destroy);
        bool SafeFullRemoveItemFromSlot(int8 ContainerSlot, int16 slot);                        // destroys item fully
        bool SafeFullRemoveItemByGuid(uint64 guid);                                             // destroys item fully
        AddItemResult AddItemToFreeSlot(Item* item);
        AddItemResult AddItemToFreeBankSlot(Item* item);

        Item* FindItemLessMax(uint32 itemid, uint32 cnt, bool IncBank);
        uint32 GetItemCount(uint32 itemid, bool IncBank = false);
        uint32 RemoveItemAmt(uint32 id, uint32 amt);
        uint32 RemoveItemAmt_ProtectPointer(uint32 id, uint32 amt, Item** pointer);
        uint32 RemoveItemAmtByGuid(uint64 guid, uint32 amt);
        void RemoveAllConjured();
        void BuyItem(ItemProperties const* item, uint32 total_amount, Creature* pVendor);

        uint32 CalculateFreeSlots(ItemProperties const* proto);
        void ReduceItemDurability();

        uint8 LastSearchItemBagSlot() { return m_result.ContainerSlot; }
        uint8 LastSearchItemSlot() { return m_result.Slot; }
        SlotResult* LastSearchResult() { return &m_result; }

        //Searching functions
        SlotResult FindFreeInventorySlot(ItemProperties const* proto);
        SlotResult FindFreeBankSlot(ItemProperties const* proto);
        SlotResult FindAmmoBag();
        int8 FindFreeBackPackSlot();
        uint8 FindFreeBackPackSlotMax();
        int8 FindFreeKeyringSlot();
        int16 FindFreeCurrencySlot();
        int8 FindSpecialBag(Item* item);


        int8 CanEquipItemInSlot(int8 DstInvSlot, int8 slot, ItemProperties const* item, bool ignore_combat = false, bool skip_2h_check = false);
        int8 CanEquipItemInSlot2(int8 DstInvSlot, int8 slot, Item* item, bool ignore_combat = false, bool skip_2h_check = false);
        int8 CanReceiveItem(ItemProperties const* item, uint32 amount);
        int8 CanAffordItem(ItemProperties const* item, uint32 amount, Creature* pVendor);
        int8 GetItemSlotByType(uint32 type);
        Item* GetItemByGUID(uint64 itemGuid);

        void SwapItemSlots(int8 srcslot, int8 dstslot);

        uint8 GetInternalBankSlotFromPlayer(int8 islot);         // converts inventory slots into 0-x numbers

        // buyback stuff
        inline Item* GetBuyBack(int32 slot)
        {
            if (slot >= 0 && slot < MAX_BUYBACK_SLOT)
                return m_pBuyBack[slot];
            else
                return nullptr;
        }
        void AddBuyBackItem(Item* it, uint32 price);
        void RemoveBuyBackItem(uint8_t index);
        void EmptyBuyBack();
        bool IsEquipped(uint32 itemid);

        void CheckAreaItems();

        uint32 GetItemCountByLimitId(uint32 LimitId, bool IncBank);
        uint32 GetEquippedCountByItemLimit(uint32 LimitId);

        void HandleItemDurations();

        // Refundable item stuff start
        void AddRefundable(uint64 GUID, uint32 extendedcost);
        void AddRefundable(uint64 GUID, uint32 extendedcost, time_t buytime);
        void AddRefundable(Item* item, uint32 extendedcost, time_t buytime);
        void RemoveRefundable(uint64 GUID);
        std::pair<time_t, uint32> LookupRefundable(uint64 GUID);
        // Refundable item stuff end

    public:

        //////////////////////////////////////////////////////////////////////////////////////////
        // bool AddItemById(uint32 itemid, uint32 count, int32 randomprop)
        // Adds item(s) to a Player
        //
        // \param uint32 itemid     -  ID of the item(s) to add
        // \param uint32 count      -  Amount of items to add
        // \param int32 randomprop  -  Random prop or suffix id for the items to add.
        //
        // \note If positive it's prop, if negative it's suffix, if 0 it's autogenerated.
        //
        // \return true on success, false on failure.
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        bool AddItemById(uint32 itemid, uint32 count, int32 randomprop);


        //////////////////////////////////////////////////////////////////////////////////////////
        // void SwapItems(int8 DstInvSlot, int8 DstSlot, int8 SrcInvSlot, int8 SrcSlot)
        // Exchanges items A and B
        //
        // \param int8 DstInvSlot  -  Item A's bag inventory slot (-1 if it's an equipment slot)
        // \param int8 DstSlot     -  Item A's slotid within that bag
        // \param int8 SrcInvSlot  -  Item B's bag inventory slot (-1 if it's an equipment slot)
        // \param int8 SrcSlot     -  Item B's slotid within that bag
        //
        // \return true on success, false on failure.
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        bool SwapItems(int8 DstInvSlot, int8 DstSlot, int8 SrcInvSlot, int8 SrcSlot);

        void removeLootableItems();
};

class ItemIterator
{
    bool m_atEnd;
    bool m_searchInProgress;
    uint32 m_slot;
    uint32 m_containerSlot;
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
