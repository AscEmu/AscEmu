/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const SpiresOfAscensionEncounterCount = 4;

enum SpiresOfAscensionData
{
    DATA_VENTUNAX   = 0,  // 2356
    // Kin-Tara and Azules
    DATA_KIN_TARA   = 1,  // 2357
    DATA_ORYPHRION  = 2,  // 2358
    DATA_DEVOS      = 3   // 2359
};

enum SpiresOfAscensionCreatures
{
    BOSS_VENTUNAX           = 162058,
    NPC_SHADOWHIRL          = 166136,
    NPC_SHADOWHIRL_TRAIL    = 166152,

    BOSS_KIN_TARA           = 162059,
    BOSS_AZULES             = 163077,
    // companion template of a later journal revision
    BOSS_AZORAS             = 174212,

    BOSS_ORYPHRION          = 162060,
    NPC_COALESCED_ANIMA     = 165807,

    BOSS_DEVOS              = 162061
};

enum SpiresOfAscensionActions
{
    ACTION_SPIRES_COMPANION_FALLEN = 1,
    ACTION_SPIRES_ANIMA_RETURN
};

enum SpiresVentunaxSpells
{
    SPELL_VENTUNAX_DARK_STRIDE          = 324148,
    SPELL_VENTUNAX_BLINDING_FLASH       = 324205,
    SPELL_VENTUNAX_RECHARGE             = 334485,
    SPELL_SHADOWHIRL_DARK_BOLT          = 324141
};

enum SpiresKinTaraSpells
{
    SPELL_KINTARA_OVERHEAD_SLASH        = 320966,
    SPELL_KINTARA_DARK_LANCE            = 327481,
    SPELL_KINTARA_CHARGED_SPEAR         = 321009,
    SPELL_KINTARA_ENRAGE                = 323828,

    SPELL_AZULES_ATTENUATED_BARRAGE     = 324370,
    SPELL_AZORAS_ATTENUATED_BARRAGE     = 342654,
    SPELL_AZULES_MAW_TOUCHED_VENOM      = 317626,
    SPELL_AZULES_INSIDIOUS_VENOM        = 323636
};

enum SpiresOryphrionSpells
{
    SPELL_ORYPHRION_DRAINED             = 323878,
    SPELL_ORYPHRION_RECHARGE_ANIMA      = 324046,
    SPELL_ORYPHRION_EMPYREAL_ORDNANCE   = 324444,
    SPELL_ORYPHRION_PURIFYING_BLAST     = 323195,
    SPELL_ORYPHRION_CHARGED_STOMP       = 324608,
    SPELL_ANIMA_OVERCHARGE_ANIMA        = 324307
};

enum SpiresDevosSpells
{
    SPELL_DEVOS_ABYSSAL_DETONATION      = 334625,
    SPELL_DEVOS_LOST_CONFIDENCE         = 322818,
    SPELL_DEVOS_LINGERING_DOUBT         = 322817,
    SPELL_DEVOS_RUN_THROUGH             = 323943,
    SPELL_DEVOS_UNLEASHED_ANIMA         = 323011
};
