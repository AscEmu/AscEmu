/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonStdAfx.h"
#include "Logon.h"
#include "RealmsMgr.h"
#include "Util.hpp"
#include <Threading/AEThreadPool.h>

initialiseSingleton(RealmsMgr);

RealmsMgr::RealmsMgr()
{
    LogNotice("RealmsMgr : Starting...");
    usePings = !logonConfig.logonServer.disablePings;

    LoadRealms();
    LogDetail("RealmsMgr : Loaded %u realms definitisons.", static_cast<uint32_t>(sRealmsMgr._realmStore.size()));
}

void RealmsMgr::LoadRealms()
{
    const auto result = sLogonSQL->Query("SELECT id, password, status FROM realms");
    if (result != nullptr)
    {
        do
        {
            const auto field = result->Fetch();
            const uint32_t realmsCount = result->GetRowCount();
            _realmStore.reserve(realmsCount);

            auto realms = std::make_shared<Realms>();
            realms->id = field[0].GetUInt32();
            realms->password = field[1].GetString();
            realms->status = field[2].GetUInt8();
            realms->lastPing = Util::TimeNow();

            _realmStore.emplace_back(std::move(realms));
        } while (result->NextRow());

        delete result;
    }
}

std::shared_ptr<Realms> RealmsMgr::getRealmById(uint32_t id) const
{
    for (const auto& realm : _realmStore)
    {
        if (realm->id == id)
            return realm;
    }

    return nullptr;
}

void RealmsMgr::setStatusForRealm(uint8_t realm_id, uint32_t status)
{
    if (_realmStore.empty())
    {
        auto realms = std::make_shared<Realms>();
        realms->id = realm_id;
        realms->status = uint8_t(status);
        realms->lastPing = Util::TimeNow();

        _realmStore.push_back(std::move(realms));

        sLogonSQL->Query("REPLACE INTO realms(id, status, status_change_time) VALUES(%u, %u, NOW())", status, uint32_t(realm_id));
    }
    else
    {
        for (auto& realm : _realmStore)
        {
            if (realm->id == realm_id)
                realm->status = status;
        }

        sLogonSQL->Query("UPDATE realms SET status = %u WHERE id = %u", status, uint32_t(realm_id));
    }
}

void RealmsMgr::setLastPing(uint8_t realm_id)
{
    for (auto& realm : _realmStore)
    {
        if (realm->id == realm_id)
            realm->lastPing = Util::TimeNow();
    }
}

void RealmsMgr::checkRealmStatus()
{
    for (auto& realm : _realmStore)
    {
        // if there was no ping in the last 2 minutes (in miliseconds) we set the status to the realm to offline.
        if (Util::GetTimeDifferenceToNow(realm->lastPing) > 2 * 60 * 1000 && realm->status != 0)
        {
            realm->status = 0;
            LogDetail("Realm %u status gets set to 0 (offline) since there was no ping the last 2 minutes (%u).", uint32_t(realm->id), Util::GetTimeDifferenceToNow(realm->lastPing));
            sLogonSQL->Query("UPDATE realms SET status = 0 WHERE id = %u", uint32_t(realm->id));
        }
    }
}

