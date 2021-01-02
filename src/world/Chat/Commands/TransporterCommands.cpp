/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Exceptions/Exceptions.hpp"
#include "Storage/MySQLDataStore.hpp"

bool ChatHandler::HandleGetTransporterTime(const char* /*args*/, WorldSession* m_session)
{
    auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_guid));
    if (transporter)
        transporter->GetTimer();
    else
    {
        RedSystemMessage(m_session, "You must be on a transport to use this command.");
        return true;
    }

    SystemMessage(m_session, "Current period: %dms", transporter->GetTransportTemplate()->pathTime);

    return true;
}

bool ChatHandler::HandleGetTransporterInfo(const char* /*args*/, WorldSession* m_session)
{
    auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_guid));
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
        SystemMessage(m_session, "Time on Path: %u", transporter->GetTimer());
        SystemMessage(m_session, "Period: %u", transporter->GetTransportTemplate()->pathTime);
        SystemMessage(m_session, "Current WP: %u", transporter->getCurrentFrame());
    }

    return true;
}

bool ChatHandler::HandleStopTransport(const char* /*args*/, WorldSession* m_session)
{
    auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_guid));
    if (transporter)
        transporter->EnableMovement(false, m_session->GetPlayer()->GetMapMgr());
    else
        RedSystemMessage(m_session, "You must be on a transport to use this command.");
    
    return true;
}

bool ChatHandler::HandleStartTransport(const char* /*args*/, WorldSession* m_session)
{
    Transporter* transport = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_guid));
    if (transport)
        transport->EnableMovement(true, m_session->GetPlayer()->GetMapMgr());
    else
        RedSystemMessage(m_session, "You must be on a transport to use this command.");

    return true;
}

bool ChatHandler::HandleSpawnInstanceTransport(const char* args, WorldSession* m_session)
{
    char* pEntry = strtok(const_cast<char*>(args), " ");
    if (!pEntry)
        return false;

    uint32 entry = atoi(pEntry);
    sTransportHandler.createTransport(entry, (m_session->GetPlayerOrThrow()->GetMapMgr()));

    return true;
}
