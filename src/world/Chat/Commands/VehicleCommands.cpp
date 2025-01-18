/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <sstream>

#include "Chat/ChatHandler.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"

#ifdef FT_VEHICLES
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

    Unit* u = p->getWorldMap()->getUnit(p->getTargetGuid());
    if (u == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }

    if (u->getVehicleKit())
    {
        if (Unit* passenger = u->getVehicleKit()->getPassenger(static_cast<int8_t>(seat)))
            passenger->callExitVehicle();
    }
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
    Unit* u = p->getWorldMap()->getUnit(p->getTargetGuid());
    if (u == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->getVehicleKit() == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->getVehicleKit()->removeAllPassengers();
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
    Unit* u = p->getWorldMap()->getUnit(p->getTargetGuid());
    if (u == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->getVehicleKit() == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->getVehicleKit()->loadAllAccessories(false);
    return true;
}

//.vehicle addpassenger
bool ChatHandler::HandleVehicleAddPassengerCommand(const char* args, WorldSession* session)
{
    std::stringstream ss(args);
    uint32_t creature_entry;
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
    Unit* u = session->GetPlayer()->getWorldMap()->getUnit(session->GetPlayer()->getTargetGuid());
    if (u == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->getVehicleKit() == nullptr)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (!u->getVehicleKit()->hasEmptySeat())
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
    Creature* c = u->getWorldMap()->createCreature(creature_entry);
    c->Load(cp, u->GetPositionX(), u->GetPositionY(), u->GetPositionZ(), u->GetOrientation());
    c->PushToWorld(u->getWorldMap());
    c->callEnterVehicle(u);
    return true;
}
#endif
