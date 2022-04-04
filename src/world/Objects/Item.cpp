/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Container.h"
#include "Data/WoWItem.hpp"
#include "Item.h"
#include "Map/MapMgrDefines.hpp"
#include "Server/Packets/SmsgEnchantmentLog.h"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLDataStore.hpp"

using namespace AscEmu::Packets;

Item::Item()
{
    //////////////////////////////////////////////////////////////////////////
    m_objectType |= TYPE_ITEM;
    m_objectTypeId = TYPEID_ITEM;
    m_valuesCount = getSizeOfStructure(WoWItem);
    //////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    m_updateFlag = UPDATEFLAG_ALL;
#endif
#if VERSION_STRING == TBC
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID);
#endif
#if VERSION_STRING == WotLK
    m_updateFlag = UPDATEFLAG_LOWGUID;
#endif
#if VERSION_STRING == Cata
    m_updateFlag = UPDATEFLAG_NONE;
#endif
#if VERSION_STRING == Mop
    m_updateFlag = UPDATEFLAG_NONE;
#endif

    //\todo Why is there a pointer to the same thing in a derived class? ToDo: sort this out..
    m_uint32Values = _fields;

    memset(m_uint32Values, 0, sizeof(WoWItem));
    m_updateMask.SetCount(getSizeOfStructure(WoWItem));
}

Item::~Item()
{
    if (loot != nullptr)
    {
        delete loot;
        loot = nullptr;
    }

    sEventMgr.RemoveEvents(this);

#if VERSION_STRING >= Cata
    for (auto itr = Enchantments.begin(); itr != Enchantments.end(); ++itr)
    {
        // These are allocated with new
        if (itr->second.Slot == REFORGE_ENCHANTMENT_SLOT || itr->second.Slot == TRANSMOGRIFY_ENCHANTMENT_SLOT)
        {
            delete itr->second.Enchantment;
            itr->second.Enchantment = nullptr;
        }
    }
#endif
    Enchantments.clear();

    if (m_owner != nullptr)
    {
        m_owner->getItemInterface()->removeTemporaryEnchantedItem(this);
#if VERSION_STRING >= WotLK
        m_owner->getItemInterface()->removeTradeableItem(this);
#endif
    }

    if (IsInWorld())
        RemoveFromWorld();

    m_owner = nullptr;
}

void Item::init(uint32_t high, uint32_t low)
{
    setObjectType(TYPEID_ITEM);
    setScale(1.f);
    setGuid(low, high);

    m_isDirty = true;
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
    if (!m_itemProperties)
    {
        sLogger.failure("Item::create: Can't create item %u missing properties!", itemId);
        return;
    }

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
uint64_t Item::getContainerGuid() const { return itemData()->container_guid.guid; }

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
    if (index < WOWITEM_SPELL_CHARGES_COUNT)
        write(itemData()->spell_charges[index], count);
}

void Item::modSpellCharges(uint8_t index, int32_t mod)
{
    if (index < WOWITEM_SPELL_CHARGES_COUNT)
    {
        int32_t newSpellCharges = getSpellCharges(index);
        newSpellCharges += mod;

        if (newSpellCharges < 0)
            newSpellCharges = 0;

        setSpellCharges(index, newSpellCharges);
    }
}

uint32_t Item::getFlags() const { return itemData()->flags; }
void Item::setFlags(uint32_t flags) { write(itemData()->flags, flags); }
void Item::addFlags(uint32_t flags) { setFlags(getFlags() | flags); }
void Item::removeFlags(uint32_t flags) { setFlags(getFlags() & ~flags); }
bool Item::hasFlags(uint32_t flags) const { return (getFlags() & flags) != 0; }

uint32_t Item::getEnchantmentId(uint8_t index) const { return itemData()->enchantment[index].id; }
void Item::setEnchantmentId(uint8_t index, uint32_t id) { write(itemData()->enchantment[index].id, id); }

