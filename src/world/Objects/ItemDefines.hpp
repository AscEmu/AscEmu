/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"
#include <cstdint>

enum ItemQualities : uint8_t
{
    ITEM_QUALITY_POOR                       = 0, // GREY
    ITEM_QUALITY_NORMAL                     = 1, // WHITE
    ITEM_QUALITY_UNCOMMON                   = 2, // GREEN
    ITEM_QUALITY_RARE                       = 3, // BLUE
    ITEM_QUALITY_EPIC                       = 4, // PURPLE
    ITEM_QUALITY_LEGENDARY                  = 5, // ORANGE
    ITEM_QUALITY_ARTIFACT                   = 6, // LIGHT YELLOW
    ITEM_QUALITY_HEIRLOOM                   = 7
};

enum ItemEnchantmentType : uint8_t
{
    ITEM_ENCHANTMENT_TYPE_NONE              = 0,
    ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL      = 1,
    ITEM_ENCHANTMENT_TYPE_DAMAGE            = 2,
    ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL       = 3,
    ITEM_ENCHANTMENT_TYPE_RESISTANCE        = 4,
    ITEM_ENCHANTMENT_TYPE_STAT              = 5,
    ITEM_ENCHANTMENT_TYPE_TOTEM             = 6,
    ITEM_ENCHANTMENT_TYPE_USE_SPELL         = 7,
    ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET  = 8
};

// -1 from client enchantment slot number
enum EnchantmentSlot : uint8_t
{
#if VERSION_STRING <= TBC
    PERM_ENCHANTMENT_SLOT                   = 0,
    TEMP_ENCHANTMENT_SLOT                   = 1,
    SOCK_ENCHANTMENT_SLOT1                  = 2,
    SOCK_ENCHANTMENT_SLOT2                  = 3,
    SOCK_ENCHANTMENT_SLOT3                  = 4,
    BONUS_ENCHANTMENT_SLOT                  = 5,
    MAX_INSPECTED_ENCHANTMENT_SLOT          = 6,

    PROP_ENCHANTMENT_SLOT_0                 = 6,  // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_1                 = 7,  // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_2                 = 8,  // used with RandomSuffix and RandomProperty
    PROP_ENCHANTMENT_SLOT_3                 = 9,  // used with RandomProperty
    PROP_ENCHANTMENT_SLOT_4                 = 10, // used with RandomProperty
    MAX_ENCHANTMENT_SLOT                    = 11
#endif

#if VERSION_STRING == WotLK
    PERM_ENCHANTMENT_SLOT                   = 0,
    TEMP_ENCHANTMENT_SLOT                   = 1,
    SOCK_ENCHANTMENT_SLOT1                  = 2,
    SOCK_ENCHANTMENT_SLOT2                  = 3,
    SOCK_ENCHANTMENT_SLOT3                  = 4,
    BONUS_ENCHANTMENT_SLOT                  = 5,
    PRISMATIC_ENCHANTMENT_SLOT              = 6,
    MAX_INSPECTED_ENCHANTMENT_SLOT          = 7,

    PROP_ENCHANTMENT_SLOT_0                 = 7,  // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_1                 = 8,  // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_2                 = 9,  // used with RandomSuffix and RandomProperty
    PROP_ENCHANTMENT_SLOT_3                 = 10, // used with RandomProperty
    PROP_ENCHANTMENT_SLOT_4                 = 11, // used with RandomProperty
    MAX_ENCHANTMENT_SLOT                    = 12
#endif

#if VERSION_STRING >= Cata
    PERM_ENCHANTMENT_SLOT                   = 0,
    TEMP_ENCHANTMENT_SLOT                   = 1,
    SOCK_ENCHANTMENT_SLOT1                  = 2,
    SOCK_ENCHANTMENT_SLOT2                  = 3,
    SOCK_ENCHANTMENT_SLOT3                  = 4,
    BONUS_ENCHANTMENT_SLOT                  = 5,
    PRISMATIC_ENCHANTMENT_SLOT              = 6,

    REFORGE_ENCHANTMENT_SLOT                = 8,
    TRANSMOGRIFY_ENCHANTMENT_SLOT           = 9,
    MAX_INSPECTED_ENCHANTMENT_SLOT          = 10,

