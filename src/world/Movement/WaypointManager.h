/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WaypointDefines.h"

#include <vector>
#include <unordered_map>

class SERVER_DECL WaypointMgr
{
    public:
        static WaypointMgr* getInstance();

        // Loads all paths from database, should only run on startup
        void load();

        // Returns the path from a given id
        WaypointPath const* getPath(uint32_t id) const;

    private:
        WaypointMgr() { }

        std::unordered_map<uint32_t, WaypointPath> _waypointStore;
};

#define sWaypointMgr WaypointMgr::getInstance()
