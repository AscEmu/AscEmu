/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#include "RingOfTrials.h"
#include "Map/MapMgr.h"
#include "Objects/GameObject.h"

RingOfTrials::RingOfTrials(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t, uint32 players_per_side) :
    Arena(mgr, id, lgroup, t, players_per_side)
{}

RingOfTrials::~RingOfTrials()
{}

void RingOfTrials::OnCreate()
{
    GameObject* obj = NULL;

    obj = SpawnGameObject(183979, 559, 4090.064453f, 2858.437744f, 10.236313f, 0.492805f, 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->SetRotationQuat(0.f, 0.f, 0.243916f, 0.969796f);
    obj->PushToWorld(m_mapMgr);

    obj = SpawnGameObject(183980, 559, 4081.178955f, 2874.970459f, 12.391714f, 0.492805f, 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->SetRotationQuat(0.f, 0.f, 0.243916f, 0.969796f);
    m_gates.insert(obj);

    obj = SpawnGameObject(183977, 559, 4023.709473f, 2981.776611f, 10.701169f, -2.648788f, 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->SetRotationQuat(0.f, 0.f, 0.969796f, -0.243916f);
    obj->PushToWorld(m_mapMgr);

    obj = SpawnGameObject(183978, 559, 4031.854248f, 2966.833496f, 12.646200f, -2.648788f, 32, 1375, 1.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->SetRotationQuat(0.f, 0.f, 0.969796f, -0.243916f);
    m_gates.insert(obj);

    Arena::OnCreate();
}

void RingOfTrials::HookOnShadowSight()
{
    m_buffs[0] = SpawnGameObject(184664, 559, 4011.113232f, 2896.879980f, 12.523950f, 0.486944f, 32, 1375, 1.0f);
    m_buffs[0]->setState(GO_STATE_CLOSED);
    m_buffs[0]->SetRotationQuat(0.f, 0.f, 0.904455f, -0.426569f);
    m_buffs[0]->setGoType(GAMEOBJECT_TYPE_TRAP);
    m_buffs[0]->setAnimationProgress(100);
    m_buffs[0]->PushToWorld(m_mapMgr);

    m_buffs[1] = SpawnGameObject(184664, 559, 4102.111426f, 2945.843262f, 12.662578f, 3.628544f, 32, 1375, 1.0f);
    m_buffs[1]->setState(GO_STATE_CLOSED);
    m_buffs[1]->SetRotationQuat(0.f, 0.f, 0.90445f, -0.426569f);
    m_buffs[1]->setGoType(GAMEOBJECT_TYPE_TRAP);
    m_buffs[1]->setAnimationProgress(100);
    m_buffs[1]->PushToWorld(m_mapMgr);
}

LocationVector RingOfTrials::GetStartingCoords(uint32 Team)
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
    plr->SafeTeleport(m_mapMgr->GetMapId(), m_mapMgr->GetInstanceID(), dest);
    return true;
}
