/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#include "StdAfx.h"
#include "Item.h"
#include "Container.h"
#include "ItemPrototype.h"
#include "Units/Players/Player.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Data/WoWItem.h"
#include "Data/WoWPlayer.h"

ItemInterface::ItemInterface(Player* pPlayer) : m_EquipmentSets(pPlayer->getGuidLow())
{
    m_pOwner = pPlayer;
    memset(m_pItems, 0, sizeof(Item*)*MAX_INVENTORY_SLOT);
    memset(m_pBuyBack, 0, sizeof(Item*)*MAX_BUYBACK_SLOT);
    m_refundableitems.clear();

}

ItemInterface::~ItemInterface()
{
    for (uint8 i = 0; i < MAX_INVENTORY_SLOT; ++i)
    {
        if (m_pItems[i] != nullptr && m_pItems[i]->getOwner() == m_pOwner)
        {
            m_pItems[i]->DeleteMe();
        }
    }
    this->m_refundableitems.clear();
}

uint32 ItemInterface::m_CreateForPlayer(ByteBuffer* data)       // 100%
{
    ARCEMU_ASSERT(m_pOwner != nullptr);
    uint32 count = 0;

    for (uint8 i = 0; i < MAX_INVENTORY_SLOT; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->IsContainer())
            {
                count += static_cast<Container*>(m_pItems[i])->buildCreateUpdateBlockForPlayer(data, m_pOwner);

                for (uint32 e = 0; e < m_pItems[i]->getItemProperties()->ContainerSlots; ++e)
                {
                    Item* pItem = static_cast<Container*>(m_pItems[i])->GetItem(static_cast<int16>(e));
                    if (pItem)
                    {
                        if (pItem->IsContainer())
                        {
                            count += static_cast<Container*>(pItem)->buildCreateUpdateBlockForPlayer(data, m_pOwner);
                        }
                        else
                        {
                            count += pItem->buildCreateUpdateBlockForPlayer(data, m_pOwner);
                        }
                    }
                }
            }
            else
            {
                count += m_pItems[i]->buildCreateUpdateBlockForPlayer(data, m_pOwner);
            }
        }
    }
    return count;
}

void ItemInterface::m_DestroyForPlayer()        // 100%
{
    ARCEMU_ASSERT(m_pOwner != nullptr);

    for (uint8 i = 0; i < MAX_INVENTORY_SLOT; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->IsContainer())
            {
                for (uint32 e = 0; e < m_pItems[i]->getItemProperties()->ContainerSlots; ++e)
                {
                    Item* pItem = static_cast<Container*>(m_pItems[i])->GetItem(static_cast<int16>(e));
                    if (pItem)
                    {
                        m_pOwner->SendDestroyObject(pItem->getGuid());
                    }
                }
                m_pOwner->SendDestroyObject(m_pItems[i]->getGuid());
            }
            else
            {
                m_pOwner->SendDestroyObject(m_pItems[i]->getGuid());
            }
        }
    }
}

/// Creates and adds a item that can be manipulated after
Item* ItemInterface::SafeAddItem(uint32 ItemId, int8 ContainerSlot, int16 slot)
{
    Item* pItem;
    ItemProperties const* pProto = sMySQLStore.getItemProperties(ItemId);
    if (!pProto)
        return nullptr;

    if (pProto->InventoryType == INVTYPE_BAG)
    {
        pItem = static_cast<Item*>(new Container(HIGHGUID_TYPE_CONTAINER, objmgr.GenerateLowGuid(HIGHGUID_TYPE_CONTAINER)));
        static_cast<Container*>(pItem)->Create(ItemId, m_pOwner);
        if (m_AddItem(pItem, ContainerSlot, slot))
        {
            return pItem;
        }
        else
        {
            pItem->DeleteMe();
            return nullptr;
        }
    }
    else
    {
        pItem = new Item;
        pItem->init(HIGHGUID_TYPE_ITEM, objmgr.GenerateLowGuid(HIGHGUID_TYPE_ITEM));
        pItem->create(ItemId, m_pOwner);
        if (m_AddItem(pItem, ContainerSlot, slot))
        {
            return pItem;
        }
        else
        {
            delete pItem;
            return nullptr;
        }
    }
}

/// Creates and adds a item that can be manipulated after
AddItemResult ItemInterface::SafeAddItem(Item* pItem, int8 ContainerSlot, int16 slot)
{
    return m_AddItem(pItem, ContainerSlot, slot);
}

/// Adds items to player inventory, this includes all types of slots.
AddItemResult ItemInterface::m_AddItem(Item* item, int8 ContainerSlot, int16 slot)
{
    ARCEMU_ASSERT(slot < MAX_INVENTORY_SLOT);
    ARCEMU_ASSERT(ContainerSlot < MAX_INVENTORY_SLOT);
    if (item == nullptr || !item->getItemProperties() || slot < 0)
        return ADD_ITEM_RESULT_ERROR;

    item->m_isDirty = true;

    for (uint8_t i = 0; i < MAX_INVENTORY_SLOT; ++i)
    {
        Item * tempitem = m_pItems[i];
        if (tempitem != nullptr)
        {
            if (tempitem == item)
            {
                return ADD_ITEM_RESULT_DUPLICATED;
            }

            if (tempitem->IsContainer())
            {
                uint32_t k = tempitem->getItemProperties()->ContainerSlots;
                for (uint16_t j = 0; j < k; ++j)
                {
                    if (static_cast<Container*>(tempitem)->GetItem(j) == item)
                    {
                        return ADD_ITEM_RESULT_DUPLICATED;
                    }
                }
            }
        }
    }

    //case 1, item is from backpack container
    if (ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        //ARCEMU_ASSERT(  m_pItems[slot] == nullptr);
        if (GetInventoryItem(slot) != nullptr /*|| (slot == EQUIPMENT_SLOT_OFFHAND && !m_pOwner->HasSkillLine(118))*/)
        {
            //LOG_ERROR("bugged inventory: %u %u", m_pOwner->GetName(), item->getGuid());
            SlotResult result = this->FindFreeInventorySlot(item->getItemProperties());

            // send message to player
            sChatHandler.BlueSystemMessage(m_pOwner->GetSession(), "A duplicated item, `%s` was found in your inventory. We've attempted to add it to a free slot in your inventory, if there is none this will fail. It will be attempted again the next time you log on.",
                item->getItemProperties()->Name.c_str());
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

                return ADD_ITEM_RESULT_ERROR;
            }
        }

        if (!GetInventoryItem(slot))        //slot is free, add item.
        {
            item->setOwner(m_pOwner);
            item->setContainerGuid(m_pOwner->getGuid());
            m_pItems[(int)slot] = item;

            if (item->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP)
            {
                if (item->getItemProperties()->Flags & ITEM_FLAG_ACCOUNTBOUND)       // don't "Soulbind" account-bound items
                    item->AccountBind();
                else
                    item->SoulBind();
            }

            if (m_pOwner->IsInWorld() && !item->IsInWorld())
            {
                item->PushToWorld(m_pOwner->GetMapMgr());
                ByteBuffer buf(2500);
                uint32 count = item->buildCreateUpdateBlockForPlayer(&buf, m_pOwner);
                m_pOwner->PushCreationData(&buf, count);
            }
            m_pOwner->SetInventorySlot(slot, item->getGuid());
        }
        else
        {
            return ADD_ITEM_RESULT_ERROR;
        }
    }
    else //case 2: item is from a bag container
    {
        if (GetInventoryItem(ContainerSlot) && GetInventoryItem(ContainerSlot)->IsContainer() &&
            slot < (int32)GetInventoryItem(ContainerSlot)->getItemProperties()->ContainerSlots) //container exists
        {
            bool result = static_cast<Container*>(m_pItems[(int)ContainerSlot])->AddItem(slot, item);
            if (!result)
            {
                return ADD_ITEM_RESULT_ERROR;
            }
        }
        else
        {
            item->DeleteFromDB(); //wpe dupefix ..we don't want it reappearing on the next relog now do we?
            return ADD_ITEM_RESULT_ERROR;
        }
    }

#if VERSION_STRING > TBC
    if (slot < EQUIPMENT_SLOT_END && ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        int VisibleBase = GetOwner()->GetVisibleBase(slot);
        if (VisibleBase > PLAYER_VISIBLE_ITEM_19_ENTRYID)
        {
            LOG_DEBUG("Slot warning: slot: %d", slot);
        }
        else
        {
            m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase), item->GetEntry());
            m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase + 1), item->GetEnchantmentId(0));
        }
    }
#else
    if (slot < EQUIPMENT_SLOT_END && ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
        if (VisibleBase > PLAYER_VISIBLE_ITEM_19_0)
        {
            LOG_DEBUG("Slot warning: slot: %d", slot);
        }
        else
        {
            m_pOwner->setUInt32Value(VisibleBase, item->getUInt32Value(OBJECT_FIELD_ENTRY));
            m_pOwner->setUInt32Value(VisibleBase + 1, item->getUInt32Value(ITEM_FIELD_ENCHANTMENT));
            m_pOwner->setUInt32Value(VisibleBase + 2, item->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 3));
            m_pOwner->setUInt32Value(VisibleBase + 3, item->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 6));
            m_pOwner->setUInt32Value(VisibleBase + 4, item->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 9));
            m_pOwner->setUInt32Value(VisibleBase + 5, item->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 12));
            m_pOwner->setUInt32Value(VisibleBase + 6, item->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 15));
            m_pOwner->setUInt32Value(VisibleBase + 7, item->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 18));
            m_pOwner->setInt32Value(VisibleBase + 8, item->getInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));
        }
    }
#endif

    if (m_pOwner->IsInWorld() && slot < INVENTORY_SLOT_BAG_END && ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        m_pOwner->ApplyItemMods(item, slot, true);
    }

    if (slot >= CURRENCYTOKEN_SLOT_START && slot < CURRENCYTOKEN_SLOT_END)
    {
        m_pOwner->UpdateKnownCurrencies(item->GetEntry(), true);
    }

    if (ContainerSlot == INVENTORY_SLOT_NOT_SET && slot == EQUIPMENT_SLOT_OFFHAND && item->getItemProperties()->Class == ITEM_CLASS_WEAPON)
    {
        m_pOwner->SetDualWield(true);

        /////////////////////////////////////////// Titan's grip stuff ////////////////////////////////////////////////////////////

        uint32 subclass = item->getItemProperties()->SubClass;
        if (subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_AXE || subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_MACE || subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD)
        {
            m_pOwner->CastSpell(m_pOwner, 49152, true);

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
    }

#if VERSION_STRING > TBC
    m_pOwner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM, item->GetEntry(), 1, 0);
