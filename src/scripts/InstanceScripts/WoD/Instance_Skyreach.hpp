/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const SkyreachEncounterCount = 4;

enum SkyreachData
{
    DATA_RANJIT   = 0, // 1698
    DATA_ARAKNATH = 1, // 1699
    DATA_RUKHRAN  = 2, // 1700
    DATA_VIRYX    = 3  // 1701
};

enum SkyreachCreatures
{
    BOSS_RANJIT                       = 75964,
    BOSS_ARAKNATH                     = 76141,
    BOSS_RUKHRAN                      = 76143,
    BOSS_HIGH_SAGE_VIRYX              = 76266,

    NPC_SKYREACH_SOLAR_FLARE          = 76227,
    NPC_SKYREACH_PILE_OF_ASH          = 79505,
    NPC_SKYREACH_SOLAR_ZEALOT         = 76267,
    NPC_SKYREACH_SHIELD_CONSTRUCT     = 76292
};

enum SkyreachGameObjects
{
    GO_SKYREACH_RANJIT_ENTRANCE       = 234311,
    GO_SKYREACH_RANJIT_EXIT           = 234310,
    GO_SKYREACH_ARAKNATH_ENTRANCE_1   = 234314,
    GO_SKYREACH_ARAKNATH_ENTRANCE_2   = 234315,
    GO_SKYREACH_ARAKNATH_EXIT_1       = 234312,
    GO_SKYREACH_ARAKNATH_EXIT_2       = 234313,
    GO_SKYREACH_RUKHRAN_ENTRANCE      = 229038,
    GO_SKYREACH_RUKHRAN_EXIT          = 234316,
    GO_SKYREACH_VIRYX_ENTRANCE        = 235994
};

enum RanjitSpells
{
    SPELL_RANJIT_SPINNING_BLADE       = 153544,
    SPELL_RANJIT_WINDWALL             = 153315,
    SPELL_RANJIT_FOUR_WINDS           = 156793,
    SPELL_RANJIT_FAN_OF_BLADES        = 153757,
    SPELL_RANJIT_PIERCING_RUSH        = 165731
};

enum AraknathSpells
{
    SPELL_ARAKNATH_SMASH              = 154110,
    SPELL_ARAKNATH_BURST              = 154135
};

enum RukhranSpells
{
    SPELL_RUKHRAN_PIERCE_ARMOR        = 153794,
    SPELL_RUKHRAN_SCREECH             = 153896,
    SPELL_RUKHRAN_QUILLS              = 159382,
    SPELL_SOLAR_FLARE_FIXATE          = 176544,
    SPELL_SOLAR_FLARE_SUNSTRIKE       = 153828,
    SPELL_SOLAR_FLARE_SUNBURST        = 169810,
    SPELL_PILE_OF_ASH_DORMANT         = 160641
};

enum ViryxSpells
{
    SPELL_VIRYX_SOLAR_BURST           = 154396,
    SPELL_VIRYX_LENS_FLARE            = 154032,
    SPELL_SHIELD_CONSTRUCT_SHIELDING  = 154055
};
