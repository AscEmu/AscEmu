/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Gossip/GossipScript.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

uint32_t const EncounterCount           = 6;

enum Encounters
{
    DATA_NORTHREND_BEASTS               = 0,
    DATA_JARAXXUS                       = 1,
    DATA_FACTION_CRUSADERS              = 2,
    DATA_TWIN_VALKIRIES                 = 3,
    DATA_LICH_KING                      = 4,
    DATA_ANUBARAK                       = 5,
};

enum NorthrendBeasts
{
    GORMOK_IN_PROGRESS                  = 1000,
    GORMOK_DONE                         = 1001,
    SNAKES_IN_PROGRESS                  = 2000,
    DREADSCALE_SUBMERGED                = 2001,
    ACIDMAW_SUBMERGED                   = 2002,
    SNAKES_SPECIAL                      = 2003,
    SNAKES_DONE                         = 2004,
    ICEHOWL_IN_PROGRESS                 = 3000,
    ICEHOWL_DONE                        = 3001
};

enum Data
{
    // Additional Data
    DATA_GORMOK_THE_IMPALER             = 0,
    DATA_ACIDMAW                        = 1,
    DATA_DREADSCALE                     = 2,
    DATA_ICEHOWL                        = 3,
    DATA_FJOLA_LIGHTBANE                = 4,
    DATA_EYDIS_DARKBANE                 = 5,
    DATA_FORDRING                       = 6,
    DATA_FORDRING_ANUBARAK              = 7,
    DATA_VARIAN                         = 8,
    DATA_GARROSH                        = 9,
    DATA_FIZZLEBANG                     = 10,
    DATA_FACTION_CHAMPIONS              = 11,

    DATA_CRUSADERS_CHEST                = 12,
    DATA_COLISEUM_FLOOR                 = 13,
    DATA_MAIN_GATE                      = 14,
    DATA_EAST_PORTCULLIS                = 15,
    DATA_WEB_DOOR                       = 16,
    DATA_TRIBUTE_CHEST                  = 17,
    DATA_BEASTS_COMBAT_STALKER          = 18,
    DATA_FURIOUS_CHARGE                 = 19,
    DATA_DESPAWN_SNOBOLDS               = 20,
    DATA_LICH_KING_VOICE                = 21
};

enum MiscData
{
    TYPE_COUNTER                        = 100,
    TYPE_NORTHREND_BEASTS               ,
    DATA_SNOBOLD_COUNT                  ,
    DATA_MISTRESS_OF_PAIN_COUNT         
};

enum Spells
{
    SPELL_WILFRED_PORTAL                = 68424,
    SPELL_OPEN_PORTAL                   = 67864,
    SPELL_JARAXXUS_CHAINS               = 67924,
    SPELL_DESTROY_FLOOR_KNOCKUP         = 68193,
    SPELL_ARTHAS_PORTAL                 = 51807,
    SPELL_LK_FROST_NOVA                 = 68198,
    SPELL_CORPSE_TELEPORT               = 69016
};

enum Misc
{
    DESPAWN_TIME                        = 1200000,
    PLAYER_VEHICLE_ID                   = 444
};

enum Actions
{
    ACTION_START_GORMOK                 = 1,
    ACTION_START_GORMOK_FAIL,
    ACTION_START_JORMUNGARS,
    ACTION_START_ICEHOWL,
    ACTION_NORTHREND_BEASTS_WIPE,
    ACTION_NORTHREND_BEASTS_DEFEATED,
    ACTION_START_JARAXXUS_EVENT,
    ACTION_KILL_JARAXXUS,
    ACTION_JARAXXUS_DEFEATED,
    ACTION_START_CHAMPIONS,
    ACTION_SUMMON_CHAMPIONS,
    ACTION_TIRION_ALLOW,
    ACTION_CHAMPIONS_DEFEATED,
    ACTION_SUMMON_JARAXXUS,
    ACTION_JARAXXUS_INTRO,
    ACTION_START_VALKYR,
    ACTION_START_LK_EVENT,
    ACTION_SAY_KILLED_PLAYER,
    ACTION_VALKYR_DEFEATED,
    ACTION_LK_EVENT_FINISHED,
    ACTION_JARAXXUS_ENGAGE,
    ACTION_START_CHAMPIONS_ENGAGE,
    ACTION_START_VALKYR_ENGAGE,
    ACTION_JARAXXUS_WIPE,
    ACTION_FACTION_WIPE,
    ACTION_VALKYR_WIPE,

