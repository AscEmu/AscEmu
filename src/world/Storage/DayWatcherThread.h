/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <ctime>

enum DAYWATCHERSETTINGS
{
    WEEKLY  = 1,
    DAILY   = 2,
    MONTHLY = 3,
    HOURLY  = 4
};

class DayWatcherThread
{
    bool m_updateDBSettings;

    time_t m_currenttime;
    tm m_localCurrenttime;
    time_t m_lastArenaTime;
    tm m_localLastArenaTime;

    time_t m_lastDailyTime;
    tm m_localLastDailyTime;

    uint32_t m_arenaPeriod;
    uint32_t m_dailyPeriod;

    std::unique_ptr<AscEmu::Threading::AEThread> m_thread;

    void threadInit();
    void threadRunner(AscEmu::Threading::AEThread& thread);

public:

    DayWatcherThread();
    ~DayWatcherThread();

    static void dupe_tm_pointer(tm* returnvalue, tm* mypointer);
    void load_settings();
    void update_settings();
    void set_tm_pointers();
    uint32_t get_timeout_from_string(std::string string, uint32_t def);
    bool has_timeout_expired(tm* now_time, tm* last_time, uint32_t timeoutval);
    void update_arena();
    void update_daily();
};
