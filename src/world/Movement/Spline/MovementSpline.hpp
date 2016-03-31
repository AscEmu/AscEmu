/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _MOVEMENT_SPLINE_HPP
#define _MOVEMENT_SPLINE_HPP

#include "MovementSplineDefines.hpp"
#include "StdAfx.h"

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


        class MoveSpline
        {
            protected:

                uint32 m_splineFlags;
                std::vector<::Movement::Spline::SplinePoint> m_splinePoints;

            public:

                void SetSplineFlag(uint32 pFlags) { m_splineFlags = pFlags; }
                uint32 HasSplineFlag(uint32 pFlags) { return m_splineFlags & pFlags; }
                void AddSplineFlag(uint32 pFlags) { m_splineFlags |= pFlags; }
                void RemoveSplineFlag(uint32 pFlags) { m_splineFlags &= ~pFlags; }

                uint32 GetSplineFlags() { return m_splineFlags; }
                MoveSpline();
                MoveSpline(uint32 pInitialFlags) { m_splineFlags = pInitialFlags; }
        };
    }
}

#endif // _MOVEMENT_SPLINE_HPP
