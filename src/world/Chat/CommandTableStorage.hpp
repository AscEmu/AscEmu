/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <vector>

struct ChatCommand;

class CommandTableStorage
{
    CommandTableStorage();
    ~CommandTableStorage();

public:
    static CommandTableStorage& getInstance();

    CommandTableStorage(CommandTableStorage&&) = delete;
    CommandTableStorage(CommandTableStorage const&) = delete;
    CommandTableStorage& operator=(CommandTableStorage&&) = delete;
    CommandTableStorage& operator=(CommandTableStorage const&) = delete;

    void loadOverridePermission();
    void overridePermission(const char* command, const char* level);

    inline const std::vector<ChatCommand>& getCommandRegistry() const { return m_commandRegistry; }

private:

    std::vector<ChatCommand> m_commandRegistry;

};

#define sCommandTableStorage CommandTableStorage::getInstance()
