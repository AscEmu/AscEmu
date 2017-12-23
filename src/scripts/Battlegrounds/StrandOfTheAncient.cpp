/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

 /*
 Strand of the Ancients
 ======================

 * Other Game Objects
 * Defender's Portal, 190763
 * Defender's Portal, 191575
 * Defender's Portal, 192819
 -gps at:
 -gps to:
 * Titan Relic, 194083 (different one, not supported atm)
 * Collision PC Size, 188215
 * The Coffin Carrier, 193184
 * The Blightbringer, 193183
 * The Frostbreaker, 193185
 * The Graceful Maiden (boat?), 193182
 * Doodad_WG_Keep_Door01_collision01, 194162 (Not implemented atm)

 * Revive everyone after round one
 * bg->EventResurrectPlayers()

 * Setup index 34 in worldstring_tables to equal "Strand of the Ancients"

 * Fix level requirements to join the battleground. And fix which npc text is used
 for the battlemaster gossip scripts, displaying the proper of 3 messages
 npc_text
 13832 = You are not yet strong enough to do battle in the Strand of the Ancients. Return when you have gained more experience.
 13834 = We cannot allow the Alliance to turn the hidden secrets of the Strand of the Ancients against us. Will you join our brave warriors there?
 +13833 = We cannot allow the Horde to turn the hidden secrets of the Strand of the Ancients against us. Will you join our brave warriors there?

 * Increase the view distance on map 607 to 500 or 0 (Unlimited). See the
 transporter patch... Need a way to see the gates from as far away as
 the boats.

 * Besure to spawn, platforms, vehicels, and relic so only the proper faction
 can use them.

 * Fix it where a BG is instanced as soon as the first player joins, only
 after one faction has field their entire queue for a particular battlefield,
 would a new BG instance be created. It might actually be this way, if so
 just patch so that these pre-loaded instances appear in the battlemaster lists.

 * Also change so numbers are reused, once SOTA instance 1 is deleted, there is
 no reason why that instance id can't be reused. Also each BG needs it own
 unique numbering, instead of a shared pool.
*/

#include "StdAfx.h"
#include "StrandOfTheAncient.h"
#include "Management/WorldStates.h"
#include "Map/MapMgr.h"
#include "Objects/GameObject.h"
#include "Chat/ChatDefines.hpp"

const float sotaTitanRelic[4] = { 836.5f, -108.8f, 120.59f, 0.0f };

const uint32 GateGOIds[6] =
{
    190722,    // Gate of the Green Emerald
    190727,    // Gate of the Yellow Moon
    190724,    // Gate of the Blue Sapphire
    190726,    // Gate of the Red Sun
    190723,    // Gate of the Purple Amethyst
    192549     // Chamber of Ancient Relics
};
const float sotaGates[GATE_COUNT][4] =
{
    { 1411.571f,  108.163f, 30.192f, 5.4411f }, // Gate of the Green Emerald
    { 1054.452f, -107.011f, 83.522f, 0.0341f }, // Gate of the Yellow Moon
    { 1429.541f, -221.437f, 32.353f, 0.9536f }, // Gate of the Blue Sapphire
    { 1227.467f, -212.555f, 56.801f, 0.4123f }, // Gate of the Red Sun
    { 1212.681f,   83.211f, 54.813f, 5.7451f }  // Gate of the Purple Amethyst
};
const float sotaChamberGate[4] = { 878.555f, -108.21f, 117.845f, 0.01f };

// Things radiating out from the gates... same orientation as door.
const uint32 GateSigilGOIds[5] = { 192687, 192685, 192689, 192690, 192691 };
const float sotaGateSigils[GATE_COUNT][4] =
{
    { 1414.254f,  104.78f, 42.692f, 5.4411f }, // Sigil of the Green Emerald
    { 1058.031f, -106.99f, 95.889f, 0.0341f }, // Sigil of the Yellow Moon
    { 1431.983f, -217.91f, 45.001f, 0.9536f }, // Sigil of the Blue Sapphire
    { 1230.451f, -211.19f, 69.211f, 0.4123f }, // Sigil of the Red Sun
    { 1215.811f,  81.532f, 67.481f, 5.7451f }  // Sigil of the Purple Amethyst
};

// Defender transporter platform locations
const float sotaTransporters[GATE_COUNT][4] =
{
    { 1394.0444f,   72.586f, 31.0535f, 0.0f }, // Transporter of the Green Emerald
    { 1065.0000f, -89.6990f, 81.3000f, 0.0f }, // Transporter of the Yellow Moon
    { 1467.9499f, -225.670f, 31.1201f, 0.0f }, // Transporter of the Blue Sapphire
    { 1255.9300f, -233.382f, 56.6600f, 0.0f }, // Transporter of the Red Sun
    { 1215.6789f,  47.4700f, 54.5010f, 0.0f }  // Transporter of the Purple Amethyst
};

// Defender transporter destination locations
const float sotaTransporterDestination[GATE_COUNT][4] =
{
    { 1388.9399f,  103.0670f, 34.4905f, 5.4571f }, // Destination of the Green Emerald
    { 1043.6899f,  -87.9499f, 87.1138f, 0.0030f }, // Destination of the Yellow Moon
    { 1441.0411f, -240.9739f, 35.2641f, 0.9490f }, // Destination of the Blue Sapphire
    { 1228.3420f, -235.2333f, 60.0282f, 0.4584f }, // Destination of the Red Sun
    { 1193.8570f,   69.9000f, 58.0462f, 5.7245f }  // Destination of the Purple Amethyst
};

