/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <vector>

namespace Movement
{
    //Zyres: Change this to: 0 = walk, 1 = run, 3 = fly
    enum WaypointMoveType
    {
        WP_MOVE_TYPE_WALK = 0,
        WP_MOVE_TYPE_RUN = 256,
        WP_MOVE_TYPE_FLY = 768
    };

    enum WaypointMovementScript
    {
        WP_MOVEMENT_SCRIPT_NONE             = 0,
        WP_MOVEMENT_SCRIPT_RANDOMWP         = 1,    // reimplemented
        WP_MOVEMENT_SCRIPT_CIRCLEWP         = 2,    // reimplemented
        WP_MOVEMENT_SCRIPT_WANTEDWP         = 3,    // reimplemented
        //WP_MOVEMENT_SCRIPT_DONTMOVEWP     = 4,
        WP_MOVEMENT_SCRIPT_QUEST            = 5,    //\todo same as foward then stop with returning to position
        WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP  = 6,    // reimplemented
        WP_MOVEMENT_SCRIPT_PATROL           = 7     // new
    };

    struct Point
    {
        float x;
        float y;
        float z;
    };

    struct Location
    {
        float x;
        float y;
        float z;
        float o;
    };

    //Zyres: Temp structure to seperate wp related structure from LocationExtra
    struct LocationWithFlag
    {
        Location wp_location;
        uint32_t wp_flag;
    };

    struct WayPoint
    {
        WayPoint()
        {
            id = 0;
            x = 0;
            y = 0;
            z = 0;
            o = 0;
            waittime = 0;
            flags = WP_MOVE_TYPE_WALK;
            forwardemoteoneshot = false;
            forwardemoteid = 0;
            backwardemoteoneshot = false;
            backwardemoteid = 0;
            forwardskinid = 0;
            backwardskinid = 0;
        }
        uint32_t id;
        float x;
        float y;
        float z;
        float o;
        uint32_t waittime; //ms
        uint32_t flags;
        bool forwardemoteoneshot;
        uint32_t forwardemoteid;
        bool backwardemoteoneshot;
        uint32_t backwardemoteid;
        uint32_t forwardskinid;
        uint32_t backwardskinid;

    };

    typedef std::vector<WayPoint*> WayPointMap;
}
