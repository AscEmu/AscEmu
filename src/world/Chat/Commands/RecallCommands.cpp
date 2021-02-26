/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"
#include "Objects/ObjectMgr.h"
#include "Storage/MySQLDataStore.hpp"


//.recall port
bool ChatHandler::HandleRecallGoCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        if (m_session->GetPlayer() != nullptr)
        {
            m_session->GetPlayer()->SafeTeleport(recall->mapId, 0, recall->location);
            return true;
        }
    }

    return false;
}

//.recall portus
bool ChatHandler::HandleRecallPortUsCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* player = m_session->GetPlayer();
    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        Player* target = GetSelectedPlayer(m_session, true, false);
        if (!target)
            return true;

        player->SafeTeleport(recall->mapId, 0, recall->location);
        target->SafeTeleport(recall->mapId, 0, recall->location);
        return true;
    }

    return false;
}

//.recall add
bool ChatHandler::HandleRecallAddCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        RedSystemMessage(m_session, "Name in use, please use another name for your location.");
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

    char buf[256];
    snprintf((char*)buf, 256, "Added location to DB with MapID: %u, X: %f, Y: %f, Z: %f, O: %f",
             player->GetMapId(), player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation());
    GreenSystemMessage(m_session, buf);

    sGMLog.writefromsession(m_session, "used recall add, added \'%s\' location to database.", args);

    return true;
}

//.recall del
bool ChatHandler::HandleRecallDelCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        WorldDatabase.Execute("DELETE FROM recall WHERE name = %s;", recall->name.c_str());

        GreenSystemMessage(m_session, "Recall location removed.");
        sGMLog.writefromsession(m_session, "used recall delete, removed \'%s\' location from database.", args);

        sMySQLStore.loadRecallTable();

        return true;
    }

    return false;
}

//.recall list
bool ChatHandler::HandleRecallListCommand(const char* args, WorldSession* m_session)
{
    uint32_t count = 0;

    std::string recout = MSG_COLOR_GREEN;
    recout += "Recall locations|r:\n\n";

    std::string search(args);
    Util::StringToLowerCase(search);

    for (auto* recall : sMySQLStore.getRecallStore())
    {
        std::string recallName(recall->name);
        Util::StringToLowerCase(recallName);
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
bool ChatHandler::HandleRecallPortPlayerCommand(const char* args, WorldSession* m_session)
{
    char location[255];
    char playerName[255];
    if (sscanf(args, "%s %s", playerName, location) != 2)
        return false;

    Player* player = sObjectMgr.GetPlayer(playerName, false);
    if (!player)
        return false;

    if (const auto recall = sMySQLStore.getRecallByName(args))
    {
        sGMLog.writefromsession(m_session, "ported %s to %s ( map: %u, x: %f, y: %f, z: %f, 0: %f )", player->getName().c_str(), recall->name.c_str(), recall->mapId, recall->location.x, recall->location.y, recall->location.z, recall->location.o);
        if (player->GetSession() && (player->GetSession()->CanUseCommand('a') || !m_session->GetPlayer()->m_isGmInvisible))
            player->GetSession()->SystemMessage("%s teleported you to location %s!", m_session->GetPlayer()->getName().c_str(), recall->name.c_str());

        if (player->GetInstanceID() != m_session->GetPlayer()->GetInstanceID())
            sEventMgr.AddEvent(player, &Player::EventSafeTeleport, recall->mapId, uint32_t(0), recall->location, EVENT_PLAYER_TELEPORT, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        else
            player->SafeTeleport(recall->mapId, 0, recall->location);

        return true;
    }

    return false;
}
