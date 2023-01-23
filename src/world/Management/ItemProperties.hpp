/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>
#include "Common.hpp"
#include "Macros/ItemMacros.hpp"
#include "Objects/ItemDefines.hpp"

class Spell;

struct ItemProperties
{
    uint32_t ItemId;
    uint32_t Class;
    uint16_t SubClass;
    uint32_t unknown_bc;
    std::string Name;
    uint32_t DisplayInfoID;
    uint32_t Quality;
    uint32_t Flags;
    uint32_t Flags2;
    uint32_t BuyPrice;
    uint32_t SellPrice;
    uint32_t InventoryType;
    uint32_t AllowableClass;
    uint32_t AllowableRace;
    uint32_t ItemLevel;
    uint32_t RequiredLevel;
    uint16_t RequiredSkill;
    uint32_t RequiredSkillRank;
    uint32_t RequiredSkillSubRank;    /// required spell
    uint32_t RequiredPlayerRank1;
    uint32_t RequiredPlayerRank2;
    uint32_t RequiredFaction;
    uint32_t RequiredFactionStanding;
    uint32_t Unique;
    uint32_t MaxCount;
    uint32_t ContainerSlots;
    uint32_t itemstatscount;
    ItemStat Stats[MAX_ITEM_PROTO_STATS];
    uint32_t ScalingStatsEntry;
    uint32_t ScalingStatsFlag;
    ItemDamage Damage[MAX_ITEM_PROTO_DAMAGES];
    uint32_t Armor;
    uint32_t HolyRes;
    uint32_t FireRes;
    uint32_t NatureRes;
    uint32_t FrostRes;
    uint32_t ShadowRes;
    uint32_t ArcaneRes;
    uint32_t Delay;
    uint32_t AmmoType;
    float Range;
    ItemSpell Spells[MAX_ITEM_PROTO_SPELLS];
    uint32_t Bonding;
    std::string Description;
    uint32_t PageId;
    uint32_t PageLanguage;
    uint32_t PageMaterial;
    uint32_t QuestId;
    uint32_t LockId;
    uint32_t LockMaterial;
    uint32_t SheathID;
    uint32_t RandomPropId;
    uint32_t RandomSuffixId;
    uint32_t Block;
    int32_t ItemSet;
    uint32_t MaxDurability;
    uint32_t ZoneNameID;
    uint32_t MapID;
    uint32_t BagFamily;
    uint32_t TotemCategory;
    SocketInfo Sockets[MAX_ITEM_PROTO_SOCKETS];
    uint32_t SocketBonus;
    uint32_t GemProperties;
    int32_t DisenchantReqSkill;
    uint32_t ArmorDamageModifier;
    uint32_t ExistingDuration;
    uint32_t ItemLimitCategory;
    uint32_t HolidayId;
    uint32_t FoodType;

    std::string lowercase_name;      // used in auctions
    int32_t ForcedPetId;

    bool HasFlag(uint32_t flag) const
    {
        if ((Flags & flag) != 0)
            return true;

        return false;
    }
    
    bool HasFlag2(uint32_t flag) const
    {
        if ((Flags2 & flag) != 0)
            return true;
        
        return false;
    }

    bool isPotion() const { return Class == ITEM_CLASS_CONSUMABLE && SubClass == ITEM_SUBCLASS_POTION; }
    bool isVellum() const { return Class == ITEM_CLASS_TRADEGOODS && SubClass == ITEM_SUBCLASS_ARMOR_ENCHANTMENT; }
    bool isConjuredConsumable() const { return Class == ITEM_CLASS_CONSUMABLE && (Flags & ITEM_FLAG_CONJURED); }
    bool isCurrencyToken() const { return BagFamily & ITEM_TYPE_CURRENCY; }

    bool isRangedWeapon() const
    {
        return Class == ITEM_CLASS_WEAPON ||
            SubClass == ITEM_SUBCLASS_WEAPON_BOW ||
            SubClass == ITEM_SUBCLASS_WEAPON_GUN ||
            SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW;
    }

    uint32_t getBuyPriceForItem(uint32_t count, uint32_t factionStanding) const
    {
        const float pricemod[9] =
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

        int32_t cost = BuyPrice;

        if (factionStanding && factionStanding <= 8)
            cost = float2int32(ceilf(BuyPrice * pricemod[factionStanding]));

        return cost * count;
    }

};

typedef struct
{
    int32_t setid;
    uint32_t itemscount;
    //Spell* spell[8];
} ItemSet;
