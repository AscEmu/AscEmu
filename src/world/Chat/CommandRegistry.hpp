/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>
#include "CommandInterface.hpp"
#include "CommandOverrides.hpp"

class CommandRegistry
{
public:
    using CommandPtr = std::unique_ptr<ICommand>;

    static CommandRegistry& getInstance()
    {
        static CommandRegistry instance;
        return instance;
    }

    // Register a main command
    void registerCommand(const std::string& name, CommandPtr command);

    // Load command overrides
    void loadOverrides();

    // Execute a command by name, after checking permissions and parsing the fullCommand
    bool executeCommand(const std::string& fullCommand, WorldSession* session);

private:
    std::unordered_map<std::string, CommandPtr> commands_;
    CommandOverrides overrides;  // Add the command overrides handler
};
