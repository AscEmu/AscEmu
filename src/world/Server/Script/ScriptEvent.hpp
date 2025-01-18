/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <map>

class SERVER_DECL scriptEventMap
{
    struct ScriptEvent
    {
        uint32_t timer;
        uint32_t bossPhase;
    };

    typedef std::multimap<uint32_t, ScriptEvent> EventMap;

public:
    scriptEventMap() : bossPhase(0) {}

    void resetEvents();
    void updateEvents(uint32_t _diff, uint32_t _phase);

    void addEvent(uint32_t _eventId, uint32_t _time, uint32_t _phase = 0);
    void removeEvent(uint32_t _eventId);

    // Return the first finished event
    // In order for example if you have multiple event with 10 seconds timer they get executed in event id order --> 1 .... 2 .... 3
    uint32_t getFinishedEvent();

    void delayEvent(uint32_t _eventId, uint32_t _delay);
    void delayAllEvents(uint32_t _delay, uint32_t _phase = 0);

private:
    uint32_t bossPhase;
    ScriptEvent scriptEventData{};
    EventMap eventMapStore;
};
