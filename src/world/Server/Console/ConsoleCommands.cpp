/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ConsoleCommands.h"
#include <git_version.hpp>
#include <iostream>
#include <sstream>

#include "Common.hpp"
#include "Chat/ChatDefines.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/Console/BaseConsole.h"
#include "Server/Master.h"
#include "Management/MailMgr.h"
#include "Server/World.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSocket.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Strings.hpp"
#include "Threading/LegacyThreading.h"
#include "Utilities/Util.hpp"

#include <openssl/opensslv.h>
#include <openssl/crypto.h>

bool handleSendChatAnnounceCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::stringstream worldAnnounce;
    worldAnnounce << MSG_COLOR_LIGHTBLUE << "Console: |r" << consoleInput;
    sWorld.sendMessageToAll(worldAnnounce.str());

    baseConsole->Write("Message '%s' sent.\r\n", worldAnnounce.str().c_str());

    return true;
}

bool handleBanAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
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

bool handleCreateAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
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

bool handleAccountPermission(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
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

bool handleCancelShutdownCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool /*isWebClient*/)
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

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        const Player* player = playerPair.second;
        if (player->getSession())
        {
            onlineCount++;
            avgLatency += player->getSession()->GetLatency();
            if (player->getSession()->hasPermissions())
                gmCount++;
        }
    }

    if (isWebClient)
    {
        // send pure data to webclient
        baseConsole->Write("'%d', '%.3f', '%3.2f', '%4.2f'\r\n", clientsNum, onlineCount ? (float)avgLatency / (float)onlineCount : 0.0f, sWorld.getCPUUsage(), sWorld.getRAMUsage());
    }
    else
    {
        baseConsole->Write("======================================================================\r\n");
        baseConsole->Write("Server Information: \r\n");
        baseConsole->Write("======================================================================\r\n");
        baseConsole->Write("Info: AscEmu %s/%s-%s-%s (www.ascemu.org)\r\n", AE_BUILD_HASH, CONFIG, AE_PLATFORM, AE_ARCHITECTURE);
        baseConsole->Write("Using %s/Library %s\r\n", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
        baseConsole->Write("Uptime: %s\r\n", sWorld.getWorldUptimeString().c_str());
        baseConsole->Write("Active Branch: %s\r\n", AE_BUILD_BRANCH);
        baseConsole->Write("Current Players: %d (%d GMs, %d queued)\r\n", clientsNum, gmCount, 0);
        baseConsole->Write("Active Thread Count: %u\r\n", ThreadPool.GetActiveThreadCount());
        baseConsole->Write("Free Thread Count: %u\r\n", ThreadPool.GetFreeThreadCount());
        baseConsole->Write("Average Latency: %.3fms\r\n", onlineCount ? (float)avgLatency / (float)onlineCount : 0.0f);
        baseConsole->Write("CPU Usage: %3.2f %%\r\n", sWorld.getCPUUsage());
        baseConsole->Write("RAM Usage: %4.2f MB\r\n", sWorld.getRAMUsage());
        baseConsole->Write("SQL Query Cache Size (World): %u queries delayed\r\n", WorldDatabase.GetQueueSize());
        baseConsole->Write("SQL Query Cache Size (Character): %u queries delayed\r\n", CharacterDatabase.GetQueueSize());
    }

    sSocketMgr.ShowStatus();

    return true;
}

bool handleOnlineGmsCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool /*isWebClient*/)
{
    baseConsole->Write("There are the following GM's online on this server: \r\n");
    baseConsole->Write("======================================================================\r\n");
    baseConsole->Write("| %21s | %15s | % 03s                                                |\r\n", "Name", "Permissions", "Latency");
    baseConsole->Write("======================================================================\r\n");

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        const Player* player = playerPair.second;
        if (player->getSession()->hasPermissions())
        {
            baseConsole->Write("| %21s | %15s | %03u ms |\r\n", player->getName().c_str(), player->getSession()->GetPermissions().get(),
                player->getSession()->GetLatency());
        }
    }

    baseConsole->Write("======================================================================\r\n\r\n");

    return true;
}

bool handleKickPlayerCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
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

    Player* player = sObjectMgr.getPlayer(characterName.c_str());
    if (player == nullptr)
    {
        baseConsole->Write("Could not find player, %s.\r\n", characterName.c_str());
        return true;
    }

    std::stringstream worldAnnounce;
    worldAnnounce << MSG_COLOR_LIGHTBLUE << "Console:|r " << player->getName().c_str() << " was removed from the server. Reason: " << kickReason;
    sWorld.sendMessageToAll(worldAnnounce.str());

    player->broadcastMessage("You are now being removed by the game by an administrator via the console. Reason: %s", kickReason.c_str());
    player->kickFromServer(5000);
    baseConsole->Write("Kicked player %s.\r\n", player->getName().c_str());

    return true;
}

bool handleMotdCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
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

bool handleListOnlinePlayersCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool /*isWebClient*/)
{
    baseConsole->Write("There following players online on this server: \r\n");
    baseConsole->Write("======================================================================\r\n");
    baseConsole->Write("| %21s | %15s | % 03s                  |\r\n", "Name", "Level", "Latency");
    baseConsole->Write("======================================================================\r\n");

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        const Player* player = playerPair.second;
        baseConsole->Write("| %21s | %15u | %03u ms                   |\r\n", player->getName().c_str(), player->getSession()->GetPlayer()->getLevel(),
            player->getSession()->GetLatency());
    }

    baseConsole->Write("======================================================================\r\n\r\n");
    return true;
}

