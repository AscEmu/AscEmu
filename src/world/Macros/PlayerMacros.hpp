/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/ClientProtocol.hpp"
#include "AEVersion.hpp"

inline constexpr uint32_t DBC_NUM_RACES = 92;

inline constexpr size_t MAX_TAXI_MASK_SIZE = 258;

[[nodiscard]] constexpr size_t getTaxiMaskSize(WoW::Expansion expansion) noexcept
{
    switch (expansion)
    {
        case WoW::Expansion::_Classic:
        case WoW::Expansion::_TBC:
            return 12;
        case WoW::Expansion::_WotLK:
            return 14;
        case WoW::Expansion::_Cata:
            return 114;
        case WoW::Expansion::_Mop:
            return 255;
        case WoW::Expansion::_WoD:
            return 217;
        default: // Legion to Midnight (incl. fallback)
            return 258;
    }
}

[[nodiscard]] inline size_t getTaxiMaskSize() noexcept
{
    return getTaxiMaskSize(WoW::getServerExpansion());
}

// Highest level of all expansions (BfA)
inline constexpr uint32_t MAX_LEVEL_CAP = 120;

//////////////////////////////////////////////////////////////////////////////////////////
// DBC_STAT_LEVEL_CAP
//
// \param level cap for pre-generated player stats in gt*.dbc files
//
// Vanilla = 100
// The Burning Crusade = 100
// Wrath of the Lich King = 100
// Cataclysm = 100
// Mists of Pandaria = ??
// Warlords of Draenor = ??
// Legion = ??
// Battle for Azeroth = ??
// Shadowlands = ??
//
//////////////////////////////////////////////////////////////////////////////////////////

#define DBC_STAT_LEVEL_CAP 100

// Max-Capacity for internal structures (Dragonflight to Midnight need 180)
inline constexpr uint8_t PLAYER_ACTION_BUTTON_COUNT = 180;
inline constexpr size_t MIN_PLAYER_ACTION_BUTTON_PACKET_SIZE = 120 * 4;

//////////////////////////////////////////////////////////////////////////////////////////
// GLYPHS_COUNT
// 
// \param Action bar / button defines 
// 
// Vanilla = 
// The Burning Crusade = 
// Wrath of the Lich King = 
// Cataclysm = 
// Mists of Pandaria = 
// Warlords of Draenor = 
// Legion = 
//
//////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    #define GLYPHS_COUNT 6
#elif VERSION_STRING == TBC
    #define GLYPHS_COUNT 6
#elif VERSION_STRING == WotLK
    #define GLYPHS_COUNT 6
#elif VERSION_STRING == Cata
    #define GLYPHS_COUNT 9
#elif VERSION_STRING == Mop
    #define GLYPHS_COUNT 6
#elif VERSION_STRING == WoD
    #define GLYPHS_COUNT 6
#elif VERSION_STRING == Legion || VERSION_STRING == BfA || VERSION_STRING == Shadowlands || VERSION_STRING == Dragonflight || VERSION_STRING == TheWarWithin || VERSION_STRING == Midnight
    // 7.x and later have no glyph slots anymore (a glyph list per specialization); keeps the 6.x slot count until glyphs are ported
    #define GLYPHS_COUNT 6
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Minimum level required arena
#define PLAYER_ARENA_MIN_LEVEL 70

/// -
#define PLAYER_HONORLESS_TARGET_SPELL 2479

// #define ACHIEVEMENT_SEND_DELAY 1000 // we have this delay of sending auras to other players so client will have time to create object first
#define LOGIN_CIENT_SEND_DELAY 1000 // we have this delay of sending auras to other players so client will have time to create object first

/// -
#ifdef FT_DUAL_SPEC
#define MAX_SPEC_COUNT 2
#else
#define MAX_SPEC_COUNT 1
#endif

/// Maximum number of glyphs a player can have in a single spec (25 since TBC)
inline constexpr uint32_t MAX_QUEST_SLOT = 25;

//#define TOTAL_NORMAL_RUNE_TYPES 3
//#define TOTAL_USED_RUNES (TOTAL_NORMAL_RUNE_TYPES * 2)

/// -
#define MAX_RUNES 6
//#define TOTAL_RUNE_TYPES 4
//#define MAX_RUNE_VALUE 1

/// -
#define COLLISION_INDOOR_CHECK_INTERVAL 1000

/// -
#define BASE_BLOCK_CHANCE 5.0f
#define BASE_PARRY_CHANCE 5.0f

/// -
#define COOLDOWN_SKIP_SAVE_IF_MS_LESS_THAN 10000
