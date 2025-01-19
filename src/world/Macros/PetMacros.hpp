/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

/// -
#define PET_WATER_ELEMENTAL 510
#define PET_WATER_ELEMENTAL_NEW 37994
#define PET_IMP 416
#define PET_VOIDWALKER 1860
#define PET_SUCCUBUS 1863
#define PET_FELHUNTER 417
#define PET_FELGUARD 17252
#define PET_SHADOWFIEND 19668
#define PET_GHOUL 26125

/// -
//#define SPIRITWOLF 29264

/// -
//#define DANCINGRUNEWEAPON 27893

/// -
#define PET_SPELL_SPAM_COOLDOWN 2000        // applied only to spells that have no cooldown

/// -
#define PET_TALENT_TREE_START 409           // Tenacity

/// -
#define PET_TALENT_TREE_END 411             // Cunning

/// -
//#define PET_DELAYED_REMOVAL_TIME 60000    // 1 min

/// -
#define DEFAULT_SPELL_STATE 0x8100

/// -
#define AUTOCAST_SPELL_STATE 0xC100

/// -
#define PET_HAPPINESS_UPDATE_VALUE 333000

/// -
#define PET_HAPPINESS_UPDATE_TIMER 7500

/// -
#define PET_ACTION_ACTION 0x700             // 1792

/// -
#define PET_ACTION_STATE 0x600              // 1536

///\todo grep see the way pet spells contain the same flag?
#define PET_ACTION_SPELL 0xC100             // 49408
#define PET_ACTION_SPELL_1 0x8100           // 33024
#define PET_ACTION_SPELL_2 0x0100           // 256