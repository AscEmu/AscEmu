/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountMgr.h"
#include <Logging/Logger.hpp>
#include <Logging/Log.hpp>
#include <Cryptography/BigNumber.h>
#include <Utilities/Strings.hpp>
#include <Database/Database.h>
#include "Master.hpp"

AccountMgr& AccountMgr::getInstance()
{
    static AccountMgr mInstance;
    return mInstance;
}

void AccountMgr::initialize(uint32_t reloadTime)
{
    sLogger.info("AccountMgr : Started precaching accounts...");
    m_reloadThread = nullptr;
    m_reloadTime = reloadTime;

    reloadAccounts(true);

    sLogger.info("AccountMgr : loaded {} accounts.", static_cast<uint32_t>(getCount()));

    m_reloadThread = std::make_unique<AscEmu::Threading::AEThread>("ReloadAccounts", [this](AscEmu::Threading::AEThread& /*thread*/) { this->reloadAccounts(false); }, std::chrono::seconds(m_reloadTime));
}

void AccountMgr::finalize()
{
    sLogger.info("AccountMgr : Stop Manager...");

    m_reloadThread->killAndJoin();
}

void AccountMgr::addAccount(Field* field)
{
    auto account = std::make_unique<Account>();

    account->AccountId = field[0].asUint32();

    std::string accountName = field[1].asCString();
    std::string encryptedPassword = field[2].asCString();

    account->AccountFlags = field[3].asUint8();
    account->Banned = field[4].asUint32();
    account->forcedLanguage = field[5].asCString();
    account->Muted = field[6].asUint32();

    if (static_cast<uint32_t>(UNIXTIME) > account->Banned && account->Banned != 0 && account->Banned != 1)
    {
        account->Banned = 0;
        sLogger.debug("Account {}'s ban has expired.", accountName);
        sLogonSQL->Execute("UPDATE accounts SET banned = 0 WHERE id = %u", account->AccountId);
    }
    
    if (account->forcedLanguage != "enUS")
        account->forcedLocale = true;
    else
        account->forcedLocale = false;

    if (static_cast<uint32_t>(UNIXTIME) > account->Muted && account->Muted != 0 && account->Muted != 1)
    {
        account->Muted = 0;
        sLogger.debug("Account {}'s mute has expired.", accountName);
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
        sLogger.failure("Account `{}` has incorrect number of bytes in encrypted password! Disabling.", accountName);
        memset(account->SrpHash, 0, 20);
    }

    AscEmu::Util::Strings::toUpperCase(accountName);

    _accountMap.insert_or_assign(accountName, std::move(account));
}

Account* AccountMgr::getAccountByName(std::string const& Name) const
{
    std::lock_guard lock(accountMgrMutex);

    auto pAccount = _getAccountByNameLockFree(Name);

    return pAccount;
}

void AccountMgr::updateAccount(Account* account, Field* field) const
{
    const uint32_t id = field[0].asUint32();
    std::string accountName = field[1].asCString();
    std::string encryptedPassword = field[2].asCString();

    if (id != account->AccountId)
    {
        sLogger.failure("AccountMgr : deleting duplicate account {} [{}]...", id, accountName);
        sLogonSQL->Execute("DELETE FROM accounts WHERE id = %u", id);
        return;
    }

    account->AccountFlags = field[3].asUint8();
    account->Banned = field[4].asUint32();
    account->forcedLanguage = field[5].asCString();
    account->Muted = field[6].asUint32();

    if (static_cast<uint32_t>(UNIXTIME) > account->Banned && account->Banned != 0 && account->Banned != 1)
    {
        account->Banned = 0;
        sLogger.debug("Account {}'s ban has expired.", accountName);
        sLogonSQL->Execute("UPDATE accounts SET banned = 0 WHERE id = %u", account->AccountId);
    }

    if (account->forcedLanguage != "enUS")
        account->forcedLocale = true;
    else
        account->forcedLocale = false;

    if (static_cast<uint32_t>(UNIXTIME) > account->Muted && account->Muted != 0 && account->Muted != 1)
    {
        account->Muted = 0;
        sLogger.debug("Account {}'s mute has expired.", accountName);
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
        sLogger.failure("Account `{}` has incorrect number of bytes in encrypted password! Disabling.", accountName);
        memset(account->SrpHash, 0, 20);
    }
}

void AccountMgr::reloadAccounts(bool silent)
{
    std::lock_guard lock(accountMgrMutex);

    if (!silent)
        sLogger.info("[AccountMgr] Reloading Accounts...");

    std::set<std::string> account_list;

    auto result = sLogonSQL->Query("SELECT id, acc_name, encrypted_password, flags, banned, forceLanguage, muted FROM accounts");
    if (result)
    {
        do
        {
            Field* field = result->Fetch();
            std::string accountName = field[1].asCString();

            AscEmu::Util::Strings::toUpperCase(accountName);

            const auto account = _getAccountByNameLockFree(accountName);
            if (account == nullptr)
                addAccount(field);
            else
                updateAccount(account, field);

            account_list.insert(accountName);

        } while (result->NextRow());
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
        sLogger.info("[AccountMgr] Found {} accounts.", static_cast<uint32_t>(_accountMap.size()));
}

size_t AccountMgr::getCount() const
{
    return _accountMap.size();
}

std::map<std::string, std::unique_ptr<Account>> const& AccountMgr::getAccountMap() const
{
    return _accountMap;
}

Account* AccountMgr::_getAccountByNameLockFree(std::string const& Name) const
{
    const auto itr = _accountMap.find(Name);
    if (itr == _accountMap.end())
        return nullptr;

    return itr->second.get();
}
