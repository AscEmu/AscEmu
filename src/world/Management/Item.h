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
 */

#ifndef WOWSERVER_ITEM_H
#define WOWSERVER_ITEM_H

#include "Skill.h"
#include "Management/ItemPrototype.h"
#include "Storage/DBC/DBCStructures.hpp"
#include "Objects/Object.h"
#include "WorldConf.h"
#include "LootMgr.h"

class Container;

struct EnchantmentInstance
{
    DBC::Structures::SpellItemEnchantmentEntry const* Enchantment;
    bool BonusApplied;
    //\todo slot is < 255 set to uint8_t and get rid of casts.
    uint32 Slot;
    time_t ApplyTime;
    uint32 Duration;
    bool RemoveAtLogout;
    uint32 RandomSuffix;
};

const static ItemProf prof[22] =
{
    {4, 2}, {4, 4}, {4, 8}, {4, 16}, {4, 64},
    {2, 1}, {2, 2}, {2, 4}, {2, 8}, {2, 16}, {2, 32}, {2, 64}, {2, 128}, {2, 256}, {2, 1024}, {2, 8192}, {2, 32768}, {2, 65536}, {2, 131072},
    {2, 262144}, {2, 524288}, {2, 1048576}
};

const static uint32 arm_skills[7] =
{
    0,
    SKILL_CLOTH,
    SKILL_LEATHER,
    SKILL_MAIL,
    SKILL_PLATE_MAIL,
    0,
    SKILL_SHIELD
};

const static uint32 weap_skills[21] =
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
    SKILL_THROWN,
    SKILL_ASSASSINATION,
    SKILL_CROSSBOWS,
    SKILL_WANDS,
    SKILL_FISHING
};

const static float pricemod[9] =
{
    1.0f,        // HATED
    1.0f,        // HOSTILE
    1.0f,        // UNFRIENDLY
    1.0f,        // NEUTRAL
    0.95f,       // FRIENDLY
    0.90f,       // HONORED
    0.85f,       // REVERED
    0.80f        // EXHALTED
};

const static double SuffixMods[NUM_INVENTORY_TYPES] =
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

typedef std::map<uint32, EnchantmentInstance> EnchantmentMap;

enum scalingstatmodtypes
{
    SCALINGSTATSTAT,
    SCALINGSTATARMOR,
    SCALINGSTATDAMAGE,
    SCALINGSTATSPELLPOWER
};

/// -1 from client enchantment slot number
enum EnchantmentSlot
{
    PERM_ENCHANTMENT_SLOT           = 0,
    TEMP_ENCHANTMENT_SLOT           = 1,
    SOCK_ENCHANTMENT_SLOT1          = 2,
    SOCK_ENCHANTMENT_SLOT2          = 3,
    SOCK_ENCHANTMENT_SLOT3          = 4,
    BONUS_ENCHANTMENT_SLOT          = 5,
    PRISMATIC_ENCHANTMENT_SLOT      = 6,
#if VERSION_STRING < Cata
    MAX_INSPECTED_ENCHANTMENT_SLOT  = 7,

    PROP_ENCHANTMENT_SLOT_0         = 7,        /// used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_1         = 8,        /// used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_2         = 9,        /// used with RandomSuffix and RandomProperty
    PROP_ENCHANTMENT_SLOT_3         = 10,       /// used with RandomProperty
    PROP_ENCHANTMENT_SLOT_4         = 11,       /// used with RandomProperty
    MAX_ENCHANTMENT_SLOT            = 12
#else
    REFORGE_ENCHANTMENT_SLOT = 8,
    TRANSMOGRIFY_ENCHANTMENT_SLOT = 9,
    MAX_INSPECTED_ENCHANTMENT_SLOT = 10,

    PROP_ENCHANTMENT_SLOT_0 = 10,   // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_1 = 11,   // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_2 = 12,   // used with RandomSuffix and RandomProperty
    PROP_ENCHANTMENT_SLOT_3 = 13,   // used with RandomProperty
    PROP_ENCHANTMENT_SLOT_4 = 14,   // used with RandomProperty
    MAX_ENCHANTMENT_SLOT = 15
#endif
};

enum RandomEnchantmentTypes
{
    RANDOMPROPERTY = 1,
    RANDOMSUFFIX   = 2
};

#define RANDOM_SUFFIX_MAGIC_CALCULATION(__suffix, __scale) float2int32(float(__suffix) * float(__scale) / 10000.0f);

