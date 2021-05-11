/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WaypointManager.h"

void WaypointMgr::load()
{
    auto oldMSTime = Util::TimeNow();

    //                                                0    1         2           3          4            5           6        7      8           9
    QueryResult* result = WorldDatabase.Query("SELECT id, point, position_x, position_y, position_z, orientation, move_type, delay, action, action_chance FROM waypoint_data ORDER BY id, point");

    if (!result)
    {
        sLogger.info("WaypointMgr >> Loaded 0 waypoints. DB table `waypoint_data` is empty!");
        return;
    }

    uint32_t count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32_t pathId = fields[0].GetUInt32();
        float x = fields[2].GetFloat();
        float y = fields[3].GetFloat();
        float z = fields[4].GetFloat();
        float o = fields[5].GetFloat();

        normalizeMapCoord(x);
        normalizeMapCoord(y);

        WaypointNode waypoint;
        waypoint.id = fields[1].GetUInt32();
        waypoint.x = x;
        waypoint.y = y;
        waypoint.z = z;
        waypoint.orientation = o;
        waypoint.moveType = fields[6].GetUInt32();

        if (waypoint.moveType >= WAYPOINT_MOVE_TYPE_MAX)
        {
            sLogger.failure("WaypointMgr Waypoint %u in waypoint_data has invalid move_type, ignoring", waypoint.id);
            continue;
        }

        waypoint.delay = fields[7].GetUInt32();
        waypoint.eventId = fields[8].GetUInt32();
        waypoint.eventChance = fields[9].GetInt16();

        WaypointPath& path = _waypointStore[pathId];
        path.id = pathId;
        path.nodes.push_back(std::move(waypoint));
        ++count;
    }
    while (result->NextRow());

    sLogger.info("WaypointMgr >> Loaded %u waypoints in %u ms", count, Util::GetTimeDifferenceToNow(oldMSTime));
}

WaypointMgr* WaypointMgr::getInstance()
{
    static WaypointMgr mInstance;
    return &mInstance;
}

WaypointPath const* WaypointMgr::getPath(uint32_t id) const
{
    auto itr = _waypointStore.find(id);
    if (itr != _waypointStore.end())
        return &itr->second;

    return nullptr;
}
