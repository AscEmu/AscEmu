/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Silvershard Mines (map 727). Gate/track-switch entries and positions, and the
// mine cart creature entry, are confirmed present on map 727.
//
// The interactive track-switch lever: it's a creature (entry 60283, "Track Switch")
// tagged per crossroads, spellclicked by players (npc 60283 -> spell 124491) rather than a
// gameobject. Two spawns exist on map 727 - by proximity to each junction's own waypoint
// coordinates, (715.642, 100.165, 320.284) is the Eastern Crossroads switch and (845.557, 307.552,
// 347.038) is the Northern Crossroads switch.
enum SilvershardMinesObjects
{
    SILVERSHARD_GATE_1_ENTRY = 212939,
    SILVERSHARD_GATE_2_ENTRY = 212940,
    SILVERSHARD_GATE_3_ENTRY = 212941,
    SILVERSHARD_GATE_4_ENTRY = 212942,
    SILVERSHARD_MINE_CART_ENTRY = 60140,
    SILVERSHARD_TRACK_SWITCH_ENTRY = 60283,
    SILVERSHARD_NUM_MINES = 3,
    SILVERSHARD_NUM_GATES = 4
};

enum SilvershardMinesSpells
{
    // (npc 60283).
    SILVERSHARD_SPELL_TRACK_SWITCH_CLICK = 124491,

    // Faction-crest visual shown floating above a mine cart while a team controls it - cast on
    // the cart itself, one aura at a time (remove the other two before casting the new one).
    SILVERSHARD_SPELL_CONTROL_ALLIANCE = 116086,
    SILVERSHARD_SPELL_CONTROL_HORDE = 116085,
    SILVERSHARD_SPELL_CONTROL_NEUTRAL = 118001
};

enum SilvershardMineIndex
{
    SILVERSHARD_MINE_SOUTH = 0,
    SILVERSHARD_MINE_NORTH = 1,
    SILVERSHARD_MINE_EAST = 2
};

enum
{
    SILVERSHARD_SCORE_MAX = 1500,
    SILVERSHARD_SCORE_CAPTURE = 150,
    SILVERSHARD_CART_STEP_INTERVAL = 2000, // ms between control re-checks / movement steps

    // Closed = 1, Open = 2. Per-junction meaning:
    //   North Closed(1)->North-West, Open(2)->North-East;
    //   East Closed(1)->East-North, Open(2)->East-South.
    // Default state (before any player interacts) is not confirmed - Closed(1)
    SILVERSHARD_TRACK_CLOSED = 1,
    SILVERSHARD_TRACK_OPEN = 2
};

static constexpr float SILVERSHARD_CONTROL_RADIUS = 15.0f;

// Gate coordinates
static constexpr float SilvershardGateCoords[SILVERSHARD_NUM_GATES][4] =
{
    // x, y, z, orientation
    { 853.163f, 158.905f, 328.082f, 6.25165f },
    { 830.201f, 144.680f, 329.090f, 0.111991f },
    { 635.255f, 208.181f, 327.966f, 3.49866f },
    { 652.856f, 227.673f, 329.004f, 3.52452f }
};

// Track-switch spawn coordinates (map 727, entry 60283).
static constexpr float SilvershardTrackSwitchEast[4] = { 715.642f, 100.165f, 320.284f, 4.59256f };
static constexpr float SilvershardTrackSwitchNorth[4] = { 845.557f, 307.552f, 347.038f, 0.622478f };

// Mine cart spawn coordinates (track start).
static constexpr float SilvershardCartSpawn[SILVERSHARD_NUM_MINES][4] =
{
    { 739.29517f, 203.76389f, 319.54398f, 2.26144f },   // South
    { 759.32465f, 198.33160f, 319.53058f, 0.42151f },   // North
    { 744.51740f, 183.19792f, 319.54395f, 4.33812f },   // East
};

// World states.
enum SilvershardMinesWorldStates
{
    WORLDSTATE_SILVERSHARD_ALLIANCE_SCORE = 6437,
    WORLDSTATE_SILVERSHARD_HORDE_SCORE = 6438,

    WORLDSTATE_SILVERSHARD_ALLIANCE_CONTROLS_SOUTH = 6881,
    WORLDSTATE_SILVERSHARD_HORDE_CONTROLS_SOUTH = 6882,
    WORLDSTATE_SILVERSHARD_ALLIANCE_CONTROLS_NORTH = 6880,
    WORLDSTATE_SILVERSHARD_HORDE_CONTROLS_NORTH = 6879,
    WORLDSTATE_SILVERSHARD_ALLIANCE_CONTROLS_EAST = 6439,
    WORLDSTATE_SILVERSHARD_HORDE_CONTROLS_EAST = 6440,

    WORLDSTATE_SILVERSHARD_EASTERN_TRACK_SWITCH = 6467,
    WORLDSTATE_SILVERSHARD_NORTHERN_TRACK_SWITCH = 6468
};

