/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const NexusPointXenasEncounterCount = 3;

enum NexusPointXenasData
{
    DATA_KASRETH    = 0,  // 3328
    DATA_NYSARRA    = 1,  // 3332
    DATA_LOTHRAXION = 2   // 3333
};

enum NexusPointXenasCreatures
{
    BOSS_KASRETH                = 241539,

    BOSS_NYSARRA                = 241542,
    // second Nysarra template with the same model
    BOSS_NYSARRA_ALT            = 254227,
    NPC_GRAND_NULLIFIER         = 251031,
    NPC_GRAND_NULLIFIER_ALT     = 251853,
    // Lightscar Flare image, 251791 is the Voidspire projection
    NPC_NYSARRA_FRACTURED_IMAGE = 255133,

    BOSS_LOTHRAXION             = 241546,
    NPC_FRACTURED_IMAGE         = 251568,
    NPC_FRACTURED_IMAGE_ALT_1   = 254259
};

enum NexusPointXenasGameObjects
{
    GO_NEXUS_POINT_BOSS_ONE_DOOR = 625725,
    GO_NEXUS_POINT_BOSS_TWO_DOOR = 625714
};

enum NexusPointXenasKasrethSpells
{
    SPELL_KASRETH_ARCANE_ZAP           = 1250553,
    SPELL_KASRETH_REFLUX_CHARGE        = 1251767,
    SPELL_KASRETH_FLUX_COLLAPSE        = 1264040,
    SPELL_KASRETH_CORESPARK_DETONATION = 1257509
};

enum NexusPointXenasNysarraSpells
{
    SPELL_NYSARRA_LIGHTSCARRED   = 1253965,
    SPELL_NYSARRA_UMBRAL_LASH    = 1247937,
    SPELL_NYSARRA_ECLIPSING_STEP = 1249014,
    SPELL_NYSARRA_NULL_VANGUARD  = 1252703,

    SPELL_IMAGE_LIGHTSCAR_FLARE  = 1247976,

    SPELL_NULLIFIER_NULLIFY      = 1282722,
    SPELL_NULLIFIER_DUSK_FRIGHTS = 1282723
};

enum NexusPointXenasLothraxionSpells
{
    SPELL_LOTHRAXION_SEARING_REND         = 1253950,
    SPELL_LOTHRAXION_BRILLIANT_DISPERSION = 1253848,
    SPELL_LOTHRAXION_DIVINE_GUILE         = 1257613,

    SPELL_IMAGE_MIRRORED_REND             = 1266713,
    SPELL_IMAGE_FLICKER                   = 1255531
};