uint32_t Item::getEnchantmentDuration(uint8_t index) const { return itemData()->enchantment[index].duration; }
void Item::setEnchantmentDuration(uint8_t index, uint32_t duration) { write(itemData()->enchantment[index].duration, duration); }

uint32_t Item::getEnchantmentCharges(uint8_t index) const { return itemData()->enchantment[index].charges; }
void Item::setEnchantmentCharges(uint8_t index, uint32_t charges) { write(itemData()->enchantment[index].charges, charges); }

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
// Enchantments
EnchantmentInstance* Item::getEnchantment(EnchantmentSlot slot)
{
    auto itr = Enchantments.find(slot);
    return itr != Enchantments.end() ? &itr->second : nullptr;
}

EnchantmentInstance const* Item::getEnchantment(EnchantmentSlot slot) const
{
    auto itr = Enchantments.find(slot);
    return itr != Enchantments.end() ? &itr->second : nullptr;
}

bool Item::hasEnchantment(uint32_t enchantmentId) const
{
    for (uint8_t slot = PERM_ENCHANTMENT_SLOT; slot < MAX_ENCHANTMENT_SLOT; ++slot)
    {
        if (getEnchantmentId(slot) == enchantmentId)
            return true;
    }

    return false;
}

int16_t Item::hasEnchantmentReturnSlot(uint32_t enchantmentId) const
{
    for (uint8_t slot = PERM_ENCHANTMENT_SLOT; slot < MAX_ENCHANTMENT_SLOT; ++slot)
    {
        if (getEnchantmentId(slot) == enchantmentId)
            return slot;
    }

    return -1;
}

bool Item::addEnchantment(uint32_t enchantmentId, EnchantmentSlot slot, uint32_t duration, bool removedAtLogout/* = false*/, uint32_t randomSuffix/* = 0*/)
{
    m_isDirty = true;

    DBC::Structures::SpellItemEnchantmentEntry const* Enchantment = nullptr;
    switch (slot)
    {
#if VERSION_STRING >= Cata
        case TRANSMOGRIFY_ENCHANTMENT_SLOT:
        case REFORGE_ENCHANTMENT_SLOT:
        {
            auto custom_enchant = new DBC::Structures::SpellItemEnchantmentEntry();
            custom_enchant->Id = enchantmentId;

            Enchantment = custom_enchant;
        } break;
#endif
        default:
        {
            const auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(enchantmentId);
            if (spell_item_enchant == nullptr)
                return false;

            Enchantment = spell_item_enchant;
        } break;
    }

    EnchantmentInstance enchantInstance;
    enchantInstance.BonusApplied = false;
    enchantInstance.Slot = slot;
    enchantInstance.Enchantment = Enchantment;
    enchantInstance.RemoveAtLogout = removedAtLogout;
    enchantInstance.RandomSuffix = randomSuffix;

    // Set enchantment to item's wowdata fields
    _setEnchantmentDataFields(slot, Enchantment->Id, duration, 0);

    Enchantments.insert(std::make_pair(slot, enchantInstance));

    if (m_owner == nullptr)
        return true;

    // Add the removal event
    if (duration)
        m_owner->getItemInterface()->addTemporaryEnchantedItem(this, slot);

    // Do not send log packet if owner is not yet in world
    if (!m_owner->IsInWorld())
        return true;

#if VERSION_STRING >= Cata
    if (slot == TRANSMOGRIFY_ENCHANTMENT_SLOT)
        return true;
#endif

    m_owner->sendPacket(SmsgEnchantmentLog(m_owner->getGuid(), m_owner->getGuid(), getEntry(), Enchantment->Id).serialise().get());

    // Apply enchantment bonus only if the item is equipped
    // but send enchant time update packet for items in inventory as well
    const auto equipSlot = m_owner->getItemInterface()->GetInventorySlotByGuid(getGuid());
    if (equipSlot >= EQUIPMENT_SLOT_START && equipSlot < EQUIPMENT_SLOT_END)
        ApplyEnchantmentBonus(slot, true);
    else if (duration)
        SendEnchantTimeUpdate(slot, duration / 1000);

    return true;
}

