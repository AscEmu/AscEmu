/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include <cstdint>

static inline constexpr int32_t PET_HAPPINESS_UPDATE_VALUE = 333000;
static inline constexpr uint16_t PET_HAPPINESS_UPDATE_TIMER = 7500;

static inline constexpr uint8_t PET_MAX_ACTION_BAR_SLOT = 10;

union PetActionButtonData
{
    struct Parts
    {
        uint32_t spellId : 24;
        uint32_t state   : 8;
    } parts;
    uint32_t raw;
};

enum PetEntries : uint32_t
{
    PET_WATER_ELEMENTAL                 = 510,
    PET_WATER_ELEMENTAL_NEW             = 37994,
    PET_IMP                             = 416,
    PET_VOIDWALKER                      = 1860,
    PET_SUCCUBUS                        = 1863,
    PET_FELHUNTER                       = 417,
    PET_INFERNAL                        = 89,
    PET_DOOMGUARD                       = 11859,
    PET_FELGUARD                        = 17252,
    PET_SHADOWFIEND                     = 19668,
    PET_GHOUL                           = 26125,
    PET_DANCING_RUNEWEAPON              = 27893,
    PET_SPIRITWOLF                      = 29264
};

/* Taken from ItemPetFood.dbc
 * Each value is equal to a flag
 * so 1 << PET_FOOD_BREAD for example
 * will result in a usable value.
 */
enum PetFoodDiets : uint8_t
{
    PET_FOOD_NONE,
    PET_FOOD_MEAT,
    PET_FOOD_FISH,
    PET_FOOD_CHEESE,
    PET_FOOD_BREAD,
    PET_FOOD_FUNGUS,
    PET_FOOD_FRUIT,
    PET_FOOD_RAW_MEAT,                  /// not used in pet diet
    PET_FOOD_RAW_FISH                   /// not used in pet diet
};

// Used with PET_SPELL_STATE_SET_REACT
enum PetReactStates : uint8_t
{
    PET_STATE_PASSIVE                   = 0,
    PET_STATE_DEFENSIVE                 = 1,
    PET_STATE_AGGRESSIVE                = 2
};

// Used with PET_SPELL_STATE_SET_ACTION
enum PetCommands : uint8_t
{
    PET_ACTION_STAY                     = 0,
    PET_ACTION_FOLLOW                   = 1,
    PET_ACTION_ATTACK                   = 2,
    PET_ACTION_DISMISS                  = 3,
    PET_ACTION_CASTING                  = 4
};

enum PetActionFeedback : uint8_t
{
    PET_FEEDBACK_NONE,
    PET_FEEDBACK_PET_DEAD,
    PET_FEEDBACK_NOTHING_TO_ATTACK,
    PET_FEEDBACK_CANT_ATTACK_TARGET
};

enum PetSpellState : uint8_t
{
    PET_SPELL_STATE_PASSIVE             = 0x01, // passive spell
    PET_SPELL_STATE_SET_REACT           = 0x06,
    PET_SPELL_STATE_SET_ACTION          = 0x07,

    PET_SPELL_STATE_DEFAULT             = 0x81, // castable spell
    PET_SPELL_STATE_AUTOCAST            = 0xC1, // enabled autocast
    PET_SPELL_STATE_NOT_SPELL           = 0x04  // react or action
};

enum PetSlots : uint8_t
{
#if VERSION_STRING < WotLK
    PET_SLOT_MAX_ACTIVE_SLOT            = 1,
    PET_SLOT_MAX_STABLE_SLOT            = 2,
#elif VERSION_STRING == WotLK
    PET_SLOT_MAX_ACTIVE_SLOT            = 1,
    PET_SLOT_MAX_STABLE_SLOT            = 4,
#elif VERSION_STRING == Cata
    PET_SLOT_MAX_ACTIVE_SLOT            = 5,
    PET_SLOT_MAX_STABLE_SLOT            = 20,
#elif VERSION_STRING >= Mop
    PET_SLOT_MAX_ACTIVE_SLOT            = 5,
    PET_SLOT_MAX_STABLE_SLOT            = 50,
#endif

    PET_SLOT_FIRST_ACTIVE_SLOT          = 0,
    PET_SLOT_FIRST_STABLE_SLOT          = 5, // Reserve 0-4 slots for active pets in all versions
    PET_SLOT_LAST_STABLE_SLOT           = PET_SLOT_FIRST_STABLE_SLOT + PET_SLOT_MAX_STABLE_SLOT, // Slot is zero indexed so actual value is -1
    PET_SLOT_MAX_TOTAL_PET_COUNT        = PET_SLOT_MAX_ACTIVE_SLOT + PET_SLOT_MAX_STABLE_SLOT,
};

enum HappinessStates : uint8_t
{
    HAPPINESS_STATE_UNHAPPY             = 1,
    HAPPINESS_STATE_CONTENT             = 2,
    HAPPINESS_STATE_HAPPY               = 3
};

enum AutoCastEvents : uint8_t
{
    AUTOCAST_EVENT_NONE                 = 0,
    AUTOCAST_EVENT_ATTACK               = 1,
    AUTOCAST_EVENT_ON_SPAWN             = 2,
    AUTOCAST_EVENT_OWNER_ATTACKED       = 3,
    AUTOCAST_EVENT_LEAVE_COMBAT         = 4,
    AUTOCAST_EVENT_COUNT                = 5
};

enum PetTypes : uint8_t
{
    PET_TYPE_NONE                       = 0,
    PET_TYPE_HUNTER                     = 1, // Hunter pet
    PET_TYPE_SUMMON                     = 2, // All summoned pets (warlock minions, water elemental etc)
};

namespace PetStableResult
{
    enum Messages : uint8_t
    {
        NotEnoughMoney                  = 1,
#if VERSION_STRING >= Cata
        SlotLocked                      = 3,
#endif
        Error                           = 6, // todo: confirm if exists in cata or replaced by 12
        StableSuccess                   = 8,
        UnstableSuccess                 = 9,
        BuySuccess                      = 10,
#if VERSION_STRING == WotLK
        ExoticNotAvailable              = 12,
#elif VERSION_STRING >= Cata
        ExoticNotAvailable              = 11,
        InternalError                   = 12,
#endif
    };
}
