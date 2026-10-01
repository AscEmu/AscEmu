/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DynamicObjectData.hpp"
#include "Definitions/DynamicObjectData.hpp"
#include "Trace.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeDynamicObjectDataUpdate(ByteBuffer& data, Fields::DynamicObjectData const& fields)
    {
        traceChangedFields<Definitions::DynamicObjectDataUpdate>("DynamicObjectData", fields);
        Definitions::DynamicObjectDataUpdate::writeUpdate(data, fields);
    }
}