void Item::removeEnchantment(EnchantmentSlot slot, bool timerExpired/* = false*/)
{
    const auto itr = Enchantments.find(slot);
    if (itr == Enchantments.end())
        return;

    // Remove enchantment bonus
    if (itr->second.BonusApplied)
        ApplyEnchantmentBonus(slot, false);

    _setEnchantmentDataFields(slot, 0, 0, 0);

#if VERSION_STRING >= Cata
    // These are allocated with new
    if (slot == REFORGE_ENCHANTMENT_SLOT || slot == TRANSMOGRIFY_ENCHANTMENT_SLOT)
    {
        delete itr->second.Enchantment;
        itr->second.Enchantment = nullptr;
    }
#endif

    Enchantments.erase(itr);

    if (!timerExpired)
        m_owner->getItemInterface()->removeTemporaryEnchantedItem(this, slot);
}

void Item::modifyEnchantmentTime(EnchantmentSlot slot, uint32_t duration)
{
    auto itr = Enchantments.find(slot);
    if (itr == Enchantments.end())
        return;

    setEnchantmentDuration(slot, duration);
    SendEnchantTimeUpdate(itr->second.Slot, duration / 1000);
}

void Item::applyAllEnchantmentBonuses()
{
    for (auto itr = Enchantments.cbegin(); itr != Enchantments.cend();)
    {
        auto itr2 = itr++;
        ApplyEnchantmentBonus(itr2->first, true);
    }
}

void Item::removeAllEnchantmentBonuses()
{
    for (auto itr = Enchantments.cbegin(); itr != Enchantments.cend();)
    {
        auto itr2 = itr++;
        ApplyEnchantmentBonus(itr2->first, false);
    }
}

void Item::removeAllEnchantments(bool onlyTemporary)
{
    for (auto itr = Enchantments.cbegin(); itr != Enchantments.cend();)
    {
        auto itr2 = itr++;
        if (onlyTemporary && getEnchantmentDuration(itr2->second.Slot) == 0)
            continue;

        removeEnchantment(itr2->first);
    }
}

void Item::removeSocketBonusEnchant()
{
    for (const auto& enchantment : Enchantments)
    {
        if (enchantment.second.Enchantment->Id == getItemProperties()->SocketBonus)
        {
            removeEnchantment(enchantment.first);
            return;
        }
    }
}

void Item::_setEnchantmentDataFields(EnchantmentSlot slot, uint32_t enchantmentId, uint32_t duration, uint32_t charges)
{
    if (getEnchantmentId(slot) == enchantmentId && getEnchantmentDuration(slot) == duration && getEnchantmentCharges(slot) == charges)
        return;

    setEnchantmentId(slot, enchantmentId);
    setEnchantmentDuration(slot, duration);
    setEnchantmentCharges(slot, charges);

    m_isDirty = true;
}

