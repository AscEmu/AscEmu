/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Maps/InstanceMap.hpp"
#include "Map/Maps/InstanceMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Packets/SmsgInstanceReset.h"
#include "Storage/MySQLStructures.h"

using namespace AscEmu::Packets;

//.instance create
bool ChatCommandHandler::HandleCreateInstanceCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr == nullptr)
        return true;

    float x, y, z;
    uint32_t mapid;

    if (sscanf(args, "%u %f %f %f", &mapid, &x, &y, &z) != 4)
        return false;

    // Create Map Manager
    WorldMap* mgr = sMapMgr.createInstanceForPlayer(mapid, m_session->GetPlayer());
    if (mgr == nullptr)
    {
        sLogger.failure("call failed for map {}", mapid);
        return false;
    }
    sLogger.info("CreateInstanceGMCommand : GM created instance for map {}", mapid);

    LocationVector vec(x, y, z);
    m_session->GetPlayer()->safeTeleport(mgr, vec);

    return true;
}

//.instance countcreature
bool ChatCommandHandler::HandleCountCreaturesCommand(const char* args, WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if (plr == nullptr)
        return true;

    uint32_t entry;
    if (sscanf(args, "%u", &entry) != 1)
        return false;

    plr->displayCreatureSetForEntry(entry);

    return true;
}

//.instance exit
bool ChatCommandHandler::HandleExitInstanceCommand(const char* /*args*/, WorldSession* m_session)
{
    blueSystemMessage(m_session, "Attempting to exit from instance...");

    bool result = m_session->GetPlayer()->exitInstance();
    if (!result)
        redSystemMessage(m_session, "Entry points not found.");
    else
        greenSystemMessage(m_session, "Removal successful.");

    return true;
}

//.instance info
bool ChatCommandHandler::HandleGetInstanceInfoCommand(const char* args, WorldSession* m_session)
{
    Player* plr = m_session->GetPlayer();
    if (plr == nullptr)
        return false;

    bool userInput = true;
    uint32_t instanceId = (args ? atoi(args) : 0);
    if (instanceId == 0)
    {
        userInput = false;
        instanceId = plr->GetInstanceID();
        if (instanceId == 0)
            return false;
    }

    InstanceMap* instance = sMapMgr.findInstanceMap(instanceId);
    if (instance == nullptr)
    {
        if (userInput)
        {
            redSystemMessage(m_session, "Instance with id {} not found.", instanceId);
            return true;
        }
        return false;
    }

    InstanceSaved* save = sInstanceMgr.getInstanceSave(instanceId);

    std::stringstream ss;
    ss << "Instance ID: " << MSG_COLOR_CYAN << instance->getInstanceId() << "|r (" << MSG_COLOR_CYAN;
    if (instance->getBaseMap()->getMapInfo() == nullptr)
        ss << instance->getBaseMap()->getMapId();
    else
        ss << instance->getBaseMap()->getMapInfo()->name;
    ss << "|r)\n";
    ss << "Persistent: " << MSG_COLOR_CYAN << (save->canReset() ? "No" : "Yes") << "|r\n";
    if (instance->getBaseMap()->getMapInfo() != nullptr)
    {
        ss << "Type: " << MSG_COLOR_CYAN << GetMapTypeString(static_cast<uint8_t>(instance->getBaseMap()->getMapInfo()->type)) << "|r";

        if (instance->getBaseMap()->getMapInfo()->isMultimodeDungeon())
        {
            ss << " (" << MSG_COLOR_CYAN << GetDifficultyString(instance->getDifficulty()) << "|r)";
        }

        if (instance->getBaseMap()->isRaid())
        {
            ss << " (" << MSG_COLOR_CYAN << GetRaidDifficultyString(instance->getDifficulty()) << "|r)";
        }

        ss << "\n";
    }
    //ss << "Created: " << MSG_COLOR_CYAN << Util::GetDateTimeStringFromTimeStamp(0) << "|r\n";
    if (save->getResetTime() != 0)
        ss << "Expires: " << MSG_COLOR_CYAN << Util::GetDateTimeStringFromTimeStamp((uint32_t)save->getResetTime()) << "|r\n";

    if (instance == nullptr)
    {
        ss << "Status: " << MSG_COLOR_LIGHTRED << "Shut Down|r\n";
    }
    else if (!instance->hasPlayers())
    {
        /*ss << "Status: " << MSG_COLOR_LIGHTRED << "Idle|r";
        if (instance->m_mapMgr->InactiveMoveTime && !instance->m_mapMgr->GetMapInfo()->isNonInstanceMap())
            ss << " (" << MSG_COLOR_CYAN << "Shutdown in " << MSG_COLOR_LIGHTRED << (((long)instance->m_mapMgr->InactiveMoveTime - UNIXTIME) / 60) << MSG_COLOR_CYAN << " minutes|r)";
        ss << "\n";*/
    }
    else
    {
        ss << "Status: " << MSG_COLOR_GREEN << "In use|r (" << MSG_COLOR_GREEN << instance->getPlayerCount() << MSG_COLOR_CYAN << " players inside|r)\n";

    }
    SendMultilineMessage(m_session, ss.str().c_str());

    if (instance != nullptr && instance->getScript() != nullptr)
        plr->displayDataStateList();
    else
        plr->broadcastMessage("NO INSTANCE SCRIPT FOUND NO BOSS DATA AVAILABLE");

    return true;
}