#endif
    ////////////////////////////////////////////////////// existingduration stuff /////////////////////////////////////////////////////
    if (item->getItemProperties()->ExistingDuration != 0)
    {
        if (item->GetItemExpireTime() == 0)
        {
            item->SetItemExpireTime(UNIXTIME + item->getItemProperties()->ExistingDuration);
            item->setDuration(item->getItemProperties()->ExistingDuration);
            sEventMgr.AddEvent(item, &Item::EventRemoveItem, EVENT_REMOVE_ITEM, item->getItemProperties()->ExistingDuration * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
        }
        else
        {
            item->setDuration(static_cast<uint32>(item->GetItemExpireTime() - UNIXTIME));
            sEventMgr.AddEvent(item, &Item::EventRemoveItem, EVENT_REMOVE_ITEM, (item->GetItemExpireTime() - UNIXTIME) * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
        }

        // if we are already in the world we will send the durationupdate now, so we can see the remaining duration in the client
        // otherwise we will send the updates in Player::Onpushtoworld anyways
        if (m_pOwner->IsInWorld())
            sEventMgr.AddEvent(item, &Item::SendDurationUpdate, EVENT_SEND_PACKET_TO_PLAYER_AFTER_LOGIN, 0, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }

    return ADD_ITEM_RESULT_OK;
}

/// Checks if the slot is a Bag slot
bool ItemInterface::IsBagSlot(int16 slot)
{
    if ((slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
    {
        return true;
    }
    return false;
}

/// Removes the item safely and returns it back for usage
Item* ItemInterface::SafeRemoveAndRetreiveItemFromSlot(int8 ContainerSlot, int16 slot, bool destroy)
{
    ARCEMU_ASSERT(slot < MAX_INVENTORY_SLOT);
    ARCEMU_ASSERT(ContainerSlot < MAX_INVENTORY_SLOT);
    Item* pItem = nullptr;

    if (ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        pItem = GetInventoryItem(ContainerSlot, slot);

        if (pItem == nullptr)
        {
            return nullptr;
        }

        if (pItem->getItemProperties()->ContainerSlots > 0 && pItem->IsContainer() && static_cast<Container*>(pItem)->HasItems())
        {
            // sounds weird? no. this will trigger a callstack display due to my other debug code.
            pItem->DeleteFromDB();
            return nullptr;
        }

        m_pItems[(int)slot] = nullptr;
        if (pItem->getOwner() == m_pOwner)
        {
            pItem->m_isDirty = true;

            m_pOwner->SetInventorySlot(slot, 0);

            if (slot < EQUIPMENT_SLOT_END)
            {
                m_pOwner->ApplyItemMods(pItem, slot, false);
                int VisibleBase = GetOwner()->GetVisibleBase(slot);
                for (int i = VisibleBase; i < VisibleBase + 2; ++i)
                {
                    m_pOwner->setUInt32Value(static_cast<uint16_t>(i), 0);
                }
            }
            else if (slot < INVENTORY_SLOT_BAG_END)
                m_pOwner->ApplyItemMods(pItem, slot, false);

            if (slot == EQUIPMENT_SLOT_OFFHAND)
                m_pOwner->SetDualWield(false);

            if (destroy)
            {
                if (pItem->IsInWorld())
                {
                    pItem->RemoveFromWorld();
                }
                pItem->DeleteFromDB();
            }
        }
        else
            pItem = nullptr;
    }
    else
    {
        Item* pContainer = GetInventoryItem(ContainerSlot);
        if (pContainer && pContainer->IsContainer())
        {
            pItem = static_cast<Container*>(pContainer)->SafeRemoveAndRetreiveItemFromSlot(slot, destroy);
        }
    }

    return pItem;
}

/// Removes the item safely by guid and returns it back for usage, supports full inventory
Item* ItemInterface::SafeRemoveAndRetreiveItemByGuid(uint64 guid, bool destroy)
{
    int16 i = 0;

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
            if (item && item->IsContainer())
            {
                for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                    if (item2 && item2->getGuid() == guid)
                    {
                        return static_cast<Container*>(item)->SafeRemoveAndRetreiveItemFromSlot(static_cast<int16>(j), destroy);
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
            if (item && item->IsContainer())
            {
                for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                    if (item2 && item2->getGuid() == guid)
                    {
                        return static_cast<Container*>(item)->SafeRemoveAndRetreiveItemFromSlot(static_cast<int16>(j), destroy);
                    }
                }
            }
        }
    }

    return nullptr;
}

/// Completely removes item from player
/// \return true if item removal was succefull
bool ItemInterface::SafeFullRemoveItemFromSlot(int8 ContainerSlot, int16 slot)
{
    ARCEMU_ASSERT(slot < MAX_INVENTORY_SLOT);
    ARCEMU_ASSERT(ContainerSlot < MAX_INVENTORY_SLOT);

    if (ContainerSlot == INVENTORY_SLOT_NOT_SET)
    {
        Item* pItem = GetInventoryItem(slot);

        if (pItem == nullptr)
            return false;

        if (pItem->getItemProperties()->ContainerSlots > 0 && pItem->IsContainer() && static_cast<Container*>(pItem)->HasItems())
        {
            // sounds weird? no. this will trigger a callstack display due to my other debug code.
            pItem->DeleteFromDB();
            return false;
        }

        m_pItems[(int)slot] = nullptr;
        // hacky crashfix
        if (pItem->getOwner() == m_pOwner)
        {
            pItem->m_isDirty = true;

            m_pOwner->SetInventorySlot(slot, 0);

            if (slot < EQUIPMENT_SLOT_END)
            {
                m_pOwner->ApplyItemMods(pItem, slot, false);
                int VisibleBase = GetOwner()->GetVisibleBase(slot);
                for (int i = VisibleBase; i < VisibleBase + 2; ++i)
                {
                    m_pOwner->setUInt32Value(static_cast<uint16_t>(i), 0);
                }
            }
            else if (slot < INVENTORY_SLOT_BAG_END)
                m_pOwner->ApplyItemMods(pItem, slot, false);  //watch containers that give attackspeed and stuff ;)

            if (slot == EQUIPMENT_SLOT_OFFHAND)
                m_pOwner->SetDualWield(false);

            if (pItem->IsInWorld())
            {
                pItem->RemoveFromWorld();
            }

            pItem->DeleteFromDB();

            //delete pItem;
            // We make it a garbage item, so when it's used for a spell, it gets deleted in the next Player update
            // otherwise we get a nice crash
            m_pOwner->AddGarbageItem(pItem);
        }
    }
    else
    {
        Item* pContainer = GetInventoryItem(ContainerSlot);
        if (pContainer && pContainer->IsContainer())
        {
            static_cast<Container*>(pContainer)->SafeFullRemoveItemFromSlot(slot);
        }
    }
    return true;
}

/// Removes the item safely by guid, supports full inventory
bool ItemInterface::SafeFullRemoveItemByGuid(uint64 guid)
{
    int16 i = 0;

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
            if (item && item->IsContainer())
            {
                for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                    if (item2 && item2->getGuid() == guid)
                    {
                        return static_cast<Container*>(item)->SafeFullRemoveItemFromSlot(static_cast<int16>(j));
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
            if (item && item->IsContainer())
            {
                for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                    if (item2 && item2->getGuid() == guid)
                    {
                        return static_cast<Container*>(item)->SafeFullRemoveItemFromSlot(static_cast<int16>(j));
                    }
                }
            }
        }
    }
    return false;
}

/// Gets a item from Inventory
Item* ItemInterface::GetInventoryItem(int16 slot)
{
    if (slot < 0 || slot >= MAX_INVENTORY_SLOT)
        return nullptr;

    return m_pItems[(int)slot];
}

/// Gets a Item from inventory or container
Item* ItemInterface::GetInventoryItem(int8 ContainerSlot, int16 slot)
{
    if (ContainerSlot <= INVENTORY_SLOT_NOT_SET)
    {
        if (slot < 0 || slot >= MAX_INVENTORY_SLOT)
            return nullptr;

        return m_pItems[(int)slot];
    }
    else
    {
        if (IsBagSlot(ContainerSlot))
        {
            if (m_pItems[(int)ContainerSlot])
            {
                return static_cast<Container*>(m_pItems[(int)ContainerSlot])->GetItem(slot);
            }
        }
    }
    return nullptr;
}

Container* ItemInterface::GetContainer(int8 containerSlot)
{
    if (!IsBagSlot(containerSlot))
        return nullptr;

    if (m_pItems[containerSlot] == nullptr)
        return nullptr;

    return static_cast<Container*>(m_pItems[containerSlot]);
}

/// Checks for stacks that didn't reached max capacity
Item* ItemInterface::FindItemLessMax(uint32 itemid, uint32 cnt, bool IncBank)
{
    uint32 i;
    for (i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item)
        {
            uint32 itemMaxStack = (item->getOwner()->ItemStackCheat) ? 0x7fffffff : item->getItemProperties()->MaxCount;
            if ((item->GetEntry() == itemid && item->wrapped_item_id == 0) && (itemMaxStack >= (item->GetStackCount() + cnt)))
            {
                return item;
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item && item->IsContainer())
        {
            for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                if (item2)
                {
                    uint32 itemMaxStack = (item2->getOwner()->ItemStackCheat) ? 0x7fffffff : item2->getItemProperties()->MaxCount;
                    if ((item2->getItemProperties()->ItemId == itemid && item2->wrapped_item_id == 0) && (itemMaxStack >= (item2->GetStackCount() + cnt)))
                    {
                        return item2;
                    }
                }
            }

        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item)
        {
            uint32 itemMaxStack = (item->getOwner()->ItemStackCheat) ? 0x7fffffff : item->getItemProperties()->MaxCount;
            if ((item->GetEntry() == itemid && item->wrapped_item_id == 0) && (itemMaxStack >= (item->GetStackCount() + cnt)))
            {
                return item;
            }
        }
    }

    if (IncBank)
    {
        for (i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16>(i));
            if (item)
            {
                uint32 itemMaxStack = (item->getOwner()->ItemStackCheat) ? 0x7fffffff : item->getItemProperties()->MaxCount;
                if ((item->GetEntry() == itemid && item->wrapped_item_id == 0) && (itemMaxStack >= (item->GetStackCount() + cnt)))
                {
                    return item;
                }
            }
        }

        for (i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16>(i));
            if (item && item->IsContainer())
            {

                for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                    if (item2)
                    {
                        uint32 itemMaxStack = (item2->getOwner()->ItemStackCheat) ? 0x7fffffff : item2->getItemProperties()->MaxCount;
                        if ((item2->getItemProperties()->ItemId == itemid && item2->wrapped_item_id == 0) && (itemMaxStack >= (item2->GetStackCount() + cnt)))
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
uint32 ItemInterface::GetItemCount(uint32 itemid, bool IncBank)
{
    uint32 cnt = 0;
    uint32 i;
    for (i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));

        if (item)
        {
            if (item->GetEntry() == itemid && item->wrapped_item_id == 0)
            {
                cnt += item->GetStackCount() ? item->GetStackCount() : 1;
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item && item->IsContainer())
        {
            for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                if (item2)
                {
                    if (item2->GetEntry() == itemid && item->wrapped_item_id == 0)
                    {
                        cnt += item2->GetStackCount() ? item2->GetStackCount() : 1;
                    }
                }
            }

        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));

        if (item)
        {
            if (item->getItemProperties()->ItemId == itemid && item->wrapped_item_id == 0)
            {
                cnt += item->GetStackCount() ? item->GetStackCount() : 1;
            }
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));

        if (item)
        {
            if (item->getItemProperties()->ItemId == itemid && item->wrapped_item_id == 0)
            {
                cnt += item->GetStackCount() ? item->GetStackCount() : 1;
            }
        }
    }

    if (IncBank)
    {
        for (i = BANK_SLOT_ITEM_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16>(i));
            if (item)
            {
                if (item->getItemProperties()->ItemId == itemid && item->wrapped_item_id == 0)
                {
                    cnt += item->GetStackCount() ? item->GetStackCount() : 1;
                }
            }
        }

        for (i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16>(i));
            if (item)
            {
                if (item->IsContainer())
                {
                    for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                    {
                        Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                        if (item2)
                        {
                            if (item2->getItemProperties()->ItemId == itemid && item->wrapped_item_id == 0)
                            {
                                cnt += item2->GetStackCount() ? item2->GetStackCount() : 1;
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
uint32 ItemInterface::RemoveItemAmt(uint32 id, uint32 amt)
{
    uint32 i;

    for (i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item)
        {
            if (item->GetEntry() == id && item->wrapped_item_id == 0)
            {
                if (item->getItemProperties()->ContainerSlots > 0 && item->IsContainer() && ((Container*)item)->HasItems())
                {
                    // sounds weird? no. this will trigger a callstack display due to my other debug code.
                    item->DeleteFromDB();
                    continue;
                }

                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
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
                    amt -= item->GetStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
                }
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item && item->IsContainer())
        {
            for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = ((Container*)item)->GetItem(static_cast<int16>(j));
                if (item2)
                {
                    if (item2->getItemProperties()->ItemId == id && item->wrapped_item_id == 0)
                    {
                        if (item2->GetStackCount() > amt)
                        {
                            item2->setStackCount(item2->GetStackCount() - amt);
                            item2->m_isDirty = true;
                            return amt;
                        }
                        else if (item2->GetStackCount() == amt)
                        {
                            bool result = SafeFullRemoveItemFromSlot(static_cast<int8>(i), static_cast<int16>(j));
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
                            amt -= item2->GetStackCount();
                            SafeFullRemoveItemFromSlot(static_cast<int8>(i), static_cast<int16>(j));

                        }
                    }
                }
            }
        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item)
        {
            if (item->getItemProperties()->ItemId == id && item->wrapped_item_id == 0)
            {
                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
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
                    amt -= item->GetStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
                }
            }
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item)
        {
            if (item->getItemProperties()->ItemId == id && item->wrapped_item_id == 0)
            {
                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
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
                    amt -= item->GetStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
                }
            }
        }
    }
    return 0;
}

uint32 ItemInterface::RemoveItemAmt_ProtectPointer(uint32 id, uint32 amt, Item** pointer)
{
    //this code returns shit return value is fucked
    if (GetItemCount(id) < amt)
    {
        return 0;
    }
    uint32 i;

    for (i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item)
        {
            if (item->GetEntry() == id && item->wrapped_item_id == 0)
            {
                if (item->getItemProperties()->ContainerSlots > 0 && item->IsContainer() && ((Container*)item)->HasItems())
                {
                    // sounds weird? no. this will trigger a callstack display due to my other debug code.
                    item->DeleteFromDB();
                    continue;
                }

                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));

                    // bool result = true;

                    if (pointer != NULL && *pointer == item)
                        *pointer = NULL;

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
                    amt -= item->GetStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));

                    if (pointer != NULL && *pointer == item)
                        *pointer = NULL;
                }
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item && item->IsContainer())
        {
            for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                if (item2)
                {
                    if (item2->getItemProperties()->ItemId == id && item->wrapped_item_id == 0)
                    {
                        if (item2->GetStackCount() > amt)
                        {
                            item2->setStackCount(item2->GetStackCount() - amt);
                            item2->m_isDirty = true;
                            return amt;
                        }
                        else if (item2->GetStackCount() == amt)
                        {
                            bool result = SafeFullRemoveItemFromSlot(static_cast<int8>(i), static_cast<int16>(j));
                            if (pointer != NULL && *pointer == item2)
                                *pointer = NULL;

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
                            amt -= item2->GetStackCount();
                            SafeFullRemoveItemFromSlot(static_cast<int8>(i), static_cast<int16>(j));

                            if (pointer != NULL && *pointer == item2)
                                *pointer = NULL;
                        }
                    }
                }
            }
        }
    }

    for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item)
        {
            if (item->getItemProperties()->ItemId == id && item->wrapped_item_id == 0)
            {
                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
                    if (pointer != NULL && *pointer == item)
                        *pointer = NULL;

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
                    amt -= item->GetStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));

                    if (pointer != NULL && *pointer == item)
                        *pointer = NULL;
                }
            }
        }
    }

    for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item)
        {
            if (item->getItemProperties()->ItemId == id && item->wrapped_item_id == 0)
            {
                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
                {
                    bool result = SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
                    if (pointer != NULL && *pointer == item)
                        *pointer = NULL;

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
                    amt -= item->GetStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));

                    if (pointer != NULL && *pointer == item)
                        *pointer = NULL;
                }
            }
        }
    }
    return 0;
}

