/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once

#include <cstdint>
#include <string_view>

namespace AscEmu::Version::Forever
{
    struct BuildProfile
    {
        uint32_t build;
        std::string_view name;
        bool supported;
    };

    // Verified Forever beta client profile. Protocol constants and opcodes stay
    // specific to Forever and must be filled only from Forever observations.
    inline constexpr BuildProfile ActiveBuild{70009U, "Forever 1.60.1", true};
    inline constexpr uint32_t Build = ActiveBuild.build;

    inline constexpr bool supportsBuild(uint32_t build)
    {
        return ActiveBuild.supported && (build == 69893U || build == 70009U);
    }
}
