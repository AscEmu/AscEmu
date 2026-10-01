/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "Network/ByteBuffer.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeItemDataCreate(ByteBuffer& data, Fields::ItemData const& fields, int32_t itemEntry);
    void writeItemDataUpdate(ByteBuffer& data, Fields::ItemData const& fields);
}
