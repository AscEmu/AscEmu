/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LocationVector.h"
#include "Setup.h"
#include "Management/Gossip/GossipScript.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/SpellScript.hpp"

LocationVector const DefenseSystemLocation = { 1888.146f, 803.382f,  58.60389f, 3.071779f };

LocationVector const CyanigosaSpawnLocation = { 1922.109f, 804.4493f, 52.49254f, 3.176499f };
LocationVector const CyanigosaJumpLocation = { 1888.32f,  804.473f,  38.3578f,  0.0f };

LocationVector const SaboteurSpawnLocation = { 1886.251f, 803.0743f, 38.42326f, 3.211406f };

uint8_t const PortalIntroCount = 3;
uint32_t const PortalPositionsSize = 5;
LocationVector const PortalPositions[PortalPositionsSize] =
{
    { 1877.523f, 850.1788f, 45.36822f, 4.34587f   }, // 0
    { 1890.679f, 753.4202f, 48.771f,   1.675516f  }, // 1
    { 1936.09f,  803.1875f, 54.09715f, 3.054326f  }, // 2
    { 1858.243f, 770.2379f, 40.42146f, 0.9075712f }, // 3
    { 1907.288f, 831.1111f, 40.22015f, 3.560472f  }  // 4
};

uint32_t const PortalElitePositionsSize = 3;
LocationVector const PortalElitePositions[PortalElitePositionsSize] =
{
    { 1911.281f, 800.9722f, 39.91673f, 3.01942f  }, // 5
    { 1926.516f, 763.6616f, 52.35725f, 2.251475f }, // 6
    { 1922.464f, 847.0699f, 48.50161f, 3.961897f }  // 7
};

uint32_t const PortalIntroPositionsSize = 5;
LocationVector const PortalIntroPositions[PortalIntroPositionsSize] =
{
    { 1877.51f,  850.1042f, 44.65989f, 4.782202f }, // 0 - Intro
    { 1890.637f, 753.4705f, 48.72239f, 1.710423f }, // 1 - Intro
    { 1936.073f, 803.1979f, 53.37491f, 3.124139f }, // 2 - Intro
    { 1886.545f, 803.2014f, 40.40931f, 3.159046f }, // 3 - Boss 1/2
    { 1924.096f, 804.3707f, 54.29256f, 3.228859f }  // 4 - Boss 3
};

uint32_t const EncouterPortalsCount = PortalPositionsSize + PortalElitePositionsSize;

LocationVector const SinclariPositions[] =
{
    { 1829.142f, 798.219f,  44.36212f, 0.122173f }, // 0 - Crystal
    { 1820.12f,  803.916f,  44.36466f, 0.0f      }, // 1 - Outside
    { 1816.185f, 804.0629f, 44.44799f, 3.176499f }, // 2 - Second Spawn Point
    { 1827.886f, 804.0555f, 44.36467f, 0.0f      }  // 3 - Outro
};

LocationVector const GuardsMovePosition = { 1802.099f, 803.7724f, 44.36466f, 0.0f };

// 3 Bosses out of Total 9
uint32_t const EncounterCount = 3 + 6;

// Waypoints
LocationVector const FirstPortalWPs[] =
{
    {1877.670288f, 842.280273f, 43.333591f},
    {1877.338867f, 834.615356f, 38.762287f},
    {1872.161011f, 823.854309f, 38.645401f},
    {1864.860474f, 815.787170f, 38.784843f},
    {1858.953735f, 810.048950f, 44.008759f},
    {1843.707153f, 805.807739f, 44.135197f}
    //{1825.736084f, 807.305847f, 44.363785f}
};

LocationVector const SecondPortalFirstWPs[] =
{
    {1902.561401f, 853.334656f, 47.106117f},
    {1895.486084f, 855.376404f, 44.334591f},
    {1882.805176f, 854.993286f, 43.333591f},
    {1877.670288f, 842.280273f, 43.333591f},
    {1877.338867f, 834.615356f, 38.762287f},
    {1872.161011f, 823.854309f, 38.645401f},
    {1864.860474f, 815.787170f, 38.784843f},
    {1858.953735f, 810.048950f, 44.008759f},
    {1843.707153f, 805.807739f, 44.135197f}
    //{1825.736084f, 807.305847f, 44.363785f}
};

