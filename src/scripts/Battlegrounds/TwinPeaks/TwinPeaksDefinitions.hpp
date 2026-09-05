/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum
{
    TP_TIME_LEFT = 25,

    TP_BUFF_RESPAWN_TIME = 90000,

    // Twin Peaks reuses Warsong Gulch's own generic dropped-flag props (same client model,
    // position is set dynamically on drop) - real spawn-confirmed entries, not a guess.
    ALLIANCE_FLAG_DROP = 179785,
    HORDE_FLAG_DROP = 179786,
};

// Real, spawn-confirmed AreaTrigger.dbc entries for map 726 - matched to their real gameobject
// counterparts by coordinate (each trigger sits within a few yards of the GO it corresponds to).
enum TwinPeaksAreaTriggers
{
    AREATRIGGER_TP_A_SPEED = 5906,
    AREATRIGGER_TP_H_SPEED = 5907,
    AREATRIGGER_TP_H_RESTORATION = 5908,
    AREATRIGGER_TP_A_RESTORATION = 5909,
    AREATRIGGER_TP_H_BERSERKING = 5910,
    AREATRIGGER_TP_A_BERSERKING = 5911,
    AREATRIGGER_TP_A_SPAWN = 5904,
    AREATRIGGER_TP_H_SPAWN = 5905,
    // Remaining real map-726 AreaTrigger.dbc entries not tied to a buff or a base - encounter/
    // boundary triggers the same way WSG's own ENCOUNTER_01-04 are handled (no-op).
    AREATRIGGER_TP_ENCOUNTER_01 = 5914,
    AREATRIGGER_TP_ENCOUNTER_02 = 5916,
    AREATRIGGER_TP_ENCOUNTER_03 = 5917,
    AREATRIGGER_TP_ENCOUNTER_04 = 5918,
    AREATRIGGER_TP_ENCOUNTER_05 = 5920,
    AREATRIGGER_TP_ENCOUNTER_06 = 5921,
    AREATRIGGER_TP_ENCOUNTER_07 = 6803,
    AREATRIGGER_TP_ENCOUNTER_08 = 6804,
    AREATRIGGER_TP_ENCOUNTER_09 = 6805,
    AREATRIGGER_TP_ENCOUNTER_10 = 6806
};

// Twin Peaks' client UI panel keys off the exact same world state ids as Warsong Gulch - confirmed
// via WorldStateUI.dbc (map 726, zone 5031 rows point at world states 1581/1582/1601/2338/2339/
// 4247/4248, byte-identical to WSG's own map-489 rows). Blizzard never minted separate ids for
// this "reskinned WSG" map, so this file has no world state enum of its own - use
// WORLDSTATE_WSG_ALLIANCE_SCORE / WORLDSTATE_WSG_HORDE_SCORE / WORLDSTATE_WSG_MAX_SCORE /
// WORLDSTATE_WSG_ALLIANCE_FLAG_DISPLAY / WORLDSTATE_WSG_HORDE_FLAG_DISPLAY /
// WORLDSTATE_WSG_TIME_ENABLED / WORLDSTATE_WSG_TIME_LEFT from WorldStates.hpp directly.
