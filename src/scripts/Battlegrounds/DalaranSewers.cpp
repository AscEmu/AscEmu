/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DalaranSewers.h"
#include "Map/MapMgr.h"
#include "Objects/GameObject.h"
#include "Server/Master.h"

DalaranSewers::DalaranSewers(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t, uint32 players_per_side) :
    Arena(mgr, id, lgroup, t, players_per_side)
{}

DalaranSewers::~DalaranSewers()
{}

void DalaranSewers::OnCreate()
{
    GameObject* obj = NULL;

    obj = SpawnGameObject(192643, 617, 1232.11f, 764.699f, 20.3f, 0.0f, 32, 1375, 2.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setAnimationProgress(100);
    m_gates.insert(obj);

    obj = SpawnGameObject(192642, 617, 1350.02f, 817.502f, 19.1398f, 0.0f, 32, 1375, 2.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setAnimationProgress(100);
    m_gates.insert(obj);

    obj = SpawnGameObject(191877, 617, 1291.974487f, 791.844666f, 9.339742f, 3.116816f, 32, 1375, 1.0f);
    obj->PushToWorld(m_mapMgr);

    Arena::OnCreate();
}

LocationVector DalaranSewers::GetStartingCoords(uint32 Team)
{
    if (Team)
        return LocationVector(1363.3609f, 817.3569f, 14.8128f);
    else
        return LocationVector(1219.5115f, 765.0264f, 14.8253f);
}

void DalaranSewers::HookOnAreaTrigger(Player* plr, uint32 trigger)
{
    switch (trigger)
    {
    case 5347:
    case 5348:
        plr->removeAllAurasById(48018); // Demonic Circle
        break;
    default:
        DLLLogDetail("Encountered unhandled areatrigger id %u", trigger);
        return;
        break;
    }
}

bool DalaranSewers::HookHandleRepop(Player* plr)
{
    LocationVector dest;
    dest.ChangeCoords({ 1292.51f, 792.05f, 9.34f });
    plr->SafeTeleport(m_mapMgr->GetMapId(), m_mapMgr->GetInstanceID(), dest);
    return true;
}
