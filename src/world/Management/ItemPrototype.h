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

#ifndef ITEMPROTOTYPE_H
#define ITEMPROTOTYPE_H

#include <cstdint>
#include <string>

class Spell;

#define MAX_INVENTORY_SLOT 150
#define MAX_BUYBACK_SLOT 12
//#define MAX_BUYBACK_SLOT ((PLAYER_FIELD_KEYRING_SLOT_1 - PLAYER_FIELD_VENDORBUYBACK_SLOT_1) >> 1)

#define ITEM_NO_SLOT_AVAILABLE -1   /// works for all kind of slots now
#define INVENTORY_SLOT_NOT_SET -1

#define EQUIPMENT_SLOT_START        0
#define EQUIPMENT_SLOT_HEAD         0
#define EQUIPMENT_SLOT_NECK         1
#define EQUIPMENT_SLOT_SHOULDERS    2
#define EQUIPMENT_SLOT_BODY         3
#define EQUIPMENT_SLOT_CHEST        4
#define EQUIPMENT_SLOT_WAIST        5
#define EQUIPMENT_SLOT_LEGS         6
#define EQUIPMENT_SLOT_FEET         7
#define EQUIPMENT_SLOT_WRISTS       8
#define EQUIPMENT_SLOT_HANDS        9
#define EQUIPMENT_SLOT_FINGER1      10
#define EQUIPMENT_SLOT_FINGER2      11
#define EQUIPMENT_SLOT_TRINKET1     12
#define EQUIPMENT_SLOT_TRINKET2     13
#define EQUIPMENT_SLOT_BACK         14
#define EQUIPMENT_SLOT_MAINHAND     15
#define EQUIPMENT_SLOT_OFFHAND      16
#define EQUIPMENT_SLOT_RANGED       17
#define EQUIPMENT_SLOT_TABARD       18
#define EQUIPMENT_SLOT_END          19

#define INVENTORY_SLOT_BAG_START    19
#define INVENTORY_SLOT_BAG_1        19
#define INVENTORY_SLOT_BAG_2        20
#define INVENTORY_SLOT_BAG_3        21
#define INVENTORY_SLOT_BAG_4        22
#define INVENTORY_SLOT_BAG_END      23

#define INVENTORY_SLOT_ITEM_START   23
#define INVENTORY_SLOT_ITEM_1       23
#define INVENTORY_SLOT_ITEM_2       24
#define INVENTORY_SLOT_ITEM_3       25
#define INVENTORY_SLOT_ITEM_4       26
#define INVENTORY_SLOT_ITEM_5       27
#define INVENTORY_SLOT_ITEM_6       28
#define INVENTORY_SLOT_ITEM_7       29
#define INVENTORY_SLOT_ITEM_8       30
#define INVENTORY_SLOT_ITEM_9       31
#define INVENTORY_SLOT_ITEM_10      32
#define INVENTORY_SLOT_ITEM_11      33
#define INVENTORY_SLOT_ITEM_12      34
#define INVENTORY_SLOT_ITEM_13      35
#define INVENTORY_SLOT_ITEM_14      36
#define INVENTORY_SLOT_ITEM_15      37
#define INVENTORY_SLOT_ITEM_16      38
#define INVENTORY_SLOT_ITEM_END     39

#define BANK_SLOT_ITEM_START        39
#define BANK_SLOT_ITEM_1            39
#define BANK_SLOT_ITEM_2            40
#define BANK_SLOT_ITEM_3            41
#define BANK_SLOT_ITEM_4            42
#define BANK_SLOT_ITEM_5            43
#define BANK_SLOT_ITEM_6            44
#define BANK_SLOT_ITEM_7            45
#define BANK_SLOT_ITEM_8            46
#define BANK_SLOT_ITEM_9            47
#define BANK_SLOT_ITEM_10           48
#define BANK_SLOT_ITEM_11           49
#define BANK_SLOT_ITEM_12           50
#define BANK_SLOT_ITEM_13           51
#define BANK_SLOT_ITEM_14           52
#define BANK_SLOT_ITEM_15           53
#define BANK_SLOT_ITEM_16           54
#define BANK_SLOT_ITEM_17           55
#define BANK_SLOT_ITEM_18           56
#define BANK_SLOT_ITEM_19           57
#define BANK_SLOT_ITEM_20           58
#define BANK_SLOT_ITEM_21           59
#define BANK_SLOT_ITEM_22           60
#define BANK_SLOT_ITEM_23           61
#define BANK_SLOT_ITEM_24           62
#define BANK_SLOT_ITEM_25           63
#define BANK_SLOT_ITEM_26           64
#define BANK_SLOT_ITEM_27           65
#define BANK_SLOT_ITEM_28           66
#define BANK_SLOT_ITEM_END          67

