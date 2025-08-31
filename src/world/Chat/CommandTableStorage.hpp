/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

struct ChatCommandNEW;

class SERVER_DECL CommandTableStorage
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

    inline const std::vector<ChatCommandNEW>& Get() const { return m_commandRegistry; }

private:

    std::vector<ChatCommandNEW> m_commandRegistry;

};

#define sCommandTableStorage CommandTableStorage::getInstance()
