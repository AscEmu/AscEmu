/*
*Copyright (c) 2014-2017 AscEmu Team <http://ww.ascemu.org/>
*This file is released under the MIT license. See README-MIT for more information.
*/

#include "AITimer.h"

// Resets all Timers
void TimerMap::Reset()
{ 
    clear();
}

// Updates All Timers for the given Phase by xx time
void TimerMap::Update(int32_t time, uint32_t phase)
{
    if (!empty())
    {
        for (auto itr = begin(); itr != end();)
        {
            // Timers with Phases
            if (itr->second.Phase == phase)
            {
                itr->second.Time = itr->second.Time - time;      
            }
            else if (itr->second.Phase == 0)
            {
                itr->second.Time = itr->second.Time - time;
            }
            ++itr;
        }
    }
}

// Adds A timer with the given Id , time and phase
void TimerMap::AddTimer(uint32_t timerId, int32_t time, uint32_t phase)
{
    timer.Time = time;
    timer.Phase = phase;

    // When There is already a Timer with the same id in the storage delete them and overwrite it.
    if (!empty())
    {
        auto itr = find(timerId);
        if (itr != end())
        {
            if (itr->first == timerId)
            {
                erase(itr);
            }
        }
    }

    // Insert Timers
    insert(TimerMap::value_type(timerId, timer));
}

uint32_t TimerMap::ExecuteTimers(uint32_t phase)
{
    if (!empty())
    {
        for (auto itr = begin(); itr != end();)
        {
            // Timers with Phases
            if (itr->second.Time <= 0 && itr->second.Phase == phase)
            {
                uint32_t timerId;
                timerId = itr->first;
                erase(itr);
                return timerId;
            }
            else if (itr->second.Time <= 0 && itr->second.Phase == 0)
            {
                uint32_t timerId;
                timerId = itr->first;
                erase(itr);
                return timerId;
            }
            ++itr;
        }
    }
    return 0;
}

// Get Timer 
int32_t TimerMap::GetTimer(uint32_t timerId)
{
    if (!empty())
    {
        auto itr = find(timerId);
        if (itr != end())
            return itr->second.Time;
    }
    return 0;
}

// Delay all Timers
void TimerMap::DelayTimers(int32_t delay, uint32_t phase)
{
    if (!empty())
    {
        for (auto itr = begin(); itr != end();)
        {
            // Only Delay Timers that are not Finished and in our Current Phase
            if (itr->second.Time > 0 && itr->second.Phase == phase)
                itr->second.Time = itr->second.Time + delay;
            else if (itr->second.Time > 0 && itr->second.Phase == 0)
                itr->second.Time = itr->second.Time + delay;
              
            ++itr;
        }
    }
}

// Delete Timer with the given Id
void TimerMap::DeleteTimer(uint32_t timerId)
{
    auto itr = find(timerId);
    if (itr != end())
    {
        if (itr->first == timerId)
        {
            erase(itr);
        }
    }
}

// Search if a Timer is Already Existing
bool TimerMap::HasTimer(uint32_t timerId)
{
    if (!empty())
    {
        auto itr = find(timerId);
        if (itr != end())
        {
            if (itr->first == timerId)
            {
                return true;
            }
        }
    }
    return false;
}
