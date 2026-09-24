/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "VersionDataBridge.hpp"

#include "MySQLDataStore.hpp"

namespace AscEmu::World::Storage
{
    CreatureProperties const* getCreaturePropertiesForVersionClient(uint32_t entry)
    {
        return sMySQLStore.getCreatureProperties(entry);
    }
}
