/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <unordered_map>
#include <string>

class CommandOverrides
{
public:
    void loadOverrides();  // Function to load overrides from MySQL
    const std::string* getOverride(const std::string& command) const;  // Get the override permission for a command
    size_t getSize() const;
private:
    std::unordered_map<std::string, std::string> overrides;
};