LocationVector const SecondPortalSecondWPs[] =
{
    {1929.392212f, 837.614990f, 47.136166f},
    {1928.290649f, 824.750427f, 45.474411f},
    {1915.544922f, 826.919373f, 38.642811f},
    {1900.933960f, 818.855652f, 38.801647f},
    {1886.810547f, 813.536621f, 38.490490f},
    {1869.079712f, 808.701538f, 38.689003f},
    {1860.843384f, 806.645020f, 44.008789f},
    {1843.707153f, 805.807739f, 44.135197f}
    //{1825.736084f, 807.305847f, 44.363785f}
};

LocationVector const ThirdPortalWPs[] =
{
    {1934.049438f, 815.778503f, 52.408699f},
    {1928.290649f, 824.750427f, 45.474411f},
    {1915.544922f, 826.919373f, 38.642811f},
    {1900.933960f, 818.855652f, 38.801647f},
    {1886.810547f, 813.536621f, 38.490490f},
    {1869.079712f, 808.701538f, 38.689003f},
    {1860.843384f, 806.645020f, 44.008789f},
    {1843.707153f, 805.807739f, 44.135197f}
    //{1825.736084f, 807.305847f, 44.363785f}
};

LocationVector const FourthPortalWPs[] =
{
    {1921.658447f, 761.657043f, 50.866741f},
    {1910.559814f, 755.780457f, 47.701447f},
    {1896.664673f, 752.920898f, 47.667004f},
    {1887.398804f, 763.633240f, 47.666851f},
    {1879.020386f, 775.396973f, 38.705990f},
    {1872.439087f, 782.568604f, 38.808292f},
    {1863.573364f, 791.173584f, 38.743660f},
    {1857.811890f, 796.765564f, 43.950329f},
    {1845.577759f, 800.681152f, 44.104248f}
    //{1827.100342f, 801.605957f, 44.363358f}
};

LocationVector const FifthPortalWPs[] =
{
    {1887.398804f, 763.633240f, 47.666851f},
    {1879.020386f, 775.396973f, 38.705990f},
    {1872.439087f, 782.568604f, 38.808292f},
    {1863.573364f, 791.173584f, 38.743660f},
    {1857.811890f, 796.765564f, 43.950329f},
    {1845.577759f, 800.681152f, 44.104248f}
    //{1827.100342f, 801.605957f, 44.363358f}
};

LocationVector const SixthPortalWPs[] =
{
    {1888.861084f, 805.074768f, 38.375790f},
    {1869.793823f, 804.135804f, 38.647018f},
    {1861.541504f, 804.149780f, 43.968292f},
    {1843.567017f, 804.288208f, 44.139091f}
    //{1826.889648f, 803.929993f, 44.363239f}
};

LocationVector const DefaultPortalWPs[] =
{
    { 1843.567017f, 804.288208f, 44.139091f }
};

LocationVector const SaboteurMoraggPath[] =
{
    { 1886.251f, 803.0743f, 38.42326f },
    { 1885.71f,  799.8929f, 38.37241f },
    { 1889.505f, 762.3288f, 47.66684f },
    { 1894.542f, 742.1829f, 47.66684f },
    { 1894.603f, 739.9231f, 47.66684f },
};

LocationVector const SaboteurErekemPath[] =
{
    { 1886.251f, 803.0743f, 38.42326f },
    { 1881.047f, 829.6866f, 38.64856f },
    { 1877.585f, 844.6685f, 38.49014f },
    { 1876.085f, 851.6685f, 42.99014f },
    { 1873.747f, 864.1373f, 43.33349f }
};

LocationVector const SaboteurIchoronPath[] =
{
    { 1886.251f, 803.0743f, 38.42326f },
    { 1888.672f, 801.2348f, 38.42305f },
    { 1901.987f, 793.3254f, 38.65126f }
};

