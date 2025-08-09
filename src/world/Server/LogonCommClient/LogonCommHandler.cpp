/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/LogonCommClient/LogonCommHandler.h"

#include <sstream>

#include "WorldPacket.h"
#include "Server/Master.h"
#include "Config/Config.h"
#include "Cryptography/LogonCommDefines.h"
#include "Cryptography/Sha1.hpp"
#include "Logging/Logger.hpp"
#include "Server/ConfigMgr.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Threading/LegacyThreadPool.h"
#include "Utilities/Strings.hpp"
#include "Threading/LegacyThreading.h"

LogonCommHandler& LogonCommHandler::getInstance()
{
    static LogonCommHandler mInstance;
    return mInstance;
}

void LogonCommHandler::initialize()
{
    idhigh = 1;
    next_request = 1;
    pings = !worldConfig.logonServer.disablePings;
    std::string logon_pass = worldConfig.logonServer.remotePassword;

    // sha1 hash it
    Sha1Hash hash;
    hash.updateData(logon_pass);
    hash.finalize();

    memset(sql_passhash, 0, 20);
    memcpy(sql_passhash, hash.getDigest(), 20);

    server_population = 0;

    _realmType = 0;

    // cleanup
    servers.clear();
    realms.clear();
}

void LogonCommHandler::finalize()
{
    servers.clear();
    realms.clear();
}

class LogonCommWatcherThread : public ThreadBase
{
    bool running;

    Arcemu::Threading::ConditionVariable cond;

public:
    LogonCommWatcherThread()
    {
        running = true;
    }

    ~LogonCommWatcherThread()
    {}

    void onShutdown()
    {
        running = false;
        cond.Signal();
    }

    bool runThread()
    {
        sLogonCommHandler.connectToLogonServer();
        while (running)
        {
            sLogonCommHandler.updateLogonServerConnection();
            cond.Wait(5000);
        }
        return true;
    }
};

void LogonCommHandler::startLogonCommHandler()
{
    loadRealmsConfiguration();
    loadAccountPermissions();

    ThreadPool.ExecuteTask(new LogonCommWatcherThread());
}

void LogonCommHandler::loadAccountPermissions()
{
    sLogger.info("LogonCommClient : Loading account permissions...");

    auto result = CharacterDatabase.Query("SELECT id, permissions FROM account_permissions");
    if (result != nullptr)
    {
        do
        {
            uint32_t id = result->Fetch()[0].asUint32();
            std::string dbPermission = result->Fetch()[1].asCString();
            if (AscEmu::Util::Strings::isEqual(dbPermission, "az"))
                dbPermission = "12stulfbvrjiqdmwcogenaz";

            accountPermissionsStore.insert(make_pair(id, dbPermission));

        } while (result->NextRow());
    }
}

void LogonCommHandler::connectToLogonServer()
{
    sLogger.info("LogonCommClient : Attempting to connect to logon server...");
    for (auto itr = servers.begin(); itr != servers.end(); ++itr)
    {
        tryLogonServerConnection((*itr).get());
    }
}

void LogonCommHandler::tryLogonServerConnection(LogonServerStructure* server)
{
    if (sMaster.m_ShutdownEvent == true && sMaster.m_ShutdownTimer <= 120000)
    {
        return;
    }

    server->nextRetryTime = (uint32_t)UNIXTIME + 10;
    server->isRegistered = false;

    LogonCommClientSocket* logonCommSocket = createReturnLogonServerConnection(server->address, server->port);
    logons[server] = logonCommSocket;

    if (logonCommSocket == nullptr)
    {
        sLogger.failure("Connection failed. Will try again in 10 seconds.");
        return;
    }

    sLogger.info("LogonCommClient : Authenticating...");
    uint32_t nextRetryTime1 = (uint32_t)UNIXTIME + 10;
    logonCommSocket->SendChallenge();

    while (!logonCommSocket->authenticated)
    {
        if ((uint32_t)UNIXTIME >= nextRetryTime1)
        {
            sLogger.failure("Authentication timed out.");
            logonCommSocket->Disconnect();
            logons[server] = nullptr;
            return;
        }

        Arcemu::Sleep(50);
    }

    if (logonCommSocket->authenticated != 1)
    {
        sLogger.failure("Authentication failed.");
        logons[server] = nullptr;
        logonCommSocket->Disconnect();
        return;
    }

    sLogger.info("Authentication successful.");
    sLogger.info("LogonCommClient : Logonserver was connected on [{}:{}].", server->address, server->port);

    // Send the initial ping
    logonCommSocket->SendPing();

    sLogger.info("LogonCommClient : Registering Realms...");
    logonCommSocket->_id = server->id;

    addRealmToRealmlist(logonCommSocket);

    uint32_t nextRetryTime = (uint32_t)UNIXTIME + 10;

    // Wait for register ACK
    while (server->isRegistered == false)
    {
        // Don't wait more than.. like 10 seconds for a registration
        if ((uint32_t)UNIXTIME >= nextRetryTime)
        {
            sLogger.failure("Realm registration timed out.");
            logons[server] = nullptr;
            logonCommSocket->Disconnect();
            break;
        }
        Arcemu::Sleep(50);
    }

    if (!server->isRegistered)
    {
        return;
    }

    // Wait for all realms to register
    Arcemu::Sleep(200);

    sLogger.info("LogonCommClient : Logonserver latency is {}ms.", logonCommSocket->latency);
}