    PROP_ENCHANTMENT_SLOT_0                 = 10, // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_1                 = 11, // used with RandomSuffix
    PROP_ENCHANTMENT_SLOT_2                 = 12, // used with RandomSuffix and RandomProperty
    PROP_ENCHANTMENT_SLOT_3                 = 13, // used with RandomProperty
    PROP_ENCHANTMENT_SLOT_4                 = 14, // used with RandomProperty
    MAX_ENCHANTMENT_SLOT                    = 15
#endif
};

enum class RandomEnchantmentType : uint8_t
{
    PROPERTY                                = 1,
    SUFFIX                                  = 2
};

enum INV_ERR
{
    INV_ERR_OK,
    INV_ERR_YOU_MUST_REACH_LEVEL_N,
    INV_ERR_SKILL_ISNT_HIGH_ENOUGH,
    INV_ERR_ITEM_DOESNT_GO_TO_SLOT,
    INV_ERR_BAG_FULL,
    INV_ERR_NONEMPTY_BAG_OVER_OTHER_BAG,
    INV_ERR_CANT_TRADE_EQUIP_BAGS,
    INV_ERR_ONLY_AMMO_CAN_GO_HERE,
    INV_ERR_NO_REQUIRED_PROFICIENCY,
    INV_ERR_NO_EQUIPMENT_SLOT_AVAILABLE,
    INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM,
    INV_ERR_YOU_CAN_NEVER_USE_THAT_ITEM2,
    INV_ERR_NO_EQUIPMENT_SLOT_AVAILABLE2,
    INV_ERR_CANT_EQUIP_WITH_TWOHANDED,
    INV_ERR_CANT_DUAL_WIELD,
    INV_ERR_ITEM_DOESNT_GO_INTO_BAG,
    INV_ERR_ITEM_DOESNT_GO_INTO_BAG2,
    INV_ERR_CANT_CARRY_MORE_OF_THIS,
    INV_ERR_NO_EQUIPMENT_SLOT_AVAILABLE3,
    INV_ERR_ITEM_CANT_STACK,
    INV_ERR_ITEM_CANT_BE_EQUIPPED,
    INV_ERR_ITEMS_CANT_BE_SWAPPED,
    INV_ERR_SLOT_IS_EMPTY,
    INV_ERR_ITEM_NOT_FOUND,
    INV_ERR_CANT_DROP_SOULBOUND,
    INV_ERR_OUT_OF_RANGE,
    INV_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT,
    INV_ERR_COULDNT_SPLIT_ITEMS,
    INV_ERR_MISSING_REAGENT,
    INV_ERR_NOT_ENOUGH_MONEY,
    INV_ERR_NOT_A_BAG,
    INV_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS,
    INV_ERR_DONT_OWN_THAT_ITEM,
    INV_ERR_CAN_EQUIP_ONLY1_QUIVER,
    INV_ERR_MUST_PURCHASE_THAT_BAG_SLOT,
    INV_ERR_TOO_FAR_AWAY_FROM_BANK,
    INV_ERR_ITEM_LOCKED,
    INV_ERR_YOU_ARE_STUNNED,
    INV_ERR_YOU_ARE_DEAD,
    INV_ERR_CANT_DO_RIGHT_NOW,
    INV_ERR_BAG_FULL2,
    INV_ERR_CAN_EQUIP_ONLY1_QUIVER2,
    INV_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH,
    INV_ERR_STACKABLE_CANT_BE_WRAPPED,
    INV_ERR_EQUIPPED_CANT_BE_WRAPPED,
    INV_ERR_WRAPPED_CANT_BE_WRAPPED,
    INV_ERR_BOUND_CANT_BE_WRAPPED,
    INV_ERR_UNIQUE_CANT_BE_WRAPPED,
    INV_ERR_BAGS_CANT_BE_WRAPPED,
    INV_ERR_ALREADY_LOOTED,
    INV_ERR_INVENTORY_FULL,
    INV_ERR_BANK_FULL,
    INV_ERR_ITEM_IS_CURRENTLY_SOLD_OUT,
    INV_ERR_BAG_FULL3,
    INV_ERR_ITEM_NOT_FOUND2,
    INV_ERR_ITEM_CANT_STACK2,
    INV_ERR_BAG_FULL4,
    INV_ERR_ITEM_SOLD_OUT,
    INV_ERR_OBJECT_IS_BUSY,
    INV_ERR_NONE,
    INV_ERR_CANT_DO_IN_COMBAT,
    INV_ERR_CANT_DO_WHILE_DISARMED,
    INV_ERR_BAG_FULL6,
    INV_ERR_ITEM_RANK_NOT_ENOUGH,
    INV_ERR_ITEM_REPUTATION_NOT_ENOUGH,
    INV_ERR_MORE_THAN1_SPECIAL_BAG,
    INV_ERR_LOOT_CANT_LOOT_THAT_NOW,
    INV_ERR_ITEM_UNIQUE_EQUIPABLE,
    INV_ERR_VENDOR_MISSING_TURNINS,
    INV_ERR_NOT_ENOUGH_HONOR_POINTS,
    INV_ERR_NOT_ENOUGH_ARENA_POINTS,
    INV_ERR_ITEM_MAX_COUNT_SOCKETED,
    INV_ERR_MAIL_BOUND_ITEM,
    INV_ERR_NO_SPLIT_WHILE_PROSPECTING,
    INV_ERR_NOT_USED_SO_DO_NOT_USE,
    INV_ERR_ITEM_MAX_COUNT_EQUIPPED_SOCKETED,
    INV_ERR_ITEM_UNIQUE_EQUIPPABLE_SOCKETED,
    INV_ERR_TOO_MUCH_GOLD,
    INV_ERR_NOT_DURING_ARENA_MATCH,
    INV_ERR_CANNOT_TRADE_THAT,
    INV_ERR_PERSONAL_ARENA_RATING_TOO_LOW,
    INV_ERR_EVENT_AUTOEQUIP_BIND_CONFIRM,
    INV_ERR_ARTEFACTS_ONLY_FOR_OWN_CHARACTERS,
    INV_ERR_OK2,
    INV_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED,
    INV_ERR_ITEM_MAX_LIMIT_CATEGORY_SOCKETED_EXCEEDED,
    INV_ERR_SCALING_STAT_ITEM_LEVEL_EXCEEDED,
    INV_ERR_PURCHASE_LEVEL_TOO_LOW,
    INV_ERR_CANT_EQUIP_NEED_TALENT,
    INV_ERR_ITEM_MAX_LIMIT_CATEGORY_EQUIPPED_EXCEEDED
};

