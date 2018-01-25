/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Data/WoWObject.h"
#include "Data/WoWItem.h"

void Item::init(uint32_t high, uint32_t low)
{
    memset(m_uint32Values, 0, sizeof(WoWItem));
    write(objectData()->type, TYPE_ITEM | TYPE_OBJECT);
    write(objectData()->scale_x, 1);
    write(objectData()->guid, low, high);
    m_wowGuid.Init(objectData()->guid);

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
    m_faction = nullptr;
    m_factionDBC = nullptr;
    m_instanceId = INSTANCEID_NOT_IN_WORLD;
    m_inQueue = false;
    m_loadedFromDB = false;
}


void Item::create(uint32_t itemId, Player* owner)
{
    write(itemData()->entry, itemId);

    if (owner != nullptr)
    {
        const auto owner_guid = owner->objectData()->guid;

        write(itemData()->owner_guid, owner_guid);
        write(itemData()->container_guid, owner_guid);
    }

    write(itemData()->stack_count, 1);

    m_itemProperties = sMySQLStore.getItemProperties(itemId);

    ARCEMU_ASSERT(m_itemProperties != nullptr);

    for (auto i = 0; i < 4; ++i)
        write(itemData()->spell_charges[i], m_itemProperties->Spells[i].Charges);

    write(itemData()->durability, m_itemProperties->MaxDurability);
    write(itemData()->max_durability, m_itemProperties->MaxDurability);

    m_owner = owner;
    if (m_itemProperties->LockId > 1)
        locked = true;
    else
        locked = false;
}

Player* Item::getOwner() const { return m_owner; }

void Item::setOwner(Player* owner)
{
    write(itemData()->owner_guid, owner ? owner->objectData()->guid : 0);
    m_owner = owner;
}

ItemProperties const* Item::getItemProperties() const { return m_itemProperties; }
void Item::setItemProperties(ItemProperties const* itemProperties) { m_itemProperties = itemProperties; }