LocationVector const SaboteurLavanthorPath[] =
{
    { 1886.251f, 803.0743f, 38.42326f },
    { 1867.925f, 778.8035f, 38.64702f },
    { 1853.304f, 759.0161f, 38.65761f }
};

LocationVector const SaboteurXevozzPath[] =
{
    { 1886.251f, 803.0743f, 38.42326f },
    { 1889.096f, 810.0487f, 38.43871f },
    { 1896.547f, 823.5473f, 38.72863f },
    { 1906.666f, 842.3111f, 38.63351f }
};

LocationVector const SaboteurZuramatPath[] =
{
    { 1886.251f, 803.0743f, 38.42326f },
    { 1889.69f,  807.0032f, 38.39914f },
    { 1906.91f,  818.2574f, 38.86596f },
    { 1929.03f,  824.2713f, 46.09165f },
    { 1928.441f, 842.8891f, 47.15078f },
    { 1927.454f, 851.6091f, 47.19094f },
    { 1927.947f, 852.2986f, 47.19637f }
};

uint32_t const MoraggPathSize = 3;
LocationVector const MoraggPath[MoraggPathSize] = // sniff
{
    { 1893.895f, 728.1261f, 47.75016f },
    { 1892.997f, 738.4987f, 47.66684f },
    { 1889.76f,  758.1089f, 47.66684f }
};

uint32_t const ErekemPathSize = 3;
LocationVector const ErekemPath[ErekemPathSize] = // sniff
{
    { 1871.456f, 871.0361f, 43.41524f },
    { 1874.948f, 859.5452f, 43.33349f },
    { 1877.245f, 851.967f,  43.3335f  }
};

uint32_t const ErekemGuardLeftPathSize = 3;
LocationVector const ErekemGuardLeftPath[ErekemGuardLeftPathSize] = // sniff
{
    { 1853.752f, 862.4528f, 43.41614f },
    { 1866.931f, 854.577f,  43.3335f  },
    { 1872.973f, 850.7875f, 43.3335f  }
};

uint32_t const ErekemGuardRightPathSize = 3;
LocationVector const ErekemGuardRightPath[ErekemGuardRightPathSize] = // sniff
{
    { 1892.418f, 872.2831f, 43.41563f },
    { 1885.639f, 859.0245f, 43.3335f  },
    { 1882.432f, 852.2423f, 43.3335f  }
};

uint32_t const IchoronPathSize = 5;
LocationVector const IchoronPath[IchoronPathSize] = // sniff
{
    { 1942.041f, 749.5228f, 30.95229f },
    { 1930.571f, 762.9065f, 31.98814f },
    { 1923.657f, 770.6718f, 34.07256f },
    { 1910.631f, 784.4096f, 37.09015f },
    { 1906.595f, 788.3828f, 37.99429f }
};

uint32_t const LavanthorPathSize = 3;
LocationVector const LavanthorPath[LavanthorPathSize] = // sniff
{
    { 1844.557f, 748.7083f, 38.74205f },
    { 1854.618f, 761.5295f, 38.65631f },
    { 1862.17f,  773.2255f, 38.74879f }
};

uint32_t const XevozzPathSize = 3;
LocationVector const XevozzPath[XevozzPathSize] = // sniff
{
    { 1908.417f, 845.8502f, 38.71947f },
    { 1905.557f, 841.3157f, 38.65529f },
    { 1899.453f, 832.533f,  38.70752f }
};

uint32_t const ZuramatPathSize = 3;
LocationVector const ZuramatPath[ZuramatPathSize] = // sniff
{
    { 1934.151f, 860.9463f, 47.29499f },
    { 1927.085f, 852.1342f, 47.19214f },
    { 1923.226f, 847.3297f, 47.15541f }
};

/*
 * Violet hold bosses:
 *
 * 1 - Moragg
 * 2 - Erekem
 * 3 - Ichoron
 * 4 - Lavanthor
 * 5 - Xevozz
 * 6 - Zuramat
 * 7 - Cyanigosa
 */

enum Data : uint8_t
{
    // Main encounters
    DATA_1ST_BOSS                       = 0,
    DATA_2ND_BOSS                       = 1,
    DATA_CYANIGOSA                      = 2,

