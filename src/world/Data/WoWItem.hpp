/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

/*
 * THIS IS A WOW DATA STRUCT FILE
 *
 * DO NOT ADD ADDITIONAL MEMBERS TO IT
 *
 * DO NOT EDIT IT UNLESS YOU KNOW EXACTLY WHAT YOU ARE DOING
 */

#pragma once

#include "WoWObject.hpp"

#include <array>

#pragma pack(push, 1)

#if VERSION_STRING == Classic

static inline constexpr uint8_t WOWITEM_SPELL_CHARGES_COUNT = 5;
static inline constexpr uint8_t WOWITEM_ENCHANTMENT_COUNT = 7;

//\todo verify this
struct WoWItem_Enchantment
{
    uint32_t id;
    uint32_t duration;
    uint32_t charges;
};

struct WoWItem : WoWObject
{
    guid_union owner_guid;
    guid_union container_guid;
    guid_union creator_guid;
    guid_union gift_creator_guid;
    uint32_t stack_count;
    uint32_t duration;
    std::array<int32_t, WOWITEM_SPELL_CHARGES_COUNT> spell_charges;
    uint32_t flags;
    std::array<WoWItem_Enchantment, WOWITEM_ENCHANTMENT_COUNT> enchantment;
    uint32_t property_seed;
    uint32_t random_properties_id;
    uint32_t item_text_id;
    uint32_t durability;
    uint32_t max_durability;
};
#endif

#if VERSION_STRING == TBC

static inline constexpr uint8_t WOWITEM_SPELL_CHARGES_COUNT = 5;
static inline constexpr uint8_t WOWITEM_ENCHANTMENT_COUNT = 11;

//\todo verify this
struct WoWItem_Enchantment
{
    uint32_t id;
    uint32_t duration;
    uint32_t charges;
};

struct WoWItem : WoWObject
{
    guid_union owner_guid;
    guid_union container_guid;
    guid_union creator_guid;
    guid_union gift_creator_guid;
    uint32_t stack_count;
    uint32_t duration;
    std::array<int32_t, WOWITEM_SPELL_CHARGES_COUNT> spell_charges;
    uint32_t flags;
    std::array<WoWItem_Enchantment, WOWITEM_ENCHANTMENT_COUNT> enchantment;
    uint32_t property_seed;
    uint32_t random_properties_id;
    uint32_t item_text_id;
    uint32_t durability;
    uint32_t max_durability;
};
#endif

#if VERSION_STRING == WotLK

static inline constexpr uint8_t WOWITEM_SPELL_CHARGES_COUNT = 5;
static inline constexpr uint8_t WOWITEM_ENCHANTMENT_COUNT = 12;

struct WoWItem_Enchantment
{
    uint32_t id;
    uint32_t duration;
    uint32_t charges;
};

struct WoWItem : WoWObject
{
    guid_union owner_guid;
    guid_union container_guid;
    guid_union creator_guid;
    guid_union gift_creator_guid;
    uint32_t stack_count;
    uint32_t duration;
    std::array<int32_t, WOWITEM_SPELL_CHARGES_COUNT> spell_charges;
    uint32_t flags;
    std::array<WoWItem_Enchantment, WOWITEM_ENCHANTMENT_COUNT> enchantment;
    uint32_t property_seed;
    uint32_t random_properties_id;
    uint32_t durability;
    uint32_t max_durability;
    uint32_t create_played_time;
    uint32_t item_padding_0;
};
#endif

#if VERSION_STRING == Cata

static inline constexpr uint8_t WOWITEM_SPELL_CHARGES_COUNT = 5;
static inline constexpr uint8_t WOWITEM_ENCHANTMENT_COUNT = 15;

struct WoWItem_Enchantment
{
    uint32_t id;
    uint32_t duration;
    uint32_t charges;
};

struct WoWItem : WoWObject
{
    guid_union owner_guid;
    guid_union container_guid;
    guid_union creator_guid;
    guid_union gift_creator_guid;
    uint32_t stack_count;
    uint32_t duration;
    std::array<int32_t, WOWITEM_SPELL_CHARGES_COUNT> spell_charges;
    uint32_t flags;
    std::array<WoWItem_Enchantment, WOWITEM_ENCHANTMENT_COUNT> enchantment;
    uint32_t property_seed;
    uint32_t random_properties_id;
    uint32_t durability;
    uint32_t max_durability;
    uint32_t create_played_time;
};
#endif

#if VERSION_STRING == Mop

static inline constexpr uint8_t WOWITEM_SPELL_CHARGES_COUNT = 5;
static inline constexpr uint8_t WOWITEM_ENCHANTMENT_COUNT = 13;

struct WoWItem_Enchantment
{
    uint32_t id;
    uint32_t duration;
    uint32_t charges;
};

struct WoWItem : WoWObject
{
    guid_union owner_guid;
    guid_union container_guid;
    guid_union creator_guid;
    guid_union gift_creator_guid;
    uint32_t stack_count;
    uint32_t duration;
    std::array<int32_t, WOWITEM_SPELL_CHARGES_COUNT> spell_charges;
    uint32_t flags;
    std::array<WoWItem_Enchantment, WOWITEM_ENCHANTMENT_COUNT> enchantment;
    uint32_t property_seed;
    uint32_t random_properties_id;
    uint32_t durability;
    uint32_t max_durability;
    uint32_t create_played_time;
    uint32_t modifier_mask;
};
#endif

#pragma pack(pop)
