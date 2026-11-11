/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

/// -
#define MAX_ITEM_PROTO_DAMAGES 2

/// -
#define MAX_ITEM_PROTO_SOCKETS 3

/// -
#define MAX_ITEM_PROTO_SPELLS  5

/// -
#define MAX_ITEM_PROTO_STATS  10


/// -
#define MAX_INVENTORY_SLOT 150

/// -
#define ITEM_NO_SLOT_AVAILABLE -1   /// works for all kind of slots now

/// -
#define INVENTORY_SLOT_NOT_SET -1

/// Count of visible items (equippable items + bags)
/// i.e. arrow quivers are also visible on character screen
#define DBC_PLAYER_ITEMS 23 // INVENTORY_SLOT_BAG_END

/// -
#define RANDOM_SUFFIX_MAGIC_CALCULATION(__suffix, __scale) Util::float2int32(float(__suffix) * float(__scale) / 10000.0f);

namespace ItemSlotUtils
{
    // Keyring: Slots 86 to 117 (32 Slots, til WotLK, since Cata unused)
    inline constexpr uint8_t KEYRING_START = 86;
    inline constexpr uint8_t KEYRING_COUNT = 32;
    inline constexpr uint8_t KEYRING_END = KEYRING_START + KEYRING_COUNT; // 118

    // Currency Tokens: Slots 118 to 149 (32 Slots, active in WotLK, since Cata own  UI / system)
    inline constexpr uint8_t CURRENCY_START = 118;
    inline constexpr uint8_t CURRENCY_COUNT = 32;
    inline constexpr uint8_t CURRENCY_END = CURRENCY_START + CURRENCY_COUNT; // 150

    [[nodiscard]] constexpr bool isKeyringSlot(uint8_t slot) noexcept
    {
        return slot >= KEYRING_START && slot < KEYRING_END;
    }

    [[nodiscard]] constexpr bool isCurrencySlot(uint8_t slot) noexcept
    {
        return slot >= CURRENCY_START && slot < CURRENCY_END;
    }
}

// Downwards compatibility defines, for code that doesn't use ItemSlotUtils yet
#define INVENTORY_KEYRING_START     ItemSlotUtils::KEYRING_START
#define INVENTORY_KEYRING_END       ItemSlotUtils::KEYRING_END
#define CURRENCYTOKEN_SLOT_START    ItemSlotUtils::CURRENCY_START
#define CURRENCYTOKEN_SLOT_END      ItemSlotUtils::CURRENCY_END
