/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum OHF_ENTRIES
{
    MAP_OLD_HILSBRAD = 560,
};

enum Data
{
    OHF_DATA_NOT_STARTED = 1,
    OHF_DATA_IN_PROGRESS = 2,
    OHF_DATA_PERFORMED = 3,
    OHF_DATA_DONE = 4
};

enum DataIndex
{
    OHF_PHASE_1 = 0, // pre bosss spawn
    OHF_PHASE_2 = 1, // 1st boss
    OHF_PHASE_3 = 2, // pre 2nd boss, trall escort part 1
    OHF_PHASE_4 = 3, // 2nd boss
    OHF_PHASE_5 = 4, // pre 3rd boss, trall escort part 2
    OHF_PHASE_6 = 5, // 3rd boss
    OHF_PHASE_DONE = 6, // Event done

    OHF_END = 7
};

enum CreatureEntry
{
    CN_EROZION = 18723,
    CN_BRAZEN = 18725,
    CN_LIEUTENANT_DRAKE = 17848,
    CN_THRALL = 17876

};

enum CreatureSpells
{

};

enum CreatureSay
{

};

enum GameObjectEntry
{
    GO_LODGE_ABLAZE = 182589,
    GO_FIRE = 183816,
};

enum eGossipTexts
{
    EROZION_ON_HELLO = 10475,
    EROZION_ON_FINISH = 10474,
    BRAZAN_ON_HELLO = 9779,
    BRAZAN_NEED_ITEM = 9780,
    THRALL_ON_HELLO = 9568,

    //GossipMenu
    EROZION_NEED_PACKET     = 425,
    BRAZAN_DURNHOLDE_KEEP   = 426,
    THRALL_START_ESCORT     = 427,
};