bool handlePlayerInfoCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    Player* player = sObjectMgr.getPlayer(consoleInput.c_str());
    if (player == nullptr)
    {
        baseConsole->Write("Player not found.\r\n");
        return true;
    }

    baseConsole->Write("Player: %s\r\n", player->getName().c_str());
#if VERSION_STRING < Cata
    baseConsole->Write("Race: %s\r\n", player->getDbcRaceEntry()->name[sWorld.getDbcLocaleLanguageId()]);
    baseConsole->Write("Class: %s\r\n", player->getDbcClassEntry()->name[sWorld.getDbcLocaleLanguageId()]);
#else
    baseConsole->Write("Race: %s\r\n", player->getDbcRaceEntry()->name);
    baseConsole->Write("Class: %s\r\n", player->getDbcClassEntry()->name);
#endif
    baseConsole->Write("IP: %s\r\n", player->getSession()->GetSocket() ? player->getSession()->GetSocket()->GetRemoteIP().c_str() : "disconnected");
    baseConsole->Write("Level: %u\r\n", player->getLevel());
    baseConsole->Write("Account: %s\r\n", player->getSession()->GetAccountNameS());
    return true;
}

bool handleShutDownServerCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string consoleInput, bool /*isWebClient*/)
{
    uint32_t delay = 5;

    if (consoleInput.empty())
    {
        std::lock_guard guard(sObjectMgr.m_playerLock);
        for (const auto playerPair : sObjectMgr.getPlayerStorage())
        {
            Player* player = playerPair.second;
            if (player->getSession())
                player->saveToDB(false);
        }

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

bool handleRehashConfigCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool /*isWebClient*/)
{
    baseConsole->Write("Config file re-parsed.\r\n");
    sWorld.loadWorldConfigValues(true);

    return true;
}

bool handleUnbanAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    sLogonCommHandler.setAccountBanned(consoleInput.c_str(), 0, "");
    baseConsole->Write("Account '%s' has been unbanned.\r\n", consoleInput.c_str());

    return true;
}

bool handleSendWAnnounceCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::stringstream areaMessage;
    areaMessage << MSG_COLOR_LIGHTBLUE << "Console: |r" << consoleInput;

    sWorld.sendAreaTriggerMessage(areaMessage.str());
    baseConsole->Write("Message '%s'  has been sent to all clients.\r\n", consoleInput.c_str());

    return true;
}

bool handleWhisperCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
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

    Player* player = sObjectMgr.getPlayer(characterName.c_str());
    if (player == nullptr)
    {
        baseConsole->Write("Could not find player, %s.\r\n", characterName.c_str());
        return true;
    }

    std::stringstream whisperOut;
    whisperOut << MSG_COLOR_LIGHTBLUE << "Console whisper: |r" << consoleInput;

    player->broadcastMessage(whisperOut.str().c_str());
    baseConsole->Write("Message '%s' sent to player %s.\r\n", consoleInput.c_str(), player->getName().c_str());

    return true;
}

bool handleRevivePlayerCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    Player* player = sObjectMgr.getPlayer(consoleInput.c_str(), false);
    if (player == nullptr)
    {
        baseConsole->Write("Could not find player %s.\r\n", consoleInput.c_str());
        return true;
    }

    if (player->isDead())
    {
        player->setResurrect();
        baseConsole->Write("Revived player %s.\r\n", player->getName().c_str());
    }
    else
    {
        baseConsole->Write("Player %s is not dead.\r\n", player->getName().c_str());
    }

    return true;
}

bool handleClearConsoleCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool /*isWebClient*/)
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

bool handlePrintTimeDateCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool /*isWebClient*/)
{
    std::string current_time = Util::GetCurrentDateTimeString();

    std::stringstream currentTimeStream;
    currentTimeStream << "Date and time according to localtime() (american style): " << current_time << "\n";

    baseConsole->Write(currentTimeStream.str().c_str());

    return true;
}

bool handleGetAccountsCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool /*isWebClient*/)
{
    sLogonCommHandler.requestAccountData();

    std::cout << "Command result is: " << sLogonCommHandler.accountResult << "\n";

    baseConsole->Write("%s\r\n", sLogonCommHandler.accountResult.c_str());

    return true;
}

bool handleSendMailGold(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool /*isWebClient*/)
{
    if (argumentCount > 0 && consoleInput.empty())
        return false;

    std::vector<std::string> mailVector = AscEmu::Util::Strings::split(consoleInput, "|");

    std::string* mailString = mailVector.data();
    std::string charName(mailString[0].erase(0, 1));
    std::string subject(mailString[1]);
    std::string body(mailString[2]);
    uint32_t gold(std::stoul(mailString[3]));

    std::cout << charName << " check" << "\n";
    std::cout << subject << " check" << "\n";
    std::cout << body << " check" << "\n";
    std::cout << gold << " check" << "\n";

    if (auto result = CharacterDatabase.Query("SELECT guid FROM characters WHERE name = '%s'", charName.c_str()))
    {
        uint64_t guid = result->Fetch()[0].asUint64();
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, guid, guid, subject, body, gold, 0, 0, MAIL_STATIONERY_GM);
    }

    return true;
}
