/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const ManaforgeOmegaEncounterCount = 8;

enum ManaforgeOmegaData
{
    // Adarus Duskblaze, Velaryn Bloodwrath and Ilyssa Darksorrow
    DATA_SOUL_HUNTERS           = 0,    // 3122
    DATA_PLEXUS_SENTINEL        = 1,    // 3129
    DATA_SOULBINDER_NAAZINDHRI  = 2,    // 3130
    DATA_LOOMITHAR              = 3,    // 3131
    DATA_FORGEWEAVER_ARAZ       = 4,    // 3132
    DATA_FRACTILLUS             = 5,    // 3133
    DATA_NEXUS_KING_SALHADAAR   = 6,    // 3134
    DATA_DIMENSIUS              = 7     // 3135
};

enum ManaforgeOmegaCreatures
{
    // The Soul Hunters
    BOSS_ADARUS_DUSKBLAZE                   = 237661,
    BOSS_VELARYN_BLOODWRATH                 = 237660,
    BOSS_ILYSSA_DARKSORROW                  = 237662,

    // Plexus Sentinel
    BOSS_PLEXUS_SENTINEL                    = 233814,
    NPC_PLEXUS_VOLATILE_MANIFESTATION       = 243241,

    // Soulbinder Naazindhri
    BOSS_SOULBINDER_NAAZINDHRI              = 233816,
    NPC_NAAZINDHRI_SHADOWGUARD_MAGE         = 237981,
    NPC_NAAZINDHRI_SHADOWGUARD_ASSASSIN     = 237897,
    NPC_NAAZINDHRI_SHADOWGUARD_PHASEBLADE   = 238463,

    // Loom'ithar
    BOSS_LOOMITHAR                          = 233815,

    // Forgeweaver Araz
    BOSS_FORGEWEAVER_ARAZ                   = 233817,
    NPC_ARAZ_ARCANE_COLLECTOR               = 240905,
    // second collector template of the same encounter
    NPC_ARAZ_ARCANE_COLLECTOR_ALT           = 243256,
    NPC_ARAZ_ARCANE_MANIFESTATION           = 242586,
    NPC_ARAZ_ARCANE_ECHO                    = 241923,
    NPC_ARAZ_SHIELDED_ATTENDANT             = 241832,
    NPC_ARAZ_VOID_MANIFESTATION             = 242589,

    // Fractillus
    BOSS_FRACTILLUS                         = 237861,

    // Nexus-King Salhadaar
    BOSS_NEXUS_KING_SALHADAAR               = 237763,
    NPC_SALHADAAR_ROYAL_VOIDWING            = 233823,
    NPC_SALHADAAR_MANAFORGED_TITAN          = 241800,
    NPC_SALHADAAR_SHADOWGUARD_REAPER        = 241801,
    NPC_SALHADAAR_NEXUS_PRINCE_XEVVOS       = 241798,
    NPC_SALHADAAR_NEXUS_PRINCE_KYVOR        = 241803,

    // Dimensius, the All-Devouring
    BOSS_DIMENSIUS                          = 233824,
    NPC_DIMENSIUS_LIVING_MASS               = 242587,
    NPC_DIMENSIUS_ARTOSHION                 = 245255,
    NPC_DIMENSIUS_PARGOTH                   = 245222,
    NPC_DIMENSIUS_VOIDWARDEN                = 245705,
    NPC_DIMENSIUS_NULLBINDER                = 248589
};

enum ManaforgeOmegaActions
{
    ACTION_ARAZ_EXPOSE_COLLECTOR = 1
};

enum ManaforgeOmegaSharedSpells
{
    SPELL_MANAFORGE_BERSERK                         = 26662
};

enum SoulHuntersSpells
{
    // Adarus Duskblaze
    SPELL_ADARUS_DEVOURERS_IRE                      = 1222232,
    SPELL_ADARUS_CONSUME                            = 1234565,
    SPELL_ADARUS_VOIDSTEP                           = 1227355,
    SPELL_ADARUS_HUNGERING_SLASH                    = 1227685,
    SPELL_ADARUS_COLLAPSING_STAR                    = 1233093,

