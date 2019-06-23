/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Container.h"
#include "Data/WoWObject.h"
#include "Data/WoWItem.h"
#include "Item.h"
#include "Map/MapMgrDefines.hpp"
#include "Spell/Definitions/SpellEffects.h"
#include "Storage/MySQLDataStore.hpp"

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

    for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        setSpellCharges(i, m_itemProperties->Spells[i].Charges);

    setDurability(m_itemProperties->MaxDurability);
    setMaxDurability(m_itemProperties->MaxDurability);

    m_owner = owner;
    if (m_itemProperties->LockId > 1)
        locked = true;
    else
        locked = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// WoWData

uint64_t Item::getOwnerGuid() const { return itemData()->owner_guid.guid; }
uint32_t Item::getOwnerGuidLow() const { return itemData()->owner_guid.parts.low; }
uint32_t Item::getOwnerGuidHigh() const { return itemData()->owner_guid.parts.high; }
void Item::setOwnerGuid(uint64_t guid) { write(itemData()->owner_guid.guid, guid); }

void Item::setContainerGuid(uint64_t guid) { write(itemData()->container_guid.guid, guid); }

uint64_t Item::getCreatorGuid() const { return itemData()->creator_guid.guid; }
void Item::setCreatorGuid(uint64_t guid) { write(itemData()->creator_guid.guid, guid); }

uint64_t Item::getGiftCreatorGuid() const { return itemData()->gift_creator_guid.guid; }
void Item::setGiftCreatorGuid(uint64_t guid) { write(itemData()->gift_creator_guid.guid, guid); }

uint32_t Item::getStackCount() const { return itemData()->stack_count; }
void Item::setStackCount(uint32_t count) { write(itemData()->stack_count, count); }
void Item::modStackCount(int32_t mod)
{
    int32_t newStackCount = getStackCount();
    newStackCount += mod;

    if (newStackCount < 0)
        newStackCount = 0;

    setStackCount(newStackCount);
}

#ifdef AE_TBC
void Item::setTextId(const uint32 textId)
{
	write(itemData()->item_text_id, textId);
}
#endif

uint32_t Item::getDuration() const { return itemData()->duration; }
void Item::setDuration(uint32_t seconds) { write(itemData()->duration, seconds); }

int32_t Item::getSpellCharges(uint8_t index) const { return itemData()->spell_charges[index]; }
void Item::setSpellCharges(uint8_t index, int32_t count)
{
    ARCEMU_ASSERT(index < WOWITEM_SPELL_CHARGES_COUNT)

    write(itemData()->spell_charges[index], count);
}

void Item::modSpellCharges(uint8_t index, int32_t mod)
{
    ARCEMU_ASSERT(index < WOWITEM_SPELL_CHARGES_COUNT)

    int32_t newSpellCharges = getSpellCharges(index);
    newSpellCharges += mod;

    if (newSpellCharges < 0)
        newSpellCharges = 0;

    setSpellCharges(index, newSpellCharges);
}

uint32_t Item::getFlags() const { return itemData()->flags; }
void Item::setFlags(uint32_t flags) { write(itemData()->flags, flags); }
void Item::addFlags(uint32_t flags) { setFlags(getFlags() | flags); }
void Item::removeFlags(uint32_t flags) { setFlags(getFlags() & ~flags); }
bool Item::hasFlags(uint32_t flags) const { return (getFlags() & flags) != 0; }

#if VERSION_STRING >= WotLK
uint32_t Item::getEnchantmentId(uint8_t index) const { return itemData()->enchantment[index].id; }
void Item::setEnchantmentId(uint8_t index, uint32_t id) { write(itemData()->enchantment[index].id, id); }

uint32_t Item::getEnchantmentDuration(uint8_t index) const { return itemData()->enchantment[index].duration; }
void Item::setEnchantmentDuration(uint8_t index, uint32_t duration) { write(itemData()->enchantment[index].duration, duration); }

