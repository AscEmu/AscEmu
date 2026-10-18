/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "AlteracValley.h"

#include "Management/Loot/LootItem.hpp"
#include "Management/Loot/LootRoll.hpp"
#include "Management/HonorHandler.h"
#include "Management/WorldStates.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Map/Management/ObjectFactory.hpp"
#include "Map/Management/SpawnManager.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Server/Master.h"
#include "Server/World.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/PacketBroadcast.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Utilities/CommonTime.hpp"
#include <cstdarg>

#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////////////////
// Fire Locations

const AVSpawnLocation g_fireLocations[AV_NUM_CONTROL_POINTS][30] =
{
    { { 0.0f, 0.0f, 0.0f }, }, // Aid Station
    { { 0.0f, 0.0f, 0.0f }, }, // Stormpike Graveyard
    { { 0.0f, 0.0f, 0.0f }, }, // Stonehearth Graveyard
    { { 0.0f, 0.0f, 0.0f }, }, // Snowfall graveyard
    { { 0.0f, 0.0f, 0.0f }, }, // Coldtooth mine
    { { 0.0f, 0.0f, 0.0f }, }, // Irondeep mine
    { { 0.0f, 0.0f, 0.0f }, }, // Iceblood gy
    { { 0.0f, 0.0f, 0.0f }, }, // Frostwolf gy
    { { 0.0f, 0.0f, 0.0f }, }, // Frostwolf relief
    {
        // dun baldar nth
        { 667.917297f, -137.020660f, 63.645058f, 1.094849f },
        { 680.675903f, -125.554802f, 63.666199f, 0.089540f },
        { 685.923828f, -148.394562f, 56.618328f, 1.138047f },
        { 667.368164f, -147.642548f, 56.621113f, 4.523115f },
        { 666.998413f, -143.095612f, 49.673607f, 4.122564f },
        { 676.130615f, -123.653824f, 49.672577f, 2.744190f },
        { 666.790649f, -127.394073f, 49.638577f, 3.348947f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // dun baldar sth
        { 568.422852f, -83.935478f, 51.942871f, 2.324003f },
        { 547.981018f, -93.095024f, 51.942841f, 0.898504f },
        { 549.647034f, -80.551971f, 44.820171f, 2.858073f },
        { 552.589355f, -96.653267f, 44.819138f, 0.619688f },
        { 562.575439f, -78.773170f, 37.949261f, 1.644633f },
        { 568.352722f, -92.772797f, 37.949261f, 0.761060f },
        { 575.015137f, -78.856613f, 37.921127f, 0.596126f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // icewing
        { 198.799316f, -361.116241f, 56.391476f, 5.363494f },
        { 217.211273f, -375.348236f, 56.368317f, 0.494023f },
        { 200.838547f, -362.912231f, 49.267738f, 2.708846f },
        { 208.172836f, -379.141357f, 49.267738f, 5.826873f },
        { 209.692886f, -356.993683f, 42.398838f, 2.335778f },
        { 223.358307f, -374.435516f, 42.396271f, 5.288878f },
        { 222.380539f, -357.525726f, 42.361271f, 1.208734f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // stonehearth
        { -162.115128f, -458.521393f, 40.389114f, 5.921130f },
        { -146.283035f, -451.985413f, 40.392101f, 1.593585f },
        { -154.911880f, -440.288513f, 33.280674f, 3.392147f },
        { -166.347992f, -458.094086f, 33.279636f, 4.809789f },
        { -146.530441f, -458.876343f, 26.394993f, 5.257466f },
        { -146.420120f, -445.917725f, 26.408756f, 1.020243f },
        { -139.567215f, -456.592163f, 26.380287f, 2.649942f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // iceblood
        { -569.760010f, -263.288513f, 75.008682f, 2.104091f },
        { -570.742493f, -271.907837f, 74.988449f, 3.238992f },
        { -578.531372f, -253.600327f, 74.959206f, 0.647177f },
        { -561.223206f, -260.727386f, 68.457199f, 3.820190f },
        { -579.971008f, -265.969604f, 68.469582f, 2.068751f },
        { -571.457458f, -256.716736f, 63.293938f, 0.509736f },
        { -567.616211f, -265.129028f, 59.324265f, 4.538830f },
        { -579.480103f, -261.568329f, 52.497894f, 1.821352f },
        { -566.202026f, -260.122009f, 52.728100f, 5.634461f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // tower point
        { -769.523010f, -358.549561f, 68.635841f, 3.853180f },
        { -762.504395f, -362.904480f, 68.543678f, 5.730278f },
        { -761.258606f, -360.407471f, 72.666672f, 1.618719f },
        { -772.757813f, -365.472168f, 79.463570f, 5.408264f },
        { -764.915161f, -358.086975f, 84.355766f, 3.004945f },
        { -773.573181f, -366.593475f, 84.355766f, 4.414734f },
        { -777.062927f, -370.895477f, 90.868019f, 5.832375f },
        { -758.860840f, -358.962280f, 90.825951f, 4.073083f },
        { -768.840515f, -362.985535f, 90.894974f, 4.304777f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // east frostwolf
        { -1304.755249f, -322.213409f, 91.419922f, 5.141234f },
        { -1304.633057f, -310.950684f, 91.677505f, 0.746931f },
        { -1301.672974f, -309.800598f, 95.747795f, 3.586145f },
        { -1303.074585f, -321.931915f, 102.658630f, 0.259982f },
        { -1303.221680f, -310.574463f, 107.328194f, 3.590071f },
        { -1302.911133f, -323.911835f, 107.328049f, 4.913466f },
        { -1302.600464f, -328.408600f, 113.846321f, 1.210312f },
        { -1303.177612f, -307.762390f, 113.797401f, 6.177956f },
        { -1305.213989f, -315.778412f, 113.867081f, 5.251186f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // west frostwolf
        { -1299.614014f, -263.684998f, 114.151276f, 4.944896f },
        { -1297.918335f, -269.959930f, 114.151276f, 4.772108f },
        { -1304.088379f, -272.878387f, 114.098404f, 5.200147f },
        { -1287.346313f, -267.391785f, 114.115089f, 1.826860f },
        { -1298.288818f, -272.091309f, 107.612137f, 2.883221f },
        { -1298.199463f, -258.249390f, 107.612183f, 3.052081f },
        { -1300.405640f, -273.112244f, 100.317612f, 4.929180f },
        { -1291.555908f, -261.006134f, 91.648979f, 3.244500f },
        { -1299.686279f, -273.323853f, 91.897820f, 3.876745f },
        { 0.0f, 0.0f, 0.0f },
    },
};

//////////////////////////////////////////////////////////////////////////////////////////
// Waypoints

///\todo Big TODO

//(06:46:20) (~compboy) FoS -> SH Bunker -> SH Outpost -> IW Bunker -> Dun Baldar

/*
lokholar ice lord spawn:
x: -301.823792
y: -271.596832
z: 6.667519
o: 3.5790528

ivus the forest lord
x: -221.667648
y: -311.846893
z: 6.667575
o: 2.295235
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Initial Guard Locations

const AVSpawnLocation g_initalGuardLocations[AV_NUM_CONTROL_POINTS][30] =
{
    { { 0.0f, 0.0f, 0.0f }, }, // Aid Station
    { { 0.0f, 0.0f, 0.0f }, }, // Stormpike Graveyard
    { { 0.0f, 0.0f, 0.0f }, }, // Stonehearth Graveyard
    { { 0.0f, 0.0f, 0.0f }, }, // Snowfall graveyard
    { { 0.0f, 0.0f, 0.0f }, }, // Coldtooth mine
    { { 0.0f, 0.0f, 0.0f }, }, // Irondeep mine
    { { 0.0f, 0.0f, 0.0f }, }, // Iceblood gy
    { { 0.0f, 0.0f, 0.0f }, }, // Frostwolf gy
    { { 0.0f, 0.0f, 0.0f }, }, // Frostwolf relief

    {
        // dun baldar nth
        { 672.141296f, -120.239807f, 64.147659f, 2.379753f },
        { 664.585083f, -126.076088f, 64.120972f, 2.768524f },
        { 661.464905f, -137.339371f, 64.216965f, 3.436112f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // dun baldar sth
        { 564.124146f, -71.107430f, 52.488060f, 1.300614f },
        { 571.863770f, -77.084518f, 52.367657f, 0.295303f },
        { 574.969543f, -90.801270f, 52.412132f, 5.910901f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // icewing
        { 222.674530f, -378.605408f, 57.147060f, 5.577111f },
        { 210.093506f, -384.971558f, 56.793076f, 4.595366f },
        { 199.602188f, -380.995575f, 56.864891f, 3.754991f },
        { 192.782074f, -370.546204f, 57.015110f, 3.586132f },
        { 193.377426f, -360.313507f, 57.044708f, 2.891055f },
        { 201.253113f, -351.977631f, 56.802952f, 3.036354f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // stonehearth
        { -139.692780f, -445.390533f, 40.982857f, 0.527005f },
        { -140.723969f, -457.597168f, 40.862610f, 5.643875f },
        { -148.143784f, -464.959808f, 40.933720f, 5.639948f },
        { -161.013535f, -466.793640f, 41.118568f, 4.512904f },
        { -170.595215f, -457.926331f, 40.875153f, 3.385859f },
        { -172.157013f, -447.972717f, 40.888519f, 2.894984f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // iceblood
        { -568.816711f, -255.542038f, 75.008690f, 1.025736f },
        { -580.106445f, -264.803497f, 74.931145f, 3.931711f },
        { -572.966248f, -271.499786f, 74.933746f, 4.198746f },
        { -565.585876f, -268.645294f, 74.914063f, 5.459307f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // tower point
        { -760.347595f, -358.281586f, 90.885597f, 0.358147f },
        { -759.719116f, -367.059113f, 90.826775f, 5.722414f },
        { -768.441956f, -372.756653f, 90.933365f, 4.991992f },
        { -775.595032f, -365.525177f, 90.894867f, 2.989226f },
        { -771.311890f, -353.218842f, 90.821220f, 1.795420f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // east frostwolf
        { -1297.069092f, -309.338623f, 113.769043f, 1.002180f },
        { -1293.462036f, -316.529602f, 113.774048f, 0.067557f },
        { -1298.062256f, -326.274994f, 113.820679f, 5.015566f },
        { -1311.626953f, -317.071228f, 113.775551f, 2.977457f },
        { -1305.169922f, -309.871796f, 113.824280f, 5.192283f },
        { -1293.822021f, -317.798065f, 113.771339f, 3.103136f },
        { 0.0f, 0.0f, 0.0f },
    },

    {
        // west frostwolf
        { -1299.964233f, -275.591461f, 114.055862f, 1.241742f },
        { -1306.900757f, -268.969574f, 114.055481f, 3.464420f },
        { -1305.109375f, -261.103363f, 114.068550f, 2.298103f },
        { -1295.956177f, -258.076904f, 114.056999f, 1.277085f },
        { -1288.039917f, -264.262085f, 114.115341f, 0.224653f },
        { 0.0f, 0.0f, 0.0f },
    },
};

//////////////////////////////////////////////////////////////////////////////////////////
// Home NPCs (that sit in the base)

struct AVHomeNPC { uint32_t id_a; uint32_t id_h; float a_x; float a_y; float a_z; float a_o; float h_x; float h_y; float h_z; float h_o; };
const AVHomeNPC g_HomeNpcInfo[AV_NUM_CONTROL_POINTS] =
{
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // stormpike aid station
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // stormpike graveyard
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // stonehearth graveyard
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // snowfall graveyard
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // coldtooth mine
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // irondeep mine
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // iceblood graveyard
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // frostwolf graveyard
    { 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },                                                                        // frostwolf relief hut
    { 14762, 14770, 726.534058f, -31.043175f, 50.621307f, 1.958136f, -1373.700684f, -238.947479f, 99.362076f, 1.217441f },           // north dun baldar bunker
    { 14763, 14771, 722.385925f, -32.735390f, 50.621307f, 1.958136f, -239.058929f, 99.367973f, 1.866966f },                          // south dun baldar bunker
    { 14764, 14774, 717.237793f, -34.835552f, 50.621307f, 1.958136f, -1356.338135f, -232.723816f, 99.365128f, 2.250240f },           // icewing bunker
    { 14765, 14775, 711.430664f, -36.070595f, 50.621307f, 1.958136f, -1349.830688f, -222.138000f, 99.370834f, 3.518692f },           // stonehearth bunker
    { 14766, 14773, 697.683472f, 3.335777f, 50.621426f, 5.021224f,   -1359.854248f, -212.682831f, 99.371040f, 4.256965f },           // iceblood tower
    { 14767, 14776, 700.351013f, 4.186835f, 50.621426f, 5.021224f,   -1372.945435f, -210.555832f, 99.371292f, 5.054141f },           // tower point
    { 14768, 14772, 703.704468f, 5.256737f, 50.621426f, 5.021224f,   -1383.414795f, -219.701355f, 99.371292f, 5.971476f },           // east frostwolf tower
    { 14769, 14777, 708.544128f, 6.800801f, 50.621426f, 5.021224f,   -1384.437012f, -233.046158f, 99.371338f, 0.227871f },           // west frostwolf tower
};

//////////////////////////////////////////////////////////////////////////////////////////
// Control Point Templates

static AVNodeTemplate g_nodeTemplates[AV_NUM_CONTROL_POINTS] =
{
    //      Name                gy?  cap?           graveyard location
    { "Stormpike Aid Station", true, true, { 634.607971f, 47.408627f, 69.890488f },
    // Control Point Definition, ids, position vector, rotations
    // neutral  a-ass   a-con   h-ass   h-con                           position                            rotation
    { { 179286, 179286, 179284, 179287, 179285 }, 639.184326f, -31.640333f, 46.202557f, 1.3546723f, 0.9025853f, -0.4305111f },
    // Aura Definition
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    // Glow Definition
    { { 180102, 180102, 180100, 180102, 180101 }, 639.184326f, -31.640333f, 46.202557f, 1.3546723f, 0.0f, 0.0f },
    // Guard IDs
    //N,   A,     H   guardC  boss ids peon locs   boss location  fire locations
    { 0, 12050, 12053 }, 5, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 0, 0,
    // World State Fields
    WORLDSTATE_AV_STORMPIKE_AID_STATION_ALLIANCE_ASSAULTING, WORLDSTATE_AV_STORMPIKE_AID_STATION_ALLIANCE_CONTROLLED,
    WORLDSTATE_AV_STORMPIKE_AID_STATION_HORDE_ASSAULTING, WORLDSTATE_AV_STORMPIKE_AID_STATION_HORDE_CONTROLLED,
    AV_NODE_STATE_ALLIANCE_CONTROLLED, }, // Default World State

    { "Stormpike Graveyard", true, true, { 675.970947f, -373.707336f, 29.780785f },
    { { 179286, 179286, 179284, 179287, 179285 }, 670.904175f, -294.181641f, 30.283506f, 4.232887f, 0.4067366f, 0.9135454f },
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { { 180102, 180102, 180100, 180102, 180101 }, 670.904175f, -294.181641f, 30.283506f, 4.232887f, 0.0f, 0.0f },
    { 0, 12050, 12053 }, 5, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 0, 0,
    WORLDSTATE_AV_STORMPIKE_GRAVE_ALLIANCE_ASSAULTING, WORLDSTATE_AV_STORMPIKE_GRAVE_ALLIANCE_CONTROLLED,
    WORLDSTATE_AV_STORMPIKE_GRAVE_HORDE_ASSAULTING, WORLDSTATE_AV_STORMPIKE_GRAVE_HORDE_CONTROLLED, AV_NODE_STATE_ALLIANCE_CONTROLLED, },

    { "Stonehearth Graveyard", true, true, { 73.869370f, -495.597656f, 48.740143f },
    { { 179286, 179286, 179284, 179287, 179285 }, 78.367012f, -404.769928f, 47.051014f, 4.350670f, 0.2672384f, 0.9636304f },
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { { 180102, 180102, 180100, 180102, 180101 }, 78.367012f, -404.769928f, 47.051014f, 4.350670f, 0.0f, 0.0f },
    { 0, 12050, 12053 }, 5, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 0, 0, WORLDSTATE_AV_STONEHEARTH_GRAVE_ALLIANCE_ASSAULTING,
    WORLDSTATE_AV_STONEHEARTH_GRAVE_ALLIANCE_CONTROLLED, WORLDSTATE_AV_STONEHEARTH_GRAVE_HORDE_ASSAULTING,
    WORLDSTATE_AV_STONEHEARTH_GRAVE_HORDE_CONTROLLED, AV_NODE_STATE_ALLIANCE_CONTROLLED, },

    { "Snowfall Graveyard", true, true, { -161.472916f, 34.797512f, 77.191841f },
    { { 180418, 179286, 179284, 179287, 179285 }, -202.571854f, -112.612862f, 78.489014f, 1.666146f, 0.9366722f, 0.3502073f },
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { { 180102, 180102, 180100, 180102, 180101 }, -202.571854f, -112.612862f, 78.489014f, 1.666146f, 0.0f, 0.0f },
    { 0, 12050, 12053 }, 5, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 0, WORLDSTATE_AV_SNOWFALL_GRAVE_NEUTRAL_CONTROLLED,
    WORLDSTATE_AV_SNOWFALL_GRAVE_ALLIANCE_ASSAULTING, WORLDSTATE_AV_SNOWFALL_GRAVE_ALLIANCE_CONTROLLED,
    WORLDSTATE_AV_SNOWFALL_GRAVE_HORDE_ASSAULTING, WORLDSTATE_AV_SNOWFALL_GRAVE_HORDE_CONTROLLED, AV_NODE_STATE_NEUTRAL_CONTROLLED, },

    // sch: mine ownership is changed by killing the neutral mine boss. The bosses are
    // already database-spawned; do not create another copy here.
    { "Coldtooth Mine", false, true, { 0.0f, 0.0f, 0.0f }, { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, AV_NPC_TASKMASTER_SNIVVLE }, nullptr, { -875.337f, -116.294f, 65.0083f }, 0, WORLDSTATE_AV_COLDTOOTH_MINE_KOBOLD_CONTROLLED, 0,
    WORLDSTATE_AV_COLDTOOTH_MINE_ALLIANCE_CONTROLLED, 0, WORLDSTATE_AV_COLDTOOTH_MINE_HORDE_CONTROLLED, AV_NODE_STATE_NEUTRAL_CONTROLLED, },

    { "Irondeep Mine", false, true, { 0.0f, 0.0f, 0.0f }, { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, AV_NPC_MORLOCH }, nullptr, { 802.0f, -323.101f, 52.365f }, 0, WORLDSTATE_AV_IRONDEEP_MINE_TROGG_CONTROLLED, 0,
    WORLDSTATE_AV_IRONDEEP_MINE_ALLIANCE_CONTROLLED, 0, WORLDSTATE_AV_IRONDEEP_MINE_HORDE_CONTROLLED, AV_NODE_STATE_NEUTRAL_CONTROLLED, },

    { "Iceblood Graveyard", true, true, { -531.067627f, -405.459778f, 49.552971f },
    { { 179286, 179286, 179284, 179287, 179285 }, -611.891113f, -395.925751f, 60.798248f, 3.193738f, 0.9135455f, -0.4067366f },
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { { 180102, 180102, 180100, 180102, 180101 }, -611.891113f, -395.925751f, 60.798248f, 3.193738f, 0.0f, 0.0f },
    { 0, 12050, 12053 }, 5, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 0, 0, WORLDSTATE_AV_ICEBLOOD_GRAVE_ALLIANCE_ASSAULTING,
    WORLDSTATE_AV_ICEBLOOD_GRAVE_ALLIANCE_CONTROLLED, WORLDSTATE_AV_ICEBLOOD_GRAVE_HORDE_ASSAULTING,
    WORLDSTATE_AV_ICEBLOOD_GRAVE_HORDE_CONTROLLED, AV_NODE_STATE_HORDE_CONTROLLED, },

    { "Frostwolf Graveyard", true, true, { -1088.008545f, -248.774918f, 57.680843f },
    { { 179286, 179286, 179284, 179287, 179285 }, -1082.267578f, -344.454590f, 55.272388f, 1.579743f, 0.9025853f, -0.4305111f },
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { { 180102, 180102, 180100, 180102, 180101 }, -1082.267578f, -344.454590f, 55.272388f, 1.579743f, 0.0f, 0.0f },
    { 0, 12050, 12053 }, 5, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 0, 0, WORLDSTATE_AV_FROSTWOLF_GRAVE_ALLIANCE_ASSAULTING,
    WORLDSTATE_AV_FROSTWOLF_GRAVE_ALLIANCE_CONTROLLED, WORLDSTATE_AV_FROSTWOLF_GRAVE_HORDE_ASSAULTING,
    WORLDSTATE_AV_FROSTWOLF_GRAVE_HORDE_CONTROLLED, AV_NODE_STATE_HORDE_CONTROLLED, },

    { "Frostwolf Relief Hut", true, true, { -1500.063599f, -333.146393f, 101.133186f },
    { { 179286, 179286, 179284, 179287, 179285 }, -1401.263550f, -309.601624f, 89.412560f, 3.538534f, 0.4067366f, 0.9135454f },
    { { 0, 0, 0, 0, 0 }, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    { { 180102, 180102, 180100, 180102, 180101 }, -1401.263550f, -309.601624f, 89.412560f, 3.538534f, 0.0f, 0.0f },
    { 0, 12050, 12053 }, 5, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 0, 0, WORLDSTATE_AV_FROSTWOLF_RELIEF_HUT_ALLIANCE_ASSAULTING,
    WORLDSTATE_AV_FROSTWOLF_RELIEF_HUT_ALLIANCE_CONTROLLED, WORLDSTATE_AV_FROSTWOLF_RELIEF_HUT_HORDE_ASSAULTING,
    WORLDSTATE_AV_FROSTWOLF_RELIEF_HUT_HORDE_CONTROLLED, AV_NODE_STATE_HORDE_CONTROLLED, },

    { "North Dun Baldar Bunker", false, false, { 0.0f, 0.0f, 0.0f },
    { { 0, 179286, 179284, 179287, 0 }, 673.361877f, -144.169357f, 63.651852f, 0.969958f, 0.2672384f, 0.9636304f },
    { { 0, 179446, 178927, 179446, 0 }, 678.393250f, -136.145126f, 76.004265f, 4.515254f, 0.0f, 0.0f },
    { { 0, 180423, 180421, 180423, 0 }, 678.393250f, -136.145126f, 76.004265f, 4.515254f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 13358, 0, 0, WORLDSTATE_AV_DUN_BALDAR_NORTH_BUNKER_CONTROLLED,
    WORLDSTATE_AV_DUN_BALDAR_NORTH_BUNKER_ASSAULTING, WORLDSTATE_AV_DUN_BALDAR_NORTH_BUNKER_DESTROYED, AV_NODE_STATE_ALLIANCE_CONTROLLED, },

    { "South Dun Baldar Bunker", false, false, { 0.0f, 0.0f, 0.0f },
    { { 0, 179286, 179284, 179287, 0 }, 552.990234f, -77.527374f, 51.926868f, 3.5410520f, 0.9366722f, 0.3502073f },
    { { 0, 179450, 178932, 179450, 0 }, 556.081543f, -84.437355f, 64.411903f, 5.006125f, 0.0f, 0.0f },
    { { 0, 180423, 180421, 180423, 0 }, 556.081543f, -84.437355f, 64.411903f, 5.006125f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 13358, 0, 0, WORLDSTATE_AV_DUN_BALDAR_SOUTH_BUNKER_CONTROLLED,
    WORLDSTATE_AV_DUN_BALDAR_SOUTH_BUNKER_ASSAULTING, WORLDSTATE_AV_DUN_BALDAR_SOUTH_BUNKER_DESTROYED, AV_NODE_STATE_ALLIANCE_CONTROLLED, },

    { "Icewing Bunker", false, false, { 0.0f, 0.0f, 0.0f },
    { { 0, 179286, 179284, 179287, 0 }, 201.845840f, -358.886963f, 56.370701f, 5.403111f, 0.9135455f, -0.4067366f },
    { { 0, 179454, 178947, 179454, 0 }, 208.691223f, -367.553314f, 68.696632f, 3.317518f, 0.0f, 0.0f },
    { { 0, 180423, 180421, 180423, 0 }, 208.691223f, -367.553314f, 68.696632f, 3.317518f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 13358, 0, 0, WORLDSTATE_AV_ICEWING_BUNKER_CONTROLLED,
    WORLDSTATE_AV_ICEWING_BUNKER_ASSAULTING, WORLDSTATE_AV_ICEWING_BUNKER_DESTROYED, AV_NODE_STATE_ALLIANCE_CONTROLLED, },

    { "Stonehearth Bunker", false, false, { 0.0f, 0.0f, 0.0f },
    { { 0, 179286, 179284, 179287, 0 }, -151.776291f, -439.776245f, 40.381840f, 4.319230f, 0.9025853f, -0.4305111f },
    { { 0, 179458, 178948, 179458, 0 }, -156.257721f, -448.993652f, 52.797112f, 2.803080f, 0.0f, 0.0f },
    { { 0, 180423, 180421, 180423, 0 }, -156.257721f, -448.993652f, 52.797112f, 2.803080f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 13358, 0, 0, WORLDSTATE_AV_STONEHEARTH_BUNKER_CONTROLLED,
    WORLDSTATE_AV_STONEHEARTH_BUNKER_ASSAULTING, WORLDSTATE_AV_STONEHEARTH_BUNKER_DESTROYED, AV_NODE_STATE_ALLIANCE_CONTROLLED, },

    { "Iceblood Tower", false, false, { 0.0f, 0.0f, 0.0f },
    { { 0, 179286, 0, 179287, 179285 }, -572.502869f, -262.418365f, 75.008713f, 5.483192f, 0.4067366f, 0.9135454f },
    { { 0, 179436, 0, 179436, 178955 }, -572.408386f, -262.620422f, 88.636307f, 0.712354f, 0.0f, 0.0f },
    { { 0, 180423, 0, 180423, 180422 }, -572.408386f, -262.620422f, 88.636307f, 0.712354f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 13359, 0, WORLDSTATE_AV_ICEBLOOD_TOWER_ASSAULTING,
    WORLDSTATE_AV_ICEBLOOD_TOWER_DESTROYED, 0, WORLDSTATE_AV_ICEBLOOD_TOWER_CONTROLLED, AV_NODE_STATE_HORDE_CONTROLLED, },

    { "Tower Point", false, false, { 0.0f, 0.0f, 0.0f },
    { { 0, 179286, 0, 179287, 179285 }, -767.986511f, -363.091064f, 90.894920f, 4.265791f, 0.2672384f, 0.9636304f },
    { { 0, 179440, 0, 179440, 178956 }, -768.181519f, -362.942719f, 104.553452f, 4.761085f, 0.0f, 0.0f },
    { { 0, 180423, 0, 180423, 180422 }, -768.181519f, -362.942719f, 104.553452f, 4.761085f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 13359, 0, WORLDSTATE_AV_TOWERPOINT_ASSAULTING,
    WORLDSTATE_AV_TOWERPOINT_DESTROYED, 0, WORLDSTATE_AV_TOWERPOINT_CONTROLLED, AV_NODE_STATE_HORDE_CONTROLLED, },

    { "East Frostwolf Tower", false, false, { 0.0f, 0.0f, 0.0f },
    { { 0, 179286, 0, 179287, 179285 }, -1302.814453f, -315.957458f, 113.867195f, 1.956738f, 0.9366722f, 0.3502073f },
    { { 0, 179442, 0, 179442, 178957 }, -1302.916626f, -316.606079f, 127.526337f, 6.176374f, 0.0f, 0.0f },
    { { 0, 180423, 0, 180423, 180422 }, -1302.916626f, -316.606079f, 127.526337f, 6.176374f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 13359, 0, WORLDSTATE_AV_FROSTWOLF_EAST_TOWER_ASSAULTING,
    WORLDSTATE_AV_FROSTWOLF_EAST_TOWER_DESTROYED, 0, WORLDSTATE_AV_FROSTWOLF_EAST_TOWER_CONTROLLED, AV_NODE_STATE_HORDE_CONTROLLED, },

    { "West Frostwolf Tower", false, false, { 0.0f, 0.0f, 0.0f },
    { { 0, 179286, 0, 179287, 179285 }, -1296.913818f, -266.521271f, 114.151245f, 3.336680f, 0.9135455f, -0.4067366f },
    { { 0, 179444, 0, 179444, 178958 }, -1297.889404f, -266.714233f, 127.789467f, 5.931325f, 0.0f, 0.0f },
    { { 0, 180423, 0, 180423, 180422 }, -1297.889404f, -266.714233f, 127.789467f, 5.931325f, 0.0f, 0.0f },
    { 0, 0, 0 }, 0, { 0, 0, 0 }, nullptr, { 0.0f, 0.0f, 0.0f }, 13359, 0, WORLDSTATE_AV_FROSTWOLF_WEST_TOWER_ASSAULTING,
    WORLDSTATE_AV_FROSTWOLF_WEST_TOWER_DESTROYED, 0, WORLDSTATE_AV_FROSTWOLF_WEST_TOWER_CONTROLLED, AV_NODE_STATE_HORDE_CONTROLLED, },
};

// gameobject faction def'n
static uint32_t g_gameObjectFactions[AV_NODE_STATE_COUNT] =
{
    35,               // neutral
    2,                // alliance assault
    2,                // alliance controlled
    1,                // horde assault
    1,                // horde controlled
};

const static int32_t g_stateToGuardType[AV_NODE_STATE_COUNT] =
{
    0,                // neutral
    -1,               // alliance assault
    1,                // alliance controlled
    -2,               // horde assault
    2,                // horde controlled
};

static const char* g_stateNames[AV_NODE_STATE_COUNT] =
{
    "AV_NODE_STATE_NEUTRAL_CONTROLLED",
    "AV_NODE_STATE_ALLIANCE_ASSAULTING",
    "AV_NODE_STATE_ALLIANCE_CONTROLLED",
    "AV_NODE_STATE_HORDE_ASSAULTING",
    "AV_NODE_STATE_HORDE_CONTROLLED",
};

AlteracValley::AVNode::AVNode(AlteracValley* parent, AVNodeTemplate* tmpl, uint32_t nodeid)
{
    m_bg = parent;
    m_template = tmpl;
    m_nodeId = nodeid;
    m_boss = nullptr;
    m_flag = nullptr;
    m_aura = nullptr;
    m_glow = nullptr;
    m_state = tmpl->m_defaultState;
    m_lastState = tmpl->m_defaultState;
    m_destroyed = false;
    m_homeNPC = nullptr;
    m_spiritGuide = nullptr;

    // set state
    ChangeState(m_state);

    // spawn initial guards (if we have any)
    if (!m_template->m_capturable && m_template->m_initialSpawnId)
    {
        // then we are probably a tower.
        const AVSpawnLocation* spi = g_initalGuardLocations[nodeid];
        CreatureProperties const* cp = sMySQLStore.getCreatureProperties(m_template->m_initialSpawnId);
        if (cp == nullptr)
        {
            DLLLogDetail("AlteracValley : Invalid creature entry {}!", m_template->m_initialSpawnId);
            return;
        }
        Creature* sp;
        DLLLogDetail("AlteracValley : spawning guards at bunker {} of {} ({})", m_template->m_name, cp->Name, cp->Id);

        while (spi->x != 0.0f)
        {
            sp = m_bg->getWorldMap()->getSpawnManager().createCreature(
                cp->Id, LocationVector(spi->x, spi->y, spi->z, spi->o));
            if (sp != nullptr)
                sp->PushToWorld(m_bg->getWorldMap());
            ++spi;
        }
    }

    // spawn initial home npc
    if (g_HomeNpcInfo[m_nodeId].id_a != 0)
    {
        // spawn alliance npcs if its a horde tower
        if (m_template->m_defaultState == AV_NODE_STATE_ALLIANCE_CONTROLLED)
        {
            m_homeNPC = m_bg->spawnCreature(g_HomeNpcInfo[m_nodeId].id_a, g_HomeNpcInfo[m_nodeId].a_x, g_HomeNpcInfo[m_nodeId].a_y,
                g_HomeNpcInfo[m_nodeId].a_z, g_HomeNpcInfo[m_nodeId].a_o);
        }
        else
        {
            m_homeNPC = m_bg->spawnCreature(g_HomeNpcInfo[m_nodeId].id_h, g_HomeNpcInfo[m_nodeId].h_x, g_HomeNpcInfo[m_nodeId].h_y,
                g_HomeNpcInfo[m_nodeId].h_z, g_HomeNpcInfo[m_nodeId].h_o);
        }
    }
}

AlteracValley::AVNode::~AVNode()
{
}

void AlteracValley::AVNode::Assault(Player* plr)
{
    // player assaulted the control point.
    // safety check
    DLLLogDetail("AlteracValley : AVNode::Assault({}) : entering", m_template->m_name);
    if (plr->isTeamAlliance() && (m_state == AV_NODE_STATE_ALLIANCE_ASSAULTING || m_state == AV_NODE_STATE_ALLIANCE_CONTROLLED))
        return;

    if (plr->isTeamHorde() && (m_state == AV_NODE_STATE_HORDE_ASSAULTING || m_state == AV_NODE_STATE_HORDE_CONTROLLED))
        return;

    if (m_destroyed)
        return;

    // are we returning the node to us?
    if ((plr->isTeamAlliance() && m_lastState == AV_NODE_STATE_ALLIANCE_CONTROLLED) ||
        (plr->isTeamHorde() && m_lastState == AV_NODE_STATE_HORDE_CONTROLLED))
    {
        // set the state for capture
        m_state = plr->isTeamAlliance() ? AV_NODE_STATE_ALLIANCE_ASSAULTING : AV_NODE_STATE_HORDE_ASSAULTING;

        // no timer delay
        Capture();

        // pvp data
        if (m_template->m_isGraveyard)
            plr->m_bgScore.MiscData[BattlegroundDef::AV_GRAVEYARDS_DEFENDED]++;
        else
            plr->m_bgScore.MiscData[BattlegroundDef::AV_TOWERS_DEFENDED]++;

        return;
    }

    // start timer
    sEventMgr.RemoveEvents(m_bg, EVENT_AV_CAPTURE_CP_0 + m_nodeId);

    sEventMgr.AddEvent(m_bg, &AlteracValley::EventAssaultControlPoint, m_nodeId, EVENT_AV_CAPTURE_CP_0 + m_nodeId, TimeVars::Minute * 4 * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    // update state
    ChangeState(plr->isTeamHorde() ? AV_NODE_STATE_HORDE_ASSAULTING : AV_NODE_STATE_ALLIANCE_ASSAULTING);

    // pvp data
    if (m_template->m_isGraveyard)
    {
        // send message
        const uint8_t chatMsgType = static_cast<uint8_t>(CHAT_MSG_BG_EVENT_ALLIANCE) + static_cast<uint8_t>(plr->getTeam());
        m_bg->sendChatMessage(chatMsgType, 0, "{} claims the {}! If left unchallenged, the {} will control it!", plr->getName(), m_template->m_name, plr->isTeamHorde() ? "Horde" : "Alliance");

        plr->m_bgScore.MiscData[BattlegroundDef::AV_GRAVEYARDS_ASSAULTED]++;
    }
    else
    {
        m_bg->sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "{} is under attack! If left unchecked the {} will destroy it!", m_template->m_name, plr->isTeamHorde() ? "Horde" : "Alliance");
        plr->m_bgScore.MiscData[BattlegroundDef::AV_TOWERS_ASSAULTED]++;
    }
}

void AlteracValley::AVNode::Spawn()
{
    // spawn control point (if we should have one)
    DLLLogDetail("AlteracValley : AVNode::Spawn for {}, state = {} {}, old_state = {} {}", m_template->m_name, m_state, g_stateNames[m_state], m_lastState, g_stateNames[m_lastState]);
    if (m_template->m_flagLocation.id[m_state] == 0)
    {
        // no flag should be spawned, despawn if one exists
        if (m_flag != nullptr)
        {
            DLLLogDetail("AlteracValley : AVNode::Spawn({}) : Despawning main flag", m_template->m_name);
            m_flag->despawn(0, 0);
            m_flag = nullptr;
        }
    }
    else
    {
        // spawn the flag.
        const AVGameObject* g = &m_template->m_flagLocation;
        DLLLogDetail("AlteracValley : AVNode::Spawn({}) : Spawning main flag", m_template->m_name);
        if (m_flag == nullptr)
        {
            // initial spawn
            m_flag = m_bg->createGameObject(g->id[m_state], LocationVector(g->x, g->y, g->z, g->o), 0, 0, 1.0f);
            m_flag->SetFaction(g_gameObjectFactions[m_state]);
            m_flag->setAnimationProgress(100);
            m_flag->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
            m_flag->PushToWorld(m_bg->getWorldMap());
        }
        else
        {
            // change entry, but to do this change guid
            if (m_flag->getEntry() != g->id[m_state] || !m_flag->IsInWorld())
            {
                auto gameobject_info = sMySQLStore.getGameObjectProperties(g->id[m_state]);
                m_flag->RemoveFromWorld(false);
                m_flag->setEntry(g->id[m_state]);
                m_bg->getWorldMap()->getSpawnManager().regenerateGameObjectGuid(m_flag, g->id[m_state]);
                m_flag->SetGameObjectProperties(gameobject_info);
                m_flag->setDisplayId(gameobject_info->display_id);
                m_flag->setGoType(static_cast<uint8_t>(gameobject_info->type));
                m_flag->SetFaction(g_gameObjectFactions[m_state]);
                m_flag->setAnimationProgress(100);
                m_flag->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
                m_flag->PushToWorld(m_bg->getWorldMap());
            }
        }
    }

    // spawn secondary control node aura (if there should be one)
    if (m_template->m_auraLocation.id[m_state] == 0)
    {
        // no flag should be spawned, despawn if one exists
        if (m_aura != nullptr)
        {
            DLLLogDetail("AlteracValley : AVNode::Spawn({}) : Despawning secondary flag", m_template->m_name);
            m_aura->despawn(0, 0);
            m_aura = nullptr;
        }
    }
    else
    {
        // spawn the flag.
        const AVGameObject* g = &m_template->m_auraLocation;
        DLLLogDetail("AlteracValley : AVNode::Spawn({}) : Spawning secondary flag", m_template->m_name);
        if (m_aura == nullptr)
        {
            // initial spawn
            m_aura = m_bg->createGameObject(g->id[m_state], LocationVector(g->x, g->y, g->z, g->o), 0, 0, 3.0f);
            m_aura->SetFaction(g_gameObjectFactions[m_state]);
            m_aura->setAnimationProgress(100);
            m_aura->setFlags(GO_FLAG_NONSELECTABLE);
            m_aura->setState(GO_STATE_CLOSED);
            m_aura->PushToWorld(m_bg->getWorldMap());
        }
        else
        {
            // change entry, but to do this change guid
            if (m_aura->getEntry() != g->id[m_state] || !m_aura->IsInWorld())
            {
                auto gameobject_info = sMySQLStore.getGameObjectProperties(g->id[m_state]);
                m_aura->RemoveFromWorld(false);
                m_aura->setEntry(g->id[m_state]);
                m_bg->getWorldMap()->getSpawnManager().regenerateGameObjectGuid(m_aura, g->id[m_state]);
                m_aura->SetGameObjectProperties(gameobject_info);
                m_aura->setDisplayId(gameobject_info->display_id);
                m_aura->setGoType(static_cast<uint8_t>(gameobject_info->type));
                m_aura->SetFaction(g_gameObjectFactions[m_state]);
                m_aura->setAnimationProgress(100);
                m_aura->setFlags(GO_FLAG_NONSELECTABLE);
                m_aura->setState(GO_STATE_CLOSED);
                m_aura->PushToWorld(m_bg->getWorldMap());
            }
        }
    }

    // spawn glow
    if (m_template->m_glowLocation.id[m_state] == 0)
    {
        // no flag should be spawned, despawn if one exists
        if (m_glow != nullptr)
        {
            DLLLogDetail("AlteracValley : AVNode::Spawn({}) : Despawning glow", m_template->m_name);
            m_glow->despawn(0, 0);
            m_glow = nullptr;
        }
    }
    else
    {
        // spawn the flag.
        const AVGameObject* g = &m_template->m_glowLocation;
        DLLLogDetail("AlteracValley : AVNode::Spawn({}) : Spawning glow", m_template->m_name);
        if (m_glow == nullptr)
        {
            // initial spawn
            m_glow = m_bg->createGameObject(g->id[m_state], LocationVector(g->x, g->y, g->z, g->o), 0, 0, 1.0f);
            m_glow->SetFaction(g_gameObjectFactions[m_state]);
            m_glow->setAnimationProgress(100);
            m_glow->setFlags(GO_FLAG_NONSELECTABLE);
            m_glow->setState(GO_STATE_CLOSED);
            if (m_glow->getEntry() == 180422 || m_glow->getEntry() == 180423)
                m_glow->setScale(10.0f);
            else
                m_glow->setScale(2.0f);
            m_glow->PushToWorld(m_bg->getWorldMap());
        }
        else
        {
            // change entry, but to do this change guid
            if (m_glow->getEntry() != g->id[m_state] || !m_glow->IsInWorld())
            {
                auto gameobject_info = sMySQLStore.getGameObjectProperties(g->id[m_state]);
                m_glow->RemoveFromWorld(false);
                m_glow->setEntry(g->id[m_state]);
                m_bg->getWorldMap()->getSpawnManager().regenerateGameObjectGuid(m_glow, g->id[m_state]);
                m_glow->SetGameObjectProperties(gameobject_info);
                m_glow->setDisplayId(gameobject_info->display_id);
                m_glow->setGoType(static_cast<uint8_t>(gameobject_info->type));
                m_glow->SetFaction(g_gameObjectFactions[m_state]);
                m_glow->setAnimationProgress(100);
                m_glow->setFlags(GO_FLAG_NONSELECTABLE);
                m_glow->setState(GO_STATE_CLOSED);
                if (m_glow->getEntry() == 180422 || m_glow->getEntry() == 180423)
                    m_glow->setScale(10.0f);
                else
                    m_glow->setScale(2.0f);
                m_glow->PushToWorld(m_bg->getWorldMap());
            }
        }
    }

    // update field states :O
    if (m_template->m_worldStateFields[m_lastState] != 0)
        m_bg->setWorldState(m_template->m_worldStateFields[m_lastState], 0);

    if (m_template->m_worldStateFields[m_state] != 0)
        m_bg->setWorldState(m_template->m_worldStateFields[m_state], 1);

    // sch: spawn/despawn guards according to state (delegated to SpawnGuards)
    if (g_stateToGuardType[m_state] > 0)
        SpawnGuards(static_cast<uint32_t>(g_stateToGuardType[m_state]));

    // spirit guides
    if (m_template->m_isGraveyard)
    {
        // should one be spawned
        if (m_spiritGuide != nullptr)
        {
            DLLLogDetail("AlteracValley : AVNode::Spawn({}) : despawning spirit guide", m_template->m_name);
            // move everyone in the revive queue to a different node
            std::map<Creature*, std::set<WoWGuid>>::iterator itr = m_bg->m_resurrectMap.find(m_spiritGuide);
            if (itr != m_bg->m_resurrectMap.end())
            {
                for (std::set<WoWGuid>::iterator it2 = itr->second.begin(); it2 != itr->second.end(); ++it2)
                {
                    // repop him at a new GY
                    Player* plr_tmp = m_bg->getWorldMap()->getPlayer(*it2);
                    if (plr_tmp != nullptr)
                    {
                        m_bg->HookHandleRepop(plr_tmp);
                        m_bg->queueAtNearestSpiritGuide(plr_tmp, m_spiritGuide);
                    }
                }
                itr->second.clear();
            }
            m_bg->removeSpiritGuide(m_spiritGuide);
            m_spiritGuide->Despawn(0, 0);
            m_spiritGuide = nullptr;
        }

        if (m_state == AV_NODE_STATE_ALLIANCE_CONTROLLED)
        {
            DLLLogDetail("AlteracValley : AVNode::Spawn({}) : spawning spirit guide", m_template->m_name);

            // spawn new spirit guide
            m_spiritGuide = m_bg->spawnSpiritGuide(m_template->m_graveyardLocation.x, m_template->m_graveyardLocation.y,
                m_template->m_graveyardLocation.z, m_template->m_graveyardLocation.o, 0);

            // add
            m_bg->addSpiritGuide(m_spiritGuide);
        }
        else if (m_state == AV_NODE_STATE_HORDE_CONTROLLED)
        {
            DLLLogDetail("AlteracValley : AVNode::Spawn({}) : spawning spirit guide", m_template->m_name);

            // spawn new spirit guide
            m_spiritGuide = m_bg->spawnSpiritGuide(m_template->m_graveyardLocation.x, m_template->m_graveyardLocation.y,
                m_template->m_graveyardLocation.z, m_template->m_graveyardLocation.o, 1);

            // add
            m_bg->addSpiritGuide(m_spiritGuide);
        }
    }
    DLLLogDetail("AlteracValley : AVNode::Spawn({}) : completed for state {} {}", m_template->m_name, m_state, g_stateNames[m_state]);
}

void AlteracValley::AVNode::SpawnGuards(uint32_t guardType)
{
    // sch: guardType is 1 for Alliance and 2 for Horde. Neutral and assault states
    // must not call this method.
    if (guardType < 1 || guardType > 2)
        return;

    // sch: remove guards belonging to the previous owner before creating the new set.
    DLLLogDetail("AlteracValley : AVNode::SpawnGuards({}) : despawning {} guards", m_template->m_name, m_guards.size());
    for (Creature* guard : m_guards)
    {
        if (guard != nullptr)
            guard->Despawn(0, 0);
    }
    m_guards.clear();

    const uint32_t guardEntry = m_template->m_guardId[guardType];
    if (guardEntry == 0 || m_template->m_guardCount == 0)
    {
        DLLLogDetail("AlteracValley : AVNode::SpawnGuards({}) : no dynamic guards configured for team {}", m_template->m_name, guardType);
        return;
    }

    DLLLogDetail("AlteracValley : AVNode::SpawnGuards({}) : spawning {} guards of entry {} for team {}", m_template->m_name, m_template->m_guardCount, guardEntry, guardType);

    for (uint32_t i = 0; i < m_template->m_guardCount; ++i)
    {
        // uniform random angle + radius for circular distribution around flag
        const float angle = Util::getRandomFloat(6.28318530718f); // 2*pi
        const float radius = Util::getRandomFloat(10.0f); // up to 10 units
        const float x = m_template->m_flagLocation.x + radius * cos(angle);
        const float y = m_template->m_flagLocation.y + radius * sin(angle);
        const float z = m_bg->getWorldMap()->getHeight(LocationVector(x, y, m_template->m_flagLocation.z));

        Creature* guard = m_bg->spawnCreature(guardEntry, x, y, z, 0.0f);
        if (guard != nullptr)
            m_guards.push_back(guard);
    }
}

void AlteracValley::AVNode::ChangeState(uint32_t new_state)
{
    DLLLogDetail("AlteracValley : AVNode::ChangeState({}) : changing to state {} {}, old state was {} {}", m_template->m_name, new_state, g_stateNames[new_state], m_state, g_stateNames[m_state]);
    m_lastState = m_state;
    m_state = new_state;

    // update spawns
    Spawn();
}

void AlteracValley::AVNode::Capture()
{
    if (m_destroyed)
        return;

    DLLLogDetail("AlteracValley : AVNode::Capture({}) : entering", m_template->m_name);

    // sooo easy
    sEventMgr.RemoveEvents(m_bg, EVENT_AV_CAPTURE_CP_0 + m_nodeId);
    if (m_state == AV_NODE_STATE_HORDE_ASSAULTING)
        ChangeState(AV_NODE_STATE_HORDE_CONTROLLED);
    else
        ChangeState(AV_NODE_STATE_ALLIANCE_CONTROLLED);

    // were we destroyed?
    if (!m_template->m_capturable)
    {
        if (m_state != m_template->m_defaultState)
        {
            // spawn fire!
            const AVSpawnLocation* spi = g_fireLocations[m_nodeId];
            GameObject* go;

            DLLLogDetail("AlteracValley : spawning fires at bunker {}", m_template->m_name);
            while (spi->x != 0.0f)
            {
                go = m_bg->createGameObject(AV_GAMEOBJECT_FIRE, LocationVector(spi->x, spi->y, spi->z, spi->o), 0, 35, 1.0f);
                go->PushToWorld(m_bg->getWorldMap());
                ++spi;
            }

            // disable the flag
            if (m_flag != nullptr)
            {
                m_flag->setFlags(GO_FLAG_NONSELECTABLE);
                m_flag->setDynamicFlags(GO_DYN_FLAG_NONE);
                m_flag->setState(GO_STATE_CLOSED);
            }

            // prevent further actions
            m_destroyed = true;

            // send message
            m_bg->sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The {} has destroyed the {}.", (m_template->m_defaultState == AV_NODE_STATE_ALLIANCE_CONTROLLED) ? "Horde" : "Alliance", m_template->m_name);

            if (m_template->m_defaultState == AV_NODE_STATE_ALLIANCE_CONTROLLED)
            {
                for (std::set<Player*>::iterator itx = m_bg->m_players[1].begin(); itx != m_bg->m_players[1].end(); ++itx)
                {
                    Player* plr = (*itx);
                    if (!plr) continue;

                    HonorHandler::AddHonorPointsToPlayer(plr, 62);
                    HonorHandler::AddBattlegroundObjectiveXp(plr, 3);
                }
            }
            else if (m_template->m_defaultState == AV_NODE_STATE_HORDE_CONTROLLED)
            {
                for (std::set<Player*>::iterator itx = m_bg->m_players[0].begin(); itx != m_bg->m_players[0].end(); ++itx)
                {
                    Player* plr = (*itx);
                    if (!plr) continue;

                    HonorHandler::AddHonorPointsToPlayer(plr, 62);
                    HonorHandler::AddBattlegroundObjectiveXp(plr, 3);
                }
            }

            // the npc at our home has to change.
            if (m_homeNPC != nullptr)
                m_homeNPC->Despawn(0, 0);

            // respawn if we have one
            if (g_HomeNpcInfo[m_nodeId].id_a != 0)
            {
                // spawn alliance npcs if its a horde tower
                if (m_template->m_defaultState == AV_NODE_STATE_HORDE_CONTROLLED)
                {
                    m_homeNPC = m_bg->spawnCreature(g_HomeNpcInfo[m_nodeId].id_a, g_HomeNpcInfo[m_nodeId].a_x, g_HomeNpcInfo[m_nodeId].a_y,
                        g_HomeNpcInfo[m_nodeId].a_z, g_HomeNpcInfo[m_nodeId].a_o);
                }
                else
                {
                    m_homeNPC = m_bg->spawnCreature(g_HomeNpcInfo[m_nodeId].id_h, g_HomeNpcInfo[m_nodeId].h_x, g_HomeNpcInfo[m_nodeId].h_y,
                        g_HomeNpcInfo[m_nodeId].h_z, g_HomeNpcInfo[m_nodeId].h_o);
                }
            }

            // lose 75 reinforcements
            if (m_template->m_defaultState == AV_NODE_STATE_ALLIANCE_CONTROLLED)
                m_bg->RemoveReinforcements(0, AV_POINTS_ON_DESTROY_BUNKER);
            else
                m_bg->RemoveReinforcements(1, AV_POINTS_ON_DESTROY_BUNKER);
        }
        else
        {
            // saved the tower
            m_bg->sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The {} has taken the {}.", (m_state == AV_NODE_STATE_ALLIANCE_CONTROLLED) ? "Alliance" : "Horde", m_template->m_name);
        }
    }
    else
    {
        // captured message
        m_bg->sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The {} has taken the {}.", (m_state == AV_NODE_STATE_ALLIANCE_CONTROLLED) ? "Alliance" : "Horde", m_template->m_name);
    }
}

AlteracValley::AlteracValley(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t) : Battleground(mgr, id, lgroup, t)
{
    m_playerCountPerTeam = 40;
    m_reinforcements[0] = AV_NUM_REINFORCEMENTS;
    m_reinforcements[1] = AV_NUM_REINFORCEMENTS;
    m_zoneId = 2597;

    memset(m_nodes, 0, sizeof(m_nodes));
}

AlteracValley::~AlteracValley()
{
    for (uint8_t x = 0; x < AV_NUM_CONTROL_POINTS; ++x)
        delete m_nodes[x];
}

bool AlteracValley::HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* /*pSpell*/)
{
    uint32_t nodeid = 0;

    for (nodeid = 0; nodeid < AV_NUM_CONTROL_POINTS; nodeid++)
    {
        if (m_nodes[nodeid] == nullptr)
            continue;

        if (m_nodes[nodeid]->m_flag == nullptr)
            continue;

        if (m_nodes[nodeid]->m_flag->getGuid() == pGo->getGuid())
            break;
    }

    if (nodeid == AV_NUM_CONTROL_POINTS)
        return false;

    m_nodes[nodeid]->Assault(pPlayer);

    return true;
}

void AlteracValley::HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/)
{

}

void AlteracValley::HookFlagStand(Player* /*plr*/, GameObject* /*obj*/)
{

}

void AlteracValley::HookOnMount(Player* /*plr*/)
{

}

void AlteracValley::HookOnAreaTrigger(Player* plr, uint32_t trigger)
{
    switch (trigger)
    {
        case 95:
        case 2608: // alliance exits
        {
            if (plr->getTeam() != TEAM_ALLIANCE)
                plr->sendAreaTriggerMessage("Only The Alliance can use that portal");
            else
                removePlayer(plr, false);
        }break;
        case 2606: // horde exits
        {
            if (plr->getTeam() != TEAM_HORDE)
                plr->sendAreaTriggerMessage("Only The Horde can use that portal");
            else
                removePlayer(plr, false);
        }break;
        case 3326:
        case 3327:
        case 3328:
        case 3329:
        case 3330:
        case 3331:
            //unmount
            break;
        default:
            DLLLogDetail("Encountered unhandled areatrigger id {}", trigger);
            break;
    }
}

bool AlteracValley::HookHandleRepop(Player* plr)
{
    float dist = 999999.0f;
    float dt;
    LocationVector dest_pos;
    if (plr->isTeamHorde())
        dest_pos.changeCoords({ -1433.550903f, -608.329529f, 51.149689f });
    else
        dest_pos.changeCoords({ 876.434448f, -489.599579f, 96.517174f });

    if (m_hasStarted)
    {
        for (uint8_t x = 0; x < AV_NUM_CONTROL_POINTS; ++x)
        {
            // skip non-graveyards
            if (!m_nodes[x]->m_template->m_isGraveyard)
                continue;

            // make sure they're owned by us
            if ((plr->isTeamAlliance() && m_nodes[x]->m_state == AV_NODE_STATE_ALLIANCE_CONTROLLED) ||
                (plr->isTeamHorde() && m_nodes[x]->m_state == AV_NODE_STATE_HORDE_CONTROLLED))
            {
                dt = plr->GetPositionNC().distance2DSq({ m_nodes[x]->m_template->m_graveyardLocation.x, m_nodes[x]->m_template->m_graveyardLocation.y });
                if (dt < dist)
                {
                    // new one
                    dest_pos.changeCoords({ m_nodes[x]->m_template->m_graveyardLocation.x, m_nodes[x]->m_template->m_graveyardLocation.y, m_nodes[x]->m_template->m_graveyardLocation.z });
                    dist = dt;
                }
            }
        }
    }

    // port to it
    plr->safeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest_pos);
    return false;
}

void AlteracValley::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    updatePvPData();
}

void AlteracValley::DropFlag(Player* /*plr*/)
{

}

void AlteracValley::OnCreate()
{
    // Alliance Gate
    GameObject* gate = createGameObject(AV_GAMEOBJECT_GATE, LocationVector(780.487f, -493.024f, 99.9553f, 3.0976f), 32, 114, 3.000000f);
    gate->setLocalRotation(0.f, 0.f, 0.0129570f, -0.0602880f);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    // Horde gate
    gate = createGameObject(AV_GAMEOBJECT_GATE, LocationVector(-1375.73f, -538.966f, 55.3006f, 0.791198f), 32, 114, 3.000000f);
    gate->setLocalRotation(0.f, 0.f, 0.36f, 0.922766f);
    gate->PushToWorld(m_mapMgr);
    m_gates.push_back(gate);

    for (uint8_t x = 0; x < AV_NUM_CONTROL_POINTS; ++x)
        m_nodes[x] = new AVNode(this, &g_nodeTemplates[x], x);

    // generals/leaders!
    spawnCreature(AV_NPC_GENERAL_VANNDAR_STORMPIKE, 726.969604f, -9.716300f, 50.621391f, 3.377580f);
    spawnCreature(AV_NPC_GENERAL_DREK_THAR, -1367.080933f, -229.453140f, 98.421570f, 2.023553f);

    // some captains
    spawnCreature(AV_NPC_CAPTAIN_BALINDA_STONEHEARTH, -57.368469f, -286.770966f, 15.564562f, 6.068771f);
    spawnCreature(AV_NPC_CAPTAIN_GALVANGAR, -537.177429f, -168.004944f, 57.008938f, 2.749793f);

    struct AVLegacyCreatureSpawn
    {
        uint32_t entry;
        float x, y, z, o;
        uint32_t displayId;
        uint32_t faction;
    };

    static constexpr AVLegacyCreatureSpawn legacyCreatureSpawns[] =
    {
        { 603, -1584.000000f, -402.081000f, 79.561300f, 3.404700f, 10792, 14 },
        { 2344, -1300.000000f, -1193.690000f, 41.750000f, 3.980000f, 3763, 88 },
        { 10696, -1232.000000f, -2572.910000f, 27.380000f, 5.220000f, 4137, 11 },
        { 10981, -1139.000000f, -630.510000f, 60.340000f, 5.140000f, 10278, 1214 },
        { 12122, -1380.000000f, -211.844000f, 99.370100f, 5.350040f, 10278, 1214 },
        { 12127, 656.000000f, -22.392600f, 49.561800f, 3.359800f, 13253, 1216 },
        { 12127, 717.000000f, -0.564321f, 50.621300f, 3.637490f, 13253, 1216 },
        { 12127, 624.000000f, -232.804000f, 37.472700f, 0.151753f, 13253, 1216 },
        { 12127, 637.000000f, -230.508000f, 37.508000f, 3.314790f, 13253, 1216 },
        { 12127, 627.000000f, -272.228000f, 30.133600f, 4.895510f, 13253, 1216 },
        { 12127, 660.000000f, -36.637900f, 49.548700f, 3.333930f, 13253, 1216 },
        { 12127, 646.000000f, -102.444000f, 49.582000f, 2.967780f, 13253, 1216 },
        { 12127, 647.000000f, -108.819000f, 49.575400f, 2.847640f, 13253, 1216 },
        { 12127, 723.000000f, -21.537100f, 50.621300f, 3.471030f, 13253, 1216 },
        { 12127, 645.000000f, -270.983000f, 30.132900f, 4.854660f, 13253, 1216 },
        { 12127, 618.000000f, -191.652000f, 38.715000f, 0.171264f, 13253, 1216 },
        { 13080, 874.000000f, -347.491000f, 65.300100f, 3.489520f, 13311, 1216 },
        { 13082, 933.000000f, -361.370000f, 66.145100f, 2.377400f, 4016, 32 },
        { 13446, -157.000000f, -437.912000f, 33.279600f, 4.656720f, 13662, 1216 },
        { 14284, 817.000000f, -490.789000f, 100.416000f, 5.150740f, 14322, 1216 },
        { 14284, 930.000000f, -521.183000f, 93.683400f, 2.803640f, 14322, 1216 },
        { 14284, 867.000000f, -508.146000f, 96.523500f, 2.815420f, 14322, 1216 },
        { 14285, -877.000000f, -558.075000f, 57.135700f, 1.697720f, 14320, 1214 },
        { 14285, -878.000000f, -535.248000f, 55.011700f, 2.675540f, 14320, 1214 },
        { 14285, -889.000000f, -543.776000f, 55.660400f, 1.843020f, 14320, 1214 },
        { 14285, -872.000000f, -548.455000f, 57.472200f, 3.001480f, 14320, 1214 },
        { 22760, -1207.000000f, -253.251000f, 72.636900f, 5.662720f, 13402, 35 },
        { 13331, 704.394000f, -38.076000f, 50.620100f, 1.888830f, 13258, 1216 },
        { 14284, 778.840000f, -496.352000f, 100.050000f, 2.887840f, 14322, 1216 },
        { 14284, 935.655000f, -506.179000f, 93.970500f, 6.007430f, 14324, 1216 },
        { 14284, 927.280000f, -512.103000f, 93.556200f, 4.627480f, 14324, 1216 },
        { 10990, 373.362000f, -211.642000f, -27.320900f, 2.963220f, 13340, 1216 },
        { 13676, 597.492000f, 2.920820f, 41.995500f, 5.130910f, 13340, 32 },
        { 13676, 592.818000f, 0.386293f, 41.995500f, 5.818140f, 13340, 32 },
        { 13676, 593.909000f, -5.105330f, 41.995500f, 0.320360f, 13340, 32 },
        { 13676, 603.449000f, 4.280150f, 41.995500f, 4.180590f, 13340, 32 },
        { 13576, 659.224000f, -287.451000f, 29.987100f, 2.935740f, 13709, 1216 },
        { 13576, 660.876000f, -290.322000f, 30.046100f, 2.814000f, 13709, 1216 },
        { 11604, -875.396000f, -110.514000f, 65.379500f, 6.162960f, 371, 14 },
        { 14285, -759.694000f, -672.978000f, 52.479700f, 4.004660f, 14321, 1214 },
        { 13439, -1196.740000f, -316.610000f, 57.127300f, 4.895320f, 13472, 1216 },
        { 13437, -1305.460000f, -277.686000f, 107.577000f, 0.649455f, 13470, 1216 },
        { 11605, -877.606000f, -118.166000f, 65.054100f, 0.544185f, 371, 14 },
        { 13086, -823.941000f, -63.064800f, 72.051400f, 3.194900f, 13298, 1216 },
        { 11603, -864.691000f, -119.627000f, 64.923000f, 2.663980f, 371, 14 },
        { 11677, -875.337000f, -116.294000f, 65.008300f, 5.738820f, 371, 14 },
        { 13419, -224.724000f, -336.261000f, 9.411460f, 2.766090f, 14884, 79 },
        { 13327, -132.195000f, -339.019000f, 10.876200f, 2.707170f, 13229, 11 },
        { 13327, -124.359000f, -321.222000f, 11.315800f, 2.715020f, 13229, 11 },
        { 13327, -133.022000f, -344.395000f, 10.672600f, 2.872100f, 13229, 11 },
        { 13327, -115.117000f, -334.872000f, 11.023100f, 2.954570f, 13229, 11 },
        { 13326, -269.568000f, -403.440000f, 18.215200f, 1.678290f, 13263, 1216 },
        { 13326, -252.892000f, -412.017000f, 17.675400f, 1.894280f, 13263, 1216 },
        { 13326, -163.655000f, -443.332000f, 40.403400f, 2.411860f, 13262, 1216 },
        { 13326, -258.208000f, -417.328000f, 17.594400f, 1.944550f, 13262, 1216 },
        { 13335, 199.835000f, -366.418000f, 56.391500f, 3.154050f, 13270, 32 },
        { 13335, 202.744000f, -376.652000f, 56.391500f, 3.790220f, 13270, 32 },
        { 13325, 82.357000f, -401.931000f, 46.859000f, 1.814940f, 13266, 55 },
        { 13325, 76.453700f, -404.947000f, 46.693100f, 1.983800f, 13266, 55 },
        { 13325, 79.196800f, -411.959000f, 48.849500f, 1.583250f, 13266, 55 },
        { 13428, -345.407000f, -223.561000f, 12.031100f, 6.107090f, 13365, 29 },
        { 13428, -340.612000f, -209.647000f, 11.959100f, 5.773300f, 13365, 29 },
        { 13428, -349.338000f, -212.912000f, 11.994400f, 5.828270f, 13365, 29 },
        { 12052, -380.283000f, -350.126000f, 17.561800f, 0.067366f, 12081, 1214 },
        { 12052, -389.900000f, -338.912000f, 17.516500f, 0.291204f, 12081, 1214 },
        { 12052, -373.829000f, -338.162000f, 17.912300f, 0.444356f, 12081, 1214 },
        { 12052, -387.005000f, -356.491000f, 17.575800f, 6.232740f, 12081, 1214 },
        { 12052, -578.767000f, -269.987000f, 74.945100f, 4.991810f, 12081, 1214 },
        { 13140, -567.432000f, -267.254000f, 74.977400f, 5.466980f, 13462, 1214 },
        { 12052, -611.611000f, -393.206000f, 60.844100f, 2.277540f, 12081, 1214 },
        { 12052, -615.997000f, -399.060000f, 60.457700f, 3.365320f, 12081, 1214 },
        { 12052, -610.651000f, -402.909000f, 60.374900f, 4.947900f, 12081, 1214 },
        { 12052, -738.124000f, -384.589000f, 76.486200f, 0.321898f, 12081, 1214 },
        { 12052, -748.118000f, -395.505000f, 76.629400f, 5.968910f, 12081, 1214 },
        { 12052, -759.633000f, -357.885000f, 90.872000f, 0.467193f, 12081, 1214 },
        { 12052, -737.483000f, -318.905000f, 70.053800f, 6.212380f, 12081, 1214 },
        { 12052, -747.103000f, -322.058000f, 70.020700f, 5.949270f, 12081, 1214 },
        { 13756, -202.467000f, -113.246000f, 78.869500f, 1.631940f, 1141, 32 },
        { 13796, -849.603000f, -92.855300f, 68.506500f, 4.272440f, 1141, 32 },
        { 13796, 876.882000f, -444.261000f, 54.612200f, 2.540640f, 1141, 32 },
        { 13778, -160.034000f, -459.994000f, 40.384600f, 1.279290f, 1141, 1216 },
        { 13778, -574.742000f, -260.625000f, 75.008700f, 5.567580f, 1141, 1214 },
        { 14029, -1307.270000f, -277.884000f, 107.553000f, 0.769577f, 7804, 32 },
        { 14030, 574.656000f, -36.096500f, 37.721500f, 4.220610f, 7804, 32 },
        { 14031, -1196.900000f, -313.479000f, 57.557200f, 4.927470f, 7804, 32 },
        { 13256, -317.917000f, -209.097000f, 16.859200f, 5.840120f, 13174, 1214 },
        { 13179, 147.607000f, -328.224000f, 56.387100f, 4.526170f, 13473, 1214 },
        { 14026, 148.097000f, -326.244000f, 57.308700f, 4.414640f, 7804, 32 },
        { 14028, 366.901000f, -518.782000f, 71.808100f, 1.510250f, 7804, 32 },
        { 14027, -1342.070000f, -332.718000f, 91.201500f, 1.086140f, 7804, 32 },
    };

    for (const auto& spawn : legacyCreatureSpawns)
    {
        Creature* creature = spawnCreature(spawn.entry, spawn.x, spawn.y, spawn.z, spawn.o);
        if (creature == nullptr)
            continue;

        if (spawn.displayId != 0)
            creature->setDisplayId(spawn.displayId);
        if (spawn.faction != 0)
            creature->setFaction(spawn.faction);
    }

    struct AVLegacyGameObjectSpawn
    {
        uint32_t entry;
        float x, y, z, o;
        float rotation0, rotation1, rotation2, rotation3;
        float scale;
    };

    static constexpr AVLegacyGameObjectSpawn legacyGameObjectSpawns[] =
    {
        { 24746, 649.424000f, -59.096400f, 41.657200f, 3.701610f, 0.000000f, 0.000000f, 0.961053f, -0.276363f, 1.000000f },
        { 1748, 645.943000f, -57.247300f, 41.761300f, 3.608110f, 0.000000f, 0.000000f, 0.972919f, -0.231147f, 1.000000f },
        { 1748, -1252.930000f, -317.127000f, 62.611800f, 0.303152f, 0.000000f, 0.000000f, 0.150996f, 0.988534f, 1.000000f },
        { 34571, -1250.220000f, -310.042000f, 63.052500f, 4.846680f, 0.000000f, 0.000000f, 0.658069f, -0.752957f, 1.000000f },
        { 32882, -1279.840000f, -219.751000f, 72.584200f, 5.745960f, 0.000000f, 0.000000f, 0.265394f, -0.964140f, 1.000000f },
        { 110233, -1286.210000f, -184.348000f, 72.068400f, 1.469460f, 0.000000f, 0.000000f, 0.670387f, 0.742011f, 1.000000f },
        { 110233, -743.145000f, -397.896000f, 76.483400f, 5.655630f, 0.000000f, 0.000000f, 0.308654f, -0.951174f, 1.000000f },
        { 3817, 30.672700f, -428.376000f, 41.373100f, 4.290620f, 0.000000f, 0.000000f, 0.839456f, -0.543428f, 1.000000f },
        { 3817, 50.550800f, -421.329000f, 44.708200f, 2.619290f, 0.000000f, 0.000000f, 0.966094f, 0.258192f, 1.000000f },
        { 178806, 642.578000f, -54.715500f, 41.912400f, 1.295900f, 0.000000f, 0.000000f, 0.603554f, 0.797322f, 1.000000f },
        { 178806, 643.681000f, -55.015200f, 41.886000f, 1.543300f, 0.000000f, 0.000000f, 0.697320f, 0.716760f, 1.000000f },
        { 178806, 643.391000f, -55.823000f, 41.727400f, 1.398000f, 0.000000f, 0.000000f, 0.643454f, 0.765485f, 1.000000f },
        { 178806, 642.127000f, -55.856400f, 41.818400f, 1.416070f, 0.000000f, 0.000000f, 0.650342f, 0.759642f, 1.000000f },
        { 178806, 642.516000f, -54.735500f, 42.176900f, 1.341460f, 0.000000f, 0.000000f, 0.621556f, 0.783370f, 1.000000f },
        { 178935, 79.591400f, -406.301000f, 47.585500f, 2.256460f, 0.000000f, 0.000000f, 0.903655f, 0.428261f, 1.000000f },
        { 178940, -202.558000f, -112.577000f, 78.489100f, 1.676840f, 0.000000f, 0.000000f, 0.743587f, 0.668639f, 1.000000f },
        { 178943, -611.907000f, -396.359000f, 60.850800f, 3.521720f, 0.000000f, 0.000000f, 0.981992f, -0.188923f, 1.000000f },
        { 178909, -1258.700000f, -321.584000f, 62.271600f, 2.164550f, 0.000000f, 0.000000f, 0.883027f, 0.469322f, 1.000000f },
        { 178909, -1261.080000f, -323.418000f, 62.644500f, 3.829590f, 0.000000f, 0.000000f, 0.941413f, -0.337255f, 1.000000f },
        { 178909, -1258.720000f, -321.517000f, 63.397200f, 0.676218f, 0.000000f, 0.000000f, 0.331704f, 0.943384f, 1.000000f },
        { 178909, -1260.560000f, -319.483000f, 62.760500f, 2.254870f, 0.000000f, 0.000000f, 0.903315f, 0.428979f, 1.000000f },
        { 178909, -1260.940000f, -323.323000f, 63.770200f, 5.239370f, 0.000000f, 0.000000f, 0.498533f, -0.866871f, 1.000000f },
        { 179024, 50.190400f, -11.486700f, -4.096220f, 1.935860f, 0.000000f, 0.000000f, 0.823714f, 0.567005f, 1.000000f },
        { 179025, -1552.930000f, -360.928000f, 66.939700f, 3.710860f, 0.000000f, 0.000000f, 0.959765f, -0.280804f, 1.000000f },
        { 178785, 827.600000f, -323.704000f, 64.039500f, 0.650982f, 0.000000f, 0.000000f, 0.319774f, 0.947494f, 1.000000f },
        { 178785, 943.377000f, -385.359000f, 64.167400f, 1.188970f, 0.000000f, 0.000000f, 0.560083f, 0.828436f, 1.000000f },
        { 178785, 929.363000f, -369.254000f, 66.357800f, 2.084330f, 0.000000f, 0.000000f, 0.863498f, 0.504352f, 1.000000f },
        { 178785, 898.573000f, -473.403000f, 59.143000f, 3.886820f, 0.000000f, 0.000000f, 0.931379f, -0.364052f, 1.000000f },
        { 178785, 875.341000f, -414.489000f, 52.872400f, 1.644510f, 0.000000f, 0.000000f, 0.732682f, 0.680571f, 1.000000f },
        { 178785, 854.192000f, -438.814000f, 49.954800f, 3.529470f, 0.000000f, 0.000000f, 0.981253f, -0.192725f, 1.000000f },
        { 178785, 887.427000f, -441.768000f, 54.667800f, 5.983840f, 0.000000f, 0.000000f, 0.149114f, -0.988820f, 1.000000f },
        { 178785, 811.868000f, -461.475000f, 48.019000f, 2.162880f, 0.000000f, 0.000000f, 0.882635f, 0.470059f, 1.000000f },
        { 178785, 859.797000f, -366.986000f, 47.349900f, 0.175818f, 0.000000f, 0.000000f, 0.087796f, 0.996139f, 1.000000f },
        { 178785, 810.505000f, -325.827000f, 52.542400f, 0.596006f, 0.000000f, 0.000000f, 0.293612f, 0.955925f, 1.000000f },
        { 178785, 955.496000f, -439.145000f, 55.323400f, 1.008340f, 0.000000f, 0.000000f, 0.483081f, 0.875576f, 1.000000f },
        { 178784, -943.177000f, -183.263000f, 64.079900f, 0.572443f, 0.000000f, 0.000000f, 0.282329f, 0.959318f, 1.000000f },
        { 178784, -923.619000f, -126.165000f, 61.250400f, 1.734830f, 0.000000f, 0.000000f, 0.762661f, 0.646798f, 1.000000f },
        { 178784, -855.476000f, -85.696100f, 68.542900f, 1.247890f, 0.000000f, 0.000000f, 0.584240f, 0.811581f, 1.000000f },
        { 178784, -850.528000f, -119.829000f, 64.071300f, 5.426210f, 0.000000f, 0.000000f, 0.415498f, -0.909594f, 1.000000f },
        { 178784, -823.771000f, -67.550000f, 72.643000f, 5.952420f, 0.000000f, 0.000000f, 0.164629f, -0.986356f, 1.000000f },
        { 178784, -853.288000f, -23.480600f, 70.558300f, 5.767860f, 0.000000f, 0.000000f, 0.254822f, -0.966988f, 1.000000f },
        { 178784, -932.640000f, -58.001000f, 80.072600f, 3.592300f, 0.000000f, 0.000000f, 0.974715f, -0.223453f, 1.000000f },
        { 178784, -962.343000f, -168.729000f, 77.651000f, 3.529480f, 0.000000f, 0.000000f, 0.981252f, -0.192731f, 1.000000f },
        { 178784, -887.882000f, -110.300000f, 76.155300f, 0.164038f, 0.000000f, 0.000000f, 0.081927f, 0.996638f, 1.000000f },
        { 178784, -916.790000f, -148.447000f, 76.900700f, 3.254580f, 0.000000f, 0.000000f, 0.998405f, -0.056466f, 1.000000f },
        { 178784, -869.515000f, -19.012100f, 69.240100f, 3.211390f, 0.000000f, 0.000000f, 0.999391f, -0.034890f, 1.000000f },
        { 178784, -820.736000f, -155.585000f, 61.904000f, 4.593690f, 0.000000f, 0.000000f, 0.747804f, -0.663920f, 1.000000f },
        { 178664, 497.861000f, -521.355000f, 67.600000f, 3.200430f, 0.000000f, 0.000000f, 0.999567f, -0.029412f, 1.000000f },
        { 178665, -509.600000f, -359.731000f, 31.800000f, 1.206290f, 0.000000f, 0.000000f, 0.567234f, 0.823557f, 1.000000f },
    };

    for (const auto& spawn : legacyGameObjectSpawns)
    {
        GameObject* gameObject = createGameObject(spawn.entry, LocationVector(spawn.x, spawn.y, spawn.z, spawn.o), 0, 0, spawn.scale);
        if (gameObject == nullptr)
            continue;

        gameObject->setLocalRotation(spawn.rotation0, spawn.rotation1, spawn.rotation2, spawn.rotation3);
        gameObject->PushToWorld(m_mapMgr);
    }

    // home spirit guides
    addSpiritGuide(spawnSpiritGuide(876.434448f, -489.599579f, 96.517174f, 0.0f, 0));
    addSpiritGuide(spawnSpiritGuide(-1433.550903f, -608.329529f, 51.149689f, 0.0f, 1));
}

void AlteracValley::OnStart()
{
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            (*itr)->removeAllAurasById(BattlegroundDef::PREPARATION);
        }
    }

    // open gates
    for (std::list<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        (*itr)->setFlags(GO_FLAG_TRIGGERED);
        (*itr)->setState(GO_STATE_OPEN);
    }

    playSoundToAll(BattlegroundDef::BATTLEGROUND_BEGIN);

    m_hasStarted = true;

    sEventMgr.AddEvent(this, &AlteracValley::EventUpdateResources, EVENT_BATTLEGROUND_RESOURCEUPDATE, AV_REINFORCEMENT_ADD_INTERVAL, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void AlteracValley::OnAddPlayer(Player* plr)
{
    if (!m_hasStarted)
        plr->castSpell(plr, BattlegroundDef::PREPARATION, true);

    if (plr->isTeamHorde())
    {
        plr->setFactionAtWar(730, true);
        plr->setFactionStanding(730, -9000);
    }
    else
    {
        plr->setFactionAtWar(729, true);
        plr->setFactionStanding(729, -9000);
    }
}

void AlteracValley::OnRemovePlayer(Player* plr)
{
    plr->removeAllAurasById(BattlegroundDef::PREPARATION);
}

LocationVector AlteracValley::GetStartingCoords(uint32_t Team)
{
    if (Team)
        return LocationVector(-1433.550903f, -608.329529f, 51.149689f, 1.140702f);
    else
        return LocationVector(876.434448f, -489.599579f, 96.517174f, 3.222486f);
}

void AlteracValley::HookOnPlayerDeath(Player* plr)
{
    RemoveReinforcements(plr->getTeam(), AV_POINTS_ON_KILL);
}

void AlteracValley::AddReinforcements(uint32_t teamId, uint32_t amt)
{
    if (teamId > TEAM_HORDE || amt == 0 || hasEnded())
        return;

    m_reinforcements[teamId] = std::min(m_reinforcements[teamId] + amt, static_cast<uint32_t>(AV_NUM_REINFORCEMENTS));
    setWorldState(WORLDSTATE_AV_ALLIANCE_SCORE + teamId, m_reinforcements[teamId]);
}

void AlteracValley::RemoveReinforcements(uint32_t teamId, uint32_t amt)
{
    if (teamId > TEAM_HORDE || amt == 0 || hasEnded())
        return;

    m_reinforcements[teamId] = amt >= m_reinforcements[teamId] ? 0 : m_reinforcements[teamId] - amt;
    setWorldState(WORLDSTATE_AV_ALLIANCE_SCORE + teamId, m_reinforcements[teamId]);

    // We've lost. :(
    if (m_reinforcements[teamId] == 0)
    {
        Finish(teamId);
    }
}

void AlteracValley::HookOnPlayerKill(Player* plr, Player* pVictim)
{
    if (pVictim->isPlayer())
    {
        plr->m_bgScore.KillingBlows++;
        updatePvPData();
    }
}

void AlteracValley::HookOnUnitKill(Player* plr, Unit* pVictim)
{
    if (pVictim == nullptr || pVictim->isPlayer())
        return;

    // Mine bosses are neutral database spawns. Killing one transfers the mine
    // directly to the killer's faction; mines do not use the four-minute flag timer.
    if (plr != nullptr)
    {
        uint32_t mineNode = AV_NUM_CONTROL_POINTS;
        if (m_nodes[AV_CONTROL_POINT_COLDTOOTH_MINE] != nullptr &&
            pVictim->getEntry() == m_nodes[AV_CONTROL_POINT_COLDTOOTH_MINE]->m_template->m_bossId[2])
            mineNode = AV_CONTROL_POINT_COLDTOOTH_MINE;
        else if (m_nodes[AV_CONTROL_POINT_IRONDEEP_MINE] != nullptr &&
            pVictim->getEntry() == m_nodes[AV_CONTROL_POINT_IRONDEEP_MINE]->m_template->m_bossId[2])
            mineNode = AV_CONTROL_POINT_IRONDEEP_MINE;

        if (mineNode < AV_NUM_CONTROL_POINTS && m_nodes[mineNode] != nullptr)
        {
            AVNode* mine = m_nodes[mineNode];
            const uint32_t newState = plr->isTeamAlliance() ? AV_NODE_STATE_ALLIANCE_CONTROLLED : AV_NODE_STATE_HORDE_CONTROLLED;
            if (mine->m_state != newState)
            {
                mine->ChangeState(newState);
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The {} has taken control of {} Mine!",
                    plr->isTeamAlliance() ? "Alliance" : "Horde", mineNode == AV_CONTROL_POINT_COLDTOOTH_MINE ? "Coldtooth" : "Irondeep");
            }
        }
    }

    Player* plr2;
    if (pVictim->getEntry() == AV_NPC_GENERAL_VANNDAR_STORMPIKE)
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "The Stormpike General is dead!");
        RemoveReinforcements(0, AV_NUM_REINFORCEMENTS);

        for (std::set<Player*>::iterator itx = m_players[1].begin(); itx != m_players[1].end(); ++itx)
        {
            plr2 = (*itx);
            if (!plr2) continue;

            HonorHandler::AddHonorPointsToPlayer(plr2, 62);
            HonorHandler::AddBattlegroundObjectiveXp(plr2, 4);
        }
    }
    else if (pVictim->getEntry() == AV_NPC_GENERAL_DREK_THAR)
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Frostwolf General is dead!");
        RemoveReinforcements(1, AV_NUM_REINFORCEMENTS);

        for (std::set<Player*>::iterator itx = m_players[0].begin(); itx != m_players[0].end(); ++itx)
        {
            plr2 = (*itx);
            if (!plr2) continue;

            HonorHandler::AddHonorPointsToPlayer(plr2, AV_HONOR_ON_KILL_BOSS);
            HonorHandler::AddBattlegroundObjectiveXp(plr2, 4);
        }
    }
    else if (pVictim->getEntry() == AV_NPC_CAPTAIN_GALVANGAR)
    {
        RemoveReinforcements(1, AV_POINTS_ON_KILL_CAPTAIN);
        for (std::set<Player*>::iterator itx = m_players[0].begin(); itx != m_players[0].end(); ++itx)
        {
            plr2 = (*itx);
            if (!plr2) continue;

            HonorHandler::AddHonorPointsToPlayer(plr2, AV_HONOR_ON_KILL_BOSS);
            HonorHandler::AddBattlegroundObjectiveXp(plr2, 3);
        }
    }
    else if (pVictim->getEntry() == AV_NPC_CAPTAIN_BALINDA_STONEHEARTH)
    {
        RemoveReinforcements(0, AV_POINTS_ON_KILL_CAPTAIN);
        for (std::set<Player*>::iterator itx = m_players[1].begin(); itx != m_players[1].end(); ++itx)
        {
            plr2 = (*itx);
            if (!plr2) continue;

            HonorHandler::AddHonorPointsToPlayer(plr2, AV_HONOR_ON_KILL_BOSS);
            HonorHandler::AddBattlegroundObjectiveXp(plr2, 3);
        }
    }
}

