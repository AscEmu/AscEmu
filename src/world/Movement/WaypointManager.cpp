/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WaypointManager.h"
#include "Utilities/Util.hpp"
#include "Logging/Logger.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/MovementInfo.hpp"
#include "Server/DatabaseDefinition.hpp"

void WaypointMgr::load()
{
    auto oldMSTime = Util::TimeNow();
    _waypointStore.clear();

    //                                                 0    1         2           3          4            5           6        7      8           9
    auto result = sMySQLStore.getWorldDBQuery("SELECT id, point, position_x, position_y, position_z, orientation, move_type, delay, action, action_chance FROM creature_waypoints ORDER BY id, point");

    if (!result)
    {
        sLogger.info("WaypointMgr : Loaded 0 waypoints. DB table `creature_waypoints` is empty!");
        return;
    }

    uint32_t count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32_t pathId = fields[0].asUint32();
        float x = fields[2].asFloat();
        float y = fields[3].asFloat();
        float z = fields[4].asFloat();
        float o = fields[5].asFloat();

        normalizeMapCoord(x);
        normalizeMapCoord(y);

        WaypointNode waypoint;
        waypoint.id = fields[1].asUint32();
        waypoint.x = x;
        waypoint.y = y;
        waypoint.z = z;
        waypoint.orientation = o;
        waypoint.moveType = fields[6].asUint32();

        if (waypoint.moveType >= WAYPOINT_MOVE_TYPE_MAX)
        {
            sLogger.failure("WaypointMgr Waypoint {} in creature_waypoints has invalid move_type, ignoring", waypoint.id);
            continue;
        }

        waypoint.delay = fields[7].asUint32();
        waypoint.eventId = fields[8].asUint32();
        waypoint.eventChance = fields[9].asInt8();

        WaypointPath& path = _waypointStore[pathId];
        path.id = pathId;
        path.nodes.push_back(std::move(waypoint));
        ++count;
    }
    while (result->NextRow());

    sLogger.info("WaypointMgr : Loaded {} waypoints in {} ms", count, Util::GetTimeDifferenceToNow(oldMSTime));
}

void WaypointMgr::loadCustomWaypoints()
{
    auto oldMSTime = Util::TimeNow();

    //                                         0    1         2           3          4            5           6        7      8           9
    auto result = WorldDatabase.Query("SELECT id, point, position_x, position_y, position_z, orientation, move_type, delay, action, action_chance FROM creature_script_waypoints ORDER BY id, point");

    if (!result)
    {
        sLogger.info("WaypointMgr : Loaded 0 waypoints. DB table `creature_script_waypoints` is empty!");
        return;
    }

    uint32_t count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32_t pathId = fields[0].asUint32();
        float x = fields[2].asFloat();
        float y = fields[3].asFloat();
        float z = fields[4].asFloat();
        float o = fields[5].asFloat();

        normalizeMapCoord(x);
        normalizeMapCoord(y);

        WaypointNode waypoint;
        waypoint.id = fields[1].asUint32();
        waypoint.x = x;
        waypoint.y = y;
        waypoint.z = z;
        waypoint.orientation = o;
        waypoint.moveType = fields[6].asUint32();

        if (waypoint.moveType >= WAYPOINT_MOVE_TYPE_MAX)
        {
            sLogger.failure("WaypointMgr Waypoint {} in creature_waypoints has invalid move_type, ignoring", waypoint.id);
            continue;
        }

        waypoint.delay = fields[7].asUint32();
        waypoint.eventId = fields[8].asUint32();
        waypoint.eventChance = fields[9].asInt8();

        WaypointPath& path = _waypointStore[pathId];
        path.id = pathId;
        path.nodes.push_back(std::move(waypoint));
        ++count;
    } while (result->NextRow());

    sLogger.info("WaypointMgr : Loaded {} custom waypoints in {} ms", count, Util::GetTimeDifferenceToNow(oldMSTime));
}

WaypointMgr* WaypointMgr::getInstance()
{
    static WaypointMgr mInstance;
    return &mInstance;
}

WaypointPath* WaypointMgr::getPath(uint32_t id)
{
    auto itr = _waypointStore.find(id);
    if (itr != _waypointStore.end())
        return &itr->second;

    return nullptr;
}

WaypointPath* WaypointMgr::getCustomScriptWaypointPath(uint32_t id)
{
    auto itr = _customWaypointStore.find(id);
    if (itr != _customWaypointStore.end())
        return &itr->second;

    return nullptr;
}

uint32_t WaypointMgr::generateWaypointPathId()
{
    auto result = sMySQLStore.getWorldDBQuery("SELECT MAX(id) FROM creature_waypoints");
    if (result)
    {
        uint32_t maxPathId = result->Fetch()[0].asUint32();

        WaypointPath& path = _waypointStore[maxPathId];
        path.id = maxPathId;
        path.nodes.clear();

        return maxPathId + 1;
    }

    return 0;
}

void WaypointMgr::addWayPoint(uint32_t pathid, WaypointNode waypoint, bool saveToDB /*=false*/)
{
    WaypointPath& path = _waypointStore[pathid];
    path.id = pathid;
    path.nodes.push_back(std::move(waypoint));

    if (saveToDB)
        WorldDatabase.Execute("INSERT INTO creature_waypoints VALUES(%u, %u, %f, %f, %f, %f, %u, %u, %u, %u, %u)", pathid, waypoint.id, waypoint.x, waypoint.y, waypoint.z, waypoint.orientation, waypoint.delay, waypoint.moveType, waypoint.eventId, waypoint.eventChance, 0);
}

void WaypointMgr::deleteWayPointById(uint32_t pathid, uint32_t waypointId)
{
    WorldDatabase.Execute("DELETE FROM creature_waypoints WHERE id = %u AND point = %u", pathid, waypointId);
    load();
}

void WaypointMgr::deleteAllWayPoints(uint32_t pathid)
{
    WorldDatabase.Execute("DELETE FROM creature_waypoints WHERE id = %u", pathid);

    auto itr = _waypointStore.find(pathid);
    if (itr != _waypointStore.end())
        _waypointStore.erase(pathid);

    sLogger.debug("Deleted waypoints for pathID {}", pathid);
}
