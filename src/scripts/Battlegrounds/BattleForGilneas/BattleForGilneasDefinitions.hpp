/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum
{
    BFG_RESOURCES_WARNING_THRESHOLD = 1400,
    BFG_RESOURCES_WINVAL = 1600,

    BFG_BUFF_RESPAWN_TIME = 90000
};

enum BFGControlPoints
{
    BFG_CONTROL_POINT_LIGHTHOUSE = 0,
    BFG_CONTROL_POINT_WATERWORKS = 1,
    BFG_CONTROL_POINT_MINE = 2,
    BFG_NUM_CONTROL_POINTS = 3
};

enum BFGSpawnTypes
{
    BFG_SPAWN_TYPE_NEUTRAL = 0,
    BFG_SPAWN_TYPE_ALLIANCE_ASSAULT = 1,
    BFG_SPAWN_TYPE_HORDE_ASSAULT = 2,
    BFG_SPAWN_TYPE_ALLIANCE_CONTROLLED = 3,
    BFG_SPAWN_TYPE_HORDE_CONTROLLED = 4,
    BFG_NUM_SPAWN_TYPES = 5
};

// Real world state ids, sourced from the public open-source AzerothCore "Battle for Gilneas"
// module (github.com/azerothcore/mod-bg-battle-for-gilneas, src/BattlegroundBFG.h) - a working,
// independently-implemented Cata BG port whose ids this session cross-checked and trusts over an
// earlier (and apparently wrong) guess inferred from WorldStateUI.dbc's label-string placeholders.
// The per-node ids are a clean match to Arathi Basin's own real per-node ids in WorldStates.hpp
// (Lighthouse reuses AB's Stables slot, Waterworks reuses Blacksmith, Mine reuses Farm) - Blizzard
// evidently repurposed 3 of AB's 5 per-node world state slots rather than minting new ones, the
// same shortcut confirmed for Twin Peaks/WSG. The aggregate ids (occupied bases/resources/max),
// however, are NOT shared with AB - Gilneas has its own dedicated 6200-6204 range for those.
enum BattleForGilneasWorldStates
{
    WORLDSTATE_BFG_ALLIANCE_OCCUPIED_BASES = 6200,
    WORLDSTATE_BFG_HORDE_OCCUPIED_BASES = 6201,
    WORLDSTATE_BFG_ALLIANCE_RESOURCES = 6202,
    WORLDSTATE_BFG_HORDE_RESOURCES = 6203,
    WORLDSTATE_BFG_MAX_SCORE = 6204,
    WORLDSTATE_BFG_RESOURCES_WARNING = 1955,

    // Lighthouse - reuses Arathi Basin's Stables world states
    WORLDSTATE_BFG_CAPTURED_LIGHTHOUSE_ALLIANCE = 1767,
    WORLDSTATE_BFG_CAPTURED_LIGHTHOUSE_HORDE = 1768,
    WORLDSTATE_BFG_CAPTURING_LIGHTHOUSE_ALLIANCE = 1769,
    WORLDSTATE_BFG_CAPTURING_LIGHTHOUSE_HORDE = 1770,
    WORLDSTATE_BFG_SHOW_LIGHTHOUSE_ICON = 1842,

    // Waterworks - reuses Arathi Basin's Blacksmith world states
    WORLDSTATE_BFG_CAPTURED_WATERWORKS_ALLIANCE = 1782,
    WORLDSTATE_BFG_CAPTURED_WATERWORKS_HORDE = 1783,
    WORLDSTATE_BFG_CAPTURING_WATERWORKS_ALLIANCE = 1784,
    WORLDSTATE_BFG_CAPTURING_WATERWORKS_HORDE = 1785,
    WORLDSTATE_BFG_SHOW_WATERWORKS_ICON = 1846,

    // Mine - reuses Arathi Basin's Farm world states
    WORLDSTATE_BFG_CAPTURED_MINE_ALLIANCE = 1772,
    WORLDSTATE_BFG_CAPTURED_MINE_HORDE = 1773,
    WORLDSTATE_BFG_CAPTURING_MINE_ALLIANCE = 1774,
    WORLDSTATE_BFG_CAPTURING_MINE_HORDE = 1775,
    WORLDSTATE_BFG_SHOW_MINE_ICON = 1845
};
