/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountCommand.hpp"
#include "Account/AccountCommandCreate.hpp"
#include "Account/AccountCommandSetGm.hpp"
#include "Account/AccountCommandMute.hpp"
#include "Account/AccountCommandUnmute.hpp"
#include "Account/AccountCommandBan.hpp"
#include "Account/AccountCommandUnban.hpp"
#include "Account/AccountCommandChangePw.hpp"
#include "Account/AccountCommandGetId.hpp"
#include "Server/WorldSession.h"

AccountCommand::AccountCommand()
{
    // Register subcommands like ".account create"
    registerSubCommand("create", std::make_unique<AccountCommandCreate>());
    registerSubCommand("setgm", std::make_unique<AccountCommandSetGm>());
    registerSubCommand("mute", std::make_unique<AccountCommandMute>());
    registerSubCommand("unmute", std::make_unique<AccountCommandUnmute>());
    registerSubCommand("ban", std::make_unique<AccountCommandBan>());
    registerSubCommand("unban", std::make_unique<AccountCommandUnban>());
    registerSubCommand("changepw", std::make_unique<AccountCommandChangePw>());
    registerSubCommand("getid", std::make_unique<AccountCommandGetId>());
}

bool AccountCommand::execute(const std::vector<std::string>& args, WorldSession* session)
{
    if (hasSubcommands() && args.empty())
    {
        // If no subcommand is provided, list available subcommands
        listAvailableSubCommands(session);
        return true;
    }

    // Extract the subcommand
    std::string subCommand = args[0];

    // Handle arguments after the subcommand
    std::vector<std::string> subCommandArgs(args.begin() + 1, args.end());

    auto it = subCommands_.find(subCommand);
    if (it != subCommands_.end())
    {
        ICommand* subCmd = it->second.get();

        // Check if the user has permission to execute the subcommand
        if (!session->hasPermission(subCmd->getRequiredPermission()))
        {
            session->systemMessage("You do not have permission to use this subcommand.");
            return false;
        }

        // Execute the subcommand with the provided arguments
        subCmd->execute(subCommandArgs, session);
        return true;
    }

    session->systemMessage("Unknown subcommand.");
    return false;
}

void AccountCommand::registerSubCommand(const std::string& name, std::unique_ptr<ICommand> command)
{
    subCommands_[name] = std::move(command);
}

void AccountCommand::listAvailableSubCommands(WorldSession* session) const
{
    session->systemMessage("Available subcommands:");

    for (const auto& pair : subCommands_)
    {
        const std::string& subCommandName = pair.first;
        const ICommand* subCommand = pair.second.get();

        // Only show subcommands that the user has permission to execute
        const std::string commandPermission(subCommand->getRequiredPermission());
        if (session->hasPermission(subCommand->getRequiredPermission()) || commandPermission == "0")
        {
            std::string outList = " - " + subCommandName + ": " + subCommand->getHelp();
            session->systemMessage(outList.c_str());
        }
    }
}

std::string AccountCommand::getHelp() const
{
    return "Usage: .account <subcommand>\nUse .account to see available subcommands.";
}

const char* AccountCommand::getRequiredPermission() const
{
    return "0";  // This is the permission required for the main ".account" command
}

bool AccountCommand::hasSubcommands() const
{
    return !subCommands_.empty();
}
