/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Server/WorldSocket.h"
#include "Server/LogonCommClient/LogonCommClient.h"

#include <string>
#include <map>
#include <set>

struct LogonServerStructure
{
    uint32_t id;
    std::string name;
    std::string address;
    uint32_t port;
    uint32_t serverId;
    uint32_t nextRetryTime;
    bool isRegistered;
};

struct RealmStructure
{
    std::string name;
    std::string address;
    uint32_t flags;
    uint32_t icon;
    uint32_t timeZone;
    float population;
    uint8_t lock;
    uint32_t gameBuild;
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
    typedef std::unordered_map<uint32_t, std::string> AccountPermissionMap;
    AccountPermissionMap accountPermissionsStore;

    typedef std::map<LogonServerStructure*, LogonCommClientSocket*> LogonServerConnection;
    LogonServerConnection logons;

    typedef std::map<uint32_t, WorldSocket*> ActiveWorldSocketsMap;
    ActiveWorldSocketsMap pending_logons;

    typedef std::set<RealmStructure*> RealmsSet;
    RealmsSet realms;

    typedef std::set<LogonServerStructure*> LogonServerSet;
    LogonServerSet servers;

    uint32_t idhigh;
    uint32_t next_request;

    Mutex mapLock;
    Mutex pendingLock;

    bool pings;

    uint32_t _realmType;

    float server_population;

    public:

        uint8_t sql_passhash[20];

        LogonCommHandler();
        ~LogonCommHandler();

        void startLogonCommHandler();
        void loadRealmsConfiguration();
        void loadAccountPermissions();

        // LogonCommWatcherThread
        void connectToLogonServer();
        void updateRealmPopulation();

        void updateLogonServerConnection();
        void tryLogonServerConnection(LogonServerStructure* server);
        LogonCommClientSocket* createReturnLogonServerConnection(std::string Address, uint32_t Port);

        void addRealmToRealmlist(LogonCommClientSocket* Socket);
        void addRealmToRealmlistResult(uint32_t ID, uint32_t ServID);

        void setRealmType(uint32_t type);
        uint32_t getRealmType();

        float getRealmPopulation();

        void updateAccountCount(uint32_t account_id, uint8_t add);

        void dropLogonServerConnection(uint32_t ID);

        void checkIfAccountExist(const char* account, const char* request_name, const char* additional, uint32_t method = 1);
        void requestAccountData();
        
        LogonCommClientSocket* getLogonServerSocket();
        
        void createAccount(const char* name, const char* password, const char* account_name);
        void setAccountBanned(const char* account, uint32_t banned, const char* reason);
        void setAccountPermissions(const char* account, const char* flags);
        void setAccountMute(const char* account, uint32_t muted);
        void changeAccountPassword(const char* old_pw, const char* new_password, const char* account_name);

        void setAccountPermission(uint32_t acct, std::string perm);
        const std::string* getPermissionStringForAccountId(uint32_t username);
        void removeAccountPermission(uint32_t acct);

        void addIpBan(const char* ip, uint32_t duration, const char* reason);
        void removeIpBan(const char* ip);

        // Worldsocket stuff
        uint32 clientConnectionId(std::string AccountName, WorldSocket* Socket);
        void removeUnauthedClientSocketClose(uint32_t id);
        void removeUnauthedClientSocket(uint32_t id);

        WorldSocket* getWorldSocketForClientRequestId(uint32_t id);

        Mutex & getPendingLock() { return pendingLock; }
 
        void testConsoleLogon(std::string & username, std::string & password, uint32_t requestnum);
        std::string accountResult;
};

#define sLogonCommHandler LogonCommHandler::getSingleton()
