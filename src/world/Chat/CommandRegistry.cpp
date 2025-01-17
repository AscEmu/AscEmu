/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CommandRegistry.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include "CommandInterface.hpp"
#include "Server/WorldSession.h"
#include "CommandOverrides.hpp"
#include "Logging/Logger.hpp"


// Register a main command
void CommandRegistry::registerCommand(const std::string& name, CommandPtr command)
{
    commands_[name] = std::move(command);
}

// Load command overrides
void CommandRegistry::loadOverrides()
{
    overrides.loadOverrides();
    sLogger.info("Loaded {} command overrides into new command system.", overrides.getSize());
}

// Execute a command by name, after checking permissions and parsing the fullCommand
bool CommandRegistry::executeCommand(const std::string& fullCommand, WorldSession* session)
{
    std::istringstream iss(fullCommand);
    std::string commandName;

    // Extract the first word as the main command
    iss >> commandName;

    auto it = commands_.find(commandName);
    if (it != commands_.end())
    {
        ICommand* command = it->second.get();

        // Check for command permission override
        const std::string* overridePermission = overrides.getOverride(commandName);
        if (overridePermission)
        {
            // Check if the user has the overridden permission is it is not 0
            if (overridePermission[0] != "0")
            {
                if (!session->hasPermission(overridePermission->c_str()))
                {
                    session->systemMessage("You do not have permission to use this command.");
                    return false;
                }
            }
        }
        else
        {
            // Check if the user has the default permission for the command if it is not 0
            const std::string commandPermission(command->getRequiredPermission());
            if (commandPermission != "0")
            {
                if (!session->hasPermission(commandPermission.c_str()))
                {
                    session->systemMessage("You do not have permission to use this command.");
                    return false;
                }
            }
        }

        // Collect all remaining parts (subcommands and arguments)
        std::vector<std::string> args;
        std::string token;
        while (iss >> token)
            args.push_back(token);

        // Check if the command has subcommands
        if (command->hasSubcommands())
        {
            // Pass the remaining part as arguments, where the first one is the subcommand
            return command->execute(args, session);
        }

        // Check if the command expects arguments
        if (args.size() < command->getArgumentCount())
        {
            session->systemMessage("Incorrect number of arguments. Usage: {} ", command->getHelp());
            return false;
        }

        // If no subcommands, execute the command with parsed arguments (if required)
        return command->execute(args, session);
    }

    return false;  // Command not found
}
