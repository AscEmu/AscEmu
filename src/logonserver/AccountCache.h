/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

#ifndef __ACCOUNTCACHE_H
#define __ACCOUNTCACHE_H

#include "Common.h"
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

    char Locale[4];
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

class AccountMgr : public Singleton < AccountMgr >
{
    public:

        ~AccountMgr()
        {
            for (std::map<std::string, Account*>::iterator itr = AccountDatabase.begin(); itr != AccountDatabase.end(); ++itr)
            {
                delete itr->second;
            }
        }

        void AddAccount(Field* field);

        Account* GetAccount(std::string Name)
        {
            setBusy.Acquire();
            Account* pAccount = NULL;
            // this should already be uppercase!

            std::map<std::string, Account*>::iterator itr = AccountDatabase.find(Name);

            if (itr == AccountDatabase.end())    pAccount = NULL;
            else                                pAccount = itr->second;

            setBusy.Release();
            return pAccount;
        }

        void UpdateAccount(Account* acct, Field* field);
        void ReloadAccounts(bool silent);
        void ReloadAccountsCallback();

        inline size_t GetCount() { return AccountDatabase.size(); }

    private:

        Account* __GetAccount(std::string Name)
        {
            // this should already be uppercase!
            std::map<std::string, Account*>::iterator itr = AccountDatabase.find(Name);

            if (itr == AccountDatabase.end())    return NULL;
            else                                return itr->second;
        }

        std::map<std::string, Account*> AccountDatabase;

    protected:

        Mutex setBusy;
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
    std::map<uint32, Realm*>          m_realms;
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
            usepings = !Config.MainConfig.GetBoolDefault("LogonServer", "DisablePings", false);
            m_realms.clear();
        }

        // Packets
        void SendRealms(AuthSocket* Socket);

        // Realm management
        uint32 GenerateRealmID()
        {
            return ++realmhigh;
        }

        Realm* AddRealm(uint32 realm_id, Realm* rlm);
        Realm* GetRealm(uint32 realm_id);
        int32 GetRealmIdByName(std::string Name);
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
#define sAccountMgr AccountMgr::getSingleton()
#define sInfoCore InformationCore::getSingleton()

#endif  //__ACCOUNTCACHE_H
