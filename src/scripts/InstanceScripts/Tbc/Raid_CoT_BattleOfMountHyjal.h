/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum
{
    CN_JAINA_PROUDMOORE = 17772,

    CN_THRALL = 17852,

    CN_RAGE_WINTERCHILL = 17767,

    FROSTBOLT = 31249, // it's not correct spell for sure (not sure to others too :P)
    DEATCH_AND_DECAY = 31258,
    FROST_NOVA = 31250,
    FROST_ARMOR = 31256,

    CN_ANETHERON = 17808,

    //CARRION_SWARM = 31306,
    VAMPIRIC_AURA = 38196, // 31317
    INFERNO = 31299, // doesn't summon infernal - core bug
    SLEEP = 31298, // 12098
    BERSERK = 26662,

    CN_KAZROGAL = 17888,

    K_CLEAVE = 31345,
    WAR_STOMP = 31480,
    MARK_OF_KAZROGAL = 31447,
    MARK_OF_KAZROGAL2 = 31463, // should it be scripted to attack friends?

    CN_AZGALOR = 17842,

    CLEAVE = 31345,
    RAIN_OF_FIRE = 31340,
    HOWL_OF_AZGALOR = 31344,
    DOOM = 31347, // it's applied, but doesn't do anything more - should be scripted?

    CN_ARCHIMONDE_CHANNEL_TRIGGER = 30004,

    // Additional
    DRAIN_WORLD_TREE_VISUAL = 39140,
    DRAIN_WORLD_TREE_VISUAL2 = 39141,

    CN_DOOMFIRE = 18095,

    CN_ARCHIMONDE = 17968,

    FEAR = 33547,
    AIR_BURST = 32014,
    GRIP_OF_THE_LEGION = 31972,
    DOOMFIRE_STRIKE = 31903,
    //FINGER_OF_DEATH = 31984, // should be casted only when no one in melee range
    //HAND_OF_DEATH = 35354, // used if too close to Well of Eternity or if after 10 min caster has more than 10% hp
    //SOUL_CHARGER = 32053, // If player dies whole raid gets one of those 3 buffs
    //SOUL_CHARGEO = 32054,
    //SOUL_CHARGEG = 32057,
};

enum HyjalEvents
{
    HYJAL_EVENT_RAGE_WINTERCHILL,
    HYJAL_EVENT_ANETHERON,
    HYJAL_EVENT_KAZROGAL,
    HYJAL_EVENT_AZGALOR,
    HYJAL_EVENT_ARCHIMONDE
};

enum HyjalPhases
{
    HYJAL_PHASE_NOT_STARTED = 0,
    HYJAL_PHASE_RAGE_WINTERCHILL_COMPLETE,
    HYJAL_PHASE_ANETHERON_COMPLETE,
    HYJAL_PHASE_KAZROGAL_COMPLETE,
    HYJAL_PHASE_AZGALOR_COMPLETE,
    HYJAL_PHASE_ARCHIMONDE_COMPLETE,
};

enum HyjalType
{
    HYJAL_TYPE_BASIC = 0,
    HYJAL_TYPE_END
};

enum CreatureEntry
{
};

enum CreatureSpells
{
};
