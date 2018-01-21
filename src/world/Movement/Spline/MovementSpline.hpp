/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "Movement/MovementCommon.hpp"
#include "SplineFlags.hpp"
#include "G3D/Vector3.h"
#include "MonsterMoveFaceType.h"

namespace Movement { namespace Spline {
    //Assume previous point can be reached through linked list or current creature position.
    struct SplinePoint
    {
        G3D::Vector3 pos;
        uint32_t setoff; /// mstime when npc set off of this point
        uint32_t arrive; /// mstime the npc reaches the destination
    };

    static SplinePoint InvalidPoint = {{0.0f, 0.0f, 0.0f}, 0, 0};

    class MoveSpline
    {
    protected:
        SplineFlags m_splineFlags;
        std::vector<SplinePoint> m_splinePoints;
        uint32_t m_currentSplineIndex;

    public:
        MonsterMoveFaceType m_splineFaceType;
        uint32_t m_currentSplineTotalMoveTime;
        float m_splineTrajectoryVertical;
        uint32_t m_splineTrajectoryTime;

        /*void SetSplineFlag(uint32_t pFlags);
        uint32_t HasSplineFlag(uint32_t pFlags);
        void AddSplineFlag(uint32_t pFlags);
        void RemoveSplineFlag(uint32_t pFlags);*/

        SplinePoint GetFirstSplinePoint();
        std::vector<SplinePoint> GetMidPoints();
        SplinePoint GetLastSplinePoint();
        std::vector<SplinePoint>* GetSplinePoints();

        void ClearSpline();
        void AddSplinePoint(SplinePoint pSplinePoint);

        uint32_t GetCurrentSplineIndex();
        void SetCurrentSplineIndex(uint32_t pIndex);
        void IncrementCurrentSplineIndex();
        void DecrementCurrentSplineIndex();

        SplinePoint* GetSplinePoint(uint32_t pPointIndex);
        SplinePoint* GetNextSplinePoint();
        SplinePoint* GetCurrentSplinePoint();
        SplinePoint* GetPreviousSplinePoint();

        void SetFacing(::Movement::Point pPoint);
        void SetFacing(uint64_t pGuid);
        void SetFacing(float pAngle);

        bool IsSplineMoveDone();
        bool IsSplineEmpty();

        SplineFlags* GetSplineFlags();
        MoveSpline();
        MoveSpline(uint32_t pInitialFlags);
    };
}}
