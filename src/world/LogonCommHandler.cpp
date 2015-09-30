/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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


initialiseSingleton(LogonCommHandler);

LogonCommHandler::LogonCommHandler()
{
    idhigh = 1;
    next_request = 1;
    pings = !Config.MainConfig.GetBoolDefault("LogonServer", "DisablePings", false);
    std::string logon_pass = Config.MainConfig.GetStringDefault("LogonServer", "RemotePassword", "r3m0t3");

    // sha1 hash it
    Sha1Hash hash;
    hash.UpdateData(logon_pass);
    hash.Finalize();

    memset(sql_passhash, 0, 20);
    memcpy(sql_passhash, hash.GetDigest(), 20);

    // player limit
    pLimit = Config.MainConfig.GetIntDefault("Server", "PlayerLimit", 500);
    if (pLimit == 0) pLimit = 1;
    server_population = 0;

    _realmType = 0;

    // cleanup
    servers.clear();
    realms.clear();
}

LogonCommHandler::~LogonCommHandler()
{
    for (std::set<LogonServer*>::iterator i = servers.begin(); i != servers.end(); ++i)
        delete(*i);

    for (std::set<Realm*>::iterator i = realms.begin(); i != realms.end(); ++i)
        delete(*i);
}

LogonCommClientSocket* LogonCommHandler::ConnectToLogon(std::string Address, uint32 Port)
{
    LogonCommClientSocket* conn = ConnectTCPSocket<LogonCommClientSocket>(Address.c_str(), static_cast<u_short>(Port));
    return conn;
}

void LogonCommHandler::RequestAddition(LogonCommClientSocket* Socket)
{
    std::set<Realm*>::iterator itr = realms.begin();

    for (; itr != realms.end(); ++itr)
    {
        WorldPacket data(LRCMSG_REALM_REGISTER_REQUEST, 100);

        // Add realm to the packet
        Realm* realm = *itr;
        data << realm->Name;
        data << realm->Address;
        data << realm->flags;
        data << realm->Icon;
        data << realm->TimeZone;
        data << float(realm->Population);
        data << uint8(realm->Lock);
        Socket->SendPacket(&data, false);
    }
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

    void OnShutdown()
    {
        running = false;
        cond.Signal();
    }

    bool run()
    {
        sLogonCommHandler.ConnectAll();
        while (running)
        {
            sLogonCommHandler.UpdateSockets();
            cond.Wait(5000);
        }
        return true;
    }
};

void LogonCommHandler::Startup()
{
    // Try to connect to all logons.
    LoadRealmConfiguration();

    Log.Notice("LogonCommClient", "Loading forced permission strings...");
    QueryResult* result = CharacterDatabase.Query("SELECT login, permissions FROM account_forced_permissions");
    if (result != NULL)
    {
        do
        {
            std::string acct = result->Fetch()[0].GetString();
            std::string perm = result->Fetch()[1].GetString();

            arcemu_TOUPPER(acct);
            forced_permissions.insert(make_pair(acct, perm));

        }
        while (result->NextRow());
        delete result;
    }
    ThreadPool.ExecuteTask(new LogonCommWatcherThread());
}

void LogonCommHandler::AddForcedPermission(std::string acct, std::string perm)
{
    auto account_name = acct.c_str();
    auto permission_string = perm.c_str();
    arcemu_TOUPPER(acct);

    ForcedPermissionMap::iterator itr = forced_permissions.find(acct);
    if (itr != forced_permissions.end())
    {
        Log.Notice("LogonCommClient", "Permission for %s already available!", account_name);
        forced_permissions.erase(acct);
    }

    Log.Notice("LogonCommClient", "Permission set to %s for account %s", permission_string, account_name);
    forced_permissions.insert(make_pair(acct, perm));

}


void LogonCommHandler::ConnectAll()
{
    Log.Success("LogonCommClient", "Attempting to connect to logon server...");
    for (std::set<LogonServer*>::iterator itr = servers.begin(); itr != servers.end(); ++itr)
        Connect(*itr);
}

const std::string* LogonCommHandler::GetForcedPermissions(std::string & username)
{
    ForcedPermissionMap::iterator itr = forced_permissions.find(username);
    if (itr == forced_permissions.end())
        return NULL;

    return &itr->second;
}

