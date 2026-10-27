/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const VenomousAbyssEncounterCount = 8;

enum VenomousAbyssData
{
    DATA_SSZORAK                = 0,    // 3420
    // Ithraz and Vexhul
    DATA_TWIN_FANGS             = 1,    // 3421
    // Zul'jan and Hex Lord Malacrass
    DATA_COILED_ALTAR           = 2,    // 3429
    // Blood of Ula'tek and Breath of Ula'tek
    DATA_ENTOMBED_SENTINELS     = 3,    // 3445
    DATA_VASHNIK                = 4,    // 3455
    DATA_NEKZALI                = 5,    // 3470
    DATA_ULATEK                 = 6,    // 3492
    // Mor'zahi and the possessed tortollans
    DATA_LOST_EXPLORERS         = 7     // 3497
};

enum VenomousAbyssCreatures
{
    // Sszorak
    BOSS_SSZORAK                            = 257347,

    // The Twin Fangs
    BOSS_ITHRAZ                             = 257368,
    BOSS_VEXHUL                             = 257361,
    NPC_SPAWN_OF_VEXHUL                     = 264023,
    NPC_BROODLING_OF_ITHRAZ                 = 270898,
    NPC_BARBED_BULWARK                      = 269194,

    // The Coiled Altar, further templates share the boss models
    BOSS_ZULJAN                             = 257911,
    BOSS_ZULJAN_ALT                         = 271406,
    BOSS_ZULJAN_ALT2                        = 272706,
    BOSS_HEX_LORD_MALACRASS                 = 259854,
    BOSS_HEX_LORD_MALACRASS_ALT             = 270405,
    NPC_COALESCED_VENOM                     = 264005,
    NPC_MANIFESTATION_OF_DREAD              = 261218,
    NPC_SPITEFUL_SOULCOILER                 = 261521,
    // not in the 12.1.0 creature templates yet: 0 keeps it unused
    NPC_FRAGMENT_OF_MALACRASS_PLACEHOLDER   = 0,

    // Entombed Sentinels
    BOSS_BLOOD_OF_ULATEK                    = 258558,
    BOSS_BREATH_OF_ULATEK                   = 258557,

    // Vashnik the Malignant
    BOSS_VASHNIK                            = 265948,
    BOSS_VASHNIK_ALT                        = 266403,
    NPC_CLOTTING_VENOM                      = 259408,
    NPC_SHROUDED_VENOM                      = 260895,
    NPC_BURNING_VENOM                       = 260905,

    // Nek'zali the Soulcoiler
    BOSS_NEKZALI                            = 259927,
    NPC_RESTLESS_AMANI                      = 261509,
    NPC_LATENT_CULTIST                      = 261908,
    NPC_ECHO_OF_JAWAE                       = 263050,
    NPC_ECHO_OF_JAWAE_ALT                   = 263685,
    NPC_ECHO_OF_JAWAE_ALT2                  = 271337,

    // Ula'tek
    BOSS_ULATEK                             = 257758,
    BOSS_ULATEK_ALT                         = 268029,
    BOSS_ULATEK_ALT2                        = 268863,
    BOSS_ULATEK_ALT3                        = 268956,
    NPC_GORE_RATTLE                         = 259555,
    NPC_BLIGHTSCALE_SPAWN                   = 265644,
    NPC_BLIGHTSCALE_VIPER                   = 261915,
    NPC_BLIGHTSCALE_RAWLING                 = 261874,
    NPC_BLIGHTSCALE_CLUTCH                  = 263535,
    NPC_BLIGHTSCALE_SHRIEKER                = 273577,
    NPC_DOOMSCALE_WARDEN                    = 264045,
    NPC_DOOMSCALE_WARDEN_ALT                = 272947,
    NPC_DOOMSCALE_EGG                       = 268164,
    NPC_RAVENOUS_DOOMSCALE                  = 263900,

