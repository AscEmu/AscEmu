/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommandUnmute.hpp"

#include "Server/World.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

bool AccountCommandUnmute::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (args.size() != getArgumentCount())
    {
        session->systemMessage("Usage: .account unmute <username>");
        return false;
    }

    const std::string& username = args[0];

    sLogonCommHandler.setAccountMute(username.c_str(), 0);

    session->systemMessage("Account '{}' has been unmuted.", username);
    sGMLog.write(session, "unmuted account {}", username.data());

    WorldSession* targetSession = sWorld.getSessionByAccountName(username);
    if (targetSession)
    {
        targetSession->m_muted = 0;
        targetSession->systemMessage("Your voice has restored. You may speak again.");
    }

    return true;
}

std::string AccountCommandUnmute::getHelp() const
{
    return "Unmutes account <username>.";
}

const char* AccountCommandUnmute::getRequiredPermission() const
{
    return "a";
}

size_t AccountCommandUnmute::getArgumentCount() const
{
    return 1;
}