    // Velaryn Bloodwrath
    SPELL_VELARYN_EYE_BEAM                          = 1218103,
    SPELL_VELARYN_FELBLADE                          = 1225127,
    SPELL_VELARYN_THE_HUNT                          = 1227809,
    SPELL_VELARYN_BLADE_DANCE                       = 1241306,
    SPELL_VELARYN_FEL_INFERNO                       = 1223725,
    SPELL_VELARYN_FEL_RUSH                          = 1233863,

    // Ilyssa Darksorrow
    SPELL_ILYSSA_FRACTURE                           = 1241833,
    SPELL_ILYSSA_SPIRIT_BOMB                        = 1242259,
    SPELL_ILYSSA_SIGIL_OF_CHAINS                    = 1240891,
    SPELL_ILYSSA_IMMOLATION_AURA                    = 1225154,
    SPELL_ILYSSA_INFERNAL_STRIKE                    = 1227113,
    SPELL_ILYSSA_FEL_DEVASTATION                    = 1227117,

    // every hunter while leading an intermission
    SPELL_SOUL_HUNTERS_SOUL_TETHER                  = 1245978
};

enum PlexusSentinelSpells
{
    SPELL_PLEXUS_POWERED_AUTOMATON                  = 1223364,
    SPELL_PLEXUS_ARCANE_LIGHTNING                   = 1227794,
    SPELL_PLEXUS_ERADICATING_SALVO                  = 1219532,
    SPELL_PLEXUS_MANIFEST_MATRICES                  = 1219450,
    SPELL_PLEXUS_OBLITERATION_ARCANOCANNON          = 1219263,
    SPELL_PLEXUS_PROTOCOL_PURGE                     = 1220489,
    SPELL_PLEXUS_ENERGY_CUTTER                      = 1218669,
    SPELL_PLEXUS_PURGING_LIGHTNING                  = 1233110,
    SPELL_PLEXUS_EXPULSION_ZONE                     = 1219471,
    SPELL_PLEXUS_CLEANSE_THE_CHAMBER                = 1234733,

    SPELL_VOLATILE_MANIFESTATION_ENERGY_OVERLOAD    = 1235816
};

enum SoulbinderNaazindhriSpells
{
    SPELL_NAAZINDHRI_SOUL_CALLING                   = 1225582,
    SPELL_NAAZINDHRI_SOULFRAY_ANNIHILATION          = 1227276,
    SPELL_NAAZINDHRI_ARCANE_EXPULSION               = 1223859,
    SPELL_NAAZINDHRI_SOULFIRE_CONVERGENCE           = 1225616,
    SPELL_NAAZINDHRI_SPELLBURN                      = 1240754,
    SPELL_NAAZINDHRI_MYSTIC_LASH                    = 1241100,

    SPELL_SHADOWGUARD_ASSASSIN_VOIDBLADE_AMBUSH     = 1227048,
    SPELL_SHADOWGUARD_MAGE_VOID_BURST               = 1227052,
    SPELL_SHADOWGUARD_PHASEBLADE_VOID_RESONANCE     = 1242018,
    SPELL_SHADOWGUARD_PHASEBLADE_PHASE_BLADES       = 1235576
};

enum LoomitharSpells
{
    SPELL_LOOMITHAR_LAIR_WEAVING                    = 1237272,
    SPELL_LOOMITHAR_INFUSION_TETHER                 = 1226315,
    SPELL_LOOMITHAR_OVERINFUSION_BURST              = 1226395,
    SPELL_LOOMITHAR_PRIMAL_SPELLSTORM               = 1226867,
    SPELL_LOOMITHAR_ARCANE_OVERFLOW                 = 1231408,
    SPELL_LOOMITHAR_PIERCING_STRAND                 = 1227263,
    SPELL_LOOMITHAR_UNBOUND_RAGE                    = 1228059,
    SPELL_LOOMITHAR_ARCANE_OUTRAGE                  = 1227782,
    SPELL_LOOMITHAR_WRITHING_WAVE                   = 1227226
};

