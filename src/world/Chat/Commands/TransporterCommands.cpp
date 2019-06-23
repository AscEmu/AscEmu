/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Exceptions/Exceptions.hpp"
#include "Storage/MySQLDataStore.hpp"

bool ChatHandler::HandleGetTransporterTime(const char* /*args*/, WorldSession* m_session)
{
#if VERSION_STRING < Cata
    auto transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_data.transportGuid));
#else
    auto transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.getTransportGuid()));
#endif
    if (transporter == nullptr)
    {
        RedSystemMessage(m_session, "You must be on a transport to use this command.");
        return true;
    }

    SystemMessage(m_session, "Current period: %dms", transporter->GetPeriod());

    return true;
}

bool ChatHandler::HandleGetTransporterInfo(const char* /*args*/, WorldSession* m_session)
{
#if VERSION_STRING < Cata
    auto transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_data.transportGuid));
#else
    auto transporter = objmgr.GetTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.getTransportGuid()));
#endif
    if (transporter == nullptr)
    {
        RedSystemMessage(m_session, "You must be on a transport to use this command.");
        return true;
    }

    auto gameobject_info = sMySQLStore.getGameObjectProperties(transporter->getEntry());
    if (gameobject_info != nullptr)
    {
        SystemMessage(m_session, "Entry: %u", gameobject_info->entry);
        SystemMessage(m_session, "Name: %s", gameobject_info->name.c_str());
        SystemMessage(m_session, "Path: %u", gameobject_info->mo_transport.taxi_path_id);
        SystemMessage(m_session, "Time on Path: %u", transporter->m_timer);
        SystemMessage(m_session, "Period: %u", transporter->GetPeriod());
        //SystemMessage(m_session, "Current WP: %u", transporter->mCurrentWaypoint->first);
    }

    return true;
}

bool ChatHandler::HandleModPeriodCommand(const char* args, WorldSession* m_session)
{
    try
    {
        int32 time = args ? atol(args) : 0;
        if (time == 0)
            return false;

#if VERSION_STRING < Cata
        Transporter* transport = objmgr.GetTransportOrThrow(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_data.transportGuid));
#else
        Transporter* transport = objmgr.GetTransportOrThrow(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.getTransportGuid()));
#endif
        transport->SetPeriod(time);
        BlueSystemMessage(m_session, "Period of %s set to %u.", transport->GetGameObjectProperties()->name.c_str(), time);
    }
    catch (AscEmu::Exception::AscemuException e)
    {
        RedSystemMessage(m_session, e.AEwhat());
    }

    return true;
}

bool ChatHandler::HandleStopTransport(const char* /*args*/, WorldSession* m_session)
{
    try
    {
#if VERSION_STRING < Cata
        Transporter* transport = objmgr.GetTransportOrThrow(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_data.transportGuid));
#else
        Transporter* transport = objmgr.GetTransportOrThrow(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.getTransportGuid()));
#endif
        if (transport->getState() == GO_STATE_OPEN)
        {
            transport->m_WayPoints.clear();
            transport->removeFlags(GO_FLAG_NONSELECTABLE);
            transport->setState(GO_STATE_CLOSED);
        }
    }
    catch (AscEmu::Exception::AscemuException e)
    {
        RedSystemMessage(m_session, e.AEwhat());
    }

    return true;
}

bool ChatHandler::HandleStartTransport(const char* /*args*/, WorldSession* m_session)
{
    try
    {
#if VERSION_STRING < Cata
        Transporter* transport = objmgr.GetTransportOrThrow(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_data.transportGuid));
#else
        Transporter* transport = objmgr.GetTransportOrThrow(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.getTransportGuid()));
#endif
        if (transport->getState() == GO_STATE_CLOSED)
        {
            transport->setFlags(GO_FLAG_NONSELECTABLE);
            transport->setState(GO_STATE_OPEN);
            transport->setDynamic(0x10830010); //\todo When people see things in sniffs... probably wrong
            transport->setParentRotation(3, 1.0f);
            std::set<uint32> mapsUsed;
            GameObjectProperties const* goinfo = transport->GetGameObjectProperties();

            transport->GenerateWaypoints(goinfo->raw.parameter_0);
        }
    }
    catch (AscEmu::Exception::AscemuException e)
    {
        RedSystemMessage(m_session, e.AEwhat());
    }

    return true;
}

bool ChatHandler::HandleSpawnInstanceTransport(const char* args, WorldSession* m_session)
{
    char* pEntry = strtok(const_cast<char*>(args), " ");
    if (!pEntry)
        return false;

    char* pPeriod = strtok(nullptr, " ");
    if (!pPeriod)
        return false;

    uint32 entry = atoi(pEntry);
    uint32 period = atoi(pPeriod);

    objmgr.LoadTransportInInstance(m_session->GetPlayerOrThrow()->GetMapMgr(), entry, period);

    return true;
}

bool ChatHandler::HandleDespawnInstanceTransport(const char* /*args*/, WorldSession* m_session)
{
    try
    {
#if VERSION_STRING < Cata
        Transporter* transport = objmgr.GetTransportOrThrow(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_data.transportGuid));
#else
        Transporter* transport = objmgr.GetTransportOrThrow(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.getTransportGuid()));
#endif
        objmgr.UnloadTransportFromInstance(transport);
    }
    catch (AscEmu::Exception::AscemuException e)
    {
        RedSystemMessage(m_session, e.AEwhat());
    }

    return true;
}