/// Removes desired amount of items by guid
uint32 ItemInterface::RemoveItemAmtByGuid(uint64 guid, uint32 amt)
{
    int16 i;

    for (i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item)
        {
            if (item->getGuid() == guid && item->wrapped_item_id == 0)
            {
                if (item->getItemProperties()->ContainerSlots > 0 && item->IsContainer() && static_cast<Container*>(item)->HasItems())
                {
                    // sounds weird? no. this will trigger a callstack display due to my other debug code.
                    item->DeleteFromDB();
                    continue;
                }

                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
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
                    amt -= item->GetStackCount();
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, i);
                    return amt;
                }
            }
        }
    }

    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* item = GetInventoryItem(i);
        if (item && item->IsContainer())
        {
            for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item)->GetItem(static_cast<int16>(j));
                if (item2)
                {
                    if (item2->getGuid() == guid && item->wrapped_item_id == 0)
                    {
                        if (item2->GetStackCount() > amt)
                        {
                            item2->setStackCount(item2->GetStackCount() - amt);
                            item2->m_isDirty = true;
                            return amt;
                        }
                        else if (item2->GetStackCount() == amt)
                        {
                            bool result = SafeFullRemoveItemFromSlot(static_cast<int8>(i), static_cast<int16>(j));
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
                            amt -= item2->GetStackCount();
                            SafeFullRemoveItemFromSlot(static_cast<int8>(i), static_cast<int16>(j));
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
            if (item->getGuid() == guid && item->wrapped_item_id == 0)
            {
                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
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
                    amt -= item->GetStackCount();
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
            if (item->getGuid() == guid && item->wrapped_item_id == 0)
            {
                if (item->GetStackCount() > amt)
                {
                    item->setStackCount(item->GetStackCount() - amt);
                    item->m_isDirty = true;
                    return amt;
                }
                else if (item->GetStackCount() == amt)
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
                    amt -= item->GetStackCount();
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
    for (uint32 x = INVENTORY_SLOT_BAG_START; x < INVENTORY_SLOT_ITEM_END; ++x)
    {
        if (m_pItems[x] != nullptr)
        {
            if (IsBagSlot(static_cast<int16>(x)) && m_pItems[x]->IsContainer())
            {
                Container* bag = static_cast<Container*>(m_pItems[x]);

                for (uint32 i = 0; i < bag->getItemProperties()->ContainerSlots; ++i)
                {
                    if (bag->GetItem(static_cast<int16>(i)) != nullptr && bag->GetItem(static_cast<int16>(i))->getItemProperties()->Flags & 2)
                        bag->SafeFullRemoveItemFromSlot(static_cast<int16>(i));
                }
            }
            else
            {
                if (m_pItems[x]->getItemProperties()->Flags & 2)
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(x));
            }
        }
    }
}

/// Gets slot number by itemid, banks not included
int16 ItemInterface::GetInventorySlotById(uint32 ID)
{
    for (uint16 i = 0; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getItemProperties()->ItemId == ID)
            {
                return i;
            }
        }
    }

    for (uint16 i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getItemProperties()->ItemId == ID)
            {
                return i;
            }
        }
    }

    for (uint16 i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
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
int16 ItemInterface::GetInventorySlotByGuid(uint64 guid)
{
    for (uint16 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    for (uint16 i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    for (uint16 i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
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

int16 ItemInterface::GetBagSlotByGuid(uint64 guid)
{
    for (uint16 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    for (uint16 i = INVENTORY_KEYRING_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (m_pItems[i])
        {
            if (m_pItems[i]->getGuid() == guid)
            {
                return i;
            }
        }
    }

    for (uint16 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] && m_pItems[i]->IsContainer())
        {
            for (uint32 j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
            {
                Item* inneritem = (static_cast<Container*>(m_pItems[i]))->GetItem(static_cast<int16>(j));
                if (inneritem && inneritem->getGuid() == guid)
                    return i;
            }
        }
    }

    return ITEM_NO_SLOT_AVAILABLE; //was changed from 0 cuz 0 is the slot for head
}

/// Adds a Item to a free slot
AddItemResult ItemInterface::AddItemToFreeSlot(Item* item)
{
    if (item == nullptr)
        return ADD_ITEM_RESULT_ERROR;

    if (item->getItemProperties() == nullptr)
        return ADD_ITEM_RESULT_ERROR;

    uint8 i = 0;
    bool result2;
    AddItemResult result3;
    Player* p = m_pOwner;
    uint32 itemMaxStack = item->getItemProperties()->MaxCount;

    //detect special bag item
    if (item->getItemProperties()->BagFamily)
    {
        if (item->getItemProperties()->BagFamily & ITEM_TYPE_KEYRING || item->getItemProperties()->Class == ITEM_CLASS_KEY)
        {
            for (i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
            {
                if (m_pItems[i] == nullptr)
                {
                    result3 = SafeAddItem(item, INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
                    if (result3)
                    {
                        m_result.ContainerSlot = INVENTORY_SLOT_NOT_SET;
                        m_result.Slot = static_cast<int8>(i);
                        m_result.Result = true;
                        return ADD_ITEM_RESULT_OK;
                    }
                }
            }
        }
        else if (item->getItemProperties()->BagFamily & ITEM_TYPE_CURRENCY)
        {
            for (i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
            {
                if (m_pItems[i])
                    itemMaxStack = (p->ItemStackCheat) ? 0x7fffffff : m_pItems[i]->getItemProperties()->MaxCount;

                if (m_pItems[i] == nullptr)
                {
                    result3 = SafeAddItem(item, INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
                    if (result3)
                    {
                        m_result.ContainerSlot = INVENTORY_SLOT_NOT_SET;
                        m_result.Slot = static_cast<int8>(i);
                        m_result.Result = true;
                        p->UpdateKnownCurrencies(m_pItems[i]->GetEntry(), true);
                        return ADD_ITEM_RESULT_OK;
                    }
                }
                else if (m_pItems[i]->getItemProperties()->ItemId == item->getItemProperties()->ItemId && itemMaxStack > 1 &&
                    m_pItems[i]->GetStackCount() < itemMaxStack  &&
                    m_pItems[i]->GetStackCount() + item->GetStackCount() <= itemMaxStack)
                {
                    m_pItems[i]->setStackCount(m_pItems[i]->GetStackCount() + item->GetStackCount());
                    m_result.Slot = static_cast<int8>(i);
                    m_result.Result = true;
                    p->UpdateKnownCurrencies(m_pItems[i]->GetEntry(), true);
                    return ADD_ITEM_RESULT_OK;
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
                        if (m_pItems[i]->IsContainer())
                        {
                            uint32 r_slot;
                            result2 = static_cast<Container*>(m_pItems[i])->AddItemToFreeSlot(item, &r_slot);
                            if (result2)
                            {
                                m_result.ContainerSlot = static_cast<int8>(i);
                                m_result.Slot = static_cast<int8>(r_slot);
                                m_result.Result = true;
                                return ADD_ITEM_RESULT_OK;
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
            itemMaxStack = (p->ItemStackCheat) ? 0x7fffffff : m_pItems[i]->getItemProperties()->MaxCount;
        if (m_pItems[i] == nullptr)
        {
            result3 = SafeAddItem(item, INVENTORY_SLOT_NOT_SET, static_cast<int16>(i));
            if (result3)
            {
                m_result.ContainerSlot = INVENTORY_SLOT_NOT_SET;
                m_result.Slot = static_cast<int8>(i);
                m_result.Result = true;
                return ADD_ITEM_RESULT_OK;
            }
        }
        else if (m_pItems[i]->getItemProperties()->ItemId == item->getItemProperties()->ItemId &&
            itemMaxStack > 1 &&
            m_pItems[i]->GetStackCount() < itemMaxStack  &&
            m_pItems[i]->GetStackCount() + item->GetStackCount() <= itemMaxStack)
        {
            m_pItems[i]->setStackCount(m_pItems[i]->GetStackCount() + item->GetStackCount());
            m_pItems[i]->m_isDirty = true;
            m_result.Slot = static_cast<int8>(i);
            m_result.Result = true;

            // delete the item because we added the stacks to another one
            item->DeleteFromDB();
            // We make it a garbage item, so if it's used after calling this method, it gets deleted in the next Player update
            // otherwise we get a nice crash
            m_pOwner->AddGarbageItem(item);

            return ADD_ITEM_RESULT_OK;
        }
    }

    //INVENTORY BAGS
    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr && m_pItems[i]->getItemProperties()->BagFamily == 0 && m_pItems[i]->IsContainer()) //special bags ignored
        {
            for (uint32 j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = (static_cast<Container*>(m_pItems[i]))->GetItem(static_cast<int16>(j));
                if (item2)
                    itemMaxStack = (p->ItemStackCheat) ? 0x7fffffff : item2->getItemProperties()->MaxCount;
                if (item2 == nullptr)
                {
                    result3 = SafeAddItem(item, static_cast<int8>(i), static_cast<int16>(j));
                    if (result3)
                    {
                        m_result.ContainerSlot = static_cast<int8>(i);
                        m_result.Slot = static_cast<int8>(j);
                        m_result.Result = true;
                        return ADD_ITEM_RESULT_OK;
                    }
                }
                else if (item2->getItemProperties()->ItemId == item->getItemProperties()->ItemId &&
                    itemMaxStack > 1 &&
                    item2->GetStackCount() < itemMaxStack &&
                    item2->GetStackCount() + item->GetStackCount() <= itemMaxStack)
                {
                    item2->setStackCount(item2->GetStackCount() + item->GetStackCount());
                    item2->m_isDirty = true;
                    m_result.Slot = static_cast<int8>(i);
                    m_result.Result = true;

                    // delete the item because we added the stacks to another one
                    item->DeleteFromDB();
                    // We make it a garbage item, so if it's used after calling this method, it gets deleted in the next Player update
                    // otherwise we get a nice crash
                    m_pOwner->AddGarbageItem(item);

                    return ADD_ITEM_RESULT_OK;
                }
            }
        }
    }
    return ADD_ITEM_RESULT_ERROR;
}

/// Calculates inventory free slots, bag inventory slots not included
uint32 ItemInterface::CalculateFreeSlots(ItemProperties const* proto)
{
    uint32 count = 0;
    uint32 i;

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
                    if (m_pItems[i] && m_pItems[i]->IsContainer())
                    {
                        if (m_pItems[i]->getItemProperties()->BagFamily & proto->BagFamily)
                        {
                            int8 slot = static_cast<Container*>(m_pItems[i])->FindFreeSlot();
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
            if (m_pItems[i]->IsContainer() && !m_pItems[i]->getItemProperties()->BagFamily)
            {

                for (uint32 j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
                {
                    Item* item2 = (static_cast<Container*>(m_pItems[i]))->GetItem(static_cast<int16>(j));
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
int8 ItemInterface::FindFreeBackPackSlot()
{
    //search for backpack slots
    for (int8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (!item)
        {
            return i;
        }
    }

    return ITEM_NO_SLOT_AVAILABLE;      //no slots available
}

uint8 ItemInterface::FindFreeBackPackSlotMax()
{
    //search for backpack slots
    uint8 slotfree = 0;
    for (int8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (!item) slotfree++;
    }
    return slotfree;
}

/// Converts bank bags slot ids into player bank byte slots(0-5)
int8 ItemInterface::GetInternalBankSlotFromPlayer(int8 islot)
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
int8 ItemInterface::CanEquipItemInSlot2(int8 DstInvSlot, int8 slot, Item* item, bool ignore_combat /* = false */, bool skip_2h_check /* = false */)
{
    ItemProperties const* proto = item->getItemProperties();

    if (int8 ret = CanEquipItemInSlot(DstInvSlot, slot, proto, ignore_combat, skip_2h_check))
        return ret;

    if ((slot < INVENTORY_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET))
    {
        for (uint32 count = 0; count < item->GetSocketsCount(); count++)
        {
            EnchantmentInstance* ei = item->GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + count);
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
                        uint32 LimitId = ip->ItemLimitCategory;
                        auto item_limit_category = sItemLimitCategoryStore.LookupEntry(LimitId);
                        if (item_limit_category)
                        {
                            uint32 gemCount = 0;
                            if ((item_limit_category->equippedFlag & ILFLAG_EQUIP_ONLY  && slot < EQUIPMENT_SLOT_END) || (!(item_limit_category->equippedFlag & ILFLAG_EQUIP_ONLY) && slot > EQUIPMENT_SLOT_END))
                                gemCount = item->CountGemsWithLimitId(item_limit_category->Id);

                            uint32 gCount = GetEquippedCountByItemLimit(item_limit_category->Id);
                            if ((gCount + gemCount) > item_limit_category->maxAmount)
                                return INV_ERR_CANT_CARRY_MORE_OF_THIS;
                        }
                    }
#endif
                }
            }
        }
    }

    return 0;
}

/// Checks if the item can be equipped on a specific slot
int8 ItemInterface::CanEquipItemInSlot(int8 DstInvSlot, int8 slot, ItemProperties const* proto, bool ignore_combat /* = false */, bool skip_2h_check /* = false */)
{
    if (proto == nullptr)
        return INV_ERR_ITEMS_CANT_BE_SWAPPED;

    uint32 type = proto->InventoryType;

    if (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END && DstInvSlot == -1)
        if (proto->ContainerSlots == 0)
            return INV_ERR_ITEMS_CANT_BE_SWAPPED;

    if ((slot < INVENTORY_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET) || (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET))
    {
        if (!ignore_combat && m_pOwner->CombatStatus.IsInCombat() && (slot < EQUIPMENT_SLOT_MAINHAND || slot > EQUIPMENT_SLOT_RANGED))
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
            Standing current_standing = Player::GetReputationRankFromStanding(m_pOwner->GetStanding(proto->RequiredFaction));
            if (current_standing < (Standing)proto->RequiredFactionStanding)       // Not enough rep rankage..
                return INV_ERR_ITEM_REPUTATION_NOT_ENOUGH;
        }

        // Check to see if we have the correct level.
        if (proto->RequiredLevel > m_pOwner->getLevel())
            return INV_ERR_YOU_MUST_REACH_LEVEL_N;

        if (proto->Class == 4)
        {
            uint32 bogus_subclass = 0;
            uint32 playerlevel = 0;
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

            if (!(m_pOwner->GetArmorProficiency() & (((uint32)(1)) << bogus_subclass)))
                return INV_ERR_NO_REQUIRED_PROFICIENCY;

        }
        else if (proto->Class == 2)
        {
            if (!(m_pOwner->GetWeaponProficiency() & (((uint32)(1)) << proto->SubClass)))
                return INV_ERR_NO_REQUIRED_PROFICIENCY;
        }

        if (proto->RequiredSkill)
            if (proto->RequiredSkillRank > m_pOwner->_GetSkillLineCurrent(proto->RequiredSkill, true))
                return INV_ERR_SKILL_ISNT_HIGH_ENOUGH;

        if (proto->RequiredSkillSubRank)
            if (!m_pOwner->HasSpell(proto->RequiredSkillSubRank))
                return INV_ERR_NO_REQUIRED_PROFICIENCY;

        // You are dead !
        if (m_pOwner->getDeathState() != ALIVE)
            return INV_ERR_YOU_ARE_DEAD;
    }

    switch (uint8(slot))        //CURRENCYTOKEN_SLOT_ are over 128
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
                (type == INVTYPE_2HWEAPON && (!GetInventoryItem(EQUIPMENT_SLOT_OFFHAND) || skip_2h_check || m_pOwner->DualWield2H)))
                return 0;
            else
                return INV_ERR_ITEM_DOESNT_GO_TO_SLOT;
        }
        case EQUIPMENT_SLOT_OFFHAND:
        {
            if ((type == INVTYPE_2HWEAPON || type == INVTYPE_SHIELD) && m_pOwner->DualWield2H)
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
                        if (m_pOwner->_HasSkillLine(SKILL_DUAL_WIELD))
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
                    if (m_pOwner->_HasSkillLine(SKILL_DUAL_WIELD))
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
            int32 bytes, slots;
            int8 islot;

            if (!GetInventoryItem(INVENTORY_SLOT_NOT_SET, slot))        //check if player got that slot.
            {
                bytes = m_pOwner->getUInt32Value(PLAYER_BYTES_2);
                slots = (uint8)(bytes >> 16);
                islot = GetInternalBankSlotFromPlayer(slot);
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
int8 ItemInterface::CanReceiveItem(ItemProperties const* item, uint32 amount)
{
    if (!item)
    {
        return INV_ERR_OK;
    }

    if (item->Unique)
    {
        uint32 count = GetItemCount(item->ItemId, true);
        if (count == item->Unique || ((count + amount) > item->Unique))
        {
            return INV_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }

#if VERSION_STRING > TBC
    if (item->ItemLimitCategory > 0)
    {
        auto item_limit_category = sItemLimitCategoryStore.LookupEntry(item->ItemLimitCategory);
        if (item_limit_category && !(item_limit_category->equippedFlag & ILFLAG_EQUIP_ONLY))
        {
            uint32 count = GetItemCountByLimitId(item_limit_category->Id, false);
            if (count >= item_limit_category->maxAmount || ((count + amount) > item_limit_category->maxAmount))
                return INV_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED;
        }
    }
#endif

    return INV_ERR_OK;
}

void ItemInterface::BuyItem(ItemProperties const* item, uint32 total_amount, Creature* pVendor)
{
    if (item->BuyPrice)
    {
        uint32 itemprice = GetBuyPriceForItem(item, total_amount, m_pOwner, pVendor);
        if (!m_pOwner->HasGold(itemprice))
            m_pOwner->SetGold(0);
        else
            m_pOwner->ModGold(-(int32)itemprice);
    }
    auto item_extended_cost = pVendor->GetItemExtendedCostByItemId(item->ItemId);
    if (item_extended_cost != nullptr)
    {
        for (uint8 i = 0; i < 5; ++i)
        {
            if (item_extended_cost->item[i])
                m_pOwner->GetItemInterface()->RemoveItemAmt(item_extended_cost->item[i], total_amount * item_extended_cost->count[i]);
        }

        if (m_pOwner->GetHonorCurrency() >= (item_extended_cost->honor_points * total_amount))
        {
            m_pOwner->ModHonorCurrency(-int32((item_extended_cost->honor_points * total_amount)));
            m_pOwner->m_honorPoints -= int32(item_extended_cost->honor_points * total_amount);
        }
        if (m_pOwner->GetArenaCurrency() >= (item_extended_cost->arena_points * total_amount))
        {
            m_pOwner->ModArenaCurrency(-int32(item_extended_cost->arena_points * total_amount));
            m_pOwner->m_arenaPoints -= int32(item_extended_cost->arena_points * total_amount);
        }
    }
}

int8 ItemInterface::CanAffordItem(ItemProperties const* item, uint32 amount, Creature* pVendor)
{
    auto item_extended_cost = pVendor->GetItemExtendedCostByItemId(item->ItemId);
    if (item_extended_cost != nullptr)
    {
        for (uint8 i = 0; i < 5; ++i)
        {
            if (item_extended_cost->item[i])
            {
                if (m_pOwner->GetItemInterface()->GetItemCount(item_extended_cost->item[i], false) < (item_extended_cost->count[i] * amount))
                    return INV_ERR_VENDOR_MISSING_TURNINS;
            }
        }

        if (m_pOwner->GetHonorCurrency() < (item_extended_cost->honor_points * amount))
            return INV_ERR_NOT_ENOUGH_HONOR_POINTS;
        if (m_pOwner->GetArenaCurrency() < (item_extended_cost->arena_points * amount))
            return INV_ERR_NOT_ENOUGH_ARENA_POINTS;
        if (m_pOwner->GetMaxPersonalRating() < item_extended_cost->personalrating)
            return INV_ERR_PERSONAL_ARENA_RATING_TOO_LOW;
    }

    if (item->BuyPrice)
    {
        int32 price = GetBuyPriceForItem(item, amount, m_pOwner, pVendor) * amount;
        if (!m_pOwner->HasGold(price))
        {
            return INV_ERR_NOT_ENOUGH_MONEY;
        }
    }

    if (item->RequiredFaction)
    {
        DBC::Structures::FactionEntry const* factdbc = sFactionStore.LookupEntry(item->RequiredFaction);
        if (!factdbc || factdbc->RepListId < 0)
            return INV_ERR_OK;

        if (m_pOwner->GetReputationRankFromStanding(m_pOwner->GetStanding(item->RequiredFaction)) < (int32)item->RequiredFactionStanding)
        {
            return INV_ERR_ITEM_REPUTATION_NOT_ENOUGH;
        }
    }
    return INV_ERR_OK;
}

/// Gets the Item slot by item type
int8 ItemInterface::GetItemSlotByType(uint32 type)
{
    switch (type)
    {
        case INVTYPE_NON_EQUIP:
            return ITEM_NO_SLOT_AVAILABLE;
        case INVTYPE_HEAD:
        {
            return EQUIPMENT_SLOT_HEAD;
        }
        case INVTYPE_NECK:
        {
            return EQUIPMENT_SLOT_NECK;
        }
        case INVTYPE_SHOULDERS:
        {
            return EQUIPMENT_SLOT_SHOULDERS;
        }
        case INVTYPE_BODY:
        {
            return EQUIPMENT_SLOT_BODY;
        }
        case INVTYPE_CHEST:
        {
            return EQUIPMENT_SLOT_CHEST;
        }
        case INVTYPE_ROBE: // ???
        {
            return EQUIPMENT_SLOT_CHEST;
        }
        case INVTYPE_WAIST:
        {
            return EQUIPMENT_SLOT_WAIST;
        }
        case INVTYPE_LEGS:
        {
            return EQUIPMENT_SLOT_LEGS;
        }
        case INVTYPE_FEET:
        {
            return EQUIPMENT_SLOT_FEET;
        }
        case INVTYPE_WRISTS:
        {
            return EQUIPMENT_SLOT_WRISTS;
        }
        case INVTYPE_HANDS:
        {
            return EQUIPMENT_SLOT_HANDS;
        }
        case INVTYPE_FINGER:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_FINGER1))
                return EQUIPMENT_SLOT_FINGER1;
            else if (!GetInventoryItem(EQUIPMENT_SLOT_FINGER2))
                return EQUIPMENT_SLOT_FINGER2;
            else
                return EQUIPMENT_SLOT_FINGER1;          //auto equips always in finger 1
        }
        case INVTYPE_TRINKET:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_TRINKET1))
                return EQUIPMENT_SLOT_TRINKET1;
            else if (!GetInventoryItem(EQUIPMENT_SLOT_TRINKET2))
                return EQUIPMENT_SLOT_TRINKET2;
            else
                return EQUIPMENT_SLOT_TRINKET1;         //auto equips always on trinket 1
        }
        case INVTYPE_CLOAK:
        {
            return EQUIPMENT_SLOT_BACK;
        }
        case INVTYPE_WEAPON:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
                return EQUIPMENT_SLOT_MAINHAND;
            else if (!GetInventoryItem(EQUIPMENT_SLOT_OFFHAND))
                return EQUIPMENT_SLOT_OFFHAND;
            else
                return EQUIPMENT_SLOT_MAINHAND;
        }
        case INVTYPE_SHIELD:
        {
            return EQUIPMENT_SLOT_OFFHAND;
        }
        case INVTYPE_RANGED:
        {
            return EQUIPMENT_SLOT_RANGED;
        }
        case INVTYPE_2HWEAPON:
        {
            if (!GetInventoryItem(EQUIPMENT_SLOT_MAINHAND))
                return EQUIPMENT_SLOT_MAINHAND;
            else if (!GetInventoryItem(EQUIPMENT_SLOT_OFFHAND))
                return EQUIPMENT_SLOT_OFFHAND;
            else
                return EQUIPMENT_SLOT_MAINHAND;
        }
        case INVTYPE_TABARD:
        {
            return EQUIPMENT_SLOT_TABARD;
        }
        case INVTYPE_WEAPONMAINHAND:
        {
            return EQUIPMENT_SLOT_MAINHAND;
        }
        case INVTYPE_WEAPONOFFHAND:
        {
            return EQUIPMENT_SLOT_OFFHAND;
        }
        case INVTYPE_HOLDABLE:
        {
            return EQUIPMENT_SLOT_OFFHAND;
        }
        case INVTYPE_THROWN:
            return EQUIPMENT_SLOT_RANGED;   // ?
        case INVTYPE_RANGEDRIGHT:
            return EQUIPMENT_SLOT_RANGED;   // ?
        case INVTYPE_RELIC:
            return EQUIPMENT_SLOT_RANGED;
        case INVTYPE_BAG:
        {
            for (int8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
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
Item* ItemInterface::GetItemByGUID(uint64 Guid)
{
    uint32 i;

    //EQUIPMENT
    for (i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getGuid() == Guid)
            {
                m_result.ContainerSlot = static_cast<int8>(INVALID_BACKPACK_SLOT);//not a containerslot. In 1.8 client marked wrong slot like this
                m_result.Slot = static_cast<int8>(i);
                return m_pItems[i];
            }
        }
    }

    //INVENTORY BAGS
    for (i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr && m_pItems[i]->IsContainer())
        {
            if (m_pItems[i]->getGuid() == Guid)
            {
                m_result.ContainerSlot = static_cast<int8>(INVALID_BACKPACK_SLOT);
                m_result.Slot = static_cast<int8>(i);
                return m_pItems[i];
            }

            for (uint32 j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = (static_cast<Container*>(m_pItems[i]))->GetItem(static_cast<int16>(j));
                if (item2)
                {
                    if (item2->getGuid() == Guid)
                    {
                        m_result.ContainerSlot = static_cast<int8>(i);
                        m_result.Slot = static_cast<int8>(j);
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
                m_result.ContainerSlot = static_cast<int8>(INVALID_BACKPACK_SLOT);
                m_result.Slot = static_cast<int8>(i);
                return m_pItems[i];
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
                m_result.ContainerSlot = static_cast<int8>(INVALID_BACKPACK_SLOT);
                m_result.Slot = static_cast<int8>(i);
                return m_pItems[i];
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
                m_result.ContainerSlot = static_cast<int8>(INVALID_BACKPACK_SLOT);
                m_result.Slot = static_cast<int8>(i);
                return m_pItems[i];
            }
        }
    }
    return nullptr;
}

/// Inventory Error report
void ItemInterface::BuildInventoryChangeError(Item* SrcItem, Item* DstItem, uint8 Error)
{
    WorldPacket data(SMSG_INVENTORY_CHANGE_FAILURE, 22);

    data << uint8(Error);

    if (SrcItem != nullptr)
        data << uint64(SrcItem->getGuid());
    else
        data << uint64(0);

    if (DstItem != nullptr)
        data << uint64(DstItem->getGuid());
    else
        data << uint64(0);

    data << uint8(0);

    if ((Error == INV_ERR_YOU_MUST_REACH_LEVEL_N) || (Error == INV_ERR_PURCHASE_LEVEL_TOO_LOW))
    {
        if (SrcItem)
        {
            data << uint32(SrcItem->getItemProperties()->RequiredLevel);
        }
    }

    m_pOwner->SendPacket(&data);
}

void ItemInterface::EmptyBuyBack()
{
    for (uint32 j = 0; j < MAX_BUYBACK_SLOT; ++j)
    {
        if (m_pBuyBack[j] != nullptr)
        {
            m_pOwner->SendDestroyObject(m_pBuyBack[j]->getGuid());
            m_pBuyBack[j]->DeleteFromDB();

            if (m_pBuyBack[j]->IsContainer())
            {
                if (static_cast<Container*>(m_pBuyBack[j])->IsInWorld())
                    static_cast<Container*>(m_pBuyBack[j])->RemoveFromWorld();

                delete static_cast<Container*>(m_pBuyBack[j]);
            }
            else
            {
                if (m_pBuyBack[j]->IsInWorld())
                    m_pBuyBack[j]->RemoveFromWorld();
                delete m_pBuyBack[j];
                m_pBuyBack[j] = nullptr;
            }

            m_pOwner->setUInt64Value(static_cast<uint16_t>(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2 * j)), 0);
            m_pOwner->setUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_PRICE_1 + j), 0);
            m_pOwner->setUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j), 0);
            m_pBuyBack[j] = nullptr;
        }
        else
            break;
    }
}

void ItemInterface::AddBuyBackItem(Item* it, uint32 price)
{
    if ((m_pBuyBack[MAX_BUYBACK_SLOT - 1] != nullptr) && (m_pOwner->getUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (MAX_BUYBACK_SLOT - 1) * 2) != 0))
    {
        if (m_pBuyBack[0] != nullptr)
        {
            m_pOwner->SendDestroyObject(m_pBuyBack[0]->getGuid());
            m_pBuyBack[0]->DeleteFromDB();

            if (m_pBuyBack[0]->IsContainer())
            {
                if (static_cast<Container*>(m_pBuyBack[0])->IsInWorld())
                    static_cast<Container*>(m_pBuyBack[0])->RemoveFromWorld();

                delete static_cast<Container*>(m_pBuyBack[0]);
            }
            else
            {
                if (m_pBuyBack[0]->IsInWorld())
                    m_pBuyBack[0]->RemoveFromWorld();
                delete m_pBuyBack[0];
            }

            m_pBuyBack[0] = nullptr;
        }

        for (uint8 j = 0; j < MAX_BUYBACK_SLOT - 1; ++j)
        {
            //SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2*j),buyback[j+1]->getGuid());
            m_pOwner->setUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2 * j), m_pOwner->getUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ((j + 1) * 2)));
            m_pOwner->setUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + j, m_pOwner->getUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + j + 1));
            m_pOwner->setUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j, m_pOwner->getUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j + 1));
            m_pBuyBack[j] = m_pBuyBack[j + 1];
        }
        m_pBuyBack[MAX_BUYBACK_SLOT - 1] = it;

        m_pOwner->setUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2 * (MAX_BUYBACK_SLOT - 1)), m_pBuyBack[MAX_BUYBACK_SLOT - 1]->getGuid());
        m_pOwner->setUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + MAX_BUYBACK_SLOT - 1, price);
        m_pOwner->setUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + MAX_BUYBACK_SLOT - 1, (uint32)UNIXTIME);
        return;
    }

    for (uint8 i = 0; i <= (MAX_BUYBACK_SLOT - 1) * 2; i += 2) //at least 1 slot is empty
    {
        if ((m_pOwner->getUInt32Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + i) == 0) || (m_pBuyBack[i / 2] == nullptr))
        {
            LOG_DETAIL("setting buybackslot %u", i / 2);
            m_pBuyBack[i >> 1] = it;

            m_pOwner->setUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + i, m_pBuyBack[i >> 1]->getGuid());
            //SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + i,it->getGuid());
            m_pOwner->setUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + (i >> 1), price);
            m_pOwner->setUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + (i >> 1), (uint32)UNIXTIME);
            return;
        }
    }
}

