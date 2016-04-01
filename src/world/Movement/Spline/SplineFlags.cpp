/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

namespace Movement
{
    namespace Spline
    {
        void SplineFlags::UnsetAllFacingFlags()
        {
            m_splineFlags = m_splineFlags & ~(SPLINEFLAG_FINALPOINT | SPLINEFLAG_FINALTARGET | SPLINEFLAG_FINALANGLE);
        }

        void SplineFlags::SetFacingPointFlag()
        {
            UnsetAllFacingFlags();
            m_splineFlags = m_splineFlags & SPLINEFLAG_FINALPOINT;
        }

        void SplineFlags::SetFacingTargetFlag()
        {
            UnsetAllFacingFlags();
            m_splineFlags = m_splineFlags & SPLINEFLAG_FINALTARGET;
        }

        void SplineFlags::SetFacingAngleFlag()
        {
            UnsetAllFacingFlags();
            m_splineFlags = m_splineFlags & SPLINEFLAG_FINALANGLE;
        }
        
    }
}
