/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once

#include "Movement/Spline/MovementSpline.hpp"

namespace Movement {
    class UnitMovementManager
    {
    protected:
        // Used to limit updates to once per MapMgr tick
        uint32_t m_lastUpdateTick;
    public:

        Spline::MoveSpline m_spline;

        void ForceUpdate();
        void Update(uint32_t pLastUpdate);

        bool CanUpdate(uint32_t pLastUpdate);

        bool IsMovementFinished();

        bool isFlying();

        UnitMovementManager();
        UnitMovementManager(Spline::MoveSpline pSpline);
    };
}
