/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef LOGONCOMMHANDLER_H
#define LOGONCOMMHANDLER_H

#include "CommonTypes.hpp"
#include "Server/WorldSocket.h"
#include "Server/LogonCommClient/LogonCommClient.h"
#include <string>
#include <map>
#include <set>

struct LogonServer
{
    uint32 ID;
    std::string Name;
    std::string Address;
    uint32 Port;
    uint32 ServerID;
    uint32 RetryTime;
    bool Registered;
};

struct Realm
{
    std::string Name;
    std::string Address;
    uint32 flags;
    uint32 Icon;
    uint32 TimeZone;
    float Population;
    uint8 Lock;
    uint32 GameBuild;
};

enum RealmType
{
    REALMTYPE_NORMAL = 0,
    REALMTYPE_PVP    = 3,
    REALMTYPE_RP     = 6,
    REALMTYPE_RPPVP  = 8,
};


class LogonCommHandler : public Singleton<LogonCommHandler>
{
#ifdef WIN32
    typedef std::unordered_map<uint32, std::string> ForcedPermissionMap;
#else
    typedef std::map<std::string, std::string> ForcedPermissionMap;
#endif

    ForcedPermissionMap forced_permissions;
    std::map<LogonServer*, LogonCommClientSocket*> logons;
    std::map<uint32, WorldSocket*> pending_logons;
    std::set<Realm*> realms;
    std::set<LogonServer*> servers;
    uint32 idhigh;
    uint32 next_request;
    Mutex mapLock;
    Mutex pendingLock;
    bool pings;
    uint32 _realmType;
    uint32 pLimit;
    float server_population;

    public:

        uint8 sql_passhash[20];

        LogonCommHandler();
        ~LogonCommHandler();

        LogonCommClientSocket* ConnectToLogon(std::string Address, uint32 Port);
        void UpdateAccountCount(uint32 account_id, uint8 add);
        void RequestAddition(LogonCommClientSocket* Socket);
        void CheckAllServers();
        void Startup();
        void AddForcedPermission(uint32 acct, std::string perm);
        void RemoveForcedPermission(uint32 acct);
        void ConnectionDropped(uint32 ID);
        void AdditionAck(uint32 ID, uint32 ServID);
        void UpdateSockets();
        void Connect(LogonServer* server);
        void ConnectAll();
        //void LogonDatabaseSQLExecute(const char* str, ...);
        //void LogonDatabaseReloadAccounts();
        void RefreshRealmPop();

        void Account_SetBanned(const char* account, uint32 banned, const char* reason);
        void Account_SetGM(const char* account, const char* flags);
        void Account_SetMute(const char* account, uint32 muted);
        void Account_CheckExist(const char* account, const char* request_name, const char* additional, uint32 method = 1);
        void RequestAccountData();
        void IPBan_Add(const char* ip, uint32 duration, const char* reason);
        void IPBan_Remove(const char* ip);
        void AccountChangePassword(const char* old_pw, const char* new_password, const char* account_name);
        void AccountCreate(const char* name, const char* password, const char* account_name);

        void LoadRealmConfiguration();
        void AddServer(std::string Name, std::string Address, uint32 Port);

        inline uint32 GetRealmType() { return _realmType; }
        void SetRealmType(uint32 type) { _realmType = type; }
        float GetServerPopulation() { return server_population; }

        /////////////////////////////
        // Worldsocket stuff
        ///////

        uint32 ClientConnected(std::string AccountName, WorldSocket* Socket);
        void UnauthedSocketClose(uint32 id);
        void RemoveUnauthedSocket(uint32 id);
        WorldSocket* GetSocketByRequest(uint32 id)
        {
            //pendingLock.Acquire();

            WorldSocket* sock;
            std::map<uint32, WorldSocket*>::iterator itr = pending_logons.find(id);
            sock = (itr == pending_logons.end()) ? 0 : itr->second;

            //pendingLock.Release();
            return sock;
        }
        inline Mutex & GetPendingLock() { return pendingLock; }
        const std::string* GetForcedPermissions(uint32 username);

        void TestConsoleLogon(std::string & username, std::string & password, uint32 requestnum);
        std::string accountResult;
};

#define sLogonCommHandler LogonCommHandler::getSingleton()

#endif // LOGONCOMMHANDLER_H