#define BANK_SLOT_BAG_START         67
#define BANK_SLOT_BAG_1             67
#define BANK_SLOT_BAG_2             68
#define BANK_SLOT_BAG_3             69
#define BANK_SLOT_BAG_4             70
#define BANK_SLOT_BAG_5             71
#define BANK_SLOT_BAG_6             72
#define BANK_SLOT_BAG_7             73
#define BANK_SLOT_BAG_END           74

#define INVENTORY_KEYRING_START     86
#define INVENTORY_KEYRING_1         86
#define INVENTORY_KEYRING_2         87
#define INVENTORY_KEYRING_3         88
#define INVENTORY_KEYRING_4         89
#define INVENTORY_KEYRING_5         90
#define INVENTORY_KEYRING_6         91
#define INVENTORY_KEYRING_7         92
#define INVENTORY_KEYRING_8         93
#define INVENTORY_KEYRING_9         94
#define INVENTORY_KEYRING_10        95
#define INVENTORY_KEYRING_11        96
#define INVENTORY_KEYRING_12        97
#define INVENTORY_KEYRING_13        98
#define INVENTORY_KEYRING_14        99
#define INVENTORY_KEYRING_15        100
#define INVENTORY_KEYRING_16        101
#define INVENTORY_KEYRING_17        102
#define INVENTORY_KEYRING_18        103
#define INVENTORY_KEYRING_19        104
#define INVENTORY_KEYRING_20        105
#define INVENTORY_KEYRING_21        106
#define INVENTORY_KEYRING_22        107
#define INVENTORY_KEYRING_23        108
#define INVENTORY_KEYRING_24        109
#define INVENTORY_KEYRING_25        110
#define INVENTORY_KEYRING_26        111
#define INVENTORY_KEYRING_27        112
#define INVENTORY_KEYRING_28        113
#define INVENTORY_KEYRING_29        114
#define INVENTORY_KEYRING_30        115
#define INVENTORY_KEYRING_31        116
#define INVENTORY_KEYRING_32        117
#define INVENTORY_KEYRING_END       118

#define CURRENCYTOKEN_SLOT_START    118
#define CURRENCYTOKEN_SLOT_1        118
#define CURRENCYTOKEN_SLOT_2        119
#define CURRENCYTOKEN_SLOT_3        120
#define CURRENCYTOKEN_SLOT_4        121
#define CURRENCYTOKEN_SLOT_5        122
#define CURRENCYTOKEN_SLOT_6        123
#define CURRENCYTOKEN_SLOT_7        124
#define CURRENCYTOKEN_SLOT_8        125
#define CURRENCYTOKEN_SLOT_9        126
#define CURRENCYTOKEN_SLOT_10       127
#define CURRENCYTOKEN_SLOT_11       128
#define CURRENCYTOKEN_SLOT_12       129
#define CURRENCYTOKEN_SLOT_13       130
#define CURRENCYTOKEN_SLOT_14       131
#define CURRENCYTOKEN_SLOT_15       132
#define CURRENCYTOKEN_SLOT_16       133
#define CURRENCYTOKEN_SLOT_17       134
#define CURRENCYTOKEN_SLOT_18       135
#define CURRENCYTOKEN_SLOT_19       136
#define CURRENCYTOKEN_SLOT_20       137
#define CURRENCYTOKEN_SLOT_21       138
#define CURRENCYTOKEN_SLOT_22       139
#define CURRENCYTOKEN_SLOT_23       140
#define CURRENCYTOKEN_SLOT_24       141
#define CURRENCYTOKEN_SLOT_25       142
#define CURRENCYTOKEN_SLOT_26       143
#define CURRENCYTOKEN_SLOT_27       144
#define CURRENCYTOKEN_SLOT_28       145
#define CURRENCYTOKEN_SLOT_29       146
#define CURRENCYTOKEN_SLOT_30       147
#define CURRENCYTOKEN_SLOT_31       148
#define CURRENCYTOKEN_SLOT_32       149
#define CURRENCYTOKEN_SLOT_END      150

