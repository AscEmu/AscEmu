/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "BinaryTypes.hpp"
#include <span>
#include <cstdint>

namespace cp
{

BinaryType getBinaryType(std::span<const std::uint8_t> _data);

// build number of a client binary from the version of its embedded manifest, 0 when there is none
uint32_t getBuildNumber(std::span<const std::uint8_t> _data);

} // namespace cp