    ACTION_OPEN_GATE,
    ACTION_CLOSE_GATE,

    ACTION_ENCOUNTER_STARTED,
    ACTION_ENCOUNTER_STOPPED
};

enum WorldStates
{
    UPDATE_STATE_UI_SHOW                = 4390,
    UPDATE_STATE_UI_COUNT               = 4389
};

enum Creatures
{
    NPC_BARRETT_BEASTS                  = 34816,
    NPC_BARRETT_BEASTS_HC               = 35909,
    NPC_BARRETT_JARAXXUS                = 35035,
    NPC_BARRETT_FACTION                 = 35766,
    NPC_BARRETT_VALKYR                  = 35770,
    NPC_BARRETT_LK                      = 35771,
    NPC_TIRION_FORDRING                 = 34996,
    NPC_TIRION_FORDRING_ANUBARAK        = 36095,
    NPC_ARGENT_MAGE                     = 36097,
    NPC_FIZZLEBANG                      = 35458,
    NPC_GARROSH                         = 34995,
    NPC_VARIAN                          = 34990,
    NPC_LICH_KING                       = 35877,

    NPC_THRALL                          = 34994,
    NPC_PROUDMOORE                      = 34992,
    NPC_WILFRED_PORTAL                  = 17965,
    NPC_PURPLE_GROUND                   = 35651,

    NPC_ICEHOWL                         = 34797,
    NPC_GORMOK                          = 34796,
    NPC_DREADSCALE                      = 34799,
    NPC_ACIDMAW                         = 35144,
    NPC_BEASTS_COMBAT_STALKER           = 36549,
    NPC_FURIOUS_CHARGE_STALKER          = 35062,
    NPC_SNOBOLD_VASSAL                  = 34800,
    NPC_FIREBOMB                        = 34854,
    NPC_SLIME_POOL                      = 35176,

    NPC_JARAXXUS                        = 34780,

    NPC_CHAMPIONS_CONTROLLER            = 34781,
    NPC_ALLIANCE_DEATH_KNIGHT           = 34461,
    NPC_ALLIANCE_DRUID_BALANCE          = 34460,
    NPC_ALLIANCE_DRUID_RESTORATION      = 34469,
    NPC_ALLIANCE_HUNTER                 = 34467,
    NPC_ALLIANCE_MAGE                   = 34468,
    NPC_ALLIANCE_PALADIN_HOLY           = 34465,
    NPC_ALLIANCE_PALADIN_RETRIBUTION    = 34471,
    NPC_ALLIANCE_PRIEST_DISCIPLINE      = 34466,
    NPC_ALLIANCE_PRIEST_SHADOW          = 34473,
    NPC_ALLIANCE_ROGUE                  = 34472,
    NPC_ALLIANCE_SHAMAN_ENHANCEMENT     = 34463,
    NPC_ALLIANCE_SHAMAN_RESTORATION     = 34470,
    NPC_ALLIANCE_WARLOCK                = 34474,
    NPC_ALLIANCE_WARRIOR                = 34475,

    NPC_HORDE_DEATH_KNIGHT              = 34458,
    NPC_HORDE_DRUID_BALANCE             = 34451,
    NPC_HORDE_DRUID_RESTORATION         = 34459,
    NPC_HORDE_HUNTER                    = 34448,
    NPC_HORDE_MAGE                      = 34449,
    NPC_HORDE_PALADIN_HOLY              = 34445,
    NPC_HORDE_PALADIN_RETRIBUTION       = 34456,
    NPC_HORDE_PRIEST_DISCIPLINE         = 34447,
    NPC_HORDE_PRIEST_SHADOW             = 34441,
    NPC_HORDE_ROGUE                     = 34454,
    NPC_HORDE_SHAMAN_ENHANCEMENT        = 34455,
    NPC_HORDE_SHAMAN_RESTORATION        = 34444,
    NPC_HORDE_WARLOCK                   = 34450,
    NPC_HORDE_WARRIOR                   = 34453,

    NPC_PET_WARLOCK                     = 35465,
    NPC_PET_HUNTER                      = 35610,

    NPC_FJOLA_LIGHTBANE                 = 34497,
    NPC_EYDIS_DARKBANE                  = 34496,

