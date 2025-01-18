/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementCommand.hpp"
#include "Server/WorldSession.h"

#if VERSION_STRING > TBC
#include "Achievement/AchievementCommandComplete.hpp"
#include "Achievement/AchievementCommandCriteria.hpp"
#include "Achievement/AchievementCommandReset.hpp"
#endif

AchievementCommand::AchievementCommand()
{
#if VERSION_STRING > TBC
    registerSubCommand("complete", std::make_unique<AchievementCommandComplete>());
    registerSubCommand("criteria", std::make_unique<AchievementCommandCriteria>());
    registerSubCommand("reset", std::make_unique<AchievementCommandReset>());
#endif
}

bool AchievementCommand::execute(const std::vector<std::string>& args, WorldSession* session)
{
#if VERSION_STRING > TBC
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
#else
    session->systemMessage("This command is available for version >= WotLK");
    return false;
#endif
}

void AchievementCommand::registerSubCommand(const std::string& name, std::unique_ptr<ICommand> command)
{
    subCommands_[name] = std::move(command);
}

void AchievementCommand::listAvailableSubCommands(WorldSession* session) const
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

std::string AchievementCommand::getHelp() const
{
    return "Usage: .achieve <subcommand>\nUse .achievement to see available subcommands.";
}

const char* AchievementCommand::getRequiredPermission() const
{
    return "m";
}

bool AchievementCommand::hasSubcommands() const
{
    return !subCommands_.empty();
}