    // Bosses
    DATA_MORAGG                         = 3,
    DATA_EREKEM                         = 4,
    DATA_ICHORON                        = 5,
    DATA_LAVANTHOR                      = 6,
    DATA_XEVOZZ                         = 7,
    DATA_ZURAMAT                        = 8,

    // Misc
    DATA_MAIN_EVENT_STATE,
    DATA_WAVE_COUNT,
    DATA_DOOR_INTEGRITY,
    DATA_PORTAL_LOCATION,
    DATA_START_BOSS_ENCOUNTER,
    DATA_DEFENSELESS,

    // Bosses
    DATA_EREKEM_GUARD_1,
    DATA_EREKEM_GUARD_2,

    // Cells
    DATA_MORAGG_CELL,
    DATA_EREKEM_CELL,
    DATA_EREKEM_LEFT_GUARD_CELL,
    DATA_EREKEM_RIGHT_GUARD_CELL,
    DATA_ICHORON_CELL,
    DATA_LAVANTHOR_CELL,
    DATA_XEVOZZ_CELL,
    DATA_ZURAMAT_CELL,

    // Misc
    DATA_MAIN_DOOR,
    DATA_SINCLARI,
    DATA_SINCLARI_TRIGGER,
    DATA_PRISON_SEAL,
    DATA_HANDLE_CELLS
};

enum CreatureEntry
{
    //Main event
    NPC_SINCLARI                        = 30658,
    NPC_SINCLARI_TRIGGER                = 32204,
    NPC_VIOLET_HOLD_GUARD               = 30659,
    NPC_PRISON_SEAL                     = 30896,

    // Portals
    NPC_TELEPORTATION_PORTAL_INTRO      = 31011,
    NPC_TELEPORTATION_PORTAL            = 30679,
    NPC_TELEPORTATION_PORTAL_ELITE      = 32174,

    // Creatures
    NPC_PORTAL_GUARDIAN                 = 30660,
    NPC_PORTAL_KEEPER                   = 30695,
    NPC_XEVOZZ                          = 29266,
    NPC_LAVANTHOR                       = 29312,
    NPC_ICHORON                         = 29313,
    NPC_ICHOR_GLOBULE                   = 29321,
    NPC_ICHORON_SUMMON_TARGET           = 29326,
    NPC_ZURAMAT                         = 29314,
    NPC_ETHEREALSPHERE                  = 29271,
    NPC_VOID_SENTRY                     = 29364,
    NPC_VOID_SENTRY_BALL                = 29365,
    NPC_EREKEM                          = 29315,
    NPC_EREKEM_GUARD                    = 29395,
    NPC_MORAGG                          = 29316,

    NPC_DUMMY_XEVOZZ                    = 32231,
    NPC_DUMMY_LAVANTHOR                 = 32237,
    NPC_DUMMY_ICHORON                   = 32234,
    NPC_DUMMY_ZURAMAT                   = 32230,
    NPC_DUMMY_EREKEM                    = 32226,
    NPC_DUMMY_EREKEM_GUARD              = 32228,
    NPC_DUMMY_MORAGG                    = 32235,

    NPC_CYANIGOSA                       = 31134,
    NPC_SABOTEOUR                       = 31079,
    NPC_DEFENSE_SYSTEM                  = 30837
};

enum PortalCreatureIds
{
    NPC_AZURE_INVADER_1                 = 30661,
    NPC_AZURE_SPELLBREAKER_1            = 30662,
    NPC_AZURE_BINDER_1                  = 30663,
    NPC_AZURE_MAGE_SLAYER_1             = 30664,
    NPC_VETERAN_MAGE_HUNTER             = 30665,
    NPC_AZURE_CAPTAIN_1                 = 30666,
    NPC_AZURE_SORCEROR_1                = 30667,
    NPC_AZURE_RAIDER_1                  = 30668,

