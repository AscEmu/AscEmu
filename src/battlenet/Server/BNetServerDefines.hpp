/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Config/Config.hpp"

namespace AscEmu::Battlenet
{
    class ConfigManager
    {
    public:
        ConfigFile mainConfig;
    };

    inline ConfigManager configManager;
}
