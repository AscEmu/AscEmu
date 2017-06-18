/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include <git_version.h>
#include "ConsoleCommands.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/Console/BaseConsole.h"
#include "Server/MainServerDefines.h"
#include "Server/Master.h"
#include "crc32.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "../../../scripts/Common/Base.h"

bool HandleTimeDateCommand(BaseConsole* console, int argc, const char* argv[])
{
    std::string current_time = Util::GetCurrentDateTimeString();

    std::stringstream ss;
    ss << "Date and time according to localtime() (american style): " << current_time << std::endl;

    console->Write(ss.str().c_str());
    return true;
}

bool HandleInfoCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    uint32 clientsNum = (uint32)sWorld.getSessionCount();
    int gm = 0;
    int count = 0;
    int avg = 0;
    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession())
        {
            count++;
            avg += itr->second->GetSession()->GetLatency();
            if (itr->second->GetSession()->GetPermissionCount())
                gm++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    pConsole->Write("======================================================================\r\n");
    pConsole->Write("Server Information: \r\n");
    pConsole->Write("======================================================================\r\n");
    pConsole->Write("Server Revision: AscEmu %s/%s-%s-%s (www.ascemu.org)\r\n", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
    pConsole->Write("Server Uptime: %s\r\n", sWorld.getWorldUptimeString().c_str());
    pConsole->Write("Current Players: %d (%d GMs, %d queued)\r\n", clientsNum, gm, 0);
    pConsole->Write("Active Thread Count: %u\r\n", ThreadPool.GetActiveThreadCount());
    pConsole->Write("Free Thread Count: %u\r\n", ThreadPool.GetFreeThreadCount());
    pConsole->Write("Average Latency: %.3fms\r\n", count ? ((float)((float)avg / (float)count)) : 0.0f);
    pConsole->Write("CPU Usage: %3.2f %%\r\n", sWorld.getCPUUsage());
    pConsole->Write("RAM Usage: %4.2f MB\r\n", sWorld.getRAMUsage());
    pConsole->Write("SQL Query Cache Size (World): %u queries delayed\r\n", WorldDatabase.GetQueueSize());
    pConsole->Write("SQL Query Cache Size (Character): %u queries delayed\r\n", CharacterDatabase.GetQueueSize());
    sSocketMgr.ShowStatus();

    return true;
}

bool HandleGMsCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    WorldPacket data;
    //bool first = true;

    pConsole->Write("There are the following GM's online on this server: \r\n");
    pConsole->Write("======================================================\r\n");
    pConsole->Write("| %21s | %15s | % 03s  |\r\n", "Name", "Permissions", "Latency");
    pConsole->Write("======================================================\r\n");

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession()->GetPermissionCount())
        {
            pConsole->Write("| %21s | %15s | %03u ms |\r\n", itr->second->GetName(), itr->second->GetSession()->GetPermissions(), itr->second->GetSession()->GetLatency());
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    pConsole->Write("======================================================\r\n\r\n");
    return true;
}


bool HandleOnlinePlayersCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    WorldPacket data;
    //bool first = true;

    pConsole->Write("There following players online on this server: \r\n");
    pConsole->Write("======================================================\r\n");
    pConsole->Write("| %21s | %15s | % 03s  |\r\n", "Name", "Level", "Latency");
    pConsole->Write("======================================================\r\n");

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        pConsole->Write("| %21s | %15u | %03u ms |\r\n", itr->second->GetName(), itr->second->GetSession()->GetPlayer()->getLevel(), itr->second->GetSession()->GetLatency());
    }
    objmgr._playerslock.ReleaseReadLock();

    pConsole->Write("======================================================\r\n\r\n");
    return true;
}

void ConcatArgs(std::string & outstr, int argc, int startoffset, const char* argv[])
{
    for (int i = startoffset + 1; i < argc; ++i)
    {
        outstr += argv[i];
        if ((i + 1) != (argc))
            outstr += " ";
    }
}
bool HandleAnnounceCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    std::string outstr;
    if (argc < 2)
        return false;

    ConcatArgs(outstr, argc, 0, argv);

    std::stringstream worldAnnounce;
    worldAnnounce << MSG_COLOR_LIGHTBLUE << "Console: |r" << outstr;

    sWorld.sendMessageToAll(worldAnnounce.str());
    pConsole->Write("Message sent.\r\n");
    return true;
}

bool HandleWAnnounceCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    char pAnnounce[1024];
    std::string outstr;

    if (argc < 2)
        return false;

    ConcatArgs(outstr, argc, 0, argv);

    std::stringstream areaMessage;
    areaMessage << MSG_COLOR_LIGHTBLUE << "Console: |r" << outstr;

    sWorld.sendAreaTriggerMessage(areaMessage.str());
    pConsole->Write("Message sent.\r\n");
    return true;
}
bool HandleWhisperCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    char pAnnounce[1024];
    Player* pPlayer;
    std::string outstr;

    if (argc < 3)
        return false;

    pPlayer = objmgr.GetPlayer(argv[1]);

    if (pPlayer == NULL)
    {
        pConsole->Write("Could not find player, %s.\r\n", argv[1]);
        return true;
    }

    ConcatArgs(outstr, argc, 1, argv);
    snprintf(pAnnounce, 1024, "%sConsole whisper: |r%s", MSG_COLOR_MAGENTA, outstr.c_str());

    pPlayer->BroadcastMessage(pAnnounce);
    pConsole->Write("Message sent to %s.\r\n", pPlayer->GetName());
    return true;
}

