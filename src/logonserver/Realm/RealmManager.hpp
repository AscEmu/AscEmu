/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Auth/AuthSocket.h"
#include <chrono>

namespace AscEmu::Realm
{
    struct Realm
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

    //class ::AuthSocket;
    //class ::LogonCommServerSocket;

    class RealmManager
    {
    public:
        RealmManager(RealmManager&&) = delete;
        RealmManager(RealmManager const&) = delete;
        RealmManager& operator=(RealmManager&&) = delete;
        RealmManager& operator=(RealmManager const&) = delete;

        static RealmManager& getInstance();

        void initialize(uint32_t checkTime);

        void finalize();

        void loadRealms();

        Realm* getRealmById(uint32_t id) const;

        void setStatusForRealm(uint8_t realm_id, uint8_t status);

        void setLastPing(uint8_t realm_id);

        void checkRealmStatus(bool silent);

        void sendRealms(::AuthSocket* Socket);

        std::mutex& getServerSocketLock() { return serverSocketLock; }
        std::mutex& getRealmLock() { return realmLock; }

        void timeoutSockets();
        void checkServers();

        void setRealmOffline(uint32_t realm_id);
        void setRealmPopulation(uint32_t realm_id, float population);

        void addServerSocket(::LogonCommServerSocket* sock);
        void removeServerSocket(::LogonCommServerSocket* sock);

    private:
        RealmManager() = default;
        ~RealmManager() = default;

        std::vector<std::unique_ptr<Realm>> realms;
        std::set<::LogonCommServerSocket*> serverSockets;
        std::mutex serverSocketLock;
        std::mutex realmLock;

        bool usePings;

        std::unique_ptr<AscEmu::Threading::AEThread> checkThread;
        uint32_t checkTime;
    };
}
#define sRealmManager AscEmu::Realm::RealmManager::getInstance()
