/*
Copyright(c) 2014 - 2026 AscEmu Team < http://www.ascemu.org>
This file is released under the MIT license.See README - MIT for more information.
*/

#pragma once

#include "mpqlib/CascStorage.hpp"

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace fs = std::filesystem;

class CascExtractor
{
public:
    // fullScan: scan the whole archive instead of the indexed table list of the build
    static bool run(mpqlib::CascStorage const& storage, std::string_view expansionName, uint32_t buildNumber, bool fullScan = false);
};