void ItemInterface::RemoveBuyBackItem(uint32 index)
{
    uint32_t j = 0;
    for (j = index; j < MAX_BUYBACK_SLOT - 1; ++j)
    {
        if (m_pOwner->getUInt64Value(static_cast<uint16_t>(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (j * 2))) != 0)
        {
            m_pOwner->setUInt64Value(static_cast<uint16_t>(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2 * j)), m_pOwner->getUInt64Value(static_cast<uint16_t>(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ((j + 1) * 2))));
            m_pOwner->setUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_PRICE_1 + j), m_pOwner->getUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_PRICE_1 + j + 1)));
            m_pOwner->setUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j), m_pOwner->getUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j + 1)));

            if (m_pBuyBack[j + 1] != nullptr && (m_pOwner->getUInt64Value(static_cast<uint16_t>(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ((j + 1) * 2))) != 0))
            {
                m_pBuyBack[j] = m_pBuyBack[j + 1];
            }
            else
            {
                m_pBuyBack[j] = nullptr;

                LOG_DETAIL("nulling %u", j);
            }
        }
        else
            return;
    }

    j = 11;
    m_pOwner->setUInt64Value(static_cast<uint16_t>(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (2 * j)), m_pOwner->getUInt64Value(static_cast<uint16_t>(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + ((j + 1) * 2))));
    m_pOwner->setUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_PRICE_1 + j), m_pOwner->getUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_PRICE_1 + j + 1)));
    m_pOwner->setUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j), m_pOwner->getUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + j + 1)));

    if (m_pBuyBack[MAX_BUYBACK_SLOT - 1])
    {
        m_pBuyBack[MAX_BUYBACK_SLOT - 1] = nullptr;
    }

}

