/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace AscEmu::World::Storage
{
    struct ForeverTactKeyRecord
    {
        uint32_t id{0};
        std::array<uint8_t, 16> key{};
        int32_t verifiedBuild{0};
    };

    std::vector<ForeverTactKeyRecord> loadForeverTactKeys(uint32_t maxBuild);
}
