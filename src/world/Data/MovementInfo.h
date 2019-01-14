/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Data/Flags.h"
#include "Units/UnitDefines.hpp"

#if VERSION_STRING == Classic
struct MovementInfo
{
    uint32_t flags;
    uint8_t flags2;
    uint32_t time;
    LocationVector position;
    TransportData transport_data;
    uint32_t transport_time;
    /*
    *  -1.55   looking down
    *  0       looking forward
    *  +1.55   looking up
    */
    float pitch;
    uint32_t fall_time;
    float redirect_velocity;
    float redirect_sin;
    float redirect_cos;
    // ReSharper disable once CppInconsistentNaming
    float redirect_2d_speed;
    float spline_elevation;
    uint32_t unk_13;

    MovementInfo() :
        flags(0),
        flags2(0),
        time(0),
        position(0.f, 0.f, 0.f, 0.f),
        transport_data(),
        transport_time(0),
        pitch(0.f),
        fall_time(0),
        redirect_velocity(0.f),
        redirect_sin(0.f),
        redirect_cos(0.f),
        redirect_2d_speed(0.f),
        spline_elevation(0.f),
        unk_13(0)
    {
    }

    bool hasFlag(uint32_t flag) const { return flags & flag; }
    bool isOnTransport() const { return hasFlag(MOVEFLAG_TRANSPORT); }
    bool isSwimming() const { return hasFlag(MOVEFLAG_SWIMMING); }
    bool isFlying() const { return hasFlag(MOVEFLAG_FLYING); }
    bool isSwimmingOrFlying() const { return isSwimming() || isFlying(); }
    bool isFalling() const { return hasFlag(MOVEFLAG_FALLING); }
    bool isRedirected() const { return hasFlag(MOVEFLAG_REDIRECTED); }
    bool isFallingOrRedirected() const { return isFalling() || isRedirected(); }
    bool isSplineMover() const { return hasFlag(MOVEFLAG_SPLINE_MOVER); }
};
#endif

#if VERSION_STRING == TBC
struct MovementInfo
{
    uint32_t flags;
    uint8_t flags2;
    uint32_t time;
    LocationVector position;
    TransportData transport_data;
    uint32_t transport_time;
    /*
     *  -1.55   looking down
     *  0       looking forward
     *  +1.55   looking up
     */
    float pitch;
    uint32_t fall_time;
    float redirect_velocity;
    float redirect_sin;
    float redirect_cos;
    // ReSharper disable once CppInconsistentNaming
    float redirect_2d_speed;
    float spline_elevation;
    uint32_t unk_13;

    MovementInfo() :
        flags(0),
        flags2(0),
        time(0),
        position(0.f, 0.f, 0.f, 0.f),
        transport_data(),
        transport_time(0),
        pitch(0.f),
        fall_time(0),
        redirect_velocity(0.f),
        redirect_sin(0.f),
        redirect_cos(0.f),
        redirect_2d_speed(0.f),
        spline_elevation(0.f),
        unk_13(0)
    {
    }

    bool hasFlag(uint32_t flag) const { return flags & flag; }
    bool isOnTransport() const { return hasFlag(MOVEFLAG_TRANSPORT); }
    bool isSwimming() const { return hasFlag(MOVEFLAG_SWIMMING); }
    bool isFlying() const { return hasFlag(MOVEFLAG_FLYING); }
    bool isSwimmingOrFlying() const { return isSwimming() || isFlying(); }
    bool isFalling() const { return hasFlag(MOVEFLAG_FALLING); }
    bool isRedirected() const { return hasFlag(MOVEFLAG_REDIRECTED); }
    bool isFallingOrRedirected() const { return isFalling() || isRedirected(); }
    bool isSplineMover() const { return hasFlag(MOVEFLAG_SPLINE_MOVER); }
};
#endif

#if VERSION_STRING == WotLK
struct MovementInfo
{
    uint32_t flags;
    uint16_t flags2;

    uint32_t time;
    LocationVector position;
    TransportData transport_data;
    uint32_t transport_time;
    uint8_t transport_seat;
    float transport_time2;
    /*
    *  -1.55   looking down
    *  0       looking forward
    *  +1.55   looking up
    */
    float pitch;
    uint32_t fall_time;
    float redirect_velocity;
    float redirect_sin;
    float redirect_cos;
    // ReSharper disable once CppInconsistentNaming
    float redirect_2d_speed;
    float spline_elevation;
    uint32_t unk_13;

    MovementInfo() :
        flags(0),
        flags2(0),
        time(0),
        position(0.f, 0.f, 0.f, 0.f),
        transport_data(),
        transport_time(0),
        transport_seat(0),
        transport_time2(0.f),
        pitch(0.f),
        fall_time(0),
        redirect_velocity(0.f),
        redirect_sin(0.f),
        redirect_cos(0.f),
        redirect_2d_speed(0.f),
        spline_elevation(0.f),
        unk_13(0)
    {
    }

    bool hasFlag(uint32_t flag) const { return (flags & flag) != 0; }
    bool isOnTransport() const { return hasFlag(MOVEFLAG_TRANSPORT); }
    bool isSwimming() const { return hasFlag(MOVEFLAG_SWIMMING); }
    bool isFlying() const { return hasFlag(MOVEFLAG_FLYING); }
    bool isSwimmingOrFlying() const { return isSwimming() || isFlying(); }
    bool isFalling() const { return hasFlag(MOVEFLAG_FALLING); }
    bool isRedirected() const { return hasFlag(MOVEFLAG_REDIRECTED); }
    bool isFallingOrRedirected() const { return isFalling() || isRedirected(); }
    bool isSplineMover() const { return hasFlag(MOVEFLAG_SPLINE_MOVER); }

    bool hasFlag2(uint32_t flag2) const { return (flags2 & flag2) != 0; }
    bool isInterpolated() const { return hasFlag2(MOVEFLAG2_INTERPOLATED_MOVE); }
};
#endif
