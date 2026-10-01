/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

class ByteBuffer;

namespace AscEmu::Version::Forever::Fields { struct UnitData; }

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeUnitDataCreate(ByteBuffer& data, Fields::UnitData const& fields, bool ownerVisible);
    void writeUnitDataUpdate(ByteBuffer& data, Fields::UnitData const& fields);
}