// for SMSG_INVENTORY_CHANGE_FAILURE
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

enum ITEM_STAT_TYPE
{
    POWER                                   = 0,
    HEALTH                                  = 1,
    UNKNOWN                                 = 2,
    AGILITY                                 = 3,
    STRENGTH                                = 4,
    INTELLECT                               = 5,
    SPIRIT                                  = 6,
    STAMINA                                 = 7,
    WEAPON_SKILL_RATING                     = 11,
    DEFENSE_RATING                          = 12,
    DODGE_RATING                            = 13,
    PARRY_RATING                            = 14,
    SHIELD_BLOCK_RATING                     = 15,
    MELEE_HIT_RATING                        = 16,
    RANGED_HIT_RATING                       = 17,
    SPELL_HIT_RATING                        = 18,
    MELEE_CRITICAL_STRIKE_RATING            = 19,
    RANGED_CRITICAL_STRIKE_RATING           = 20,
    SPELL_CRITICAL_STRIKE_RATING            = 21,
    MELEE_HIT_AVOIDANCE_RATING              = 22,
    RANGED_HIT_AVOIDANCE_RATING             = 23,
    SPELL_HIT_AVOIDANCE_RATING              = 24,
    MELEE_CRITICAL_AVOIDANCE_RATING         = 25,
    RANGED_CRITICAL_AVOIDANCE_RATING        = 26,
    SPELL_CRITICAL_AVOIDANCE_RATING         = 27,
    MELEE_HASTE_RATING                      = 28,
    RANGED_HASTE_RATING                     = 29,
    SPELL_HASTE_RATING                      = 30,
    HIT_RATING                              = 31,
    CRITICAL_STRIKE_RATING                  = 32,
    HIT_AVOIDANCE_RATING                    = 33,
    CRITICAL_AVOIDANCE_RATING               = 34,
    RESILIENCE_RATING                       = 35,
    HASTE_RATING                            = 36,
    EXPERTISE_RATING                        = 37,
    ATTACK_POWER                            = 38,
    RANGED_ATTACK_POWER                     = 39,
    FERAL_ATTACK_POWER                      = 40,
    SPELL_HEALING_DONE                      = 41,
    SPELL_DAMAGE_DONE                       = 42,
    MANA_REGENERATION                       = 43,
    ARMOR_PENETRATION_RATING                = 44,
    SPELL_POWER                             = 45,
    HEALTH_REGEN                            = 46,
    SPELL_PENETRATION                       = 47,
    BLOCK_VALUE                             = 48
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
    ITEM_FLAG_UNKNOWN_09            = 0x00000100, // some wands & relics
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
    ITEM_FLAG_UNKNOWN_26            = 0x02000000,
    ITEM_FLAG_NOT_USEABLE_IN_ARENA  = 0x04000000,
    ITEM_FLAG_ACCOUNTBOUND          = 0x08000000,
    ITEM_FLAG_ENCHANT_SCROLL        = 0x10000000, // enchant scrolls
    ITEM_FLAG_MILLABLE              = 0x20000000,
    ITEM_FLAG_UNKNOWN_31            = 0x40000000,
    ITEM_FLAG_UNKNOWN_32            = 0x80000000
};

enum ITEM_FLAGS2
{
    ITEM_FLAG2_HORDE_ONLY               = 0x00001,
    ITEM_FLAG2_ALLIANCE_ONLY            = 0x00002,
    ITEM_FLAG2_EXT_COST_REQUIRES_GOLD   = 0x00004,
    ITEM_FLAG2_NEED_ROLL_DISABLED       = 0x00100,
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

#define MAX_ITEM_PROTO_DAMAGES 2
#define MAX_ITEM_PROTO_SOCKETS 3
#define MAX_ITEM_PROTO_SPELLS  5
#define MAX_ITEM_PROTO_STATS  10

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
    uint32_t RequiredSkill;
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
};

typedef struct
{
    int32_t setid;
    uint32_t itemscount;
    //Spell* spell[8];
} ItemSet;


#endif // ITEMPROTOTYPE_H