/// Swap inventory slots
void ItemInterface::SwapItemSlots(int8 srcslot, int8 dstslot)
{
    // srcslot and dstslot are int... NULL might not be an int depending on arch where it is compiled
    if (srcslot >= INVENTORY_KEYRING_END || srcslot < 0)
        return;

    if (dstslot >= INVENTORY_KEYRING_END || dstslot < 0)
        return;

    Item* SrcItem = GetInventoryItem(srcslot);
    Item* DstItem = GetInventoryItem(dstslot);

    LOG_DEBUG("ItemInterface::SwapItemSlots(%u, %u);", srcslot, dstslot);
    //Item * temp = GetInventoryItem(srcslot);
    //if (temp)
    //    LOG_DEBUG("Source item: %s (inventoryType=%u, realslot=%u);" , temp->GetProto()->Name1 , temp->GetProto()->InventoryType , GetItemSlotByType(temp->GetProto()->InventoryType));
    //    temp = GetInventoryItem(dstslot);
    //if (temp)
    //    LOG_DEBUG("Destination: Item: %s (inventoryType=%u, realslot=%u);" , temp->GetProto()->Name1 , temp->GetProto()->InventoryType , GetItemSlotByType(temp->GetProto()->InventoryType));
    //else
    //    LOG_DEBUG("Destination: Empty");

    // don't stack equipped items (even with ItemStackCheat), just swap them
    uint32 srcItemMaxStack, dstItemMaxStack;
    if (SrcItem != nullptr)
    {
        if (srcslot < INVENTORY_SLOT_BAG_END || !(SrcItem->getOwner()->ItemStackCheat))
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
        if (dstslot < INVENTORY_SLOT_BAG_END || !(DstItem->getOwner()->ItemStackCheat))
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

    if (SrcItem != nullptr && DstItem != nullptr && SrcItem->GetEntry() == DstItem->GetEntry() && srcItemMaxStack > 1 && SrcItem->wrapped_item_id == 0 && DstItem->wrapped_item_id == 0)
    {
        uint32 total = SrcItem->GetStackCount() + DstItem->GetStackCount();
        if (total <= dstItemMaxStack)
        {
            DstItem->ModStackCount(SrcItem->GetStackCount());
            SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, srcslot);
            DstItem->m_isDirty = true;
            return;
        }
        else
        {
            if (DstItem->GetStackCount() == dstItemMaxStack)
            {

            }
            else
            {
                int32 delta = dstItemMaxStack - DstItem->GetStackCount();
                DstItem->setStackCount(dstItemMaxStack);
                SrcItem->ModStackCount(-delta);
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
            m_pOwner->ApplyItemMods(m_pItems[(int)srcslot], srcslot, false);
    }

    //dst item was equipped previously
    if (dstslot < INVENTORY_SLOT_BAG_END)
    {
        if (m_pItems[(int)dstslot] != nullptr)
            m_pOwner->ApplyItemMods(m_pItems[(int)dstslot], dstslot, false);
    }

    //LOG_DEBUG("Putting items into slots...");



    m_pItems[(int)dstslot] = SrcItem;

    // Moving a bag with items to a empty bagslot
    if (DstItem == nullptr && SrcItem != nullptr && SrcItem->IsContainer())
    {
        Item* tSrcItem = nullptr;

        for (uint32 Slot = 0; Slot < SrcItem->getItemProperties()->ContainerSlots; ++Slot)
        {
            tSrcItem = (static_cast<Container*>((m_pItems[(int)srcslot])))->GetItem(static_cast<int16>(Slot));

            m_pOwner->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srcslot, static_cast<int16>(Slot), false);

            if (tSrcItem != nullptr)
            {
                auto result = m_pOwner->GetItemInterface()->SafeAddItem(tSrcItem, dstslot, static_cast<int16>(Slot));
                if (!result)
                {
                    LOG_ERROR("Error while adding item %u to player %s", tSrcItem->GetEntry(), m_pOwner->GetNameString());
                    return;
                }
            }
        }
    }

    m_pItems[(int)srcslot] = DstItem;

    // swapping 2 bags filled with items
    if (DstItem != nullptr && SrcItem != nullptr && SrcItem->IsContainer() && DstItem->IsContainer())
    {
        Item* tDstItem = nullptr;
        Item* tSrcItem = nullptr;
        uint32 TotalSlots = 0;

        // Determine the max amount of slots to swap
        if (SrcItem->getItemProperties()->ContainerSlots > DstItem->getItemProperties()->ContainerSlots)
            TotalSlots = SrcItem->getItemProperties()->ContainerSlots;
        else
            TotalSlots = DstItem->getItemProperties()->ContainerSlots;

        // swap items in the bags
        for (uint32 Slot = 0; Slot < TotalSlots; ++Slot)
        {
            tSrcItem = (static_cast<Container*>((m_pItems[(int)srcslot])))->GetItem(static_cast<int16>(Slot));
            tDstItem = (static_cast<Container*>((m_pItems[(int)dstslot])))->GetItem(static_cast<int16>(Slot));

            if (tSrcItem != nullptr)
                m_pOwner->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(srcslot, static_cast<int16>(Slot), false);
            if (tDstItem != nullptr)
                m_pOwner->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(dstslot, static_cast<int16>(Slot), false);

            if (tSrcItem != nullptr)
                (static_cast<Container*>(DstItem))->AddItem(static_cast<int16>(Slot), tSrcItem);
            if (tDstItem != nullptr)
                (static_cast<Container*>(SrcItem))->AddItem(static_cast<int16>(Slot), tDstItem);
        }
    }

    if (DstItem != nullptr)
        DstItem->m_isDirty = true;
    if (SrcItem != nullptr)
        SrcItem->m_isDirty = true;

    if (m_pItems[(int)dstslot] != nullptr)
    {
        //LOG_DEBUG("(SrcItem) PLAYER_FIELD_INV_SLOT_HEAD + %u is now %u" , dstslot * 2 , m_pItems[(int)dstslot]->getGuid());
        m_pOwner->SetInventorySlot(dstslot, m_pItems[(int)dstslot]->getGuid());
    }
    else
    {
        //LOG_DEBUG("(SrcItem) PLAYER_FIELD_INV_SLOT_HEAD + %u is now 0" , dstslot * 2);
        m_pOwner->SetInventorySlot(dstslot, 0);
    }

    if (m_pItems[(int)srcslot] != nullptr)
    {
        //LOG_DEBUG("(DstItem) PLAYER_FIELD_INV_SLOT_HEAD + %u is now %u" , dstslot * 2 , m_pItems[(int)srcslot]->getGuid());
        m_pOwner->SetInventorySlot(srcslot, m_pItems[(int)srcslot]->getGuid());
    }
    else
    {
        //LOG_DEBUG("(DstItem) PLAYER_FIELD_INV_SLOT_HEAD + %u is now 0" , dstslot * 2);
        m_pOwner->SetInventorySlot(srcslot, 0);
    }

#if VERSION_STRING > TBC
    if (srcslot < INVENTORY_SLOT_BAG_END)    // source item is equipped
    {
        if (m_pItems[(int)srcslot])   // dstitem goes into here.
        {
            // Bags aren't considered "visible".
            if (srcslot < EQUIPMENT_SLOT_END)
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_ENTRYID + (srcslot * 2);
                m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase), m_pItems[(int)srcslot]->GetEntry());
                m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase + 1), m_pItems[(int)srcslot]->GetEnchantmentId(0));
            }

            // handle bind on equip
            if (m_pItems[(int)srcslot]->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
                m_pItems[(int)srcslot]->SoulBind();
        }
        else
        {
            // Bags aren't considered "visible".
            if (srcslot < EQUIPMENT_SLOT_END)
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_ENTRYID + (srcslot * 2);
                m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase), 0);
                m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase + 1), 0);
                /*                m_pOwner->SetUInt32Value(VisibleBase + 2, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 3, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 4, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 5, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 6, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 7, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 8, 0);*/
            }
        }
    }
