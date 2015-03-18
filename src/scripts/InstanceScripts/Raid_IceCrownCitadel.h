/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _ICECROWNCITADEL_H
#define _ICECROWNCITADEL_H

enum IceCrownCitadelGOs
{
    //GO_Tele
    GO_TELE_1 = 202242,
    GO_TELE_2 = 202243,
    GO_TELE_3 = 202244,
    GO_TELE_4 = 202245,
    GO_TELE_5 = 202246,

    //GO spawn id
    GO_MARROWGAR_ICEWALL_1 = 201911,
    GO_MARROWGAR_ICEWALL_2 = 201910,
    GO_MARROWGAR_DOOR = 201563
};

enum IceCrownCitadelCNentry
{
   CN_LORD_MARROWGAR        = 36612,
   CN_COLDFLAME             = 36672,
   CN_BONE_SPIKE            = 36619,
   CN_LADY_DEATHWHISPER     = 36855,
   CN_VALITHRIA_DREAMWALKER = 36789,
};

// CN ids for Encounter State ?!?
enum IceCrownCitadelCNid
{
   CNID_LORD_MARROWGAR         = 0,
   CNID_COLDFLAME              = 1,
   CNID_LADY_DEATHWHISPER      = 2,
   CNID_VALITHRIA_DREAMWALKER  = 3,
   CNID_END
};

static Location Doors[] =
{
    { -407.35f, 2147.88f, 42.85f, 0 },       //IceWall1
    { -412.97f, 2285.24f, 42.01f, 0 },       //IceWall2
    { -520.44f, 2211.47f, 63.14f, 0 },       //Door behinde ice problem with viewing distance....
};

// Teleport coords for Gossip
static float ICCTeleCoords[6][5] =
{    // moving this to a clean mysql table with all teleports in it... maybe as spell or event?
    { MAP_ICECROWNCITADEL, -17.856115f, 2211.640137f, 30.115812f, 0.0f },     //1   Teleport to Light's Hammer 
    { MAP_ICECROWNCITADEL, -503.632599f, 2211.219971f, 62.823246f, 0.0f },    //2   Teleport to Oratory of The Damned
    { MAP_ICECROWNCITADEL, -615.098267f, 2211.509766f, 199.973083f, 0.0f },   //3   Teleport to Rampart of Skulls 
    { MAP_ICECROWNCITADEL, -549.151001f, 2211.463967f, 539.290222f, 0.0f },   //4   Teleport to Deathbringer's Rise 
    { MAP_ICECROWNCITADEL, 4356.780273f, 2863.636230f, 349.337982f, 0.0f },   //5   Teleport to the Upper Spire.
    { MAP_ICECROWNCITADEL, 4453.248535f, 2769.325684f, 349.347473f, 0.0f }    //6   Teleport to Sindragosa's Lair
};

void SetupICC(ScriptMgr* mgr);

#endif      // _ICECROWNCITADEL_H