// Waypoint geometry (x, y, z). South has no junction. North and East
// each have a base segment up to their crossroads
static constexpr float SilvershardSouthRoute[25][3] =
{
    { 738.9360f, 204.1040f, 319.6030f },
    { 736.0470f, 207.6940f, 319.6430f },
    { 732.5240f, 213.6250f, 320.1090f },
    { 730.5610f, 219.1160f, 320.5440f },
    { 728.0050f, 231.5760f, 321.0370f },
    { 725.6280f, 236.9440f, 321.0320f },
    { 722.8370f, 241.6770f, 320.7480f },
    { 720.7120f, 244.5210f, 321.2490f },
    { 715.6460f, 251.4240f, 321.1340f },
    { 710.8590f, 261.0870f, 320.9000f },
    { 707.2900f, 270.0540f, 320.6330f },
    { 703.6530f, 278.1770f, 320.5890f },
    { 696.5240f, 290.1790f, 320.6530f },
    { 686.2600f, 301.6040f, 321.0390f },
    { 667.9500f, 317.1370f, 323.1470f },
    { 660.3980f, 323.9060f, 324.7100f },
    { 651.4030f, 331.2070f, 327.3730f },
    { 638.6810f, 342.3390f, 332.2760f },
    { 633.8320f, 345.2590f, 334.5670f },
    { 626.2170f, 345.4910f, 337.4410f },
    { 616.7410f, 343.2240f, 340.5500f },
    { 601.7220f, 340.3070f, 344.5740f },
    { 592.0590f, 339.2740f, 346.0720f },
    { 585.4740f, 337.6740f, 346.3020f },
    { 563.5090f, 337.6150f, 347.0050f },
};

static constexpr float SilvershardNorthBase[14][3] =
{
    { 759.3250f, 198.3320f, 319.5310f },
    { 762.0160f, 199.5380f, 319.5830f },
    { 765.5940f, 202.5300f, 319.9600f },
    { 770.7860f, 210.3820f, 321.3460f },
    { 776.9570f, 222.9170f, 323.3070f },
    { 780.1960f, 232.6220f, 324.9790f },
    { 783.5830f, 241.6460f, 327.9300f },
    { 787.6270f, 249.4010f, 330.7880f },
    { 794.5850f, 259.5800f, 336.8150f },
    { 803.3460f, 269.5900f, 341.3370f },
    { 816.4270f, 282.5640f, 344.8490f },
    { 823.8920f, 289.4270f, 345.9660f },
    { 830.2730f, 294.8580f, 346.5570f },
    { 835.5920f, 300.4180f, 346.9560f },
};

static constexpr float SilvershardNorthEastFork[39][3] =
{
    { 838.2570f, 302.2190f, 347.3340f },
    { 843.2570f, 303.4690f, 347.3340f },
    { 848.2570f, 303.7190f, 347.3340f },
    { 854.0070f, 303.2190f, 347.3340f },
    { 860.0070f, 301.4690f, 347.5840f },
    { 863.5070f, 299.9690f, 347.5840f },
    { 867.2570f, 297.4690f, 347.5840f },
    { 877.2570f, 287.7190f, 347.5840f },
    { 881.7570f, 279.2190f, 347.3340f },
    { 885.7570f, 270.4690f, 346.8340f },
    { 889.2570f, 261.2190f, 346.3340f },
    { 893.5070f, 248.7190f, 346.0840f },
    { 894.7570f, 241.7190f, 346.5840f },
    { 897.7570f, 226.4690f, 350.0840f },
    { 904.2570f, 210.4690f, 354.8340f },
    { 911.5070f, 178.4690f, 363.8340f },
    { 912.5070f, 168.2190f, 366.5840f },
    { 912.0070f, 163.2190f, 366.8340f },
    { 909.0070f, 153.4690f, 366.8340f },
    { 907.0070f, 149.7190f, 366.8340f },
    { 898.2570f, 138.7190f, 366.8340f },
    { 888.2570f, 126.2190f, 366.5840f },
    { 885.5070f, 120.4690f, 366.3340f },
    { 882.7570f, 109.2190f, 365.8340f },
    { 882.2570f, 104.7190f, 365.5840f },
    { 883.7570f, 97.2188f, 365.3340f },
    { 889.0070f, 89.4688f, 365.0840f },
    { 891.0070f, 82.4688f, 365.0840f },
    { 891.0070f, 76.4688f, 364.8340f },
    { 890.2570f, 70.9688f, 365.0840f },
    { 890.7570f, 62.9688f, 364.8340f },
    { 891.7570f, 56.2188f, 364.5840f },
    { 893.9220f, 45.5191f, 363.7120f },
    { 893.2970f, 41.3802f, 363.8320f },
    { 893.2970f, 37.8802f, 364.0820f },
    { 893.2970f, 35.1302f, 364.0820f },
    { 893.5470f, 33.1302f, 364.0820f },
    { 895.2970f, 29.3802f, 364.0820f },
    { 897.1720f, 25.2413f, 363.9520f },
};

