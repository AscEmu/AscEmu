/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ScriptEvent.hpp"

void scriptEventMap::resetEvents()
{
    eventMapStore.clear();
    bossPhase = 0;
}

void scriptEventMap::updateEvents(uint32_t _diff, uint32_t _phase)
{
    bossPhase = _phase;

    if (!eventMapStore.empty())
    {
        for (EventMap::iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
        {
            auto& scriptEvent = itr->second;
            if (scriptEvent.bossPhase == 0 || scriptEvent.bossPhase == bossPhase)
            {
                if (scriptEvent.timer > _diff)
                    scriptEvent.timer -= _diff;
                else
                    scriptEvent.timer = 0;
            }

            ++itr;
        }
    }
}

void scriptEventMap::addEvent(uint32_t _eventId, uint32_t _time, uint32_t _phase)
{
    scriptEventData.timer = _time;
    scriptEventData.bossPhase = _phase;

    removeEvent(_eventId);

    eventMapStore.insert(EventMap::value_type(_eventId, scriptEventData));
}

void scriptEventMap::removeEvent(uint32_t _eventId)
{
    if (!eventMapStore.empty())
    {
        for (EventMap::const_iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
        {
            if (itr->first == _eventId)
            {
                eventMapStore.erase(itr);
                itr = eventMapStore.begin();
                break;
            }

            ++itr;
        }
    }
}

// Return the first finished event
// In order for example if you have multiple event with 10 seconds timer they get executed in event id order --> 1 .... 2 .... 3
uint32_t scriptEventMap::getFinishedEvent()
{
    uint32_t scriptEventId = 0;

    if (!eventMapStore.empty())
    {
        for (EventMap::const_iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
        {
            if ((itr->second.bossPhase == 0 || itr->second.bossPhase == bossPhase) && itr->second.timer == 0)
            {
                scriptEventId = itr->first;
                eventMapStore.erase(itr);
                return scriptEventId;
            }

            ++itr;
        }
    }
    return scriptEventId;
}

void scriptEventMap::delayEvent(uint32_t _eventId, uint32_t _delay)
{
    if (!eventMapStore.empty())
    {
        for (auto itr = eventMapStore.begin(); itr != eventMapStore.end();)
        {
            // Only Delay Timers that are not Finished
            if (itr->second.timer > 0 && itr->first == _eventId)
            {
                itr->second.timer = itr->second.timer + _delay;
                break;
            }

            ++itr;
        }
    }
}

void scriptEventMap::delayAllEvents(uint32_t _delay, uint32_t _phase)
{
    if (!eventMapStore.empty())
    {
        for (auto itr = eventMapStore.begin(); itr != eventMapStore.end();)
        {
            // Only Delay Timers that are not Finished and in our Current Phase
            if (itr->second.timer > 0 && (itr->second.bossPhase == 0 || itr->second.bossPhase == _phase))
                itr->second.timer = itr->second.timer + _delay;

            ++itr;
        }
    }
}