#else
    if (srcslot < INVENTORY_SLOT_BAG_END)	// source item is equiped
    {
        if (m_pItems[(int)srcslot]) // dstitem goes into here.
        {
            // Bags aren't considered "visible".
            if (srcslot < EQUIPMENT_SLOT_END)
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (srcslot * 16);
                m_pOwner->setUInt32Value(VisibleBase, m_pItems[(int)srcslot]->GetEntry());
                m_pOwner->setUInt32Value(VisibleBase + 1, m_pItems[(int)srcslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT));
                m_pOwner->setUInt32Value(VisibleBase + 2, m_pItems[(int)srcslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 3));
                m_pOwner->setUInt32Value(VisibleBase + 3, m_pItems[(int)srcslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 6));
                m_pOwner->setUInt32Value(VisibleBase + 4, m_pItems[(int)srcslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 9));
                m_pOwner->setUInt32Value(VisibleBase + 5, m_pItems[(int)srcslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 12));
                m_pOwner->setUInt32Value(VisibleBase + 6, m_pItems[(int)srcslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 15));
                m_pOwner->setUInt32Value(VisibleBase + 7, m_pItems[(int)srcslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 18));
                m_pOwner->setInt32Value(VisibleBase + 8, m_pItems[(int)srcslot]->getInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));
            }

            // handle bind on equip
            if (m_pItems[(int)srcslot]->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
                m_pItems[(int)srcslot]->SoulBind();
        }
        else
        {
            // Bags aren't considered "visible".
            if (srcslot < EQUIPMENT_SLOT_END)
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (srcslot * 16);
                m_pOwner->setUInt32Value(VisibleBase, 0);
                m_pOwner->setUInt32Value(VisibleBase + 1, 0);
                m_pOwner->setUInt32Value(VisibleBase + 2, 0);
                m_pOwner->setUInt32Value(VisibleBase + 3, 0);
                m_pOwner->setUInt32Value(VisibleBase + 4, 0);
                m_pOwner->setUInt32Value(VisibleBase + 5, 0);
                m_pOwner->setUInt32Value(VisibleBase + 6, 0);
                m_pOwner->setUInt32Value(VisibleBase + 7, 0);
                m_pOwner->setUInt32Value(VisibleBase + 8, 0);
            }
        }
    }
#endif

#if VERSION_STRING > TBC
    if (dstslot < INVENTORY_SLOT_BAG_END)     // source item is inside inventory
    {
        if (m_pItems[(int)dstslot] != nullptr)   // srcitem goes into here.
        {
            // Bags aren't considered "visible".
            if (dstslot < EQUIPMENT_SLOT_END)
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_ENTRYID + (dstslot * 2);
                m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase), m_pItems[(int)dstslot]->GetEntry());
                m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase + 1), m_pItems[(int)dstslot]->GetEnchantmentId(0));
            }

            // handle bind on equip
            if (m_pItems[(int)dstslot]->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
                m_pItems[(int)dstslot]->SoulBind();

        }
        else
        {

            // bags aren't considered visible
            if (dstslot < EQUIPMENT_SLOT_END)
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_ENTRYID + (dstslot * 2);
                m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase), 0);
                m_pOwner->setUInt32Value(static_cast<uint16_t>(VisibleBase + 1), 0);
                /*                m_pOwner->SetUInt32Value(VisibleBase + 2, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 3, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 4, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 5, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 6, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 7, 0);
                                m_pOwner->SetUInt32Value(VisibleBase + 8, 0);*/
            }
        }
    }
#else
    if (dstslot < INVENTORY_SLOT_BAG_END)   // source item is inside inventory
    {
        if (m_pItems[(int)dstslot] != nullptr) // srcitem goes into here.
        {
            // Bags aren't considered "visible".
            if (dstslot < EQUIPMENT_SLOT_END)
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (dstslot * 16);
                m_pOwner->setUInt32Value(VisibleBase, m_pItems[(int)dstslot]->GetEntry());
                m_pOwner->setUInt32Value(VisibleBase + 1, m_pItems[(int)dstslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT));
                m_pOwner->setUInt32Value(VisibleBase + 2, m_pItems[(int)dstslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 3));
                m_pOwner->setUInt32Value(VisibleBase + 3, m_pItems[(int)dstslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 6));
                m_pOwner->setUInt32Value(VisibleBase + 4, m_pItems[(int)dstslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 9));
                m_pOwner->setUInt32Value(VisibleBase + 5, m_pItems[(int)dstslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 12));
                m_pOwner->setUInt32Value(VisibleBase + 6, m_pItems[(int)dstslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 15));
                m_pOwner->setUInt32Value(VisibleBase + 7, m_pItems[(int)dstslot]->getUInt32Value(ITEM_FIELD_ENCHANTMENT + 18));
                m_pOwner->setInt32Value(VisibleBase + 8, m_pItems[(int)dstslot]->getInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));
            }

            // handle bind on equip
            if (m_pItems[(int)dstslot]->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
                m_pItems[(int)dstslot]->SoulBind();

        }
        else
        {

            // bags aren't considered visible
            if (dstslot < EQUIPMENT_SLOT_END)
            {
                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (dstslot * 16);
                m_pOwner->setUInt32Value(VisibleBase, 0);
                m_pOwner->setUInt32Value(VisibleBase + 1, 0);
                m_pOwner->setUInt32Value(VisibleBase + 2, 0);
                m_pOwner->setUInt32Value(VisibleBase + 3, 0);
                m_pOwner->setUInt32Value(VisibleBase + 4, 0);
                m_pOwner->setUInt32Value(VisibleBase + 5, 0);
                m_pOwner->setUInt32Value(VisibleBase + 6, 0);
                m_pOwner->setUInt32Value(VisibleBase + 7, 0);
                m_pOwner->setUInt32Value(VisibleBase + 8, 0);
            }
        }
    }
#endif
    // handle dual wield
    if (dstslot == EQUIPMENT_SLOT_OFFHAND || srcslot == EQUIPMENT_SLOT_OFFHAND)
    {
        if (m_pItems[EQUIPMENT_SLOT_OFFHAND] != nullptr && m_pItems[EQUIPMENT_SLOT_OFFHAND]->getItemProperties()->Class == ITEM_CLASS_WEAPON)
        {
            m_pOwner->SetDualWield(true);

            /////////////////////////////////////////// Titan's grip stuff ////////////////////////////////////////////////////////////
            uint32 subclass = m_pItems[EQUIPMENT_SLOT_OFFHAND]->getItemProperties()->SubClass;
            if (subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_AXE || subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_MACE || subclass == ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD)
            {
                m_pOwner->CastSpell(m_pOwner, 49152, true);
            }
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
        else
            m_pOwner->SetDualWield(false);
    }

    //src item is equipped now
    if (srcslot < INVENTORY_SLOT_BAG_END)
    {
        if (m_pItems[(int)srcslot] != nullptr)
            m_pOwner->ApplyItemMods(m_pItems[(int)srcslot], srcslot, true);
        else if (srcslot == EQUIPMENT_SLOT_MAINHAND || srcslot == EQUIPMENT_SLOT_OFFHAND)
            m_pOwner->CalcDamage();
    }

    //dst item is equipped now
    if (dstslot < INVENTORY_SLOT_BAG_END)
    {
        if (m_pItems[(int)dstslot] != nullptr)
            m_pOwner->ApplyItemMods(m_pItems[(int)dstslot], dstslot, true);
        else if (dstslot == EQUIPMENT_SLOT_MAINHAND || dstslot == EQUIPMENT_SLOT_OFFHAND)
            m_pOwner->CalcDamage();
    }

    //Recalculate Expertise (for Weapon specs)
    m_pOwner->CalcExpertise();
}

/// Item Loading
void ItemInterface::mLoadItemsFromDatabase(QueryResult* result)
{
    int8 containerslot, slot;
    Item* item;

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            containerslot = fields[13].GetInt8();
            slot = fields[14].GetInt8();

            ItemProperties const* proto = sMySQLStore.getItemProperties(fields[2].GetUInt32());
            if (proto != nullptr)
            {
                if (proto->InventoryType == INVTYPE_BAG)
                {
                    item = new Container(HIGHGUID_TYPE_CONTAINER, fields[1].GetUInt32());
                    static_cast<Container*>(item)->LoadFromDB(fields);

                }
                else
                {
                    item = new Item;
                    item->init(HIGHGUID_TYPE_ITEM, fields[1].GetUInt32());
                    item->LoadFromDB(fields, m_pOwner, false);

                }

                // if we encounter an item that expired, we remove it from db
                if (item->GetItemExpireTime() > 0 && UNIXTIME > item->GetItemExpireTime())
                {
                    item->DeleteFromDB();
                    item->DeleteMe();
                    continue;
                }

                if (SafeAddItem(item, containerslot, slot))
                    item->m_isDirty = false;
                else
                {
                    delete item;
                    item = nullptr;
                }
            }
        } while (result->NextRow());
    }
}

/// Item saving
void ItemInterface::mSaveItemsToDatabase(bool first, QueryBuffer* buf)
{
    int16 x;

    for (x = EQUIPMENT_SLOT_START; x < CURRENCYTOKEN_SLOT_END; ++x)
    {
        if (GetInventoryItem(x) != nullptr)
        {
            if (IsBagSlot(x) && GetInventoryItem(x)->IsContainer())
            {
                static_cast<Container*>(GetInventoryItem(x))->SaveBagToDB(static_cast<int8>(x), first, buf);
            }
            else
            {
                GetInventoryItem(x)->SaveToDB(INVENTORY_SLOT_NOT_SET, static_cast<int8>(x), first, buf);
            }
        }
    }
}

AddItemResult ItemInterface::AddItemToFreeBankSlot(Item* item)
{
    //special items first
    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getItemProperties()->BagFamily & item->getItemProperties()->BagFamily)
            {
                if (m_pItems[i]->IsContainer())
                {
                    bool result = static_cast<Container*>(m_pItems[i])->AddItemToFreeSlot(item, NULL);
                    if (result)
                        return ADD_ITEM_RESULT_OK;
                }
            }
        }
    }

    for (int16 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        if (m_pItems[i] == nullptr)
        {
            return SafeAddItem(item, INVENTORY_SLOT_NOT_SET, i);
        }
    }

    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr && m_pItems[i]->getItemProperties()->BagFamily == 0 && m_pItems[i]->IsContainer())   //special bags ignored
        {
            for (uint32 j = 0; j < m_pItems[i]->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(m_pItems[i])->GetItem(static_cast<int16>(j));
                if (item2 == nullptr)
                {
                    return SafeAddItem(item, static_cast<int8>(i), static_cast<int16>(j));
                }
            }
        }
    }
    return ADD_ITEM_RESULT_ERROR;
}

int8 ItemInterface::FindSpecialBag(Item* item)
{
    for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_pItems[i] != nullptr)
        {
            if (m_pItems[i]->getItemProperties()->BagFamily & item->getItemProperties()->BagFamily)
            {
                return i;
            }
        }
    }
    return ITEM_NO_SLOT_AVAILABLE;
}

