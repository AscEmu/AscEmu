/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Shirrak the Dead WatcherAI
    CN_SHIRRAK_THE_DEAD_WATCHER = 18371,

    // Avatar of the MartyredAI
    CN_AVATAR_OF_THE_MARTYRED   = 18478,

    // Exarch MaladaarAI
    CN_EXARCH_MALADAAR          = 18373,

};

enum CreatureSpells
{
    // Shirrak the Dead WatcherAI
    INHIBIT_MAGIC           = 32264,
    CARNIVOROUS_BITE        = 41092,    // Also can be: 36383 or 41092
    FOCUS_FIRE              = 32310,    // not fully functional for now =/ Let's try: 32310, 32301 or 32300    - needs further researches
    ATTRACT_MAGIC           = 32265,    // doesn't work anyway

    // Avatar of the MartyredAI
    SUNDER_ARMOR            = 16145,
    MORTAL_STRIKE           = 15708,    // not sure to spells ofc :)
    PHASE_IN                = 33422,

    // Exarch MaladaarAI
    SOUL_SCREAM             = 32421,
    RIBBON_OF_SOULS         = 32422,
    STOLEN_SOUL             = 32346,
    SUMMON_AVATAR           = 32424,
    //SOUL_CLEAVE             = 32346,

};

enum CreatureSay
{
    // Exarch MaladaarAI
    SAY_MALADAAR_01         = 4567,     // You will pay with your life!
    SAY_MALADAAR_02         = 4568,     // There is no turning back now!
    SAY_MALADAAR_03         = 4569,     // Serve your penitence!
    SAY_MALADAAR_04         = 4572,     // These walls will be your tomb! old: These walls will be your DOOM!
    SAY_MALADAAR_05         = 4573,     // Now you'll stay... for eternity. old: Haha, now you'll stay for eternity! Mwahahah!
    SAY_MALADAAR_06         = 4574,     // This is... where I belong.
    SAY_MALADAAR_07         = 4566,     // You have defiled the resting place of our ancestors. For this offense, there can be but one punishment...

};
