/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

struct Account
{
    uint32_t AccountId;
    char* GMFlags;
    uint8_t AccountFlags;
    uint32_t Banned;
    uint8_t SrpHash[20]; // the encrypted password field, reversed
    uint8_t* SessionKey;
    std::string* UsernamePtr;
    std::string forcedLanguage;
    uint32_t Muted;

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

    void SetSessionKey(const uint8_t* key)
    {
        if (SessionKey == NULL)
            SessionKey = new uint8[40];
        memcpy(SessionKey, key, 40);
    }

    bool forcedLocale;

};

class AccountMgr : public Singleton <AccountMgr>
{
public:

    AccountMgr();

    void addAccount(Field* field);

    std::shared_ptr<Account> getAccountByName(std::string& Name);

    void updateAccount(std::shared_ptr<Account> account, Field* field);
    void reloadAccounts(bool silent);
    void reloadAccountsCallback();

    size_t getCount() const;

    std::map<std::string, std::shared_ptr<Account>> getAccountMap() const;

private:

    std::shared_ptr<Account> _getAccountByNameLockFree(std::string& Name);

    std::map<std::string, std::shared_ptr<Account>> _accountMap;

protected:

    Mutex accountMgrMutex;
};

#define sAccountMgr AccountMgr::getSingleton()