static constexpr float SilvershardNorthWestFork[23][3] =
{
    { 837.7410f, 303.2180f, 347.3940f },
    { 840.7410f, 307.4680f, 347.3940f },
    { 844.7410f, 314.2180f, 347.3940f },
    { 847.9910f, 321.7180f, 347.6440f },
    { 848.7410f, 326.9680f, 347.3940f },
    { 846.9910f, 342.7180f, 347.3940f },
    { 843.4910f, 350.9680f, 347.6440f },
    { 831.9910f, 372.4680f, 347.3940f },
    { 822.4910f, 397.4680f, 347.1440f },
    { 818.7410f, 406.9680f, 348.1440f },
    { 817.9910f, 410.4680f, 348.8940f },
    { 816.7410f, 419.2180f, 350.6440f },
    { 817.2410f, 422.7180f, 351.3940f },
    { 815.9910f, 429.2180f, 353.3940f },
    { 814.2410f, 434.7180f, 355.1440f },
    { 810.9910f, 442.2180f, 357.1440f },
    { 807.4910f, 457.7180f, 358.8940f },
    { 806.7410f, 466.4680f, 359.1440f },
    { 803.9910f, 472.9680f, 359.3940f },
    { 800.9910f, 476.9680f, 359.6440f },
    { 793.7410f, 485.2180f, 359.6440f },
    { 791.3910f, 487.5170f, 359.3320f },
    { 778.2710f, 502.7500f, 359.2930f },
};

static constexpr float SilvershardEastBase[10][3] =
{
    { 744.5170f, 183.1980f, 319.5440f },
    { 742.8540f, 178.9640f, 319.6020f },
    { 739.8910f, 171.5190f, 319.3650f },
    { 735.5960f, 163.5430f, 319.2190f },
    { 730.4770f, 155.1940f, 319.1300f },
    { 728.8580f, 152.4060f, 319.6250f },
    { 723.9240f, 143.1040f, 319.6670f },
    { 719.3650f, 131.0940f, 319.5530f },
    { 717.6010f, 122.7220f, 320.0980f },
    { 716.9510f, 113.5170f, 320.8440f },
};

static constexpr float SilvershardEastSouthFork[14][3] =
{
    { 715.8230f, 109.0370f, 321.0150f },
    { 713.3230f, 104.2870f, 320.2650f },
    { 706.8230f, 99.2873f, 319.0150f },
    { 701.8230f, 97.2873f, 317.7650f },
    { 695.0730f, 96.5373f, 316.2650f },
    { 686.5730f, 95.5373f, 314.5150f },
    { 680.3230f, 94.5373f, 312.7650f },
    { 670.3230f, 90.0373f, 309.5150f },
    { 663.8230f, 86.5373f, 307.0150f },
    { 660.3230f, 84.5373f, 305.7650f },
    { 647.3230f, 82.0373f, 302.0150f },
    { 638.5730f, 81.5373f, 299.5150f },
    { 634.6940f, 81.0573f, 298.6860f },
    { 615.4150f, 79.6458f, 298.1010f },
};

static constexpr float SilvershardEastNorthFork[32][3] =
{
    { 716.9750f, 108.5020f, 321.1370f },
    { 719.9750f, 101.5020f, 321.3870f },
    { 724.4750f, 98.2517f, 322.1370f },
    { 732.7250f, 94.7517f, 323.8870f },
    { 737.7250f, 93.7517f, 325.8870f },
    { 744.4750f, 92.2517f, 328.1370f },
    { 758.7250f, 89.2517f, 332.8870f },
    { 767.2250f, 86.0017f, 336.1370f },
    { 774.2250f, 82.2517f, 339.3870f },
    { 784.2250f, 79.5017f, 342.8870f },
    { 793.7250f, 78.2517f, 346.1370f },
    { 798.2250f, 77.2517f, 348.1370f },
    { 803.9750f, 74.7517f, 349.8870f },
    { 807.7250f, 72.2517f, 351.1370f },
    { 813.4750f, 67.7517f, 352.8870f },
    { 819.9750f, 64.0017f, 354.8870f },
    { 823.9750f, 63.0017f, 355.8870f },
    { 829.7250f, 63.0017f, 357.1370f },
    { 833.4750f, 64.2517f, 357.8870f },
    { 842.2250f, 70.5017f, 359.8870f },
    { 850.4750f, 71.5017f, 361.3870f },
    { 859.4750f, 69.0017f, 362.8870f },
    { 865.4750f, 65.2517f, 363.8870f },
    { 873.4750f, 58.0017f, 364.6370f },
    { 879.7250f, 51.7517f, 364.6370f },
    { 885.4980f, 42.9861f, 363.9300f },
    { 887.8980f, 40.0521f, 363.9600f },
    { 890.1480f, 37.0521f, 363.9600f },
    { 891.8980f, 34.8021f, 364.2100f },
    { 893.1480f, 33.3021f, 364.2100f },
    { 895.1480f, 29.3021f, 364.2100f },
    { 896.7970f, 25.6181f, 363.9900f },
};
