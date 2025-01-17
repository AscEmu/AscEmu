/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

class Unit;
class AIInterface;

typedef void(AIInterface::*pAIEvent)(Unit* punit, uint32_t misc1);

enum AiEvents
{
    EVENT_ENTERCOMBAT,
    EVENT_LEAVECOMBAT,
    EVENT_DAMAGETAKEN,
    EVENT_FEAR,
    EVENT_UNFEAR,
    EVENT_FOLLOWOWNER,
    EVENT_WANDER, // unused
    EVENT_UNWANDER, // unused
    EVENT_UNITDIED,
    EVENT_HOSTILEACTION,
    EVENT_FORCEREDIRECTED, // unused
    NUM_AI_EVENTS
};

extern pAIEvent AIEventHandlers[NUM_AI_EVENTS];
