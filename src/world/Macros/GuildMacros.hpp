/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

/// -
#define CHARTER_DISPLAY_ID 16161

/// -
#define MAX_GUILD_BANK_SLOTS        98

/// -
#if VERSION_STRING >= Cata
#define MAX_GUILD_BANK_TABS         8
#else
#define MAX_GUILD_BANK_TABS         6
#endif

/// -
#define GUILD_BANK_MONEY_TAB        100

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
