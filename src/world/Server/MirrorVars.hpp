/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/ClientProtocol.hpp"

#include <string>
#include <vector>

namespace AscEmu
{
    struct MirrorVar
    {
        std::string name;
        std::string value;
        bool update = false;
    };

    [[nodiscard]] const std::vector<MirrorVar>& getMirrorVars(const WoW::ClientProtocol& protocol);
}
