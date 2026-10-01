/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ContainerData.hpp"
#include "Definitions/ContainerData.hpp"
#include "Trace.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeContainerDataUpdate(ByteBuffer& data, Fields::ContainerData const& fields)
    {
        traceChangedFields<Definitions::ContainerDataUpdate>("ContainerData", fields);
        Definitions::ContainerDataUpdate::writeUpdate(data, fields);
    }
}
