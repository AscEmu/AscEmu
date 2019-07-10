/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonStdAfx.h"
#include "AccountMgr.h"

initialiseSingleton(AccountMgr);

AccountMgr::AccountMgr()
{
    LogNotice("AccountMgr : Started precaching accounts...");

    reloadAccounts(true);

    LogDetail("AccountMgr : loaded %u accounts.", static_cast<uint32_t>(getCount()));
}

void AccountMgr::addAccount(Field* field)
{
    auto account = std::make_shared<Account>();

    account->AccountId = field[0].GetUInt32();

    std::string accountName = field[1].GetString();
    std::string encryptedPassword = field[2].GetString();

    account->AccountFlags = field[3].GetUInt8();
    account->Banned = field[4].GetUInt32();
    account->forcedLanguage = field[5].GetString();
    account->Muted = field[6].GetUInt32();

    if (static_cast<uint32_t>(UNIXTIME) > account->Banned && account->Banned != 0 && account->Banned != 1)
    {
        account->Banned = 0;
        LOG_DEBUG("Account %s's ban has expired.", accountName.c_str());
        sLogonSQL->Execute("UPDATE accounts SET banned = 0 WHERE id = %u", account->AccountId);
    }
    
    if (account->forcedLanguage != "enUS")
        account->forcedLocale = true;
    else
        account->forcedLocale = false;

    if (static_cast<uint32_t>(UNIXTIME) > account->Muted && account->Muted != 0 && account->Muted != 1)
    {
        account->Muted = 0;
        LOG_DEBUG("Account %s's mute has expired.", accountName.c_str());
        sLogonSQL->Execute("UPDATE accounts SET muted = 0 WHERE id = %u", account->AccountId);
    }

    if (encryptedPassword.size() == 40)
    {
        BigNumber bn;
        bn.SetHexStr(encryptedPassword.c_str());
        if (bn.GetNumBytes() < 20)
        {
            memcpy(account->SrpHash, bn.AsByteArray(), bn.GetNumBytes());
            for (auto n = bn.GetNumBytes(); n <= 19; n++)
                account->SrpHash[n] = static_cast<uint8_t>(0);

            std::reverse(std::begin(account->SrpHash), std::end(account->SrpHash));
        }
        else
        {
            memcpy(account->SrpHash, bn.AsByteArray(), 20);
            std::reverse(std::begin(account->SrpHash), std::end(account->SrpHash));
        }
    }
    else
    {
        LOG_ERROR("Account `%s` has incorrect number of bytes in encrypted password! Disabling.", accountName.c_str());
        memset(account->SrpHash, 0, 20);
    }

    Util::StringToUpperCase(accountName);

    _accountMap[accountName] = account;
}

std::shared_ptr<Account> AccountMgr::getAccountByName(std::string& Name)
{
    accountMgrMutex.Acquire();

    auto pAccount = _getAccountByNameLockFree(Name);

    accountMgrMutex.Release();
    return pAccount;
}

void AccountMgr::updateAccount(std::shared_ptr<Account> account, Field* field)
{
    const uint32_t id = field[0].GetUInt32();
    std::string accountName = field[1].GetString();
    std::string encryptedPassword = field[2].GetString();

    if (id != account->AccountId)
    {
        LOG_ERROR(" >> deleting duplicate account %u [%s]...", id, accountName.c_str());
        sLogonSQL->Execute("DELETE FROM accounts WHERE id = %u", id);
        return;
    }

    account->AccountFlags = field[3].GetUInt8();
    account->Banned = field[4].GetUInt32();
    account->forcedLanguage = field[5].GetString();
    account->Muted = field[6].GetUInt32();

    if (static_cast<uint32_t>(UNIXTIME) > account->Banned && account->Banned != 0 && account->Banned != 1)
    {
        account->Banned = 0;
        LOG_DEBUG("Account %s's ban has expired.", accountName.c_str());
        sLogonSQL->Execute("UPDATE accounts SET banned = 0 WHERE id = %u", account->AccountId);
    }

    if (account->forcedLanguage != "enUS")
        account->forcedLocale = true;
    else
        account->forcedLocale = false;

    if (static_cast<uint32_t>(UNIXTIME) > account->Muted && account->Muted != 0 && account->Muted != 1)
    {
        account->Muted = 0;
        LOG_DEBUG("Account %s's mute has expired.", accountName.c_str());
        sLogonSQL->Execute("UPDATE accounts SET muted = 0 WHERE id = %u", account->AccountId);
    }

    if (encryptedPassword.size() == 40)
    {
        BigNumber bn;
        bn.SetHexStr(encryptedPassword.c_str());
        if (bn.GetNumBytes() < 20)
        {
            memcpy(account->SrpHash, bn.AsByteArray(), bn.GetNumBytes());
            for (auto n = bn.GetNumBytes(); n <= 19; n++)
                account->SrpHash[n] = static_cast<uint8_t>(0);

            std::reverse(std::begin(account->SrpHash), std::end(account->SrpHash));
        }
        else
        {
            memcpy(account->SrpHash, bn.AsByteArray(), 20);
            std::reverse(std::begin(account->SrpHash), std::end(account->SrpHash));
        }
    }
    else
    {
        LOG_ERROR("Account `%s` has incorrect number of bytes in encrypted password! Disabling.", accountName.c_str());
        memset(account->SrpHash, 0, 20);
    }
}

void AccountMgr::reloadAccounts(bool silent)
{
    accountMgrMutex.Acquire();

    if (!silent)
        LogDefault("[AccountMgr] Reloading Accounts...");

    std::set<std::string> account_list;

    QueryResult* result = sLogonSQL->Query("SELECT id, acc_name, encrypted_password, flags, banned, forceLanguage, muted FROM accounts");
    if (result)
    {
        do
        {
            Field* field = result->Fetch();
            std::string accountName = field[1].GetString();

            Util::StringToUpperCase(accountName);

            const auto account = _getAccountByNameLockFree(accountName);
            if (account == nullptr)
                addAccount(field);
            else
                updateAccount(account, field);

            account_list.insert(accountName);

        } while (result->NextRow());

        delete result;
    }

    for (auto accounts = _accountMap.begin(); accounts != _accountMap.end();)
    {
        auto accountsCopy = accounts;
        ++accounts;

        if (account_list.find(accountsCopy->first) == account_list.end())
            _accountMap.erase(accountsCopy);
        else
            accountsCopy->second->UsernamePtr = const_cast<std::string*>(&accountsCopy->first);
    }

    if (!silent)
        LogDefault("[AccountMgr] Found %u accounts.", _accountMap.size());

    accountMgrMutex.Release();
}

void AccountMgr::reloadAccountsCallback()
{
    reloadAccounts(true);
}

size_t AccountMgr::getCount() const
{
    return _accountMap.size();
}

std::map<std::string, std::shared_ptr<Account>> AccountMgr::getAccountMap() const
{
    return _accountMap;
}

std::shared_ptr<Account> AccountMgr::_getAccountByNameLockFree(std::string& Name)
{
    const auto itr = _accountMap.find(Name);
    if (itr == _accountMap.end())
        return nullptr;

    return itr->second;
}