    NPC_AZURE_BINDER_2                  = 30918,
    NPC_AZURE_INVADER_2                 = 30961,
    NPC_AZURE_SPELLBREAKER_2            = 30962,
    NPC_AZURE_MAGE_SLAYER_2             = 30963,
    NPC_AZURE_BINDER_3                  = 31007,
    NPC_AZURE_INVADER_3                 = 31008,
    NPC_AZURE_SPELLBREAKER_3            = 31009,
    NPC_AZURE_MAGE_SLAYER_3             = 31010,
    NPC_AZURE_RAIDER_2                  = 31118,
    NPC_AZURE_STALKER_1                 = 32191
};

enum AzureInvaderSpells
{
    SPELL_CLEAVE                        = 15496,
    SPELL_IMPALE                        = 58459,
    SPELL_BRUTAL_STRIKE                 = 58460,
    SPELL_SUNDER_ARMOR                  = 58461
};

enum AzureSellbreakerSpells
{
    SPELL_ARCANE_BLAST                  = 58462,
    SPELL_SLOW                          = 25603,
    SPELL_CHAINS_OF_ICE                 = 58464,
    SPELL_CONE_OF_COLD                  = 58463,
};

enum AzureBinderSpells
{
    SPELL_ARCANE_BARRAGE                = 58456,
    SPELL_ARCANE_EXPLOSION              = 58455,
    SPELL_FROST_NOVA                    = 58458,
    SPELL_FROSTBOLT                     = 58457,
};

enum AzureMageSlayerSpells
{
    SPELL_ARCANE_EMPOWERMENT            = 58469,
    SPELL_SPELL_LOCK                    = 30849
};

enum AzureCaptainSpells
{
    SPELL_MORTAL_STRIKE                 = 32736,
    SPELL_WHIRLWIND_OF_STEEL            = 41057
};

enum AzureSorcerorSpells
{
    SPELL_ARCANE_STREAM                 = 60181,
    SPELL_MANA_DETONATION               = 60182,
};

enum AzureRaiderSpells
{
    SPELL_CONCUSSION_BLOW               = 52719,
    SPELL_MAGIC_REFLECTION              = 60158
};

enum AzureStalkerSpells
{
    SPELL_BACKSTAB                      = 58471,
    SPELL_TACTICAL_BLINK                = 58470
};

enum AzureSaboteurSpells
{
    SPELL_SHIELD_DISRUPTION             = 58291,
    SPELL_TELEPORT_VISUAL               = 51347
};

enum TrashDoorSpell
{
    SPELL_DESTROY_DOOR_SEAL             = 58040
};

enum DefenseSystemSpells
{
    SPELL_ARCANE_LIGHTNING_DAMAGE       = 57912,
    SPELL_ARCANE_LIGHTNING_INSTAKILL    = 58152,
    SPELL_ARCANE_LIGHTNING_DUMMY        = 57930
};

enum Spells
{
    SPELL_CYANIGOSA_TRANSFORM           = 58668,
    SPELL_CYANIGOSA_ARCANE_POWER_STATE  = 49411,
    SPELL_MORAGG_EMOTE_ROAR             = 48350,
    SPELL_LAVANTHOR_SPECIAL_UNARMED     = 33334,
    SPELL_ZURAMAT_COSMETIC_CHANNEL_OMNI = 57552
};

enum MiscSpells
{
    SPELL_PORTAL_PERIODIC               = 58008,
    SPELL_PORTAL_CHANNEL                = 58012,
    SPELL_CRYSTAL_ACTIVATION            = 57804,

    SPELL_TELEPORT_PLAYER               = 62138,
    SPELL_TELEPORT_PLAYER_EFFECT        = 62139
};

enum GameobjectEntrys
{
    GO_MAIN_DOOR                        = 191723,
    GO_XEVOZZ_DOOR                      = 191556,
    GO_LAVANTHOR_DOOR                   = 191566,
    GO_ICHORON_DOOR                     = 191722,
    GO_ZURAMAT_DOOR                     = 191565,
    GO_EREKEM_DOOR                      = 191564,
    GO_EREKEM_GUARD_1_DOOR              = 191563,
    GO_EREKEM_GUARD_2_DOOR              = 191562,
    GO_MORAGG_DOOR                      = 191606,
    GO_ACTIVATION_CRYSTAL               = 193611,
    GO_INTRO_ACTIVATION_CRYSTAL         = 193615
};

