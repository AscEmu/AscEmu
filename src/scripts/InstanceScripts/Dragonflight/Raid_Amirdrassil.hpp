/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const AmirdrassilEncounterCount = 9;

enum AmirdrassilData
{
    DATA_FYRAKK             = 0,    // 2677
    DATA_NYMUE              = 1,    // 2708
    DATA_IGIRA              = 2,    // 2709
    // Urctos, Aerwynn and Pip
    DATA_COUNCIL_OF_DREAMS  = 3,    // 2728
    DATA_LARODAR            = 4,    // 2731
    DATA_VOLCOROSS          = 5,    // 2737
    DATA_TINDRAL            = 6,    // 2786
    DATA_GNARLROOT          = 7,    // 2820
    DATA_SMOLDERON          = 8     // 2824
};

enum AmirdrassilCreatures
{
    // Fyrakk the Blazing
    BOSS_FYRAKK                             = 204931,
    NPC_FYRAKK_BURNING_COLOSSUS             = 207796,
    NPC_FYRAKK_SCREAMING_SOUL               = 214608,

    // Nymue, Weaver of the Cycle
    BOSS_NYMUE                              = 206172,
    NPC_NYMUE_CYCLE_WARDEN                  = 209800,

    // Igira the Cruel
    BOSS_IGIRA                              = 200926,

    // Council of Dreams
    BOSS_URCTOS                             = 208363,
    BOSS_AERWYNN                            = 208365,
    BOSS_PIP                                = 208367,

    // Larodar, Keeper of the Flame
    BOSS_LARODAR                            = 208445,
    NPC_LARODAR_FIERY_TREANT                = 208459,
    NPC_LARODAR_SCORCHING_ROOTS             = 208461,
    NPC_LARODAR_CHARRED_TREANT              = 208849,
    NPC_LARODAR_RENEWED_TREANT              = 208850,
    NPC_LARODAR_CHARRED_BRAMBLES            = 209542,
    NPC_LARODAR_RENEWED_BRAMBLE_BARRIER     = 209540,
    NPC_LARODAR_ASHEN_TREANT                = 210512,

    // Volcoross
    BOSS_VOLCOROSS                          = 208478,

    // Tindral Sageswift, Seer of the Flame
    BOSS_TINDRAL                            = 209090,

    // Gnarlroot
    BOSS_GNARLROOT                          = 209333,
    NPC_GNARLROOT_TAINTED_LASHER            = 210231,

    // Smolderon
    BOSS_SMOLDERON                          = 200927,
    NPC_SMOLDERON_LIVING_FLAME              = 200936
};

enum AmirdrassilSharedSpells
{
    SPELL_AMIRDRASSIL_BERSERK               = 26662
};

enum AmirdrassilFyrakkSpells
{
    // Stage One: The Dream Render
    SPELL_FYRAKK_FYRALATHS_BITE             = 417431,
    SPELL_FYRAKK_WILDFIRE                   = 420422,
    SPELL_FYRAKK_FIRESTORM                  = 419506,
    SPELL_FYRAKK_DREAM_REND                 = 417455,
    SPELL_FYRAKK_BLAZE                      = 414186,
    SPELL_FYRAKK_BURNING_PRESENCE           = 417806,
    SPELL_FYRAKK_AFLAME                     = 417807,
    SPELL_FYRAKK_PREDATOR                   = 422137,

    // Intermission: Amirdrassil in Peril
    SPELL_FYRAKK_CORRUPT                    = 419144,
    SPELL_FYRAKK_INCARNATE                  = 412761,
    SPELL_FYRAKK_SHADOWFLAME_ORBS           = 421937,
    SPELL_FYRAKK_BURNING_SCALES             = 422836,

    // Stage Two: Children of the Stars
    SPELL_FYRAKK_GREATER_FIRESTORM          = 422518,
    SPELL_FYRAKK_FLAMEFALL                  = 419123,
    SPELL_FYRAKK_SHADOWFLAME_DEVASTATION    = 422524,

    // Stage Three: Shadowflame Incarnate
    SPELL_FYRAKK_ETERNAL_FIRESTORM          = 422935,
    SPELL_FYRAKK_APOCALYPSE_ROAR            = 422837,
    SPELL_FYRAKK_SHADOWFLAME_BREATH         = 410223,
    SPELL_FYRAKK_INFERNAL_MAW               = 425492,

