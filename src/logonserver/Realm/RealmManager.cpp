/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Logon.h"
#include "RealmManager.hpp"
#include "Utilities/Util.hpp"
#include <Threading/AEThreadPool.h>
#include "Realm/RealmFlag.hpp"
#include <Logging/Logger.hpp>
#include "Server/Master.hpp"
#include <WorldPacket.h>
#include <LogonCommServer/LogonCommServer.h>
#include "Auth/AuthSocket.h"
#include "Database/Database.h"

namespace AscEmu::Realm
{
    RealmManager& RealmManager::getInstance()
    {
        static RealmManager mInstance;
        return mInstance;
    }

    void RealmManager::initialize(uint32_t _checkTime)
    {
        sLogger.trace("[RealmManager] Initializing...");
        this->checkTime = _checkTime;
        this->checkThread = nullptr;
        this->usePings = !logonConfig.logonServer.disablePings;

        loadRealms();

        this->checkThread = std::make_unique<AscEmu::Threading::AEThread>("CheckRealmStatus", [this](AscEmu::Threading::AEThread& /*thread*/) { this->checkRealmStatus(false); }, std::chrono::seconds(this->checkTime));
    }

    void RealmManager::finalize()
    {
        sLogger.trace("[RealmManager] Finalizing...");

        this->checkThread->killAndJoin();
    }

    void RealmManager::loadRealms()
    {
        const auto result = sLogonSQL->Query("SELECT id, password, status FROM realms");
        if (result != nullptr)
        {
            do
            {
                const auto field = result->Fetch();
                const uint32_t realmCount = result->GetRowCount();
                this->realms.reserve(realmCount);

                auto realm = std::make_unique<Realm>();
                realm->id = field[0].asUint32();
                realm->password = field[1].asCString();
                realm->status = field[2].asUint8();
                realm->lastPing = ::Util::TimeNow();

                this->realms.emplace_back(std::move(realm));
            } while (result->NextRow());
        }
        sLogger.info("[RealmManager] Loaded {} realms.", static_cast<uint32_t>(this->realms.size()));
    }

    Realm* RealmManager::getRealmById(uint32_t id) const
    {
        for (const auto& realm : this->realms)
        {
            if (realm->id == id)
                return realm.get();
        }

        return nullptr;
    }

    void RealmManager::setStatusForRealm(uint8_t realm_id, uint8_t status)
    {
        if (this->realms.empty())
        {
            auto realm = std::make_unique<Realm>();
            realm->id = realm_id;
            realm->status = status;
            realm->lastPing = ::Util::TimeNow();

            this->realms.push_back(std::move(realm));

            sLogonSQL->Query("REPLACE INTO realms(id, status, status_change_time) VALUES(%u, %u, NOW())", status, uint32_t(realm_id));
        }
        else
        {
            for (auto& realm : this->realms)
            {
                if (realm->id == realm_id)
                    realm->status = status;
            }

            sLogonSQL->Query("UPDATE realms SET status = %u WHERE id = %u", status, uint32_t(realm_id));
        }
    }

    void RealmManager::setLastPing(uint8_t realm_id)
    {
        for (auto& realm : this->realms)
        {
            if (realm->id == realm_id)
                realm->lastPing = ::Util::TimeNow();
        }
    }

    ///\todo Function only used with silent=false
    void RealmManager::checkRealmStatus(bool silent)
    {
        if (!silent)
            sLogger.info("[RealmManager] Checking Realm status...");

        for (auto& realm : this->realms)
        {
            // if there was no ping in the last 2 minutes (in miliseconds) we set the status to the realm to offline.
            if (::Util::GetTimeDifferenceToNow(realm->lastPing) > 2 * 60 * 1000 && realm->status != 0)
            {
                realm->status = 0;
                sLogger.info("Realm {} status gets set to 0 (offline) since there was no ping the last 2 minutes ({}).", uint32_t(realm->id), ::Util::GetTimeDifferenceToNow(realm->lastPing));
                sLogonSQL->Query("UPDATE realms SET status = 0 WHERE id = %u", uint32_t(realm->id));
            }
        }
    }

