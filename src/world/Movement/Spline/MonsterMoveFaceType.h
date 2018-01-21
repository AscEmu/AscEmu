/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once
#include "MovementSplineDefines.hpp"

namespace Movement { namespace Spline {
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

        uint8_t GetFlag() { return MoveFlag; }
        void SetFlag(uint8_t pFlag) { MoveFlag = pFlag; }

        /* MonsterMoveFacingLocation */
        float GetX() { return TargetPointX; }
        float GetY() { return TargetPointY; }
        float GetZ() { return TargetPointZ; }
        void SetX(float pX) { TargetPointX = pX; }
        void SetY(float pY) { TargetPointY = pY; }
        void SetZ(float pZ) { TargetPointZ = pZ; }

        /* MonsterMoveFacingTarget */
        uint64_t GetGuid() { return TargetGuid; }
        void SetGuid(uint64_t pGuid) { TargetGuid = pGuid; }

        /* MonsterMoveFacingAngle */
        float GetAngle() { return TargetAngle; }
        void SetAngle(float pAngle) { TargetAngle = pAngle; }

    protected:

        uint8_t MoveFlag;

        /* MonsterMoveNormal */

        /* MonsterMoveStop */

        /* MonsterMoveFacingLocation */
        float TargetPointX;
        float TargetPointY;
        float TargetPointZ;

        /* MonsterMoveFacingTarget */
        uint64_t TargetGuid;

        /* MonsterMoveFacingAngle */
        float TargetAngle;
    };
}}