enum CanAffordItem
{
    CAN_AFFORD_ITEM_ERROR_NOT_FOUND                 = 0,
    CAN_AFFORD_ITEM_ERROR_SOLD_OUT                  = 1,
    CAN_AFFORD_ITEM_ERROR_NOT_ENOUGH_MONEY          = 2,
    CAN_AFFORD_ITEM_ERROR_DOESNT_LIKE_YOU           = 4,
    CAN_AFFORD_ITEM_ERROR_TOO_FAR_AWAY              = 5,
    CAN_AFFORD_ITEM_ERROR_CANT_CARRY_ANY_MORE       = 8,
    CAN_AFFORD_ITEM_ERROR_NOT_REQUIRED_RANK         = 11,
    CAN_AFFORD_ITEM_ERROR_REPUTATION                = 12
};

enum ItemModType
{
    ITEM_MOD_MANA                                   = 0,
    ITEM_MOD_HEALTH                                 = 1,
    ITEM_MOD_UNKNOWN                                = 2,
    ITEM_MOD_AGILITY                                = 3,
    ITEM_MOD_STRENGTH                               = 4,
    ITEM_MOD_INTELLECT                              = 5,
    ITEM_MOD_SPIRIT                                 = 6,
    ITEM_MOD_STAMINA                                = 7,
    ITEM_MOD_WEAPON_SKILL_RATING                    = 11,
    ITEM_MOD_DEFENSE_RATING                         = 12,
    ITEM_MOD_DODGE_RATING                           = 13,
    ITEM_MOD_PARRY_RATING                           = 14,
    ITEM_MOD_SHIELD_BLOCK_RATING                    = 15,
    ITEM_MOD_MELEE_HIT_RATING                       = 16,
    ITEM_MOD_RANGED_HIT_RATING                      = 17,
    ITEM_MOD_SPELL_HIT_RATING                       = 18,
    ITEM_MOD_MELEE_CRITICAL_STRIKE_RATING           = 19,
    ITEM_MOD_RANGED_CRITICAL_STRIKE_RATING          = 20,
    ITEM_MOD_SPELL_CRITICAL_STRIKE_RATING           = 21,
    ITEM_MOD_MELEE_HIT_AVOIDANCE_RATING             = 22,
    ITEM_MOD_RANGED_HIT_AVOIDANCE_RATING            = 23,
    ITEM_MOD_SPELL_HIT_AVOIDANCE_RATING             = 24,
    ITEM_MOD_MELEE_CRITICAL_AVOIDANCE_RATING        = 25,
    ITEM_MOD_RANGED_CRITICAL_AVOIDANCE_RATING       = 26,
    ITEM_MOD_SPELL_CRITICAL_AVOIDANCE_RATING        = 27,
    ITEM_MOD_MELEE_HASTE_RATING                     = 28,
    ITEM_MOD_RANGED_HASTE_RATING                    = 29,
    ITEM_MOD_SPELL_HASTE_RATING                     = 30,
    ITEM_MOD_HIT_RATING                             = 31,
    ITEM_MOD_CRITICAL_STRIKE_RATING                 = 32,
    ITEM_MOD_HIT_AVOIDANCE_RATING                   = 33,
    ITEM_MOD_CRITICAL_AVOIDANCE_RATING              = 34,
    ITEM_MOD_RESILIENCE_RATING                      = 35,
    ITEM_MOD_HASTE_RATING                           = 36,
    ITEM_MOD_EXPERTISE_RATING                       = 37,
    ITEM_MOD_ATTACK_POWER                           = 38,
    ITEM_MOD_RANGED_ATTACK_POWER                    = 39,
    ITEM_MOD_FERAL_ATTACK_POWER                     = 40,
    ITEM_MOD_SPELL_HEALING_DONE                     = 41,
    ITEM_MOD_SPELL_DAMAGE_DONE                      = 42,
    ITEM_MOD_MANA_REGENERATION                      = 43,
    ITEM_MOD_ARMOR_PENETRATION_RATING               = 44,
    ITEM_MOD_SPELL_POWER                            = 45,
    ITEM_MOD_HEALTH_REGEN                           = 46,
    ITEM_MOD_SPELL_PENETRATION                      = 47,
    ITEM_MOD_BLOCK_VALUE                            = 48,
    ITEM_MOD_MASTERY_RATING                         = 49,
    ITEM_MOD_EXTRA_ARMOR                            = 50,
    ITEM_MOD_FIRE_RESISTANCE                        = 51,
    ITEM_MOD_FROST_RESISTANCE                       = 52,
    ITEM_MOD_HOLY_RESISTANCE                        = 53,
    ITEM_MOD_SHADOW_RESISTANCE                      = 54,
    ITEM_MOD_NATURE_RESISTANCE                      = 55,
    ITEM_MOD_ARCANE_RESISTANCE                      = 56,
    MAX_ITEM_MOD                                    = 57
};

