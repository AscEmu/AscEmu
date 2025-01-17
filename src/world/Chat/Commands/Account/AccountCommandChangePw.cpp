/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommandChangePw.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"

bool AccountCommandChangePw::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (args.size() != getArgumentCount())
    {
        session->systemMessage("Usage: .account changepw <oldPassword> <newPassword> <newPassword>");
        return false;
    }

    const std::string& oldPassword = args[0];
    const std::string& newPassword1 = args[1];
    const std::string& newPassword2 = args[3];

    if (newPassword1 != newPassword2)
    {
        session->systemMessage("Your new passwords doesn't match!");
        return false;
    }

    sLogonCommHandler.changeAccountPassword(oldPassword.c_str(), newPassword1.c_str(), session->GetAccountNameS());

    return true;
}

std::string AccountCommandChangePw::getHelp() const
{
    return "Change the password of your account <oldPassword> <newPassword> <newPassword>";
}

const char* AccountCommandChangePw::getRequiredPermission() const
{
    return "0";
}

size_t AccountCommandChangePw::getArgumentCount() const
{
    return 3;
}
