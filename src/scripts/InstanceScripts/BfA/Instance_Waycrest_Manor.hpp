/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const WaycrestManorEncounterCount = 5;

enum WaycrestManorData
{
    // Sister Solena, Sister Malady and Sister Briar
    DATA_HEARTSBANE_TRIAD       = 0,  // 2113
    DATA_SOULBOUND_GOLIATH      = 1,  // 2114
    DATA_RAAL_THE_GLUTTONOUS    = 2,  // 2115
    // Lord Waycrest and Lady Waycrest
    DATA_LORD_AND_LADY_WAYCREST = 3,  // 2116
    DATA_GORAK_TUL              = 4   // 2117
};

enum WaycrestManorCreatures
{
    BOSS_SISTER_MALADY          = 131823,
    BOSS_SISTER_SOLENA          = 131824,
    BOSS_SISTER_BRIAR           = 131825,

    BOSS_SOULBOUND_GOLIATH      = 131667,
    NPC_SOUL_THORNS             = 136330,

    BOSS_RAAL_THE_GLUTTONOUS    = 131863,
    NPC_WASTING_SERVANT         = 133361,
    NPC_BILE_OOZELING           = 136541,

    BOSS_LORD_WAYCREST          = 131527,
    BOSS_LADY_WAYCREST          = 131545,

    BOSS_GORAK_TUL              = 131864,
    NPC_DEATHTOUCHED_SLAVER     = 135552,
    NPC_DRUST_SUMMONING_PORTAL  = 133435
};

enum WaycrestManorGameObjects
{
    GO_HEARTSBANE_TRIAD_DOOR    = 282410,
    GO_WAYCREST_ORGAN_BARRIER   = 297809
};

enum WaycrestManorActions
{
    ACTION_TRIAD_CLAIM_IRIS     = 1,
    ACTION_LADY_VITALITY_TRANSFER,
    ACTION_LADY_TRANSFER_AND_DESCEND,
    ACTION_LORD_REVITALIZED
};

enum WaycrestManorTriadSpells
{
    SPELL_BRIAR_BRAMBLE_BOLT            = 260701,
    SPELL_BRIAR_BRAMBLE_BOLT_FOCUSED    = 260697,
    SPELL_BRIAR_JAGGED_NETTLES          = 260741,
    SPELL_BRIAR_IRONBARK_SHIELD         = 261265,

    SPELL_MALADY_RUINOUS_BOLT           = 260700,
    SPELL_MALADY_RUINOUS_BOLT_FOCUSED   = 260696,
    SPELL_MALADY_UNSTABLE_RUNIC_MARK    = 260703,
    SPELL_MALADY_RUNIC_WARD             = 261266,

    SPELL_SOLENA_SOUL_BOLT              = 260699,
    SPELL_SOLENA_SOUL_BOLT_FOCUSED      = 260698,
    SPELL_SOLENA_SOUL_ARMOR             = 261264,

    SPELL_TRIAD_CLAIM_THE_IRIS          = 260852,
    SPELL_TRIAD_FOCUSING_IRIS           = 260805,
    SPELL_TRIAD_DIRE_RITUAL             = 260773
};

enum WaycrestManorGoliathSpells
{
    SPELL_GOLIATH_CRUSH                 = 260508,
    SPELL_GOLIATH_SOUL_THORNS           = 267907
};

enum WaycrestManorRaalSpells
{
    SPELL_RAAL_TENDERIZE                = 264923,
    SPELL_RAAL_ROTTEN_EXPULSION         = 264694,
    SPELL_RAAL_CALL_SERVANT             = 264931,
    SPELL_RAAL_CONSUME_ALL              = 264734,
    SPELL_RAAL_CONSUME_SERVANT          = 265002,
    SPELL_RAAL_CONSUMED_SERVANT         = 265005
};

enum WaycrestManorWaycrestSpells
{
    SPELL_LORD_WASTING_STRIKE           = 261438,
    SPELL_LORD_VIRULENT_PATHOGEN        = 261440,
    SPELL_LORD_PUTRID_VITALITY          = 261447,

    SPELL_LADY_SOUL_ARMOR               = 271590,
    SPELL_LADY_WRACKING_CHORD           = 268271,
    SPELL_LADY_DISCORDANT_CADENZA       = 268308,
    SPELL_LADY_VITALITY_TRANSFER        = 261446
};

enum WaycrestManorGorakTulSpells
{
    SPELL_GORAK_DARKENED_LIGHTNING      = 266225,
    SPELL_GORAK_SUMMON_SLAVER           = 266266,
    SPELL_GORAK_DREAD_ESSENCE           = 266181,

    SPELL_SLAVER_DARK_LEAP              = 273657,
    SPELL_SLAVER_DREAD_BOLT             = 266460,
    SPELL_SLAVER_DEATH_LENS             = 268202
};