void LogonCommHandler::Connect(LogonServer* server)
{
    if (sMaster.m_ShutdownEvent == true && sMaster.m_ShutdownTimer <= 120000) // 2minutes
        return;

    server->RetryTime = (uint32)UNIXTIME + 10;
    server->Registered = false;
    LogonCommClientSocket* conn = ConnectToLogon(server->Address, server->Port);
    logons[server] = conn;
    if (conn == 0)
    {
        Log.Error("LogonCommClient", "Connection failed. Will try again in 10 seconds.");
        return;
    }
    Log.Success("LogonCommClient", "Authenticating...");
    uint32 tt = (uint32)UNIXTIME + 10;
    conn->SendChallenge();
    while (!conn->authenticated)
    {
        if ((uint32)UNIXTIME >= tt)
        {
            Log.Error("LogonCommClient", "Authentication timed out.");
            conn->Disconnect();
            logons[server] = NULL;
            return;
        }

        Arcemu::Sleep(50);
    }

    if (conn->authenticated != 1)
    {
        Log.Error("LogonCommClient", "Authentication failed.");
        logons[server] = 0;
        conn->Disconnect();
        return;
    }

    Log.Success("LogonCommClient", "Authentication OK.");
    Log.Notice("LogonCommClient", "Logonserver was connected on [%s:%u].", server->Address.c_str(), server->Port);

    // Send the initial ping
    conn->SendPing();

    Log.Notice("LogonCommClient", "Registering Realms...");
    conn->_id = server->ID;

    RequestAddition(conn);

    uint32 st = (uint32)UNIXTIME + 10;

    // Wait for register ACK
    while (server->Registered == false)
    {
        // Don't wait more than.. like 10 seconds for a registration
        if ((uint32)UNIXTIME >= st)
        {
            Log.Error("LogonCommClient", "Realm registration timed out.");
            logons[server] = 0;
            conn->Disconnect();
            break;
        }
        Arcemu::Sleep(50);
    }

    if (!server->Registered)
        return;

    // Wait for all realms to register
    Arcemu::Sleep(200);

    Log.Success("LogonCommClient", "Logonserver latency is %ums.", conn->latency);
}

void LogonCommHandler::AdditionAck(uint32 ID, uint32 ServID)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    for (; itr != logons.end(); ++itr)
    {
        if (itr->first->ID == ID)
        {
            itr->first->ServerID = ServID;
            itr->first->Registered = true;
            return;
        }
    }
}

void LogonCommHandler::UpdateSockets()
{
    mapLock.Acquire();

    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    LogonCommClientSocket* cs;
    uint32 t = (uint32)UNIXTIME;
    for (; itr != logons.end(); ++itr)
    {
        cs = itr->second;
        if (cs != 0)
        {
            if (!pings) continue;

            if (cs->IsDeleted() || !cs->IsConnected())
            {
                cs->_id = 0;
                itr->second = 0;
                continue;
            }

            if (cs->last_pong < t && ((t - cs->last_pong) > 60))
            {
                // no pong for 60 seconds -> remove the socket
                LOG_DETAIL(" >> realm id %u connection dropped due to pong timeout.", (unsigned int)itr->first->ID);
                cs->_id = 0;
                cs->Disconnect();
                itr->second = 0;
                continue;
            }

            if ((t - cs->last_ping) > 15)
            {
                // send a ping packet.
                cs->SendPing();
            }
        }
        else
        {
            // check retry time
            if (t >= itr->first->RetryTime)
            {
                Connect(itr->first);
            }
        }
    }
    mapLock.Release();
}

void LogonCommHandler::ConnectionDropped(uint32 ID)
{
    mapLock.Acquire();
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    for (; itr != logons.end(); ++itr)
    {
        if (itr->first->ID == ID && itr->second != 0)
        {
            LOG_ERROR(" >> realm id %u connection was dropped unexpectedly. reconnecting next loop.", ID);
            itr->second = 0;
            break;
        }
    }
    mapLock.Release();
}

uint32 LogonCommHandler::ClientConnected(std::string AccountName, WorldSocket* Socket)
{
    uint32 request_id = next_request++;
    size_t i = 0;
    const char* acct = AccountName.c_str();
    LOG_DEBUG(" >> sending request for account information: `%s` (request %u).", AccountName.c_str(), request_id);

    // Send request packet to server.
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0)
    {
        // No valid logonserver is connected.
        return (uint32)-1;
    }

    LogonCommClientSocket* s = itr->second;
    if (s == NULL)
        return (uint32)-1;

    pendingLock.Acquire();

    WorldPacket data(LRCMSG_ACC_SESSION_REQUEST, 100);
    data << request_id;

    // strip the shitty hash from it
    for (; acct[i] != '#' && acct[i] != '\0'; ++i)
        data.append(&acct[i], 1);

    data.append("\0", 1);
    s->SendPacket(&data, false);

    pending_logons[request_id] = Socket;
    pendingLock.Release();

    RefreshRealmPop();
    return request_id;
}

void LogonCommHandler::UnauthedSocketClose(uint32 id)
{
    pendingLock.Acquire();
    pending_logons.erase(id);
    pendingLock.Release();
}

void LogonCommHandler::RemoveUnauthedSocket(uint32 id)
{
    pending_logons.erase(id);
}

