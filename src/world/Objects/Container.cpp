/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/Container.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Data/WoWContainer.hpp"
#include "Logging/Logger.hpp"
#include "Management/AchievementMgr.h"
#include "Units/Players/Player.hpp"

Container::Container(uint32_t high, uint32_t low) : Item(), m_Slot(nullptr)
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
            m_Slot[i] = nullptr;
    }
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

        m_Slot = std::make_unique<std::unique_ptr<Item>[]>(m_itemProperties->ContainerSlots);
        for (uint16_t i = 0; i < m_itemProperties->ContainerSlots; ++i)
            m_Slot[i] = nullptr;

        m_owner = owner;
    }
    else
    {
        sLogger.failure("Container::create: Can't create item {} missing properties!", itemid);
        return;
    }
}

void Container::loadFromDB(Field* fields)
{
    uint32_t itemId = fields[2].asUint32();

    if (m_itemProperties = sMySQLStore.getItemProperties(itemId))
    {
        setEntry(itemId);

        setCreatorGuid(fields[5].asUint32());
        setStackCount(1);

        setFlags(fields[8].asUint32());
        setRandomPropertiesId(fields[9].asUint32());

        setMaxDurability(m_itemProperties->MaxDurability);
        setDurability(fields[12].asUint32());

        setSlotCount(m_itemProperties->ContainerSlots);

        m_Slot = std::make_unique<std::unique_ptr<Item>[]>(m_itemProperties->ContainerSlots);
        for (uint16_t i = 0; i < m_itemProperties->ContainerSlots; ++i)
            m_Slot[i] = nullptr;
    }
    else
    {
        sLogger.failure("Container::loadFromDB: Can't load item {} missing properties!", itemId);
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

std::tuple<bool, std::unique_ptr<Item>> Container::addItem(int16_t slot, std::unique_ptr<Item> itemHolder)
{
    if (slot < 0 || (uint32_t)slot >= getItemProperties()->ContainerSlots)
        return { false, std::move(itemHolder) };

    if (m_Slot[slot])
    {
        sLogger.failure("Container::addItem: Bad container item {} slot {}", itemHolder->getGuidLow(), slot);
        return { false, std::move(itemHolder) };
    }

    if (!m_owner)
        return { false, std::move(itemHolder) };

    auto* item = itemHolder.get();
    m_Slot[slot] = std::move(itemHolder);
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

    return { true, nullptr };
}

std::tuple<bool, std::unique_ptr<Item>> Container::addItemToFreeSlot(std::unique_ptr<Item> itemHolder, uint32_t* r_slot)
{
    for (uint8_t slot = 0; slot < getItemProperties()->ContainerSlots; ++slot)
    {
        if (!m_Slot[slot])
        {
            auto* item = itemHolder.get();
            m_Slot[slot] = std::move(itemHolder);
            item->m_isDirty = true;

            item->setContainer(this);
            item->setOwner(m_owner);

            setSlot(slot, item->getGuid());

            forceCreationUpdate(item);

            if (r_slot)
                *r_slot = slot;

            return { true, nullptr };
        }
    }
    return { false, std::move(itemHolder) };
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
        return m_Slot[slot].get();
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

    std::unique_ptr<Item> temp = std::move(m_Slot[SrcSlot]);
    m_Slot[SrcSlot] = std::move(m_Slot[DstSlot]);
    m_Slot[DstSlot] = std::move(temp);

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

std::unique_ptr<Item> Container::safeRemoveAndRetreiveItemFromSlot(int16_t slot, bool destroy)
{
    if (slot < 0 || static_cast<uint32_t>(slot) >= getItemProperties()->ContainerSlots)
        return nullptr;

    Item* rawItem = m_Slot[slot].get();
    if (!rawItem || rawItem == this)
        return nullptr;

    if (rawItem->getOwner() != m_owner)
        return nullptr;

    std::unique_ptr<Item> item = std::move(m_Slot[slot]);
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

    Item* rawItem = m_Slot[slot].get();
    if (!rawItem || rawItem == this)
        return false;

    std::unique_ptr<Item> item = std::move(m_Slot[slot]);

    setSlot(slot, 0);
    item->setContainer(nullptr);

    if (item->IsInWorld())
        item->removeFromWorld();

    item->deleteFromDB();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// WoWData

uint32_t Container::getSlotCount() const { return containerData()->slot_count; }
void Container::setSlotCount(uint32_t count) { write(containerData()->slot_count, count); }

uint64_t Container::getSlot(uint16_t slot) const { return containerData()->item_slot[slot].guid; }
void Container::setSlot(uint16_t slot, uint64_t guid) { write(containerData()->item_slot[slot].guid, guid); }
