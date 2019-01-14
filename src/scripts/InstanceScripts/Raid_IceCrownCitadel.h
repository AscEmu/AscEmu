/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// Allianze & Horde spawns
struct ICCSpawns
{
    uint32 entry;
    float x;
    float y;
    float z;
    float o;
    uint32 faction;
};

const ICCSpawns AllySpawns[13] = {
    // Light's Hammer
    { 37190, -47.59f, 2208.98f, 27.99f, 3.12f, 1732 },      // Commander
    { 37190, -47.93f, 2216.32f, 27.99f, 3.12f, 1732 },      // Commander
    { 37190, -46.08f, 2212.61f, 27.99f, 3.12f, 1732 },      // Commander
    { 37119, -48.92f, 2213.55f, 27.99f, 3.23f, 2073 },      // Highlord Tirion Fordring
    { 37200, -48.81f, 2211.06f, 27.99f, 3.14f, 1732 },      // Muradin Bronzebart
    { 39371, -49.00f, 2219.47f, 27.99f, 3.12f, 1733 },      // King Varian Wrynn <King of Stormwind>
    { 37997, -79.44f, 2269.37f, 30.66f, 5.3f, 1732 },       // Yili <Rogue Armor>
    { 37998, -70.96f, 2269.32f, 30.66f, 4.46f, 1732 },      // Talan Moonstrike <Hunter Armor>
    { 37999, -75.84f, 2270.65f, 30.66f, 4.92f, 1732 },      // Alana Moonstrike <Druid Armor>
    { 38182, -63.37f, 2260.46f, 30.65f, 1.83f, 1732 },      // Niby the Almighty <Warlock Armor>
    { 38283, -75.8f, 2283.46f, 32.87f, 4.69f, 1732 },       // Malfus Grimfrost <Mage Armor>
    { 38840, -67.79f, 2270.71f, 30.65f, 4.91f, 1732 },      // Jedebia <Shaman Armor>
    //Ramparts of Skull
    { 37200, -555.96f, 2211.4f, 539.37f, 6.27f, 1732 },      // Muradin Bronzebart
};

const ICCSpawns HordeSpawns[13] = {
    // Light's Hammer
    { 37189, -47.59f, 2208.98f, 27.99f, 3.12f, 1734 },      // General
    { 37189, -47.93f, 2216.32f, 27.99f, 3.12f, 1734 },      // General
    { 37189, -46.08f, 2212.61f, 27.99f, 3.12f, 1734 },      // General
    { 39372, -49.00f, 2219.47f, 27.99f, 3.12f, 1734 },      // Garosh Hellscream
    { 37187, -48.81f, 2211.06f, 27.99f, 3.14f, 1734 },      // Highlord Overloard Saurfang
    { 37119, -48.92f, 2213.55f, 27.99f, 3.23f, 2073 },      // Highlord Tirion Fordring
    { 37991, -79.42f, 2269.39f, 30.65f, 5.62f, 1734 },      // Ikfirus the Vile <Rogue Armor>
    { 37992, -75.76f, 2270.64f, 30.65f, 4.85f, 1734 },      // Tortunok <Druid Armor>
    { 37993, -70.92f, 2269.2f, 30.65f, 4.45f, 1734 },       // Gerardo the Suave <Hunter Armor>
    { 38181, -63.37f, 2260.46f, 30.65f, 1.83f, 1734 },      // Haragg the Unseen <Warlock Armor>
    { 38284, -75.81f, 2283.34f, 32.87f, 4.74f, 1734 },      // Uvlus Banefire <Mage Armor>
    { 38841, -67.77f, 2270.51f, 30.65f, 4.93f, 1734 },      // Vol'guk <Shaman Armor>
    //Ramparts of Skull
    { 37187, -555.96f, 2211.4f, 539.37f, 6.27f, 1734 },      // Highlord Overloard Saurfang
};

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
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

static Movement::Location Doors[] =
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
