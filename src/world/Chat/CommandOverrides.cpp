/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CommandOverrides.hpp"

#include "Server/DatabaseDefinition.hpp"

void CommandOverrides::loadOverrides()
{
    overrides.clear();

    auto result = CharacterDatabase.Query("SELECT command_name, access_level FROM command_overrides");
    if (!result)
        return;

    while (result->NextRow())
    {
        std::string command(result->Fetch()[0].asCString());
        std::string permission(result->Fetch()[1].asCString());
        
        overrides[command] = permission;  // Store in the map
    }
}

const std::string* CommandOverrides::getOverride(const std::string& command) const
{
    auto it = overrides.find(command);
    if (it != overrides.end())
        return &it->second;  // Return the override permission

    return nullptr;  // No override found
}

size_t CommandOverrides::getSize() const
{
    return overrides.size();
}