// Two guns per gate, GUN_LEFT and GUN_RIGHT
static LocationVector CanonLocations[SOTA_NUM_CANONS] =
{
    LocationVector(1436.429f, 110.05f, 41.407f, 5.4f),
    LocationVector(1404.9023f, 84.758f, 41.183f, 5.46f),
    LocationVector(1068.693f, -86.951f, 93.81f, 0.02f),
    LocationVector(1068.83f, -127.56f, 96.45f, 0.0912f),
    LocationVector(1422.115f, -196.433f, 42.1825f, 1.0222f),
    LocationVector(1454.887f, -220.454f, 41.956f, 0.9627f),
    LocationVector(1232.345f, -187.517f, 66.945f, 0.45f),
    LocationVector(1249.634f, -224.189f, 66.72f, 0.635f),
    LocationVector(1236.213f, 92.287f, 64.965f, 5.751f),
    LocationVector(1215.11f, 57.772f, 64.739f, 5.78f)
};

static LocationVector DemolisherLocations[SOTA_NUM_DEMOLISHERS] =
{
    LocationVector(1618.04f, 61.42f, 7.24f, 3.97f),
    LocationVector(1575.56f, -158.42f, 5.02f, 2.12f),
    LocationVector(1575.11f, 98.87f, 2.83f, 3.75f),
    LocationVector(1610.35f, -116.36f, 8.81f, 2.51f),

    LocationVector(1353.13f, 223.74f, 35.26f, 4.34f),
    LocationVector(1342.26f, 195.08f, 30.89f, 4.34f),
    LocationVector(1371.05f, -317.07f, 35.01f, 1.94f),
    LocationVector(1360.56f, -290.51f, 30.89f, 1.94f)
};

// ---- Verify remaining ----- //
// There should only be two boats. boats three and four here
// are a lazy hack for not wanting to program the boats to move via waypoints
const float sotaBoats[4][4] =
{
    { 1623.9f,   35.61f, 1.72f, 0.78f },  // end potition
    { 1611.3f,  -87.95f, 3.07f, 5.68f },  // end potition
    { 2679.6f, -826.89f, 3.71f, 5.78f },  // initial potition
    { 2574.3f,  981.26f, 2.63f, 0.807f }  // initial potition
};

static LocationVector sotaAttackerStartingPosition[SOTA_NUM_ROUND_STAGES] =
{
    LocationVector(1598.85f, -103.99f, 8.873f, 4.016f), // this is momentary.
    LocationVector(1607.99f, 47.6378f, 7.579f, 2.265f)  // this is momentary.
};

static LocationVector sotaDefenderStartingPosition
= LocationVector(1209.7f, -65.16f, 70.1f, 0.0f);

// Npcs
const uint32 sotaNPCsIds[3] = { 29260, 29262, 29 };
static LocationVector sotaNPCSLocations[SOTA_NPCS] =
{
    LocationVector(1348.644165f, -298.786469f, 31.080130f, 1.710423f),
    LocationVector(1358.191040f, 195.527786f, 31.018187f, 4.171337f),
    LocationVector(841.921f, -134.194f, 109.838f, 6.23082f)
};

