/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ContainerData.hpp"
#include "Definitions/ContainerData.hpp"
#include "WireHelpers.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeContainerDataCreate(ByteBuffer& data, Fields::ContainerData const& fields)
    {
        // Modern ContainerData create layout: all 98 slot GUIDs followed by NumSlots.
        // The field order is taken from the same modern reference used for the
        // existing ContainerData VALUES descriptor.
        for (WoWGuid const& slot : fields.slots)
            writeModernGuid(data, slot);

        data << fields.numSlots;
    }

    void writeContainerDataUpdate(ByteBuffer& data, Fields::ContainerData const& fields)
    {
        Definitions::ContainerDataUpdate::writeUpdate(data, fields);
    }
}