LogonCommClientSocket* LogonCommHandler::createReturnLogonServerConnection(std::string Address, uint32_t Port)
{
    return ConnectTCPSocket<LogonCommClientSocket>(Address.c_str(), static_cast<u_short>(Port));
}

void LogonCommHandler::addRealmToRealmlist(LogonCommClientSocket* Socket)
{
    for (const auto& realm : realms)
    {
        WorldPacket data(LRCMSG_REALM_REGISTER_REQUEST, 100);

        // Add realm to the packet
        data << realm->id;
        data << realm->name;
        data << realm->address;
        data << realm->flags;
        data << realm->icon;
        data << realm->timeZone;
        data << float(realm->population);
        data << uint8_t(realm->lock);
        data << uint32_t(realm->gameBuild);
        Socket->SendPacket(&data, false);
    }
}

void LogonCommHandler::setAccountPermission(uint32_t acct, std::string perm)
{
    AccountPermissionMap::iterator itr = accountPermissionsStore.find(acct);
    if (itr != accountPermissionsStore.end())
        accountPermissionsStore.erase(acct);

    sLogger.info("LogonCommClient : Permission set to {} for account {}", perm, acct);
    accountPermissionsStore.insert(make_pair(acct, perm));
}

void LogonCommHandler::removeAccountPermission(uint32_t acct)
{
    setAccountPermission(acct, "");
}

std::string LogonCommHandler::getPermissionStringForAccountId(uint32_t username)
{
    std::string permission = "";
    AccountPermissionMap::iterator itr = accountPermissionsStore.find(username);
    if (itr != accountPermissionsStore.end())
        permission = itr->second;

    return permission;
}

void LogonCommHandler::addRealmToRealmlistResult(uint32_t ID, uint32_t ServID)
{
    for (auto const &itr : logons)
    {
        if (itr.first->id == ID)
        {
            itr.first->serverId = ServID;
            itr.first->isRegistered = true;
            return;
        }
    }
}

uint32_t LogonCommHandler::getRealmType()
{
    return _realmType;
}

void LogonCommHandler::setRealmType(uint32_t type)
{
    _realmType = type;
}

float LogonCommHandler::getRealmPopulation()
{
    return server_population;
}

void LogonCommHandler::updateLogonServerConnection()
{
    mapLock.acquire();

    uint32_t time = (uint32_t)UNIXTIME;

    for (auto &itr : logons)
    {
        LogonCommClientSocket* logonCommSocket = itr.second;
        if (logonCommSocket != nullptr)
        {
            if (!pings)
            {
                continue;
            }

            if (logonCommSocket->IsDeleted() || !logonCommSocket->IsConnected())
            {
                logonCommSocket->_id = 0;
                itr.second = nullptr;
                continue;
            }

            if (logonCommSocket->last_pong < time && ((time - logonCommSocket->last_pong) > 60))
            {
                // no pong for 60 seconds -> remove the socket
                sLogger.info("Logonserver {} connection dropped due to pong timeout!", itr.first->id);
                logonCommSocket->_id = 0;
                logonCommSocket->Disconnect();
                itr.second = nullptr;
                continue;
            }

            if ((time - logonCommSocket->last_ping) > 15)
            {
                // send a ping packet.
                logonCommSocket->SendPing();
            }
        }
        else
        {
            // check retry time
            if (time >= itr.first->nextRetryTime)
            {
                tryLogonServerConnection(itr.first);
            }
        }
    }

    mapLock.release();
}

