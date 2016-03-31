/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

namespace Movement
{
    namespace Spline
    {
        MoveSpline::MoveSpline()
        {

        }

        MoveSpline::MoveSpline(uint32 pInitialFlags)
        {
            m_splineFlags = pInitialFlags;
        }

        void MoveSpline::SetSplineFlag(uint32 pFlags)
        {
            m_splineFlags = pFlags;
        }

        uint32 MoveSpline::HasSplineFlag(uint32 pFlags)
        {
            return m_splineFlags & pFlags;
        }

        void MoveSpline::AddSplineFlag(uint32 pFlags)
        {
            m_splineFlags |= pFlags;
        }

        void MoveSpline::RemoveSplineFlag(uint32 pFlags)
        {
            m_splineFlags &= ~pFlags;
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
                for (uint32 i = 1; i < m_splinePoints.size() - 1; ++i)
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

        void MoveSpline::ClearSpline()
        {
            m_splinePoints.clear();
        }

        void MoveSpline::AddSplinePoint(::Movement::Spline::SplinePoint pSplinePoint)
        {
            m_splinePoints.push_back(pSplinePoint);
        }
    }
} 