enum ForgeweaverArazSpells
{
    SPELL_ARAZ_INVOKE_COLLECTOR                     = 1231720,
    SPELL_ARAZ_ARCANE_OBLITERATION                  = 1228218,
    SPELL_ARAZ_SILENCING_TEMPEST                    = 1228188,
    SPELL_ARAZ_OVERWHELMING_POWER                   = 1228502,
    SPELL_ARAZ_ARCANE_EXPULSION                     = 1227631,
    SPELL_ARAZ_ASTRAL_BURN                          = 1240705,
    SPELL_ARAZ_UNSTABLE_SURGE                       = 1232409,
    SPELL_ARAZ_MANA_SPLINTER                        = 1233415,
    SPELL_ARAZ_FOCUSING_IRIS                        = 1232412,
    SPELL_ARAZ_DARK_SINGULARITY                     = 1233076,
    SPELL_ARAZ_CRUSHING_DARKNESS                    = 1233074,
    SPELL_ARAZ_VOID_HARVEST                         = 1243901,

    SPELL_ARCANE_COLLECTOR_ASTRAL_HARVEST           = 1228214,
    SPELL_ARCANE_COLLECTOR_PRIME_SEQUENCE           = 1237322,
    SPELL_ARCANE_COLLECTOR_ARCANE_SIPHON            = 1228103,
    SPELL_ARCANE_COLLECTOR_ARCANE_BARRIER           = 1231726,
    SPELL_ARCANE_COLLECTOR_PHOTON_BLAST             = 1234328,
    SPELL_ARCANE_COLLECTOR_ARCANE_CONVERGENCE       = 1226260,
    SPELL_ARCANE_MANIFESTATION_ASTRAL_SURGE         = 1231009,
    SPELL_ARCANE_ECHO_ECHOING_INVOCATION            = 1238867,
    SPELL_ARCANE_ECHO_ECHOING_TEMPEST               = 1238874,
    SPELL_SHIELDED_ATTENDANT_HARDENED_SHELL         = 1232738,
    SPELL_SHIELDED_ATTENDANT_RAMPING_POWER          = 1238266,
    SPELL_VOID_MANIFESTATION_VOID_SURGE             = 1243641
};

enum FractillusSpells
{
    SPELL_FRACTILLUS_NETHER_PRISM                   = 1233657,
    SPELL_FRACTILLUS_NEXUS_SHRAPNEL                 = 1232130,
    SPELL_FRACTILLUS_CRYSTALINE_OVERCHARGE          = 1233917,
    SPELL_FRACTILLUS_REFRACTED_ENTROPY              = 1241137,
    SPELL_FRACTILLUS_CRYSTALLINE_SHOCKWAVE          = 1224414,
    SPELL_FRACTILLUS_SHATTERING_BACKHAND            = 1220394,
    SPELL_FRACTILLUS_SHATTERSHELL                   = 1227373,
    SPELL_FRACTILLUS_SHOCKWAVE_SLAM                 = 1231871
};

enum NexusKingSalhadaarSpells
{
    SPELL_SALHADAAR_DECREE_OATH_BOUND               = 1224731,
    SPELL_SALHADAAR_INVOKE_THE_OATH                 = 1224906,
    SPELL_SALHADAAR_SUBJUGATION_RULE                = 1224776,
    SPELL_SALHADAAR_CONQUER                         = 1224787,
    SPELL_SALHADAAR_VANQUISH                        = 1224812,
    SPELL_SALHADAAR_BANISHMENT                      = 1227529,
    SPELL_SALHADAAR_TYRANNY                         = 1224822,
    SPELL_SALHADAAR_COALESCE_VOIDWING               = 1227891,
    SPELL_SALHADAAR_NETHERBREAKER                   = 1228113,
    SPELL_SALHADAAR_RALLY_THE_SHADOWGUARD           = 1228065,
    SPELL_SALHADAAR_SEAL_THE_FORGE                  = 1232327,
    SPELL_SALHADAAR_ROYAL_WARD                      = 1228284,
    SPELL_SALHADAAR_KINGS_HUNGER                    = 1228265,
    SPELL_SALHADAAR_GALACTIC_SMASH                  = 1226648,
    SPELL_SALHADAAR_STARKILLER_SWING                = 1226347,
    SPELL_SALHADAAR_WORLD_IN_TWILIGHT               = 1225634,

