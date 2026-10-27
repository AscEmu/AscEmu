/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const AltarOfFangsEncounterCount = 3;

enum AltarOfFangsData
{
    DATA_RAVI          = 0,  // 3456
    DATA_WRITHING_COIL = 1,  // 3457
    DATA_ZULJAN        = 2   // 3458
};

enum AltarOfFangsCreatures
{
    BOSS_RAVI                 = 259445,

    BOSS_WRITHING_COIL        = 259446,
    NPC_UNCOILED_WRITHE       = 262398,
    // second Uncoiled Writhe template with the same model
    NPC_UNCOILED_WRITHE_ALT   = 270417,

    BOSS_ZULJAN               = 259447
};

enum AltarOfFangsRaviSpells
{
    SPELL_RAVI_HYDRASTRIKE     = 1298683,
    SPELL_RAVI_TRIPLE_SHOT     = 1297876,
    SPELL_RAVI_REGURGITATE     = 1296069,
    SPELL_RAVI_RAVENOUS_STOMP  = 1307915,
    SPELL_RAVI_SCENT_OF_BLOOD  = 1310378,
    SPELL_RAVI_SSSCAVENGING    = 1298221,
    SPELL_RAVI_FEEDING_FRENZY  = 1307765,
    SPELL_RAVI_CARRION_BURST   = 1307700
};

enum AltarOfFangsWrithingCoilSpells
{
    SPELL_COIL_DEATH_RATTLE          = 1299053,
    SPELL_COIL_UNCOIL                = 1287811,
    SPELL_COIL_TOXIC_BARRAGE         = 1310357,
    SPELL_COIL_TOXIC_ATROPHY         = 1310974,
    SPELL_COIL_VINDICTIVE_ONSLAUGHT  = 1299940,
    SPELL_COIL_BURROWING_CHARGE      = 1300083,
    SPELL_COIL_VENOM_JET             = 1300044,
    SPELL_COIL_TAIL_SCYTHE           = 1298949,
    SPELL_COIL_SYNCHRONIZED_VENOM    = 1299189,

    SPELL_WRITHE_UNCOILED            = 1300612,
    SPELL_WRITHE_UNDERMINING         = 1305393
};

enum AltarOfFangsZuljanSpells
{
    SPELL_ZULJAN_RITUAL_OF_THE_FANG = 1300876,
    SPELL_ZULJAN_FANG_EMPOWERED     = 1300888,
    SPELL_ZULJAN_RITUAL_VENOM       = 1300894,
    SPELL_ZULJAN_BLOODLETTING       = 1301217,
    SPELL_ZULJAN_BONESLICER         = 1301413,
    SPELL_ZULJAN_AXEGRINDER         = 1301111,
    SPELL_ZULJAN_CHOP_DOWN          = 1301350
};
