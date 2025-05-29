/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <map>
#include <Database/Field.hpp>
#include <Threading/AEThread.h>

struct Account
{
    uint32_t AccountId;
    std::unique_ptr<char[]> GMFlags;
    uint8_t AccountFlags;
    uint32_t Banned;
    uint8_t SrpHash[20]; // the encrypted password field, reversed
    std::unique_ptr<uint8_t[]> SessionKey;
    std::string* UsernamePtr;
    std::string forcedLanguage;
    uint32_t Muted;

    Account()
    {
        GMFlags = nullptr;
        SessionKey = nullptr;
        AccountId = 0;
        AccountFlags = 0;
        Banned = 0;
        Muted = 0;
        forcedLocale = false;
        UsernamePtr = nullptr;
    }

    ~Account() = default;

    void SetGMFlags(const char* flags)
    {
        size_t len = strlen(flags);
        if (len == 0 || (len == 1 && flags[0] == '0'))
        {
            // no flags
            GMFlags = nullptr;
            return;
        }

        GMFlags = std::make_unique<char[]>(len + 1);
        memcpy(GMFlags.get(), flags, len);
        GMFlags[len] = 0;
    }

    void SetSessionKey(const uint8_t* key)
    {
        if (SessionKey == nullptr)
            SessionKey = std::make_unique<uint8_t[]>(40);
        memcpy(SessionKey.get(), key, 40);
    }

    bool forcedLocale;

};

class AccountMgr
{
private:
    AccountMgr() = default;
    ~AccountMgr() = default;

public:
    static AccountMgr& getInstance();
    void initialize(uint32_t reloadTime);
    void finalize();

    AccountMgr(AccountMgr&&) = delete;
    AccountMgr(AccountMgr const&) = delete;
    AccountMgr& operator=(AccountMgr&&) = delete;
    AccountMgr& operator=(AccountMgr const&) = delete;

    void addAccount(Field* field);

    Account* getAccountByName(std::string const& Name) const;

    void updateAccount(Account* account, Field* field) const;
    void reloadAccounts(bool silent);

    size_t getCount() const;

    std::map<std::string, std::unique_ptr<Account>> const& getAccountMap() const;

private:
    Account* _getAccountByNameLockFree(std::string const& Name) const;

    std::map<std::string, std::unique_ptr<Account>> _accountMap;

    std::unique_ptr<AscEmu::Threading::AEThread> m_reloadThread;
    uint32_t m_reloadTime;

protected:
    mutable std::mutex accountMgrMutex;
};

#define sAccountMgr AccountMgr::getInstance()
