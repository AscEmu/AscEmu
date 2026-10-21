/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>

// The account operations of the logonserver, shared by the console and the LogonComm requests of the
// world servers. One account (accounts) serves every client version: the classic clients log in with the
// SHA1 credential, the clients from 6.x on with the SRP credential of the Battle.net web login. Both are
// derived from the same password, so the operations keep them in step.
namespace AscEmu::Accounts
{
    enum class Result
    {
        Ok,
        Exists,             // an account of that name exists
        NotFound,           // no account of that name
        WrongPassword,
        CredentialsError,   // the SRP credential could not be generated
        DatabaseError
    };

    [[nodiscard]] const char* describe(Result result);

    Result createAccount(const std::string& name, const std::string& password, const std::string& email);

    // Sets both credentials of the account.
    Result setPassword(const std::string& name, const std::string& password);

    // Verifies the current password, then sets the new one like setPassword.
    Result changePassword(const std::string& name, const std::string& oldPassword, const std::string& newPassword);

    Result deleteAccount(const std::string& name);

    // An account as the Battle.net login needs it
    struct BattleNetLogin
    {
        uint32_t id = 0;
        std::string name;           // acc_name, the SRP identity
        std::string email;
        std::string saltHex;        // empty until the password was set with this server version
        std::string verifierHex;
    };

    // Resolves the name a client typed into the web login: the account name or its email.
    bool findBattleNetLogin(const std::string& login, BattleNetLogin& result);
}
