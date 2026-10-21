/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountService.hpp"

#include "Cryptography/BNetSrpV1.hpp"
#include "Database/Database.hpp"
#include "Database/Field.hpp"
#include "Server/AccountMgr.h"
#include "Server/LogonServerDefines.hpp"
#include "Server/Master.hpp"

#include <sstream>

namespace AscEmu::Accounts
{
    namespace
    {
        std::string fieldString(Field const& field)
        {
            return field.asCString() != nullptr ? field.asCString() : "";
        }

        // the classic credential: SHA1 of "NAME:PASSWORD" in upper case, hashed by the database
        std::string classicPasswordSource(const std::string& name, const std::string& password)
        {
            return sLogonSQL->escapeString(name + ":" + password);
        }

        bool getAccountId(const std::string& name, uint32_t& id)
        {
            id = 0;
            auto result = sLogonSQL->query("SELECT id FROM accounts WHERE UPPER(acc_name) = UPPER('%s') LIMIT 1", sLogonSQL->escapeString(name).c_str());
            if (!result)
                return false;

            id = result->fetch()[0].asUint32();
            return id != 0;
        }

        // the SRP credential of the Battle.net web login, the account name is its identity
        bool makeSrpCredential(const std::string& name, const std::string& password, std::string& saltHex, std::string& verifierHex)
        {
            AscEmu::Cryptography::BNetSrpV1::RegistrationData registration;
            if (!AscEmu::Cryptography::BNetSrpV1::makeRegistrationData(name, password, registration))
                return false;

            saltHex = registration.saltHex;
            verifierHex = registration.verifierHex;
            return true;
        }
    }

    const char* describe(Result result)
    {
        switch (result)
        {
            case Result::Ok: return "ok";
            case Result::Exists: return "an account with that name exists";
            case Result::NotFound: return "there is no account with that name";
            case Result::WrongPassword: return "the current password does not match";
            case Result::CredentialsError: return "the Battle.net credential could not be generated";
            case Result::DatabaseError: return "the database query failed";
        }

        return "unknown";
    }

    Result createAccount(const std::string& name, const std::string& password, const std::string& email)
    {
        if (!sLogonSQL)
            return Result::DatabaseError;

        uint32_t accountId = 0;
        if (getAccountId(name, accountId))
            return Result::Exists;

        std::string saltHex;
        std::string verifierHex;
        if (!makeSrpCredential(name, password, saltHex, verifierHex))
            return Result::CredentialsError;

        std::stringstream query;
        query << "INSERT INTO `accounts` (`acc_name`, `encrypted_password`, `srp_salt`, `srp_verifier`, `banned`, `email`, `flags`, `banreason`) VALUES ('"
              << sLogonSQL->escapeString(name) << "', SHA(UPPER('" << classicPasswordSource(name, password) << "')), '"
              << sLogonSQL->escapeString(saltHex) << "', '" << sLogonSQL->escapeString(verifierHex) << "', '0', '"
              << sLogonSQL->escapeString(email) << "', '" << NEW_ACCOUNT_EXPANSION_FLAGS << "', '')";
        if (!sLogonSQL->waitExecuteNA(query.str().c_str()))
            return Result::DatabaseError;

        sAccountMgr.reloadAccounts(true);
        return Result::Ok;
    }

    Result setPassword(const std::string& name, const std::string& password)
    {
        if (!sLogonSQL)
            return Result::DatabaseError;

        uint32_t accountId = 0;
        if (!getAccountId(name, accountId))
            return Result::NotFound;

        // the SRP identity is the stored spelling of the account name
        auto row = sLogonSQL->query("SELECT acc_name FROM accounts WHERE id = %u", accountId);
        if (!row)
            return Result::DatabaseError;
        const std::string storedName = fieldString(row->fetch()[0]);

        std::string saltHex;
        std::string verifierHex;
        if (!makeSrpCredential(storedName, password, saltHex, verifierHex))
            return Result::CredentialsError;

        std::stringstream query;
        query << "UPDATE `accounts` SET `encrypted_password` = SHA(UPPER('" << classicPasswordSource(storedName, password) << "')), `srp_salt` = '"
              << sLogonSQL->escapeString(saltHex) << "', `srp_verifier` = '" << sLogonSQL->escapeString(verifierHex) << "' WHERE `id` = " << accountId;
        if (!sLogonSQL->waitExecuteNA(query.str().c_str()))
            return Result::DatabaseError;

        sAccountMgr.reloadAccounts(true);
        return Result::Ok;
    }

    Result changePassword(const std::string& name, const std::string& oldPassword, const std::string& newPassword)
    {
        if (!sLogonSQL)
            return Result::DatabaseError;

        uint32_t accountId = 0;
        if (!getAccountId(name, accountId))
            return Result::NotFound;

        auto match = sLogonSQL->query("SELECT id FROM accounts WHERE id = %u AND encrypted_password = SHA(UPPER('%s'))", accountId, classicPasswordSource(name, oldPassword).c_str());
        if (!match)
            return Result::WrongPassword;

        return setPassword(name, newPassword);
    }

    Result deleteAccount(const std::string& name)
    {
        if (!sLogonSQL)
            return Result::DatabaseError;

        uint32_t accountId = 0;
        if (!getAccountId(name, accountId))
            return Result::NotFound;

        if (!sLogonSQL->waitExecute("DELETE FROM `accounts` WHERE `id` = %u", accountId))
            return Result::DatabaseError;

        sAccountMgr.reloadAccounts(true);
        return Result::Ok;
    }

    bool findBattleNetLogin(const std::string& login, BattleNetLogin& result)
    {
        if (!sLogonSQL || login.empty())
            return false;

        const std::string escapedLogin = sLogonSQL->escapeString(login);
        auto row = sLogonSQL->query("SELECT id, acc_name, email, srp_salt, srp_verifier FROM accounts WHERE UPPER(acc_name) = UPPER('%s') OR (email <> '' AND UPPER(email) = UPPER('%s')) ORDER BY UPPER(acc_name) = UPPER('%s') DESC LIMIT 1",
                                    escapedLogin.c_str(), escapedLogin.c_str(), escapedLogin.c_str());
        if (!row)
            return false;

        Field* fields = row->fetch();
        result.id = fields[0].asUint32();
        result.name = fieldString(fields[1]);
        result.email = fieldString(fields[2]);
        result.saltHex = fieldString(fields[3]);
        result.verifierHex = fieldString(fields[4]);
        return result.id != 0;
    }
}
