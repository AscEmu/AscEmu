/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const CityOfThreadsEncounterCount = 4;

enum CityOfThreadsData
{
    DATA_THE_COAGLAMATION       = 0,    // 2905
    DATA_ORATOR_KRIX_VIZK       = 1,    // 2907
    // Nx and Vx
    DATA_FANGS_OF_THE_QUEEN     = 2,    // 2908
    DATA_IZO_THE_GRAND_SPLICER  = 3     // 2909
};

enum CityOfThreadsCreatures
{
    BOSS_THE_COAGLAMATION       = 216320,

    // 220769 is the conversation copy of the orator
    BOSS_ORATOR_KRIX_VIZK       = 216619,

    BOSS_FANG_NX                = 216648,
    BOSS_FANG_VX                = 216649,
    NPC_NX_ECHOING_SHADE        = 219675,

    BOSS_IZO_THE_GRAND_SPLICER  = 216658,
    NPC_IZO_RAVENOUS_SCARAB     = 219198,
    NPC_IZO_UMBRAL_WEB          = 220065
};

enum CityOfThreadsActions
{
    ACTION_FANGS_SYNERGIC_STEP  = 1,
    ACTION_FANGS_PARTNER_FELL
};

enum CoaglamationSpells
{
    SPELL_COAGLAMATION_VISCOUS_DARKNESS = 441216,
    SPELL_COAGLAMATION_OOZING_SMASH     = 461842,
    SPELL_COAGLAMATION_BLOOD_SURGE      = 438620,
    SPELL_COAGLAMATION_DARK_PULSE       = 437533
};

enum KrixVizkSpells
{
    SPELL_KRIXVIZK_CHAINS_OF_OPPRESSION         = 434691,
    SPELL_KRIXVIZK_SUBJUGATE                    = 434722,
    SPELL_KRIXVIZK_TERRORIZE                    = 434779,
    SPELL_KRIXVIZK_VOCIFEROUS_INDOCTRINATION    = 434829,
    SPELL_KRIXVIZK_INDOCTRINATION_PULSE         = 434832
};

enum FangsSpells
{
    SPELL_FANGS_TWIN_FANGS          = 439518,
    SPELL_FANGS_SYNERGIC_STEP       = 439522,
    SPELL_FANGS_KNIFE_THROW         = 440107,

    SPELL_NX_SHADE_SLASH            = 439621,
    SPELL_NX_SHADE_SLASH_ECHO       = 439686,
    SPELL_NX_DUSKBRINGER            = 439692,

    SPELL_VX_ICE_SICKLES            = 440218,
    SPELL_VX_ICE_SICKLES_HIT        = 440238,
    SPELL_VX_RIME_DAGGER            = 440468,
    SPELL_VX_RIME_DAGGER_BURST      = 441305,
    SPELL_VX_FREEZING_BLOOD         = 440470,
    SPELL_VX_FROZEN_SOLID           = 458741
};

enum IzoSpells
{
    SPELL_IZO_SHIFTING_ANOMALIES            = 439401,
    SPELL_IZO_SPLICE                        = 439341,
    SPELL_IZO_TREMOR_SLAM                   = 437700,
    SPELL_IZO_UMBRAL_WEAVE                  = 438860,
    SPELL_IZO_UMBRAL_WEAVE_BIND             = 439324,
    SPELL_IZO_PROCESS_OF_ELIMINATION        = 439646,
    SPELL_IZO_PROCESS_OF_ELIMINATION_BLAST  = 439763,
    SPELL_IZO_SCARAB_GORGE                  = 450042
};
