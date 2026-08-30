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

} // namespace cp
