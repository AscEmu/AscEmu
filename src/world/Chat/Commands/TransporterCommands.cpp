/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatCommandHandler.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Transporter.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"

bool ChatCommandHandler::HandleGetTransporterTime(const char* /*args*/, WorldSession* m_session)
{
    auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_guid));
    if (transporter)
        transporter->GetTimer();
    else
    {
        redSystemMessage(m_session, "You must be on a transport to use this command.");
        return true;
    }

    systemMessage(m_session, "Current period: {}ms", transporter->GetTransportTemplate()->pathTime);

    return true;
}

bool ChatCommandHandler::HandleGetTransporterInfo(const char* /*args*/, WorldSession* m_session)
{
    auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_guid));
    if (transporter == nullptr)
    {
        redSystemMessage(m_session, "You must be on a transport to use this command.");
        return true;
    }

    auto gameobject_info = sMySQLStore.getGameObjectProperties(transporter->getEntry());
    if (gameobject_info != nullptr)
    {
        systemMessage(m_session, "Entry: {}", gameobject_info->entry);
        systemMessage(m_session, "Name: {}", gameobject_info->name.c_str());
        systemMessage(m_session, "Path: {}", gameobject_info->mo_transport.taxi_path_id);
        systemMessage(m_session, "Time on Path: {}", transporter->GetTimer());
        systemMessage(m_session, "Period: {}", transporter->GetTransportTemplate()->pathTime);
        systemMessage(m_session, "Current WP: {}", transporter->getCurrentFrame());
    }

    return true;
}

bool ChatCommandHandler::HandleStopTransport(const char* /*args*/, WorldSession* m_session)
{
    auto transporter = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_guid));
    if (transporter)
        transporter->EnableMovement(false, m_session->GetPlayer()->getWorldMap());
    else
        redSystemMessage(m_session, "You must be on a transport to use this command.");
    
    return true;
}

bool ChatCommandHandler::HandleStartTransport(const char* /*args*/, WorldSession* m_session)
{
    Transporter* transport = sTransportHandler.getTransporter(WoWGuid::getGuidLowPartFromUInt64(m_session->GetPlayerOrThrow()->obj_movement_info.transport_guid));
    if (transport)
        transport->EnableMovement(true, m_session->GetPlayer()->getWorldMap());
    else
        redSystemMessage(m_session, "You must be on a transport to use this command.");

    return true;
}

bool ChatCommandHandler::HandleSpawnInstanceTransport(const char* args, WorldSession* m_session)
{
    char* pEntry = strtok(const_cast<char*>(args), " ");
    if (!pEntry)
        return false;

    uint32_t entry = atoi(pEntry);
    sTransportHandler.createTransport(entry, (m_session->GetPlayerOrThrow()->getWorldMap()));

    return true;
}