    void RealmManager::sendRealms(AuthSocket* authSocket)
    {
        sLogger.trace("[RealmManager] Sending realms to a socket.");
        realmLock.lock();

        ByteBuffer data(this->realms.size() * 150 + 20);
        data << uint8_t(0x10);
        data << uint16_t(0);
        data << uint32_t(0);

        if (authSocket->GetChallenge()->build == 5875)
            data << uint8_t(this->realms.size());
        else
            data << uint16_t(this->realms.size());

        std::unordered_map<uint32_t, uint8_t>::iterator it;
        for (const auto& realm : this->realms)
        {
            if (realm->gameBuild == authSocket->GetChallenge()->build)
            {
                if (realm->gameBuild == 5875)
                {
                    data << uint32_t(realm->icon);
                    data << uint8_t(realm->flags);

                    data << realm->name;
                    data << realm->address;
                    data << float(realm->population);

                    it = realm->_characterMap.find(authSocket->GetAccountID());
                    data << uint8_t((it == realm->_characterMap.end()) ? 0 : it->second);
                    data << uint8_t(realm->timeZone);
                    data << uint8_t(0);
                }
                else
                {
                    data << uint8_t(realm->icon);
                    data << uint8_t(realm->lock);
                    data << uint8_t(realm->flags);

                    data << realm->name;
                    data << realm->address;
                    data << float(realm->population);

                    it = realm->_characterMap.find(authSocket->GetAccountID());
                    data << uint8_t(it == realm->_characterMap.end() ? 0 : it->second);
                    data << uint8_t(realm->timeZone);
                    data << uint8_t(realm->id);

                    if (realm->flags & AscEmu::Realm::RealmFlag::SPECIFIC_BUILD)
                    {
                        data << authSocket->GetChallenge()->version1;
                        data << authSocket->GetChallenge()->version2;
                        data << authSocket->GetChallenge()->version3;
                        data << authSocket->GetChallenge()->build;
                    }
                }
            }
            else // send empty packet for other gameserver which not supports client_build
            {
                data << uint8_t(0);
                data << uint8_t(0);
                data << uint8_t(0);

                data << "";
                data << "";
                data << float(0);

                data << uint8_t(0);
                data << uint8_t(0);
                data << uint8_t(realm->id);
            }
        }
        data << uint8_t(0x17);
        data << uint8_t(0);

        realmLock.unlock();

        *reinterpret_cast<uint16_t*>(&data.contents()[1]) = uint16_t(data.size() - 3);

        authSocket->Send(static_cast<const uint8_t*>(data.contents()), uint32_t(data.size()));

        std::list<LogonCommServerSocket*> server_sockets;

        serverSocketLock.lock();

        if (this->serverSockets.empty())
        {
            serverSocketLock.unlock();
            return;
        }

        for (const auto& serverSocket : this->serverSockets)
            server_sockets.push_back(serverSocket);

        serverSocketLock.unlock();

        for (const auto& serverSocket : server_sockets)
            serverSocket->RefreshRealmsPop();
    }

    void RealmManager::timeoutSockets()
    {
        if (!this->usePings)
            return;

        const auto now = uint32_t(time(nullptr));

        std::lock_guard lock(serverSocketLock);

        for (auto logonCommServerSocket = this->serverSockets.begin(); logonCommServerSocket != this->serverSockets.end();)
        {
            auto commServerSocket = *logonCommServerSocket;
            ++logonCommServerSocket;

            const uint32_t last_ping = commServerSocket->last_ping;
            if (last_ping < now && ((now - last_ping) > 300))
            {
                for (auto realmIds : commServerSocket->server_ids)
                    setRealmOffline(realmIds);

                commServerSocket->removed = true;
                this->serverSockets.erase(commServerSocket);
                commServerSocket->Disconnect();
            }
        }
    }

    void RealmManager::checkServers()
    {
        std::lock_guard lock(serverSocketLock);

        for (auto logonCommServerSocket = this->serverSockets.begin(); logonCommServerSocket != this->serverSockets.end();)
        {
            auto commServerSocket = *logonCommServerSocket;
            auto it2 = logonCommServerSocket;
            ++logonCommServerSocket;

            if (!sMasterLogon.IsServerAllowed(commServerSocket->GetRemoteAddress().s_addr))
            {
                sLogger.log(Logging::Severity::INFO, Logging::MessageType::MAJOR, "[RealmManager] Disconnecting socket: {} due to it no longer being on an allowed IP.", commServerSocket->GetRemoteIP());
                commServerSocket->Disconnect();
            }
        }
    }

    void RealmManager::setRealmOffline(uint32_t realm_id)
    {
        std::lock_guard lock(realmLock);

        auto realm = getRealmById(realm_id);
        if (realm != nullptr)
        {
            realm->flags = RealmFlag::OFFLINE | RealmFlag::INVALID;
            realm->_characterMap.clear();
            sLogger.info("[RealmManager] Realm {} is now offline (socket close).", realm_id);
            sLogonSQL->Query("UPDATE realms SET status = 0 WHERE id = %u", uint32_t(realm->id));
        }
    }

    void RealmManager::setRealmPopulation(uint32_t realm_id, float population)
    {
        std::lock_guard lock(realmLock);

        auto realm = getRealmById(realm_id);
        if (realm != nullptr)
        {
            uint8_t flags;
            if (population >= 3)
                flags = RealmFlag::FULL | RealmFlag::INVALID;
            else if (population >= 2)
                flags = RealmFlag::INVALID;
            else if (population >= 1.25)
                flags = 0;
            else if (population >= 0.5)
                flags = RealmFlag::RECOMMENDED;
            else
                flags = RealmFlag::NEW_PLAYERS;

            realm->population = (population > 0) ? (population >= 1) ? (population >= 2) ? 2.0f : 1.0f : 0.0f : 0.0f;
            realm->flags = flags;
        }
    }

    void RealmManager::addServerSocket(::LogonCommServerSocket* sock)
    {
        std::lock_guard lock(serverSocketLock);
        this->serverSockets.insert(sock);
    }

    void RealmManager::removeServerSocket(::LogonCommServerSocket* sock)
    {
        std::lock_guard lock(serverSocketLock);
        this->serverSockets.erase(sock);
    }
}
