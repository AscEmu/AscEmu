/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

inline constexpr uint32_t DBC_NUM_RACES = 92;

//////////////////////////////////////////////////////////////////////////////////////////
// DBC_TAXI_MASK_SIZE
//
// \param max taxi mask
//
// Vanilla = ??
// The Burning Crusade = ??
// Wrath of the Lich King = 12
// Cataclysm = 114
// Mists of Pandaria = ??
// Warlords of Draenor = ??
// Legion = ??
// Battle for Azeroth = ??
// Shadowlands = ??
//
//////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    #define DBC_TAXI_MASK_SIZE 12
#elif VERSION_STRING == TBC
    #define DBC_TAXI_MASK_SIZE 12
#elif VERSION_STRING == WotLK
    #define DBC_TAXI_MASK_SIZE 14
#elif VERSION_STRING == Cata
    #define DBC_TAXI_MASK_SIZE 114
#elif VERSION_STRING == Mop
    #define DBC_TAXI_MASK_SIZE 255
#elif VERSION_STRING == WoD
    #define DBC_TAXI_MASK_SIZE 217
#elif VERSION_STRING == Legion || VERSION_STRING == BfA || VERSION_STRING == Shadowlands || VERSION_STRING == Dragonflight || VERSION_STRING == TheWarWithin || VERSION_STRING == Midnight
    #define DBC_TAXI_MASK_SIZE 258
#endif

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

//////////////////////////////////////////////////////////////////////////////////////////
// DBC_PLAYER_SKILL_MAX
//
// \param skill max
//
// Vanilla = 300
// The Burning Crusade = 375
// Wrath of the Lich King = 450
// Cataclysm = 525
// Mists of Pandaria = 600
// Warlords of Draenor = 700
// Legion = 800
//
//////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    #define DBC_PLAYER_SKILL_MAX 300
#elif VERSION_STRING == TBC
    #define DBC_PLAYER_SKILL_MAX 375
#elif VERSION_STRING == WotLK
    #define DBC_PLAYER_SKILL_MAX 450
#elif VERSION_STRING == Cata
    #define DBC_PLAYER_SKILL_MAX 525
#elif VERSION_STRING >= Mop
    #define DBC_PLAYER_SKILL_MAX 600
#endif

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

/// -
#if VERSION_STRING > Classic
#define PLAYER_EXPLORED_ZONES_LENGTH 128
#else
#define PLAYER_EXPLORED_ZONES_LENGTH 64
#endif

// #define ACHIEVEMENT_SEND_DELAY 1000 // we have this delay of sending auras to other players so client will have time to create object first
#define LOGIN_CIENT_SEND_DELAY 1000 // we have this delay of sending auras to other players so client will have time to create object first

/// -
#ifdef FT_DUAL_SPEC
#define MAX_SPEC_COUNT 2
#else
#define MAX_SPEC_COUNT 1
#endif

/// -
#if VERSION_STRING == Classic
#define MAX_QUEST_SLOT 20
#else
#define MAX_QUEST_SLOT 25
#endif

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
