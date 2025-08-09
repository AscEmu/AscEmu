/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ItemDefines.hpp"
#include "Management/ItemProperties.hpp"
#include "Objects/Object.hpp"
#include "Management/Loot/LootDefines.hpp"
#include "Data/WoWItem.hpp"
#include "Server/UpdateFieldInclude.h"

class QueryBuffer;
class Field;
struct Loot;

namespace WDB::Structures
{
    struct SpellItemEnchantmentEntry;
}

class Container;

struct EnchantmentInstance
{
    // Durations for temporary enchantments are stored in ItemInterface and WoWItem data
    WDB::Structures::SpellItemEnchantmentEntry const* Enchantment;
#if VERSION_STRING >= Cata
    std::unique_ptr<WDB::Structures::SpellItemEnchantmentEntry> customEnchantmentHolder;
#endif
    bool BonusApplied;
    EnchantmentSlot Slot;
    bool RemoveAtLogout;
    uint32_t RandomSuffix;
};

typedef std::map<EnchantmentSlot, EnchantmentInstance> EnchantmentMap;

struct WoWItem;
class SERVER_DECL Item : public Object
{
public:
    Item();
    virtual ~Item();

private:
    const WoWItem* itemData() const { return reinterpret_cast<WoWItem*>(wow_data); }
public:
    void init(uint32_t high, uint32_t low);
    void create(uint32_t itemId, Player* owner);

    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    uint64_t getOwnerGuid() const;
    uint32_t getOwnerGuidLow() const;
    uint32_t getOwnerGuidHigh() const;
    void setOwnerGuid(uint64_t guid);

    void setContainerGuid(uint64_t guid);
    uint64_t getContainerGuid() const;

    uint64_t getCreatorGuid() const;
    void setCreatorGuid(uint64_t guid);

    uint64_t getGiftCreatorGuid() const;
    void setGiftCreatorGuid(uint64_t guid);

    uint32_t getStackCount() const;
    void setStackCount(uint32_t count);
    void modStackCount(int32_t mod);

    uint32_t getDuration() const;
    void setDuration(uint32_t seconds);

    int32_t getSpellCharges(uint8_t index) const;
    void setSpellCharges(uint8_t index, int32_t count);
    void modSpellCharges(uint8_t index, int32_t mod);

    uint32_t getFlags() const;
    void setFlags(uint32_t flags);
    void addFlags(uint32_t flags);
    void removeFlags(uint32_t flags);
    bool hasFlags(uint32_t flags) const;

    uint32_t getEnchantmentId(uint8_t index) const;
    void setEnchantmentId(uint8_t index, uint32_t id);

    uint32_t getEnchantmentDuration(uint8_t index) const;
    void setEnchantmentDuration(uint8_t index, uint32_t duration);

    uint32_t getEnchantmentCharges(uint8_t index) const;
    void setEnchantmentCharges(uint8_t index, uint32_t charges);

    uint32_t getPropertySeed() const;
    void setPropertySeed(uint32_t seed);

    uint32_t getRandomPropertiesId() const;
    void setRandomPropertiesId(uint32_t id);

    uint32_t getDurability() const;
    void setDurability(uint32_t durability);

    uint32_t getMaxDurability() const;
    void setMaxDurability(uint32_t maxDurability);

#if VERSION_STRING >= WotLK
    uint32_t getCreatePlayedTime() const;
    void setCreatePlayedTime(uint32_t time);
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions

    // Returns unit owner
    Unit* getUnitOwner() override;
    // Returns unit owner
    Unit const* getUnitOwner() const override;
    // Returns player owner
    Player* getPlayerOwner() override;
    // Returns player owner
    Player const* getPlayerOwner() const override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // m_enchantments
    EnchantmentInstance* getEnchantment(EnchantmentSlot slot);
    EnchantmentInstance const* getEnchantment(EnchantmentSlot slot) const;
    bool hasEnchantment(uint32_t enchantmentId) const;
    bool hasEnchantments() { return m_enchantments.size() > 0 ? true : false; }

    int16_t hasEnchantmentReturnSlot(uint32_t enchantmentId) const;

    bool addEnchantment(uint32_t enchantmentId, EnchantmentSlot slot, uint32_t duration, bool removedAtLogout = false, uint32_t randomSuffix = 0);
    void removeEnchantment(EnchantmentSlot slot, bool timerExpired = false);

    void modifyEnchantmentTime(EnchantmentSlot slot, uint32_t duration);

    void applyAllEnchantmentBonuses();
    void removeAllEnchantmentBonuses();

    void removeAllEnchantments(bool onlyTemporary);
    void removeSocketBonusEnchant();

