/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "SplineFlags.hpp"
#include "MovementSplineDefines.hpp"

namespace Movement
{
    namespace Spline
    {
        /* Returns m_splineFlagsRaw with some flags removed */
        uint32_t SplineFlags::GetFlagsForMonsterMove()
        {
            return uint32_t(m_splineFlagsRaw.as_uint32() & uint32_t(~(SPLINEFLAG_FINALPOINT | SPLINEFLAG_FINALTARGET | SPLINEFLAG_FINALANGLE | 0xff | SPLINEFLAG_DONE)));
        }

        void SplineFlags::UnsetAllFacingFlags()
        {
            m_splineFlagsRaw.finalpoint = false;
            m_splineFlagsRaw.finaltarget = false;
            m_splineFlagsRaw.finalangle = false;
        }

        void SplineFlags::SetFacingPointFlag()
        {
            UnsetAllFacingFlags();
            m_splineFlagsRaw.finalpoint = true;
        }

        void SplineFlags::SetFacingTargetFlag()
        {
            UnsetAllFacingFlags();
            m_splineFlagsRaw.finaltarget = true;
        }

        void SplineFlags::SetFacingAngleFlag()
        {
            UnsetAllFacingFlags();
            m_splineFlagsRaw.finalangle = true;
        }
        
    }
}