const float sotaBombsLocations[SOTA_NUM_BOMBS][4]
{
    // North Bombs
    { 1592.49f, 47.5969f, 7.52271f, 4.63218f },
    { 1593.91f, 47.8036f, 7.65856f, 4.63218f },
    { 1593.13f, 46.8106f, 7.54073f, 4.63218f },
    { 1589.22f, 36.3616f, 7.45975f, 4.64396f },
    { 1588.24f, 35.5842f, 7.55613f, 4.79564f },
    { 1588.14f, 36.7611f, 7.49675f, 4.79564f },
    { 1595.74f, 35.5278f, 7.46602f, 4.90246f },
    { 1596.85f, 36.6475f, 7.47991f, 4.90246f },
    { 1597.03f, 36.2356f, 7.48631f, 4.90246f },
    { 1597.93f, 37.1214f, 7.51725f, 4.90246f },
    { 1598.16f, 35.8881f, 7.50018f, 4.90246f },
    { 1579.61f, -98.0917f, 8.48478f, 1.37996f },
    { 1581.21f, -98.4011f, 8.47483f, 1.37996f },
    { 1580.38f, -98.9556f, 8.47721f, 1.38781f },
    { 1585.68f, -104.966f, 8.88551f, 0.49324f },
    { 1586.15f, -106.033f, 9.10616f, 0.49324f },
    { 1584.88f, -105.394f, 8.82985f, 0.49324f },
    { 1581.87f, -100.899f, 8.46164f, 0.92914f },
    { 1581.48f, -99.4657f, 8.46926f, 0.92914f },
    { 1583.21f, -91.2291f, 8.49227f, 1.40038f },
    { 1581.94f, -91.0119f, 8.49977f, 1.40038f },
    { 1582.33f, -91.9511f, 8.49353f, 1.18441f },
    // East Bombs
    { 1342.06f, -304.049f, 30.9532f, 5.59507f },
    { 1340.96f, -304.536f, 30.9458f, 1.28323f },
    { 1341.22f, -303.316f, 30.9413f, 0.48605f },
    { 1342.22f, -302.939f, 30.9861f, 4.87643f },
    { 1382.16f, -287.466f, 32.3063f, 4.80968f },
    { 1381.12f, -287.581f, 32.2805f, 4.80968f },
    { 1381.55f, -286.536f, 32.3929f, 2.84225f },
    { 1382.75f, -286.354f, 32.4099f, 1.00442f },
    { 1379.92f, -287.341f, 32.2872f, 3.81615f },
    // West Bombs
    { 1333.45f, 211.354f, 31.0538f, 5.03666f },
    { 1334.29f, 209.582f, 31.0532f, 1.28088f },
    { 1332.72f, 210.049f, 31.0532f, 1.28088f },
    { 1334.28f, 210.781f, 31.0538f, 3.85856f },
    { 1332.64f, 211.391f, 31.0532f, 1.29266f },
    { 1371.41f, 194.028f, 31.5107f, 0.75309f },
    { 1372.39f, 194.951f, 31.4679f, 0.75309f },
    { 1371.58f, 196.942f, 30.9349f, 1.01777f },
    { 1370.43f, 196.614f, 30.9349f, 0.95729f },
    { 1369.46f, 196.877f, 30.9351f, 2.45348f },
    { 1370.35f, 197.361f, 30.9349f, 1.08689f },
    { 1369.47f, 197.941f, 30.9349f, 0.98478f },
    { 1100.52f, -2.41391f, 70.2984f, 0.1315f },
    { 1099.35f, -2.13851f, 70.3375f, 4.4586f },
    { 1099.59f, -1.00329f, 70.238f, 2.49903f },
    { 1097.79f, 0.571316f, 70.159f, 4.00307f },
    { 1098.74f, -7.23252f, 70.7972f, 4.1523f },
    { 1098.46f, -5.91443f, 70.6715f, 4.1523f },
    { 1097.53f, -7.39704f, 70.7959f, 4.1523f },
    { 1097.32f, -6.64233f, 70.7424f, 4.1523f },
    { 1096.45f, -5.96664f, 70.7242f, 4.1523f },
    { 972.524f, 1.25333f, 86.8351f, 5.28497f },
    { 971.993f, 2.05668f, 86.8584f, 5.28497f },
    { 973.635f, 2.11805f, 86.8197f, 2.36722f },
    { 974.791f, 1.74679f, 86.7942f, 1.59361f },
    { 974.771f, 3.0445f, 86.8125f, 0.647199f },
    { 979.554f, 3.6037f, 86.7923f, 1.691781f },
    { 979.758f, 2.57519f, 86.7748f, 1.76639f },
    { 980.769f, 3.48904f, 86.7939f, 1.76639f },
    { 979.122f, 2.87109f, 86.7794f, 1.76639f },
    { 986.167f, 4.85363f, 86.8439f, 1.57791f },
    { 986.176f, 3.50367f, 86.8217f, 1.57791f },
    { 987.331f, 4.67389f, 86.8486f, 1.57791f },
    { 985.231f, 4.65898f, 86.8368f, 1.57791f },
    { 984.556f, 3.54097f, 86.8137f, 1.57791f }
};

static LocationVector FlagPolePositions[NUM_SOTA_CONTROL_POINTS] =
{
    LocationVector(1338.863892f, -153.336533f, 30.895121f, -2.530723f),
    LocationVector(1322.611816f,   19.509911f, 31.249935f,  1.561744f),
    LocationVector(1215.114258f,  -65.711861f, 70.084267f, -3.124123f)
};

static LocationVector FlagPositions[NUM_SOTA_CONTROL_POINTS] = 
{
    LocationVector(1338.859253f, -153.327316f, 30.895077f, -2.530723f),
    LocationVector(1322.611816f,   19.509911f, 31.249935f,  1.561744f),
    LocationVector(1215.108032f,  -65.715767f, 70.084267f, -3.124123f)
};

static const uint32 FlagIDs[NUM_SOTA_CONTROL_POINTS][MAX_PLAYER_TEAMS] =
{
    { 191306, 191305 },
    { 191308, 191307 },
    { 191310, 191309 }
};

static const uint32 CPWorldstates[NUM_SOTA_CONTROL_POINTS][MAX_PLAYER_TEAMS] =
{
    { WORLDSTATE_SOTA_GY_E_A, WORLDSTATE_SOTA_GY_E_H },
    { WORLDSTATE_SOTA_GY_W_A, WORLDSTATE_SOTA_GY_W_H },
    { WORLDSTATE_SOTA_GY_S_A, WORLDSTATE_SOTA_GY_S_H }
};

static const uint32 SOTA_FLAGPOLE_ID = 191311;

const char* ControlPointNames[NUM_SOTA_CONTROL_POINTS] =
{
    "East Graveyard",
    "West Graveyard",
    "South Graveyard"
};

