/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <numbers>

namespace AscEmu::Math
{
    inline constexpr double Pi = std::numbers::pi;
    inline constexpr double HalfPi = std::numbers::pi / 2.0;
    inline constexpr double QuarterPi = std::numbers::pi / 4.0;

    inline constexpr float PiF = std::numbers::pi_v<float>;
    inline constexpr float HalfPiF = std::numbers::pi_v<float> / 2.0f;
    inline constexpr float QuarterPiF = std::numbers::pi_v<float> / 4.0f;
}
