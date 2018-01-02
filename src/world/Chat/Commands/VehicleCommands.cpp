/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"

//.vehicle ejectpassenger
bool ChatHandler::HandleVehicleEjectPassengerCommand(const char* args, WorldSession* session)
{
    if (!args)
    {
        RedSystemMessage(session, "You need to specify a seat number.");
        return false;
    }

    uint32_t seat = atoi(args);

    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }

    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }

    u->GetVehicleComponent()->EjectPassengerFromSeat(seat);
    return true;
}

//.vehicle ejectallpassengers
bool ChatHandler::HandleVehicleEjectAllPassengersCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->EjectAllPassengers();
    return true;
}

//.vehicle installaccessories
bool ChatHandler::HandleVehicleInstallAccessoriesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->InstallAccessories();
    return true;
}

//.vehicle removeaccessories
bool ChatHandler::HandleVehicleRemoveAccessoriesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->RemoveAccessories();
    return true;
}

//.vehicle addpassenger
bool ChatHandler::HandleVehicleAddPassengerCommand(const char* args, WorldSession* session)
{
    std::stringstream ss(args);
    uint32 creature_entry;
    ss >> creature_entry;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify a creature id.");
        return false;
    }
    if (session->GetPlayer()->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = session->GetPlayer()->GetMapMgr()->GetUnit(session->GetPlayer()->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (!u->GetVehicleComponent()->HasEmptySeat())
    {
        RedSystemMessage(session, "That vehicle has no more empty seats.");
        return false;
    }

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(creature_entry);
    if (cp == nullptr)
    {
        RedSystemMessage(session, "Creature %u doesn't exist in the database", creature_entry);
        return false;
    }
    Creature* c = u->GetMapMgr()->CreateCreature(creature_entry);
    c->Load(cp, u->GetPositionX(), u->GetPositionY(), u->GetPositionZ(), u->GetOrientation());
    c->PushToWorld(u->GetMapMgr());
    c->EnterVehicle(u->GetGUID(), 1);
    return true;
}
