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
#include "RingOfTrials.h"

#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"

RingOfTrials::RingOfTrials(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t, uint32_t players_per_side) :
    Arena(mgr, id, lgroup, t, players_per_side)
{}

RingOfTrials::~RingOfTrials()
{}

void RingOfTrials::OnCreate()
{
    GameObject* obj = nullptr;

    obj = spawnGameObject(183979, LocationVector(4090.064453f, 2858.437744f, 10.236313f, 0.492805f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.243916f, 0.969796f);
    obj->PushToWorld(m_mapMgr);

    obj = spawnGameObject(183980, LocationVector(4081.178955f, 2874.970459f, 12.391714f, 0.492805f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.243916f, 0.969796f);
    m_gates.insert(obj);

    obj = spawnGameObject(183977, LocationVector(4023.709473f, 2981.776611f, 10.701169f, -2.648788f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.969796f, -0.243916f);
    obj->PushToWorld(m_mapMgr);

    obj = spawnGameObject(183978, LocationVector(4031.854248f, 2966.833496f, 12.646200f, -2.648788f), 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setLocalRotation(0.f, 0.f, 0.969796f, -0.243916f);
    m_gates.insert(obj);

    Arena::OnCreate();
}

void RingOfTrials::HookOnShadowSight()
{
    m_buffs[0] = spawnGameObject(184664, LocationVector(4011.113232f, 2896.879980f, 12.523950f, 0.486944f), 32, 1375, 1.0f);
    m_buffs[0]->setState(GO_STATE_CLOSED);
    m_buffs[0]->setLocalRotation(0.f, 0.f, 0.904455f, -0.426569f);
    m_buffs[0]->setGoType(GAMEOBJECT_TYPE_TRAP);
    m_buffs[0]->setAnimationProgress(100);
    m_buffs[0]->PushToWorld(m_mapMgr);

    m_buffs[1] = spawnGameObject(184664, LocationVector(4102.111426f, 2945.843262f, 12.662578f, 3.628544f), 32, 1375, 1.0f);
    m_buffs[1]->setState(GO_STATE_CLOSED);
    m_buffs[1]->setLocalRotation(0.f, 0.f, 0.90445f, -0.426569f);
    m_buffs[1]->setGoType(GAMEOBJECT_TYPE_TRAP);
    m_buffs[1]->setAnimationProgress(100);
    m_buffs[1]->PushToWorld(m_mapMgr);
}

LocationVector RingOfTrials::GetStartingCoords(uint32_t Team)
{
    if (Team)
        return LocationVector(4027.004883f, 2976.964844f, 11.600499f);
    else
        return LocationVector(4085.861328f, 2866.750488f, 12.417445f);
}

bool RingOfTrials::HookHandleRepop(Player* plr)
{
    LocationVector dest;
    dest.ChangeCoords({ 4057.042725f, 2918.686523f, 13.051933f });
    plr->safeTeleport(m_mapMgr->getBaseMap()->getMapId(), m_mapMgr->getInstanceId(), dest);
    return true;
}
