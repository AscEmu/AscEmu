/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const OperationFloodgateEncounterCount = 4;

enum OperationFloodgateData
{
    // Bront and Keeza Quickfuse
    DATA_DEMOLITION_DUO     = 0,    // 3019
    DATA_BIG_MOMMA          = 1,    // 3020
    DATA_SWAMPFACE          = 2,    // 3053
    DATA_GEEZLE_GIGAZAP     = 3     // 3054
};

enum OperationFloodgateCreatures
{
    BOSS_BRONT                      = 226402,
    BOSS_KEEZA_QUICKFUSE            = 226403,
    NPC_KEEZA_TICKING_TIME_BOMB     = 234528,

    BOSS_BIG_MOMMA                  = 226398,
    NPC_MOMMA_DARKFUSE_MECHADRONE   = 228424,

    BOSS_SWAMPFACE                  = 226396,

    BOSS_GEEZLE_GIGAZAP             = 226404,
    NPC_GEEZLE_LEAPING_SPARK        = 232690
};

enum OperationFloodgateGameObjects
{
    // flames sealing the demolition duo's yard
    GO_DUO_FIRE_BARRIER             = 503552
};

enum OperationFloodgateActions
{
    ACTION_DUO_PARTNER_FELL         = 1,
    ACTION_KEEZA_BOMB_TRAMPLED
};

enum DemolitionDuoSpells
{
    SPELL_DUO_DIVIDED_DUO               = 470090,

    SPELL_KEEZA_QUICK_SHOT              = 460602,
    SPELL_KEEZA_BIG_BADA_BOOM           = 460867,
    SPELL_KEEZA_BOMB_DEFLAGRATION       = 460787,
    SPELL_KEEZA_BOMB_SHRAPNEL           = 472755,
    SPELL_KEEZA_BBBFG                   = 1217653,
    SPELL_KEEZA_BBBFG_BLAST             = 1217751,

    SPELL_BRONT_WALLOP                  = 459799,
    SPELL_BRONT_BARRELING_CHARGE        = 459779,
    SPELL_BRONT_BARRELING_CHARGE_HIT    = 460965
};

enum BigMommaSpells
{
    SPELL_MOMMA_JUMPSTART               = 460156,
    SPELL_MOMMA_MOBILIZE_MECHADRONES    = 471585,
    SPELL_MOMMA_KILL_O_BLOCK_BARRIER    = 469981,
    SPELL_MOMMA_ELECTROCRUSH            = 473351,
    SPELL_MOMMA_SONIC_BOOM              = 473220,
    SPELL_MOMMA_SONIC_BOOM_HIT          = 473240,

    SPELL_MECHADRONE_SHOOT              = 460393,
    SPELL_MECHADRONE_DOOM_STORM         = 472452
};

enum SwampfaceSpells
{
    SPELL_SWAMPFACE_SLUDGE_CLAWS        = 469478,
    SPELL_SWAMPFACE_SLUDGE_CLAWS_HIT    = 469479,
    SPELL_SWAMPFACE_SLUDGE_CLAWS_ABSORB = 472878,
    SPELL_SWAMPFACE_MUDSLIDE            = 473112,
    SPELL_SWAMPFACE_MUDSLIDE_HIT        = 473126,
    SPELL_SWAMPFACE_AWAKEN_THE_SWAMP    = 473070
};

enum GeezleSpells
{
    SPELL_GEEZLE_TURBO_CHARGE           = 465456,
    SPELL_GEEZLE_TURBO_CHARGE_BLAST     = 465462,
    SPELL_GEEZLE_TURBO_BOLT             = 465982,
    SPELL_GEEZLE_DAM                    = 468276,
    SPELL_GEEZLE_LEAPING_SPARKS         = 468846,
    SPELL_GEEZLE_SPARK_JOLT             = 468647,
    SPELL_GEEZLE_GIGAZAP                = 468812,
    SPELL_GEEZLE_GIGAZAP_HIT            = 468815,
    SPELL_GEEZLE_THUNDER_PUNCH          = 466197,
    SPELL_GEEZLE_THUNDER_PUNCH_HIT      = 466189,
    SPELL_GEEZLE_THUNDER_PUNCH_KNOCK    = 466190
};
