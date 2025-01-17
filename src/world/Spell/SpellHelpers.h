/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace AscEmu::World::Spell::Helpers
{
    inline uint32_t decimalToMask(uint32_t dec) { return (static_cast<uint32_t>(1) << (dec - 1)); }
}
