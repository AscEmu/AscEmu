/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const LiberationOfUndermineEncounterCount = 8;

enum LiberationOfUndermineData
{
    // The Geargrinder, Vexie rides along
    DATA_VEXIE_AND_THE_GEARGRINDERS     = 0,    // 3009
    // Flarendo and Torq
    DATA_CAULDRON_OF_CARNAGE            = 1,    // 3010
    DATA_RIK_REVERB                     = 2,    // 3011
    DATA_STIX_BUNKJUNKER                = 3,    // 3012
    DATA_SPROCKETMONGER_LOCKENSTOCK     = 4,    // 3013
    DATA_ONE_ARMED_BANDIT               = 5,    // 3014
    // Mug'Zee with the heads Mug and Zee
    DATA_MUGZEE                         = 6,    // 3015
    DATA_CHROME_KING_GALLYWIX           = 7     // 3016
};

enum LiberationOfUndermineCreatures
{
    // Vexie and the Geargrinders
    BOSS_THE_GEARGRINDER                = 225821,
    NPC_VEXIE_FULLTHROTTLE              = 225822,
    // rider of the bike vehicle, fights on foot here
    NPC_GEARGRINDER_BIKER               = 228665,
    NPC_GEARGRINDER_BIKER_ALT           = 228037,
    NPC_GEARGRINDER_BIKER_ALT2          = 237599,
    NPC_PIT_MECHANIC                    = 228514,

    // Cauldron of Carnage
    BOSS_FLARENDO                       = 229181,
    BOSS_TORQ                           = 229177,

    // Rik Reverb
    BOSS_RIK_REVERB                     = 228648,
    NPC_AMPLIFIER                       = 230197,
    NPC_AMPLIFIER_ALT                   = 233072,
    NPC_AMPLIFIER_ALT2                  = 236511,

    // Stix Bunkjunker
    BOSS_STIX_BUNKJUNKER                = 230322,
    NPC_SCRAPMASTER                     = 231839,
    NPC_JUNKYARD_HYENA                  = 231935,
    NPC_TERRITORIAL_BOMBSHELL           = 231531,

    // Sprocketmonger Lockenstock
    BOSS_SPROCKETMONGER_LOCKENSTOCK     = 230583,
    NPC_BEAM_TURRET                     = 230358,
    NPC_ROCKET_LAUNCHER                 = 230360,
    NPC_MEGA_MAGNET                     = 236256,
    NPC_VOID_TURRET                     = 231846,
    NPC_VOID_LAUNCHER                   = 231903,

    // The One-Armed Bandit
    BOSS_ONE_ARMED_BANDIT               = 228458,
    NPC_REEL_ASSISTANT                  = 228463,
    NPC_REEL_ASSISTANT_ALT              = 232599,
    NPC_REEL_ASSISTANT_ALT2             = 234211,

    // Mug'Zee, Heads of Security
    BOSS_MUGZEE                         = 229953,
    NPC_MUG                             = 232720,
    NPC_ZEE                             = 232719,
    NPC_GALLAGIO_GOON                   = 233474,
    NPC_VOLUNTEER_ROCKETEER             = 230312,
    NPC_MK_II_ELECTRO_SHOCKER           = 230316,
    // summoned by Unstable Crawler Mines
    NPC_UNSTABLE_CRAWLER_MINE           = 231788,

    // Chrome King Gallywix
    BOSS_CHROME_KING_GALLYWIX           = 231075,
    NPC_DARKFUSE_TECHNICIAN             = 231977,
    NPC_SHARPSHOT_SENTRY                = 231978,
    NPC_DARKFUSE_WRENCHMONGER           = 231939,
    NPC_GIGA_JUICED_TECHNICIAN          = 237192
};

enum LiberationOfUndermineSharedSpells
{
    SPELL_UNDERMINE_BERSERK             = 26662
};

enum GeargrinderSpells
{
    SPELL_GEARGRINDER_PROTECTIVE_PLATING    = 466615,
    SPELL_GEARGRINDER_UNRELENTING_CARNAGE   = 471403,
    SPELL_GEARGRINDER_CALL_BIKERS           = 459943,
    SPELL_GEARGRINDER_SPEW_OIL              = 459666,
    SPELL_GEARGRINDER_INCENDIARY_FIRE       = 468207,
    SPELL_GEARGRINDER_BOMB_VOYAGE           = 459974,
    SPELL_GEARGRINDER_TANK_BUSTER           = 459627,
    SPELL_GEARGRINDER_MECHANICAL_BREAKDOWN  = 460603,
    SPELL_GEARGRINDER_BACKFIRE              = 460386,
    SPELL_GEARGRINDER_BURNING_SHRAPNEL      = 460625,
    SPELL_GEARGRINDER_TUNE_UP               = 460116,

