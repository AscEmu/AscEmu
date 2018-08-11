/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Auth/AccountCache.h"

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
