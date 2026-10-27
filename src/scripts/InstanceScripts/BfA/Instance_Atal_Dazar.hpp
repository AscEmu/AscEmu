/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const AtalDazarEncounterCount = 4;

enum AtalDazarData
{
    DATA_ALUNZA  = 0, // 2084
    DATA_VOLKAAL = 1, // 2085
    DATA_REZAN   = 2, // 2086
    DATA_YAZMA   = 3  // 2087
};

enum AtalDazarCreatures
{
    BOSS_ALUNZA                 = 122967,
    // second Alun'za template of the same encounter
    BOSS_ALUNZA_ALT             = 129614,
    NPC_CORRUPTED_GOLD          = 130738,
    NPC_SPIRIT_OF_GOLD          = 131009,

    BOSS_VOLKAAL                = 122965,
    NPC_REANIMATION_TOTEM       = 125977,

    BOSS_REZAN                  = 122963,

    BOSS_YAZMA                  = 122968,
    // second Yazma template of the same encounter
    BOSS_YAZMA_ALT              = 129412,
    NPC_SOULSPAWN               = 125828,
    NPC_ECHO_OF_SHADRA          = 126734
};

enum AtalDazarGameObjects
{
    // Vol'kaal's terrace
    GO_VOLKAAL_GATE_1           = 292402,
    GO_VOLKAAL_GATE_2           = 292399,
    GO_VOLKAAL_GATE_3           = 292400,
    GO_VOLKAAL_GATE_4           = 292401,

    // way up to Yazma, free once the three lower bosses are slain
    GO_YAZMA_GATE_1             = 292397,
    GO_YAZMA_GATE_2             = 292398,
    GO_WATERFALL_STAIRS         = 278422
};

enum AtalDazarActions
{
    ACTION_VOLKAAL_TOTEM_FALLEN = 1,
    ACTION_VOLKAAL_TOTEM_RISEN,
    ACTION_TOTEM_SHATTER,
    ACTION_TOTEM_RESET
};

enum AlunzaSpells
{
    SPELL_ALUNZA_RITUAL             = 258386,
    SPELL_ALUNZA_GILDED_CLAWS       = 255579,
    SPELL_ALUNZA_TAINTED_BLOOD      = 255592,
    SPELL_ALUNZA_MOLTEN_GOLD        = 255583,
    SPELL_ALUNZA_TRANSFUSION        = 255577,
    SPELL_CORRUPTED_GOLD_AURA       = 258703,
    SPELL_CORRUPTED_GOLD_TOUCH      = 258709
};

enum VolkaalSpells
{
    SPELL_VOLKAAL_BAD_VOODOO        = 250192,
    SPELL_VOLKAAL_TOXIC_LEAP        = 250258,
    SPELL_VOLKAAL_NOXIOUS_STENCH    = 259572,
    SPELL_VOLKAAL_RAPID_DECAY       = 250241,
    SPELL_TOTEM_SOUL_ANCHOR         = 259537,
    SPELL_TOTEM_REANIMATE           = 259531
};

enum RezanSpells
{
    SPELL_REZAN_TERRIFYING_VISAGE   = 255371,
    SPELL_REZAN_TAIL                = 255373,
    SPELL_REZAN_SERRATED_TEETH      = 255434,
    SPELL_REZAN_PURSUIT             = 257407,
    SPELL_REZAN_DEVOUR              = 255421
};

enum YazmaSpells
{
    SPELL_YAZMA_WRACKING_PAIN       = 250096,
    SPELL_YAZMA_SKEWER              = 249919,
    SPELL_YAZMA_SOULREND            = 249923,
    SPELL_YAZMA_ECHOES_OF_SHADRA    = 250050,
    SPELL_SOULSPAWN_SOULFEAST       = 256577,
    SPELL_ECHO_OF_SHADRA_BURST      = 250022
};
