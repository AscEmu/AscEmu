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
        void loadCustomWaypoints();

        // Returns the path from a given id
        WaypointPath* getPath(uint32_t id);
        WaypointPath* getCustomScriptWaypointPath(uint32_t id);

        void addWayPoint(uint32_t pathid, WaypointNode waypoint);
        void deleteWayPointById(uint32_t pathid, uint32_t waypointId);
        void deleteAllWayPoints(uint32_t pathid);

    private:
        WaypointMgr() { }

        std::unordered_map<uint32_t, WaypointPath> _waypointStore;
        std::unordered_map<uint32_t, WaypointPath> _customWaypointStore;
};

#define sWaypointMgr WaypointMgr::getInstance()
