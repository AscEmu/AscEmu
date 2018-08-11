/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "Server/LogonServerDefines.hpp"
#include "../shared/Database/DatabaseEnv.h"

struct Account
{
    uint32 AccountId;
    char* GMFlags;
    uint8 AccountFlags;
    uint32 Banned;
    uint8 SrpHash[20]; // the encrypted password field, reversed
    uint8* SessionKey;
    std::string* UsernamePtr;
    std::string forcedLanguage;
    uint32 Muted;

    Account()
    {
        GMFlags = NULL;
        SessionKey = NULL;
        AccountId = 0;
        AccountFlags = 0;
        Banned = 0;
        Muted = 0;
        forcedLocale = false;
        UsernamePtr = nullptr;
    }

    ~Account()
    {
        delete[] GMFlags;
        delete[] SessionKey;
    }

    void SetGMFlags(const char* flags)
    {
        delete[] GMFlags;

        size_t len = strlen(flags);
        if (len == 0 || (len == 1 && flags[0] == '0'))
        {
            // no flags
            GMFlags = NULL;
            return;
        }

        GMFlags = new char[len + 1];
        memcpy(GMFlags, flags, len);
        GMFlags[len] = 0;
    }

    void SetSessionKey(const uint8* key)
    {
        if (SessionKey == NULL)
            SessionKey = new uint8[40];
        memcpy(SessionKey, key, 40);
    }

    bool forcedLocale;

};

typedef struct
{
    unsigned int Mask;
    unsigned char Bytes;
    uint32 Expire;
    std::string db_ip;
} IPBan;

enum BAN_STATUS
{
    BAN_STATUS_NOT_BANNED = 0,
    BAN_STATUS_TIME_LEFT_ON_BAN = 1,
    BAN_STATUS_PERMANENT_BAN = 2,
};

class IPBanner : public Singleton< IPBanner >
{
public:
    void Reload();

    bool Add(const char* ip, uint32 dur);
    bool Remove(const char* ip);

    BAN_STATUS CalculateBanStatus(in_addr ip_address);

protected:
    Mutex listBusy;
    std::list<IPBan> banList;
};

typedef struct
{
    std::string Name;
    std::string Address;
    uint32 flags;
    uint32 Icon;
    uint32 TimeZone;
    float Population;
    uint8 Lock;
    uint32 GameBuild;
    std::unordered_map<uint32, uint8> CharacterMap;
} Realm;

class AuthSocket;
class LogonCommServerSocket;

class InformationCore : public Singleton<InformationCore>
{
    std::map<uint32, Realm*> m_realms;
    std::set<LogonCommServerSocket*> m_serverSockets;
    Mutex serverSocketLock;
    Mutex realmLock;

    uint32 realmhigh;
    bool usepings;

    public:

        ~InformationCore();

        inline Mutex & getServerSocketLock() { return serverSocketLock; }
        inline Mutex & getRealmLock() { return realmLock; }

        InformationCore()
        {
            realmhigh = 0;
            usepings = !Config.MainConfig.getBoolDefault("LogonServer", "DisablePings", false);
            m_realms.clear();
        }

#ifdef AE_TBC
        void writeRealmDataTbc(AuthSocket* socket);
#endif
        // Packets
        void SendRealms(AuthSocket* Socket);

        // Realm management
        uint32 GenerateRealmID()
        {
            return ++realmhigh;
        }

        Realm* AddRealm(uint32 realm_id, Realm* rlm);
        Realm* GetRealm(uint32 realm_id);
        int32 GetRealmIdByName(const std::string& Name);
        void RemoveRealm(uint32 realm_id);
        void SetRealmOffline(uint32 realm_id);
        void UpdateRealmStatus(uint32 realm_id, uint8 flags);
        void UpdateRealmPop(uint32 realm_id, float pop);

        inline void AddServerSocket(LogonCommServerSocket* sock)
        {
            serverSocketLock.Acquire();
            m_serverSockets.insert(sock);
            serverSocketLock.Release();
        }
        inline void RemoveServerSocket(LogonCommServerSocket* sock)
        {
            serverSocketLock.Acquire();
            m_serverSockets.erase(sock);
            serverSocketLock.Release();
        }

        void TimeoutSockets();
        void CheckServers();
};

#define sIPBanner IPBanner::getSingleton()
#define sInfoCore InformationCore::getSingleton()
