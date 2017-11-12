/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "ConsoleCommands.h"
#include <git_version.h>
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/Console/BaseConsole.h"
#include "Server/MainServerDefines.h"
#include "Server/Master.h"
#include "crc32.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Objects/ObjectMgr.h"


bool handleSendChatAnnounceCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::stringstream worldAnnounce;
    worldAnnounce << MSG_COLOR_LIGHTBLUE << "Console: |r" << consoleInput;
    sWorld.sendMessageToAll(worldAnnounce.str());

    baseConsole->Write("Message '%s' sent.\r\n", worldAnnounce.str().c_str());

    return true;
}

bool handleBanAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::string accountName;
    std::string timePeriod;
    std::string banReason;

    std::stringstream accountBanStream(consoleInput);
    accountBanStream >> accountName;
    accountBanStream >> timePeriod;
    std::getline(accountBanStream, banReason);

    uint32_t timeperiod = Util::GetTimePeriodFromString(timePeriod.c_str());
    if (timeperiod == 0)
        return false;

    uint32_t banned = (uint32_t)UNIXTIME + timeperiod;

    sLogonCommHandler.setAccountBanned(accountName.c_str(), banned, banReason.c_str());

    baseConsole->Write("Account '%s' has been banned until %s. The change will be effective immediately.\r\n", accountName.c_str(),
        Util::GetDateTimeStringFromTimeStamp(timeperiod + (uint32_t)UNIXTIME).c_str());

    return true;
}

bool handleCreateAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::string accountName;
    std::string password;
    std::string none("none");

    std::stringstream accountBanStream(consoleInput);
    accountBanStream >> accountName;
    accountBanStream >> password;
    std::getline(accountBanStream, password);

    sLogonCommHandler.createAccount(accountName.c_str(), password.c_str(), none.c_str());

    baseConsole->Write("Account '%s' has been created with password: '%s'. The change will be effective immediately.\r\n", accountName.c_str(), password.c_str());

    return true;
}

bool handleAccountPermission(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::string accountName;
    std::string permission;
    std::string none("none");

    std::stringstream accountBanStream(consoleInput);
    accountBanStream >> accountName;
    accountBanStream >> permission;
    std::getline(accountBanStream, permission);

    sLogonCommHandler.checkIfAccountExist(accountName.c_str(), none.c_str(), permission.c_str());

    baseConsole->Write("Permission '%s' has been set for Account: '%s'. The change will be effective immediately.\r\n", permission.c_str(), accountName.c_str());

    return true;
}

bool handleCancelShutdownCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    sMaster.m_ShutdownTimer = 5000;
    sMaster.m_ShutdownEvent = false;

    baseConsole->Write("Shutdown has been canceled.\r\n");

    return true;
}

