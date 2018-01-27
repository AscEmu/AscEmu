/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Data/Flags.h"

struct MovementInfo
{
    uint32_t flags;
#if VERSION_STRING <= TBC
    uint8_t flags2;
#else
    uint16_t flags2;
#endif
    uint32_t time;
    LocationVector position;
    TransportData transport_data;
    float_t transport_time;
    /*
     *  -1.55   looking down
     *  0       looking forward
     *  +1.55   looking up
     */
    float_t pitch;
    uint32_t fall_time;
    float_t redirect_velocity;
    float_t redirect_sin;
    float_t redirect_cos;
    // ReSharper disable once CppInconsistentNaming
    float_t redirect_2d_speed;
    float_t spline_elevation;
    uint32_t unk_13;

    MovementInfo():
        flags(0),
        flags2(0),
        time(0),
        position(0.f, 0.f, 0.f, 0.f),
        transport_data(),
        transport_time(0.f),
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