    // Burning Colossus
    SPELL_COLOSSUS_MOLTEN_GAUNTLET          = 428963,
    SPELL_COLOSSUS_PULSING_CORE             = 422517,
    SPELL_COLOSSUS_EXPLODING_CORE           = 428400,

    // Screaming Soul
    SPELL_SCREAMING_SOUL_SEARING_SCREAMS    = 430051
};

enum AmirdrassilNymueSpells
{
    SPELL_NYMUE_VERDANT_MATRIX              = 418423,
    SPELL_NYMUE_CONTINUUM                   = 420846,
    SPELL_NYMUE_IMPENDING_LOOM              = 429785,
    SPELL_NYMUE_VIRIDIAN_RAIN               = 420907,
    SPELL_NYMUE_WEAVERS_BURDEN              = 427722,
    SPELL_NYMUE_THREADED_BLAST              = 426154,
    SPELL_NYMUE_FULL_BLOOM                  = 426855,
    SPELL_NYMUE_UNRAVEL                     = 421368,

    // Cycle Warden
    SPELL_WARDEN_LIFE_WARD                  = 425794,
    SPELL_WARDEN_LUMBERING_SLAM             = 429108,
    SPELL_WARDEN_RADIAL_FLOURISH            = 425370
};

enum AmirdrassilIgiraSpells
{
    SPELL_IGIRA_VICIOUS_SWING               = 423108,
    SPELL_IGIRA_BLISTERING_SPEAR            = 414425,
    SPELL_IGIRA_TWISTING_BLADE              = 416998,
    SPELL_IGIRA_HARVEST_OF_SCREAMS          = 420251,
    SPELL_IGIRA_MARKED_FOR_TORMENT          = 422776,
    SPELL_IGIRA_GATHERING_TORMENT           = 414367,
    SPELL_IGIRA_SEARING_SPARKS              = 423715,

    // forged torments
    SPELL_IGIRA_UMBRAL_DESTRUCTION          = 416056,
    SPELL_IGIRA_SMASHING_VISCERA            = 418533,
    SPELL_IGIRA_DEVASTATION                 = 424347,
    SPELL_IGIRA_HEART_STOPPER               = 415624
};

enum AmirdrassilCouncilSpells
{
    SPELL_COUNCIL_REBIRTH                   = 418187,

    // Urctos
    SPELL_URCTOS_BLIND_RAGE                 = 420525,
    SPELL_URCTOS_BARRELING_CHARGE           = 420946,
    SPELL_URCTOS_AGONIZING_CLAWS            = 421020,

    // Aerwynn
    SPELL_AERWYNN_CONSTRICTING_THICKET      = 421292,
    SPELL_AERWYNN_NOXIOUS_BLOSSOM           = 420671,
    SPELL_AERWYNN_POISONOUS_JAVELIN         = 420856,

    // Pip
    SPELL_PIP_SONG_OF_THE_DRAGON            = 421029,
    SPELL_PIP_POLYMORPH_BOMB                = 418720,
    SPELL_PIP_EMERALD_WINDS                 = 421024
};

enum AmirdrassilLarodarSpells
{
    // Stage One: The Cycle of Flame
    SPELL_LARODAR_FIERY_FORCE_OF_NATURE     = 417653,
    SPELL_LARODAR_SCORCHING_ROOTS           = 417660,
    SPELL_LARODAR_FURIOUS_CHARGE            = 418637,
    SPELL_LARODAR_FURIOUS_OUTBURST          = 425025,
    SPELL_LARODAR_BLAZING_THORNS            = 426206,
    SPELL_LARODAR_RAGING_INFERNO            = 417634,
    SPELL_LARODAR_COMBUSTING_PRESENCE       = 417583,

    // Intermission: Unreborn Again
    SPELL_LARODAR_CONSUMING_FLAME           = 421316,

    // Stage Two: Avatar of Ash
    SPELL_LARODAR_FALLING_EMBERS            = 427252,
    SPELL_LARODAR_FLASH_FIRE                = 421326,
    SPELL_LARODAR_FIRE_WHIRL                = 427343,
    SPELL_LARODAR_SMOLDERING_BACKDRAFT      = 421318,
    SPELL_LARODAR_ASHEN_CALL                = 422461,
    SPELL_LARODAR_SEARING_ASH               = 421323,