static LocationVector GraveyardLocations[NUM_SOTA_GRAVEYARDS] =
{
    LocationVector(1396.06018066f, -288.036895752f, 32.0815124512f, 0.0f),
    LocationVector(1388.80358887f, 203.354873657f, 32.1526679993f, 0.0f),
    LocationVector(1122.27844238f, 4.41617822647f, 68.9358291626f, 0.0f),
    LocationVector(964.595275879f, -189.784011841f, 90.6604995728f, 0.0f),
    LocationVector(1457.19372559f, -53.7132720947f, 5.18109416962f, 0.0f)
};

static const uint32 TeamFactions[MAX_PLAYER_TEAMS] = {
    1,
    2
};

StrandOfTheAncient::StrandOfTheAncient(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t):CBattleground(mgr, id, lgroup, t)
{
    m_zoneid = 4384;
    std::fill(&npc[0], &npc[SOTA_NPCS], reinterpret_cast<Creature*>(NULL));
    std::fill(&canon[0], &canon[SOTA_NUM_CANONS], reinterpret_cast<Creature*>(NULL));
    std::fill(&demolisher[0], &demolisher[SOTA_NUM_DEMOLISHERS], reinterpret_cast<Creature*>(NULL));

    Attackers = 0;
    Defenders = 0;
    BattleRound = 0;
    RoundTime = 0;
    roundprogress = SOTA_ROUND_PREPARATION;

    for (uint8 i = 0; i < BUFF_COUNT; ++i)
    {
        m_buffs[i] = nullptr;
    }

    for (uint8 i = 0; i < 4; ++i)
    {
        m_boats[i] = nullptr;
    }

    for (uint8 i = 0; i < SOTA_NPCS; ++i)
    {
        npc[i] = nullptr;
    }

    for (uint8 i = 0; i < GATE_COUNT; ++i)
    {
        m_gates[i] = nullptr;
        m_gateSigils[i] = nullptr;
        m_gateTransporters[i] = nullptr;
    }

    m_relic = nullptr;
    m_endgate = nullptr;

    for (uint8 i = 0; i < SOTA_NORTH_BOMBS; ++i)
    {
        m_bomb[i] = nullptr;
    }

}

StrandOfTheAncient::~StrandOfTheAncient()
{
    std::fill(&npc[0], &npc[SOTA_NPCS], reinterpret_cast<Creature*>(NULL));
    std::fill(&canon[0], &canon[SOTA_NUM_CANONS], reinterpret_cast<Creature*>(NULL));
    std::fill(&demolisher[0], &demolisher[SOTA_NUM_DEMOLISHERS], reinterpret_cast<Creature*>(NULL));
}

void StrandOfTheAncient::HookOnAreaTrigger(Player* /*plr*/, uint32 /*id*/)
{}

void StrandOfTheAncient::HookOnPlayerKill(Player* plr, Player* /*pVictim*/)
{
    plr->m_bgScore.KillingBlows++;
    UpdatePvPData();
}

void StrandOfTheAncient::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
    UpdatePvPData();
}

void StrandOfTheAncient::OnPlatformTeleport(Player* /*plr*/)
{}

void StrandOfTheAncient::OnAddPlayer(Player* plr)
{
    if (!m_started)
    {
        plr->CastSpell(plr, BG_PREPARATION, true);
    }
}

void StrandOfTheAncient::OnRemovePlayer(Player* plr)
{
    if (!m_started)
    {
        plr->RemoveAura(BG_PREPARATION);
    }
}

LocationVector StrandOfTheAncient::GetStartingCoords(uint32 team)
{
    if (team == Attackers)
    {
        return sotaAttackerStartingPosition[roundprogress];
    }
    else
    {
        return sotaDefenderStartingPosition;
    }
}

/*! Handles end of battleground rewards (marks etc)
*  \param winningTeam Team that won the battleground
*  \returns True if CBattleground class should finish applying rewards, false if we handled it fully */
bool StrandOfTheAncient::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    CastSpellOnTeam(winningTeam, 61213);
    return true;
}

void StrandOfTheAncient::HookOnPlayerDeath(Player* plr)
{
    plr->m_bgScore.Deaths++;
    UpdatePvPData();
}

void StrandOfTheAncient::HookOnMount(Player* /*plr*/)
{
    /* Allowed */
}

bool StrandOfTheAncient::HookHandleRepop(Player* plr)
{
    float dist = 999999.0f;
    LocationVector dest_pos;
    uint32 id = 0;

    // Let's find the closests GY
    for (uint8 i = SOTA_GY_EAST; i < NUM_SOTA_GRAVEYARDS; i++)
    {
        if (graveyard[i].faction == plr->GetTeam())
        {
            if (graveyard[i].spiritguide == NULL)
                continue;

            float gydist = plr->CalcDistance(graveyard[i].spiritguide);
            if (gydist > dist)
                continue;

            dist = gydist;
            dest_pos = graveyard[i].spiritguide->GetPosition();
            id = i;
        }
    }

    if (id >= NUM_SOTA_GRAVEYARDS)
        return false;

    // port to it and queue for auto-resurrect
    plr->SafeTeleport(plr->GetMapId(), plr->GetInstanceID(), dest_pos);
    QueuePlayerForResurrect(plr, graveyard[id].spiritguide);

    return true;
}

