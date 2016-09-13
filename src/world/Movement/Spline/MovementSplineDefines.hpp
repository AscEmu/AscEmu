/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _MOVEMENT_SPLINE_DEFINES_HPP
#define _MOVEMENT_SPLINE_DEFINES_HPP

#include "StdAfx.h"
#include "MovementSpline.hpp"

namespace Movement
{
    namespace Spline
    {
        enum SplineFlagsEnum
        {
            /* The first 4 bytes of the SplineFlags are actually an animation id 
             * So the structure is:
             * uint8: anim_id
             * uint24: spline_flags
             *
             * ok i guess */
            SPLINEFLAG_UNK0                     = 0x00000008,
            SPLINEFLAG_FALLING_SLOW             = 0x00000010,   //not implemented
            SPLINEFLAG_DONE                     = 0x00000020,
            SPLINEFLAG_FALLING                  = 0x00000040,
            SPLINEFLAG_NO_SPLINE                = 0x00000080,
            SPLINEFLAG_UNK2                     = 0x00000100,
            SPLINEFLAG_FLYING                   = 0x00000200,
            SPLINEFLAG_ORIENTATION_FIXED        = 0x00000400,   //not implemented
            SPLINEFLAG_CATMULLROM               = 0x00000800,
            SPLINEFLAG_CYCLIC                   = 0x00001000,   //not implemented
            SPLINEFLAG_ENTER_CYCLIC             = 0x00002000,   //not implemented
            SPLINEFLAG_FROZEN                   = 0x00004000,   //not implemented
            SPLINEFLAG_ENTER_VEHICLE            = 0x00008000,   //not implemented
            SPLINEFLAG_EXIT_VEHICLE             = 0x00010000,   //not implemented
            SPLINEFLAG_UNK3                     = 0x00020000,
            SPLINEFLAG_UNK4                     = 0x00040000,
            SPLINEFLAG_ORIENTATION_INVERSED     = 0x00080000,   //not implemented
            SPLINEFLAG_SMOOTH_GROUND_PATH       = 0x00100000,   //not implemented
            SPLINEFLAG_WALKMODE                 = 0x00200000,
            SPLINEFLAG_UNCOMPRESSED_PATH        = 0x00400000,   //not implemented
            SPLINEFLAG_UNK6                     = 0x00800000,
            SPLINEFLAG_ANIMATION                = 0x01000000,   //not implemented
            SPLINEFLAG_TRAJECTORY               = 0x02000000,
            SPLINEFLAG_FINALPOINT               = 0x04000000,
            SPLINEFLAG_FINALTARGET              = 0x08000000,
            SPLINEFLAG_FINALANGLE               = 0x10000000,
            SPLINEFLAG_UNKNOWN10                = 0x20000000,
            SPLINEFLAG_UNKNOWN11                = 0x40000000,
            SPLINEFLAG_UNKNOWN12                = 0x80000000
        };

        enum MonsterMoveType : uint8
        {
            MonsterMoveNormal           = 0,
            MonsterMoveStop             = 1,
            MonsterMoveFacingLocation   = 2,
            MonsterMoveFacingTarget     = 3,
            MonsterMoveFacingAngle      = 4,
            MonsterMoveInvalid          = 0xff,
        };

        /* NOT ALL DATA FIELDS IN THIS STRUCT ARE FILLED 
         * 
         * Read the MoveFlag to calculate which fields are filled */
        struct MonsterMoveFaceType
        {
            public:

                MonsterMoveFaceType()
                { 
                    MoveFlag = MonsterMoveInvalid;
                    TargetPointX = 0.0f;
                    TargetPointY = 0.0f;
                    TargetPointZ = 0.0f;
                    TargetGuid = 0;
                    TargetAngle = 0.0f;
                }

                uint8 GetFlag() { return MoveFlag; }
                void SetFlag(uint8 pFlag) { MoveFlag = pFlag; }

                /* MonsterMoveFacingLocation */
                float GetX() { return TargetPointX; }
                float GetY() { return TargetPointY; }
                float GetZ() { return TargetPointZ; }
                void SetX(float pX) { TargetPointX = pX; }
                void SetY(float pY) { TargetPointY = pY; }
                void SetZ(float pZ) { TargetPointZ = pZ; }

                /* MonsterMoveFacingTarget */
                uint64 GetGuid() { return TargetGuid; }
                void SetGuid(uint64 pGuid) { TargetGuid = pGuid; }

                /* MonsterMoveFacingAngle */
                float GetAngle() { return TargetAngle; }
                void SetAngle(float pAngle) { TargetAngle = pAngle; }

            protected:

                uint8 MoveFlag;

                /* MonsterMoveNormal */

                /* MonsterMoveStop */

                /* MonsterMoveFacingLocation */
                float TargetPointX;
                float TargetPointY;
                float TargetPointZ;

                /* MonsterMoveFacingTarget */
                uint64 TargetGuid;

                /* MonsterMoveFacingAngle */
                float TargetAngle;
        };

        struct SplineAnimation
        {
            bool IsAnimating;
            uint8 Id;
            int32 StartTime;
        };

        struct SplineParabolic
        {
            bool IsParabolic;
            float VerticalAcceleration;
            int32 StartTime;
        };
    }
}

#endif // _MOVEMENT_SPLINE_DEFINES_HPP
