/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"

struct Realms
{
    // received from internal packets
    std::string name;
    std::string address;
    uint32_t flags;
    uint32_t icon;
    uint32_t timeZone;
    float population;
    uint8_t lock;
    uint32_t gameBuild;
    std::unordered_map<uint32_t, uint8_t> _characterMap;

    // database values
    uint32_t id;
    std::string password;
    uint8_t status;

    //internal
    std::chrono::high_resolution_clock::time_point lastPing;
};

class AuthSocket;
class LogonCommServerSocket;

class RealmsMgr : public Singleton<RealmsMgr>
{
    std::set<LogonCommServerSocket*> m_serverSockets;
    Mutex serverSocketLock;
    Mutex realmLock;

    bool usePings;

public:

    RealmsMgr();

    void LoadRealms();
    std::vector<std::shared_ptr<Realms>> _realmStore;

    std::shared_ptr<Realms> getRealmById(uint32_t id) const;

    void setStatusForRealm(uint8_t realm_id, uint32_t status);

    void setLastPing(uint8_t realm_id);

    void checkRealmStatus();

    void sendRealms(AuthSocket* Socket);

    Mutex& getServerSocketLock() { return serverSocketLock; }
    Mutex& getRealmLock() { return realmLock; }

    void timeoutSockets();
    void checkServers();

    void setRealmOffline(uint32_t realm_id);
    void updateRealmPop(uint32_t realm_id, float pop);

    void addServerSocket(LogonCommServerSocket* sock)
    {
        serverSocketLock.Acquire();
        m_serverSockets.insert(sock);
        serverSocketLock.Release();
    }

    void removeServerSocket(LogonCommServerSocket* sock)
    {
        serverSocketLock.Acquire();
        m_serverSockets.erase(sock);
        serverSocketLock.Release();
    }
};

#define sRealmsMgr RealmsMgr::getSingleton()