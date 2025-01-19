/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "RingOfValor.h"

#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Master.h"

RingOfValor::RingOfValor(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t, uint32_t players_per_side) :
    Arena(mgr, id, lgroup, t, players_per_side)
{}

RingOfValor::~RingOfValor()
{}

void RingOfValor::OnCreate()
{
    GameObject* obj = nullptr;

    obj = spawnGameObject(194030, LocationVector(763.93f, -295.0f, 26.0f, 0.0f), 40, 1375, 1.0f);
    obj->PushToWorld(m_mapMgr);

    obj = spawnGameObject(194031, LocationVector(763.93f, -274.0f, 26.0f, 0.0f), 40, 1375, 1.0f);
    obj->PushToWorld(m_mapMgr);

    obj = spawnGameObject(193458, LocationVector(763.630f, -261.783f, 26.0f, 0.0f), 40, 1375, 1.0f);
    obj->PushToWorld(m_mapMgr);

    obj = spawnGameObject(193461, LocationVector(723.522f, -284.428f, 24.6f, 0.0f), 40, 1375, 1.0f);
    obj->PushToWorld(m_mapMgr);

    obj = spawnGameObject(192392, LocationVector(763.93f, -295.0f, 27.0f, 0.0f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setAnimationProgress(100);
    m_gates.insert(obj);

    obj = spawnGameObject(192391, LocationVector(763.93f, -274.0f, 27.0f, 0.0f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setAnimationProgress(100);
    m_gates.insert(obj);

    Arena::OnCreate();
}

LocationVector RingOfValor::GetStartingCoords(uint32_t Team)
{
    if (Team)
        return LocationVector(763.6011f, -294.3227f, 28.4f);
    else
        return LocationVector(763.9755f, -274.0825f, 28.4f);
}

void RingOfValor::HookOnAreaTrigger(Player* /*plr*/, uint32_t trigger)
{
    switch (trigger)
    {
        case 5224:
        case 5226:
        case 5473:
        case 5474:
            break;
        default:
            DLLLogDetail("Encountered unhandled areatrigger id %u", trigger);
            break;
    }
}

bool RingOfValor::HookHandleRepop(Player* plr)
{
    LocationVector dest;
    dest.ChangeCoords({ 762.91f, -284.28f, 28.28f });
    plr->safeTeleport(m_mapMgr->getBaseMap()->getMapId(), m_mapMgr->getInstanceId(), dest);
    return true;
}