    NPC_DARK_ESSENCE                    = 34567,
    NPC_LIGHT_ESSENCE                   = 34568,
    NPC_UNLEASHED_DARK                  = 34628,
    NPC_UNLEASHED_LIGHT                 = 34630,
    NPC_BULLET_CONTROLLER               = 34743,

    NPC_VALKYR_STALKER_DARK             = 34704,    // Stalker NPC for Movement
    NPC_VALKYR_STALKER_LIGHT            = 34720,    // Stalker NPC for Movement

    NPC_LICH_KING_VOICE                 = 16980,
    NPC_ARTHAS_PORTAL                   = 22517,

    NPC_ANUBARAK                        = 34564
};

static uint32_t NPC_BARRET[] =
{
    NPC_BARRETT_BEASTS,
    NPC_BARRETT_BEASTS_HC,
    NPC_BARRETT_JARAXXUS,
    NPC_BARRETT_FACTION,
    NPC_BARRETT_VALKYR,
    NPC_BARRETT_LK
};

enum GameObjects
{
    GO_CRUSADERS_CACHE_10               = 195631,
    GO_CRUSADERS_CACHE_25               = 195632,
    GO_CRUSADERS_CACHE_10_H             = 195633,
    GO_CRUSADERS_CACHE_25_H             = 195635,

    // Tribute Chest (heroic)
    // 10-man modes
    GO_TRIBUTE_CHEST_10H_25             = 195668, // 10man 01-24 attempts
    GO_TRIBUTE_CHEST_10H_45             = 195667, // 10man 25-44 attempts
    GO_TRIBUTE_CHEST_10H_50             = 195666, // 10man 45-49 attempts
    GO_TRIBUTE_CHEST_10H_99             = 195665, // 10man 50 attempts
    // 25-man modes
    GO_TRIBUTE_CHEST_25H_25             = 195672, // 25man 01-24 attempts
    GO_TRIBUTE_CHEST_25H_45             = 195671, // 25man 25-44 attempts
    GO_TRIBUTE_CHEST_25H_50             = 195670, // 25man 45-49 attempts
    GO_TRIBUTE_CHEST_25H_99             = 195669, // 25man 50 attempts

    GO_ARGENT_COLISEUM_FLOOR            = 195527,
    GO_MAIN_GATE_DOOR                   = 195647,
    GO_EAST_PORTCULLIS                  = 195648,
    GO_WEB_DOOR                         = 195485,
    GO_PORTAL_TO_DALARAN                = 195682
};

enum AchievementData
{
    // Northrend Beasts
    UPPER_BACK_PAIN_10_PLAYER           = 11779,
    UPPER_BACK_PAIN_10_PLAYER_HEROIC    = 11802,
    UPPER_BACK_PAIN_25_PLAYER           = 11780,
    UPPER_BACK_PAIN_25_PLAYER_HEROIC    = 11801,
    // Lord Jaraxxus
    THREE_SIXTY_PAIN_SPIKE_10_PLAYER    = 11838,
    THREE_SIXTY_PAIN_SPIKE_10_PLAYER_HEROIC = 11861,
    THREE_SIXTY_PAIN_SPIKE_25_PLAYER    = 11839,
    THREE_SIXTY_PAIN_SPIKE_25_PLAYER_HEROIC = 11862,
    // Tribute
    A_TRIBUTE_TO_SKILL_10_PLAYER        = 12344,
    A_TRIBUTE_TO_SKILL_25_PLAYER        = 12338,
    A_TRIBUTE_TO_MAD_SKILL_10_PLAYER    = 12347,
    A_TRIBUTE_TO_MAD_SKILL_25_PLAYER    = 12341,
    A_TRIBUTE_TO_INSANITY_10_PLAYER     = 12349,
    A_TRIBUTE_TO_INSANITY_25_PLAYER     = 12343,
    A_TRIBUTE_TO_IMMORTALITY_HORDE      = 12358,
    A_TRIBUTE_TO_IMMORTALITY_ALLIANCE   = 12359,
    A_TRIBUTE_TO_DEDICATED_INSANITY     = 12360,
    REALM_FIRST_GRAND_CRUSADER          = 12350,