void StrandOfTheAncient::OnCreate()
{

    BattleRound = 1;
    roundprogress = SOTA_ROUND_PREPARATION;

    for (uint8 i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
        RoundFinishTime[i] = ROUND_LENGTH;
    }

    m_pvpData.clear();
    m_resurrectMap.clear();

    /* Relic */
    m_relic = m_mapMgr->CreateAndSpawnGameObject(GO_RELIC, sotaTitanRelic[0], sotaTitanRelic[1], sotaTitanRelic[2], sotaTitanRelic[3], 0.6f);

    for (uint8 i = 0; i < GATE_COUNT; i++)
    {
        m_gates[i] = m_mapMgr->CreateAndSpawnGameObject(GateGOIds[i], sotaGates[i][0], sotaGates[i][1], sotaGates[i][2], sotaGates[i][3], 1.0f);
        m_gateSigils[i] = m_mapMgr->CreateAndSpawnGameObject(GateSigilGOIds[i], sotaGateSigils[i][0], sotaGateSigils[i][1], sotaGateSigils[i][2], sotaGateSigils[i][3], 1.0f);
        m_gateTransporters[i] = m_mapMgr->CreateAndSpawnGameObject(192819,sotaTransporters[i][0], sotaTransporters[i][1], sotaTransporters[i][2], sotaTransporters[i][3], 1.0f);
    }

    // Spawn door for Chamber of Ancient Relics
    m_endgate = m_mapMgr->CreateAndSpawnGameObject(GateGOIds[5], sotaChamberGate[0], sotaChamberGate[1], sotaChamberGate[2], sotaChamberGate[3], 1.0f);

    PrepareRound();
}

void StrandOfTheAncient::OnStart()
{
    m_started = true;

    StartRound();
}

void StrandOfTheAncient::HookGenerateLoot(Player* /*plr*/, Object* /*pOCorpse*/)
{
    LOG_DEBUG("StrandOfTheAncient::HookGenerateLoot");
}

void StrandOfTheAncient::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void StrandOfTheAncient::HookOnUnitDied(Unit* victim)
{
    if (victim->IsCreature())
    {
        for (uint8 i = 0; i < SOTA_NUM_DEMOLISHERS; ++i)
        {
            Creature *c = demolisher[i];
            if (c == nullptr)
            {
                continue;
            }

            if (victim->GetGUID() != c->GetGUID())
            {
                continue;
            }

            demolisher[i] = SpawnCreature(SOTA_DEMOLISHER, DemolisherLocations[i], TeamFactions[Attackers]);
            c->Despawn(1, 0);
        }

        for (uint8 i = 0; i < SOTA_NUM_CANONS; i++)
        {
            if (canon[i] == nullptr)
            {
                continue;
            }

            if (victim->GetGUID() != canon[i]->GetGUID())
            {
                continue;
            }

            canon[i]->Despawn(1, 0);
            canon[i] = nullptr;
        }
    }
}

void StrandOfTheAncient::SetIsWeekend(bool isweekend)
{
    LOG_DEBUG("*** StrandOfTheAncient::SetIsWeekend");
    m_isWeekend = isweekend;
}

bool StrandOfTheAncient::HookSlowLockOpen(GameObject* go, Player* /*player*/, Spell* /*spell*/)
{
    uint32 goentry = go->GetEntry();

    switch (goentry)
    {
        case 191305:
        case 191306:
        {
            CaptureControlPoint(SOTA_CONTROL_POINT_EAST_GY);
            return true;
        }
        case 191307:
        case 191308:
        {
            CaptureControlPoint(SOTA_CONTROL_POINT_WEST_GY);
            return true;
        }
        case 191309:
        case 191310:
        {
            CaptureControlPoint(SOTA_CONTROL_POINT_SOUTH_GY);
            return true;
        }
        default:
        {
            LOG_DEBUG("HookSlowLockOpen called for invalid go entry: %u", goentry);
            return true;
        }
    }
}

bool StrandOfTheAncient::HookQuickLockOpen(GameObject* go, Player* /*player*/, Spell* /*spell*/)
{
    uint32 entry = go->GetEntry();
    if (entry == GO_RELIC)
    {
        PlaySoundToAll(Attackers == TEAM_ALLIANCE ? SOTA_SOUND_VICTORY_ALLIANCE : SOTA_SOUND_VICTORY_HORDE);
        FinishRound();
    }

    return true;
}

// For banners
void StrandOfTheAncient::HookFlagStand(Player* /*plr*/, GameObject* /*obj*/)
{}

// time in seconds
void StrandOfTheAncient::SetTime(uint32 secs)
{
    uint32 minutes = secs / TIME_MINUTE;
    uint32 seconds = secs % TIME_MINUTE;
    uint32 digits[3];
    digits[0] = minutes;
    digits[1] = seconds / 10;
    digits[2] = seconds % 10;

    SetWorldState(WORLDSTATE_SOTA_TIMER_MINS, digits[0]);
    SetWorldState(WORLDSTATE_SOTA_TIMER_SEC_TENS, digits[1]);
    SetWorldState(WORLDSTATE_SOTA_TIMER_SEC_DECS, digits[2]);
    SetRoundTime(secs);
}

