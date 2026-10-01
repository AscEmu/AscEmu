/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/ByteBuffer.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeGameObjectDataCreate(ByteBuffer& data, Fields::GameObjectData const& fields);
    void writeGameObjectDataUpdate(ByteBuffer& data, Fields::GameObjectData const& fields);
}
