/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Dragonflayer Forge Master
    CN_DRAGONFLAYER_FORGE_MASTER    = 24079,

    // Dragonflayer HeartSplitter
    CN_DRAGONFLAYER_HEARTSPLITTER   = 24071,

    // Dragonflayer Ironhelm
    CN_DRAGONFLAYER_IRONHELM        = 23961,

    // Dragonflayer Metalworker
    CN_DRAGONFLAYER_METALWORKER     = 24078,

    // Dragonflayer Overseer
    CN_DRAGONFLAYER_OVERSEER        = 24085,

    // Tunneling Ghoul
    CN_TUNNELING_GHOUL              = 24084,

    // Dragonflayer Runecaster
    CN_DRAGONFLAYER_RUNECASTER      = 23960,

    // Dragonflayer Spiritualist
    CN_DRAGONFLAYER_SPIRITUALIST    = 28410,

    // Dragonflayer Strategist
    CN_DRAGONFLAYER_STRATEGIST      = 23956,

    // Proto-Drake Handler
    CN_PROTO_DRAKE_HANDLER          = 24083,

    // Proto-Drake Rider
    CN_PROTO_DRAKE_RIDER            = 24849,

    // Frenzied Geist
    CN_FRENZIED_GEIST               = 28419,

    // Savage Worg
    CN_SAVAGE_WORG                  = 29735,

    // Dragonflayer Bonecrusher
    CN_DRAGONFLAYER_BONECRUSHER     = 24069,

    // Skarvald the Constructor
    CN_SKARVALD_GHOST               = 27390,

    // Dalronn the Controller
    CN_DALRONN_GHOST                = 27389,

    // Prince Keleseth
    CN_PRINCE_KELESETH              = 23953,
    CN_FROST_TOMB                   = 23965,

    CN_SKARVALD                     = 24200,
    CN_DALRONN                      = 24201,

    // Ingvar the Plunderer
    CN_INGVAR                       = 23954,
    CN_INGVAR_UNDEAD                = 23980,
    CN_SHADOW_AXE                   = 23996,

};

enum CreatureSpells
{
    // Dragonflayer Forge Master
    DRAGONFLAYER_FORGE_MASTER_BURNING_BRAND     = 43757,

    // Dragonflayer HeartSplitter
    DRAGONFLAYER_HEARTSPLITTER_PIERCING_JAB     = 31551,
    DRAGONFLAYER_HEARTSPLITTER_THROW            = 43665,
    DRAGONFLAYER_HEARTSPLITTER_WING_CLIP        = 32908,

    // Dragonflayer Ironhelm
    DRAGONFLAYER_IRONHELM_HEROIC_STRIKE         = 29426,
    DRAGONFLAYER_IRONHELM_RINGING_SLAP          = 42780,

    // Dragonflayer Metalworker
    DRAGONFLAYER_METALWORKER_ENRAGE             = 8599,
    DRAGONFLAYER_METALWORKER_SUNDER_ARMOR       = 15572,

    // Dragonflayer Overseer
    DRAGONFLAYER_OVERSEER_BATTLE_SHOUT          = 38232,
    DRAGONFLAYER_OVERSEER_CHARGE                = 35570,
    DRAGONFLAYER_OVERSEER_DEMORALIZING_SHOUT    = 16244,

    // Tunneling Ghoul
    TUNNELING_GHOUL_DECREPIFY                   = 42702,
    TUNNELING_GHOUL_STRIKE                      = 13446,

    // Dragonflayer Runecaster
    DRAGONFLAYER_RUNECASTER_BOLTHORNS_RUNE_OF_FLAME     = 54965,
    DRAGONFLAYER_RUNECASTER_NJORDS_RUNE_OF_PROTECTION   = 42740,

    // Dragonflayer Spiritualist
    DRAGONFLAYER_SPIRITUALIST_FLAME_SHOCK       = 51588,
    DRAGONFLAYER_SPIRITUALIST_HEALING_WAVE      = 51586,
    DRAGONFLAYER_SPIRITUALIST_LIGHTNING_BOLT    = 51587,

    // Dragonflayer Strategist
    DRAGONFLAYER_STRATEGIST_BLIND               = 42972,
    DRAGONFLAYER_STRATEGIST_HURL_DAGGER         = 42772,
    DRAGONFLAYER_STRATEGIST_TICKING_BOMB        = 54962,

    // Proto-Drake Handler
    PROTO_DRAKE_HANDLER_DEBILITATING_STRIKE     = 38621,
    PROTO_DRAKE_HANDLER_THROW                   = 54983,
    PROTO_DRAKE_HANDLER_UNHOLY_RAGE             = 43664,

    // Proto-Drake Rider
    PROTO_DRAKE_RIDER_PIERCING_JAB              = 31551,
    PROTO_DRAKE_RIDER_THROW                     = 43665,
    PROTO_DRAKE_RIDER_WING_CLIP                 = 32908,

    // Frenzied Geist
    FRENZIED_GEIST_FIXATE                       = 40414,

    // Savage Worg
    SAVAGE_WORG_ENRAGE                          = 42745,
    SAVAGE_WORG_POUNCE                          = 55077,

    // Dragonflayer Bonecrusher
    DRAGONFLAYER_BONECRUSHER_HEAD_CRACK         = 9791,
    DRAGONFLAYER_BONECRUSHER_KNOCKDOWNSPIN      = 43935,

    // Skarvald the Constructor
    SKARVALD_CHARGE                             = 43651,
    STONE_STRIKE                                = 48583,

    // Dalronn the Controller
    DEBILITATE                                  = 43650,
    SHADOW_BOLT                                 = 43649,
    SHADOW_BOLT_HC                              = 59575,
    SKELETON_ADD                                = 28878,

    // Prince Keleseth
    KELESETH_SHADOW_BOLT                        = 43667,
    KELESETH_SHADOW_BOLT_HC                     = 59389,
    FROST_TOMB_SPELL                            = 48400,
    KELESETH_SKELETON_ADD                       = 23970,
    DECREPIFY                                   = 42702,
    DECREPIFY_HC                                = 59397,

    // Ingvar the Plunderer
    SHADOW_AXE_SPELL                            = 42751,
        // Phase 1 spells (Human Form)
        INGVAR_CLEAVE           = 42724,
        INGVAR_ENRAGE           = 59707,
        INGVAR_SMASH            = 42669,
        INGVAR_ROAR             = 42708,
        INGVAR_ENRAGE_HC        = 42705,
        INGVAR_SMASH_HC         = 59706,
        INGVAR_ROAR_HC          = 59708,

        // Phase 2 spells (Undead Form)
        INGVAR_DARK_SMASH       = 42723,
        INGVAR_DREADFUL_ROAR    = 42729,
        INGVAR_WOE_STRIKE       = 42730,
        INGVAR_DREADFUL_ROAR_HC = 59734,
        INGVAR_WOE_STRIKE_HC    = 59735
};

enum CreatureSay
{

};

enum GameObjectEntry
{
    // ZADNJI_DOORS_MOZDA = 186612,
    DALRONN_DOORS       = 186608,

    INGVAR_DOORS_1      = 186756,
    INGVAR_DOORS_2      = 186694,

    BELLOW_1            = 186688,
    BELLOW_2            = 186689,
    BELLOW_3            = 186690,

    FORGEFIRE_1         = 186692,
    FORGEFIRE_2         = 186693,
    FORGEFIRE_3         = 186691,

    GLOWING_ANVIL_1     = 186609,
    GLOWING_ANVIL_2     = 186610,
    GLOWING_ANVIL_3     = 186611
};