uint32_t Item::getEnchantmentCharges(uint8_t index) const { return itemData()->enchantment[index].charges; }
void Item::setEnchantmentCharges(uint8_t index, uint32_t charges) { write(itemData()->enchantment[index].charges, charges); }
#endif

uint32_t Item::getPropertySeed() const { return itemData()->property_seed; }
void Item::setPropertySeed(uint32_t seed)
{
    write(itemData()->property_seed, seed);
    random_suffix = seed;
}

uint32_t Item::getRandomPropertiesId() const { return itemData()->random_properties_id; }
void Item:: setRandomPropertiesId(uint32_t id)
{
    write(itemData()->random_properties_id, id);
    random_prop = id;
}

uint32_t Item::getDurability() const { return itemData()->durability; }
void Item::setDurability(uint32_t durability) { write(itemData()->durability, durability); }

uint32_t Item::getMaxDurability() const { return itemData()->max_durability; }
void Item::setMaxDurability(uint32_t maxDurability) { write(itemData()->max_durability, maxDurability); }

#if VERSION_STRING >= WotLK
uint32_t Item::getCreatePlayedTime() const { return itemData()->create_played_time; }
void Item::setCreatePlayedTime(uint32_t time) { write(itemData()->create_played_time, time); }
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Misc

Player* Item::getOwner() const { return m_owner; }
void Item::setOwner(Player* owner)
{
    write(itemData()->owner_guid.guid, owner ? owner->getGuid() : 0UL);
    m_owner = owner;
}

void Item::setContainer(Container* container) { setContainerGuid(container ? container->getGuid() : 0UL); }

ItemProperties const* Item::getItemProperties() const { return m_itemProperties; }
void Item::setItemProperties(ItemProperties const* itemProperties) { m_itemProperties = itemProperties; }

bool Item::fitsToSpellRequirements(SpellInfo const* spellInfo) const
{
    const auto itemProperties = getItemProperties();
    const auto isEnchantSpell = spellInfo->hasEffect(SPELL_EFFECT_ENCHANT_ITEM) || spellInfo->hasEffect(SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) || spellInfo->hasEffect(SPELL_EFFECT_ADD_SOCKET);

    if (spellInfo->getEquippedItemClass() != -1)
    {
        if (isEnchantSpell)
        {
            // Armor Vellums
            if (spellInfo->getEquippedItemClass() == ITEM_CLASS_ARMOR && itemProperties->Class == ITEM_CLASS_TRADEGOODS && itemProperties->SubClass == ITEM_SUBCLASS_ARMOR_ENCHANTMENT)
                return true;
            // Weapon Vellums
            if (spellInfo->getEquippedItemClass() == ITEM_CLASS_WEAPON && itemProperties->Class == ITEM_CLASS_TRADEGOODS && itemProperties->SubClass == ITEM_SUBCLASS_WEAPON_ENCHANTMENT)
                return true;
        }
        // Check if item classes match
        if (spellInfo->getEquippedItemClass() != static_cast<int32_t>(itemProperties->Class))
            return false;
        // Check if item subclasses match
        if (spellInfo->getEquippedItemSubClass() != 0 && !(spellInfo->getEquippedItemSubClass() & (1 << itemProperties->SubClass)))
            return false;
    }

    // Check if the enchant spell is casted on a correct item
    if (isEnchantSpell && spellInfo->getEquippedItemInventoryTypeMask() != 0)
    {
        if (itemProperties->InventoryType == INVTYPE_WEAPON &&
            (spellInfo->getEquippedItemInventoryTypeMask() & (1 << INVTYPE_WEAPONMAINHAND) ||
             spellInfo->getEquippedItemInventoryTypeMask() & (1 << INVTYPE_WEAPONOFFHAND)))
            return true;
        else if (!(spellInfo->getEquippedItemInventoryTypeMask() & (1 << itemProperties->InventoryType)))
            return false;
    }
    return true;
}
