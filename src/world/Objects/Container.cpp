/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/Container.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Data/WoWContainer.hpp"
#include "Logging/Logger.hpp"
#include "Management/AchievementMgr.h"
#include "Units/Players/Player.hpp"

Container::Container(uint32_t high, uint32_t low) : Item()
{
    m_objectType |= TYPE_ITEM | TYPE_CONTAINER;
    m_objectTypeId = TYPEID_CONTAINER;

    m_valuesCount = getSizeOfStructure(WoWContainer);
    m_uint32Values = __fields;
    memset(m_uint32Values, 0, (getSizeOfStructure(WoWContainer)) * sizeof(uint32_t));
    m_updateMask.SetCount(getSizeOfStructure(WoWContainer));

    setOType(TYPE_CONTAINER | TYPE_ITEM | TYPE_OBJECT);
    setGuid(low, high);
    setScale(1);
}

Container::~Container()
{
    for (uint8_t i = 0; i < m_itemProperties->ContainerSlots; ++i)
    {
        if (m_Slot[i] && m_Slot[i]->getOwner() == m_owner)
            m_Slot[i]->deleteMe();
    }

    delete[] m_Slot;
}

void Container::create(uint32_t itemid, Player* owner)
{
    if (m_itemProperties = sMySQLStore.getItemProperties(itemid))
    {
        setEntry(itemid);

        if (owner != NULL)
        {
            setOwnerGuid(0UL);
            setContainerGuid(owner->getGuid());
        }

        setStackCount(1);
        setSlotCount(m_itemProperties->ContainerSlots);

        m_Slot = new Item * [m_itemProperties->ContainerSlots];
        memset(m_Slot, 0, sizeof(Item*) * (m_itemProperties->ContainerSlots));

        m_owner = owner;
    }
    else
    {
        sLogger.failure("Container::create: Can't create item %u missing properties!", itemid);
        return;
    }
}

void Container::loadFromDB(Field* fields)
{
    uint32_t itemId = fields[2].GetUInt32();

    if (m_itemProperties = sMySQLStore.getItemProperties(itemId))
    {
        setEntry(itemId);

        setCreatorGuid(fields[5].GetUInt32());
        setStackCount(1);

        setFlags(fields[8].GetUInt32());
        setRandomPropertiesId(fields[9].GetUInt32());

        setMaxDurability(m_itemProperties->MaxDurability);
        setDurability(fields[12].GetUInt32());

        setSlotCount(m_itemProperties->ContainerSlots);

        m_Slot = new Item * [m_itemProperties->ContainerSlots];
        memset(m_Slot, 0, sizeof(Item*) * (m_itemProperties->ContainerSlots));
    }
    else
    {
        sLogger.failure("Container::loadFromDB: Can't load item %u missing properties!", itemId);
        return;
    }
}

void Container::saveToDB(int8_t slot, bool first, QueryBuffer* buf)
{
    Item::saveToDB(INVENTORY_SLOT_NOT_SET, slot, first, buf);

    for (uint8_t i = 0; i < m_itemProperties->ContainerSlots; ++i)
    {
        if (m_Slot[i] && !((m_Slot[i]->getItemProperties()->Flags) & 2))
            m_Slot[i]->saveToDB(slot, static_cast<int8_t>(i), first, buf);
    }
}

bool Container::addItem(int16_t slot, Item* item)
{
    if (slot < 0 || (uint32_t)slot >= getItemProperties()->ContainerSlots)
        return false;

    if (m_Slot[slot])
    {
        sLogger.failure("Container::addItem: Bad container item %u slot %d", item->getGuidLow(), slot);
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
        if (item->getItemProperties()->Flags & ITEM_FLAG_ACCOUNTBOUND)
            item->addFlags(ITEM_FLAG_ACCOUNTBOUND);
        else
            item->addFlags(ITEM_FLAG_SOULBOUND);
    }

    setSlot(slot, item->getGuid());

    forceCreationUpdate(item);

    return true;
}

bool Container::addItemToFreeSlot(Item* item, uint32_t* r_slot)
{
    for (uint8_t slot = 0; slot < getItemProperties()->ContainerSlots; ++slot)
    {
        if (!m_Slot[slot])
        {
            m_Slot[slot] = item;
            item->m_isDirty = true;

            item->setContainer(this);
            item->setOwner(m_owner);

            setSlot(slot, item->getGuid());

            forceCreationUpdate(item);

            if (r_slot)
                *r_slot = slot;

            return true;
        }
    }
    return false;
}