enum ITEM_SPELLTRIGGER_TYPE
{
    USE                     = 0,
    ON_EQUIP                = 1,
    CHANCE_ON_HIT           = 2,
    SOULSTONE               = 4,
    APPLY_AURA_ON_PICKUP    = 5, // Applies aura to player on item pickup and removes the aura on item loss
    LEARNING                = 6
};

enum ITEM_BONDING_TYPE
{
    ITEM_BIND_NONE          = 0,
    ITEM_BIND_ON_PICKUP     = 1,
    ITEM_BIND_ON_EQUIP      = 2,
    ITEM_BIND_ON_USE        = 3,
    ITEM_BIND_QUEST         = 4,
    ITEM_BIND_QUEST2        = 5
};

enum INVENTORY_TYPES
{
    INVTYPE_NON_EQUIP       = 0x00,
    INVTYPE_HEAD            = 0x01,
    INVTYPE_NECK            = 0x02,
    INVTYPE_SHOULDERS       = 0x03,
    INVTYPE_BODY            = 0x04,
    INVTYPE_CHEST           = 0x05,
    INVTYPE_WAIST           = 0x06,
    INVTYPE_LEGS            = 0x07,
    INVTYPE_FEET            = 0x08,
    INVTYPE_WRISTS          = 0x09,
    INVTYPE_HANDS           = 0x0a,
    INVTYPE_FINGER          = 0x0b,
    INVTYPE_TRINKET         = 0x0c,
    INVTYPE_WEAPON          = 0x0d,
    INVTYPE_SHIELD          = 0x0e,
    INVTYPE_RANGED          = 0x0f,
    INVTYPE_CLOAK           = 0x10,
    INVTYPE_2HWEAPON        = 0x11,
    INVTYPE_BAG             = 0x12,
    INVTYPE_TABARD          = 0x13,
    INVTYPE_ROBE            = 0x14,
    INVTYPE_WEAPONMAINHAND  = 0x15,
    INVTYPE_WEAPONOFFHAND   = 0x16,
    INVTYPE_HOLDABLE        = 0x17,
    INVTYPE_AMMO            = 0x18,
    INVTYPE_THROWN          = 0x19,
    INVTYPE_RANGEDRIGHT     = 0x1a,
    INVTYPE_QUIVER          = 0x1b,
    INVTYPE_RELIC           = 0x1c,
    NUM_INVENTORY_TYPES     = 0x1d
};

