/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "version/Forever/Fields/ForeverUpdateFields.hpp"

class ByteBuffer;

namespace AscEmu::Version::Forever::UpdateFields::Nested
{
    void writeVisibleItemCreate(ByteBuffer& data, Fields::VisibleItem const& fields);
    void writeVisibleItemUpdate(ByteBuffer& data, Fields::VisibleItem const& fields);
}
