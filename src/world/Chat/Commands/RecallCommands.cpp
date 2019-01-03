/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"
#include "Objects/ObjectMgr.h"

bool GetRecallLocation(const char* location, uint32_t& mapId, LocationVector& position)
{
    QueryResult* result = WorldDatabase.Query("SELECT id, name, MapId, positionX, positionY, positionZ, Orientation FROM recall "
        "WHERE min_build <= %u AND max_build >= %u ORDER BY name", getAEVersion(), getAEVersion());

    if (!result)
        return false;

    do
    {
        Field* fields = result->Fetch();
        std::string locname = fields[1].GetString();
        uint32_t locMap = fields[2].GetUInt32();
        float x = fields[3].GetFloat();
        float y = fields[4].GetFloat();
        float z = fields[5].GetFloat();
        float o = fields[6].GetFloat();

        std::string searchLocation(location);

        Util::StringToLowerCase(locname);
        Util::StringToLowerCase(searchLocation);

        if (locname == searchLocation)
        {
            mapId = locMap;
            position.x = x;
            position.y = y;
            position.z = z;
            position.o = o;

            delete result;
            return true;
        }
    } while (result->NextRow());

    delete result;
    return false;
}

//.recall port
bool ChatHandler::HandleRecallGoCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    uint32_t mapId;
    LocationVector position;
    if (GetRecallLocation(args, mapId, position))
    {
        if (m_session->GetPlayer() != nullptr)
        {
            m_session->GetPlayer()->SafeTeleport(mapId, 0, position);
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

    uint32_t mapId;
    LocationVector position;

    Player* player = m_session->GetPlayer();
    if (GetRecallLocation(args, mapId, position))
    {
        Player* target = GetSelectedPlayer(m_session, true, false);
        if (!target)
            return true;

        player->SafeTeleport(mapId, 0, position);
        target->SafeTeleport(mapId, 0, position);
        return true;
    }

    return false;
}

//.recall add
bool ChatHandler::HandleRecallAddCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    QueryResult* result = WorldDatabase.Query("SELECT name FROM recall WHERE min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());
    if (!result)
        return false;
    do
    {
        Field* fields = result->Fetch();
        std::string locname = fields[0].GetString();

        std::string searchLocation(args);

        Util::StringToLowerCase(locname);
        Util::StringToLowerCase(searchLocation);

        if (locname == searchLocation)
        {
            RedSystemMessage(m_session, "Name in use, please use another name for your location.");
            delete result;
            return true;
        }
    } while (result->NextRow());
    delete result;

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

    QueryResult* result = WorldDatabase.Query("SELECT id, name FROM recall WHERE min_build <= %u AND max_build >= %u", getAEVersion(), getAEVersion());
    if (!result)
        return false;

    do
    {
        Field* fields = result->Fetch();
        uint32_t id = fields[0].GetUInt32();
        std::string locName = fields[1].GetString();

        std::string searchLocation(args);

        Util::StringToLowerCase(locName);
        Util::StringToLowerCase(searchLocation);

        if (locName == searchLocation)
        {
            WorldDatabase.Execute("DELETE FROM recall WHERE id = %u;", id);

            GreenSystemMessage(m_session, "Recall location removed.");
            sGMLog.writefromsession(m_session, "used recall delete, removed \'%s\' location from database.", args);

            delete result;
            return true;
        }
    } while (result->NextRow());

    delete result;
    return false;
}

//.recall list
bool ChatHandler::HandleRecallListCommand(const char* args, WorldSession* m_session)
{
    QueryResult* result = WorldDatabase.Query("SELECT name FROM recall WHERE name LIKE '%s%c' AND min_build <= %u AND max_build >= %u ORDER BY name", args, '%', getAEVersion(), getAEVersion());
    if (!result)
        return false;

    uint32_t count = 0;

    std::string recout = MSG_COLOR_GREEN;
    recout += "Recall locations|r:\n\n";

    do
    {
        Field* fields = result->Fetch();
        std::string locName = fields[0].GetString();

        recout += MSG_COLOR_LIGHTBLUE;
        recout += locName;
        recout += "|r, ";
        count++;

        if (count == 5)
        {
            recout += "\n";
            count = 0;
        }
    } while (result->NextRow());

    SendMultilineMessage(m_session, recout.c_str());

    delete result;
    return true;
}

//.recall portplayer
bool ChatHandler::HandleRecallPortPlayerCommand(const char* args, WorldSession* m_session)
{
    char location[255];
    char playerName[255];
    if (sscanf(args, "%s %s", playerName, location) != 2)
        return false;

    Player* player = objmgr.GetPlayer(playerName, false);
    if (!player)
        return false;

    QueryResult* result = WorldDatabase.Query("SELECT id, name, MapId, positionX, positionY, positionZ, Orientation FROM recall WHERE min_build <= %u AND max_build >= %u ORDER BY name", getAEVersion(), getAEVersion());
    if (!result)
        return false;

    do
    {
        Field* fields = result->Fetch();
        std::string locName = fields[1].GetString();
        uint32_t locMap = fields[2].GetUInt32();
        float x = fields[3].GetFloat();
        float y = fields[4].GetFloat();
        float z = fields[5].GetFloat();
        float o = fields[6].GetFloat();

        std::string searchLocation(location);

        Util::StringToLowerCase(locName);
        Util::StringToLowerCase(searchLocation);

        if (locName == searchLocation)
        {
            sGMLog.writefromsession(m_session, "ported %s to %s ( map: %u, x: %f, y: %f, z: %f, 0: %f )", player->getName().c_str(), locName.c_str(), locMap, x, y, z, o);
            if (player->GetSession() && (player->GetSession()->CanUseCommand('a') || !m_session->GetPlayer()->m_isGmInvisible))
                player->GetSession()->SystemMessage("%s teleported you to location %s!", m_session->GetPlayer()->getName().c_str(), locName.c_str());

            if (player->GetInstanceID() != m_session->GetPlayer()->GetInstanceID())
                sEventMgr.AddEvent(player, &Player::EventSafeTeleport, locMap, uint32_t(0), LocationVector(x, y, z, o), EVENT_PLAYER_TELEPORT, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            else
                player->SafeTeleport(locMap, 0, LocationVector(x, y, z, o));

            delete result;
            return true;
        }

    } while (result->NextRow());

    delete result;
    return false;
}
