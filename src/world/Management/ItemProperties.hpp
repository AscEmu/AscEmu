/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Macros/ItemMacros.hpp"
#include "Objects/ItemDefines.hpp"

#include <string>

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
    uint32_t RequiredSkillSubRank; // required spell
    uint32_t RequiredPlayerRank1;
    uint32_t RequiredPlayerRank2;
    uint32_t RequiredFaction;
    uint32_t RequiredFactionStanding;
    uint32_t Unique;
    uint32_t MaxCount;
    uint32_t ContainerSlots; // uint8_t
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
    uint32_t QuestId; // id of the quest that this item starts (not required quest)
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
    float ArmorDamageModifier;
    uint32_t ExistingDuration;
    uint32_t ItemLimitCategory;
    uint32_t HolidayId;
    uint32_t FoodType;

    std::string lowercase_name; // used in auctions
    int32_t ForcedPetId;

    bool HasFlag(uint32_t flag) const;
    
    bool HasFlag2(uint32_t flag) const;

    bool isPotion() const;
    bool isVellum() const;
    bool isConjuredConsumable() const;
    bool isCurrencyToken() const;

    bool isRangedWeapon() const;

    uint32_t getBuyPriceForItem(uint32_t count, uint32_t factionStanding) const;
};

struct ItemSet
{
    int32_t setid;
    uint32_t itemscount;
    //Spell* spell[8];
};
