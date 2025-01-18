/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "DalaranSewers.h"

#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Master.h"

DalaranSewers::DalaranSewers(BattlegroundMap* mgr, uint32_t id, uint32_t lgroup, uint32_t t, uint32_t players_per_side) :
    Arena(mgr, id, lgroup, t, players_per_side)
{}

DalaranSewers::~DalaranSewers()
{}

void DalaranSewers::OnCreate()
{
    GameObject* obj = nullptr;

    obj = spawnGameObject(192643, LocationVector(1232.11f, 764.699f, 20.3f, 0.0f), 32, 1375, 2.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setAnimationProgress(100);
    m_gates.insert(obj);

    obj = spawnGameObject(192642, LocationVector(1350.02f, 817.502f, 19.1398f, 0.0f), 32, 1375, 2.0f);
    obj->setState(GO_STATE_CLOSED);
    obj->setAnimationProgress(100);
    m_gates.insert(obj);

    obj = spawnGameObject(191877, LocationVector(1291.974487f, 791.844666f, 9.339742f, 3.116816f), 32, 1375, 1.0f);
    obj->PushToWorld(m_mapMgr);

    Arena::OnCreate();
}

LocationVector DalaranSewers::GetStartingCoords(uint32_t Team)
{
    if (Team)
        return LocationVector(1363.3609f, 817.3569f, 14.8128f);
    else
        return LocationVector(1219.5115f, 765.0264f, 14.8253f);
}

void DalaranSewers::HookOnAreaTrigger(Player* plr, uint32_t trigger)
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
    plr->safeTeleport(m_mapMgr->getBaseMap()->getMapId(), m_mapMgr->getInstanceId(), dest);
    return true;
}
