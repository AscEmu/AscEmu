/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommandUnban.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

bool AccountCommandUnban::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (args.size() != getArgumentCount())
    {
        session->systemMessage("Usage: .account unban <name>");
        return false;
    }

    const std::string& accountName = args[0];

    sLogonCommHandler.setAccountBanned(accountName.c_str(), 0, "");
    session->systemMessage("Account '{}' has been unbanned. This change will be effective immediately.", accountName);

    sGMLog.write(session, "unbanned account {}", accountName.data());
    return true;
}

std::string AccountCommandUnban::getHelp() const
{
    return "Unbans account <name>.";
}

const char* AccountCommandUnban::getRequiredPermission() const
{
    return "z";
}

size_t AccountCommandUnban::getArgumentCount() const
{
    return 1;
}
