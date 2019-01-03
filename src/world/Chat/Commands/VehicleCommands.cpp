/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
    if (p->getTargetGuid() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }

    Unit* u = p->GetMapMgr()->GetUnit(p->getTargetGuid());
    if (u == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->getVehicleComponent() == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }

    u->getVehicleComponent()->EjectPassengerFromSeat(seat);
    return true;
}

//.vehicle ejectallpassengers
bool ChatHandler::HandleVehicleEjectAllPassengersCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->getTargetGuid() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->getTargetGuid());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->getVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->getVehicleComponent()->EjectAllPassengers();
    return true;
}

//.vehicle installaccessories
bool ChatHandler::HandleVehicleInstallAccessoriesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->getTargetGuid() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->getTargetGuid());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->getVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->getVehicleComponent()->InstallAccessories();
    return true;
}

//.vehicle removeaccessories
bool ChatHandler::HandleVehicleRemoveAccessoriesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->getTargetGuid() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->getTargetGuid());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->getVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->getVehicleComponent()->RemoveAccessories();
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
    if (session->GetPlayer()->getTargetGuid() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = session->GetPlayer()->GetMapMgr()->GetUnit(session->GetPlayer()->getTargetGuid());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->getVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (!u->getVehicleComponent()->HasEmptySeat())
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
    c->addPassengerToVehicle(u->getGuid(), 1);
    return true;
}
