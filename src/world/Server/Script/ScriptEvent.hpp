/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <map>

struct scriptEvent
{
    int32_t timer;
    uint32_t bossPhase;
};

class SERVER_DECL scriptEventMap
{
    typedef std::multimap< uint32_t, scriptEvent> eventMap;

public:
    scriptEventMap() : bossPhase(0) {}

    void resetEvents()
    {
        eventMapStore.clear();
        bossPhase = 0;
    }

    void updateEvents(int32_t diff, uint32_t phase)
    {
        bossPhase = phase;

        if (!eventMapStore.empty())
        {
            for (eventMap::iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                if (itr->second.bossPhase == bossPhase)
                    itr->second.timer = itr->second.timer - diff;
                else if (itr->second.bossPhase == 0)
                    itr->second.timer = itr->second.timer - diff;

                ++itr;
            }
        }
    }

    void addEvent(uint32_t eventId, int32_t time, uint32_t phase = 0)
    {
        scriptEventData.timer = time;
        scriptEventData.bossPhase = phase;

        removeEvent(eventId);

        eventMapStore.insert(eventMap::value_type(eventId, scriptEventData));
    }

    void removeEvent(uint32_t eventId)
    {
        if (!eventMapStore.empty())
        {
            for (eventMap::const_iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                if (itr->first == eventId)
                {
                    eventMapStore.erase(itr);
                    itr = eventMapStore.begin();
                    break;
                }
                else
                    ++itr;
            }
        }
    }

    // Return the first finished event
    // In order for example if you have multiple event with 10 seconds timer they get executed in event id order --> 1 .... 2 .... 3
    uint32_t getFinishedEvent()
    {
        uint32_t scriptEventId = 0;

        if (!eventMapStore.empty())
        {
            for (eventMap::const_iterator itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                if (itr->second.bossPhase == bossPhase && itr->second.timer <= 0)
                {
                    scriptEventId = itr->first;
                    eventMapStore.erase(itr);
                    return scriptEventId;
                }
                else if (itr->second.bossPhase == 0 && itr->second.timer <= 0)
                {
                    scriptEventId = itr->first;
                    eventMapStore.erase(itr);
                    return scriptEventId;
                }
                else
                    ++itr;
            }
        }
        return scriptEventId;
    }

    void delayEvent(uint32_t eventId, int32_t delay)
    {
        if (!eventMapStore.empty())
        {
            for (auto itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                // Only Delay Timers that are not Finished
                if (itr->second.timer > 0 && itr->first == eventId)
                {
                    itr->second.timer = itr->second.timer + delay;
                    break;
                }
                ++itr;
            }
        }
    }

    void delayAllEvents(int32_t delay, uint32_t phase = 0)
    {
        if (!eventMapStore.empty())
        {
            for (auto itr = eventMapStore.begin(); itr != eventMapStore.end();)
            {
                // Only Delay Timers that are not Finished and in our Current Phase
                if (itr->second.timer > 0 && itr->second.bossPhase == phase)
                    itr->second.timer = itr->second.timer + delay;
                else if (itr->second.timer > 0 && itr->second.bossPhase == 0)
                    itr->second.timer = itr->second.timer + delay;

                ++itr;
            }
        }
    }

private:
    uint32_t bossPhase;
    scriptEvent scriptEventData{};
    eventMap eventMapStore;
};
