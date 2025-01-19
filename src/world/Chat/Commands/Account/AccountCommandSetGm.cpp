/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommandSetGm.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"

bool AccountCommandSetGm::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (args.size() != getArgumentCount())
    {
        session->systemMessage("Usage: .account setgm <accountname> <permission>");
        return false;
    }

    const std::string& username = args[0];
    const std::string& level = args[1];

    sLogonCommHandler.checkIfAccountExist(username.c_str(), session->GetAccountNameS(), level.c_str());

    return true;
}

std::string AccountCommandSetGm::getHelp() const
{
    return "Sets GM level on account. Pass it accountname and permissions e.g. 123gmotsaz";
}

const char* AccountCommandSetGm::getRequiredPermission() const
{
    return "z";
}

size_t AccountCommandSetGm::getArgumentCount() const
{
    return 2;
}