bool handleServerInfoCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    uint32_t clientsNum = (uint32_t)sWorld.getSessionCount();

    int gmCount = 0;
    int onlineCount = 0;
    int avgLatency = 0;

    objmgr._playerslock.AcquireReadLock();
    for (PlayerStorageMap::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession())
        {
            onlineCount++;
            avgLatency += itr->second->GetSession()->GetLatency();
            if (itr->second->GetSession()->GetPermissionCount())
                gmCount++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    if (isWebClient)
    {
        // send pure data to webclient
        baseConsole->Write("'%d', '%.3f', '%3.2f', '%4.2f'\r\n", clientsNum, onlineCount ? ((float)((float)avgLatency / (float)onlineCount)) : 0.0f, sWorld.getCPUUsage(), sWorld.getRAMUsage());
    }
    else
    {
        baseConsole->Write("======================================================================\r\n");
        baseConsole->Write("Server Information: \r\n");
        baseConsole->Write("======================================================================\r\n");
        baseConsole->Write("Server Revision: AscEmu %s/%s-%s-%s (www.ascemu.org)\r\n", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
        baseConsole->Write("Server Uptime: %s\r\n", sWorld.getWorldUptimeString().c_str());
        baseConsole->Write("Current Players: %d (%d GMs, %d queued)\r\n", clientsNum, gmCount, 0);
        baseConsole->Write("Active Thread Count: %u\r\n", ThreadPool.GetActiveThreadCount());
        baseConsole->Write("Free Thread Count: %u\r\n", ThreadPool.GetFreeThreadCount());
        baseConsole->Write("Average Latency: %.3fms\r\n", onlineCount ? ((float)((float)avgLatency / (float)onlineCount)) : 0.0f);
        baseConsole->Write("CPU Usage: %3.2f %%\r\n", sWorld.getCPUUsage());
        baseConsole->Write("RAM Usage: %4.2f MB\r\n", sWorld.getRAMUsage());
        baseConsole->Write("SQL Query Cache Size (World): %u queries delayed\r\n", WorldDatabase.GetQueueSize());
        baseConsole->Write("SQL Query Cache Size (Character): %u queries delayed\r\n", CharacterDatabase.GetQueueSize());
    }

    sSocketMgr.ShowStatus();

    return true;
}

bool handleOnlineGmsCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    baseConsole->Write("There are the following GM's online on this server: \r\n");
    baseConsole->Write("======================================================\r\n");
    baseConsole->Write("| %21s | %15s | % 03s  |\r\n", "Name", "Permissions", "Latency");
    baseConsole->Write("======================================================\r\n");

    objmgr._playerslock.AcquireReadLock();
    for (PlayerStorageMap::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession()->GetPermissionCount())
        {
            baseConsole->Write("| %21s | %15s | %03u ms |\r\n", itr->second->GetName(), itr->second->GetSession()->GetPermissions(),
                itr->second->GetSession()->GetLatency());
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    baseConsole->Write("======================================================\r\n\r\n");

    return true;
}

bool handleKickPlayerCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::string characterName;
    std::string kickReason;

    std::stringstream characterKickStream(consoleInput);
    characterKickStream >> characterName;
    std::getline(characterKickStream, kickReason);

    if (characterName.empty())
        return false;

    Player* player = objmgr.GetPlayer(characterName.c_str());
    if (player == nullptr)
    {
        baseConsole->Write("Could not find player, %s.\r\n", characterName.c_str());
        return true;
    }

    std::stringstream worldAnnounce;
    worldAnnounce << MSG_COLOR_LIGHTBLUE << "Console:|r " << player->GetName() << " was removed from the server. Reason: " << kickReason;
    sWorld.sendMessageToAll(worldAnnounce.str());

    player->BroadcastMessage("You are now being removed by the game by an administrator via the console. Reason: %s", kickReason.c_str());
    player->Kick(5000);
    baseConsole->Write("Kicked player %s.\r\n", player->GetName());

    return true;
}

bool handleMotdCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    if (argumentCount == 0)
    {
        baseConsole->Write("The current message of the day is: '%s'.\r\n", worldConfig.getMessageOfTheDay().c_str());
    }
    else
    {
        worldConfig.setMessageOfTheDay(consoleInput);
        baseConsole->Write("The message of the day has been set to: '%s'.\r\n", worldConfig.getMessageOfTheDay().c_str());
    }

    return true;
}

bool handleListOnlinePlayersCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    baseConsole->Write("There following players online on this server: \r\n");
    baseConsole->Write("======================================================\r\n");
    baseConsole->Write("| %21s | %15s | % 03s  |\r\n", "Name", "Level", "Latency");
    baseConsole->Write("======================================================\r\n");

    objmgr._playerslock.AcquireReadLock();
    for (PlayerStorageMap::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        baseConsole->Write("| %21s | %15u | %03u ms |\r\n", itr->second->GetName(), itr->second->GetSession()->GetPlayer()->getLevel(),
            itr->second->GetSession()->GetLatency());
    }
    objmgr._playerslock.ReleaseReadLock();

    baseConsole->Write("======================================================\r\n\r\n");
    return true;
}

bool handlePlayerInfoCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    Player* player = objmgr.GetPlayer(consoleInput.c_str());
    if (player == nullptr)
    {
        baseConsole->Write("Player not found.\r\n");
        return true;
    }

    baseConsole->Write("Player: %s\r\n", player->GetName());
    baseConsole->Write("Race: %s\r\n", player->myRace->name[0]);
    baseConsole->Write("Class: %s\r\n", player->myClass->name[0]);
    baseConsole->Write("IP: %s\r\n", player->GetSession()->GetSocket() ? player->GetSession()->GetSocket()->GetRemoteIP().c_str() : "disconnected");
    baseConsole->Write("Level: %u\r\n", player->getLevel());
    baseConsole->Write("Account: %s\r\n", player->GetSession()->GetAccountNameS());
    return true;
}