void StrandOfTheAncient::PrepareRound()
{
    roundprogress = SOTA_ROUND_PREPARATION;

    if (BattleRound == 1)
    {
        Attackers = Util::getRandomUInt(1);
        if (Attackers == TEAM_ALLIANCE)
        { 
            Defenders = TEAM_HORDE;
            CastSpellOnTeam(Defenders, SOTA_SPELL_HORDE_CONTROL_PHASE_SHIFT);
            PlaySoundToTeam(Attackers, SOTA_SOUND_DEFEAT_HORDE);
        }
        else
        { 
            Defenders = TEAM_ALLIANCE;
            CastSpellOnTeam(Defenders, SOTA_SPELL_ALLIANCE_CONTROL_PHASE_SHIFT);
            PlaySoundToTeam(Attackers, SOTA_SOUND_DEFEAT_ALLIANCE);
        }
    }
    else
    {
        std::swap(Attackers, Defenders);
    }

    for (uint8 i = 0; i < GATE_COUNT; i++)
    {
        static_cast<GameObject_Destructible*>(m_gates[i])->Rebuild();
        m_gates[i]->SetFaction(TeamFactions[Defenders]);
    }

    static_cast<GameObject_Destructible*>(m_endgate)->Rebuild();
    m_endgate->SetFaction(TeamFactions[Defenders]);

    m_relic->SetFaction(TeamFactions[Attackers]);

    // Boats
    for (uint8 i = 0; i < 2; i++)
    {
        uint32 boatId = 0;
        switch (i)
        {
            case SOTA_BOAT_WEST:
                boatId = Attackers ? SOTA_BOAT_HORDER_W : SOTA_BOAT_ALLIANCE_W;
                break;
            case SOTA_BOAT_EAST:
                boatId = Attackers ? SOTA_BOAT_HORDER_E : SOTA_BOAT_ALLIANCE_E;
                break;
        }
        if (m_boats[i] != nullptr)
        {
            m_boats[i]->Despawn(0, 0);
        }

        m_boats[i] = m_mapMgr->CreateAndSpawnGameObject(boatId, sotaBoats[i][0], sotaBoats[i][1], sotaBoats[i][2], sotaBoats[i][3], 1.0f);
    }

    for (uint8 i = 0; i < GATE_COUNT; ++i)
    {
        m_gateTransporters[i]->SetFaction(TeamFactions[Defenders]);
    }

    for (uint8 i = 0; i < SOTA_NORTH_BOMBS; ++i)
    {
        m_bomb[i] = m_mapMgr->CreateAndSpawnGameObject(SOTA_BOMBS, sotaBombsLocations[i][0], sotaBombsLocations[i][1], sotaBombsLocations[i][2], sotaBombsLocations[i][3], 1.5f);
    }

    for (uint8 i = 0; i < SOTA_NUM_CANONS; ++i)
    {
        if (canon[i] != nullptr)
        {
            canon[i]->Despawn(0, 0);
        }

        canon[i] = SpawnCreature(SOTA_ANTI_PERSONNAL_CANNON, CanonLocations[i], TeamFactions[Defenders]);
    }

    for (uint8 i = 0; i < SOTA_NORTH_DEMOLISHERS; ++i)
    {
        Creature* c = demolisher[i];
        demolisher[i] = SpawnCreature(SOTA_DEMOLISHER, DemolisherLocations[i], TeamFactions[Attackers]);
        if (c != nullptr)
        {
            c->Despawn(0, 0);
        }
    }

    for (uint8 i = 0; i < SOTA_NPCS; ++i)
    {
        if (npc[i] != nullptr)
        {
            npc[i]->Despawn(0, 0);
            npc[i] = nullptr;
        }
    }

    for (uint8 i = SOTA_WEST_WS_DEMOLISHER_INDEX; i < SOTA_EAST_WS_DEMOLISHER_INDEX; ++i)
    {
        if (demolisher[i] != nullptr)
        {
            demolisher[i]->Despawn(0, 0);
            demolisher[i] = nullptr;
        }
    }

    for (uint8 i = SOTA_EAST_WS_DEMOLISHER_INDEX; i < SOTA_NUM_DEMOLISHERS; ++i)
    {
        if (demolisher[i] != nullptr)
        {
            demolisher[i]->Despawn(0, 0);
            demolisher[i] = nullptr;
        }
    }

    SOTACPStates state;

    if (Attackers == TEAM_ALLIANCE)
    {
        state = SOTA_CP_STATE_HORDE_CONTROL;
        SetWorldState(WORLDSTATE_SOTA_HORDE_ATTACKER, 0);
        SetWorldState(WORLDSTATE_SOTA_ALLIANCE_ATTACKER, 1);
        SetWorldState(WORLDSTATE_SOTA_SHOW_ALLY_ROUND, 1);
        SetWorldState(WORLDSTATE_SOTA_SHOW_HORDE_ROUND, 0);
        SetWorldState(WORLDSTATE_SOTA_SHOW_ALLY_DEFENSE, 0);
        SetWorldState(WORLDSTATE_SOTA_SHOW_HORDE_DEFENSE, 1);
        SetWorldState(WORLDSTATE_SOTA_SHOW_ALLY_BEACHHEAD1, 1);
        SetWorldState(WORLDSTATE_SOTA_SHOW_ALLY_BEACHHEAD2, 1);
        SetWorldState(WORLDSTATE_SOTA_SHOW_HORDE_BEACHHEAD1, 0);
        SetWorldState(WORLDSTATE_SOTA_SHOW_HORDE_BEACHHEAD2, 0);
    }
    else
    {
        state = SOTA_CP_STATE_ALLY_CONTROL;
        SetWorldState(WORLDSTATE_SOTA_HORDE_ATTACKER, 1);
        SetWorldState(WORLDSTATE_SOTA_ALLIANCE_ATTACKER, 0);
        SetWorldState(WORLDSTATE_SOTA_SHOW_ALLY_ROUND, 0);
        SetWorldState(WORLDSTATE_SOTA_SHOW_HORDE_ROUND, 1);
        SetWorldState(WORLDSTATE_SOTA_SHOW_ALLY_DEFENSE, 1);
        SetWorldState(WORLDSTATE_SOTA_SHOW_HORDE_DEFENSE, 0);
        SetWorldState(WORLDSTATE_SOTA_SHOW_ALLY_BEACHHEAD1, 0);
        SetWorldState(WORLDSTATE_SOTA_SHOW_ALLY_BEACHHEAD2, 0);
        SetWorldState(WORLDSTATE_SOTA_SHOW_HORDE_BEACHHEAD1, 1);
        SetWorldState(WORLDSTATE_SOTA_SHOW_HORDE_BEACHHEAD2, 1);
    }

    SpawnControlPoint(SOTA_CONTROL_POINT_EAST_GY, state);
    SpawnControlPoint(SOTA_CONTROL_POINT_WEST_GY, state);
    SpawnControlPoint(SOTA_CONTROL_POINT_SOUTH_GY, state);
    SpawnGraveyard(SOTA_GY_ATTACKER_BEACH, Attackers);
    SpawnGraveyard(SOTA_GY_DEFENDER, Defenders);

    if (BattleRound == 2)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        // Teleport players to their place and cast preparation on them

        for (std::set< Player* >::iterator itr = m_players[Attackers].begin(); itr != m_players[Attackers].end(); ++itr)
        {
            Player* p = *itr;
            p->SafeTeleport(p->GetMapId(), p->GetInstanceID(), sotaAttackerStartingPosition[0]);
            p->CastSpell(p, BG_PREPARATION, true);
        }

        for (std::set<Player*>::iterator itr = m_players[Defenders].begin(); itr != m_players[Defenders].end(); ++itr)
        {
            Player* p = *itr;
            p->SafeTeleport(p->GetMapId(), p->GetInstanceID(), sotaDefenderStartingPosition);
            p->CastSpell(p, BG_PREPARATION, true);
        }

        sEventMgr.AddEvent(this, &StrandOfTheAncient::StartRound, EVENT_SOTA_START_ROUND, MSTIME_SECOND * 10, 1, 0);
    }
};

