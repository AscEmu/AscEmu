/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommandCreate.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"

bool AccountCommandCreate::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (args.size() != getArgumentCount())
    {
        session->SystemMessage("Usage: .account create <name> <password>");
        return false;
    }

    const std::string& accountName = args[0];
    const std::string& password = args[1];

    sLogonCommHandler.createAccount(accountName.c_str(), password.c_str(), session->GetAccountNameS());

    return true;
}

std::string AccountCommandCreate::getHelp() const
{
    return "Create a new Account: .account create <name> <password>";
}

const char* AccountCommandCreate::getRequiredPermission() const
{
    return "a";
}

size_t AccountCommandCreate::getArgumentCount() const
{
    return 2;
}