    // treants and brambles
    SPELL_TREANT_BLISTERING_SPLINTERS       = 418522,
    SPELL_TREANT_FIERY_FLOURISH             = 426524,
    SPELL_TREANT_EXPLOSIVE_BARK             = 418535,
    SPELL_TREANT_NATURES_GIFT               = 424997,
    SPELL_ROOTS_SCORCHING_BRAMBLETHORN      = 426387,
    SPELL_BRAMBLE_NATURES_BULWARK           = 419485,
    SPELL_ASHEN_TREANT_BURNING_GROUND       = 417632
};

enum AmirdrassilVolcorossSpells
{
    SPELL_VOLCOROSS_FLOOD_OF_THE_FIRELANDS  = 420934,
    SPELL_VOLCOROSS_SERPENTS_FURY           = 421672,
    SPELL_VOLCOROSS_COILING_FLAMES          = 421284,
    SPELL_VOLCOROSS_SCORCHTAIL_CRASH        = 420421,
    SPELL_VOLCOROSS_VOLCANIC_DISGORGE       = 421616,
    SPELL_VOLCOROSS_CATACLYSM_JAWS          = 423117
};

enum AmirdrassilTindralSpells
{
    // humanoid form
    SPELL_TINDRAL_MASS_ENTANGLEMENT         = 424495,
    SPELL_TINDRAL_FALLING_STAR              = 420236,
    SPELL_TINDRAL_FIERY_GROWTH              = 424581,
    SPELL_TINDRAL_BLAZING_MUSHROOM          = 423260,
    SPELL_TINDRAL_SEARING_WRATH             = 422000,
    SPELL_TINDRAL_FLAME_SURGE               = 427297,

    // Searing Moonkin Form
    SPELL_TINDRAL_SUNFLAME                  = 420240,
    SPELL_TINDRAL_FIRE_BEAM                 = 421398,

    // Flaring Owl Form
    SPELL_TINDRAL_FLARE_BOMB                = 425607,
    SPELL_TINDRAL_OWL_OF_THE_FLAME          = 421603,

    // Burning Treant Form
    SPELL_TINDRAL_SUPPRESSIVE_EMBER         = 424579,
    SPELL_TINDRAL_FLAMING_GERMINATION       = 423265,

    // intermissions and Stage Three
    SPELL_TINDRAL_SUPERNOVA                 = 424140,
    SPELL_TINDRAL_SEER_SUPERNOVA            = 429169
};

enum AmirdrassilGnarlrootSpells
{
    SPELL_GNARLROOT_FLAMING_PESTILENCE      = 421898,
    SPELL_GNARLROOT_CONTROLLED_BURN         = 421971,
    SPELL_GNARLROOT_TORTURED_SCREAM         = 422026,
    SPELL_GNARLROOT_SHADOWFLAME_CLEAVE      = 422039,
    SPELL_GNARLROOT_DREADFIRE_BARRAGE       = 424352,
    SPELL_GNARLROOT_DOOM_CULTIVATION        = 421013,
    SPELL_GNARLROOT_SEARING_BRAMBLE         = 426548,
    SPELL_GNARLROOT_UPROOTED_AGONY          = 421840,
    SPELL_GNARLROOT_RISING_MANIA            = 425709,

    // Tainted Lasher
    SPELL_LASHER_SHADOW_SPINES              = 422053,
    SPELL_LASHER_VICIOUS_THICKET            = 428992
};

enum AmirdrassilSmolderonSpells
{
    // Stage One: The Firelord's Fury
    SPELL_SMOLDERON_BRAND_OF_DAMNATION      = 421343,
    SPELL_SMOLDERON_SEARING_AFTERMATH       = 422577,
    SPELL_SMOLDERON_OVERHEATED              = 421455,
    SPELL_SMOLDERON_LAVA_GEYSERS            = 422691,
    SPELL_SMOLDERON_BLISTERING_HEAT         = 420950,

    // Stage Two: World In Flames
    SPELL_SMOLDERON_DEVOUR_ESSENCE          = 422277,
    SPELL_SMOLDERON_WORLD_IN_FLAMES         = 422172,
    SPELL_SMOLDERON_HEATING_UP              = 423896,
    SPELL_SMOLDERON_BLAZING_SOUL            = 422067,

    // Living Flame
    SPELL_LIVING_FLAME_IGNITED_ESSENCE      = 421858
};