enum ITEM_CLASS
{
    ITEM_CLASS_CONSUMABLE           = 0,
    ITEM_CLASS_CONTAINER            = 1,
    ITEM_CLASS_WEAPON               = 2,
    ITEM_CLASS_JEWELRY              = 3,
    ITEM_CLASS_ARMOR                = 4,
    ITEM_CLASS_REAGENT              = 5,
    ITEM_CLASS_PROJECTILE           = 6,
    ITEM_CLASS_TRADEGOODS           = 7,
    ITEM_CLASS_GENERIC              = 8,
    ITEM_CLASS_RECIPE               = 9,
    ITEM_CLASS_MONEY                = 10,
    ITEM_CLASS_QUIVER               = 11,
    ITEM_CLASS_QUEST                = 12,
    ITEM_CLASS_KEY                  = 13,
    ITEM_CLASS_PERMANENT            = 14,
    ITEM_CLASS_MISCELLANEOUS        = 15,
    ITEM_CLASS_GLYPH                = 16
};

enum Item_Subclass
{
    // Weapon
    ITEM_SUBCLASS_WEAPON_AXE                = 0,
    ITEM_SUBCLASS_WEAPON_TWOHAND_AXE        = 1,
    ITEM_SUBCLASS_WEAPON_BOW                = 2,
    ITEM_SUBCLASS_WEAPON_GUN                = 3,
    ITEM_SUBCLASS_WEAPON_MACE               = 4,
    ITEM_SUBCLASS_WEAPON_TWOHAND_MACE       = 5,
    ITEM_SUBCLASS_WEAPON_POLEARM            = 6,
    ITEM_SUBCLASS_WEAPON_SWORD              = 7,
    ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD      = 8,
    ITEM_SUBCLASS_WEAPON_STAFF              = 10,
    ITEM_SUBCLASS_WEAPON_FIST_WEAPON        = 13,
    ITEM_SUBCLASS_WEAPON_MISC_WEAPON        = 14,
    ITEM_SUBCLASS_WEAPON_DAGGER             = 15,
    ITEM_SUBCLASS_WEAPON_THROWN             = 16,
    ITEM_SUBCLASS_WEAPON_CROSSBOW           = 18,
    ITEM_SUBCLASS_WEAPON_WAND               = 19,
    ITEM_SUBCLASS_WEAPON_FISHING_POLE       = 20,

    // Armor
    ITEM_SUBCLASS_ARMOR_MISC                = 0,
    ITEM_SUBCLASS_ARMOR_CLOTH               = 1,
    ITEM_SUBCLASS_ARMOR_LEATHER             = 2,
    ITEM_SUBCLASS_ARMOR_MAIL                = 3,
    ITEM_SUBCLASS_ARMOR_PLATE_MAIL          = 4,
    ITEM_SUBCLASS_ARMOR_SHIELD              = 6,

    // Projectile
    ITEM_SUBCLASS_PROJECTILE_ARROW          = 2,
    ITEM_SUBCLASS_PROJECTILE_BULLET         = 3,

    // Trade goods
    ITEM_SUBCLASS_PROJECTILE_TRADE_GOODS    = 0,
    ITEM_SUBCLASS_PROJECTILE_PARTS          = 1,
    ITEM_SUBCLASS_PROJECTILE_EXPLOSIVES     = 2,
    ITEM_SUBCLASS_PROJECTILE_DEVICES        = 3,
    ITEM_SUBCLASS_ARMOR_ENCHANTMENT         = 14,
    ITEM_SUBCLASS_WEAPON_ENCHANTMENT        = 15,

