/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

namespace Movement
{
    void UnitMovementManager::ForceUpdate()
    {
        m_lastUpdateTick = 0;
    }

    void UnitMovementManager::Update(uint32 pLastUpdate)
    {
        if (!CanUpdate(pLastUpdate))
            return;

        m_lastUpdateTick = pLastUpdate;
    }

    bool UnitMovementManager::CanUpdate(uint32 pLastUpdate)
    {
        // If these are true then we can NOT update, so invert the result before returning
        return !(m_spline.GetSplinePoints()->size() == 0 || pLastUpdate == m_lastUpdateTick);
    }

    bool UnitMovementManager::IsMovementFinished()
    {
        return m_lastUpdateTick == 0;
    }

    UnitMovementManager::UnitMovementManager() : m_spline(), m_lastUpdateTick(0)
    {

    }

    UnitMovementManager::UnitMovementManager(Spline::MoveSpline pSpline) : m_spline(pSpline)
    {

    }
} 
