/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#include "LogonStdAfx.h"


initialiseSingleton(IPBanner);
initialiseSingleton(InformationCore);



BAN_STATUS IPBanner::CalculateBanStatus(in_addr ip_address)
{
    Guard lguard(listBusy);
    
    for (auto itr2 = banList.begin(); itr2 != banList.end();)
    {
        const auto bannedIp = itr2;
        ++itr2;

        if (ParseCIDRBan(ip_address.s_addr, bannedIp->Mask, bannedIp->Bytes))
        {
            // ban hit
            if (bannedIp->Expire == 0)
                return BAN_STATUS_PERMANENT_BAN;

            if (static_cast<uint32>(UNIXTIME) >= bannedIp->Expire)
            {
                sLogonSQL->Execute("DELETE FROM ipbans WHERE expire = %u AND ip = \"%s\"", bannedIp->Expire, sLogonSQL->EscapeString(bannedIp->db_ip).c_str());
                banList.erase(bannedIp);
            }
            else
            {
                return BAN_STATUS_TIME_LEFT_ON_BAN;
            }
        }
    }

    return BAN_STATUS_NOT_BANNED;
}

bool IPBanner::Add(const char* ip, uint32 dur)
{
    std::string sip = std::string(ip);

    std::string::size_type i = sip.find("/");
    if (i == std::string::npos)
        return false;

    std::string stmp = sip.substr(0, i);
    std::string smask = sip.substr(i + 1);

    const unsigned int ipraw = MakeIP(stmp.c_str());
    const unsigned int ipmask = atoi(smask.c_str());
    if (ipraw == 0 || ipmask == 0)
        return false;

    IPBan ipb;
    ipb.db_ip = sip;
    ipb.Bytes = static_cast<unsigned char>(ipmask);
    ipb.Mask = ipraw;
    ipb.Expire = dur;

    listBusy.Acquire();
    banList.push_back(ipb);
    listBusy.Release();

    return true;
}

InformationCore::~InformationCore()
{
    for (auto itr = m_realms.begin(); itr != m_realms.end(); ++itr)
        delete itr->second;
}

bool IPBanner::Remove(const char* ip)
{
    listBusy.Acquire();
    for (auto itr = banList.begin(); itr != banList.end(); ++itr)
    {
        if (!strcmp(ip, itr->db_ip.c_str()))
        {
            banList.erase(itr);
            listBusy.Release();
            return true;
        }
    }
    listBusy.Release();
    return false;
}

void IPBanner::Reload()
{
    listBusy.Acquire();
    banList.clear();

    QueryResult* result = sLogonSQL->Query("SELECT ip, expire FROM ipbans");
    if (result != nullptr)
    {
        do
        {
            IPBan ipb;
            std::string smask = "32";
            std::string ip = result->Fetch()[0].GetString();
            std::string::size_type i = ip.find("/");
            std::string stmp = ip.substr(0, i);
            if (i == std::string::npos)
            {
                LOG_DETAIL("IP ban \"%s\" netmask not specified. assuming /32", ip.c_str());
            }
            else
                smask = ip.substr(i + 1);

            const unsigned int ipraw = MakeIP(stmp.c_str());
            const unsigned int ipmask = atoi(smask.c_str());
            if (ipraw == 0 || ipmask == 0)
            {
                LOG_ERROR("IP ban \"%s\" could not be parsed. Ignoring", ip.c_str());
                continue;
            }

            ipb.Bytes = static_cast<unsigned char>(ipmask);
            ipb.Mask = ipraw;
            ipb.Expire = result->Fetch()[1].GetUInt32();
            ipb.db_ip = ip;
            banList.push_back(ipb);

        } while (result->NextRow());
        delete result;
    }
    listBusy.Release();
}

Realm* InformationCore::AddRealm(uint32 realm_id, Realm* rlm)
{
    realmLock.Acquire();
    auto itr = m_realms.find(realm_id);
    if (itr == m_realms.end())
    {
        m_realms.insert(std::make_pair(realm_id, rlm));
    }
    else
    {
        delete itr->second;
        itr->second = rlm;
    }
    realmLock.Release();
    return rlm;
}

Realm* InformationCore::GetRealm(uint32 realm_id)
{
    Realm* ret = nullptr;

    realmLock.Acquire();

    const auto itr = m_realms.find(realm_id);
    if (itr != m_realms.end())
        ret = itr->second;

    realmLock.Release();
    return ret;
}