    SPELL_BIKER_HOT_WHEELS                  = 459994,
    SPELL_PIT_MECHANIC_REPAIR               = 460153
};

enum CauldronOfCarnageSpells
{
    SPELL_CAULDRON_COLOSSAL_CLASH           = 465833,
    SPELL_CAULDRON_KING_OF_CARNAGE          = 471557,

    SPELL_FLARENDO_BLISTERING_SPITE         = 472220,
    SPELL_FLARENDO_SCRAPBOMB                = 473650,
    SPELL_FLARENDO_BLASTBURN_ROARCANNON     = 472231,
    SPELL_FLARENDO_ERUPTION_STOMP           = 1214190,
    SPELL_FLARENDO_FIERY_WAVE               = 465446,

    SPELL_TORQ_GALVANIZED_SPITE             = 472223,
    SPELL_TORQ_STATIC_CHARGE                = 473951,
    SPELL_TORQ_THUNDERDRUM_SALVO            = 463840,
    SPELL_TORQ_LIGHTNING_BASH               = 466178,
    SPELL_TORQ_ZAPBOLT                      = 463800
};

enum RikReverbSpells
{
    SPELL_RIK_AMPLIFICATION                 = 473748,
    SPELL_RIK_ECHOING_CHANT                 = 466866,
    SPELL_RIK_SOUND_CANNON                  = 467606,
    SPELL_RIK_FAULTY_ZAP                    = 466961,
    SPELL_RIK_RESONANCE                     = 466128,
    SPELL_RIK_SONIC_BLAST                   = 464488,
    SPELL_RIK_BLOWOUT                       = 466722,
    SPELL_RIK_SOUND_CLOUD                   = 1213817,
    SPELL_RIK_BLARING_DROP                  = 467991,
    SPELL_RIK_HYPE_FEVER                    = 473655,

    SPELL_AMPLIFIER_NOISE_POLLUTION         = 465795,
    SPELL_AMPLIFIER_RESONANT_ECHOES         = 468119,
    SPELL_AMPLIFIER_HAYWIRE                 = 466093
};

enum StixBunkjunkerSpells
{
    SPELL_STIX_ELECTROMAGNETIC_SORTING      = 464399,
    SPELL_STIX_INCINERATOR                  = 464149,
    SPELL_STIX_DEMOLISH                     = 464112,
    SPELL_STIX_MELTDOWN                     = 1217954,
    SPELL_STIX_OVERDRIVE                    = 467117,
    SPELL_STIX_OVERCHARGED_BOLT             = 467149,
    SPELL_STIX_TRASH_COMPACTOR              = 467135,
    SPELL_STIX_MAXIMUM_OUTPUT               = 473227,

    SPELL_SCRAPMASTER_SCRAP_ROCKETS         = 1219384,
    SPELL_SCRAPMASTER_DUMPSTER_DIVE         = 466742,
    SPELL_HYENA_INFECTED_BITE               = 466748,
    SPELL_BOMBSHELL_TERRITORIAL             = 473066,
    SPELL_BOMBSHELL_SHORT_FUSE              = 473115
};

enum LockenstockSpells
{
    SPELL_LOCKENSTOCK_ACTIVATE_INVENTIONS   = 473276,
    SPELL_LOCKENSTOCK_SONIC_BA_BOOM         = 465232,
    SPELL_LOCKENSTOCK_WIRE_TRANSFER         = 466235,
    SPELL_LOCKENSTOCK_BLISTERIZER_MK_II     = 471308,
    SPELL_LOCKENSTOCK_PYRO_PARTY_PACK       = 1214872,
    SPELL_LOCKENSTOCK_GRAVI_GUNK            = 465917,
    SPELL_LOCKENSTOCK_FOOT_BLASTERS         = 1217083,
    SPELL_LOCKENSTOCK_BETA_LAUNCH           = 466765,
    SPELL_LOCKENSTOCK_BLEEDING_EDGE         = 466860,
    SPELL_LOCKENSTOCK_VOIDSPLOSION          = 1218319,
    SPELL_LOCKENSTOCK_UPGRADED_BLOODTECH    = 1218344,

    SPELL_BEAM_TURRET_BLAZING_BEAM          = 1216414,
    SPELL_ROCKET_LAUNCHER_ROCKET_BARRAGE    = 1216525,
    SPELL_MEGA_MAGNET_MEGA_MAGNETIZE        = 1215858,
    SPELL_VOID_TURRET_JUMBO_VOID_BEAM       = 1216674,
    SPELL_VOID_LAUNCHER_VOID_BARRAGE        = 1216699
};