enum Worldstates
{
    WORLD_STATE_VH_SHOW                 = 3816,
    WORLD_STATE_VH_PRISON_STATE         = 3815,
    WORLD_STATE_VH_WAVE_COUNT           = 3810,
};

enum Events
{
    EVENT_NEXT_WAVE                     = 1,
    EVENT_STATE_CHECK                   = 2,
    EVENT_TIMER1                        = 3,
    EVENT_TIMER2                        = 4,
    EVENT_TIMER3                        = 5,
    EVENT_CYANIGOSA_INTRO1              = 6,
    EVENT_CYANIGOSA_INTRO2              = 7,
    EVENT_CYANIGOSA_INTRO3              = 8,
    EVENT_ACTIVATE_CRYSTAL              = 9999
};

enum Misc
{
    ACTION_INTRO_START                  = 0,
    ACTION_SINCLARI_OUTRO               = 1,
    POINT_INTRO                         = 1
};

enum CreatureSay
{
    SAY_SINCLARI_INTRO_1                = 4522,
    SAY_SINCLARI_INTRO_2                = 10760,
    SAY_SINCLARI_OUTRO                  = 10761,

    SAY_CYANIGOSA_SPAWN                 = 5172,
    SAY_XEVOZZ_SPAWN                    = 4546,
    SAY_EREKEM_SPAWN                    = 4529,
    SAY_ICHORON_SPAWN                   = 4537,
    SAY_ZURAMAT_SPAWN                   = 4556,

    SOUND_MORAGG_SPAWN                  = 10112,

    // Sinclari Trigger
    SAY_SINCLARI_ELITE_SQUAD            = 10762,
    SAY_SINCLARI_PORTAL_GUARDIAN        = 10763,
    SAY_SINCLARI_PORTAL_KEEPER          = 10764
};

enum CreatureGossip
{
    //Lieutnant Sinclari
    SINCLARI_ON_HELLO                   = 13853,
    SINCLARI_ON_FINISH                  = 13854,
    SINCLARI_OUTSIDE                    = 14271
};

ObjectData const creatureData[] =
{
    { NPC_XEVOZZ,           DATA_XEVOZZ           },
    { NPC_LAVANTHOR,        DATA_LAVANTHOR        },
    { NPC_ICHORON,          DATA_ICHORON          },
    { NPC_ZURAMAT,          DATA_ZURAMAT          },
    { NPC_EREKEM,           DATA_EREKEM           },
    { NPC_MORAGG,           DATA_MORAGG           },
    { NPC_CYANIGOSA,        DATA_CYANIGOSA        },
    { NPC_SINCLARI,         DATA_SINCLARI         },
    { NPC_SINCLARI_TRIGGER, DATA_SINCLARI_TRIGGER },
    { NPC_PRISON_SEAL,      DATA_PRISON_SEAL      },     
    { 0,                    0                     } // END
};

ObjectData const gameObjectData[] =
{
    { GO_EREKEM_GUARD_1_DOOR, DATA_EREKEM_LEFT_GUARD_CELL  },
    { GO_EREKEM_GUARD_2_DOOR, DATA_EREKEM_RIGHT_GUARD_CELL },
    { GO_EREKEM_DOOR,         DATA_EREKEM_CELL             },
    { GO_ZURAMAT_DOOR,        DATA_ZURAMAT_CELL            },
    { GO_LAVANTHOR_DOOR,      DATA_LAVANTHOR_CELL          },
    { GO_MORAGG_DOOR,         DATA_MORAGG_CELL             },
    { GO_ICHORON_DOOR,        DATA_ICHORON_CELL            },
    { GO_XEVOZZ_DOOR,         DATA_XEVOZZ_CELL             },
    { GO_MAIN_DOOR,           DATA_MAIN_DOOR               },
    { 0,                      0                            } // END
};


class TheVioletHoldScript : public InstanceScript
{
public:
    explicit TheVioletHoldScript(WorldMap* pMapMgr);
    static InstanceScript* Create(WorldMap* pMapMgr);