bool Item::_findFreeRandomEnchantmentSlot(EnchantmentSlot* slot, RandomEnchantmentType randomType) const
{
    if (randomType == RandomEnchantmentType::PROPERTY)
    {
        for (uint8_t i = PROP_ENCHANTMENT_SLOT_2; i <= PROP_ENCHANTMENT_SLOT_4; ++i)
        {
            if (getEnchantmentId(i) == 0)
            {
                *slot = static_cast<EnchantmentSlot>(i);
                return true;
            }
        }
    }
    else if (randomType == RandomEnchantmentType::SUFFIX)
    {
        for (uint8_t i = PROP_ENCHANTMENT_SLOT_0; i <= PROP_ENCHANTMENT_SLOT_2; ++i)
        {
            if (getEnchantmentId(i) == 0)
            {
                *slot = static_cast<EnchantmentSlot>(i);
                return true;
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Sockets / gems
#if VERSION_STRING >= TBC
uint8_t Item::getSocketSlotCount([[maybe_unused]]bool includePrismatic/* = true*/) const
{
    // Containers have no sockets
    if (isContainer())
        return 0;

    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
    {
        if (getItemProperties()->Sockets[i].SocketColor)
            ++count;
    }

#if VERSION_STRING >= WotLK
    // Prismatic socket
    if (includePrismatic && getEnchantment(PRISMATIC_ENCHANTMENT_SLOT) != nullptr)
        ++count;
#endif

    return count;
}

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

uint32_t Item::getVisibleEntry() const
{
#if VERSION_STRING == Cata
    if (uint32_t transmogrification = getEnchantmentId(TRANSMOGRIFY_ENCHANTMENT_SLOT))
        return transmogrification;
#endif
    return getEntry();
}

bool Item::hasStats() const
{
    if (getRandomPropertiesId() != 0)
        return true;

    ItemProperties const* proto = getItemProperties();
    for (uint8_t i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        if (proto->Stats[i].Value != 0)
            return true;

    return false;
}

bool Item::canBeTransmogrified() const
{
    ItemProperties const* proto = getItemProperties();

    if (!proto)
        return false;

    if (proto->Quality == ITEM_QUALITY_LEGENDARY)
        return false;

    if (proto->Class != ITEM_CLASS_ARMOR &&
        proto->Class != ITEM_CLASS_WEAPON)
        return false;

    if (proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
        return false;

    if (proto->Flags2 & ITEM_FLAGS_EXTRA_CANNOT_BE_TRANSMOG)
        return false;

    if (!hasStats())
        return false;

    return true;
}

bool Item::canTransmogrify() const
{
    ItemProperties const* proto = getItemProperties();

    if (!proto)
        return false;

    if (proto->Flags2 & ITEM_FLAGS_EXTRA_CANNOT_TRANSMOG)
        return false;

    if (proto->Quality == ITEM_QUALITY_LEGENDARY)
        return false;

    if (proto->Class != ITEM_CLASS_ARMOR &&
        proto->Class != ITEM_CLASS_WEAPON)
        return false;

    if (proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
        return false;

    if (proto->Flags2 & ITEM_FLAGS_EXTRA_CAN_TRANSMOG)
        return true;

    if (!hasStats())
        return false;

    return true;
}

bool Item::canTransmogrifyItemWithItem(Item const* transmogrified, Item const* transmogrifier)
{
    if (!transmogrifier || !transmogrified)
        return false;

    ItemProperties const* proto1 = transmogrifier->getItemProperties(); // source
    ItemProperties const* proto2 = transmogrified->getItemProperties(); // dest

    if (proto1->ItemId == proto2->ItemId)
        return false;

    if (!transmogrified->canTransmogrify() || !transmogrifier->canBeTransmogrified())
        return false;

    if (proto1->InventoryType == INVTYPE_BAG ||
        proto1->InventoryType == INVTYPE_RELIC ||
        proto1->InventoryType == INVTYPE_BODY ||
        proto1->InventoryType == INVTYPE_FINGER ||
        proto1->InventoryType == INVTYPE_TRINKET ||
        proto1->InventoryType == INVTYPE_AMMO ||
        proto1->InventoryType == INVTYPE_QUIVER)
        return false;

    if (proto1->SubClass != proto2->SubClass && (proto1->Class != ITEM_CLASS_WEAPON || !proto2->isRangedWeapon() || !proto1->isRangedWeapon()))
        return false;

    if (proto1->InventoryType != proto2->InventoryType &&
        (proto1->Class != ITEM_CLASS_WEAPON || (proto2->InventoryType != INVTYPE_WEAPONMAINHAND && proto2->InventoryType != INVTYPE_WEAPONOFFHAND)) &&
        (proto1->Class != ITEM_CLASS_ARMOR || (proto1->InventoryType != INVTYPE_CHEST && proto2->InventoryType != INVTYPE_ROBE && proto1->InventoryType != INVTYPE_ROBE && proto2->InventoryType != INVTYPE_CHEST)))
        return false;

    return true;
}

bool Item::isInBag() const 
{
    if (m_owner->getItemInterface()->GetInventorySlotByGuid(getGuid()) > EQUIPMENT_SLOT_END)
        return true;
    
    return false;
}

bool Item::isEquipped() const
{
    return !isInBag() && m_owner->getItemInterface()->GetInventorySlotByGuid(getGuid()) < EQUIPMENT_SLOT_END;
}

#if VERSION_STRING >= WotLK
void Item::setSoulboundTradeable(LooterSet& allowedLooters)
{
    addFlags(ITEM_FLAG_BOP_TRADEABLE);
    allowedGUIDs = allowedLooters;

    // todo database
}

void Item::clearSoulboundTradeable(Player* /*currentOwner*/)
{
    removeFlags(ITEM_FLAG_BOP_TRADEABLE);
    if (allowedGUIDs.empty())
        return;

    allowedGUIDs.clear();

    // todo database
}

bool Item::checkSoulboundTradeExpire()
{
    uint32_t* time = getOwner()->getPlayedTime();

    if (getCreatePlayedTime() + 2 * HOUR < time[1])
    {
        clearSoulboundTradeable(getOwner());
        return true; // remove from tradeable list
    }

    return false;
}
#endif

bool Item::isTradeableWith(Player* plr)
{
    if (hasFlags(ITEM_FLAG_BOP_TRADEABLE))
        if (allowedGUIDs.find(plr->getGuidLow()) != allowedGUIDs.end())
            return true;

    if (isSoulbound())
        return false;

    if (getItemProperties()->Bonding == ITEM_BIND_QUEST)
        return false;

    return true;
}

#if VERSION_STRING == Cata
int32_t Item::getReforgableStat(ItemModType statType) const
{
    ItemProperties const* proto = getItemProperties();
    for (uint32_t i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        if (ItemModType(proto->Stats[i].Type) == statType)
            return proto->Stats[i].Value;
    
    int32_t randomPropId = getRandomPropertiesId();
    if (!randomPropId)
        return 0;

    if (randomPropId < 0)
    {
        DBC::Structures::ItemRandomSuffixEntry const* randomSuffix = sItemRandomSuffixStore.LookupEntry(-randomPropId);
        if (!randomSuffix)
            return 0;

        for (uint32_t e = PROP_ENCHANTMENT_SLOT_0; e <= PROP_ENCHANTMENT_SLOT_4; ++e)
            if (DBC::Structures::SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(getEnchantmentId(EnchantmentSlot(e))))
                for (uint8_t f = 0; f < MAX_ITEM_ENCHANTMENT_EFFECTS; ++f)
                    if (enchant->type[f] == ITEM_ENCHANTMENT_TYPE_STAT && ItemModType(enchant->spell[f]) == statType)
                        for (uint8_t k = 0; k < 5; ++k)
                            if (randomSuffix->enchantments[k] == enchant->Id)
                                return int32_t((randomSuffix->prefixes[k] * getPropertySeed()) / 10000);
    }
    else
    {
        DBC::Structures::ItemRandomPropertiesEntry const* randomProp = sItemRandomPropertiesStore.LookupEntry(randomPropId);
        if (!randomProp)
            return 0;

        for (uint32_t e = PROP_ENCHANTMENT_SLOT_0; e <= PROP_ENCHANTMENT_SLOT_4; ++e)
            if (DBC::Structures::SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(getEnchantmentId(EnchantmentSlot(e))))
                for (uint8_t f = 0; f < MAX_ITEM_ENCHANTMENT_EFFECTS; ++f)
                    if (enchant->type[f] == ITEM_ENCHANTMENT_TYPE_STAT && ItemModType(enchant->spell[f]) == statType)
                        for (uint8_t k = 0; k < MAX_ITEM_ENCHANTMENT_EFFECTS; ++k)
                            if (randomProp->spells[k] == enchant->Id)
                                return int32_t(enchant->min[k]);
    }

    return 0;
}
#endif
