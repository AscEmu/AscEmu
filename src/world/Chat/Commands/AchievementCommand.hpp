/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Chat/CommandInterface.hpp"
#include <unordered_map>
#include <memory>
#include <vector>

class AchievementCommand : public ICommand
{
public:
    AchievementCommand();
    bool execute(const std::vector<std::string>& args, WorldSession* session) override;
    std::string getHelp() const override;
    const char* getRequiredPermission() const override;
    bool hasSubcommands() const override;

private:
    std::unordered_map<std::string, std::unique_ptr<ICommand>> subCommands_;
    void registerSubCommand(const std::string& name, std::unique_ptr<ICommand> command);
    void listAvailableSubCommands(WorldSession* session) const;
};