    // The Lost Explorers
    BOSS_MORZAHI                            = 261584,
    BOSS_FIRST_MATE_NAMA                    = 262172,
    BOSS_TRADER_GEBBO                       = 262174,
    BOSS_SCROLLSAGE_IKU                     = 262173
};

enum VenomousAbyssSharedSpells
{
    SPELL_ABYSS_BERSERK                     = 26662
};

enum VenomousAbyssSszorakSpells
{
    SPELL_SSZORAK_APEX_PREDATOR             = 1277025,
    SPELL_SSZORAK_RAVAGE                    = 1277002,
    SPELL_SSZORAK_MUTILATE                  = 1277027,
    SPELL_SSZORAK_VENOMOUS_SURGE            = 1305959,
    SPELL_SSZORAK_RAGING_CROSSWINDS         = 1285419,
    SPELL_SSZORAK_CORRODING_VENOM           = 1282869,
    SPELL_SSZORAK_HOWLING_MAELSTROM         = 1285732,
    SPELL_SSZORAK_DIG_IN                    = 1286033
};

enum VenomousAbyssTwinFangsSpells
{
    // Vexhul
    SPELL_VEXHUL_CAUSTIC_DELUGE             = 1289192,
    SPELL_VEXHUL_VENOMOUS_EMERGENCE         = 1291404,
    SPELL_VEXHUL_STIR_THE_DEPTHS            = 1290956,
    SPELL_VEXHUL_VILE_FLOOD                 = 1294293,
    SPELL_VEXHUL_CONCENTRATED_SPITTLE       = 1295107,

    // Ithraz
    SPELL_ITHRAZ_RAVENOUS_FEAST             = 1290516,
    SPELL_ITHRAZ_STONE_BREAKER              = 1288538,
    SPELL_ITHRAZ_COILING_ICHOR              = 1290809,
    SPELL_ITHRAZ_ROUSE_THE_BROOD            = 1308356,
    SPELL_ITHRAZ_BLOOD_TORRENT              = 1303230,

    SPELL_TWINS_UNCOILED_WRATH              = 1308583,
    SPELL_SPAWN_CORROSIVE_SPIT              = 1291478,
    SPELL_BROODLING_VISCERAL_BURST          = 1308385
};

enum VenomousAbyssCoiledAltarSpells
{
    // Zul'jan
    SPELL_ZULJAN_FANGS_OF_THE_COILED_ALTAR  = 1282487,
    SPELL_ZULJAN_TOXIC_DELUGE               = 1299960,
    SPELL_ZULJAN_GUILLOTINE                 = 1283489,
    SPELL_ZULJAN_SEVER                      = 1299680,
    SPELL_ZULJAN_VENOMFANG                  = 1282287,
    SPELL_ZULJAN_AXEGRINDER                 = 1283832,
    SPELL_ZULJAN_SOULBOUND                  = 1309987,
    SPELL_ZULJAN_DEFILEMENT                 = 1298381,
    SPELL_ZULJAN_GRIM_GUILLOTINE            = 1299267,
    SPELL_ZULJAN_BLIGHTED_SEVER             = 1307279,
    SPELL_COALESCED_VENOM_RUPTURE           = 1299838,

    // Hex Lord Malacrass
    SPELL_MALACRASS_DREADMARCH              = 1285643,
    SPELL_MALACRASS_SOUL_SEVER              = 1286620,
    SPELL_MALACRASS_ETERNAL_NIGHTFALL       = 1286918,
    SPELL_MALACRASS_VEIL_OF_TWILIGHT        = 1286912,
    SPELL_MALACRASS_SPIRITCACKLE            = 1286441,
    SPELL_MALACRASS_GLOOMBOMB               = 1286895,
    SPELL_MALACRASS_DREAD_BOLT              = 1307184,
    SPELL_MALACRASS_SOULBINDING             = 1304032,
    SPELL_MALACRASS_GHASTLY_REGENERATION    = 1304033,
    SPELL_DREAD_UNNERVING_FIXATION          = 1285911,
    SPELL_SOULCOILER_WAIL_OF_TERROR         = 1286399
};

