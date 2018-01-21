/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "MovementSpline.hpp"
#include <cstdint>

namespace Movement
{
    namespace Spline
    {
        MoveSpline::MoveSpline() :
            m_currentSplineIndex(0xFFFFFFFF),
            m_splineFaceType(),
            m_currentSplineTotalMoveTime(0),
            m_splineTrajectoryVertical(0),
            m_splineTrajectoryTime(0)
        {
            m_splineFlags.m_splineFlagsRaw.as_uint32() = 0;
            m_currentSplineIndex = 0;
            m_currentSplineTotalMoveTime = 0;
            m_splineTrajectoryVertical = 0;
            m_splineTrajectoryTime = 0;
        }

        MoveSpline::MoveSpline(uint32_t pInitialFlags)
        {
            m_splineFlags.m_splineFlagsRaw.as_uint32() = pInitialFlags;
        }

        /*void MoveSpline::SetSplineFlag(uint32 pFlags)
        {
            m_splineFlags.m_splineFlags = pFlags;
        }

        uint32 MoveSpline::HasSplineFlag(uint32 pFlags)
        {
            return m_splineFlags.m_splineFlags & pFlags;
        }

        void MoveSpline::AddSplineFlag(uint32 pFlags)
        {
            m_splineFlags.m_splineFlags |= pFlags;
        }

        void MoveSpline::RemoveSplineFlag(uint32 pFlags)
        {
           m_splineFlags.m_splineFlags &= ~pFlags;
        }*/

        ::Movement::Spline::SplineFlags* MoveSpline::GetSplineFlags()
        {
            return &m_splineFlags;
        }

        ::Movement::Spline::SplinePoint MoveSpline::GetFirstSplinePoint()
        {
            if (m_splinePoints.size() < 1)
                return InvalidPoint;

            return m_splinePoints[0];
        }

        std::vector<::Movement::Spline::SplinePoint> MoveSpline::GetMidPoints()
        {
            std::vector<::Movement::Spline::SplinePoint> returnSpline;
            if (m_splinePoints.size() > 2)
            {
                for (uint32_t i = 1; i < m_splinePoints.size() - 1; ++i)
                {
                    auto splineCopy = SplinePoint(m_splinePoints[i]);
                    returnSpline.push_back(splineCopy);
                }
            }
            return returnSpline;
        }

        ::Movement::Spline::SplinePoint MoveSpline::GetLastSplinePoint()
        {
            if (m_splinePoints.size() < 1)
                return InvalidPoint;
            
            return m_splinePoints[m_splinePoints.size() - 1];
        }
        
        std::vector<::Movement::Spline::SplinePoint>* MoveSpline::GetSplinePoints() 
        {
            return &m_splinePoints;
        }

        uint32_t MoveSpline::GetCurrentSplineIndex()
        {
            return m_currentSplineIndex;
        }

        void MoveSpline::SetCurrentSplineIndex(uint32_t pIndex)
        {
            m_currentSplineIndex = pIndex;
        }

        void MoveSpline::IncrementCurrentSplineIndex()
        {
            ++m_currentSplineIndex;
        }

        void MoveSpline::DecrementCurrentSplineIndex()
        {
            --m_currentSplineIndex;
        }

        ::Movement::Spline::SplinePoint* MoveSpline::GetSplinePoint(uint32_t pPointIndex)
        {
            if (m_splinePoints.size() <= 0 || m_splinePoints.size() <= pPointIndex)
                return &InvalidPoint;

            return &m_splinePoints[pPointIndex];
        }

        ::Movement::Spline::SplinePoint* MoveSpline::GetNextSplinePoint()
        {
            return GetSplinePoint(m_currentSplineIndex + 1);
        }

        ::Movement::Spline::SplinePoint* MoveSpline::GetCurrentSplinePoint()
        {
            return GetSplinePoint(m_currentSplineIndex);
        }

        ::Movement::Spline::SplinePoint* MoveSpline::GetPreviousSplinePoint()
        {
            return GetSplinePoint(m_currentSplineIndex - 1);
        }

        bool MoveSpline::IsSplineMoveDone()
        {
            return m_currentSplineIndex >= m_splinePoints.size();
        }

        bool MoveSpline::IsSplineEmpty()
        {
            return m_splinePoints.size() == 0;
        }

        void MoveSpline::ClearSpline()
        {
            m_splinePoints.clear();
            m_currentSplineIndex = 1;
            m_currentSplineTotalMoveTime = 0;

            m_splineFaceType.SetFlag(MonsterMoveInvalid);
            m_splineFaceType.SetAngle(0);
            m_splineFaceType.SetGuid(0);
            m_splineFaceType.SetX(0);
            m_splineFaceType.SetY(0);
            m_splineFaceType.SetZ(0);
        }

        void MoveSpline::AddSplinePoint(::Movement::Spline::SplinePoint pSplinePoint)
        {
            m_splinePoints.push_back(pSplinePoint);
        }

        void MoveSpline::SetFacing(::Movement::Point pPoint)
        {
            m_splineFlags.SetFacingPointFlag();
            m_splineFaceType.SetFlag(::Movement::Spline::MonsterMoveFacingLocation);
            m_splineFaceType.SetX(pPoint.x);
            m_splineFaceType.SetY(pPoint.y);
            m_splineFaceType.SetZ(pPoint.z);
        }

        void MoveSpline::SetFacing(uint64_t pGuid)
        {
            m_splineFlags.SetFacingTargetFlag();
            m_splineFaceType.SetFlag(::Movement::Spline::MonsterMoveFacingTarget);
            m_splineFaceType.SetGuid(pGuid);
        }

        void MoveSpline::SetFacing(float pAngle)
        {
            m_splineFlags.SetFacingAngleFlag();
            m_splineFaceType.SetFlag(::Movement::Spline::MonsterMoveFacingAngle);
            m_splineFaceType.SetAngle(pAngle);
        }
    }
} 
