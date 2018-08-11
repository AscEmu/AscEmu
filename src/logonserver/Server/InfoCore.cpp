/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonStdAfx.h"
#include "InfoCore.h"

initialiseSingleton(InfoCore);

InfoCore::InfoCore()
{
    LogNotice("InfoCore : Starting...");

    realmhigh = 0;
    usepings = !Config.MainConfig.getBoolDefault("LogonServer", "DisablePings", false);
    m_realms.clear();

    LogDetail("InfoCore : Started");
}

std::shared_ptr<Realm> InfoCore::addRealm(uint32_t realm_id, std::shared_ptr<Realm> rlm)
{
    realmLock.Acquire();

    auto itr = m_realms.find(realm_id);
    if (itr == m_realms.end())
        m_realms.insert(std::make_pair(realm_id, rlm));
    else
        itr->second = rlm;

    realmLock.Release();
    return rlm;
}

std::shared_ptr<Realm> InfoCore::getRealm(uint32_t realm_id)
{
    std::shared_ptr<Realm> ret = nullptr;

    realmLock.Acquire();

    const auto itr = m_realms.find(realm_id);
    if (itr != m_realms.end())
        ret = itr->second;

    realmLock.Release();
    return ret;
}

uint32_t InfoCore::getRealmIdByName(const std::string& Name)
{
    for (const auto& itr : m_realms)
    {
        if (itr.second->Name == Name)
            return itr.first;
    }

    return 0;
}

void InfoCore::removeRealm(uint32_t realm_id)
{
    realmLock.Acquire();

    auto itr = m_realms.find(realm_id);
    if (itr != m_realms.end())
        m_realms.erase(itr);

    realmLock.Release();
}

void InfoCore::updateRealmStatus(uint32_t realm_id, uint8_t flags)
{
    realmLock.Acquire();

    const auto itr = m_realms.find(realm_id);
    if (itr != m_realms.end())
        itr->second->flags = flags;

    realmLock.Release();
}

void InfoCore::updateRealmPop(uint32_t realm_id, float pop)
{
    realmLock.Acquire();

    const auto itr = m_realms.find(realm_id);
    if (itr != m_realms.end())
    {
        uint8 flags;
        if (pop >= 3)
            flags = REALM_FLAG_FULL | REALM_FLAG_INVALID; // Full
        else if (pop >= 2)
            flags = REALM_FLAG_INVALID; // Red
        else if (pop >= 0.5)
            flags = 0; // Green
        else
            flags = REALM_FLAG_NEW_PLAYERS; // recommended

        itr->second->Population = (pop > 0) ? (pop >= 1) ? (pop >= 2) ? 2.0f : 1.0f : 0.0f : 0.0f;
        itr->second->flags = flags;
    }

    realmLock.Release();
}

