/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _MOVEMENT_SPLINE_HPP
#define _MOVEMENT_SPLINE_HPP

#include "StdAfx.h"
#include "Movement/MovementCommon.hpp"
#include "MovementSplineDefines.hpp"
#include "SplineFlags.hpp"
#include "G3D/Vector3.h"

namespace Movement
{
    namespace Spline
    {
        //Assume previous point can be reached through linked list or current creature position.
        struct SplinePoint
        {
            G3D::Vector3 pos;
            uint32 setoff;      /// mstime when npc set off of this point
            uint32 arrive;      /// mstime the npc reaches the destination
        };

        static SplinePoint InvalidPoint = { { 0.0f, 0.0f, 0.0f }, 0, 0 };

        class MoveSpline
        {
            protected:

                ::Movement::Spline::SplineFlags m_splineFlags;
                std::vector<::Movement::Spline::SplinePoint> m_splinePoints;
                uint32 m_currentSplineIndex;

            public:

                ::Movement::Spline::MonsterMoveFaceType m_splineFaceType;
                uint32 m_currentSplineTotalMoveTime;
                float m_splineTrajectoryVertical;
                uint32 m_splineTrajectoryTime;

                void SetSplineFlag(uint32 pFlags);
                uint32 HasSplineFlag(uint32 pFlags);
                void AddSplineFlag(uint32 pFlags);
                void RemoveSplineFlag(uint32 pFlags);
                
                ::Movement::Spline::SplinePoint GetFirstSplinePoint();
                std::vector<::Movement::Spline::SplinePoint> GetMidPoints();
                ::Movement::Spline::SplinePoint GetLastSplinePoint();
                std::vector<::Movement::Spline::SplinePoint>* GetSplinePoints();
                
                void ClearSpline();
                void AddSplinePoint(::Movement::Spline::SplinePoint pSplinePoint);

                uint32 GetCurrentSplineIndex();
                void SetCurrentSplineIndex(uint32 pIndex);
                void IncrementCurrentSplineIndex();
                void DecrementCurrentSplineIndex();

                ::Movement::Spline::SplinePoint* GetSplinePoint(uint32 pPointIndex);
                ::Movement::Spline::SplinePoint* GetNextSplinePoint();
                ::Movement::Spline::SplinePoint* GetCurrentSplinePoint();
                ::Movement::Spline::SplinePoint* GetPreviousSplinePoint();

                void SetFacing(::Movement::Point pPoint);
                void SetFacing(uint64 pGuid);
                void SetFacing(float pAngle);

                bool IsSplineMoveDone();
                bool IsSplineEmpty();

                uint32 GetSplineFlags();
                MoveSpline();
                MoveSpline(uint32 pInitialFlags);
        };
    }
}

#endif // _MOVEMENT_SPLINE_HPP
