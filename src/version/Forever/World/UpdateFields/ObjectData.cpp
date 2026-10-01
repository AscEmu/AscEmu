/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectData.hpp"
#include "Definitions/ObjectData.hpp"
#include "Trace.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeObjectDataCreate(ByteBuffer& data, Fields::ObjectData const& fields) { Definitions::ObjectDataCreate::write(data, fields); }
    void writeObjectDataUpdate(ByteBuffer& data, Fields::ObjectData const& fields)
    {
        if (fields.changes.test(Fields::ObjectData::DynamicFlagsBit))
            sLogger.info("[ForeverDebug][LootDeath][ObjectData] changes=0x{:X} dynamicFlags=0x{:08X}", fields.changes.to_ulong(), fields.dynamicFlags);
        traceChangedFields<Definitions::ObjectDataUpdate>("ObjectData", fields);
        Definitions::ObjectDataUpdate::writeUpdate(data, fields);
    }
}