struct WoWItem;
class SERVER_DECL Item : public Object
{
    // MIT Start
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

#if VERSION_STRING >= WotLK
    uint32_t getEnchantmentId(uint8_t index) const;
    void setEnchantmentId(uint8_t index, uint32_t id);

    uint32_t getEnchantmentDuration(uint8_t index) const;
    void setEnchantmentDuration(uint8_t index, uint32_t duration);

    uint32_t getEnchantmentCharges(uint8_t index) const;
    void setEnchantmentCharges(uint8_t index, uint32_t charges);
#endif

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
    // Misc

    Player* getOwner() const;
    void setOwner(Player* owner);

    void setContainer(Container* container);

    ItemProperties const* getItemProperties() const;
    void setItemProperties(ItemProperties const* itemProperties);

    bool isSoulbound() const { return hasFlags(ITEM_FLAG_SOULBOUND); }
    bool isAccountbound() const { return hasFlags(ITEM_FLAG_ACCOUNTBOUND); }

    void setDurabilityToMax() { setDurability(getMaxDurability()); }

    bool fitsToSpellRequirements(SpellInfo const* spellInfo) const;

    // MIT End

        Item();
        virtual ~Item();

        void SetDirty(){ m_isDirty = true; }

        void SetRandomSuffix(uint32 id)
        {
            int32 r_id = -(int32(id));
            uint32 v = Item::GenerateRandomSuffixFactor(m_itemProperties);
            setRandomPropertiesId((uint32)r_id);
            setPropertySeed(v);
            //\todo why override random_suffix set by functions above?
            random_suffix = id;
        }


#if VERSION_STRING <= TBC
        uint32 getEnchantmentId(uint32 index) { return getUInt32Value(ITEM_FIELD_ENCHANTMENT + 3 * index); }
        void setEnchantmentId(uint32 index, uint32 value) { setUInt32Value(ITEM_FIELD_ENCHANTMENT + 3 * index, value); }

        uint32 getEnchantmentDuration(uint32 index) { return getUInt32Value(ITEM_FIELD_ENCHANTMENT + 1 + 3 * index); }
        void setEnchantmentDuration(uint32 index, uint32 value) { setUInt32Value(ITEM_FIELD_ENCHANTMENT + 1 + 3 * index, value); }

        uint32 getEnchantmentCharges(uint32 index) { return getUInt32Value(ITEM_FIELD_ENCHANTMENT + 2 + 3 * index); }
        void setEnchantmentCharges(uint32 index, uint32 value) { setUInt32Value(ITEM_FIELD_ENCHANTMENT + 2 + 3 * index, value); }

		void setTextId(uint32 textId);
#endif

        // DB Serialization
        void LoadFromDB(Field* fields, Player* plr, bool light);
        void SaveToDB(int8 containerslot, int8 slot, bool firstsave, QueryBuffer* buf);
        bool LoadAuctionItemFromDB(uint64 guid);
        void DeleteFromDB();
        void DeleteMe();
        bool IsEligibleForRefund();