void LogonCommHandler::LoadRealmConfiguration()
{
    LogonServer* ls = new LogonServer;
    ls->ID = idhigh++;
    ls->Name = Config.RealmConfig.GetStringDefault("LogonServer", "Name", "UnkLogon");
    ls->Address = Config.RealmConfig.GetStringDefault("LogonServer", "Address", "127.0.0.1");
    ls->Port = Config.RealmConfig.GetIntDefault("LogonServer", "Port", 8093);
    servers.insert(ls);

    uint32 realmcount = Config.RealmConfig.GetIntDefault("LogonServer", "RealmCount", 1);
    if (realmcount == 0)
    {
        LOG_ERROR("   >> no realms found. this server will not be online anywhere!");
    }
    else
    {
        for (uint32 i = 1; i < realmcount + 1; ++i)
        {
            Realm* realm = new Realm;
            realm->Name = Config.RealmConfig.GetStringVA("Name", "SomeRealm", "Realm%u", i);
            realm->Address = Config.RealmConfig.GetStringVA("Address", "127.0.0.1:8129", "Realm%u", i);
            realm->flags = 0;
            realm->TimeZone = Config.RealmConfig.GetIntVA("TimeZone", 1, "Realm%u", i);
            realm->Population = Config.RealmConfig.GetFloatVA("Population", 0, "Realm%u", i);
            realm->Lock = static_cast<uint8>(Config.RealmConfig.GetIntVA("Lock", 0, "Realm%u", i));
            std::string rt = Config.RealmConfig.GetStringVA("Icon", "Normal", "Realm%u", i);
            uint32 type;

            // process realm type
            if (stricmp(rt.c_str(), "pvp") == 0)
                type = REALMTYPE_PVP;
            else if (stricmp(rt.c_str(), "rp") == 0)
                type = REALMTYPE_RP;
            else if (stricmp(rt.c_str(), "rppvp") == 0)
                type = REALMTYPE_RPPVP;
            else
                type = REALMTYPE_NORMAL;

            _realmType = type;

            realm->Icon = type;
            realms.insert(realm);
        }
    }
}

void LogonCommHandler::UpdateAccountCount(uint32 account_id, uint8 add)
{
    // Send request packet to server.
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }
    itr->second->UpdateAccountCount(account_id, add);
}

void LogonCommHandler::TestConsoleLogon(std::string & username, std::string & password, uint32 requestnum)
{
    std::string newuser = username;
    std::string newpass = password;
    std::string srpstr;

    arcemu_TOUPPER(newuser);
    arcemu_TOUPPER(newpass);

    srpstr = newuser + ":" + newpass;

    // Send request packet to server.
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    Sha1Hash hash;
    hash.UpdateData(srpstr);
    hash.Finalize();

    WorldPacket data(LRCMSG_LOGIN_CONSOLE_REQUEST, 100);
    data << requestnum;
    data << newuser;
    data.append(hash.GetDigest(), 20);

    itr->second->SendPacket(&data, false);
}

// db funcs
void LogonCommHandler::Account_SetBanned(const char* account, uint32 banned, const char* reason)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 300);
    uint32 method = Method_Account_Ban;

    data << uint32(method);
    data << account;
    data << banned;
    data << reason;
    itr->second->SendPacket(&data, false);
}

void LogonCommHandler::Account_SetGM(const char* account, const char* flags)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 50);
    uint32 method = Method_Account_Set_GM;

    data << uint32(method);
    data << account;
    data << flags;
    itr->second->SendPacket(&data, false);
}

void LogonCommHandler::Account_SetMute(const char* account, uint32 muted)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 50);
    uint32 method = Method_Account_Set_Mute;

    data << uint32(method);
    data << account;
    data << muted;
    itr->second->SendPacket(&data, false);
}

void LogonCommHandler::IPBan_Add(const char* ip, uint32 duration, const char* reason)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 300);
    uint32 method = Method_IP_Ban;

    data << uint32(method);
    data << ip;
    data << duration;
    data << reason;
    itr->second->SendPacket(&data, false);
}

void LogonCommHandler::IPBan_Remove(const char* ip)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 50);
    uint32 method = Method_IP_Unban;

    data << uint32(method);
    data << ip;
    itr->second->SendPacket(&data, false);
}

void LogonCommHandler::AccountChangePassword(const char* old_password, const char* new_password, const char* account_name)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 400);
    uint32 method = Method_Account_Change_PW;

    data << uint32(method);
    data << old_password;
    data << new_password;
    data << account_name;
    itr->second->SendPacket(&data, false);
}

void LogonCommHandler::AccountCreate(const char* name, const char* password, const char* account_name)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    WorldPacket data(LRCMSG_ACCOUNT_DB_MODIFY_REQUEST, 300);
    uint32 method = Method_Account_Create;

    data << uint32(method);
    data << name;
    data << password;
    data << account_name;
    itr->second->SendPacket(&data, false);
}

void LogonCommHandler::RefreshRealmPop()
{
    // Get realm player limit, it's better that we get the player limit once and save it! <-done
    // Calc pop: 0 >= low, 1 >= med, 2 >= hig, 3 >= full
    server_population = sWorld.getPlayerCount() * 3.0f / pLimit;
}

void LogonCommHandler::Account_CheckExist(const char* account, const char* request_name, const char* additional)
{
    std::map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
    if (logons.size() == 0 || itr->second == 0)
    {
        return;         // No valid logonserver is connected.
    }

    WorldPacket data(LRCMSG_ACCOUNT_REQUEST, 50);
    data << uint32(1);        // 1 = Account available
    data << account;
    data << request_name;

    if (additional)           // additional data (gmlevel)
        data << additional;

    itr->second->SendPacket(&data, false);
}