    // Consumables
    ITEM_SUBCLASS_CONSUMABLE                = 0,
    ITEM_SUBCLASS_POTION                    = 1,
    ITEM_SUBCLASS_ELIXIR                    = 2,
    ITEM_SUBCLASS_FLASK                     = 3,
    ITEM_SUBCLASS_SCROLL                    = 4,
    ITEM_SUBCLASS_FOOD_DRINK                = 5,
    ITEM_SUBCLASS_ITEM_ENHANCEMENT          = 6,
    ITEM_SUBCLASS_BANDAGE                   = 7,

    // Recipe
    ITEM_SUBCLASS_RECIPE_BOOK               = 0,
    ITEM_SUBCLASS_RECIPE_LEATHERWORKING     = 1,
    ITEM_SUBCLASS_RECIPE_TAILORING          = 2,
    ITEM_SUBCLASS_RECIPE_ENGINEERING        = 3,
    ITEM_SUBCLASS_RECIPE_BLACKSMITHING      = 4,
    ITEM_SUBCLASS_RECIPE_COOKING            = 5,
    ITEM_SUBCLASS_RECIPE_ALCHEMY            = 6,
    ITEM_SUBCLASS_RECIPE_FIRST_AID          = 7,
    ITEM_SUBCLASS_RECIPE_ENCHANTING         = 8,
    ITEM_SUBCLASS_RECIPE_FISNING            = 9,

    // Quiver
    ITEM_SUBCLASS_QUIVER_AMMO_POUCH         = 3,
    ITEM_SUBCLASS_QUIVER_QUIVER             = 2,

    // Misc
    ITEM_SUBCLASS_MISC_JUNK                 = 0,
};

enum ITEM_QUALITY
{
    ITEM_QUALITY_POOR_GREY              = 0,
    ITEM_QUALITY_NORMAL_WHITE           = 1,
    ITEM_QUALITY_UNCOMMON_GREEN         = 2,
    ITEM_QUALITY_RARE_BLUE              = 3,
    ITEM_QUALITY_EPIC_PURPLE            = 4,
    ITEM_QUALITY_LEGENDARY_ORANGE       = 5,
    ITEM_QUALITY_ARTIFACT_LIGHT_YELLOW  = 6,
    ITEM_QUALITY_HEIRLOOM_LIGHT_YELLOW  = 7
};

enum ITEM_FLAG
{
    ITEM_FLAGS_NONE                 = 0x00000000,
    ITEM_FLAG_SOULBOUND             = 0x00000001, // not used in proto
    ITEM_FLAG_CONJURED              = 0x00000002,
    ITEM_FLAG_LOOTABLE              = 0x00000004,
    ITEM_FLAG_WRAPPED               = 0x00000008, // not used in proto
    ITEM_FLAG_BROKEN                = 0x00000010, // many equipable items and bags
    ITEM_FLAG_INDESTRUCTIBLE        = 0x00000020, // can't destruct this item
    ITEM_FLAG_UNKNOWN_07            = 0x00000040, // many consumables
    ITEM_FLAG_UNKNOWN_08            = 0x00000080, // only 1 wand uses this
    ITEM_FLAG_BOP_TRADEABLE         = 0x00000100, // Allows trading soulbound items
    ITEM_FLAG_WRAP_GIFT             = 0x00000200,
    ITEM_FLAG_CREATE_ITEM           = 0x00000400, // probably worng
    ITEM_FLAG_FREE_FOR_ALL          = 0x00000800, // can be looted ffa
    ITEM_FLAG_REFUNDABLE            = 0x00001000,
    ITEM_FLAG_SIGNABLE              = 0x00002000, // charts
    ITEM_FLAG_READABLE              = 0x00004000, // may be worng
    ITEM_FLAG_UNKNOWN_16            = 0x00008000,
    ITEM_FLAG_EVENT_REQ             = 0x00010000, // may be wrong
    ITEM_FLAG_UNKNOWN_18            = 0x00020000,
    ITEM_FLAG_PROSPECTABLE          = 0x00040000,
    ITEM_FLAG_UNIQUE_EQUIP          = 0x00080000,
    ITEM_FLAG_UNKNOWN_21            = 0x00100000, // not used in proto
    ITEM_FLAG_USEABLE_IN_ARENA      = 0x00200000, // useable in arenas
    ITEM_FLAG_THROWN                = 0x00400000,
    ITEM_FLAG_SHAPESHIFT_OK         = 0x00800000,
    ITEM_FLAG_UNKNOWN_25            = 0x01000000,
    ITEM_FLAG_SMART_LOOT            = 0x02000000, // Profession recipes: can only be looted if you meet requirements and don't already know it
    ITEM_FLAG_NOT_USEABLE_IN_ARENA  = 0x04000000,
    ITEM_FLAG_ACCOUNTBOUND          = 0x08000000,
    ITEM_FLAG_ENCHANT_SCROLL        = 0x10000000, // enchant scrolls
    ITEM_FLAG_MILLABLE              = 0x20000000,
    ITEM_FLAG_UNKNOWN_31            = 0x40000000,
    ITEM_FLAG_UNKNOWN_32            = 0x80000000
};

