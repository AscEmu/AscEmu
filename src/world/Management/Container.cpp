/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
 *
 */

#include "StdAfx.h"
#include "Management/Container.h"
#include "Storage/MySQLDataStore.hpp"
#include "Data/WoWContainer.h"

// MIT start
//////////////////////////////////////////////////////////////////////////////////////////
// WoWData

uint32_t Container::getSlotCount() const { return containerData()->slot_count; }
void Container::setSlotCount(uint32_t count) { write(containerData()->slot_count, count); }

//\todo not used. is it really uint64_t (guid) or is it another value we want to send to the client?
uint64_t Container::getSlot(uint16_t slot) const { return containerData()->item_slot[slot].guid; }
void Container::setSlot(uint16_t slot, uint64_t guid) { write(containerData()->item_slot[slot].guid, guid); }
// MIT end

Container::Container(uint32 high, uint32 low) : Item()
{
    m_objectType |= (TYPE_ITEM | TYPE_CONTAINER);
    m_objectTypeId = TYPEID_CONTAINER;
    m_valuesCount = CONTAINER_END;

    m_uint32Values = __fields;
    memset(m_uint32Values, 0, (CONTAINER_END)*sizeof(uint32));
    m_updateMask.SetCount(CONTAINER_END);

    setOType(TYPE_CONTAINER | TYPE_ITEM | TYPE_OBJECT);

    setGuidLow(low);
    setGuidHigh(high);
    m_wowGuid.Init(getGuid());

    setScale(1);   //always 1

    m_Slot = NULL;
    random_suffix = random_prop = 0;
}

Container::~Container()
{
    for (uint32 i = 0; i < m_itemProperties->ContainerSlots; ++i)
    {
        if (m_Slot[i] && m_Slot[i]->getOwner() == m_owner)
        {
            m_Slot[i]->DeleteMe();
        }
    }

    delete[] m_Slot;
}

void Container::LoadFromDB(Field* fields)
{
    uint32 itemid = fields[2].GetUInt32();
    m_itemProperties = sMySQLStore.getItemProperties(itemid);

    ARCEMU_ASSERT(m_itemProperties != nullptr);
    setEntry(itemid);


    setCreatorGuid(fields[5].GetUInt32());
    setStackCount(1);

    setFlags(fields[8].GetUInt32());
    setRandomPropertiesId(fields[9].GetUInt32());

    setMaxDurability(m_itemProperties->MaxDurability);
    setDurability(fields[12].GetUInt32());


    setSlotCount(m_itemProperties->ContainerSlots);

    m_Slot = new Item*[m_itemProperties->ContainerSlots];
    memset(m_Slot, 0, sizeof(Item*) * (m_itemProperties->ContainerSlots));

}

void Container::Create(uint32 itemid, Player* owner)
{
    m_itemProperties = sMySQLStore.getItemProperties(itemid);
    ARCEMU_ASSERT(m_itemProperties != nullptr);

    setEntry(itemid);

    ///\todo this shouldn't get NULL form containers in mail fix me
    if (owner != NULL)
    {
        setOwnerGuid(0UL);
        setContainerGuid(owner->getGuid());
    }
    setStackCount(1);
    setSlotCount(m_itemProperties->ContainerSlots);

    m_Slot = new Item*[m_itemProperties->ContainerSlots];
    memset(m_Slot, 0, sizeof(Item*) * (m_itemProperties->ContainerSlots));

    m_owner = owner;
}

int8 Container::FindFreeSlot()
{
    int8 TotalSlots = static_cast<int8>(getSlotCount());
    for (int8 i = 0; i < TotalSlots; ++i)
    {
        if (!m_Slot[i])
        {
            return i;
        }
    }
    LOG_DEBUG("Container::FindFreeSlot: no slot available");
    return ITEM_NO_SLOT_AVAILABLE;
}

bool Container::HasItems()
{
    int8 TotalSlots = static_cast<int8>(getSlotCount());
    for (int8 i = 0; i < TotalSlots; i++)
    {
        if (m_Slot[i])
        {
            return true;
        }
    }
    return false;
}

bool Container::AddItem(int16 slot, Item* item)
{
    if (slot < 0 || (uint32)slot >= getItemProperties()->ContainerSlots)
        return false;

    //ARCEMU_ASSERT(  m_Slot[slot] == NULL);
    if (m_Slot[slot] != NULL)
    {
        LogError("Bad container item %u slot %d", item->getGuid(), slot);
        return false;
    }

    if (!m_owner)
        return false;

    m_Slot[slot] = item;
    item->m_isDirty = true;

    item->setContainer(this);
    item->setOwner(m_owner);

    if (item->getItemProperties()->Bonding == ITEM_BIND_ON_PICKUP)
    {
        if (item->getItemProperties()->Flags & ITEM_FLAG_ACCOUNTBOUND) // don't "Soulbind" account-bound items
            item->addFlags(ITEM_FLAG_ACCOUNTBOUND);
        else
            item->addFlags(ITEM_FLAG_SOULBOUND);
    }

    setSlot(slot, item->getGuid());

    //new version to fix bag issues
    if (m_owner->IsInWorld() && !item->IsInWorld())
    {
        //item->AddToWorld();
        item->PushToWorld(m_owner->GetMapMgr());

        ByteBuffer buf(3000);
        uint32 count = item->buildCreateUpdateBlockForPlayer(&buf, m_owner);
        m_owner->getUpdateMgr().pushCreationData(&buf, count);
    }
#if VERSION_STRING > TBC
    m_owner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM, item->getItemProperties()->ItemId, item->getStackCount(), 0);
