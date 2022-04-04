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
