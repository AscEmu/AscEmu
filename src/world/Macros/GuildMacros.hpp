/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

/// -
#define CHARTER_DISPLAY_ID 16161

/// -
#define MAX_GUILD_BANK_SLOTS        98

/// Maximum number of guild bank tabs, 6 for pre-Cata, 8 for Cata and later
inline constexpr uint8_t MAX_GUILD_BANK_TABS = 8;

inline uint8_t getGuildBankMaxTabs()
{
    return WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata) ? 8 : 6;
}

///
inline constexpr uint8_t GUILD_BANK_MONEY_TAB = 100;

/// -
#define MIN_GUILD_RANKS             2
#define MAX_GUILD_RANKS             10

/// -
#define UNCAPPED_GUILD_LEVEL        20

/// -
#define GUILD_RANK_NONE             0xFF
#define UNDEFINED_GUILD_TAB         0xFF
#define UNDEFINED_TAB_SLOT          0xFF
#define UNLIMITED_WITHDRAW_MONEY    0xFFFFFFFF
#define UNLIMITED_WITHDRAW_SLOTS    0xFFFFFFFF
#define UNK_EVENT_LOG_GUID          0xFFFFFFFF

/// -
#define MAX_GUILD_BANK_TAB_TEXT_LEN 500
#define GOLD                        10000
#define EMBLEM_PRICE                10 * GOLD