enum VenomousAbyssSentinelSpells
{
    // Breath of Ula'tek
    SPELL_BREATH_LIVING_VENOM               = 1284207,
    SPELL_BREATH_VENOM_COAGULATION          = 1284251,
    SPELL_BREATH_TOXIC_DROPLETS             = 1284434,
    SPELL_BREATH_EMPOWERING_SLAM            = 1284458,

    // Blood of Ula'tek
    SPELL_BLOOD_BLOOD_VENOM                 = 1284208,
    SPELL_BLOOD_BLIGHTED_BLOOD              = 1284471,
    SPELL_BLOOD_UNSTABLE_MIASMA             = 1288232,
    SPELL_BLOOD_BLOODVENOM_INJECTION        = 1284487,

    SPELL_SENTINEL_VITRIOLIC_STASIS         = 1284588,
    SPELL_SENTINEL_HELICAL_TOXINS           = 1284590
};

enum VenomousAbyssVashnikSpells
{
    SPELL_VASHNIK_IMBIBE                    = 1283164,
    SPELL_VASHNIK_TOXIC_VAPOR               = 1284561,
    SPELL_VASHNIK_MALIGNANT_BURST           = 1280189,
    SPELL_VASHNIK_BLOOD_INFUSION            = 1293969,
    SPELL_VASHNIK_SHADOW_INFUSION           = 1293968,
    SPELL_VASHNIK_FLAME_INFUSION            = 1293971,
    SPELL_VASHNIK_HEMO_EXPULSION            = 1298582,
    SPELL_VASHNIK_GLOOM_EXPULSION           = 1298583,
    SPELL_VASHNIK_CONFLAGRATING_EXPULSION   = 1298587,
    SPELL_VASHNIK_MALIGNANT_CATALYST        = 1282525,
    SPELL_VASHNIK_PLAGUE_FROTH              = 1281907,
    SPELL_VASHNIK_DRIPPING_FANGS            = 1280935,
    SPELL_VASHNIK_ADAPTIVE_INFECTION        = 1282117,

    SPELL_CLOTTING_SANGUINEOUS_FORTITUDE    = 1291530,
    SPELL_SHROUDED_MIASMIC_COATING          = 1312366,
    SPELL_BURNING_BURNING_PRESENCE          = 1305902
};

enum VenomousAbyssNekzaliSpells
{
    SPELL_NEKZALI_SOULCOIL_RITE             = 1284033,
    SPELL_NEKZALI_RITUAL_BURN               = 1297624,
    SPELL_NEKZALI_UNCOILED_RAGE             = 1284034,
    SPELL_NEKZALI_SOULCOIL_IGNITION         = 1285681,
    SPELL_NEKZALI_ESSENCE_REND              = 1287426,
    SPELL_NEKZALI_POSSESSION_BARRAGE        = 1284103,
    SPELL_NEKZALI_HOLLOWING_STRIKES         = 1284110,
    SPELL_NEKZALI_GRASPING_DEPTHS           = 1293212,
    SPELL_NEKZALI_UNCOILING                 = 1290003,
    SPELL_NEKZALI_INVOKE                    = 1299673,

    SPELL_AMANI_GRAVEBOUND_ADVANCE          = 1287533,
    SPELL_ECHO_SOUL_TRANSFER                = 1292248,
    SPELL_ECHO_TETHER_OF_AWAKENING          = 1289696,
    SPELL_ECHO_HUNGERING_PYRE               = 1289855,
    SPELL_CULTIST_ENTWINED_STEP             = 1293497
};