//.instance reset
bool ChatCommandHandler::HandleResetInstanceCommand(const char* args, WorldSession* m_session)
{
    uint32_t instanceId;
    int argc = 1;
    char* playername = nullptr;
    char* guidString = (char*)args;

    // Parse arguments
    char* space = (char*)strchr(args, ' ');
    if (space)
    {
        *space = '\0';
        playername = space + 1;
        argc = 2;
    }

    instanceId = atoi(guidString);
    if (!instanceId)
    {
        redSystemMessage(m_session, "You must specify an instance id.");
        return true;
    }


    Player* plr;

    if (argc == 1)
        plr = GetSelectedPlayer(m_session, true, true);
    else
        plr = sObjectMgr.getPlayer(playername, false);

    if (!plr)
    {
        redSystemMessage(m_session, "Player not found");
        return true;
    }

    InstanceMap* instance = sMapMgr.findInstanceMap(instanceId);
    if (instance == nullptr)
    {
        redSystemMessage(m_session, "There's no instance with id {}.", instanceId);
        return true;
    }

    if (instance && instance->hasPlayers())
    {
        redSystemMessage(m_session, "Failed to reset non-persistent instance with id {}, due to player still inside.", instanceId);
        return true;
    }

    instance->reset(INSTANCE_RESET_GLOBAL);

    // tell player the instance was reset
    plr->getSession()->SendPacket(SmsgInstanceReset(instance->getBaseMap()->getMapId()).serialise().get());

    //    redSystemMessage(m_session, "Resetting single non-persistent instances is not available yet.");
    sGMLog.writefromsession(m_session, "used reset instance command on %s, instance %u,", plr->getName().c_str(), instanceId);
    return true;
}

//.instance resetall
bool ChatCommandHandler::HandleResetAllInstancesCommand(const char* args, WorldSession* m_session)
{
    bool is_name_set = false;
    Player* player;

    if (*args)
        is_name_set = true;

    if (is_name_set)
    {
        player = sObjectMgr.getPlayer(args, false);
        if (player == nullptr)
        {
            redSystemMessage(m_session, "Player {} is not online or does not exist!", args);
            return true;
        }
    }
    else
    {
        player = GetSelectedPlayer(m_session, true);
        if (player == nullptr)
            return true;
    }

    systemMessage(m_session, "Trying to reset all instances of player {}...", player->getName());
    player->resetInstances(INSTANCE_RESET_ALL, false);
    systemMessage(m_session, "...done");

    sGMLog.writefromsession(m_session, "used reset all instances command on %s,", player->getName().c_str());
    return true;
}

//.instance shutdown
bool ChatCommandHandler::HandleShutdownInstanceCommand(const char* args, WorldSession* m_session)
{
    uint32_t instanceId = (args ? atoi(args) : 0);
    if (instanceId == 0)
        return false;

    InstanceMap* instance = sMapMgr.findInstanceMap(instanceId);
    if (instance == nullptr)
    {
        redSystemMessage(m_session, "There's no instance with id {}.", instanceId);
        return true;
    }

    if (instance->isUnloadPending())
    {
        redSystemMessage(m_session, "Instance with id {} already shut down.", instanceId);
        return true;
    }

    systemMessage(m_session, "Attempting to shutdown instance with id {}...", instanceId);

    // Remove all Players
    instance->removeAllPlayers();
    instance->setUnloadPending(true);

    systemMessage(m_session, "...done");

    sGMLog.writefromsession(m_session, "used shutdown instance command on instance %u,", instanceId);
    return true;
}

//.instance showtimers
bool ChatCommandHandler::HandleShowTimersCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* player = m_session->GetPlayer();
    if (player == nullptr)
        return true;

    player->displayTimerList();

    return true;
}
