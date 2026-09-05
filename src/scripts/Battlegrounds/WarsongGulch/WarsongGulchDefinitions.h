/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum
{
    TIME_LEFT = 25,

    BUFF_RESPAWN_TIME = 90000,

    SILVERWING_FLAG = 179785,

    WARSONG_FLAG = 179786,

    // WotLK mechanic: once BOTH flags are away from their
    // bases at the same time, both carriers get Focused Assault after 10 minutes (+50% damage
    // taken, movement speed capped at 100%), which is replaced by Brutal Assault after 15 minutes
    // total (+100% damage taken, same speed cap). The debuff persists across drops/pickups until
    // the flag actually makes it back to base.
    TIME_FOCUSED_ASSAULT_MS = 10 * 60 * 1000,
    TIME_BRUTAL_ASSAULT_MS = 15 * 60 * 1000,

    SPELL_FOCUSED_ASSAULT = 46392,
    SPELL_BRUTAL_ASSAULT = 46393,
};

enum WarsongGulchAreaTriggers
{
    AREATRIGGER_A_SPEED = 3686,
    AREATRIGGER_H_SPEED = 3687,
    AREATRIGGER_A_RESTORATION = 3706,
    AREATRIGGER_H_RESTORATION = 3708,
    AREATRIGGER_A_BERSERKING = 3707,
    AREATRIGGER_H_BERSERKING = 3709,
    AREATRIGGER_WSG_ENCOUNTER_01 = 3649,
    AREATRIGGER_WSG_ENCOUNTER_02 = 3688,
    AREATRIGGER_WSG_ENCOUNTER_03 = 4628,
    AREATRIGGER_WSG_ENCOUNTER_04 = 4629,
    AREATRIGGER_WSG_A_SPAWN = 3646,
    AREATRIGGER_WSG_H_SPAWN = 3647
};
