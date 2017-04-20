/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "StdAfx.h"
#include "World.h"
#include "Objects/ObjectMgr.h"
#include "CommonScheduleThread.h"

#define THREAD_LOOP_INTERVAL 10 // cebernic: don't modify it

CommonScheduleThread::CommonScheduleThread()
{
    m_running = true;
    BCTimerCount = 0;
    m_busy = false;
}

CommonScheduleThread::~CommonScheduleThread()
{
}

void CommonScheduleThread::terminate()
{
    BCTimerCount = 0;
    m_running = false;

    cond.Signal();
}

bool CommonScheduleThread::run()
{
    LogNotice("CommonScheduleThread : Started.");
    m_busy = false;

    if (worldConfig.broadcast.isSystemEnabled && worldConfig.broadcast.orderMode == 1)
        itOrderMSGEntry = objmgr.GetBCTotalItemBegin();

    if (objmgr.IsBCEntryStorageEmpty())
        worldConfig.broadcast.isSystemEnabled = 0;

    BCTimerCount = getMSTime() + ((uint32)worldConfig.broadcast.interval * 1000);

    while (GetThreadState() != THREADSTATE_TERMINATE)
    {
        m_busy = true;
        // do job
        BroadCastExec();
        m_busy = false;
        if (GetThreadState() == THREADSTATE_TERMINATE)
            break;

        cond.Wait(THREAD_LOOP_INTERVAL * 1000);

        if (!m_running)
            break;
    }

    return true;
}

void CommonScheduleThread::BroadCastExec()
{
    if (!worldConfig.broadcast.isSystemEnabled)
        return;

    if ((uint32)worldConfig.broadcast.interval > THREAD_LOOP_INTERVAL)
    {
        if (getMSTime() <= BCTimerCount)
            return;
        else
            BCTimerCount = getMSTime() + ((uint32)worldConfig.broadcast.interval * 1000);
    }

    switch (worldConfig.broadcast.orderMode)
    {
        case 0:
        {
            int entry = objmgr.CalcCurrentBCEntry();

            if (entry < 0)
            {
                worldConfig.broadcast.isSystemEnabled = false;
                LogNotice("BCSystem : table worldbroadcast loads failed,so BCSystem disabled already.");
                return;
            }

            if (entry == 0)
                return;
            else
                sWorld.sendBroadcastMessageById(entry);
        }
        break;
        case 1:
        {
            if (itOrderMSGEntry == objmgr.GetBCTotalItemEnd())
                itOrderMSGEntry = objmgr.GetBCTotalItemBegin();

            sWorld.sendBroadcastMessageById(itOrderMSGEntry->second);

            ++itOrderMSGEntry;
        }
        break;
        default:
            return;
    }
}