void AlteracValley::Finish(uint32_t losingTeam)
{
    if (this->hasEnded()) return;

    sEventMgr.RemoveEvents(this);
    sEventMgr.AddEvent(static_cast<Battleground*>(this), &Battleground::close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    this->endBattleground(losingTeam == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE);
}

// Static AV Loot Table
struct AVLoot
{
    uint32_t ItemId;
    int8_t Faction;
    float Chance;
    uint32_t MinCount;
    uint32_t MaxCount;
};

const static AVLoot g_avLoot[] =
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Alliance Loot

    // Stormpike Soldier's Blood
    { 17306, 0, 100.0f, 1, 1 },
    // Stormpike Soldier's Meat
    { 17326, 0, 80.0, 1, 1 },

    //////////////////////////////////////////////////////////////////////////////////////////
    // Horde Loot

    // Crystal Cluster
    { 17423, 1, 50.0f, 1, 1 },

    //////////////////////////////////////////////////////////////////////////////////////////
    // Global loot

    // Armor Scrap
    { 17422, -1, 100.0f, 1, 5 },
    // EOF
    { 0, 0, 0},
};

void AlteracValley::HookGenerateLoot(Player* plr, Object* pCorpse)
{
    auto* corpse = dynamic_cast<Corpse*>(pCorpse);
    if (!corpse)
        return;

    const AVLoot* lootPtr = &g_avLoot[0];
    while (lootPtr->ItemId != 0)
    {
        if (lootPtr->Faction == -1 || lootPtr->Faction == static_cast<int8_t>(plr->getTeam()))
        {
            if (Util::checkChance(lootPtr->Chance * worldConfig.getFloatRate(RATE_DROP0)))
            {
                if (const auto* itemProto = sMySQLStore.getItemProperties(lootPtr->ItemId))
                {
                    LootItem lootItem;
                    lootItem.is_ffa = false;
                    lootItem.itemproto = itemProto;
                    if (lootPtr->MinCount != lootPtr->MaxCount)
                    {
                        const uint32_t range = lootPtr->MaxCount - lootPtr->MinCount + 1;
                        lootItem.count = static_cast<decltype(lootItem.count)>(Util::getRandomUInt(range) + lootPtr->MinCount);
                    }
                    else
                    {
                        lootItem.count = static_cast<decltype(lootItem.count)>(lootPtr->MinCount);
                    }

                    lootItem.iRandomProperty = nullptr;
                    lootItem.iRandomSuffix = nullptr;
                    lootItem.roll = nullptr;

                    corpse->loot.items.push_back(std::move(lootItem));
                }
            }
        }

        ++lootPtr;
    }

    // add some money
    float gold = ((plr->getLevel() / 2.5f) + 1.0f) * 100.0f; // TODO: fix this later
    gold *= worldConfig.getFloatRate(RATE_MONEY);

    // set it
    corpse->loot.gold = Util::float2int32(gold);
}

