/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheUnderrotEncounterCount = 4;

enum TheUnderrotData
{
    DATA_ELDER_LEAXA            = 0,  // 2111
    DATA_SPORECALLER_ZANCHA     = 1,  // 2112
    DATA_CRAGMAW_THE_INFESTED   = 2,  // 2118
    DATA_UNBOUND_ABOMINATION    = 3   // 2123
};

enum TheUnderrotCreatures
{
    BOSS_ELDER_LEAXA            = 131318,
    NPC_BLOOD_EFFIGY            = 134701,
    NPC_BLOOD_WAVE_STALKER      = 132398,

    BOSS_SPORECALLER_ZANCHA     = 131383,

    BOSS_CRAGMAW_THE_INFESTED   = 131817,
    NPC_BLOOD_TICK_LARVA        = 132080,
    NPC_BLOOD_TICK              = 132051,

    BOSS_UNBOUND_ABOMINATION    = 133007,
    NPC_BLOOD_VISAGE            = 137103,
    NPC_TITAN_KEEPER_HEZREL     = 134419
};

enum TheUnderrotGameObjects
{
    // shortcut wall behind Sporecaller Zancha
    GO_UNDERROT_SHORTCUT_WALL   = 295356,
    // way down to the Unbound Abomination
    GO_UNDERROT_PYRAMID_DOOR    = 296385
};

enum TheUnderrotLeaxaSpells
{
    SPELL_LEAXA_BLOOD_BOLT              = 260879,
    SPELL_LEAXA_SANGUINE_FEAST          = 264757,
    SPELL_LEAXA_CREEPING_ROT            = 260889,
    SPELL_LEAXA_CREEPING_ROT_WAVE       = 261496,
    SPELL_LEAXA_BLOOD_MIRROR            = 264603
};

enum TheUnderrotZanchaSpells
{
    SPELL_ZANCHA_FESTERING_HARVEST_BAR  = 259888,
    SPELL_ZANCHA_FESTERING_HARVEST      = 259732,
    SPELL_ZANCHA_BOUNDLESS_ROT          = 259830,
    SPELL_ZANCHA_SHOCKWAVE              = 272457,
    SPELL_ZANCHA_UPHEAVAL_MARK          = 259718,
    SPELL_ZANCHA_UPHEAVAL_BLAST         = 259720
};

enum TheUnderrotCragmawSpells
{
    SPELL_CRAGMAW_CHARGE                = 260292,
    SPELL_CRAGMAW_INDIGESTION           = 260793,
    SPELL_CRAGMAW_TANTRUM               = 260333,
    SPELL_LARVA_DESTROY_LARVA           = 260418,
    SPELL_BLOOD_TICK_SERRATED_FANGS     = 260455
};

enum TheUnderrotUnboundSpells
{
    SPELL_UNBOUND_PUTRID_BLOOD          = 269301,
    SPELL_UNBOUND_VILE_EXPULSION        = 269843,

    SPELL_HEZREL_HOLY_BOLT              = 269312,
    SPELL_HEZREL_CLEANSING_LIGHT        = 269310,
    SPELL_HEZREL_PURGE_CORRUPTION       = 269406
};