    // Dummy spells - not existing in dbc but we don't need that
    SPELL_WORMS_KILLED_IN_10_SECONDS    = 68523,
    SPELL_CHAMPIONS_KILLED_IN_MINUTE    = 68620,
    SPELL_DEFEAT_FACTION_CHAMPIONS      = 68184,
    SPELL_TRAITOR_KING                  = 68186,

    // Timed events
    EVENT_START_TWINS_FIGHT             = 21853
};

enum TextIds
{
    // Highlord Tirion Fordring
    TIRION_SAY_WELCOME                  = 0,
    TIRION_SAY_GORMOK                   = 1,
    TIRION_SAY_JORMUNGARS               = 2,
    TIRION_SAY_ICEHOWL                  = 3,
    TIRION_SAY_BEASTS_DONE              = 4,
    TIRION_SAY_BEASTS_WIPE              = 5,
    TIRION_SAY_WILFRED                  = 6,
    TIRION_SAY_KILL_JARAXXUS            = 7,
    TIRION_SAY_LAMENT                   = 8,
    TIRION_SAY_CALM_DOWN                = 9,
    TIRION_SAY_CHAMPIONS                = 10,
    TIRION_SAY_ALLOW_COMBAT             = 11,
    TIRION_SAY_TRAGIC_VICTORY           = 12,
    TIRION_SAY_WORK_TOGETHER            = 13,
    TIRION_SAY_GAME_BEGIN               = 14,
    TIRION_SAY_UNITED                   = 15,
    TIRION_SAY_ARTHAS                   = 16,

    // Varian Wrynn
    VARIAN_SAY_BEASTS                   = 9,
    VARIAN_SAY_COME_PIGS                = 8,
    VARIAN_SAY_DEMAND_JUSTICE           = 7,
    VARIAN_SAY_FIGHT_GLORY              = 6,
    VARIAN_SAY_FACTION_DEAD             = 4,
    VARIAN_SAY_VALKYR_DEAD              = 5,
    VARIAN_SAY_KILLED_1                 = 3,
    VARIAN_SAY_KILLED_2                 = 2,
    VARIAN_SAY_KILLED_3                 = 1,
    VARIAN_SAY_KILLED_4                 = 0,

    // Garrosh
    GARROSH_SAY_BEASTS                  = 3,
    GARROSH_SAY_ALLIANCE_DOGS           = 2,
    GARROSH_SAY_DEMAND_JUSTICE          = 1,
    GARROSH_SAY_NO_MERCY                = 0,
    GARROSH_SAY_FACTION_DEAD            = 4,
    GARROSH_SAY_VALKYR_DEAD             = 11,
    GARROSH_SAY_KILLED_1                = 6,
    GARROSH_SAY_KILLED_2                = 7,
    GARROSH_SAY_KILLED_3                = 8,
    GARROSH_SAY_KILLED_4                = 9,

    // Wilfred Fizzlebang
    WILFRED_SAY_INTRO                   = 0,
    WILFRED_SAY_OBLIVION                = 1,
    WILFRED_SAY_MASTER                  = 2,
    WILFRED_SAY_DEAD                    = 3,

    // The Lich King Voice
    LK_VOICE_SAY_CHALLENGE              = 4,
    LK_VOICE_SAY_SOULS_WILL_BE_MINE     = 5,

    //  The Lich King
    LK_SAY_EMPIRE                       = 2,

    // Highlord Tirion Fordring (Anu'barak)
    SAY_STAGE_4_06                      = 0,
    SAY_STAGE_4_07                      = 1
};

enum TrialMisc
{
    SPLINE_INITIAL_MOVEMENT             = 1,
    POINT_SUMMON                        = 1,
    POINT_MIDDLE                        = 2,
    GROUP_VALKYR                        = 1,
    GOSSIPID_FAIL                       = 1,
    POINT_BARRETT_DESPAWN               = 1,
    AREA_TRIAL_OF_THE_CRUSADER          = 4722
};

LocationVector const BarretSpawnPosition        = { 559.1528f, 90.55729f, 395.2734f, 5.078908f };
LocationVector const WilfredSpawnPosition       = { 563.6007f, 208.5278f, 395.2696f, 4.729842f };
LocationVector const JaraxxusSpawnPosition      = { 563.8264f, 140.6563f, 393.9861f, 4.694936f };
LocationVector const PortalTargetSpawnPosition  = { 563.6597f, 139.7569f, 399.2507f, 4.712389f };
LocationVector const PurpleGroundSpawnPosition  = { 563.6858f, 139.4323f, 393.9862f, 4.694936f };
LocationVector const ArthasPortalSpawnPosition  = { 563.6996f, 175.9826f, 394.5042f, 4.694936f };
LocationVector const LichKingSpawnPosition      = { 563.5712f, 174.8351f, 394.4954f, 4.712389f };
LocationVector const CorpseTeleportPosition     = { 631.9390f, 136.5040f, 142.5540f, 0.803332f };