int8 ItemInterface::FindFreeKeyringSlot()
{
    for (uint8 i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        if (m_pItems[i] == nullptr)
        {
            return i;
        }
    }
    return ITEM_NO_SLOT_AVAILABLE;
}

int16 ItemInterface::FindFreeCurrencySlot()
{
    for (uint16 i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        if (m_pItems[i] == nullptr)
        {
            return i;
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
        //LOG_DEBUG("ItemInterface::FindFreeInventorySlot called for item %s" , proto->Name1);
        if (proto->BagFamily)
        {
            if (proto->BagFamily & ITEM_TYPE_KEYRING || proto->Class == ITEM_CLASS_KEY)
            {
                for (uint32 i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
                {
                    if (m_pItems[i] == nullptr)
                    {
                        m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
                        m_result.Slot = static_cast<int8>(i);
                        m_result.Result = true;
                        return m_result;
                    }
                }
            }
            else if (proto->BagFamily & ITEM_TYPE_CURRENCY)
            {
                for (uint32 i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
                {
                    if (m_pItems[i] == nullptr)
                    {
                        m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
                        m_result.Slot = static_cast<int8>(i);
                        m_result.Result = true;
                        return m_result;
                    }
                }
            }
            else
            {
                for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                {
                    if (m_pItems[i] != nullptr && m_pItems[i]->IsContainer())
                    {
                        if (m_pItems[i]->getItemProperties()->BagFamily & proto->BagFamily)
                        {
                            int32 slot = static_cast<Container*>(m_pItems[i])->FindFreeSlot();
                            if (slot != ITEM_NO_SLOT_AVAILABLE)
                            {
                                m_result.ContainerSlot = static_cast<int8>(i);
                                m_result.Slot = static_cast<int8>(slot);
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
    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item == nullptr)
        {
            m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            m_result.Slot = static_cast<int8>(i);
            m_result.Result = true;
            return m_result;
        }
    }

    //bags
    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item != nullptr)
        {
            if (item->IsContainer() && !item->getItemProperties()->BagFamily)
            {
                int32 slot = static_cast<Container*>(m_pItems[i])->FindFreeSlot();
                if (slot != ITEM_NO_SLOT_AVAILABLE)
                {
                    m_result.ContainerSlot = static_cast<int8>(i);
                    m_result.Slot = static_cast<int8>(slot);
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
            for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
            {
                if (m_pItems[i] != nullptr && m_pItems[i]->IsContainer())
                {
                    if (m_pItems[i]->getItemProperties()->BagFamily & proto->BagFamily)
                    {
                        int32 slot = static_cast<Container*>(m_pItems[i])->FindFreeSlot();
                        if (slot != ITEM_NO_SLOT_AVAILABLE)
                        {
                            m_result.ContainerSlot = static_cast<int8>(i);
                            m_result.Slot = static_cast<int8>(slot);
                            m_result.Result = true;
                            return m_result;
                        }
                    }
                }
            }
        }
    }

    //backpack
    for (uint32 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item == nullptr)
        {
            m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            m_result.Slot = static_cast<int8>(i);
            m_result.Result = true;
            return m_result;
        }
    }

    //bags
    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item != nullptr)
        {
            if (item->IsContainer() && !item->getItemProperties()->BagFamily)
            {
                int32 slot = static_cast<Container*>(m_pItems[i])->FindFreeSlot();
                if (slot != ITEM_NO_SLOT_AVAILABLE)
                {
                    m_result.ContainerSlot = static_cast<int8>(i);
                    m_result.Slot = static_cast<int8>(slot);
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
    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (m_pItems[i] != nullptr && m_pItems[i]->IsAmmoBag())
        {
            m_result.ContainerSlot = ITEM_NO_SLOT_AVAILABLE;
            m_result.Slot = static_cast<int8>(i);
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
    uint32 f = Util::getRandomUInt(100);
    if (f <= 10)   //10% chance to loose 1 dur from a random valid item.
    {
        int32 slot = Util::getRandomUInt(EQUIPMENT_SLOT_END);
        Item* pItem = GetInventoryItem(INVENTORY_SLOT_NOT_SET, static_cast<int16>(slot));
        if (pItem != nullptr)
        {
            if (pItem->GetDurability() && pItem->GetDurabilityMax())
            {
                pItem->SetDurability(pItem->GetDurabilityMax() - 1);
                pItem->m_isDirty = true;
                //check final durability
                if (!pItem->GetDurability())   //no dur left
                {
                    m_pOwner->ApplyItemMods(pItem, static_cast<int16>(slot), false, true);

                }
            }
        }
    }
}

bool ItemInterface::IsEquipped(uint32 itemid)
{
    for (uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        Item* it = m_pItems[x];

        if (it != nullptr)
        {
            if (it->getItemProperties()->ItemId == itemid)
                return true;

            // check gems as well
            for (uint32 count = 0; count < it->GetSocketsCount(); count++)
            {
                EnchantmentInstance* ei = it->GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + count);

                if (ei && ei->Enchantment)
                {
                    ItemProperties const* ip = sMySQLStore.getItemProperties(ei->Enchantment->GemEntry);
                    if (ip && ip->ItemId == itemid)
                        return true;
                }
            }
        }
    }
    return false;
}

void ItemInterface::CheckAreaItems()
{
    for (uint32 x = EQUIPMENT_SLOT_START; x < INVENTORY_SLOT_ITEM_END; ++x)
    {
        if (m_pItems[x] != nullptr)
        {
            if (IsBagSlot(static_cast<int16>(x)) && m_pItems[x]->IsContainer())
            {
                Container* bag = static_cast<Container*>(m_pItems[x]);

                for (uint32 i = 0; i < bag->getItemProperties()->ContainerSlots; ++i)
                {
                    if (bag->GetItem(static_cast<int16>(i)) != nullptr && bag->GetItem(static_cast<int16>(i))->getItemProperties()->MapID && bag->GetItem(static_cast<int16>(i))->getItemProperties()->MapID != GetOwner()->GetMapId())
                        bag->SafeFullRemoveItemFromSlot(static_cast<int16>(i));
                }
            }
            else
            {
                if (m_pItems[x]->getItemProperties()->MapID && m_pItems[x]->getItemProperties()->MapID != GetOwner()->GetMapId())
                    SafeFullRemoveItemFromSlot(INVENTORY_SLOT_NOT_SET, static_cast<int16>(x));
            }
        }
    }
}

uint32 ItemInterface::GetEquippedCountByItemLimit(uint32 LimitId)
{
    uint32 count = 0;
    for (uint32 x = EQUIPMENT_SLOT_START; x < EQUIPMENT_SLOT_END; ++x)
    {
        Item* it = m_pItems[x];

        if (it != nullptr)
        {
            for (uint32 socketcount = 0; socketcount < it->GetSocketsCount(); ++socketcount)
            {
                EnchantmentInstance* ei = it->GetEnchantment(SOCK_ENCHANTMENT_SLOT1 + socketcount);
                if (ei && ei->Enchantment)
                {
                    ItemProperties const* ip = sMySQLStore.getItemProperties(ei->Enchantment->GemEntry);
                    if (ip && ip->ItemLimitCategory == LimitId)
                        count++;
                }
            }
        }
    }
    return count;
}

uint32 ItemInterface::GetItemCountByLimitId(uint32 LimitId, bool IncBank)
{
    uint32 cnt = 0;

    for (uint8_t i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item != nullptr)
        {
            if (item->getItemProperties()->ItemLimitCategory == LimitId
                && item->wrapped_item_id == 0)
            {
                cnt += item->GetStackCount() ? item->GetStackCount() : 1;
            }
        }
    }

    for (uint8_t i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item && item->IsContainer())
        {
            for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = (static_cast<Container*>(item))->GetItem(static_cast<int16>(j));
                if (item2 != nullptr)
                {
                    if (item2->getItemProperties()->ItemLimitCategory == LimitId
                        && item2->wrapped_item_id == 0)
                    {
                        cnt += item2->GetStackCount() ? item2->GetStackCount() : 1;
                    }
                }
            }
        }
    }

    for (uint8_t i = INVENTORY_KEYRING_START; i < INVENTORY_KEYRING_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item != nullptr)
        {
            if (item->getItemProperties()->ItemLimitCategory == LimitId
                && item->wrapped_item_id == 0)
            {
                cnt += item->GetStackCount() ? item->GetStackCount() : 1;
            }
        }
    }

    for (uint8_t i = CURRENCYTOKEN_SLOT_START; i < CURRENCYTOKEN_SLOT_END; ++i)
    {
        Item* item = GetInventoryItem(static_cast<int16>(i));
        if (item != nullptr)
        {
            if (item->getItemProperties()->ItemLimitCategory == LimitId
                && item->wrapped_item_id == 0)
            {
                cnt += item->GetStackCount() ? item->GetStackCount() : 1;
            }
        }
    }

    if (IncBank)
    {
        for (uint8_t i = BANK_SLOT_ITEM_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16>(i));
            if (item != nullptr)
            {
                if (item->getItemProperties()->ItemLimitCategory == LimitId
                    && item->wrapped_item_id == 0)
                {
                    cnt += item->GetStackCount() ? item->GetStackCount() : 1;
                }
            }
        }

        for (uint8_t i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Item* item = GetInventoryItem(static_cast<int16>(i));
            if (item != nullptr)
            {
                if (item->IsContainer())
                {
                    for (uint32 j = 0; j < item->getItemProperties()->ContainerSlots; ++j)
                    {
                        Item* item2 = (static_cast<Container*>(item))->GetItem(static_cast<int16>(j));
                        if (item2 != nullptr)
                        {
                            if (item2->getItemProperties()->ItemLimitCategory == LimitId
                                && item2->wrapped_item_id == 0)
                            {
                                cnt += item2->GetStackCount() ? item2->GetStackCount() : 1;
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
        Item* item1 = this->GetInventoryItem(i);
        Item* realitem = nullptr;

        if (item1 != nullptr && item1->IsContainer())
        {

            for (uint32 j = 0; j < item1->getItemProperties()->ContainerSlots; ++j)
            {
                Item* item2 = static_cast<Container*>(item1)->GetItem(static_cast<int16>(j));

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
            sEventMgr.AddEvent(realitem, &Item::SendDurationUpdate, EVENT_SEND_PACKET_TO_PLAYER_AFTER_LOGIN, 0, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

/// Inserts a new entry into the RefundableMap. This should be called when purchasing the item
void ItemInterface::AddRefundable(uint64 GUID, uint32 extendedcost)
{
    std::pair< time_t, uint32 > RefundableEntry;
    std::pair< uint64, std::pair< time_t, uint32 > > insertpair;

    Item* item = this->GetItemByGUID(GUID);
    if (item == nullptr)
        return;

    uint32* played = this->GetOwner()->GetPlayedtime();

    RefundableEntry.first = played[1];               // time of purchase in playedtime
    RefundableEntry.second = extendedcost;          // extendedcost

    insertpair.first = GUID;
    insertpair.second = RefundableEntry;

    this->m_refundableitems.insert(insertpair);

    sEventMgr.AddEvent(item, &Item::RemoveFromRefundableMap, EVENT_REMOVE_ITEM_FROM_REFUNDABLE_MAP, (UNIXTIME + 60 * 60 * 2), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void ItemInterface::AddRefundable(uint64 GUID, uint32 extendedcost, time_t buytime)
{
    std::pair< time_t, uint32 > RefundableEntry;
    std::pair< uint64, std::pair< time_t, uint32 > > insertpair;

    Item* item = this->GetItemByGUID(GUID);
    if (item == nullptr)
        return;

    RefundableEntry.first = buytime;               // time of purchase in playedtime
    RefundableEntry.second = extendedcost;      // extendedcost

    insertpair.first = GUID;
    insertpair.second = RefundableEntry;

    this->m_refundableitems.insert(insertpair);

    sEventMgr.AddEvent(item, &Item::RemoveFromRefundableMap, EVENT_REMOVE_ITEM_FROM_REFUNDABLE_MAP, buytime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void ItemInterface::AddRefundable(Item* item, uint32 extendedcost, time_t buytime)
{
    std::pair< time_t, uint32 > RefundableEntry;
    std::pair< uint64, std::pair< time_t, uint32 > > insertpair;

    if (item == nullptr)
        return;

    RefundableEntry.first = buytime;      // time of purchase in playedtime
    RefundableEntry.second = extendedcost; // extendedcost

    insertpair.first = item->getGuid();
    insertpair.second = RefundableEntry;

    this->m_refundableitems.insert(insertpair);

    sEventMgr.AddEvent(item, &Item::RemoveFromRefundableMap, EVENT_REMOVE_ITEM_FROM_REFUNDABLE_MAP, (buytime + 60 * 60 * 2), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

/// Removes an entry from the RefundableMap
void ItemInterface::RemoveRefundable(uint64 GUID)
{
    this->m_refundableitems.erase(GUID);
}

/// Looks up an item in the RefundableMap, and returns the data
std::pair< time_t, uint32 > ItemInterface::LookupRefundable(uint64 GUID)
{
    std::pair< time_t, uint32 > RefundableEntry;
    RefundableMap::iterator itr;

    RefundableEntry.first = 0;          // time of purchase in playedtime
    RefundableEntry.second = 0;         // extendedcost

    itr = this->m_refundableitems.find(GUID);
    if (itr != this->m_refundableitems.end())
    {
        RefundableEntry.first = itr->second.first;
        RefundableEntry.second = itr->second.second;
    }

    return RefundableEntry;
}

bool ItemInterface::AddItemById(uint32 itemid, uint32 count, int32 randomprop)
{
    if (count == 0)
        return false;

    Player* chr = GetOwner();

    ARCEMU_ASSERT(chr != nullptr);

    ItemProperties const* it = sMySQLStore.getItemProperties(itemid);
    if (it == nullptr)
        return false;

    uint8 error = CanReceiveItem(it, count);
    if (error != 0)
    {
        return false;
    }

    uint32 maxStack = chr->ItemStackCheat ? 0x7fffffff : it->MaxCount;
    uint32 toadd;
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
                free_stack_item->ModStackCount(count);
                free_stack_item->m_isDirty = true;

                sQuestMgr.OnPlayerItemPickup(m_pOwner, free_stack_item);

                return true;
            }
        }

        // create new item
        Item* item = objmgr.CreateItem(itemid, chr);
        if (item == nullptr)
            return false;

        if (it->Bonding == ITEM_BIND_ON_PICKUP)
        {
            if (it->Flags & ITEM_FLAG_ACCOUNTBOUND)   // don't "Soulbind" account-bound items
                item->AccountBind();
            else
                item->SoulBind();
        }

        // Let's try to autogenerate randomprop / randomsuffix
        if (randomprop == 0)
        {

            if ((it->RandomPropId != 0) && (it->RandomSuffixId != 0))
            {
                LOG_ERROR("Item %u (%s) has both RandomPropId and RandomSuffixId.", itemid, it->Name.c_str());
            }

            if (it->RandomPropId != 0)
            {
                auto item_random_properties = lootmgr.GetRandomProperties(it);

                if (item_random_properties != nullptr)
                {
                    randomprop = item_random_properties->ID;
                }
                else
                {
                    LOG_ERROR("Item %u (%s) has unknown RandomPropId %u", itemid, it->Name.c_str(), it->RandomPropId);
                }
            }

            if (it->RandomSuffixId != 0)
            {
                auto item_random_suffix = lootmgr.GetRandomSuffix(it);

                if (item_random_suffix != nullptr)
                {
                    randomprop = -1 * item_random_suffix->id;
                }
                else
                {
                    LOG_ERROR("Item %u (%s) has unknown RandomSuffixId %u", itemid, it->Name.c_str(), it->RandomSuffixId);
                }
            }
        }

        if (randomprop != 0)
        {
            if (randomprop < 0)
                item->SetRandomSuffix(-randomprop);
            else
                item->SetItemRandomPropertyId(randomprop);

            item->ApplyRandomProperties(false);
        }

        if (maxStack != 0)
        {
            toadd = count > maxStack ? maxStack : count;
        }
        else
        {
            toadd = count;
        }

        item->setStackCount(toadd);

        AddItemResult res = AddItemToFreeSlot(item);
        if (res != ADD_ITEM_RESULT_ERROR)
        {
            SlotResult* lr = LastSearchResult();

            chr->SendItemPushResult(false, true, false, true, lr->ContainerSlot, lr->Slot, toadd, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
#if VERSION_STRING > TBC
            chr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, itemid, 1, 0);
#endif
            sQuestMgr.OnPlayerItemPickup(m_pOwner, item);
            count -= toadd;
        }
        else
        {
            freeslots = false;
            chr->GetSession()->SendNotification("No free slots were found in your inventory!");
            item->DeleteMe();
        }
    }
    return true;
}


bool ItemInterface::SwapItems(int8 DstInvSlot, int8 DstSlot, int8 SrcInvSlot, int8 SrcSlot)
{
    Item* SrcItem = nullptr;
    Item* DstItem = nullptr;
    bool adderror = false;
    int8 error;

    if (DstInvSlot == SrcSlot && SrcInvSlot == -1)   // player trying to add self container to self container slots
    {
        BuildInventoryChangeError(nullptr, nullptr, INV_ERR_ITEMS_CANT_BE_SWAPPED);
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
            if (DstItem->IsContainer())
            {
                if (static_cast<Container*>(DstItem)->HasItems())
                {
                    if (!IsBagSlot(SrcSlot))
                    {
                        BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                        return false;
                    }
                }
            }

            if (SrcSlot < INVENTORY_KEYRING_END)
            {
                if ((error = CanEquipItemInSlot2(SrcInvSlot, SrcSlot, DstItem)) != 0)
                {
                    BuildInventoryChangeError(SrcItem, DstItem, error);
                    return false;
                }
            }
        }
        else
        {
            if (DstItem->IsContainer())
            {
                if (static_cast<Container*>(DstItem)->HasItems())
                {
                    BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return false;
                }
            }

            if ((error = CanEquipItemInSlot2(SrcInvSlot, SrcInvSlot, DstItem)) != 0)
            {
                BuildInventoryChangeError(SrcItem, DstItem, error);
                return false;
            }
        }
    }

    if (SrcItem)
    {
        if (DstInvSlot == INVENTORY_SLOT_NOT_SET) //not bag
        {

            if (SrcItem->IsContainer())
            {
                if (static_cast<Container*>(SrcItem)->HasItems())
                {
                    if (!IsBagSlot(DstSlot))
                    {
                        BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                        return false;
                    }
                }
            }

            if (DstSlot < INVENTORY_KEYRING_END)
            {
                if ((error = CanEquipItemInSlot2(DstInvSlot, DstSlot, SrcItem)) != 0)
                {
                    BuildInventoryChangeError(SrcItem, DstItem, error);
                    return false;
                }
            }
        }
        else
        {
            if (SrcItem->IsContainer())
            {
                if (static_cast<Container*>(SrcItem)->HasItems())
                {
                    BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG);
                    return false;
                }
            }

            if ((error = CanEquipItemInSlot2(DstInvSlot, DstInvSlot, SrcItem)) != 0)
            {
                BuildInventoryChangeError(SrcItem, DstItem, error);
                return false;
            }
        }
    }

    if (SrcItem && DstSlot < INVENTORY_SLOT_BAG_END && DstInvSlot == INVENTORY_SLOT_NOT_SET)   //equip - bags can be soulbound too
    {
        if (SrcItem->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
            SrcItem->SoulBind();

#if VERSION_STRING > TBC
        m_pOwner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, SrcItem->getItemProperties()->ItemId, 0, 0);

        if (DstSlot < INVENTORY_SLOT_BAG_START) // check Superior/Epic achievement
        {
            // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
            // "213" value not found in achievement or criteria entries, have to hard-code it here? :(
            // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
            // "187" value not found in achievement or criteria entries, have to hard-code it here? :(
            if ((SrcItem->getItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && SrcItem->getItemProperties()->ItemLevel >= 187) ||
                (SrcItem->getItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && SrcItem->getItemProperties()->ItemLevel >= 213))
                m_pOwner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, DstSlot, SrcItem->getItemProperties()->Quality, 0);
        }
#endif
    }

    if (DstItem && SrcSlot < INVENTORY_SLOT_BAG_END && SrcInvSlot == INVENTORY_SLOT_NOT_SET)   //equip - make sure to soulbind items swapped from equip slot to bag slot
    {
        if (DstItem->getItemProperties()->Bonding == ITEM_BIND_ON_EQUIP)
            DstItem->SoulBind();
#if VERSION_STRING > TBC
        m_pOwner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM, DstItem->getItemProperties()->ItemId, 0, 0);
        if (SrcSlot < INVENTORY_SLOT_BAG_START) // check Superior/Epic achievement
        {
            if ((DstItem->getItemProperties()->Quality == ITEM_QUALITY_RARE_BLUE && DstItem->getItemProperties()->ItemLevel >= 187) ||
                (DstItem->getItemProperties()->Quality == ITEM_QUALITY_EPIC_PURPLE && DstItem->getItemProperties()->ItemLevel >= 213))
                m_pOwner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM, SrcSlot, DstItem->getItemProperties()->Quality, 0);
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
            static_cast<Container*>(GetInventoryItem(SrcInvSlot))->SwapItems(SrcSlot, DstSlot);
        }
    }
    else
    {
        //Check for stacking
        uint32 srcItemMaxStack = (SrcItem->getOwner()->ItemStackCheat) ? 0x7fffffff : SrcItem->getItemProperties()->MaxCount;
        uint32 dstItemMaxStack = (DstItem) ? ((DstItem->getOwner()->ItemStackCheat) ? 0x7fffffff : DstItem->getItemProperties()->MaxCount) : 0;
        if (DstItem && SrcItem && SrcItem->GetEntry() == DstItem->GetEntry() && srcItemMaxStack > 1 && SrcItem->wrapped_item_id == 0 && DstItem->wrapped_item_id == 0)
        {
            uint32 total = SrcItem->GetStackCount() + DstItem->GetStackCount();
            if (total <= dstItemMaxStack)
            {
                DstItem->ModStackCount(SrcItem->GetStackCount());
                DstItem->m_isDirty = true;
                bool result = SafeFullRemoveItemFromSlot(SrcInvSlot, SrcSlot);
                if (!result)
                {
                    BuildInventoryChangeError(SrcItem, DstItem, INV_ERR_ITEM_CANT_STACK);
                }
                return false;
            }
            else
            {
                if (DstItem->GetStackCount() == dstItemMaxStack)
                {

                }
                else
                {
                    int32 delta = dstItemMaxStack - DstItem->GetStackCount();
                    DstItem->setStackCount(dstItemMaxStack);
                    SrcItem->ModStackCount(-delta);
                    SrcItem->m_isDirty = true;
                    DstItem->m_isDirty = true;
                    return false;
                }
            }
        }

        if (SrcItem)
            SrcItem = SafeRemoveAndRetreiveItemFromSlot(SrcInvSlot, SrcSlot, false);

        if (DstItem)
            DstItem = SafeRemoveAndRetreiveItemFromSlot(DstInvSlot, DstSlot, false);

        if (SrcItem)
        {
            AddItemResult result = SafeAddItem(SrcItem, DstInvSlot, DstSlot);
            if (!result)
            {
                LOG_ERROR("HandleSwapItem: Error while adding item to dstslot");
                SrcItem->DeleteFromDB();
                SrcItem->DeleteMe();
                SrcItem = nullptr;
                adderror = true;
            }
        }

        if (DstItem)
        {
            AddItemResult result = SafeAddItem(DstItem, SrcInvSlot, SrcSlot);
            if (!result)
            {
                LOG_ERROR("HandleSwapItem: Error while adding item to srcslot");
                DstItem->DeleteFromDB();
                DstItem->DeleteMe();
                DstItem = nullptr;
                adderror = true;
            }
        }
    }

    //Recalculate Expertise (for Weapon specs)
    m_pOwner->CalcExpertise();

    if (adderror)
        return false;
    else
        return true;
}

void ItemInterface::removeLootableItems()
{
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item == nullptr)
            continue;

        if (item->loot != nullptr)
            SafeFullRemoveItemFromSlot(-1, i);
    }

    for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        Item* item = GetInventoryItem(i);
        if (item == nullptr)
            continue;

        if (item->loot != nullptr)
            SafeFullRemoveItemFromSlot(-1, i);
    }

    for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Container* container = dynamic_cast<Container*>(GetInventoryItem(i));
        if (container == nullptr)
            continue;

        uint8 s = static_cast<uint8>(container->GetNumSlots());
        for (uint8 j = 0; j < s; j++)
        {
            Item* item = container->GetItem(j);
            if (item == nullptr)
                continue;

            if (item->loot != nullptr)
                container->SafeFullRemoveItemFromSlot(j);
        }
    }

    for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        Container* container = dynamic_cast<Container*>(GetInventoryItem(i));
        if (container == nullptr)
            continue;

        uint8 s = static_cast<uint8>(container->GetNumSlots());
        for (uint8 j = 0; j < s; ++j)
        {
            Item* item = container->GetItem(j);
            if (item == nullptr)
                continue;

            if (item->loot != nullptr)
                container->SafeFullRemoveItemFromSlot(j);
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
            m_currentItem = m_container->GetItem(static_cast<int16>(m_containerSlot));
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
            if (m_target->m_pItems[m_slot]->IsContainer())
            {
                m_container = static_cast<Container*>(m_target->m_pItems[m_slot]);       /// we are a container :O lets look inside the box!
                m_containerSlot = 0;
                m_currentItem = nullptr;        /// clear the pointer up. so we can tell if we found an item or not
                ++m_slot;                       /// increment m_slot so we don't search this container again

                Increment();                    /// call increment() recursively. this will search the container.

                return;                         /// jump out so we're not wasting cycles and skipping items
            }


            m_currentItem = m_target->m_pItems[m_slot];     /// we're not a container, just a regular item. Set the pointer
            ++m_slot;                                       /// increment the slot counter so we don't do the same item again

            return;             /// jump out
        }
    }

    /// if we're here we've searched all items.
    m_atEnd = true;
    m_currentItem = nullptr;
}