void LogonCommHandler::dropLogonServerConnection(uint32_t ID)
{
    mapLock.acquire();

    for (auto &itr : logons)
    {
        if (itr.first->id == ID && itr.second != nullptr)
        {
            sLogger.failure("Logonserver connection {} was dropped. Try to reconnect next loop.", ID);
            itr.second = nullptr;
            break;
        }
    }

    mapLock.release();
}

uint32_t LogonCommHandler::clientConnectionId(std::string AccountName, WorldSocket* Socket)
{
    uint32_t request_id = next_request++;

    sLogger.debug(" Send Request for Account: `{}` (request ID: {}).", AccountName, request_id);

    // Send request packet to server.
    if (logons.empty())
    {
        // No valid logonserver is connected.
        return (uint32_t)-1;
    }

    LogonCommClientSocket* logonCommSocket = getLogonServerSocket();
    if (logonCommSocket == nullptr)
    {
        return (uint32_t)-1;
    }

    pendingLock.acquire();

    WorldPacket data(LRCMSG_ACC_SESSION_REQUEST, 100);
    data << request_id;

    // strip the shitty hash from it
    const char* acct = AccountName.c_str();

    size_t i = 0;
    for (; acct[i] != '#' && acct[i] != '\0'; ++i)
    {
        data.append(&acct[i], 1);
    }

    data.append("\0", 1);
    logonCommSocket->SendPacket(&data, false);

    pending_logons[request_id] = Socket;
    pendingLock.release();

    updateRealmPopulation();
    return request_id;
}

void LogonCommHandler::removeUnauthedClientSocketClose(uint32_t id)
{
    pendingLock.acquire();
    pending_logons.erase(id);
    pendingLock.release();
}

void LogonCommHandler::removeUnauthedClientSocket(uint32_t id)
{
    pending_logons.erase(id);
}

void LogonCommHandler::loadRealmsConfiguration()
{
    auto logonServer = std::make_unique<LogonServerStructure>();
    logonServer->id = idhigh++;
    logonServer->name = worldConfig.logonServer.name;
    logonServer->address = worldConfig.logonServer.address;
    logonServer->port = (uint32_t)worldConfig.logonServer.port;
    servers.insert(std::move(logonServer));

    uint32_t realmcount = (uint32_t)worldConfig.logonServer.realmCount;
    if (realmcount == 0)
    {
        sLogger.failure("No realm definitions found in world.conf!");
    }
    else
    {
        for (uint32_t i = 1; i < realmcount + 1; ++i)
        {
            std::stringstream realmString;
            realmString << "Realm" << i;

            auto realmStructure = std::make_unique<RealmStructure>();
            Config.MainConfig.tryGetInt(realmString.str(), "Id", &realmStructure->id);
            Config.MainConfig.tryGetString(realmString.str(), "Name", &realmStructure->name);
            Config.MainConfig.tryGetString(realmString.str(), "Address", &realmStructure->address);
            Config.MainConfig.tryGetInt(realmString.str(), "TimeZone", &realmStructure->timeZone);
            ///\ todo: not handled in core
            Config.MainConfig.tryGetInt(realmString.str(), "Lock", &realmStructure->lock);

            realmStructure->population = 0.0f;
            realmStructure->flags = 0;
            realmStructure->gameBuild = VERSION_STRING;

            std::string realmType = "Normal";
            Config.MainConfig.tryGetString(realmString.str(), "Icon", &realmType);
            AscEmu::Util::Strings::toLowerCase(realmType);

            // process realm type
            if (realmType == "pvp")
            {
                _realmType = REALMTYPE_PVP;
            }
            else if (realmType == "rp")
            {
                _realmType = REALMTYPE_RP;
            }
            else if (realmType == "rppvp")
            {
                _realmType = REALMTYPE_RPPVP;
            }
            else
            {
                _realmType = REALMTYPE_NORMAL;
            }

            realmStructure->icon = _realmType;
            realms.insert(std::move(realmStructure));
        }
    }
}

void LogonCommHandler::updateAccountCount(uint32_t account_id, uint8_t add)
{
    // Send request packet to server.
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        logonCommSocket->UpdateAccountCount(account_id, add);
    }
}