    // the Royal Voidwing and its images
    SPELL_ROYAL_VOIDWING_FRACTAL_IMAGES             = 1225099,
    SPELL_ROYAL_VOIDWING_BEHEAD                     = 1224827,
    SPELL_ROYAL_VOIDWING_BESIEGE                    = 1227330,
    SPELL_ROYAL_VOIDWING_DIMENSION_BREATH           = 1228163,
    SPELL_ROYAL_VOIDWING_COSMIC_MAW                 = 1234529,

    SPELL_MANAFORGED_TITAN_SELF_DESTRUCT            = 1230302,
    SPELL_MANAFORGED_TITAN_DREAD_MORTAR             = 1232399,
    SPELL_NEXUS_PRINCE_TWILIGHT_BARRIER             = 1237105,
    SPELL_NEXUS_PRINCE_NEXUS_BEAMS                  = 1228075,
    SPELL_NEXUS_PRINCE_NETHERBLAST                  = 1230261,
    SPELL_SHADOWGUARD_REAPER_TWILIGHT_MASSACRE      = 1237107,
    SPELL_SHADOWGUARD_REAPER_REAP                   = 1228053
};

enum DimensiusSpells
{
    // Critical Mass
    SPELL_DIMENSIUS_FISTS_OF_THE_VOIDLORD           = 1227665,
    SPELL_DIMENSIUS_COSMIC_RADIATION                = 1228367,
    SPELL_DIMENSIUS_MASSIVE_SMASH                   = 1230087,
    SPELL_DIMENSIUS_DEVOUR                          = 1229038,
    SPELL_DIMENSIUS_GROWING_HUNGER                  = 1229674,
    SPELL_DIMENSIUS_DARK_MATTER                     = 1230979,
    SPELL_DIMENSIUS_SHATTERED_SPACE                 = 1243690,
    SPELL_DIMENSIUS_REVERSE_GRAVITY                 = 1243577,

    // Event Horizon
    SPELL_DIMENSIUS_ENDLESS_DARKNESS                = 1241188,
    SPELL_DIMENSIUS_BROKEN_WORLD                    = 1237080,
    SPELL_DIMENSIUS_ASTROPHYSICAL_JET               = 1235490,
    SPELL_DIMENSIUS_BLACK_HOLE                      = 1232987,
    SPELL_DIMENSIUS_STELLAR_CORE                    = 1246930,

    // The Dark Heart
    SPELL_DIMENSIUS_EXTINCTION                      = 1238765,
    SPELL_DIMENSIUS_GAMMA_BURST                     = 1237319,
    SPELL_DIMENSIUS_GRAVITATIONAL_DISTORTION        = 1234242,
    SPELL_DIMENSIUS_ECLIPSE                         = 1237690,

    // Singularity
    SPELL_DIMENSIUS_DESTABILIZED                    = 1245292,
    SPELL_DIMENSIUS_SINGULARITY_RADIATION           = 1232894,
    SPELL_DIMENSIUS_ACCRETION_DISK                  = 1233292,
    SPELL_DIMENSIUS_EXTINGUISH_THE_STARS            = 1231716,
    SPELL_DIMENSIUS_SINGULARITY_DEVOUR              = 1233539,
    SPELL_DIMENSIUS_DARKENED_SKY                    = 1234052,
    SPELL_DIMENSIUS_COSMIC_COLLAPSE                 = 1234263,

    SPELL_LIVING_MASS_FISSION                       = 1231005,
    SPELL_DEVOURED_LORD_CONQUERORS_CROSS            = 1239262,
    SPELL_DEVOURED_LORD_TOUCH_OF_OBLIVION           = 1246143,
    SPELL_ARTOSHION_MASS_EJECTION                   = 1237694,
    SPELL_PARGOTH_STARDUST_NOVA                     = 1237695
};