int32 InformationCore::GetRealmIdByName(const std::string& Name)
{
    for (auto itr = m_realms.begin(); itr != m_realms.end(); ++itr)
    {
        if (itr->second->Name == Name)
            return itr->first;
    }

    return -1;
}

void InformationCore::RemoveRealm(uint32 realm_id)
{
    realmLock.Acquire();
    auto itr = m_realms.find(realm_id);
    if (itr != m_realms.end())
    {
        delete itr->second;
        m_realms.erase(itr);
    }
    realmLock.Release();
}

void InformationCore::UpdateRealmStatus(uint32 realm_id, uint8 flags)
{
    realmLock.Acquire();
    const auto itr = m_realms.find(realm_id);
    if (itr != m_realms.end())
        itr->second->flags = flags;

    realmLock.Release();
}

void InformationCore::UpdateRealmPop(uint32 realm_id, float pop)
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

void InformationCore::SendRealms(AuthSocket* Socket)
{
    realmLock.Acquire();

    // packet header
    ByteBuffer data(m_realms.size() * 150 + 20);
    data << uint8(0x10);
    data << uint16(0); // Size Placeholder

    // dunno what this is..
    data << uint32(0);

    //sAuthLogonChallenge_C * client = Socket->GetChallenge();
    if (Socket->GetChallenge()->build == 5875)
        data << uint8(m_realms.size());
    else
        data << uint16(m_realms.size());

    // loop realms :/
    std::unordered_map<uint32, uint8>::iterator it;
    for (auto itr = m_realms.begin(); itr != m_realms.end(); ++itr)
    {
        if (itr->second->GameBuild == Socket->GetChallenge()->build)
        {
            if (itr->second->GameBuild == 5875)
            {
                data << uint32(itr->second->Icon);
                data << uint8(itr->second->flags);

                data << itr->second->Name;
                data << itr->second->Address;
                data << float(itr->second->Population);

                // Get our character count
                it = itr->second->CharacterMap.find(Socket->GetAccountID());
                data << uint8((it == itr->second->CharacterMap.end()) ? 0 : it->second);
                data << uint8(itr->second->TimeZone);
                data << uint8(0); // Realm ID
            }
            else
            {
                data << uint8(itr->second->Icon);
                data << uint8(itr->second->Lock); // delete when using data << itr->second->Lock;
                data << uint8(itr->second->flags);

                data << itr->second->Name;
                data << itr->second->Address;
                data << float(itr->second->Population);

                // Get our character count
                it = itr->second->CharacterMap.find(Socket->GetAccountID());
                data << uint8((it == itr->second->CharacterMap.end()) ? 0 : it->second);
                data << uint8(itr->second->TimeZone);
                data << uint8(GetRealmIdByName(itr->second->Name)); // Realm ID

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
            data << uint8(0);
            data << uint8(0); // delete when using data << itr->second->Lock;
            data << uint8(0);

            data << "";
            data << "";
            data << float(0);

            data << uint8(0);
            data << uint8(0);
            data << uint8(GetRealmIdByName(itr->second->Name)); // Realm ID
        }
    }
    data << uint8(0x17);
    data << uint8(0);

    realmLock.Release();

    // Re-calculate size.

    *reinterpret_cast<uint16*>(&data.contents()[1]) = uint16(data.size() - 3);

    // Send to the socket.
    Socket->Send(static_cast<const uint8*>(data.contents()), uint32(data.size()));

    std::list< LogonCommServerSocket* > ss;
    std::list< LogonCommServerSocket* >::iterator SSitr;

    ss.clear();

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

    ss.clear();
}

void InformationCore::TimeoutSockets()
{
    if (!usepings)
        return;

    const auto now = uint32(time(nullptr));

    /* burlex: this is vulnerable to race conditions, adding a mutex to it. */
    serverSocketLock.Acquire();

    for (auto itr = m_serverSockets.begin(); itr != m_serverSockets.end();)
    {
        auto commServerSocket = *itr;
        ++itr;

        const uint32 last_ping = commServerSocket->last_ping;
        if (last_ping < now && ((now - last_ping) > 300))
        {
            for (auto RealmITR = commServerSocket->server_ids.begin(); RealmITR != commServerSocket->server_ids.end(); ++RealmITR)
            {
                const uint32 RealmID = *RealmITR;

                SetRealmOffline(RealmID);
            }

            commServerSocket->removed = true;
            m_serverSockets.erase(commServerSocket);
            commServerSocket->Disconnect();
        }
    }

    serverSocketLock.Release();
}

void InformationCore::CheckServers()
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

void InformationCore::SetRealmOffline(uint32 realm_id)
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
