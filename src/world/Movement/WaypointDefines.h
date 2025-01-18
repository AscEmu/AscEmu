/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <vector>

enum WaypointMoveType
{
    WAYPOINT_MOVE_TYPE_WALK,
    WAYPOINT_MOVE_TYPE_RUN,
    WAYPOINT_MOVE_TYPE_LAND,
    WAYPOINT_MOVE_TYPE_TAKEOFF,

    WAYPOINT_MOVE_TYPE_MAX
};

struct WaypointNode
{
    WaypointNode() : id(0), x(0.f), y(0.f), z(0.f), orientation(0.f), delay(0), eventId(0), moveType(WAYPOINT_MOVE_TYPE_RUN), eventChance(0) { }
    WaypointNode(uint32_t _id, float _x, float _y, float _z, float _orientation = 0.f, uint32_t _delay = 0)
    {
        id = _id;
        x = _x;
        y = _y;
        z = _z;
        orientation = _orientation;
        delay = _delay;
        eventId = 0;
        moveType = WAYPOINT_MOVE_TYPE_WALK;
        eventChance = 100;
    }

    uint32_t id;
    float x, y, z, orientation;
    uint32_t delay;
    uint32_t eventId;
    uint32_t moveType;
    uint8_t eventChance;
};

struct WaypointPath
{
    WaypointPath() : id(0) { }
    WaypointPath(uint32_t _id, std::vector<WaypointNode>&& _nodes)
    {
        id = _id;
        nodes = _nodes;
    }

    std::vector<WaypointNode> nodes;
    uint32_t id;
};
