/*
*Copyright (c) 2014-2017 AscEmu Team <http://ww.ascemu.org/>
*This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _TIMER_H
#define _TIMER_H

struct Timer
{
    int32_t Time;
    uint32_t Phase;
};

class TimerMap : private std::multimap<uint32_t, Timer>
{
private:
    Timer timer;

public:
    
    // Resets all Timers
    void Reset();

    // Updates All Timers for the given Phase by xx time
    void Update(int32_t time, uint32_t phase);

    // Adds A timer with the given Id , time and phase
    void AddTimer(uint32_t timerId, int32_t time, uint32_t phase = 0);

	// Executes all Timers
    uint32_t ExecuteTimers(uint32_t phase);

    // Get Timer 
    int32_t GetTimer(uint32_t timerId);

    // Delay all Timers
    void DelayTimers(int32_t delay, uint32 phase);

    // Delete Timer with the given Id
    void DeleteTimer(uint32_t timerId);

    // Search if a Timer is Already Existing
    bool HasTimer(uint32_t timerId);
};

#endif // _TIMER_H
