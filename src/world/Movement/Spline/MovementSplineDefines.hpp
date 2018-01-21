/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once

#include "WorldConf.h"

namespace Movement { namespace Spline {
    enum SplineFlagsEnum
    {
        /* The first 4 bytes of the SplineFlags are actually an animation id 
         * So the structure is:
         * uint8: anim_id
         * uint24: spline_flags */
#if VERSION_STRING != Cata
        SPLINEFLAG_DONE = 0x00000100,
        SPLINEFLAG_FALLING = 0x00000200,
        SPLINEFLAG_NO_SPLINE = 0x00000400,
        SPLINEFLAG_TRAJECTORY = 0x00000800,
        SPLINEFLAG_WALKMODE = 0x00001000,
        SPLINEFLAG_FLYING = 0x00002000,
        SPLINEFLAG_KNOCKBACK = 0x00004000,
        SPLINEFLAG_FINALPOINT = 0x00008000,
        SPLINEFLAG_FINALTARGET = 0x00010000,
        SPLINEFLAG_FINALANGLE = 0x00020000,
        SPLINEFLAG_CATMULLROM = 0x00040000,
        SPLINEFLAG_UNKNOWN1 = 0x00080000,
        SPLINEFLAG_UNKNOWN2 = 0x00100000,
        SPLINEFLAG_UNKNOWN3 = 0x00200000,
        SPLINEFLAG_UNKNOWN4 = 0x00400000,
        SPLINEFLAG_UNKNOWN5 = 0x00800000,
        SPLINEFLAG_UNKNOWN6 = 0x01000000,
        SPLINEFLAG_UNKNOWN7 = 0x02000000,
        SPLINEFLAG_UNKNOWN8 = 0x04000000,
        SPLINEFLAG_UNKNOWN9 = 0x08000000,
        SPLINEFLAG_UNKNOWN10 = 0x10000000,
        SPLINEFLAG_UNKNOWN11 = 0x20000000,
        SPLINEFLAG_UNKNOWN12 = 0x40000000
#else
        SPLINEFLAG_UNK0 = 0x00000008,
        SPLINEFLAG_FALLING_SLOW = 0x00000010,   //not implemented
        SPLINEFLAG_DONE = 0x00000020,
        SPLINEFLAG_FALLING = 0x00000040,
        SPLINEFLAG_NO_SPLINE = 0x00000080,
        SPLINEFLAG_UNK2 = 0x00000100,
        SPLINEFLAG_FLYING = 0x00000200,
        SPLINEFLAG_ORIENTATION_FIXED = 0x00000400,   //not implemented
        SPLINEFLAG_CATMULLROM = 0x00000800,
        SPLINEFLAG_CYCLIC = 0x00001000,   //not implemented
        SPLINEFLAG_ENTER_CYCLIC = 0x00002000,   //not implemented
        SPLINEFLAG_FROZEN = 0x00004000,   //not implemented
        SPLINEFLAG_ENTER_VEHICLE = 0x00008000,   //not implemented
        SPLINEFLAG_EXIT_VEHICLE = 0x00010000,   //not implemented
        SPLINEFLAG_UNK3 = 0x00020000,
        SPLINEFLAG_UNK4 = 0x00040000,
        SPLINEFLAG_ORIENTATION_INVERSED = 0x00080000,   //not implemented
        SPLINEFLAG_SMOOTH_GROUND_PATH = 0x00100000,   //not implemented
        SPLINEFLAG_WALKMODE = 0x00200000,
        SPLINEFLAG_UNCOMPRESSED_PATH = 0x00400000,   //not implemented
        SPLINEFLAG_UNK6 = 0x00800000,
        SPLINEFLAG_ANIMATION = 0x01000000,   //not implemented
        SPLINEFLAG_TRAJECTORY = 0x02000000,
        SPLINEFLAG_FINALPOINT = 0x04000000,
        SPLINEFLAG_FINALTARGET = 0x08000000,
        SPLINEFLAG_FINALANGLE = 0x10000000,
        SPLINEFLAG_UNKNOWN10 = 0x20000000,
        SPLINEFLAG_UNKNOWN11 = 0x40000000,
        SPLINEFLAG_UNKNOWN12 = 0x80000000
#endif
    };

    enum MonsterMoveType : uint8_t
    {
        MonsterMoveNormal = 0,
        MonsterMoveStop = 1,
        MonsterMoveFacingLocation = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle = 4,
        MonsterMoveInvalid = 0xff,
    };

    struct SplineAnimation
    {
        bool IsAnimating;
        uint8_t Id;
        int32_t StartTime;
    };

    struct SplineParabolic
    {
        bool IsParabolic;
        float VerticalAcceleration;
        int32_t StartTime;
    };
}}
