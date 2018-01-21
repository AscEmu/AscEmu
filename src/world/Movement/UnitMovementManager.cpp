/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "UnitMovementManager.hpp"

namespace Movement
{
    void UnitMovementManager::ForceUpdate()
    {
        m_lastUpdateTick = 0;
    }

    void UnitMovementManager::Update(uint32_t pLastUpdate)
    {
        if (!CanUpdate(pLastUpdate))
            return;

        m_lastUpdateTick = pLastUpdate;
    }

    bool UnitMovementManager::CanUpdate(uint32_t pLastUpdate)
    {
        // If these are true then we can NOT update, so invert the result before returning
        return !(m_spline.GetSplinePoints()->size() == 0 || pLastUpdate == m_lastUpdateTick);
    }

    bool UnitMovementManager::IsMovementFinished()
    {
        return m_lastUpdateTick == 0;
    }

    bool UnitMovementManager::isFlying()
    {
        return m_spline.GetSplineFlags()->m_splineFlagsRaw.flying;
    }

    UnitMovementManager::UnitMovementManager() : m_spline(::Movement::Spline::SPLINEFLAG_WALKMODE), m_lastUpdateTick(0)
    {
    }

    UnitMovementManager::UnitMovementManager(Spline::MoveSpline pSpline) : m_spline(pSpline), m_lastUpdateTick(0)
    {
    }
}
