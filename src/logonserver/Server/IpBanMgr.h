/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

struct IPBan
{
    unsigned int Mask;
    unsigned char Bytes;
    uint32_t Expire;
    std::string db_ip;
};

enum IpBanStatus
{
    BAN_STATUS_NOT_BANNED = 0,
    BAN_STATUS_TIME_LEFT_ON_BAN = 1,
    BAN_STATUS_PERMANENT_BAN = 2,
};

class IpBanMgr
{
private:
    IpBanMgr() = default;
    ~IpBanMgr() = default;

public:

    static IpBanMgr& getInstance();
    void initialize();

    IpBanMgr(IpBanMgr&&) = delete;
    IpBanMgr(IpBanMgr const&) = delete;
    IpBanMgr& operator=(IpBanMgr&&) = delete;
    IpBanMgr& operator=(IpBanMgr const&) = delete;

    void reload();

    bool add(std::string ip, uint32_t duration);
    bool remove(const std::string& ip);

    IpBanStatus getBanStatus(in_addr ip_address);

protected:
    Mutex ipBanMutex;

    std::list<IPBan> _ipBanList;
};

#define sIpBanMgr IpBanMgr::getInstance()
