/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DynamicObjectData.hpp"
#include "Definitions/DynamicObjectData.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeDynamicObjectDataUpdate(ByteBuffer& data, Fields::DynamicObjectData const& fields)
    {
        Definitions::DynamicObjectDataUpdate::writeUpdate(data, fields);
    }
}