enum VenomousAbyssUlatekSpells
{
    SPELL_ULATEK_CAUSTIC_WAVES              = 1292403,
    SPELL_ULATEK_CALL_OF_THE_SERPENT        = 1304012,
    SPELL_ULATEK_CALL_OF_THE_SERPENT_RISEN  = 1300751,
    SPELL_ULATEK_MOTHERS_WRATH              = 1298367,
    SPELL_ULATEK_NECROTIC_VAPORS            = 1286834,
    SPELL_ULATEK_RAGE_OF_THE_SHACKLED       = 1286860,
    SPELL_ULATEK_UNCHECKED_RAGE             = 1286945,
    SPELL_ULATEK_VIRULENT_SPIT              = 1302982,
    SPELL_ULATEK_BLIGHTSCALE_CLUTCH         = 1289962,
    SPELL_ULATEK_SPECTRAL_COILS             = 1287265,
    SPELL_ULATEK_CIRCLING_PREY              = 1301510,
    SPELL_ULATEK_SERPENTS_BITE              = 1295905,
    SPELL_ULATEK_FURY_UNLEASHED             = 1286905,

    SPELL_GORE_RATTLE_RATTLER_SLAM          = 1299206,
    SPELL_GORE_RATTLE_MEPHITIC_THRASH       = 1296301,
    SPELL_SPAWN_MALIGNANT_SHELL             = 1295360,
    SPELL_SPAWN_PUTRID_MEMBRANE             = 1301268,
    SPELL_VIPER_ACIDIC_BURST                = 1301800,
    SPELL_VIPER_PETRIFYING_STING            = 1303414,
    SPELL_RAWLING_POISONOUS_BITE            = 1287036,
    SPELL_WARDEN_GRASPING_FANGS             = 1301117,
    SPELL_WARDEN_SHADOW_MOLT                = 1301213,
    SPELL_WARDEN_WRITHING_GESTATION         = 1290990,
    SPELL_DOOMSCALE_SHELL                   = 1300312,
    SPELL_DOOMSCALE_HATCHING_DOOM           = 1306862,
    SPELL_DOOMSCALE_DREAD_ROAR              = 1305775,
    SPELL_SHRIEKER_ACIDIC_EXPULSION         = 1313531,
    SPELL_SHRIEKER_VICIOUS_ECHOES           = 1310764
};

enum VenomousAbyssLostExplorersSpells
{
    // Mor'zahi
    SPELL_MORZAHI_DARK_WHISPERS             = 1295451,
    SPELL_MORZAHI_COMMAND                   = 1297022,
    SPELL_MORZAHI_EVIL_EYES                 = 1292388,
    SPELL_MORZAHI_MALEVOLENT_PRESENCE       = 1295449,
    SPELL_MORZAHI_FINAL_ASCENSION           = 1292780,

    // Trader Gebbo
    SPELL_GEBBO_THROW_JUNK                  = 1291933,
    SPELL_GEBBO_MUSHROOM_TOSS               = 1292104,
    SPELL_GEBBO_EXPLOSIVE_SURPRISE          = 1297625,
    SPELL_GEBBO_SMASHING_SHOVEL             = 1296252,

    // Scrollsage Iku
    SPELL_IKU_FROSTFIRE_VOLLEY              = 1295893,
    SPELL_IKU_BLINK_NOVA                    = 1296025,
    SPELL_IKU_ICEBOUND_FLAMES               = 1286921,
    SPELL_IKU_SHREDDING_SHARDS              = 1295854,
    SPELL_IKU_CATACLYSMIC_INVOCATION        = 1291390,

    // First Mate Nama
    SPELL_NAMA_MIGHTY_THUD                  = 1296092,
    SPELL_NAMA_SHELL_SPIN                   = 1296061,
    SPELL_NAMA_STEADY_STRIKES               = 1291930,
    SPELL_NAMA_RELENTLESS_ESCALATION        = 1296227,
    SPELL_NAMA_UNITED_DEFENSE               = 1297646
};
