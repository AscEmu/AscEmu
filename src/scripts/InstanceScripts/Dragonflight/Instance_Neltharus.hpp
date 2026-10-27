/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const NeltharusEncounterCount = 4;

enum NeltharusData
{
    DATA_MAGMATUSK         = 0, // 2610
    DATA_WARLORD_SARGHA    = 1, // 2611
    DATA_FORGEMASTER_GOREK = 2, // 2612
    DATA_CHARGATH          = 3  // 2613
};

enum NeltharusCreatures
{
    BOSS_MAGMATUSK         = 181861,

    BOSS_WARLORD_SARGHA    = 189901,
    NPC_RAGING_EMBER       = 192464,

    BOSS_FORGEMASTER_GOREK = 189478,

    BOSS_CHARGATH          = 189340
};

enum NeltharusMagmatuskSpells
{
    SPELL_MAGMATUSK_VOLATILE_MUTATION   = 374365,
    SPELL_MAGMATUSK_MAGMA_TENTACLE      = 374410,
    SPELL_MAGMATUSK_MAGMA_LOB           = 375071,
    SPELL_MAGMATUSK_LAVA_SPRAY          = 375251,
    SPELL_MAGMATUSK_BLAZING_CHARGE      = 375449,
    SPELL_MAGMATUSK_LAVA_WAVE           = 375535
};

enum NeltharusWarlordSarghaSpells
{
    SPELL_SARGHA_MAGMA_SHIELD           = 376780,
    SPELL_SARGHA_FLAME_ERUPTION         = 376783,
    SPELL_SARGHA_FLAME_ERUPTION_NORMAL  = 454837,
    SPELL_SARGHA_THE_DRAGONS_KILN       = 377204,
    SPELL_SARGHA_MOLTEN_GOLD            = 377018,

    SPELL_RAGING_EMBER_BURNING_PURSUIT  = 377522
};

enum NeltharusForgemasterGorekSpells
{
    SPELL_GOREK_MIGHT_OF_THE_FORGE      = 374635,
    SPELL_GOREK_BLAZING_HAMMER          = 374640,
    SPELL_GOREK_BLAZING_AEGIS           = 374812,
    SPELL_GOREK_FORGESTORM              = 377995,
    SPELL_GOREK_HEATED_SWINGS           = 374535
};

enum NeltharusChargathSpells
{
    SPELL_CHARGATH_GROUNDING_SPEAR      = 373424,
    SPELL_CHARGATH_FIERY_FOCUS          = 375056,
    SPELL_CHARGATH_DRAGON_STRIKE        = 373733,
    SPELL_CHARGATH_MAGMA_WAVE           = 373767
};