    void OnCreaturePushToWorld(Creature* pCreature) override;
    void OnGameObjectPushToWorld(GameObject* pGameObject) override;

    // Save FirstBoss and SecondBoss to get the same bosses each try after a wipe
    void readSaveDataExtended(std::istringstream& /*data*/) override;
    void writeSaveDataExtended(std::ostringstream& /*data*/) override;
    bool setBossState(uint32_t /*id*/, EncounterStates /*state*/) override;

    void UpdateEvent() override;

    void setLocalData(uint32_t /*type*/, uint32_t /*data*/) override;
    uint32_t getLocalData(uint32_t /*type*/) const override;

    void spawnPortal();
    void startBossEncounter(uint8_t bossId);
    void resetBossEncounter(uint8_t bossId);
    void startCyanigosaIntro();
    void handleCells(uint8_t bossId, bool open = true);

    void updateKilledBoss(Creature* boss);

    void StateCheck();
    bool WipeCheck();

    bool isBossWave(uint8_t wave) { return wave && ((wave % 6) == 0); }

protected:
    static uint8_t const ErekemGuardCount = 2;
    uint32_t ErekemGuardGUIDs[ErekemGuardCount] = { 0, 0 };

    static uint8_t const ActivationCrystalCount = 5;
    uint32_t ActivationCrystalGUIDs[ActivationCrystalCount] = { 0, 0, 0, 0, 0};

    uint32_t FirstBossId;
    uint32_t SecondBossId;

    uint8_t DoorIntegrity;
    uint8_t WaveCount;
    uint8_t EventState;
    uint8_t LastPortalLocation;

    bool Defenseless;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Gossip: Sinclari
class SinclariGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override;
    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sinclari AI
class SinclariAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit SinclariAI(Creature* pCreature);

    void AIUpdate(unsigned long /*time_passed*/) override;
    void DoAction(int32_t const action) override;
    void justReachedSpawn() override;

    void onSummonedCreature(Creature* /*summon*/) override;
    void OnSummonDespawn(Creature* /*summon*/) override;

    void Reset();

protected:
    InstanceScript* mInstance;
    CreatureAISpells* spellTest;
};

class SinclariTriggerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit SinclariTriggerAI(Creature* pCreature);  

protected:
    InstanceScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trash AI
class TrashAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit TrashAI(Creature* pCreature);

    void AIUpdate(unsigned long /*time_passed*/) override;

    void waypointReached(uint32_t waypointId, uint32_t /*pathId*/) override;

    void SetCreatureData(uint32_t /*type*/, uint32_t /*data*/) override;

    template <size_t N>
    LocationVector const* getPathFrom(LocationVector const (&path)[N])
    {
        mlastWaypointId = N - 1;
        return &path[0];
    }

protected:
    InstanceScript* mInstance;
    uint32_t mlastWaypointId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Guards AI
class VHGuardsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit VHGuardsAI(Creature* pCreature);

    void OnLoad() override;

protected:
    InstanceScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
// DefenseSystem AI
class VHDefenseSystemAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit VHDefenseSystemAI(Creature* pCreature);

    void OnLoad() override;
    void AIUpdate(unsigned long /*time_passed*/) override;

protected:
    InstanceScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Activation Crystal AI
class ActivationCrystalAI : public GameObjectAIScript
{
public:
    static GameObjectAIScript* Create(GameObject* pGameObject);
    explicit ActivationCrystalAI(GameObject* pGameObject);

    void OnSpawn() override;

protected:
    InstanceScript* mInstance;
};

class ActivationCrystalGossip : public GossipScript
{
public:
    void onHello(Object* object, Player* player) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: 58040 - Destroy Door Seal
class DestroyDoorSeal : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override;
    SpellScriptExecuteState onAuraPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float_t* damage) override;

protected:
    InstanceScript* mInstance = nullptr;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: 57912,58152,57930 Arcane Lightning Targetting
class ArcaneLightning : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effIndex, std::vector<uint64_t>* effectTargets) override;
};
