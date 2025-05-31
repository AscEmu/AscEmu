/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "IpBanMgr.h"
#include <utility>
#include <Logging/Logger.hpp>
#include <Database/Database.h>
#include "Server/Master.hpp"
#include <Logging/Log.hpp>

#include "Utilities/Util.hpp"

IpBanMgr& IpBanMgr::getInstance()
{
    static IpBanMgr mInstance;
    return mInstance;
}

void IpBanMgr::initialize()
{
    sLogger.info("IpBanMgr : Started loading bans");

    reload();

    sLogger.info("IpBanMgr : loaded {} IP bans.", static_cast<uint32_t>(_ipBanList.size()));
}

void IpBanMgr::reload()
{
    std::lock_guard lock(ipBanMutex);
    _ipBanList.clear();

    auto result = sLogonSQL->Query("SELECT ip, expire FROM ipbans");
    if (result)
    {
        do
        {
            std::string ipString = result->Fetch()[0].asCString();
            const uint32_t expireTime = result->Fetch()[1].asUint32();

            std::string smask = "32";
            
            std::string::size_type i = ipString.find('/');
            std::string stmp = ipString.substr(0, i);
            if (i == std::string::npos)
                sLogger.info("IP ban '{}' netmask not specified. assuming /32", ipString);
            else
                smask = ipString.substr(i + 1);

            const unsigned int ipraw = Util::makeIP(stmp.c_str());
            const unsigned int ipmask = atoi(smask.c_str());
            if (ipraw == 0 || ipmask == 0)
            {
                sLogger.failure("IP ban '{}' could not be parsed. Ignoring", ipString);
                continue;
            }

            IPBan ipBan;
            ipBan.Bytes = static_cast<unsigned char>(ipmask);
            ipBan.Mask = ipraw;
            ipBan.Expire = expireTime;
            ipBan.db_ip = ipString;
            _ipBanList.push_back(ipBan);

        } while (result->NextRow());
    }
}

bool IpBanMgr::add(std::string ip, uint32_t duration)
{
    const std::string& ipString = ip;

    const std::string::size_type i = ipString.find('/');
    if (i == std::string::npos)
        return false;

    std::string stmp = ipString.substr(0, i);
    std::string smask = ipString.substr(i + 1);

    const unsigned int ipraw = Util::makeIP(stmp.c_str());
    const unsigned int ipmask = atoi(smask.c_str());
    if (ipraw == 0 || ipmask == 0)
        return false;

    IPBan ipBan;
    ipBan.db_ip = ipString;
    ipBan.Bytes = static_cast<unsigned char>(ipmask);
    ipBan.Mask = ipraw;
    ipBan.Expire = duration;

    std::lock_guard lock(ipBanMutex);

    _ipBanList.push_back(ipBan);

    return true;
}

bool IpBanMgr::remove(const std::string& ip)
{
    std::lock_guard lock(ipBanMutex);

    for (auto itr = _ipBanList.begin(); itr != _ipBanList.end();)
    {
        if (itr->db_ip == ip)
        {
            _ipBanList.erase(itr);
            return true;
        }
        
        ++itr;
    }

    return false;
}

IpBanStatus IpBanMgr::getBanStatus(in_addr ip_address)
{
    std::lock_guard lock(ipBanMutex);

    for (auto itr2 = _ipBanList.begin(); itr2 != _ipBanList.end();)
    {
        const auto bannedIp = itr2;
        ++itr2;

        if (Util::parseCIDRBan(ip_address.s_addr, bannedIp->Mask, bannedIp->Bytes))
        {
            if (bannedIp->Expire == 0)
                return BAN_STATUS_PERMANENT_BAN;

            if (static_cast<uint32_t>(UNIXTIME) >= bannedIp->Expire)
            {
                sLogonSQL->Execute("DELETE FROM ipbans WHERE expire = %u AND ip = \"%s\"", bannedIp->Expire, sLogonSQL->EscapeString(bannedIp->db_ip).c_str());
                _ipBanList.erase(bannedIp);
            }
            else
            {
                return BAN_STATUS_TIME_LEFT_ON_BAN;
            }
        }
    }

    return BAN_STATUS_NOT_BANNED;
}
