/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "LogonServerDefines.hpp"


struct Realm
{
    std::string Name;
    std::string Address;
    uint32_t flags;
    uint32_t Icon;
    uint32_t TimeZone;
    float Population;
    uint8_t Lock;
    uint32_t GameBuild;
    std::unordered_map<uint32_t, uint8_t> CharacterMap;
};

class AuthSocket;
class LogonCommServerSocket;

class InfoCore : public Singleton<InfoCore>
{
    std::map<uint32_t, std::shared_ptr<Realm>> m_realms;

    std::set<LogonCommServerSocket*> m_serverSockets;
    Mutex serverSocketLock;
    Mutex realmLock;

    uint32_t realmhigh;
    bool usepings;

    public:

        Mutex& getServerSocketLock() { return serverSocketLock; }
        Mutex& getRealmLock() { return realmLock; }

        InfoCore();

#ifdef AE_TBC
        void writeRealmDataTbc(AuthSocket* socket);
#endif
        // Packets
        void sendRealms(AuthSocket* Socket);

        // Realm management
        uint32_t generateRealmID()
        {
            return ++realmhigh;
        }

        std::shared_ptr<Realm> addRealm(uint32_t realm_id, std::shared_ptr<Realm> rlm);
        std::shared_ptr<Realm> getRealm(uint32_t realm_id);
        uint32_t getRealmIdByName(const std::string& Name);
        void removeRealm(uint32_t realm_id);
        void setRealmOffline(uint32_t realm_id);
        void updateRealmStatus(uint32_t realm_id, uint8_t flags);
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

        void timeoutSockets();
        void checkServers();
};

#define sInfoCore InfoCore::getSingleton()
