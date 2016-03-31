/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _UNIT_MOVEMENT_MANAGER_HPP
#define _UNIT_MOVEMENT_MANAGER_HPP

#include "StdAfx.h"
#include "Movement/Spline/MovementSpline.hpp"

namespace Movement
{
    class UnitMovementManager
    {
        public:

            Spline::MoveSpline m_spline;

            UnitMovementManager();
            UnitMovementManager(Spline::MoveSpline pSpline);
    };
}

#endif // _UNIT_MOVEMENT_MANAGER_HPP
