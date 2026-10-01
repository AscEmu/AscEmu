/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "version/Forever/Fields/ForeverUpdateFields.hpp"
#include "Network/ByteBuffer.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeDynamicObjectDataUpdate(ByteBuffer& data, Fields::DynamicObjectData const& fields);
}