void RealmsMgr::sendRealms(AuthSocket* Socket)
{
    realmLock.Acquire();

    ByteBuffer data(_realmStore.size() * 150 + 20);
    data << uint8_t(0x10);
    data << uint16_t(0);
    data << uint32_t(0);

    if (Socket->GetChallenge()->build == 5875)
        data << uint8_t(_realmStore.size());
    else
        data << uint16_t(_realmStore.size());

    std::unordered_map<uint32_t, uint8_t>::iterator it;
    for (const auto realms : _realmStore)
    {
        if (realms->gameBuild == Socket->GetChallenge()->build)
        {
            if (realms->gameBuild == 5875)
            {
                data << uint32_t(realms->icon);
                data << uint8_t(realms->flags);

                data << realms->name;
                data << realms->address;
                data << float(realms->population);

                it = realms->_characterMap.find(Socket->GetAccountID());
                data << uint8_t((it == realms->_characterMap.end()) ? 0 : it->second);
                data << uint8_t(realms->timeZone);
                data << uint8_t(0);
            }
            else
            {
                data << uint8_t(realms->icon);
                data << uint8_t(realms->lock);
                data << uint8_t(realms->flags);

                data << realms->name;
                data << realms->address;
                data << float(realms->population);

                it = realms->_characterMap.find(Socket->GetAccountID());
                data << uint8_t(it == realms->_characterMap.end() ? 0 : it->second);
                data << uint8_t(realms->timeZone);
                data << uint8_t(realms->id);

                if (realms->flags & REALM_FLAG_SPECIFYBUILD)
                {
                    data << Socket->GetChallenge()->version1;
                    data << Socket->GetChallenge()->version2;
                    data << Socket->GetChallenge()->version3;
                    data << Socket->GetChallenge()->build;
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
            data << uint8_t(realms->id);
        }
    }
    data << uint8_t(0x17);
    data << uint8_t(0);

    realmLock.Release();

    *reinterpret_cast<uint16_t*>(&data.contents()[1]) = uint16_t(data.size() - 3);

    Socket->Send(static_cast<const uint8*>(data.contents()), uint32_t(data.size()));

    std::list<LogonCommServerSocket*> server_sockets;

    serverSocketLock.Acquire();

    if (m_serverSockets.empty())
    {
        serverSocketLock.Release();
        return;
    }

    for (const auto& serverSocket : m_serverSockets)
        server_sockets.push_back(serverSocket);

    serverSocketLock.Release();

    for (const auto& serverSocket : server_sockets)
        serverSocket->RefreshRealmsPop();
}

void RealmsMgr::timeoutSockets()
{
    if (!usePings)
        return;

    const auto now = uint32_t(time(nullptr));

    serverSocketLock.Acquire();

    for (auto logonCommServerSocket = m_serverSockets.begin(); logonCommServerSocket != m_serverSockets.end();)
    {
        auto commServerSocket = *logonCommServerSocket;
        ++logonCommServerSocket;

        const uint32_t last_ping = commServerSocket->last_ping;
        if (last_ping < now && ((now - last_ping) > 300))
        {
            for (auto realmIds : commServerSocket->server_ids)
                setRealmOffline(realmIds);

            commServerSocket->removed = true;
            m_serverSockets.erase(commServerSocket);
            commServerSocket->Disconnect();
        }
    }

    serverSocketLock.Release();
}

void RealmsMgr::checkServers()
{
    serverSocketLock.Acquire();

    for (auto logonCommServerSocket = m_serverSockets.begin(); logonCommServerSocket != m_serverSockets.end();)
    {
        auto commServerSocket = *logonCommServerSocket;
        auto it2 = logonCommServerSocket;
        ++logonCommServerSocket;

        if (!sMasterLogon.IsServerAllowed(commServerSocket->GetRemoteAddress().s_addr))
        {
            LOG_DETAIL("Disconnecting socket: %s due to it no longer being on an allowed IP.", commServerSocket->GetRemoteIP().c_str());
            commServerSocket->Disconnect();
        }
    }

    serverSocketLock.Release();
}

void RealmsMgr::setRealmOffline(uint32_t realm_id)
{
    realmLock.Acquire();

    auto realm = getRealmById(realm_id);
    if (realm != nullptr)
    {
        realm->flags = REALM_FLAG_OFFLINE | REALM_FLAG_INVALID;
        realm->_characterMap.clear();
        LogNotice("RealmsMgr : Realm %u is now offline (socket close).", realm_id);
        sLogonSQL->Query("UPDATE realms SET status = 0 WHERE id = %u", uint32_t(realm->id));
    }

    realmLock.Release();
}

void RealmsMgr::updateRealmPop(uint32_t realm_id, float pop)
{
    realmLock.Acquire();

    auto realm = getRealmById(realm_id);
    if (realm != nullptr)
    {
        uint8_t flags;
        if (pop >= 3)
            flags = REALM_FLAG_FULL | REALM_FLAG_INVALID;
        else if (pop >= 2)
            flags = REALM_FLAG_INVALID;
        else if (pop >= 1.25)
            flags = 0;
        else if (pop >= 0.5)
            flags = REALM_FLAG_RECOMMENDED;
        else
            flags = REALM_FLAG_NEW_PLAYERS;

        realm->population = (pop > 0) ? (pop >= 1) ? (pop >= 2) ? 2.0f : 1.0f : 0.0f : 0.0f;
        realm->flags = flags;
    }

    realmLock.Release();
}
