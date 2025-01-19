/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Chat/CommandInterface.hpp"

class AccountCommandGetId : public ICommand
{
public:
    bool execute(const std::vector<std::string>& args, WorldSession* session) override;
    std::string getHelp() const override;
    const char* getRequiredPermission() const override;
    size_t getArgumentCount() const override;
};