bool HandleKickCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    if (argc < 3)
        return false;

    Player* pPlayer;
    std::string outstr;

    pPlayer = objmgr.GetPlayer(argv[1]);
    if (pPlayer == NULL)
    {
        pConsole->Write("Could not find player, %s.\r\n", argv[1]);
        return true;
    }

    ConcatArgs(outstr, argc, 1, argv);

    std::stringstream worldAnnounce;
    worldAnnounce << MSG_COLOR_LIGHTBLUE << "Console:|r " << pPlayer->GetName() << " was removed from the server. Reason: " << outstr;

    sWorld.sendMessageToAll(worldAnnounce.str());

    pPlayer->BroadcastMessage("You are now being removed by the game by an administrator via the console. Reason: %s", outstr.c_str());
    pPlayer->Kick(5000);
    pConsole->Write("Kicked player %s.\r\n", pPlayer->GetName());
    return true;
}

bool HandleShutDownCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    uint32 delay = 5;
    if (argc >= 2)
    {
        if (stricmp(argv[1], "fast") == 0)
        {
            PlayerStorageMap::const_iterator itr;
            objmgr._playerslock.AcquireReadLock();
            for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
            {
                if (itr->second->GetSession())
                    itr->second->SaveToDB(false);
            }
            objmgr._playerslock.ReleaseReadLock();

            exit(0);
        }
        else
        {
            delay = atoi(argv[1]);
        }
    }

    sMaster.m_ShutdownTimer = delay * 1000;
    sMaster.m_ShutdownEvent = true;
    pConsole->Write("Shutdown has initiated.\r\n");
    return true;
}

bool HandleCancelCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    pConsole->Write("Shutdown has been canceled.\r\n");
    sMaster.m_ShutdownTimer = 5000;
    sMaster.m_ShutdownEvent = false;
    return true;
}

bool HandleBanAccountCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    if (argc < 3)
        return false;

    uint32_t timeperiod = Util::GetTimePeriodFromString(argv[2]);
    if (timeperiod == 0)
        return false;

    uint32 banned = (uint32)UNIXTIME + timeperiod;

    char emptystring = 0;
    char* pReason;
    if (argc == 4)
        pReason = (char*)argv[3];
    else
        pReason = &emptystring;

    /// apply instantly in db
    sLogonCommHandler.Account_SetBanned(argv[1], banned, pReason);

    pConsole->Write("Account '%s' has been banned until %s. The change will be effective immediately.\r\n", argv[1], 
                    Util::GetDateTimeStringFromTimeStamp(timeperiod + (uint32)UNIXTIME).c_str());

    return true;
}

bool HandleUnbanAccountCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    if (argc < 2)
        return false;

    sLogonCommHandler.Account_SetBanned(argv[1], 0, "");

    pConsole->Write("Account '%s' has been unbanned.\r\n", argv[1]);
    return true;
}

bool HandleMOTDCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    if (argc < 2)
    {
        pConsole->Write("The current message of the day is: '%s'.\r\n", worldConfig.getMessageOfTheDay().c_str());
    }
    else
    {
        char set_motd[1024];
        std::string outstr;
        ConcatArgs(outstr, argc, 0, argv);
        snprintf(set_motd, 1024, "%s", outstr.c_str());

        worldConfig.setMessageOfTheDay(set_motd);
        pConsole->Write("The message of the day has been set to: '%s'.\r\n", worldConfig.getMessageOfTheDay().c_str());
    }
    return true;
}

bool HandlePlayerInfoCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    if (argc < 2)
        return false;

    Player* plr = objmgr.GetPlayer(argv[1]);
    if (plr == NULL)
    {
        pConsole->Write("Player not found.\r\n");
        return true;
    }

    pConsole->Write("Player: %s\r\n", plr->GetName());
    pConsole->Write("Race: %s\r\n", plr->myRace->name[0]);
    pConsole->Write("Class: %s\r\n", plr->myClass->name[0]);
    pConsole->Write("IP: %s\r\n", plr->GetSession()->GetSocket() ? plr->GetSession()->GetSocket()->GetRemoteIP().c_str() : "disconnected");
    pConsole->Write("Level: %u\r\n", plr->getLevel());
    pConsole->Write("Account: %s\r\n", plr->GetSession()->GetAccountNameS());
    return true;
}

void TestConsoleLogin(std::string & username, std::string & password, uint32 requestno)
{
    sLogonCommHandler.TestConsoleLogon(username, password, requestno);
}

bool HandleRevivePlayer(BaseConsole* pConsole, int argc, const char* argv[])
{
    if (!argc)
        return false;

    Player* plr = objmgr.GetPlayer(argv[1], false);
    if (!plr)
    {
        pConsole->Write("Could not find player %s.\r\n", argv[1]);
        return true;
    }

    if (plr->IsDead())
    {
        plr->RemoteRevive();
        pConsole->Write("Revived player %s.\r\n", argv[1]);
    }
    else
    {
        pConsole->Write("Player %s is not dead.\r\n", argv[1]);
    }
    return true;
}


bool HandleRehashCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    pConsole->Write("Config file re-parsed.");
    sWorld.loadWorldConfigValues(true);
    return true;
}

bool HandleNameHashCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    if (!argc)
        return false;

    std::string spname;
    ConcatArgs(spname, argc, 0, argv);
    pConsole->Write("Name Hash for %s is 0x%X", spname.c_str(), crc32((const unsigned char*)spname.c_str(), (unsigned int)spname.length()));
    sWorld.loadWorldConfigValues(true);
    return true;
}

bool HandleClearConsoleCommand(BaseConsole* pConsole, int argc, const char* argv[])
{
    system("cls");
    pConsole->Write("Out of the ashes, Chuck Norris appears! With a roundhouse kick, your console shall now be cleaned!");
    return true;
}

bool HandleScriptEngineReloadCommand(BaseConsole*, int, const char*[])
{
    sScriptMgr.ReloadScriptEngines();
    return true;
}
