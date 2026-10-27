/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const EcoDomeAldaniEncounterCount = 3;

enum EcoDomeAldaniData
{
    DATA_AZHICCAR               = 0,    // 3107
    // Taah'bat and A'wazj
    DATA_TAAHBAT_AND_AWAZJ      = 1,    // 3108
    DATA_SOUL_SCRIBE            = 2     // 3109
};

enum EcoDomeAldaniCreatures
{
    BOSS_AZHICCAR                   = 234893,
    NPC_AZHICCAR_FRENZIED_MITE      = 236190,

    // Taah'bat rides A'wazj and has no spawn of its own
    BOSS_TAAHBAT                    = 234933,
    BOSS_AWAZJ                      = 237514,
    NPC_AWAZJ_BINDING_JAVELIN       = 244302,

    BOSS_SOUL_SCRIBE                = 239058,
    NPC_SOUL_SCRIBE_WHISPER_OF_FATE = 242003
};

enum EcoDomeAldaniActions
{
    ACTION_TAAHBAT_BLITZ_BEGIN      = 1,
    ACTION_TAAHBAT_BLITZ_END,
    ACTION_TAAHBAT_DESTABILIZED,
    ACTION_TAAHBAT_WARP_STRUCK,
    ACTION_AWAZJ_FELL
};

enum AzhiccarSpells
{
    SPELL_AZHICCAR_DEVOUR                   = 1217232,
    SPELL_AZHICCAR_FEAST                    = 1217241,
    SPELL_AZHICCAR_INVADING_SHRIEK          = 1217327,
    SPELL_AZHICCAR_TOXIC_REGURGITATION      = 1217436,
    SPELL_AZHICCAR_TOXIC_REGURGITATION_DOT  = 1217439,
    SPELL_AZHICCAR_THRASH                   = 1217664,
    SPELL_AZHICCAR_MITE_UNCONTROLLED        = 1231811
};

enum TaahbatAwazjSpells
{
    SPELL_TAAHBAT_BEASTMASTERS_BOND     = 1219417,
    SPELL_TAAHBAT_ARCANE_BLITZ          = 1219700,
    SPELL_TAAHBAT_INCORPOREAL           = 1219457,
    SPELL_TAAHBAT_DESTABILIZED          = 1219731,
    SPELL_TAAHBAT_ARCANE_OVERLOAD       = 1220497,

    SPELL_AWAZJ_BLITZ_WARP_STRIKE       = 1227137,
    SPELL_AWAZJ_WARP_STRIKE             = 1220386,
    SPELL_AWAZJ_BINDING_JAVELIN         = 1219536,
    SPELL_AWAZJ_BINDING_JAVELIN_SNARE   = 1220671,
    SPELL_AWAZJ_BINDING_JAVELIN_PULSE   = 1219704,
    SPELL_AWAZJ_RIFT_CLAWS              = 1219482
};

enum SoulScribeSpells
{
    SPELL_SOUL_SCRIBE_WHISPERS_OF_FATE      = 1224793,
    SPELL_SOUL_SCRIBE_ETERNAL_WEAVE         = 1236703,
    SPELL_SOUL_SCRIBE_ETERNAL_WEAVE_HIT     = 1236910,
    SPELL_SOUL_SCRIBE_DREAD_OF_THE_UNKNOWN  = 1225218,
    SPELL_SOUL_SCRIBE_DREAD_DETONATION      = 1225308,
    SPELL_SOUL_SCRIBE_CEREMONIAL_DAGGER     = 1225162,
    SPELL_SOUL_SCRIBE_DAGGER_STRIKE         = 1225179,
    SPELL_SOUL_SCRIBE_WOUNDED_FATE          = 1226444
};
