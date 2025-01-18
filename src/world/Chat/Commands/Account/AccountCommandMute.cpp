/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommandMute.hpp"
#include "Server/World.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Util.hpp"

bool AccountCommandMute::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (args.size() != getArgumentCount())
    {
        session->systemMessage("Usage: .account mute <username> <timeperiod>");
        return false;
    }

    const std::string& username = args[0];
    const std::string& stringTimeperiod = args[1];
    const uint32_t timeperiod = Util::stringToUint32(stringTimeperiod, false);
    if (timeperiod == 0)
        return false;

    uint32_t banned = static_cast<uint32_t>(UNIXTIME) + timeperiod;

    sLogonCommHandler.setAccountMute(username.c_str(), banned);

    const std::string bannedString = Util::GetDateTimeStringFromTimeStamp(banned);
    session->systemMessage("Account '{}}' has been muted until {}}. The change will be effective immediately.",
        username, bannedString);

    sGMLog.write(session, "mutex account {} until {}", username.data(), bannedString.data());

    WorldSession* pSession = sWorld.getSessionByAccountName(username);
    if (pSession != nullptr)
    {
        pSession->m_muted = banned;
        pSession->systemMessage("Your voice has been muted until {} by a GM. Until this time, you will not be able to speak in any form.", bannedString);
    }

    return true;
}

std::string AccountCommandMute::getHelp() const
{
    return "Mutes account for <timeperiod>.";
}

const char* AccountCommandMute::getRequiredPermission() const
{
    return "a";  // This subcommand requires "gm" permission
}

size_t AccountCommandMute::getArgumentCount() const
{
    return 2;  // This command requires 2 arguments (name and password)
}
