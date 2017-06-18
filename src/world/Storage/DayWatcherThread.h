/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DAYWATCHERTHREAD_H
#define DAYWATCHERTHREAD_H

#include "Threading/ConditionVariable.h"
#include "CThreads.h"
#include <ctime>

enum DAYWATCHERSETTINGS
{
    WEEKLY  = 1,
    DAILY   = 2,
    MONTHLY = 3,
    HOURLY  = 4
};

class DayWatcherThread : public CThread
{
        Arcemu::Threading::ConditionVariable cond;

        bool m_running;
        bool m_busy;
        bool m_dirty;

        //static const time_t timeout = 120;        // check every 2 minutes
        time_t currenttime;
        tm local_currenttime;
        time_t last_arena_time;
        tm local_last_arena_time;

        time_t last_daily_time;
        tm local_last_daily_time;

        uint32_t arena_period;
        uint32_t daily_period;

    public:

        DayWatcherThread();
        ~DayWatcherThread();

        bool run();
        void terminate();
        void dupe_tm_pointer(tm* returnvalue, tm* mypointer);
        void load_settings();
        void update_settings();
        void set_tm_pointers();
        uint32_t get_timeout_from_string(const char* string, uint32_t def);
        bool has_timeout_expired(tm* now_time, tm* last_time, uint32_t timeoutval);
        void update_arena();
        void update_daily();
};

#endif // _DAYWATCHERTHREAD_H
