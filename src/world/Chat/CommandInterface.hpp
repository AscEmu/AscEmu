/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include <vector>

class WorldSession;

class ICommand
{
public:
    virtual ~ICommand() = default;

    // Executes the command
    virtual bool execute(const std::vector<std::string>& args, WorldSession* session) = 0;

    // Returns the help message for the command
    virtual std::string getHelp() const = 0;

    // Returns the required permission string for this command
    virtual const char* getRequiredPermission() const = 0;

    // Returns the expected number of arguments for this command
    virtual size_t getArgumentCount() const { return 0; }

    // Returns true if this command has subcommands
    virtual bool hasSubcommands() const { return false; }
};