#endif
    return true;
}

void Container::SwapItems(int8 SrcSlot, int8 DstSlot)
{
    Item* temp;
    if (SrcSlot < 0 || SrcSlot >= (int8)m_itemProperties->ContainerSlots)
        return;

    if (DstSlot < 0 || DstSlot >= (int8)m_itemProperties->ContainerSlots)
        return;

    uint32 destMaxCount = (m_owner->m_cheats.ItemStackCheat) ? 0x7fffffff : ((m_Slot[DstSlot]) ? m_Slot[DstSlot]->getItemProperties()->MaxCount : 0);
    if (m_Slot[DstSlot] && m_Slot[SrcSlot] && m_Slot[DstSlot]->getEntry() == m_Slot[SrcSlot]->getEntry() && m_Slot[SrcSlot]->wrapped_item_id == 0 && m_Slot[DstSlot]->wrapped_item_id == 0 && destMaxCount > 1)
    {
        uint32 total = m_Slot[SrcSlot]->getStackCount() + m_Slot[DstSlot]->getStackCount();
        m_Slot[DstSlot]->m_isDirty = m_Slot[SrcSlot]->m_isDirty = true;
        if (total <= destMaxCount)
        {
            m_Slot[DstSlot]->modStackCount(m_Slot[SrcSlot]->getStackCount());
            SafeFullRemoveItemFromSlot(SrcSlot);
            return;
        }
        else
        {
            if (m_Slot[DstSlot]->getStackCount() == destMaxCount)
            {

            }
            else
            {
                int32 delta = destMaxCount - m_Slot[DstSlot]->getStackCount();
                m_Slot[DstSlot]->setStackCount(destMaxCount);
                m_Slot[SrcSlot]->modStackCount(-delta);
                return;
            }
        }
    }

    temp = m_Slot[SrcSlot];
    m_Slot[SrcSlot] = m_Slot[DstSlot];
    m_Slot[DstSlot] = temp;

    if (m_Slot[DstSlot])
    {
        setSlot(DstSlot, m_Slot[DstSlot]->getGuid());
        m_Slot[DstSlot]->m_isDirty = true;
    }
    else
    {
        setSlot(DstSlot, 0);
    }

    if (m_Slot[SrcSlot])
    {
        setSlot(SrcSlot, m_Slot[SrcSlot]->getGuid());
        m_Slot[SrcSlot]->m_isDirty = true;
    }
    else
    {
        setSlot(SrcSlot, 0);
    }
}

Item* Container::SafeRemoveAndRetreiveItemFromSlot(int16 slot, bool destroy)
{
    if (slot < 0 || (uint32)slot >= getItemProperties()->ContainerSlots)
        return NULL;

    Item* pItem = m_Slot[slot];

    if (pItem == NULL || pItem == this)
        return NULL;

    m_Slot[slot] = NULL;

    if (pItem->getOwner() == m_owner)
    {
        setSlot(slot, 0);
        pItem->setContainer(nullptr);

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
    {
        pItem = NULL;
    }

    return pItem;
}

bool Container::SafeFullRemoveItemFromSlot(int16 slot)
{
    if (slot < 0 || (uint32)slot >= getItemProperties()->ContainerSlots)
        return false;

    Item* pItem = m_Slot[slot];

    if (pItem == NULL || pItem == this) return false;
    m_Slot[slot] = NULL;

    setSlot(slot, 0);
    pItem->setContainer(nullptr);

    if (pItem->IsInWorld())
    {
        pItem->RemoveFromWorld();
    }
    pItem->DeleteFromDB();
    pItem->DeleteMe();

    return true;
}

bool Container::AddItemToFreeSlot(Item* pItem, uint32* r_slot)
{
    uint32 slot;
    for (slot = 0; slot < getItemProperties()->ContainerSlots; slot++)
    {
        if (!m_Slot[slot])
        {
            m_Slot[slot] = pItem;
            pItem->m_isDirty = true;

            pItem->setContainer(this);
            pItem->setOwner(m_owner);

            setSlot(uint16(slot), pItem->getGuid());

            if (m_owner->IsInWorld() && !pItem->IsInWorld())
            {
                pItem->PushToWorld(m_owner->GetMapMgr());
                ByteBuffer buf(2500);
                uint32 count = pItem->buildCreateUpdateBlockForPlayer(&buf, m_owner);
                m_owner->getUpdateMgr().pushCreationData(&buf, count);
            }
            if (r_slot)
                *r_slot = slot;

#if VERSION_STRING > TBC
            m_owner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM, pItem->getItemProperties()->ItemId, pItem->getStackCount(), 0);
#endif
            return true;
        }
    }
    return false;
}

Item* Container::GetItem(int16 slot)
{
    if (slot >= 0 && (uint16)slot < getItemProperties()->ContainerSlots)
        return m_Slot[slot];
    else
        return 0;
}

void Container::SaveBagToDB(int8 slot, bool first, QueryBuffer* buf)
{
    SaveToDB(INVENTORY_SLOT_NOT_SET, slot, first, buf);

    for (uint32 i = 0; i < m_itemProperties->ContainerSlots; ++i)
    {
        if (m_Slot[i] && !((m_Slot[i]->getItemProperties()->Flags) & 2))
        {
            m_Slot[i]->SaveToDB(slot, static_cast<int8>(i), first, buf);
        }
    }
}
