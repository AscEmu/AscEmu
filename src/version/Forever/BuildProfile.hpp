/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace AscEmu::Version::Forever
{
    struct BuildProfile
    {
        uint32_t build;
        std::string_view name;
    };

    // Registered Forever builds share the current 1.60.1 transport. Build
    // authentication is independent and remains variant-specific in Auth.hpp.
    inline constexpr std::array<BuildProfile, 4> BuildProfiles =
    {{
        { 69893U, "Forever 1.60.1" },
        { 70009U, "Forever 1.60.1" },
        { 70124U, "Forever 1.60.1" },
        { 70205U, "Forever 1.60.1" },
    }};

    inline constexpr BuildProfile ActiveBuild{ 70009U, "Forever 1.60.1" };
    inline constexpr uint32_t Build = ActiveBuild.build;

    inline constexpr bool supportsBuild(uint32_t build)
    {
        for (const auto& profile : BuildProfiles)
        {
            if (profile.build == build)
                return true;
        }

        return false;
    }
}