bool handleShutDownServerCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string consoleInput, bool isWebClient)
{
    uint32_t delay = 5;

    if (consoleInput.empty())
    {
        objmgr._playerslock.AcquireReadLock();
        for (PlayerStorageMap::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
        {
            if (itr->second->GetSession())
                itr->second->SaveToDB(false);
        }
        objmgr._playerslock.ReleaseReadLock();

        exit(0);
    }
    else
    {
        delay = atoi(consoleInput.c_str());
    }

    sMaster.m_ShutdownTimer = delay * 1000;
    sMaster.m_ShutdownEvent = true;
    baseConsole->Write("Shutdown has initiated.\r\n");

    return true;
}

bool handleRehashConfigCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    baseConsole->Write("Config file re-parsed.\r\n");
    sWorld.loadWorldConfigValues(true);

    return true;
}

bool handleUnbanAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    sLogonCommHandler.setAccountBanned(consoleInput.c_str(), 0, "");
    baseConsole->Write("Account '%s' has been unbanned.\r\n", consoleInput.c_str());

    return true;
}

bool handleSendWAnnounceCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::stringstream areaMessage;
    areaMessage << MSG_COLOR_LIGHTBLUE << "Console: |r" << consoleInput;

    sWorld.sendAreaTriggerMessage(areaMessage.str());
    baseConsole->Write("Message '%s'  has been sent to all clients.\r\n", consoleInput.c_str());

    return true;
}

bool handleWhisperCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::string characterName;
    std::string whisperMessage;

    std::stringstream whisperStream(consoleInput);

    whisperStream >> characterName;
    std::getline(whisperStream, whisperMessage);

    if (whisperMessage.empty())
        return false;

    Player* player = objmgr.GetPlayer(characterName.c_str());
    if (player == nullptr)
    {
        baseConsole->Write("Could not find player, %s.\r\n", characterName.c_str());
        return true;
    }

    std::stringstream whisperOut;
    whisperOut << MSG_COLOR_LIGHTBLUE << "Console whisper: |r" << consoleInput;

    player->BroadcastMessage(whisperOut.str().c_str());
    baseConsole->Write("Message '%s' sent to player %s.\r\n", consoleInput.c_str(), player->GetName());

    return true;
}

bool handleCreateNameHashCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    baseConsole->Write("Name Hash for %s is 0x%X \r\n", consoleInput.c_str(), crc32((const unsigned char*)consoleInput.c_str(), 
        (unsigned int)consoleInput.length()));

    return true;
}

bool handleRevivePlayerCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    Player* player = objmgr.GetPlayer(consoleInput.c_str(), false);
    if (player == nullptr)
    {
        baseConsole->Write("Could not find player %s.\r\n", consoleInput.c_str());
        return true;
    }

    if (player->IsDead())
    {
        player->RemoteRevive();
        baseConsole->Write("Revived player %s.\r\n", player->GetName());
    }
    else
    {
        baseConsole->Write("Player %s is not dead.\r\n", player->GetName());
    }

    return true;
}

bool handleClearConsoleCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    system("cls");
    baseConsole->Write("Out of the ashes, Chuck Norris appears! With a roundhouse kick, your console shall now be cleaned!\r\n");

    return true;
}

bool handleReloadScriptEngineCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    sScriptMgr.ReloadScriptEngines();

    if (isWebClient)
    {
        baseConsole->Write("All scripts should be reloaded.\r\n");
    }

    return true;
}

bool handlePrintTimeDateCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    std::string current_time = Util::GetCurrentDateTimeString();

    std::stringstream currentTimeStream;
    currentTimeStream << "Date and time according to localtime() (american style): " << current_time << std::endl;

    baseConsole->Write(currentTimeStream.str().c_str());

    return true;
}

bool handleGetAccountsCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient)
{
    sLogonCommHandler.requestAccountData();

    std::cout << "Command result is: " << sLogonCommHandler.accountResult << std::endl;

    baseConsole->Write("%s\r\n", sLogonCommHandler.accountResult.c_str());

    return true;
}
