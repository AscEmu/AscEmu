/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Data/WoWObject.h"
#include "Data/WoWItem.h"
#include "Server/WUtil.h"
#include "Item.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgrDefines.hpp"
#include "Container.h"

void Item::init(uint32_t high, uint32_t low)
{
    memset(m_uint32Values, 0, sizeof(WoWItem));
    setObjectType(TYPEID_ITEM);
    setScale(1.f);
    setGuid(low, high);

    m_itemProperties = nullptr;
    m_owner = nullptr;
    loot = nullptr;
    locked = false;
    m_isDirty = true;
    random_prop = 0;
    random_suffix = 0;
    wrapped_item_id = 0;
    m_mapId = MAPID_NOT_IN_WORLD;
    m_zoneId = 0;
    m_objectUpdated = false;
    m_mapMgr = nullptr;
    m_factionTemplate = nullptr;
    m_factionEntry = nullptr;
    m_instanceId = INSTANCEID_NOT_IN_WORLD;
    m_inQueue = false;
    m_loadedFromDB = false;
}

void Item::setContainer(Container* container)
{
    setContainerGuid(container ? container->getGuid() : 0UL);
}

void Item::setContainerGuid(uint64_t guid)
{
    write(itemData()->container_guid.guid, guid);
}

uint64_t Item::getOwnerGuid() const
{
    return itemData()->owner_guid.guid;
}

uint32_t Item::getOwnerGuidLow() const
{
    return itemData()->owner_guid.parts.low;
}

uint32_t Item::getOwnerGuidHigh() const
{
    return itemData()->owner_guid.parts.high;
}

void Item::setStackCount(uint32_t count)
{
    write(itemData()->stack_count, count);
}

void Item::setDuration(uint32_t seconds)
{
    write(itemData()->duration, seconds);
}

uint32_t Item::getDuration() const
{
    return itemData()->duration;
}

void Item::setSpellCharges(uint32_t idx, int32_t count)
{
    ARCEMU_ASSERT(idx < WOWITEM_SPELL_CHARGES_COUNT)

    write(itemData()->spell_charges[idx], count);
}

void Item::create(uint32_t itemId, Player* owner)
{
    setEntry(itemId);

    if (owner != nullptr)
    {
        setOwner(owner);
        setContainerGuid(owner->getGuid());
    }

    setStackCount(1);

    m_itemProperties = sMySQLStore.getItemProperties(itemId);

    ARCEMU_ASSERT(m_itemProperties != nullptr);

    for (auto i = 0; i < 4; ++i)
        setSpellCharges(i, m_itemProperties->Spells[i].Charges);

    setDurability(m_itemProperties->MaxDurability);
    setMaxDurability(m_itemProperties->MaxDurability);

    m_owner = owner;
    if (m_itemProperties->LockId > 1)
        locked = true;
    else
        locked = false;
}

void Item::setOwnerGuid(uint64_t guid)
{
    write(itemData()->owner_guid.guid, guid);
}

Player* Item::getOwner() const { return m_owner; }

void Item::setOwner(Player* owner)
{
    write(itemData()->owner_guid.guid, owner ? owner->getGuid() : 0UL);
    m_owner = owner;
}

ItemProperties const* Item::getItemProperties() const { return m_itemProperties; }
void Item::setItemProperties(ItemProperties const* itemProperties) { m_itemProperties = itemProperties; }

void Item::setDurability(uint32_t durability)
{
    write(itemData()->durability, durability);
}

void Item::setMaxDurability(uint32_t maxDurability)
{
    write(itemData()->max_durability, maxDurability);
}
