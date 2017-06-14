/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include <string>
#include <cstdint>

#include "BroadcastMgr.h"
#include "World.h"
#include "Storage/MySQLDataStore.hpp"
#include "Management/LocalizationMgr.h"

#include "Log.hpp"

BroadcastMgr::BroadcastMgr() : mIsRunning(true)
{};

BroadcastMgr::~BroadcastMgr()
{};

void BroadcastMgr::sendBroadcast()
{
    MySQLDataStore::WorldBroadcastContainer* its = sMySQLStore.getWorldBroadcastStore();
    for (MySQLDataStore::WorldBroadcastContainer::iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        if (itr->second.nextUpdate < (uint32_t)UNIXTIME)
        {
            sWorld.sendBroadcastMessageById(itr->second.id);

            uint32_t randomMins = (itr->second.randomInterval ? RandomUInt(itr->second.randomInterval) : 0);
            uint32_t intervalMins = itr->second.interval;

            itr->second.nextUpdate = randomMins + intervalMins + (uint32_t)UNIXTIME;
        }
    }
}

bool BroadcastMgr::runThread()
{
    if (sMySQLStore.getWorldBroadcastStore()->empty())
    {
        worldConfig.broadcast.isSystemEnabled = false;
    }
    else
    {
        LogNotice("BroadcastMgr : Started");
    }

    while (GetThreadState() != THREADSTATE_TERMINATE)
    {
        sendBroadcast();

        if (GetThreadState() == THREADSTATE_TERMINATE)
        {
            break;
        }

        condition.Wait(2500);

        if (mIsRunning == false)
        {
            break;
        }
    }

    return true;
}

void BroadcastMgr::terminate()
{
    mIsRunning = false;
    condition.Signal();
}