void StrandOfTheAncient::StartRound()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    roundprogress = SOTA_ROUND_STARTED;

    for (std::set<Player*>::iterator itr = m_players[Attackers].begin(); itr != m_players[Attackers].end(); ++itr)
    {
        Player* p = *itr;

        p->SafeTeleport(p->GetMapId(), p->GetInstanceID(), sotaAttackerStartingPosition[SOTA_ROUND_STARTED]);
        p->RemoveAura(BG_PREPARATION);
    }

    RemoveAuraFromTeam(Defenders, BG_PREPARATION);

    SetWorldState(WORLDSTATE_SOTA_TIMER_MINS, 10); 
    SetTime(ROUND_LENGTH);
    sEventMgr.AddEvent(this, &StrandOfTheAncient::TimeTick, EVENT_SOTA_TIMER, MSTIME_SECOND * 1, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    UpdatePvPData();
}

void StrandOfTheAncient::FinishRound()
{
    CastSpellOnTeam(Attackers, SOTA_SPELL_END_OF_ROUND);
    CastSpellOnTeam(Defenders, SOTA_SPELL_END_OF_ROUND);

    sEventMgr.RemoveEvents(this, EVENT_SOTA_TIMER);
    EventResurrectPlayers();

    RoundFinishTime[BattleRound - 1] = RoundTime;

    if (BattleRound == 1)
    {
        BattleRound = 2;
        PrepareRound();
    }
    else
    {
        if (RoundFinishTime[0] < RoundFinishTime[1])
            Finish(Attackers);
        else
            Finish(Defenders);
    }
}

void StrandOfTheAncient::Finish(uint32 winningteam)
{
    sEventMgr.RemoveEvents(this);
    sEventMgr.AddEvent(static_cast< CBattleground* >(this), &CBattleground::Close, EVENT_BATTLEGROUND_CLOSE, MSTIME_SECOND * 120, 1, 0);
    this->EndBattleground(winningteam == TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE);
}

void StrandOfTheAncient::TimeTick()
{
    SetTime(RoundTime - 1);

    if (RoundTime == 0)
    {
        sEventMgr.RemoveEvents(this, EVENT_SOTA_TIMER);
        FinishRound();
    }
};

// Not used?
void StrandOfTheAncient::HookOnFlagDrop(Player* /*plr*/)
{}

void StrandOfTheAncient::HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/)
{}

void StrandOfTheAncient::HookOnShadowSight()
{}

