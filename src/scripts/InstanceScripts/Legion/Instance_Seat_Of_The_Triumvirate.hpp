/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const SeatOfTheTriumvirateEncounterCount = 4;

enum SeatOfTheTriumvirateData
{
    DATA_ZURAAL_THE_ASCENDED    = 0,  // 2065
    // Saprish, Darkfang and Shadewing
    DATA_SAPRISH                = 1,  // 2066
    DATA_VICEROY_NEZHAR         = 2,  // 2067
    DATA_LURA                   = 3   // 2068
};

enum SeatOfTheTriumvirateCreatures
{
    BOSS_ZURAAL_THE_ASCENDED    = 122313,
    NPC_COALESCED_VOID          = 122716,

    BOSS_SAPRISH                = 122316,
    BOSS_DARKFANG               = 122319,
    BOSS_SHADEWING              = 125340,
    NPC_VOID_TRAP               = 124427,

    BOSS_VICEROY_NEZHAR         = 122056,
    NPC_NEZHAR_CENTER_POINT     = 122832,
    NPC_UMBRAL_TENTACLE         = 122827,

    BOSS_LURA                   = 124729,
    NPC_GREATER_RIFT_WARDEN     = 124745
};

enum SeatOfTheTriumvirateGameObjects
{
    GO_SOTT_VOID_WALL       = 273789,
    GO_SOTT_KARABOR_DOOR    = 272062,
    GO_SOTT_HALLWAY_DOOR    = 273661
};

enum SeatOfTheTriumvirateActions
{
    ACTION_SOTT_OVERLOAD_TRAP = 1
};

enum ZuraalSpells
{
    SPELL_ZURAAL_NULL_PALM          = 246134,
    SPELL_ZURAAL_DECIMATE           = 244579,

    SPELL_COALESCED_VOID_SPAWN      = 244603,
    SPELL_COALESCED_VOID_EXPULSION  = 244599
};

enum SaprishSpells
{
    SPELL_SAPRISH_UMBRAL_FLANKING_MARK  = 247245,
    SPELL_SAPRISH_UMBRAL_FLANKING       = 247246,
    SPELL_SAPRISH_HUNTERS_RUSH          = 247145,
    SPELL_SAPRISH_OVERLOAD_TRAP         = 247206,

    SPELL_VOID_TRAP_AREA                = 245903,
    SPELL_VOID_TRAP_SNARE               = 246026,
    SPELL_VOID_TRAP_OVERLOAD            = 247207,

    SPELL_DARKFANG_SHADOW_POUNCE        = 245741,
    SPELL_DARKFANG_RAVAGING_DARKNESS    = 245802,

    SPELL_SHADEWING_SWOOP               = 248830,
    SPELL_SHADEWING_DREAD_SCREECH       = 248831
};

enum NezharSpells
{
    SPELL_NEZHAR_MIND_BLAST         = 244750,
    SPELL_NEZHAR_HOWLING_DARK       = 244751,
    SPELL_NEZHAR_ENTROPIC_FORCE     = 246324,

    SPELL_TENTACLE_BASE_VISUAL      = 244924,
    SPELL_TENTACLE_VOID_LASHING     = 244916
};

enum LuraSpells
{
    SPELL_LURA_CALL_TO_THE_VOID     = 247795,
    SPELL_LURA_NAARUS_LAMENT        = 245393,
    SPELL_LURA_DARKENED_SHROUD      = 254020,
    SPELL_LURA_BACKLASH             = 247816,
    SPELL_LURA_VOID_BLAST           = 245289,
    SPELL_LURA_UMBRAL_CADENCE       = 247930,

    SPELL_RIFT_WARDEN_FRAGMENT      = 245164,
    SPELL_RIFT_WARDEN_SORROW_SHOCK  = 247948
};
