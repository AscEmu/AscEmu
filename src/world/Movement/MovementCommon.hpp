/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _MOVEMENT_COMMON_HPP
#define _MOVEMENT_COMMON_HPP

namespace Movement
{
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
}

#endif // _MOVEMENT_COMMON_HPP