LocationVector const NorthrendBeastsSpawnPositions[] =
{
    { 563.9358f, 229.8299f, 394.8061f, 4.694936f },         // Gormok \ Icehowl
    { 564.2802f, 233.1322f, 394.7897f, 1.621917f },         // Dreadscale
};

LocationVector const TwinValkyrsLoc[] =
{
    { 586.060242f, 117.514809f, 394.41f, 0 },               // 0 - Dark essence 1
    { 541.602112f, 161.879837f, 394.41f, 0 },               // 1 - Dark essence 2
    { 541.021118f, 117.262932f, 394.41f, 0 },               // 2 - Light essence 1
    { 586.200562f, 162.145523f, 394.41f, 0 }                // 3 - Light essence 2
};

LocationVector const AnubarakLoc[] =
{
    { 783.9305f, 132.9722f, 142.6711f, 3.141593f },         // 0 - Anub'arak Spawn Location (sniffed)
    { 695.240051f, 137.834824f, 142.200000f, 0 },           // 1 - Anub'arak move point location
    { 694.886353f, 102.484665f, 142.119614f, 0 },           // 3 - Nerub Spawn
    { 694.500671f, 185.363968f, 142.117905f, 0 },           // 5 - Nerub Spawn
    { 731.987244f, 83.3824690f, 142.119614f, 0 },           // 2 - Nerub Spawn
    { 740.184509f, 193.443390f, 142.117584f, 0 }            // 4 - Nerub Spawn
};

LocationVector const EndSpawnLoc[] =
{
    { 648.9167f, 131.0208f, 141.6161f, 0.f },               // 0 - Highlord Tirion Fordring
    { 649.1614f, 142.0399f, 141.3057f, 0.f },               // 1 - Argent Mage
    { 644.6250f, 149.2743f, 140.6015f, 5.f }                // 2 - Portal to Dalaran
};

LocationVector const ToCSpawnLoc[] =
{
    { 563.912f, 261.625f, 394.73f, 4.70437f },              //  0 Center
    { 575.451f, 261.496f, 394.73f,  4.6541f },              //  1 Left
    { 549.951f,  261.55f, 394.73f, 4.74835f }               //  2 Right
};

LocationVector const ToCCommonLoc[] =
{
    { 559.257996f, 90.266197f, 395.122986f, 0.0f },         //  0 Barrent
    { 563.672974f, 139.57100f, 393.837006f, 0.0f },         //  1 Center
    { 563.833008f, 187.244995f, 394.50000f, 0.0f },         //  2 Backdoor
    { 577.347839f, 195.338888f, 395.14000f, 0.0f },         //  3 - Right
    { 550.955933f, 195.338888f, 395.14000f, 0.0f },         //  4 - Left
    { 563.833008f, 195.244995f, 394.585561f, 0.0f },        //  5 - Center
    { 573.500000f, 180.500000f, 395.14f, 0.0f },            //  6 Move 0 Right
    { 553.5f, 180.5f, 400.5521f, 0 },                       //  7 Move 0 Left
    { 573.0f, 170.0f, 400.5521f, 0 },                       //  8 Move 1 Right
    { 549.5139f, 170.1389f, 394.7965f, 5.009095f },         //  9 Move 1 Left
    { 563.8f, 216.1f, 395.1f, 0 },                          // 10 Behind the door

    { 575.042358f, 195.260727f, 395.137146f, 0 },           // 5
    { 552.248901f, 195.331955f, 395.132658f, 0 },           // 6
    { 573.342285f, 195.515823f, 395.135956f, 0 },           // 7
    { 554.239929f, 195.825577f, 395.137909f, 0 },           // 8
    { 571.042358f, 195.260727f, 395.137146f, 0 },           // 9
    { 556.720581f, 195.015472f, 395.132658f, 0 },           // 10
    { 569.534119f, 195.214478f, 395.139526f, 0 },           // 11
    { 569.231201f, 195.941071f, 395.139526f, 0 },           // 12
    { 558.811610f, 195.985779f, 394.671661f, 0 },           // 13
    { 567.641724f, 195.351501f, 394.659943f, 0 },           // 14
    { 560.633972f, 195.391708f, 395.137543f, 0 },           // 15
    { 565.816956f, 195.477921f, 395.136810f, 0 }            // 16
};

