/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include <cstdint>

#include "BroadcastMgr.h"
#include "World.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"

#include "Log.hpp"

using AscEmu::Threading::AEThread;
using std::chrono::milliseconds;
using std::make_unique;

void BroadcastMgr::threadRunner(AEThread& /*thread*/) { sendBroadcast(); }

void BroadcastMgr::threadInit()
{
    if (sMySQLStore.getWorldBroadcastStore()->empty())
    {
        worldConfig.broadcast.isSystemEnabled = false;
    }
    else
    {
        LogNotice("BroadcastMgr : Started");
    }

    m_thread->reboot();
}

BroadcastMgr::BroadcastMgr()
{
    m_thread = make_unique<AEThread>("BroadcastThread", [this](AEThread& thread) { this->threadRunner(thread); }, milliseconds(2500), false);
    this->threadInit();
}

BroadcastMgr::~BroadcastMgr() { m_thread->join(); }

void BroadcastMgr::sendBroadcast()
{
    MySQLDataStore::WorldBroadcastContainer* its = sMySQLStore.getWorldBroadcastStore();
    for (MySQLDataStore::WorldBroadcastContainer::iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        if (itr->second.nextUpdate < (uint32_t)UNIXTIME)
        {
            sWorld.sendBroadcastMessageById(itr->second.id);

            uint32_t randomMins = (itr->second.randomInterval ? Util::getRandomUInt(itr->second.randomInterval) : 0);
            uint32_t intervalMins = itr->second.interval;

            itr->second.nextUpdate = randomMins + intervalMins + (uint32_t)UNIXTIME;
        }
    }
}