    void removeRelatedEnchants(WDB::Structures::SpellItemEnchantmentEntry const* newEnchant);
    void applyEnchantmentBonus(EnchantmentSlot slot, bool apply);
    void sendEnchantTimeUpdate(uint32_t slot, uint32_t duration);

    uint32_t getOnUseSpellId(uint32_t index) const;
    bool hasOnUseSpellIid(uint32_t id) const;

    void setRandomSuffix(uint32_t id);

    void applyRandomProperties(bool apply);
    static uint32_t generateRandomSuffixFactor(ItemProperties const* m_itemProto);

private:
    void _setEnchantmentDataFields(EnchantmentSlot slot, uint32_t enchantmentId, uint32_t duration, uint32_t charges);
    bool _findFreeRandomEnchantmentSlot(EnchantmentSlot* slot, RandomEnchantmentType randomType) const;

    uint32_t m_onUseSpellIds[3] = { 0 };                    // Enchant type 3 spellids, like engineering gadgets appliable to items.

protected:
    EnchantmentMap m_enchantments;
    uint32_t m_randomProperties = 0;
    uint32_t m_randomSuffix = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Sockets / gems
#if VERSION_STRING >= TBC
    uint8_t getSocketSlotCount(bool includePrismatic = true) const;
#endif

    uint32_t countGemsWithLimitId(uint32_t Limit);
    bool isGemRelated(WDB::Structures::SpellItemEnchantmentEntry const* enchantment);

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Durability
    time_t getItemExpireTime() const;
    void setItemExpireTime(time_t timesec);

    void setDurabilityToMax();
    void sendDurationUpdate();

    bool repairItem(Player* player, bool isGuildMoney = false, int32_t* repairCost = nullptr);
    uint32_t repairItemCost();

protected:
    time_t m_expiresOnTime = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    // TODO: remove this and replace it with virtual Object::getPlayerOwner()
    Player* getOwner() const;
    void setOwner(Player* owner);

    void setDirty() { m_isDirty = true; }

    void setContainer(Container* container);

    ItemProperties const* getItemProperties() const;
    void setItemProperties(ItemProperties const* itemProperties);

protected:
    ItemProperties const* m_itemProperties = nullptr;

public:
    void setText(std::string& textString) { this->m_text = textString; }
    const std::string& getText() const { return this->m_text; }

private:
    std::string m_text;

public:
    bool isSoulbound() const { return hasFlags(ITEM_FLAG_SOULBOUND); }
    bool isAccountbound() const { return hasFlags(ITEM_FLAG_ACCOUNTBOUND); }
    bool isWeapon() const { return getItemProperties()->Class == ITEM_CLASS_WEAPON; }
    bool isAmmoBag() { return m_itemProperties->Class == ITEM_CLASS_QUIVER; }

    bool fitsToSpellRequirements(SpellInfo const* spellInfo) const;

    uint32_t getVisibleEntry() const;

    bool hasStats() const;
    bool canBeTransmogrified() const;
    bool canTransmogrify() const;

    bool isInBag() const;
    bool isEquipped() const;
    bool isTradeableWith(Player* player);
    uint8_t getCharterTypeForEntry() const;

    int32_t getReforgableStat(ItemModType statType) const;
    static bool canTransmogrifyItemWithItem(Item const* transmogrified, Item const* transmogrifier);

#if VERSION_STRING <= TBC
    void setTextId(uint32_t textId);
#endif

    void loadFromDB(Field* fields, Player* plr, bool light);
    void saveToDB(int8_t containerslot, int8_t slot, bool firstsave, QueryBuffer* buf);
    bool loadAuctionItemFromDB(uint64_t guid);
    void deleteFromDB();
    bool isEligibleForRefund();

    uint32_t getChargesLeft() const;
    void setChargesLeft(uint32_t charges);

    uint16_t getRequiredSkill() const;
    uint32_t getSellPrice(uint32_t count);
    void removeFromWorld();

    std::unique_ptr<Loot> m_loot;
    bool m_isLocked = false;
    bool m_isDirty = false;

    uint32_t m_wrappedItemId = 0;

    void eventRemoveItem();
    void removeFromRefundableMap();

#if VERSION_STRING >= WotLK
    // Soulbound trade system
    void setSoulboundTradeable(LooterSet& allowedLooters);
    void clearSoulboundTradeable(Player* currentOwner);
    bool checkSoulboundTradeExpire();
#endif

protected:
    uint32_t _fields[getSizeOfStructure(WoWItem)] = { 0 };  // this mem is wasted in case of container... but this will be fixed in future
    Player* m_owner = nullptr;                              // let's not bother the manager with unneeded requests

private:
    LooterSet allowedGUIDs;
};
