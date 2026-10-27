/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const IronDocksEncounterCount = 4;

enum IronDocksData
{
    // Makogg Emberblade, Neesa Nox and Ahri'ok Dugru
    DATA_ENFORCERS = 0,  // 1748
    DATA_NOKGAR    = 1,  // 1749
    DATA_OSHIR     = 2,  // 1750
    // Skulloc, Koramar and Zoggosh
    DATA_SKULLOC   = 3   // 1754
};

enum IronDocksCreatures
{
    BOSS_NOKGAR          = 81305,
    NPC_DREADFANG        = 81297,

    BOSS_MAKOGG          = 80805,
    // second Makogg template of the same encounter
    BOSS_MAKOGG_ALT      = 86231,
    BOSS_NEESA_NOX       = 80808,
    BOSS_AHRIOK_DUGRU    = 80816,
    NPC_BOMBSQUAD        = 80875,

    BOSS_OSHIR           = 79852,
    NPC_RAVENOUS_WOLF    = 89012,
    NPC_RYLAK_SKYTERROR  = 89011,

    BOSS_SKULLOC         = 83612,
    BOSS_KORAMAR         = 83613,
    BOSS_ZOGGOSH         = 83616
};

enum IronDocksGameObjects
{
    // sealed while Oshir fights
    GO_OSHIR_ENCOUNTER_GATE = 233817
};

enum IronDocksActions
{
    ACTION_NOKGAR_DISMOUNT   = 1,
    ACTION_NOKGAR_RIDERLESS,
    ACTION_KORAMAR_BLADESTORM,
    ACTION_ZOGGOSH_LEAVE_TURRET
};

enum NokgarSpells
{
    SPELL_NOKGAR_BURNING_ARROWS       = 164234,
    SPELL_NOKGAR_BARBED_ARROW_BARRAGE = 166914,
    SPELL_NOKGAR_RECKLESS_PROVOCATION = 164426
};

enum DreadfangSpells
{
    SPELL_DREADFANG_SAVAGE_MAULING    = 164837,
    SPELL_DREADFANG_SHREDDING_SWIPES  = 164730,
    SPELL_DREADFANG_BLOODLETTING_HOWL = 164835
};

enum EnforcerSpells
{
    SPELL_MAKOGG_FLAMING_SLASH   = 163665,
    SPELL_MAKOGG_LAVA_SWEEP      = 164956,

    SPELL_NEESA_GUTSHOT          = 163334,
    SPELL_NEESA_OGRE_TRAPS       = 163390,
    SPELL_NEESA_JUMPER_CABLES    = 163376,

    SPELL_DUGRU_BLOOD_BOLT       = 165122,
    SPELL_DUGRU_TAINTED_BLOOD    = 163740,
    SPELL_DUGRU_SANGUINE_SPHERE  = 163689
};

enum OshirSpells
{
    SPELL_OSHIR_PRIMAL_ASSAULT  = 161256,
    SPELL_OSHIR_RENDING_SLASHES = 161239,
    SPELL_OSHIR_TIME_TO_FEED    = 161530
};

enum SkullocSpells
{
    SPELL_SKULLOC_GRONN_SMASH      = 168227,
    SPELL_SKULLOC_CANNON_BARRAGE   = 168929,

    SPELL_KORAMAR_SHATTERING_BLADE = 169073,
    SPELL_KORAMAR_BERSERKER_LEAP   = 168970,
    SPELL_KORAMAR_BLADESTORM       = 168402,

    SPELL_ZOGGOSH_RAPID_FIRE       = 168398
};
