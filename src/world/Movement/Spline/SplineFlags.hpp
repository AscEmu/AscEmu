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
#pragma pack(push, 1)
        struct SplineFlagsData
        {
            uint8 animation_id       : 3;
            bool unknown0            : 1;
            bool fallingSlow         : 1;
            bool done                : 1;
            bool falling             : 1;
            bool no_spline           : 1;
            bool unknown2            : 1;
            bool flying              : 1;
            bool orientationFixed    : 1;
            bool catmullrom          : 1;
            bool cyclic              : 1;
            bool enter_cycle         : 1;
            bool frozen              : 1;
            bool transportEnter      : 1;
            bool transportExit       : 1;
            bool unknown3            : 1;
            bool unknown4            : 1;
            bool orientationInversed : 1;
            bool smoothGroundPath    : 1;
            bool walkmode            : 1;
            bool uncompressedPath    : 1;
            bool unknown6            : 1;
            bool animation           : 1;
            bool trajectory          : 1;
            bool finalpoint          : 1;
            bool finaltarget         : 1;
            bool finalangle          : 1;
            bool unknown7            : 1;
            bool unknown8            : 1;
            bool unknown9            : 1;

            inline uint32& as_uint32() { return (uint32&)*this; }
        };
#pragma pack(pop)

        struct SplineFlags
        {
            protected:

                void UnsetAllFacingFlags();

            public:

                SplineFlagsData m_splineFlagsRaw;
                uint32 GetFlagsForMonsterMove();
                void SetFacingPointFlag();
                void SetFacingTargetFlag();
                void SetFacingAngleFlag();
        };
    }
}

#endif // _MOVEMENT_SPLINEFLAGS_HPP
