/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LocationVector.h"

namespace Deadmines
{
    enum Encounters
    {
        // Encounters
        BOSS_RHAHKZOR           = 0,
        BOSS_SNEED              = 1,
        BOSS_GILNID             = 2,
        BOSS_MR_SMITE           = 3,
        BOSS_GREENSKIN          = 4,
        BOSS_VANCLEEF           = 5,
        BOSS_COOKIE             = 6,

        // Additional
        EVENT_STATE             = 7
    };

    static LocationVector Doors[] =
    {
        { -168.514f, -579.861f, 19.3159f, 0 },  // Gilnid doors
        { -290.294f, -536.96f, 49.4353f, 0 }    // Sneed doors
    };

    //it will useful for cannon event
    static LocationVector Guards[] =
    {
        { -89.7001f, -691.332f, 8.24514f, 0 }, //Guard
        { -102.521f, -697.942f, 8.84454f, 0 }, //Guard
        { -89.6744f, -694.063f, 8.43202f, 0 }  //Parrot
    };

    enum CreatureEntry
    {
        NPC_EDWIN_VANCLEEF = 639,
        NPC_SNEEDS_SHREDDER = 642,
        NPC_SNEED = 643,
        NPC_RHAHK_ZOR = 644,
        NPC_MR_SMITE = 646,
        NPC_GUARD1 = 657,    //Pirate
        NPC_GILNID = 1763,
        NPC_GUARD2 = 3450    //Parrot
    };

    enum CreatureSpells
    {
        //MrSmiteAI
        SMITE_STOMP = 6432,
        SMITES_HAMMER = 6436,
        SMITE_SLAM = 6435
    };

    enum DeadMinesGOIDs
    {
        GO_FACTORY_DOOR = 13965,
        GO_IRONCLAD_DOOR = 16397,
        GO_DEFIAS_CANNON = 16398,
        GO_HEAVY_DOOR = 17153,
        GO_FACTORY_DOOR_LEVER = 101831,
        GO_SNEED_DOOR_LEVER = 101832,
        GO_IRONCLAD_LEVER = 101833,
        GO_GILNID_DOOR_LEVER = 101834,
        GO_MR_SMITE_CHEST = 144111
    };
}