void LogonCommHandler::testConsoleLogon(std::string & username, std::string & password, uint32_t requestnum)
{
    std::string newuser = username;
    std::string newpass = password;

    AscEmu::Util::Strings::toUpperCase(newuser);
    AscEmu::Util::Strings::toUpperCase(newpass);

    // Send request packet to server.
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_LOGIN_CONSOLE_REQUEST, 100);
        data << requestnum;
        data << newuser;
        data << newpass;

        logonCommSocket->SendPacket(&data, false);
    }
}

// db funcs
void LogonCommHandler::setAccountBanned(const char* account, uint32_t banned, const char* reason)
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 300);
        uint32_t method = Method_Account_Ban;

        data << uint32_t(method);
        data << account;
        data << banned;
        data << reason;
        logonCommSocket->SendPacket(&data, false);
    }
}

void LogonCommHandler::setAccountPermissions(const char* account, const char* flags)
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 50);
        uint32_t method = Method_Account_Set_GM;

        data << uint32_t(method);
        data << account;
        data << flags;
        logonCommSocket->SendPacket(&data, false);
    }
}

void LogonCommHandler::setAccountMute(const char* account, uint32_t muted)
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 50);
        uint32_t method = Method_Account_Set_Mute;

        data << uint32_t(method);
        data << account;
        data << muted;
        logonCommSocket->SendPacket(&data, false);
    }
}

void LogonCommHandler::addIpBan(const char* ip, uint32_t duration, const char* reason)
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 300);
        uint32_t method = Method_IP_Ban;

        data << uint32_t(method);
        data << ip;
        data << duration;
        data << reason;
        logonCommSocket->SendPacket(&data, false);
    }
}

void LogonCommHandler::removeIpBan(const char* ip)
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 50);
        uint32_t method = Method_IP_Unban;

        data << uint32_t(method);
        data << ip;
        logonCommSocket->SendPacket(&data, false);
    }
}

void LogonCommHandler::changeAccountPassword(const char* old_password, const char* new_password, const char* account_name)
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 400);
        uint32_t method = Method_Account_Change_PW;

        data << uint32_t(method);
        data << old_password;
        data << new_password;
        data << account_name;
        logonCommSocket->SendPacket(&data, false);
    }
}

void LogonCommHandler::createAccount(const char* name, const char* password, const char* account_name)
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 300);
        uint32_t method = Method_Account_Create;

        data << uint32_t(method);
        data << name;
        data << password;
        data << account_name;
        logonCommSocket->SendPacket(&data, false);
    }
}

void LogonCommHandler::updateRealmPopulation()
{
    // Get realm player limit, it's better that we get the player limit once and save it! <-done
    // Calc pop: 0 >= low, 1 >= med, 2 >= hig, 3 >= full
    uint32_t limit = worldConfig.server.playerLimit ? worldConfig.server.playerLimit : 1;
    server_population = sWorld.getPlayerCount() * 3.0f / limit;
}

void LogonCommHandler::checkIfAccountExist(const char* account, const char* request_name, const char* additional, uint32_t method)
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ACCOUNT_REQUEST, 50);

        switch (method)
        {
            case 2:
            {
                data << uint32_t(2);        // 2 = get requested account id
                data << account;
                data << request_name;

            } break;
            default:
            {
                data << uint32_t(1);        // 1 = check requested account available
                data << account;
                data << request_name;

                if (additional)           // additional data (gmlevel)
                {
                    data << additional;
                }

            } break;
        }

        logonCommSocket->SendPacket(&data, false);
    }
}

void LogonCommHandler::requestAccountData()
{
    if (LogonCommClientSocket* logonCommSocket = getLogonServerSocket())
    {
        WorldPacket data(LRCMSG_ALL_ACCOUNT_REQUEST, 100);
        logonCommSocket->SendPacket(&data, false);
    }
}

LogonCommClientSocket* LogonCommHandler::getLogonServerSocket()
{
    std::map<LogonServerStructure*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.empty() || itr->second == nullptr)
    {
        return nullptr;
    }
    else
    {
        return itr->second;
    }
}

WorldSocket* LogonCommHandler::getWorldSocketForClientRequestId(uint32_t id)
{
    WorldSocket* sock;
    std::map<uint32_t, WorldSocket*>::iterator itr = pending_logons.find(id);
    sock = (itr == pending_logons.end()) ? nullptr : itr->second;
    return sock;
}
