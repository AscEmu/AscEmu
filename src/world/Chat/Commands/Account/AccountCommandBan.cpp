/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommandBan.hpp"

#include "Server/World.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Util.hpp"

bool AccountCommandBan::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (args.size() < getArgumentCount())
    {
        session->systemMessage("Usage: .account ban <name> [duration] [reason]");
        return false;
    }

    const std::string& accountName = args[0];
    const std::string& duration = (args.size() >= 2) ? args[1] : "31556926"; //one year in seconds
    const std::string& reason = (args.size() >= 3) ? args[2] : "No reason specified";

    uint32_t timeperiod = Util::stringToUint32(duration, false);

    uint32_t bannedUntil = static_cast<uint32_t>(UNIXTIME) + timeperiod;

    sLogonCommHandler.setAccountBanned(accountName.c_str(), bannedUntil, reason.c_str());

    session->systemMessage("Account '{}' has been banned until {} for reason : {}.", accountName, Util::GetDateTimeStringFromTimeStamp(bannedUntil), reason);

    sWorld.disconnectSessionByAccountName(accountName, session);
    sGMLog.write(session, "banned account {} until {}", accountName.data(), Util::GetDateTimeStringFromTimeStamp(bannedUntil).data());

    session->systemMessage("Account banned successfully.");

    return true;
}

std::string AccountCommandBan::getHelp() const
{
    return "Bans account: .ban account <name> [duration] [reason].";
}

const char* AccountCommandBan::getRequiredPermission() const
{
    return "a";
}

size_t AccountCommandBan::getArgumentCount() const
{
    return 1;
}