        //////////////////////////////////////////////////////////////////////////////////////////
        // uint32 GetChargesLeft()
        // Finds an on-use spell on the item and returns the charges left
        //
        // \param none
        //
        // \returns the charges left if an on-use spell is found, 0 if no such spell found.
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        uint32 GetChargesLeft() const
        {
            for (uint8_t x = 0; x < 5; ++x)
                if ((m_itemProperties->Spells[x].Id != 0) && (m_itemProperties->Spells[x].Trigger == USE))
                    return getSpellCharges(x) > 0 ? getSpellCharges(x) : 0;

            return 0;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        // void SetChargesLeft(uint32 charges)
        // Finds an on-use spell on the item, and sets the remaining charges.
        // If no such spell found, nothing changes.
        //
        // \param uint32 charges  -  Number to be set as charges left.
        //
        // \returns none
        //
        //////////////////////////////////////////////////////////////////////////////////////////
        void SetChargesLeft(uint32 charges)
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

        time_t GetEnchantmentApplytime(uint32 slot)
        {
            EnchantmentMap::iterator itr = Enchantments.find(slot);
            if (itr == Enchantments.end())
                return 0;
            else
                return itr->second.ApplyTime;
        }

        // Adds an enchantment to the item.
        int32 AddEnchantment(DBC::Structures::SpellItemEnchantmentEntry const* Enchantment, uint32 Duration, bool Perm = false, bool apply = true, bool RemoveAtLogout = false, uint32 Slot_ = 0, uint32 RandomSuffix = 0);
        uint32 GetSocketsCount();

        /// Removes an enchantment from the item.
        void RemoveEnchantment(uint32 EnchantmentSlot);

        // Removes related temporary enchants
        void RemoveRelatedEnchants(DBC::Structures::SpellItemEnchantmentEntry const* newEnchant);

        /// Adds the bonus on an enchanted item.
        void ApplyEnchantmentBonus(uint32 Slot, bool Apply);

        /// Applies all enchantment bonuses (use on equip)
        void ApplyEnchantmentBonuses();

        /// Removes all enchantment bonuses (use on dequip)
        void RemoveEnchantmentBonuses();

        /// Event to remove an enchantment.
        void EventRemoveEnchantment(uint32 Slot);

        /// Check if we have an enchantment of this id?
        int32 HasEnchantment(uint32 Id);

        /// Check if we have an enchantment on that slot
        bool HasEnchantmentOnSlot(uint32 slot);

        /// Modify the time of an existing enchantment.
        void ModifyEnchantmentTime(uint32 Slot, uint32 Duration);

        /// Find free enchantment slot.
        int32 FindFreeEnchantSlot(DBC::Structures::SpellItemEnchantmentEntry const* Enchantment, uint32 random_type);

        /// Removes all enchantments.
        void RemoveAllEnchantments(bool OnlyTemporary);

        /// Sends SMSG_ITEM_UPDATE_ENCHANT_TIME
        void SendEnchantTimeUpdate(uint32 Slot, uint32 Duration);

        void SendDurationUpdate();

        /// Applies any random properties the item has.
        void ApplyRandomProperties(bool apply);

        void RemoveProfessionEnchant();
        void RemoveSocketBonusEnchant();

        /// gets the itemlink for a message to the player
        std::string GetItemLink(uint32 language);

        bool IsAmmoBag() { return (m_itemProperties->Class == ITEM_CLASS_QUIVER); }

        uint32 CountGemsWithLimitId(uint32 Limit);

        void RemoveFromWorld();

    Loot* loot;
        bool locked;
        bool m_isDirty;

        EnchantmentInstance* GetEnchantment(uint32 slot);
        bool IsGemRelated(DBC::Structures::SpellItemEnchantmentEntry const* Enchantment);

        static uint32 GenerateRandomSuffixFactor(ItemProperties const* m_itemProto);

        bool HasEnchantments() { return (Enchantments.size() > 0) ? true : false; }

        uint32 wrapped_item_id;

        time_t GetItemExpireTime() { return ItemExpiresOn; }
        void SetItemExpireTime(time_t timesec) { ItemExpiresOn = timesec; }
        void EventRemoveItem();
        void RemoveFromRefundableMap();
        bool RepairItem(Player* pPlayer, bool guildmoney = false, int32* pCost = NULL);
        uint32 RepairItemCost();

        uint32 GetOnUseSpellID(uint32 index) { return OnUseSpellIDs[index]; }
        bool HasOnUseSpellID(uint32 id)
        {
            for (uint8 i = 0; i < 3; ++i)
                if (OnUseSpellIDs[i] == id)
                    return true;

            return false;
        }

        void SetText(std::string &textString){ this->text = textString; }
        const std::string& GetText() const{ return this->text; }
#if VERSION_STRING >= Cata
    protected:

        bool m_isInTrade;

    public:

        void setIsInTrade(bool inTrade = true) { m_isInTrade = inTrade; }
        bool isInTrade() const { return m_isInTrade; }

#endif

    protected:

        ItemProperties const* m_itemProperties;
        EnchantmentMap Enchantments;
        uint32 _fields[ITEM_END];   /// this mem is wasted in case of container... but this will be fixed in future
        Player* m_owner;            /// let's not bother the manager with unneeded requests
        uint32 random_prop;
        uint32 random_suffix;
        time_t ItemExpiresOn;       /// this is for existingduration

    private:
        /// Enchant type 3 spellids, like engineering gadgets appliable to items.
        uint32 OnUseSpellIDs[3];
        std::string text;
};

//\todo move these functions to ItemProperties/Player class.
uint32 GetSkillByProto(uint32, uint32);

uint32 GetSellPriceForItem(ItemProperties const* proto, uint32 count);
uint32 GetBuyPriceForItem(ItemProperties const* proto, uint32 count, Player* plr, Creature* vendor);

std::string GetItemLinkByProto(ItemProperties const* iProto, uint32 language);

int32 GetStatScalingStatValueColumn(ItemProperties const* proto, uint32 type);

#endif // WOWSERVER_ITEM_H
