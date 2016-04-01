/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _MOVEMENT_SPLINEFLAGS_HPP
#define _MOVEMENT_SPLINEFLAGS_HPP

#include "StdAfx.h"
#include "MovementSplineDefines.hpp"
#include "G3D/Vector3.h"

namespace Movement
{
    namespace Spline
    {
        struct SplineFlags
        {
            protected:

                void UnsetAllFacingFlags();

            public:

                uint32 m_splineFlags;
                void SetFacingPointFlag();
                void SetFacingTargetFlag();
                void SetFacingAngleFlag();
        };
    }
}

#endif // _MOVEMENT_SPLINEFLAGS_HPP
