/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

class ByteBuffer;

namespace AscEmu::Version::Forever::Fields { struct ObjectData; }

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeObjectDataCreate(ByteBuffer& data, Fields::ObjectData const& fields);
    void writeObjectDataUpdate(ByteBuffer& data, Fields::ObjectData const& fields);
}
