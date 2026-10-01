/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectData.hpp"
#include "Definitions/ObjectData.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeObjectDataCreate(ByteBuffer& data, Fields::ObjectData const& fields) { Definitions::ObjectDataCreate::write(data, fields); }
    void writeObjectDataUpdate(ByteBuffer& data, Fields::ObjectData const& fields) { Definitions::ObjectDataUpdate::writeUpdate(data, fields); }
}
