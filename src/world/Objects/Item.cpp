/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Item.hpp"
#include "Container.hpp"
#include "Data/Flags.hpp"
#include "Logging/Logger.hpp"
#include "Management/Charter.hpp"
#include "Management/ItemInterface.h"
#include "Management/Guild/Guild.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Definitions.h"
#include "Server/EventMgr.h"
#include "Server/Packets/SmsgEnchantmentLog.h"
#include "Server/Packets/SmsgItemEnchantmentTimeUpdate.h"
#include "Server/Packets/SmsgItemTimeUpdate.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Strings.hpp"

using namespace AscEmu::Packets;

Item::Item() : m_loot(nullptr)
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
    sEventMgr.RemoveEvents(this);

    m_enchantments.clear();

    if (m_owner != nullptr)
    {
        m_owner->getItemInterface()->RemoveRefundable(getGuid());
        m_owner->getItemInterface()->removeTemporaryEnchantedItem(this);
#if VERSION_STRING >= WotLK
        m_owner->getItemInterface()->removeTradeableItem(this);
#endif
    }

    if (IsInWorld())
        removeFromWorld();

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
        sLogger.failure("Item::create: Can't create item {} missing properties!", itemId);
        return;
    }

    for (uint8_t i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        setSpellCharges(i, m_itemProperties->Spells[i].Charges);

    setDurability(m_itemProperties->MaxDurability);
    setMaxDurability(m_itemProperties->MaxDurability);

    m_owner = owner;
    if (m_itemProperties->LockId > 1)
        m_isLocked = true;
    else
        m_isLocked = false;
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
void Item::setTextId(const uint32_t textId)
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
    m_randomSuffix = seed;
}