void AlteracValley::EventUpdateResources()
{
    if (!m_hasStarted || hasEnded())
        return;

    // each controlled mine grants reinforcements to its owner once per interval.
    static constexpr uint32_t mineNodes[] =
    {
        AV_CONTROL_POINT_COLDTOOTH_MINE,
        AV_CONTROL_POINT_IRONDEEP_MINE
    };

    for (const uint32_t nodeId : mineNodes)
    {
        if (m_nodes[nodeId] == nullptr)
            continue;

        switch (m_nodes[nodeId]->m_state)
        {
            case AV_NODE_STATE_ALLIANCE_CONTROLLED:
                AddReinforcements(TEAM_ALLIANCE, AV_ADD_POINTS_ON_CONTROLLED_MINE);
                break;
            case AV_NODE_STATE_HORDE_CONTROLLED:
                AddReinforcements(TEAM_HORDE, AV_ADD_POINTS_ON_CONTROLLED_MINE);
                break;
            default:
                break;
        }

        if (hasEnded())
            return;
    }

    // the event is scheduled as a one-shot in OnStart, so schedule the next tick here.
    if (!hasEnded())
        sEventMgr.AddEvent(this, &AlteracValley::EventUpdateResources, EVENT_BATTLEGROUND_RESOURCEUPDATE, AV_REINFORCEMENT_ADD_INTERVAL, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void AlteracValley::EventAssaultControlPoint(uint32_t x)
{
    m_nodes[x]->Capture();
}

bool AlteracValley::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    castSpellOnTeam(winningTeam, 43475);
    castSpellOnTeam(winningTeam, 69160);
    castSpellOnTeam(winningTeam, 69501);
    return true;
}

void AlteracValley::HookOnFlagDrop(Player* /*plr*/)
{
}

void AlteracValley::HookOnShadowSight()
{
}