enum ITEM_FLAGS2
{
    ITEM_FLAG2_HORDE_ONLY                   = 0x00000001,
    ITEM_FLAG2_ALLIANCE_ONLY                = 0x00000002,
    ITEM_FLAG2_EXT_COST_REQUIRES_GOLD       = 0x00000004,
    ITEM_FLAG2_NEED_ROLL_DISABLED           = 0x00000100,
    ITEM_FLAGS_EXTRA_CASTER_WEAPON          = 0x00000200,
    ITEM_FLAGS_EXTRA_HAS_NORMAL_PRICE       = 0x00004000,
    ITEM_FLAGS_EXTRA_BNET_ACCOUNT_BOUND     = 0x00020000,
    ITEM_FLAGS_EXTRA_CANNOT_BE_TRANSMOG     = 0x00200000,
    ITEM_FLAGS_EXTRA_CANNOT_TRANSMOG        = 0x00400000,
    ITEM_FLAGS_EXTRA_CAN_TRANSMOG           = 0x00800000,
};

// dictates what bag-types an item can go into
enum SPECIAL_ITEM_TYPE
{
    ITEM_TYPE_BOWAMMO        = 0x0001,      // Arrows (quivers)
    ITEM_TYPE_GUNAMMO        = 0x0002,      // Bullets (ammo pouches)
    ITEM_TYPE_SOULSHARD      = 0x0004,      // Soul Shards (soul bags)
    ITEM_TYPE_LEATHERWORK    = 0x0008,      // Leatherworking Supplies (lw supply bags)
    ITEM_TYPE_INSCRIPTION    = 0x0010,      // Inscription supplies (inscriber supply bags)
    ITEM_TYPE_HERBALISM      = 0x0020,      // Herbalism supplies (herb bags)
    ITEM_TYPE_ENCHANTMENT    = 0x0040,      // Enchanting Supplies (enchanting bags)
    ITEM_TYPE_ENGINEERING    = 0x0080,      // Engineering Supplies (engineering toolboxes)
    ITEM_TYPE_KEYRING        = 0x0100,      // Keys (the keyring)
    ITEM_TYPE_GEMS           = 0x0200,      // Jewelcrafting supplies (JC toolboxes)
    ITEM_TYPE_MINING         = 0x0400,      // Mining Supplies (mining toolboxes)
    ITEM_TYPE_SBEQUIPMENT    = 0x0800,      // Soulbound Equipment (wtf is this anyway?)
    ITEM_TYPE_VANITYPETS     = 0x1000,      // Vanity Pets (no idea what this is here for, there's no 'vanity-pet bag', although perhaps they started on one then changed their minds and made the pets/mounts window)
    ITEM_TYPE_CURRENCY       = 0x2000,      // Currency. duh. (currency tab in char window)
    ITEM_TYPE_QUEST_ITEMS    = 0x4000       // Quest items.
};

enum SOCKET_GEM_COLOR
{
    GEM_META_SOCKET         = 1,
    GEM_RED_SOCKET          = 2,
    GEM_YELLOW_SOCKET       = 4,
    GEM_BLUE_SOCKET         = 8
};

enum ITEM_LIMIT_FLAGS
{
    ILFLAG_NONE             = 0,
    ILFLAG_EQUIP_ONLY       = 1
};

struct SocketInfo
{
    uint32_t SocketColor;
    uint32_t Unk;
};

struct ItemSpell
{
    uint32_t Id;
    uint32_t Trigger;
    int32_t Charges;
    int32_t Cooldown;
    uint32_t Category;
    int32_t CategoryCooldown;
};

struct ItemDamage
{
    float Min;
    float Max;
    uint32_t Type;
};

struct ItemStat
{
    uint32_t Type;
    int32_t Value;
};
