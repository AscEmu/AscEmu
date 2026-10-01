/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CorpseData.hpp"
#include "Definitions/CorpseData.hpp"
#include "Trace.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    void writeCorpseDataUpdate(ByteBuffer& data, Fields::CorpseData const& fields)
    {
        traceChangedFields<Definitions::CorpseDataUpdate>("CorpseData", fields);
        Definitions::CorpseDataUpdate::writeUpdate(data, fields);
    }
}