uint32_t Item::getRandomPropertiesId() const { return itemData()->random_properties_id; }
void Item:: setRandomPropertiesId(uint32_t id)
{
    write(itemData()->random_properties_id, id);
    m_randomProperties = id;
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
// Override Object functions

Unit* Item::getUnitOwner()
{
    return m_owner;
}

Unit const* Item::getUnitOwner() const
{
    return m_owner;
}

Player* Item::getPlayerOwner()
{
    return m_owner;
}

Player const* Item::getPlayerOwner() const
{
    return m_owner;
}

//////////////////////////////////////////////////////////////////////////////////////////
// m_enchantments
EnchantmentInstance* Item::getEnchantment(EnchantmentSlot slot)
{
    auto itr = m_enchantments.find(slot);
    return itr != m_enchantments.end() ? &itr->second : nullptr;
}

EnchantmentInstance const* Item::getEnchantment(EnchantmentSlot slot) const
{
    auto itr = m_enchantments.find(slot);
    return itr != m_enchantments.end() ? &itr->second : nullptr;
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

    WDB::Structures::SpellItemEnchantmentEntry const* Enchantment = nullptr;
#if VERSION_STRING >= Cata
    std::unique_ptr<WDB::Structures::SpellItemEnchantmentEntry> custom_enchant = nullptr;
    switch (slot)
    {

        case TRANSMOGRIFY_ENCHANTMENT_SLOT:
        case REFORGE_ENCHANTMENT_SLOT:
        {
            custom_enchant = std::make_unique<WDB::Structures::SpellItemEnchantmentEntry>();
            custom_enchant->Id = enchantmentId;

            Enchantment = custom_enchant.get();
        } break;

        default:
        {
#endif
            const auto spell_item_enchant = sSpellItemEnchantmentStore.lookupEntry(enchantmentId);
            if (spell_item_enchant == nullptr)
                return false;

            Enchantment = spell_item_enchant;
#if VERSION_STRING >= Cata
        } break;
    }
#endif

    EnchantmentInstance enchantInstance;
    enchantInstance.BonusApplied = false;
    enchantInstance.Slot = slot;
    enchantInstance.Enchantment = Enchantment;
#if VERSION_STRING >= Cata
    enchantInstance.customEnchantmentHolder = std::move(custom_enchant);
#endif
    enchantInstance.RemoveAtLogout = removedAtLogout;
    enchantInstance.RandomSuffix = randomSuffix;

    // Set enchantment to item's wowdata fields
    _setEnchantmentDataFields(slot, Enchantment->Id, duration, 0);

    m_enchantments.try_emplace(slot, std::move(enchantInstance));

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
        applyEnchantmentBonus(slot, true);
    else if (duration)
        sendEnchantTimeUpdate(slot, duration / 1000);

    return true;
}

void Item::removeEnchantment(EnchantmentSlot slot, bool timerExpired/* = false*/)
{
    const auto itr = m_enchantments.find(slot);
    if (itr == m_enchantments.end())
        return;

    // Remove enchantment bonus
    if (itr->second.BonusApplied)
        applyEnchantmentBonus(slot, false);

    _setEnchantmentDataFields(slot, 0, 0, 0);
    m_enchantments.erase(itr);

    if (!timerExpired)
        m_owner->getItemInterface()->removeTemporaryEnchantedItem(this, slot);
}

void Item::modifyEnchantmentTime(EnchantmentSlot slot, uint32_t duration)
{
    auto itr = m_enchantments.find(slot);
    if (itr == m_enchantments.end())
        return;

    setEnchantmentDuration(slot, duration);
    sendEnchantTimeUpdate(itr->second.Slot, duration / 1000);
}

void Item::applyAllEnchantmentBonuses()
{
    for (auto itr = m_enchantments.cbegin(); itr != m_enchantments.cend();)
    {
        auto itr2 = itr++;
        applyEnchantmentBonus(itr2->first, true);
    }
}

void Item::removeAllEnchantmentBonuses()
{
    for (auto itr = m_enchantments.cbegin(); itr != m_enchantments.cend();)
    {
        auto itr2 = itr++;
        applyEnchantmentBonus(itr2->first, false);
    }
}

void Item::removeAllEnchantments(bool onlyTemporary)
{
    for (auto itr = m_enchantments.cbegin(); itr != m_enchantments.cend();)
    {
        auto itr2 = itr++;
        if (onlyTemporary && getEnchantmentDuration(itr2->second.Slot) == 0)
            continue;

        removeEnchantment(itr2->first);
    }
}

void Item::removeSocketBonusEnchant()
{
    for (const auto& enchantment : m_enchantments)
    {
        if (enchantment.second.Enchantment->Id == getItemProperties()->SocketBonus)
        {
            removeEnchantment(enchantment.first);
            return;
        }
    }
}

void Item::removeRelatedEnchants(WDB::Structures::SpellItemEnchantmentEntry const* newEnchant)
{
    for (EnchantmentMap::iterator itr = m_enchantments.begin(); itr != m_enchantments.end();)
    {
        EnchantmentMap::iterator itr2 = itr++;
        if (itr2->second.Enchantment->Id == newEnchant->Id || (itr2->second.Enchantment->EnchantGroups > 1 && newEnchant->EnchantGroups > 1))
            removeEnchantment(itr2->first);
    }
}

void Item::applyEnchantmentBonus(EnchantmentSlot slot, bool apply)
{
    if (m_owner == nullptr)
        return;

    const auto enchantment = m_enchantments.find(slot);
    if (enchantment == m_enchantments.end())
        return;

    WDB::Structures::SpellItemEnchantmentEntry const* Entry = enchantment->second.Enchantment;
    const uint32_t RandomSuffixAmount = enchantment->second.RandomSuffix;

    if (enchantment->second.BonusApplied == apply)
        return;

    enchantment->second.BonusApplied = apply;

    if (apply)
    {
        if (const auto duration = getEnchantmentDuration(enchantment->second.Slot) / 1000)
            sendEnchantTimeUpdate(enchantment->second.Slot, duration);
    }

    uint32_t itemSlot = m_owner->getItemInterface()->GetInventorySlotByGuid(getGuid());
    if (itemSlot < EQUIPMENT_SLOT_END)
    {
#if VERSION_STRING >= WotLK
        if (slot == PERM_ENCHANTMENT_SLOT || slot == TEMP_ENCHANTMENT_SLOT)
#else
        if (slot < MAX_INSPECTED_ENCHANTMENT_SLOT)
#endif
            m_owner->setVisibleItemEnchantment(itemSlot, slot, (apply ? static_cast<uint16_t>(Entry->Id) : 0));
    }
    else if (apply)
    {
        sLogger.failure("Item::applyEnchantmentBonus : Tried to apply visual enchantment but equipment slot {} is invalid", itemSlot);
    }

#if VERSION_STRING >= Cata
    if (slot == REFORGE_ENCHANTMENT_SLOT)
        m_owner->applyReforgeEnchantment(this, apply);
#endif

    for (uint32_t c = 0; c < 3; c++)
    {
        if (Entry->type[c])
        {
            switch (Entry->type[c])
            {
                case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:
                {
                    if (apply)
                    {
                        if (Entry->spell[c] != 0)
                        {
                            const auto procChance = Entry->min[c] == 0 ? Util::float2int32(static_cast<float>(getItemProperties()->Delay) * 0.001f / 60.0f * 100.0f) : Entry->min[c];
                            switch (m_owner->getItemInterface()->GetInventorySlotByGuid(getGuid()))
                            {
                                case EQUIPMENT_SLOT_MAINHAND:
                                    m_owner->addProcTriggerSpell(Entry->spell[c], 0, m_owner->getGuid(), procChance, static_cast<SpellProcFlags>(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT), EXTRA_PROC_ON_MAIN_HAND_HIT_ONLY, nullptr, nullptr, nullptr, this);
                                    break;
                                case EQUIPMENT_SLOT_OFFHAND:
                                    m_owner->addProcTriggerSpell(Entry->spell[c], 0, m_owner->getGuid(), procChance, static_cast<SpellProcFlags>(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT | PROC_ON_DONE_OFFHAND_ATTACK), EXTRA_PROC_ON_OFF_HAND_HIT_ONLY, nullptr, nullptr, nullptr, this);
                                    break;
                                case EQUIPMENT_SLOT_RANGED:
                                    m_owner->addProcTriggerSpell(Entry->spell[c], 0, m_owner->getGuid(), procChance, static_cast<SpellProcFlags>(PROC_ON_DONE_RANGED_HIT | PROC_ON_DONE_RANGED_SPELL_HIT), EXTRA_PROC_NULL, nullptr, nullptr, nullptr, this);
                                    break;
                                default:
                                    m_owner->addProcTriggerSpell(Entry->spell[c], 0, m_owner->getGuid(), procChance, static_cast<SpellProcFlags>(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT), EXTRA_PROC_NULL, nullptr, nullptr, nullptr, this);
                                    break;
                            }
                        }
                    }
                    else
                    {
                        m_owner->removeProcTriggerSpell(Entry->spell[c], m_owner->getGuid(), getGuid());
                    }
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_DAMAGE:
                {
                    int32_t val = Entry->min[c];
                    if (RandomSuffixAmount)
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                    if (apply)
                        m_owner->modModDamageDonePositive(SCHOOL_NORMAL, val);
                    else
                        m_owner->modModDamageDonePositive(SCHOOL_NORMAL, -val);

                    m_owner->calculateDamage();
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
                {
                    if (apply)
                    {
                        SpellCastTargets targets(m_owner->getGuid());

                        if (Entry->spell[c] != 0)
                        {
                            SpellInfo const* sp = sSpellMgr.getSpellInfo(Entry->spell[c]);
                            if (sp == nullptr)
                                continue;

                            Spell* spell = sSpellMgr.newSpell(m_owner, sp, true, nullptr);
                            spell->setItemCaster(this);
                            spell->prepare(&targets);
                        }
                    }
                    else
                    {
                        if (Entry->spell[c] != 0)
                            m_owner->removeAuraByItemGuid(Entry->spell[c], getGuid());
                    }
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_RESISTANCE:
                {
                    int32_t val = Entry->min[c];
                    if (RandomSuffixAmount)
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                    if (apply)
                        m_owner->m_flatResistanceModifierPos[Entry->spell[c]] += val;
                    else
                        m_owner->m_flatResistanceModifierPos[Entry->spell[c]] -= val;

                    m_owner->calcResistance(static_cast<uint8_t>(Entry->spell[c]));
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_STAT:
                {
                    int32_t val = Entry->min[c];
                    if (RandomSuffixAmount)
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                    m_owner->modifyBonuses(Entry->spell[c], val, apply);
                    m_owner->updateStats();
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_TOTEM:
                {
                    if (apply)
                    {
                        int32_t val = Entry->min[c];
                        if (RandomSuffixAmount)
                            val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                        int32_t value = static_cast<int32_t>(getItemProperties()->Delay) * val / 1000;
                        m_owner->modModDamageDonePositive(SCHOOL_NORMAL, value);
                    }
                    else
                    {
                        int32_t val = Entry->min[c];
                        if (RandomSuffixAmount)
                            val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                        int32_t value = -static_cast<int32_t>(getItemProperties()->Delay * val / 1000);
                        m_owner->modModDamageDonePositive(SCHOOL_NORMAL, value);
                    }
                    m_owner->calculateDamage();
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_USE_SPELL:
                {
                    for (uint8_t i = 0; i < 3; ++i)
                    {
                        if (apply)
                            m_onUseSpellIds[i] = Entry->spell[i];
                        else
                            m_onUseSpellIds[i] = 0;
                    }
                    break;
                }

                case ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET:
                {
                    break;
                }

                default:
                    sLogger.failure("Unknown enchantment type: {} ({})", Entry->type[c], Entry->Id);
                    break;
                }
        }
    }
}

void Item::sendEnchantTimeUpdate(uint32_t slot, uint32_t duration)
{
    m_owner->sendPacket(SmsgItemEnchantmentTimeUpdate(getGuid(), slot, duration, m_owner->getGuid()).serialise().get());
}

void Item::removeFromRefundableMap()
{
    Player* owner = this->getOwner();
    uint64_t guid = this->getGuid();

    if (owner && guid != 0)
        owner->getItemInterface()->RemoveRefundable(guid);
}

uint32_t Item::getOnUseSpellId(uint32_t index) const { return m_onUseSpellIds[index]; }

bool Item::hasOnUseSpellIid(uint32_t id) const
{
    for (unsigned int m_onUseSpellId : m_onUseSpellIds)
        if (m_onUseSpellId == id)
            return true;

    return false;
}

void Item::setRandomSuffix(uint32_t id)
{
    int32_t r_id = -static_cast<int32_t>(id);
    uint32_t v = generateRandomSuffixFactor(m_itemProperties);
    setRandomPropertiesId(static_cast<uint32_t>(r_id));
    setPropertySeed(v);
    //\todo why override m_randomSuffix set by functions above?
    m_randomSuffix = id;
}

void Item::applyRandomProperties(bool apply)
{
    // apply random properties
    if (getRandomPropertiesId() != 0)
    {
        if (static_cast<int32_t>(getRandomPropertiesId()) > 0)
        {
            auto item_random_properties = sItemRandomPropertiesStore.lookupEntry(getRandomPropertiesId());
            for (uint8_t k = 0; k < 3; k++)
            {
                if (item_random_properties == nullptr)
                    continue;

                if (item_random_properties->spells[k] != 0)
                {
                    auto spell_item_enchant = sSpellItemEnchantmentStore.lookupEntry(item_random_properties->spells[k]);
                    if (spell_item_enchant == nullptr)
                        continue;

                    auto slot = hasEnchantmentReturnSlot(item_random_properties->spells[k]);
                    if (slot < 0)
                    {
                        EnchantmentSlot newSlot = PROP_ENCHANTMENT_SLOT_2;
                        if (_findFreeRandomEnchantmentSlot(&newSlot, RandomEnchantmentType::PROPERTY))
                            addEnchantment(item_random_properties->spells[k], newSlot, 0, true);
                    }
                    else if (apply)
                    {
                        applyEnchantmentBonus(static_cast<EnchantmentSlot>(slot), true);
                    }
                }
            }
        }
        else
        {
            auto item_random_suffix = sItemRandomSuffixStore.lookupEntry(abs(int(getRandomPropertiesId())));
            for (uint8_t k = 0; k < 3; ++k)
            {
                if (item_random_suffix == nullptr)
                    continue;

                if (item_random_suffix->enchantments[k] != 0)
                {
                    auto spell_item_enchant = sSpellItemEnchantmentStore.lookupEntry(item_random_suffix->enchantments[k]);
                    if (spell_item_enchant == nullptr)
                        continue;

                    auto slot = hasEnchantmentReturnSlot(spell_item_enchant->Id);
                    if (slot < 0)
                    {
                        EnchantmentSlot newSlot = PROP_ENCHANTMENT_SLOT_0;
                        if (_findFreeRandomEnchantmentSlot(&newSlot, RandomEnchantmentType::SUFFIX))
                            addEnchantment(item_random_suffix->enchantments[k], newSlot, 0, true, item_random_suffix->prefixes[k]);
                    }
                    else if (apply)
                    {
                        applyEnchantmentBonus(static_cast<EnchantmentSlot>(slot), true);
                    }
                }
            }
        }
    }
}

uint32_t Item::generateRandomSuffixFactor(ItemProperties const* m_itemProto)
{
    const double SuffixMods[29] =
    {
        0.0,
        0.46,        // HEAD
        0.26,        // NECK
        0.35,        // SHOULDERS
        0.46,        // BODY
        0.46,        // CHEST
        0.35,        // WAIST
        0.46,        // LEGS
        0.34,        // FEET
        0.26,        // WRISTS
        0.35,        // HANDS
        0.26,        // FINGER
        0.0,         // TRINKET
        0.19,        // WEAPON
        0.25,        // SHEILD
        0.14,        // RANGED
        0.26,        // CLOAK
        0.46,        // 2H-WEAPON
        0.0,         // BAG
        0.0,         // TABARD
        0.46,        // ROBE
        0.19,        // MAIN-HAND WEAPON
        0.19,        // OFF-HAND WEAPON
        0.26,        // HOLDABLE
        0.0,         // AMMO
        0.26,        // THROWN
        0.14,        // RANGED
        0.0,         // QUIVER
        0.26,        // RELIC
    };

    double value;

    if (m_itemProto->Class == ITEM_CLASS_ARMOR && m_itemProto->Quality > ITEM_QUALITY_UNCOMMON_GREEN)
        value = SuffixMods[m_itemProto->InventoryType] * 1.24;
    else
        value = SuffixMods[m_itemProto->InventoryType];

    value = value * static_cast<double>(m_itemProto->ItemLevel) + 0.5;
    return Util::long2int32(value);
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

uint32_t Item::countGemsWithLimitId(uint32_t limitId)
{
#if VERSION_STRING > Classic
    uint32_t result = 0;
    for (uint8_t count = 0; count < getSocketSlotCount(); count++)
    {
        const auto ei = getEnchantment(static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT1 + count));

        //huh ? Gem without entry ?
        if (ei && ei->Enchantment->GemEntry)
        {
            ItemProperties const* ip = sMySQLStore.getItemProperties(ei->Enchantment->GemEntry);
            if (ip && ip->ItemLimitCategory == limitId)
                result++;
        }
    }
    return result;
#else
    return 0;
#endif
}

bool Item::isGemRelated(WDB::Structures::SpellItemEnchantmentEntry const* enchantment)
{
#if VERSION_STRING > Classic
    if (getItemProperties()->SocketBonus == enchantment->Id)
        return true;

    return enchantment->GemEntry != 0;
#else
    return 0;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Durability
time_t Item::getItemExpireTime() const { return m_expiresOnTime; }
void Item::setItemExpireTime(time_t timesec) { m_expiresOnTime = timesec; }

void Item::setDurabilityToMax() { setDurability(getMaxDurability()); }

void Item::sendDurationUpdate()
{
    m_owner->sendPacket(SmsgItemTimeUpdate(this->getGuid(), this->getDuration()).serialise().get());
}

bool Item::repairItem(Player* player, bool isGuildMoney, int32_t* repairCost /*= nullptr*/)
{
    const uint32_t cost = repairItemCost();
    if (cost == 0)
        return false;

    if (isGuildMoney && player->isInGuild())
    {
        if (!player->getGuild()->handleMemberWithdrawMoney(player->getSession(), cost, true))
            return false;

        if (repairCost != nullptr)
            *repairCost += static_cast<int32_t>(cost);
    }
    else
    {
        if (!player->hasEnoughCoinage(cost))
            return false;

        player->modCoinage(-static_cast<int32_t>(cost));
    }
    setDurabilityToMax();
    m_isDirty = true;

    return true;
}

uint32_t Item::repairItemCost()
{
    auto durability_costs = sDurabilityCostsStore.lookupEntry(m_itemProperties->ItemLevel);
    if (durability_costs == nullptr)
    {
        sLogger.failure("Repair: Unknown item level ({})", fmt::ptr(durability_costs));
        return 0;
    }

    auto durability_quality = sDurabilityQualityStore.lookupEntry((m_itemProperties->Quality + 1) * 2);
    if (durability_quality == nullptr)
    {
        sLogger.failure("Repair: Unknown item quality ({})", fmt::ptr(durability_quality));
        return 0;
    }

    uint32_t dmodifier = durability_costs->modifier[m_itemProperties->Class == ITEM_CLASS_WEAPON ? m_itemProperties->SubClass : m_itemProperties->SubClass + 21];
    uint32_t cost = Util::long2int32((getMaxDurability() - getDurability()) * dmodifier * double(durability_quality->quality_modifier));
    return cost;
}

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
#if VERSION_STRING < WotLK
    const auto isEnchantSpell = spellInfo->hasEffect(SPELL_EFFECT_ENCHANT_ITEM) || spellInfo->hasEffect(SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY);
#else
    const auto isEnchantSpell = spellInfo->hasEffect(SPELL_EFFECT_ENCHANT_ITEM) || spellInfo->hasEffect(SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) || spellInfo->hasEffect(SPELL_EFFECT_ADD_SOCKET);
#endif

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

bool Item::isTradeableWith(Player* player)
{
    if (hasFlags(ITEM_FLAG_BOP_TRADEABLE))
        if (allowedGUIDs.find(player->getGuidLow()) != allowedGUIDs.end())
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
        WDB::Structures::ItemRandomSuffixEntry const* randomSuffix = sItemRandomSuffixStore.lookupEntry(-randomPropId);
        if (!randomSuffix)
            return 0;

        for (uint32_t e = PROP_ENCHANTMENT_SLOT_0; e <= PROP_ENCHANTMENT_SLOT_4; ++e)
            if (WDB::Structures::SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.lookupEntry(getEnchantmentId(EnchantmentSlot(e))))
                for (uint8_t f = 0; f < MAX_ITEM_ENCHANTMENT_EFFECTS; ++f)
                    if (enchant->type[f] == ITEM_ENCHANTMENT_TYPE_STAT && ItemModType(enchant->spell[f]) == statType)
                        for (uint8_t k = 0; k < 5; ++k)
                            if (randomSuffix->enchantments[k] == enchant->Id)
                                return int32_t((randomSuffix->prefixes[k] * getPropertySeed()) / 10000);
    }
    else
    {
        WDB::Structures::ItemRandomPropertiesEntry const* randomProp = sItemRandomPropertiesStore.lookupEntry(randomPropId);
        if (!randomProp)
            return 0;

        for (uint32_t e = PROP_ENCHANTMENT_SLOT_0; e <= PROP_ENCHANTMENT_SLOT_4; ++e)
            if (WDB::Structures::SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.lookupEntry(getEnchantmentId(EnchantmentSlot(e))))
                for (uint8_t f = 0; f < MAX_ITEM_ENCHANTMENT_EFFECTS; ++f)
                    if (enchant->type[f] == ITEM_ENCHANTMENT_TYPE_STAT && ItemModType(enchant->spell[f]) == statType)
                        for (uint8_t k = 0; k < MAX_ITEM_ENCHANTMENT_EFFECTS; ++k)
                            if (randomProp->spells[k] == enchant->Id)
                                return int32_t(enchant->min[k]);
    }

    return 0;
}
#endif

uint8_t Item::getCharterTypeForEntry() const
{
    uint8_t charterType;
    switch (getEntry())
    {
        case CharterEntry::Guild:
            charterType = CHARTER_TYPE_GUILD;
            break;
        case CharterEntry::TwoOnTwo:
            charterType = CHARTER_TYPE_ARENA_2V2;
            break;
        case CharterEntry::ThreeOnThree:
            charterType = CHARTER_TYPE_ARENA_3V3;
            break;
        case CharterEntry::FiveOnFive:
            charterType = CHARTER_TYPE_ARENA_5V5;
            break;
        default:
            charterType = NUM_CHARTER_TYPES;
            break;
    }

    return charterType;
}

void Item::loadFromDB(Field* fields, Player* plr, bool light)
{
    uint32_t itemid = fields[2].asUint32();

    m_itemProperties = sMySQLStore.getItemProperties(itemid);
    if (!m_itemProperties)
    {
        sLogger.failure("Item::loadFromDB: Can't load item {} missing properties!", itemid);
        return;
    }

    if (m_itemProperties->LockId > 1)
        m_isLocked = true;
    else
        m_isLocked = false;

    setEntry(itemid);
    m_owner = plr;

    m_wrappedItemId = fields[3].asUint32();
    setGiftCreatorGuid(fields[4].asUint32());
    setCreatorGuid(fields[5].asUint32());

    uint32_t count = fields[6].asUint32();
    if (count > m_itemProperties->MaxCount && (m_owner && !m_owner->m_cheats.hasItemStackCheat))
        count = m_itemProperties->MaxCount;
    setStackCount(count);

    setChargesLeft(fields[7].asUint32());

    setFlags(fields[8].asUint32());
    uint32_t randomProp = fields[9].asUint32();
    const uint32_t randomSuffix = fields[10].asUint32();

    setRandomPropertiesId(randomProp);

    const int32_t rprop = static_cast<int32_t>(randomProp);
    if (rprop < 0)
        setPropertySeed(randomSuffix);
    else
        setPropertySeed(0);

#ifdef AE_TBC
    setTextId(fields[11].asUint32());
#endif

    setMaxDurability(m_itemProperties->MaxDurability);
    setDurability(fields[12].asUint32());

    if (light)
        return;

    std::string enchant_field = fields[15].asCString();
    if (!enchant_field.empty())
    {
        std::vector<std::string> enchants = AscEmu::Util::Strings::split(enchant_field, ";");
        uint32_t enchant_id;

        uint32_t time_left;
        uint32_t enchslot;

        for (auto& enchant : enchants)
        {
            if (sscanf(enchant.c_str(), "%u,%u,%u", &enchant_id, &time_left, &enchslot) == 3)
            {
                if (enchant_id)
                    addEnchantment(enchant_id, static_cast<EnchantmentSlot>(enchslot), time_left);
            }
        }
    }

    m_expiresOnTime = fields[16].asUint32();

    // Refund stuff
    std::pair<time_t, uint32_t> refundentry;
    refundentry.first = fields[17].asUint32();
    refundentry.second = fields[18].asUint32();

    if (refundentry.first != 0 && refundentry.second != 0 && getOwner() != nullptr)
    {
        uint32_t* played = getOwner()->getPlayedTime();
        if (played[1] < (refundentry.first + 60 * 60 * 2))
            m_owner->getItemInterface()->AddRefundable(this, refundentry.second, refundentry.first);
    }

    m_text = fields[19].asCString();

    applyRandomProperties(false);

    // Charter stuff
    const uint8_t charterType = getCharterTypeForEntry();
    if (charterType < NUM_CHARTER_TYPES)
    {
        addFlags(ITEM_FLAG_SOULBOUND);
        setStackCount(1);
        setPropertySeed(57813883);
        if (plr && plr->getCharter(charterType))
            setEnchantmentId(0, plr->getCharter(charterType)->getId());
    }
}

void Item::saveToDB(int8_t containerslot, int8_t slot, bool firstsave, QueryBuffer* buf)
{
    if (!m_isDirty && !firstsave)
        return;

    const uint64_t GiftCreatorGUID = getGiftCreatorGuid();
    const uint64_t CreatorGUID = getCreatorGuid();

    std::stringstream ss;
    ss << "DELETE FROM `playeritems` WHERE guid = " << getGuidLow() << ";";

    if (firstsave)
    {
        CharacterDatabase.WaitExecute(ss.str().c_str());
    }
    else
    {
        if (buf == nullptr)
            CharacterDatabase.Execute(ss.str().c_str());
        else
            buf->AddQueryNA(ss.str().c_str());
    }


    ss.rdbuf()->str("");

    ss << "INSERT INTO `playeritems` VALUES(";

    ss << getOwnerGuidLow() << ",";
    ss << getGuidLow() << ",";
    ss << getEntry() << ",";
    ss << m_wrappedItemId << ",";
    ss << WoWGuid::getGuidLowPartFromUInt64(GiftCreatorGUID) << ",";
    ss << WoWGuid::getGuidLowPartFromUInt64(CreatorGUID) << ",";

    ss << getStackCount() << ",";
    ss << static_cast<int32_t>(getChargesLeft()) << ",";
    ss << getFlags() << ",";
    ss << m_randomProperties << ", " << m_randomSuffix << ", ";
    ss << 0 << ",";
    ss << getDurability() << ",";
    ss << static_cast<int>(containerslot) << ",";
    ss << static_cast<int>(slot) << ",'";

    if (!m_enchantments.empty())
    {
        for (const auto& Enchantment : m_enchantments)
        {
            if (Enchantment.second.RemoveAtLogout)
                continue;

            if (getEnchantmentId(Enchantment.second.Slot) != Enchantment.second.Enchantment->Id)
                continue;

            const auto timeLeft = getEnchantmentDuration(Enchantment.second.Slot);
            if (Enchantment.second.Enchantment && (timeLeft > 5000 || timeLeft == 0))
            {
                ss << Enchantment.second.Enchantment->Id << ",";
                ss << timeLeft << ",";
                ss << static_cast<int>(Enchantment.second.Slot) << ";";
            }
        }
    }
    ss << "','";
    ss << m_expiresOnTime << "','";

    // Refund stuff
    if (this->getOwner() != nullptr)
    {
        std::pair<time_t, uint32_t> refundentry = this->getOwner()->getItemInterface()->LookupRefundable(this->getGuid());

        ss << static_cast<uint32_t>(refundentry.first) << "','";
        ss << static_cast<uint32_t>(refundentry.second);
    }
    else
    {
        ss << static_cast<uint32_t>(0) << "','";
        ss << static_cast<uint32_t>(0);
    }

    ss << "','";
    ss << m_text;
    ss << "')";

    if (firstsave)
    {
        CharacterDatabase.WaitExecute(ss.str().c_str());
    }
    else
    {
        if (buf == nullptr)
            CharacterDatabase.Execute(ss.str().c_str());
        else
            buf->AddQueryNA(ss.str().c_str());
    }

    m_isDirty = false;
}

void Item::deleteFromDB()
{
    if (m_itemProperties->ContainerSlots > 0 && isContainer())
    {
        for (uint32_t i = 0; i < m_itemProperties->ContainerSlots; ++i)
        {
            if (dynamic_cast<Container*>(this)->getItem(static_cast<int16_t>(i)) != nullptr)
                return;
        }
    }

    CharacterDatabase.Execute("DELETE FROM playeritems WHERE guid = %u", getGuidLow());
}

const static uint16_t arm_skills[7] =
{
    0,
    SKILL_CLOTH,
    SKILL_LEATHER,
    SKILL_MAIL,
    SKILL_PLATE_MAIL,
    0,
    SKILL_SHIELD
};

const static uint16_t weap_skills[21] =
{
    SKILL_AXES,
    SKILL_2H_AXES,
    SKILL_BOWS,
    SKILL_GUNS,
    SKILL_MACES,
    SKILL_2H_MACES,
    SKILL_POLEARMS,
    SKILL_SWORDS,
    SKILL_2H_SWORDS,
    0,
    SKILL_STAVES,
    0,
    0,
    SKILL_FIST_WEAPONS,
    0, // 13
    SKILL_DAGGERS,
#if VERSION_STRING <= Cata
    SKILL_THROWN,
    SKILL_ASSASSINATION,
#else
    0,
    0,
#endif
    SKILL_CROSSBOWS,
    SKILL_WANDS,
    SKILL_FISHING
};

uint16_t Item::getRequiredSkill() const
{
    if (m_itemProperties->Class == 4 && m_itemProperties->SubClass < 7)
    {
        return arm_skills[m_itemProperties->SubClass];
    }

    if (m_itemProperties->Class == 2)
    {
        if (m_itemProperties->SubClass < 20)  //no skill for fishing
            return weap_skills[m_itemProperties->SubClass];
    }

    return 0;
}

uint32_t Item::getSellPrice(uint32_t count)
{
    int32_t cost = m_itemProperties->SellPrice * ((count < 1) ? 1 : count);
    return cost;
}

void Item::removeFromWorld()
{
    if (m_owner != nullptr)
        m_owner->sendDestroyObjectPacket(getGuid());

    if (!IsInWorld())
        return;

    m_WorldMap->RemoveObject(this, false);
    m_WorldMap = nullptr;

    event_Relocate();
}

void Item::eventRemoveItem()
{
    if (this->getOwner())
        m_owner->getItemInterface()->SafeFullRemoveItemByGuid(this->getGuid());
}

bool Item::isEligibleForRefund()
{
    ItemProperties const* proto = this->getItemProperties();
    if (!(proto->Flags & ITEM_FLAG_REFUNDABLE))
        return false;

    if (proto->MaxCount > 1)
        return false;

    for (const auto spell : proto->Spells)
    {
        if (spell.Charges != -1 && spell.Charges != 0)
            return false;
    }

    return true;
}

uint32_t Item::getChargesLeft() const
{
    for (uint8_t x = 0; x < 5; ++x)
        if ((m_itemProperties->Spells[x].Id != 0) && (m_itemProperties->Spells[x].Trigger == USE))
            return getSpellCharges(x) > 0 ? getSpellCharges(x) : 0;

    return 0;
}

void Item::setChargesLeft(uint32_t charges)
{
    for (uint8_t x = 0; x < 5; ++x)
    {
        if ((m_itemProperties->Spells[x].Id != 0) && (m_itemProperties->Spells[x].Trigger == USE))
        {
            setSpellCharges(x, charges);
            break;
        }
    }
}
