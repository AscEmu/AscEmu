/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommandGetId.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

bool AccountCommandGetId::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (args.size() != getArgumentCount())
    {
        session->systemMessage("Usage: .account getid <name>");
        return false;
    }

    const std::string& accountName = args[0];

    sLogonCommHandler.checkIfAccountExist(accountName.c_str(), session->GetAccountNameS(), nullptr, 2);

    sGMLog.write(session, "looked up account id for account {}", accountName.data());

    return true;
}

std::string AccountCommandGetId::getHelp() const
{
    return "Get Account ID for account name <name>.";
}

const char* AccountCommandGetId::getRequiredPermission() const
{
    return "1";
}

size_t AccountCommandGetId::getArgumentCount() const
{
    return 1;
}