void InfoCore::sendRealms(AuthSocket* Socket)
{
    realmLock.Acquire();

    // packet header
    ByteBuffer data(m_realms.size() * 150 + 20);
    data << uint8_t(0x10);
    data << uint16_t(0); // Size Placeholder

    // dunno what this is..
    data << uint32_t(0);

    //sAuthLogonChallenge_C * client = Socket->GetChallenge();
    if (Socket->GetChallenge()->build == 5875)
        data << uint8_t(m_realms.size());
    else
        data << uint16_t(m_realms.size());

    // loop realms :/
    std::unordered_map<uint32_t, uint8_t>::iterator it;
    for (auto itr = m_realms.begin(); itr != m_realms.end(); ++itr)
    {
        if (itr->second->GameBuild == Socket->GetChallenge()->build)
        {
            if (itr->second->GameBuild == 5875)
            {
                data << uint32_t(itr->second->Icon);
                data << uint8_t(itr->second->flags);

                data << itr->second->Name;
                data << itr->second->Address;
                data << float(itr->second->Population);

                // Get our character count
                it = itr->second->CharacterMap.find(Socket->GetAccountID());
                data << uint8_t((it == itr->second->CharacterMap.end()) ? 0 : it->second);
                data << uint8_t(itr->second->TimeZone);
                data << uint8_t(0); // Realm ID
            }
            else
            {
                data << uint8_t(itr->second->Icon);
                data << uint8_t(itr->second->Lock); // delete when using data << itr->second->Lock;
                data << uint8_t(itr->second->flags);

                data << itr->second->Name;
                data << itr->second->Address;
                data << float(itr->second->Population);

                // Get our character count
                it = itr->second->CharacterMap.find(Socket->GetAccountID());
                data << uint8_t((it == itr->second->CharacterMap.end()) ? 0 : it->second);
                data << uint8_t(itr->second->TimeZone);
                data << uint8_t(getRealmIdByName(itr->second->Name)); // Realm ID

                if (itr->second->flags & REALM_FLAG_SPECIFYBUILD)
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
            data << uint8_t(0); // delete when using data << itr->second->Lock;
            data << uint8_t(0);

            data << "";
            data << "";
            data << float(0);

            data << uint8_t(0);
            data << uint8_t(0);
            data << uint8_t(getRealmIdByName(itr->second->Name)); // Realm ID
        }
    }
    data << uint8_t(0x17);
    data << uint8_t(0);

    realmLock.Release();

    // Re-calculate size.

    *reinterpret_cast<uint16_t*>(&data.contents()[1]) = uint16_t(data.size() - 3);

    // Send to the socket.
    Socket->Send(static_cast<const uint8*>(data.contents()), uint32_t(data.size()));

    std::list< LogonCommServerSocket* > server_sockets;

    serverSocketLock.Acquire();

    if (m_serverSockets.empty())
    {
        serverSocketLock.Release();
        return;
    }

    for (const auto server_socket : m_serverSockets)
        server_sockets.push_back(server_socket);

    serverSocketLock.Release();

    for (const auto server_socket : server_sockets)
        server_socket->RefreshRealmsPop();
}

void InfoCore::timeoutSockets()
{
    if (!usepings)
        return;

    const auto now = uint32_t(time(nullptr));

    serverSocketLock.Acquire();

    for (auto itr = m_serverSockets.begin(); itr != m_serverSockets.end();)
    {
        auto commServerSocket = *itr;
        ++itr;

        const uint32_t last_ping = commServerSocket->last_ping;
        if (last_ping < now && ((now - last_ping) > 300))
        {
            for (auto RealmITR = commServerSocket->server_ids.begin(); RealmITR != commServerSocket->server_ids.end(); ++RealmITR)
            {
                const uint32_t RealmID = *RealmITR;

                setRealmOffline(RealmID);
            }

            commServerSocket->removed = true;
            m_serverSockets.erase(commServerSocket);
            commServerSocket->Disconnect();
        }
    }

    serverSocketLock.Release();
}

void InfoCore::checkServers()
{
    serverSocketLock.Acquire();

    for (auto itr = m_serverSockets.begin(); itr != m_serverSockets.end();)
    {
        auto commServerSocket = *itr;
        auto it2 = itr;
        ++itr;

        if (!sMasterLogon.IsServerAllowed(commServerSocket->GetRemoteAddress().s_addr))
        {
            LOG_DETAIL("Disconnecting socket: %s due to it no longer being on an allowed IP.", commServerSocket->GetRemoteIP().c_str());
            commServerSocket->Disconnect();
        }
    }

    serverSocketLock.Release();
}

void InfoCore::setRealmOffline(uint32_t realm_id)
{
    realmLock.Acquire();

    auto itr = m_realms.find(realm_id);
    if (itr != m_realms.end())
    {
        itr->second->flags = REALM_FLAG_OFFLINE | REALM_FLAG_INVALID;
        itr->second->CharacterMap.clear();
        LogNotice("InfoCore : Realm %u is now offline (socket close).", realm_id);
    }

    realmLock.Release();
}