void StrandOfTheAncient::SpawnControlPoint(SOTAControlPoints point, SOTACPStates state)
{
    if (state >= MAX_SOTA_CP_STATES)
        return;

    SOTAControlPoint &cp = controlpoint[point];

    if (cp.worldstate != 0)
        SetWorldState(cp.worldstate, 0);

    uint32 team = TEAM_ALLIANCE;
    uint32 faction = 0;

    switch (state)
    {
        case SOTA_CP_STATE_ALLY_CONTROL:
            team = TEAM_ALLIANCE;
            faction = 2;
            break;

        case SOTA_CP_STATE_HORDE_CONTROL:
            team = TEAM_HORDE;
            faction = 1;
            break;

        default:
            return;
            break;
    }

    // First time spawning
    if (cp.pole == NULL)
    {
        cp.pole = SpawnGameObject(SOTA_FLAGPOLE_ID, FlagPolePositions[point], 0, 35, 1.0f);
        cp.pole->PushToWorld(m_mapMgr);
    }
    else
    {
        if (cp.banner->IsInWorld())
            cp.banner->RemoveFromWorld(false);
    }

    cp.banner = SpawnGameObject(FlagIDs[point][team], FlagPositions[point], 0, faction, 1.0f);
    cp.banner->PushToWorld(m_mapMgr);

    cp.state = state;
    cp.worldstate = CPWorldstates[point][team];
    SetWorldState(cp.worldstate, 1);

    //Spawn graveyard
    SpawnGraveyard(SOTAGraveyards(uint32(point)), team);
}

void StrandOfTheAncient::CaptureControlPoint(SOTAControlPoints point)
{
    if (point >= NUM_SOTA_CONTROL_POINTS)
        return;

    SOTAControlPoint &cp = controlpoint[point];

    if (cp.banner->GetFaction() == 14)
        return;

    switch (cp.state)
    {
        case SOTA_CP_STATE_ALLY_CONTROL:
            SpawnControlPoint(point, SOTA_CP_STATE_HORDE_CONTROL);
            PlaySoundToAll(SOUND_HORDE_CAPTURE);
            SendChatMessage(CHAT_MSG_BG_EVENT_HORDE, 0, "The horde has captured the %s!", ControlPointNames[point]);
            break;

        case SOTA_CP_STATE_HORDE_CONTROL:
            SpawnControlPoint(point, SOTA_CP_STATE_ALLY_CONTROL);
            PlaySoundToAll(SOUND_ALLIANCE_CAPTURE);
            SendChatMessage(CHAT_MSG_BG_EVENT_ALLIANCE, 0, "The alliance has captured the %s!", ControlPointNames[point]);
            break;
    }

    cp.banner->SetFaction(14); // So they cannot be recaptured as per SOTA rules

    //Spawn workshop demolisher
    switch (point)
    {
        case SOTA_CONTROL_POINT_EAST_GY:
            for (uint8 i = SOTA_EAST_WS_DEMOLISHER_INDEX; i < SOTA_NUM_DEMOLISHERS; i++)
            {
                demolisher[i] = SpawnCreature(SOTA_DEMOLISHER, DemolisherLocations[i], TeamFactions[Attackers]);
            }
            for (uint8 i = SOTA_EAST_BOMBS_INDEX; i < SOTA_WEST_BOMBS_INDEX; i++)
            {
                m_bomb[i] = m_mapMgr->CreateAndSpawnGameObject(SOTA_BOMBS, sotaBombsLocations[i][0], sotaBombsLocations[i][1], sotaBombsLocations[i][2], sotaBombsLocations[i][3], 1.5f);
            }
            npc[TEAM_ALLIANCE] = SpawnCreature(SOTA_RIGGER_SPARKLIGHT, sotaNPCSLocations[0], TeamFactions[Attackers]);
            break;
        case SOTA_CONTROL_POINT_WEST_GY:
            for (uint8 i = SOTA_WEST_WS_DEMOLISHER_INDEX; i < SOTA_EAST_WS_DEMOLISHER_INDEX; i++)
            {
                demolisher[i] = SpawnCreature(SOTA_DEMOLISHER, DemolisherLocations[i], TeamFactions[Attackers]);
            }
            for (uint8 i = SOTA_WEST_BOMBS_INDEX; i < SOTA_NUM_BOMBS; i++)
            {
                m_bomb[i] = m_mapMgr->CreateAndSpawnGameObject(SOTA_BOMBS, sotaBombsLocations[i][0], sotaBombsLocations[i][1], sotaBombsLocations[i][2], sotaBombsLocations[i][3], 1.5f);
            }
            npc[TEAM_HORDE] = SpawnCreature(SOTA_GORGRIL_RIGSPARK, sotaNPCSLocations[1], TeamFactions[Attackers]);
            break;
    }
}

void StrandOfTheAncient::SpawnGraveyard(SOTAGraveyards gyid, uint32 team)
{
    if (gyid >= NUM_SOTA_GRAVEYARDS)
        return;

    SOTAGraveyard &gy = graveyard[gyid];

    gy.faction = team;

    if (gy.spiritguide != nullptr)
        gy.spiritguide->Despawn(0, 0);

    gy.spiritguide = SpawnSpiritGuide(GraveyardLocations[gyid], team);
    AddSpiritGuide(gy.spiritguide);
}