ObjectData const creatureData[] =
{
    { NPC_GORMOK,                   DATA_GORMOK_THE_IMPALER    },
    { NPC_ACIDMAW,                  DATA_ACIDMAW               },
    { NPC_DREADSCALE,               DATA_DREADSCALE            },
    { NPC_ICEHOWL,                  DATA_ICEHOWL               },
    { NPC_BEASTS_COMBAT_STALKER,    DATA_BEASTS_COMBAT_STALKER },
    { NPC_FURIOUS_CHARGE_STALKER,   DATA_FURIOUS_CHARGE        },
    { NPC_JARAXXUS,                 DATA_JARAXXUS              },
    { NPC_CHAMPIONS_CONTROLLER,     DATA_FACTION_CRUSADERS     },
    { NPC_FJOLA_LIGHTBANE,          DATA_FJOLA_LIGHTBANE       },
    { NPC_EYDIS_DARKBANE,           DATA_EYDIS_DARKBANE        },
    { NPC_LICH_KING,                DATA_LICH_KING             },
    { NPC_ANUBARAK,                 DATA_ANUBARAK              },
    { NPC_TIRION_FORDRING,          DATA_FORDRING              },
    { NPC_TIRION_FORDRING_ANUBARAK, DATA_FORDRING_ANUBARAK     },
    { NPC_VARIAN,                   DATA_VARIAN                },
    { NPC_GARROSH,                  DATA_GARROSH               },
    { NPC_FIZZLEBANG,               DATA_FIZZLEBANG            },
    { NPC_LICH_KING_VOICE,          DATA_LICH_KING_VOICE       },
    { 0,                            0                          } // END
};

ObjectData const gameObjectData[] =
{
    { GO_CRUSADERS_CACHE_10,    DATA_CRUSADERS_CHEST },
    { GO_CRUSADERS_CACHE_25,    DATA_CRUSADERS_CHEST },
    { GO_CRUSADERS_CACHE_10_H,  DATA_CRUSADERS_CHEST },
    { GO_CRUSADERS_CACHE_25_H,  DATA_CRUSADERS_CHEST },
    { GO_ARGENT_COLISEUM_FLOOR, DATA_COLISEUM_FLOOR  },
    { GO_MAIN_GATE_DOOR,        DATA_MAIN_GATE       },
    { GO_EAST_PORTCULLIS,       DATA_EAST_PORTCULLIS },
    { GO_WEB_DOOR,              DATA_WEB_DOOR        },
    { GO_TRIBUTE_CHEST_10H_25,  DATA_TRIBUTE_CHEST   },
    { GO_TRIBUTE_CHEST_10H_45,  DATA_TRIBUTE_CHEST   },
    { GO_TRIBUTE_CHEST_10H_50,  DATA_TRIBUTE_CHEST   },
    { GO_TRIBUTE_CHEST_10H_99,  DATA_TRIBUTE_CHEST   },
    { GO_TRIBUTE_CHEST_25H_25,  DATA_TRIBUTE_CHEST   },
    { GO_TRIBUTE_CHEST_25H_45,  DATA_TRIBUTE_CHEST   },
    { GO_TRIBUTE_CHEST_25H_50,  DATA_TRIBUTE_CHEST   },
    { GO_TRIBUTE_CHEST_25H_99,  DATA_TRIBUTE_CHEST   },
    { 0,                        0                    } // END
};

enum GossipMenuIds
{
    MENUID_NORTHREND_BEASTS         = 616,
    MENUID_NORTHREND_BEASTS_FAIL    = 615,
    MENUID_JARAXXUS                 = 617,
    MENUID_JARAXXUS_FAIL            = 618,
    MENUID_FACTION_CHAMPIONS        = 619,
    MENUID_FACTION_CHAMPIONS_FAIL   = 620,
    MENUID_VALKYR                   = 621,
    MENUID_VALKYR_FAIL              = 622,
    MENUID_LK                       = 623
};

