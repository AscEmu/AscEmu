/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

namespace Movement
{
    namespace Spline
    {
        /* Returns m_splineFlagsRaw with some flags removed */
        uint32 SplineFlags::GetFlagsForMonsterMove()
        {
            return uint32(m_splineFlagsRaw.as_uint32() & uint32(~(SPLINEFLAG_FINALPOINT | SPLINEFLAG_FINALTARGET | SPLINEFLAG_FINALANGLE | 0xff | SPLINEFLAG_DONE)));
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