void Container::forceCreationUpdate(Item* item)
{
    if (m_owner->IsInWorld() && !item->IsInWorld())
    {
        item->PushToWorld(m_owner->getWorldMap());
        ByteBuffer buf(2500);
        uint32_t count = item->buildCreateUpdateBlockForPlayer(&buf, m_owner);
        m_owner->getUpdateMgr().pushCreationData(&buf, count);
    }

#if VERSION_STRING > TBC
    m_owner->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM, item->getItemProperties()->ItemId, item->getStackCount(), 0);
#endif
}

Item* Container::getItem(int16_t slot)
{
    if (slot >= 0 && static_cast<uint32_t>(slot) < getItemProperties()->ContainerSlots)
        return m_Slot[slot];
    else
        return nullptr;
}

int8_t Container::findFreeSlot()
{
    uint8_t totalSlots = static_cast<uint8_t>(getSlotCount());
    for (uint8_t i = 0; i < totalSlots; ++i)
        if (!m_Slot[i])
            return i;

    sLogger.debug("Container::findFreeSlot: no slot available");
    return ITEM_NO_SLOT_AVAILABLE;
}

bool Container::hasItems() const
{
    uint8_t totalSlots = static_cast<uint8_t>(getSlotCount());
    for (uint8_t i = 0; i < totalSlots; ++i)
        if (m_Slot[i])
            return true;

    return false;
}

void Container::swapItems(int8_t SrcSlot, int8_t DstSlot)
{
    if (SrcSlot < 0 || SrcSlot >= static_cast<int8_t>(m_itemProperties->ContainerSlots))
        return;

    if (DstSlot < 0 || DstSlot >= static_cast<int8_t>(m_itemProperties->ContainerSlots))
        return;

    uint32_t destMaxCount = (m_owner->m_cheats.hasItemStackCheat) ? 0x7fffffff : ((m_Slot[DstSlot]) ? m_Slot[DstSlot]->getItemProperties()->MaxCount : 0);
    if (m_Slot[DstSlot] && m_Slot[SrcSlot] && m_Slot[DstSlot]->getEntry() == m_Slot[SrcSlot]->getEntry() && m_Slot[SrcSlot]->m_wrappedItemId == 0 && m_Slot[DstSlot]->m_wrappedItemId == 0 && destMaxCount > 1)
    {
        uint32_t total = m_Slot[SrcSlot]->getStackCount() + m_Slot[DstSlot]->getStackCount();
        m_Slot[DstSlot]->m_isDirty = m_Slot[SrcSlot]->m_isDirty = true;
        if (total <= destMaxCount)
        {
            m_Slot[DstSlot]->modStackCount(m_Slot[SrcSlot]->getStackCount());
            safeFullRemoveItemFromSlot(SrcSlot);
            return;
        }
        else
        {
            if (m_Slot[DstSlot]->getStackCount() != destMaxCount)
            {
                int32_t delta = destMaxCount - m_Slot[DstSlot]->getStackCount();
                m_Slot[DstSlot]->setStackCount(destMaxCount);
                m_Slot[SrcSlot]->modStackCount(-delta);
                return;
            }
        }
    }

    Item* temp = m_Slot[SrcSlot];
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

Item* Container::safeRemoveAndRetreiveItemFromSlot(int16_t slot, bool destroy)
{
    if (slot < 0 || static_cast<uint32_t>(slot) >= getItemProperties()->ContainerSlots)
        return nullptr;

    Item* item = m_Slot[slot];
    if (!item || item == this)
        return nullptr;

    if (item->getOwner() != m_owner)
        return nullptr;

    m_Slot[slot] = nullptr;
    setSlot(slot, 0);

    item->setContainer(nullptr);

    if (destroy)
    {
        if (item->IsInWorld())
            item->removeFromWorld();

        item->deleteFromDB();
    }

    return item;
}

bool Container::safeFullRemoveItemFromSlot(int16_t slot)
{
    if (slot < 0 || static_cast<uint32_t>(slot) >= getItemProperties()->ContainerSlots)
        return false;

    Item* item = m_Slot[slot];
    if (!item || item == this)
        return false;

    m_Slot[slot] = nullptr;

    setSlot(slot, 0);
    item->setContainer(nullptr);

    if (item->IsInWorld())
        item->removeFromWorld();

    item->deleteFromDB();
    item->deleteMe();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// WoWData

uint32_t Container::getSlotCount() const { return containerData()->slot_count; }
void Container::setSlotCount(uint32_t count) { write(containerData()->slot_count, count); }

//\todo not used. is it really uint64_t (guid) or is it another value we want to send to the client?
uint64_t Container::getSlot(uint16_t slot) const { return containerData()->item_slot[slot].guid; }
void Container::setSlot(uint16_t slot, uint64_t guid) { write(containerData()->item_slot[slot].guid, guid); }
