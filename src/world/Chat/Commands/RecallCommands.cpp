/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <sstream>

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Strings.hpp"

//.recall port
bool ChatCommandHandler::HandleRecallGoCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        if (m_session->GetPlayer() != nullptr)
        {
            m_session->GetPlayer()->safeTeleport(recall->mapId, 0, recall->location);
            return true;
        }
    }

    return false;
}

//.recall portus
bool ChatCommandHandler::HandleRecallPortUsCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* player = m_session->GetPlayer();
    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        Player* target = GetSelectedPlayer(m_session, true, false);
        if (!target)
            return true;

        player->safeTeleport(recall->mapId, 0, recall->location);
        target->safeTeleport(recall->mapId, 0, recall->location);
        return true;
    }

    return false;
}

//.recall add
bool ChatCommandHandler::HandleRecallAddCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        redSystemMessage(m_session, "Name in use, please use another name for your location.");
        return true;
    }

    Player* player = m_session->GetPlayer();
    std::stringstream ss;

    ss << "INSERT INTO recall (name, min_build, max_build, mapid, positionX, positionY, positionZ, Orientation) VALUES ('"
        << WorldDatabase.EscapeString(args) << "' , "
        << getAEVersion() << ", "
        << getAEVersion() << ", "
        << player->GetMapId() << ", "
        << player->GetPositionX() << ", "
        << player->GetPositionY() << ", "
        << player->GetPositionZ() << ", "
        << player->GetOrientation() << ");";
    WorldDatabase.Execute(ss.str().c_str());

    sMySQLStore.loadRecallTable();

    greenSystemMessage(m_session, "Added location to DB with MapID: {}, X: {}, Y: {}, Z: {}, O: {}",
             player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());

    sGMLog.writefromsession(m_session, "used recall add, added \'%s\' location to database.", args);

    return true;
}

//.recall del
bool ChatCommandHandler::HandleRecallDelCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        WorldDatabase.Execute("DELETE FROM recall WHERE name = %s;", recall->name.c_str());

        greenSystemMessage(m_session, "Recall location removed.");
        sGMLog.writefromsession(m_session, "used recall delete, removed \'%s\' location from database.", args);

        sMySQLStore.loadRecallTable();

        return true;
    }

    return false;
}

//.recall list
bool ChatCommandHandler::HandleRecallListCommand(const char* args, WorldSession* m_session)
{
    uint32_t count = 0;

    std::string recout = MSG_COLOR_GREEN;
    recout += "Recall locations|r:\n\n";

    std::string search(args);
    AscEmu::Util::Strings::toLowerCase(search);

    for (const auto& recall : sMySQLStore.getRecallStore())
    {
        std::string recallName(recall->name);
        AscEmu::Util::Strings::toLowerCase(recallName);
        if (recallName.find(search) == 0)
        {
            recout += MSG_COLOR_LIGHTBLUE;
            recout += recall->name;
            recout += "|r, ";
            count++;

            if (count == 5)
            {
                recout += "\n";
                count = 0;
            }
        }
    }

    SendMultilineMessage(m_session, recout.c_str());

    return true;
}

//.recall portplayer
bool ChatCommandHandler::HandleRecallPortPlayerCommand(const char* args, WorldSession* m_session)
{
    if (!args || !*args)
        return false;

    std::istringstream iss(std::string{ args });
    std::string playerName;
    std::string location;

    if (!(iss >> playerName >> location))
        return false;

    Player* player = sObjectMgr.getPlayer(playerName.c_str(), false);
    if (!player)
        return false;

    if (const auto recall = sMySQLStore.getRecallByName(location))
    {
        sGMLog.writefromsession(m_session, "ported %s to %s ( map: %u, x: %f, y: %f, z: %f, 0: %f )", player->getName().c_str(), recall->name.c_str(), recall->mapId, recall->location.x, recall->location.y, recall->location.z, recall->location.o);
        if (player->getSession() && (player->getSession()->CanUseCommand('a') || !m_session->GetPlayer()->m_isGmInvisible))
            player->getSession()->SystemMessage("%s teleported you to location %s!", m_session->GetPlayer()->getName().c_str(), recall->name.c_str());

        if (player->GetInstanceID() != m_session->GetPlayer()->GetInstanceID())
            sEventMgr.AddEvent(player, &Player::eventTeleport, recall->mapId, recall->location, uint32_t(0), EVENT_PLAYER_TELEPORT, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else
            player->safeTeleport(recall->mapId, 0, recall->location);

        return true;
    }

    return false;
}