//////////////////////////////////////////////////////////////////////////////////////////
//Trial Of The Crusader Instance
class TrialOfTheCrusaderInstanceScript : public InstanceScript
{
public:
    explicit TrialOfTheCrusaderInstanceScript(WorldMap* pMapMgr);
    static InstanceScript* Create(WorldMap* pMapMgr);

    void UpdateEvent() override;

    void OnAreaTrigger(Player* pPlayer, uint32_t pAreaId) override;

    void OnPlayerEnter(Player* player) override;
    void OnPlayerDeath(Player* /*pVictim*/, Unit* /*pKiller*/) override;

    void readSaveDataExtended(std::istringstream& /*data*/) override;
    void writeSaveDataExtended(std::ostringstream& /*data*/) override;
    bool setBossState(uint32_t /*id*/, EncounterStates /*state*/) override;

    void setLocalData(uint32_t /*type*/, uint32_t /*data*/) override;
    uint32_t getLocalData(uint32_t /*type*/) const override;

    void OnCreaturePushToWorld(Creature* pCreature) override;
    void OnGameObjectPushToWorld(GameObject* pGameObject) override;

    void OnCreatureDeath(Creature* /*pVictim*/, Unit* /*pKiller*/) override;

    void DoAction(int32_t action) override;

    void getStalkersGuidVector(std::vector<uint32_t>& vVector) const { vVector = stalkerGUIDS; }

protected:
    uint32_t NorthrendBeasts;
    uint32_t TrialCounter;

    std::vector<uint32_t> snoboldGUIDS;
    std::vector<uint32_t> stalkerGUIDS;

    // Achievement stuff
    uint32_t NotOneButTwoJormungarsTimer;
    uint32_t ResilienceWillFixItTimer;
    uint8_t SnoboldCount;
    uint8_t MistressOfPainCount;
    bool CrusadersSpecialState;
    bool TributeToImmortalityEligible;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Gossip: Barret Ramsey
class BarretGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override;
    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override;

    void encounterStarted(uint32_t action, Creature* fordring, Creature* self);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Barret Ramsey
class BarretAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit BarretAI(Creature* pCreature);
    
    void OnReachWP(uint32_t type, uint32_t id) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Tirion Fordring
class TirionAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit TirionAI(Creature* pCreature);

    void OnLoad() override;

    void DoAction(int32_t action) override;

    void onSummonedCreature(Creature* summon) override;

    // Events
    void handleBarrettSummon(CreatureAIFunc pThis);

    void GormokIntro(CreatureAIFunc pThis);
    void GormokSpawn(CreatureAIFunc pThis);

    void JormungarsIntro(CreatureAIFunc pThis);
    void JormungarsSpawn(CreatureAIFunc pThis);

    void IcehowlIntro(CreatureAIFunc pThis);
    void IcehowlSpawn(CreatureAIFunc pThis);

    void JaraxxusIntro(CreatureAIFunc pThis);
    void JaraxxusSpawn(CreatureAIFunc pThis);
    void Lamet(CreatureAIFunc pThis);

    void summonChampions(CreatureAIFunc pThis);
    void ChampionsStart(CreatureAIFunc pThis);

    void valkyrSummon(CreatureAIFunc pThis);

    void lickkingChallenge(CreatureAIFunc pThis);
    void lichkingSouls(CreatureAIFunc pThis);
    void summonLickKing(CreatureAIFunc pThis);

protected:
    TrialOfTheCrusaderInstanceScript* mInstance;
    uint32_t mFactionLeaderData;
    bool mJormungarsSummoned;
    bool mIcehowlSummoned;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Garrosh
class GaroshAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit GaroshAI(Creature* pCreature);

    void DoAction(int32_t action) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Varian
class VarianAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit VarianAI(Creature* pCreature);

    void DoAction(int32_t action) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// LichKing
class LichKingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit LichKingAI(Creature* pCreature);

    void InitOrReset() override;
    void OnReachWP(uint32_t type, uint32_t id) override;

protected:
    void startMove(CreatureAIFunc pThis);
    void startTalk(CreatureAIFunc pThis);
    void changeWeather(CreatureAIFunc pThis);
    void breakPlatform(CreatureAIFunc pThis);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Arthas Portal
class ArthasPortalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ArthasPortalAI(Creature* pCreature);
};