enum OneArmedBanditSpells
{
    SPELL_BANDIT_SPIN_TO_WIN                = 461060,
    SPELL_BANDIT_REWARD_SHOCK_AND_FLAME     = 461083,
    SPELL_BANDIT_REWARD_SHOCK_AND_BOMB      = 461091,
    SPELL_BANDIT_REWARD_FLAME_AND_BOMB      = 461176,
    SPELL_BANDIT_REWARD_FLAME_AND_COIN      = 461389,
    SPELL_BANDIT_REWARD_COIN_AND_SHOCK      = 461101,
    SPELL_BANDIT_REWARD_COIN_AND_BOMB       = 461395,
    SPELL_BANDIT_PAY_LINE                   = 460181,
    SPELL_BANDIT_FOUL_EXHAUST               = 460164,
    SPELL_BANDIT_THE_BIG_HIT                = 460472,
    SPELL_BANDIT_CHEAT_TO_WIN               = 465309,
    SPELL_BANDIT_LINKED_MACHINES            = 465432,
    SPELL_BANDIT_HOT_HOT_HEAT               = 465322,
    SPELL_BANDIT_SCATTERED_PAYOUT           = 465580,
    SPELL_BANDIT_EXPLOSIVE_JACKPOT          = 465587,

    SPELL_REEL_ASSISTANT_OVERLOAD           = 460582,
    SPELL_REEL_ASSISTANT_ELECTRIC_BLAST     = 460847
};

enum MugZeeSpells
{
    SPELL_MUGZEE_HEAD_HONCHO                = 466376,
    SPELL_MUGZEE_MOXIE                      = 466385,
    SPELL_MUGZEE_ELEMENTAL_CARNAGE          = 468658,
    SPELL_MUGZEE_UNCONTROLLED_DESTRUCTION   = 468694,
    SPELL_MUGZEE_STATIC_CHARGE              = 1215953,
    SPELL_MUGZEE_BULLETSTORM                = 471574,
    SPELL_MUGZEE_BLOODLUST                  = 463967,

    // Mug in command
    SPELL_MUG_EARTHSHAKER_GAOL              = 472631,
    SPELL_MUG_FROSTSHATTER_BOOTS            = 466476,
    SPELL_MUG_STORMFURY_FINGER_GUN          = 466509,
    SPELL_MUG_MOLTEN_GOLD_KNUCKLES          = 466518,

    // Zee in command
    SPELL_ZEE_UNSTABLE_CRAWLER_MINES        = 466539,
    SPELL_ZEE_GOBLIN_GUIDED_ROCKET          = 467381,
    SPELL_ZEE_SPRAY_AND_PRAY                = 466545,
    SPELL_ZEE_DOUBLE_WHAMMY_SHOT            = 469490,

    SPELL_GOON_SHAKEDOWN                    = 472659,
    SPELL_ROCKETEER_DISINTEGRATION_BEAM     = 1215488,
    SPELL_ROCKETEER_ROCKET_JUMP             = 1216202,
    SPELL_SHOCKER_SURGING_ARC               = 1214991,
    SPELL_SHOCKER_FAULTY_WIRING             = 1215591
};

enum GallywixSpells
{
    SPELL_GALLYWIX_SCATTERBLAST_CANISTERS   = 466340,
    SPELL_GALLYWIX_FUSED_CANISTERS          = 466341,
    SPELL_GALLYWIX_TICK_TOCK_CANISTERS      = 466342,
    SPELL_GALLYWIX_BIG_BAD_BUNCHA_BOMBS     = 465952,
    SPELL_GALLYWIX_BIGGER_BADDER_BOMB_BLAST = 1214607,
    SPELL_GALLYWIX_SUPPRESSION              = 467182,
    SPELL_GALLYWIX_VENTING_HEAT             = 466751,
    SPELL_GALLYWIX_GATLING_CANNON           = 471225,
    SPELL_GALLYWIX_EGO_CHECK                = 466958,
    SPELL_GALLYWIX_GIGA_COILS               = 469286,
    SPELL_GALLYWIX_GIGA_BLAST               = 469327,
    SPELL_GALLYWIX_CRATERING                = 1214226,
    SPELL_GALLYWIX_ARMAGEDDON_CLASS_PLATING = 1214229,
    SPELL_GALLYWIX_TOTAL_DESTRUCTION        = 1214369,

    SPELL_TECHNICIAN_JUICE_IT               = 471352,
    SPELL_CRONY_PARTY_CRASHING_ROCKET       = 1223126,
    SPELL_SENTRY_SHOCK_BARRAGE              = 466834,
    SPELL_WRENCHMONGER_WRENCH               = 1216845,
    SPELL_WRENCHMONGER_LUMBERING_RAGE       = 1216852,
    SPELL_GIGA_TECHNICIAN_IONIZATION        = 1219039,
    SPELL_GIGA_TECHNICIAN_STATIC_ZAP        = 1219041
};
