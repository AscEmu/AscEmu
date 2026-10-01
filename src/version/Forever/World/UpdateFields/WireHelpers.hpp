/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/ByteBuffer.hpp"
#include "WoWGuid.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Version::Forever::UpdateFields
{
    inline void writeModernGuid(ByteBuffer& data, WoWGuid const& guid)
    {
        const std::vector<uint8_t> packed = guid.packModern();
        data.append(packed.data(), packed.size());
    }

    inline void writeSpellCastVisualCreate(ByteBuffer& data, Fields::SpellCastVisual const& fields)
    {
        data << fields.spellXSpellVisualId << fields.scriptVisualId;
    }

    inline void writeUnitChannelCreate(ByteBuffer& data, Fields::UnitChannel const& fields)
    {
        data << fields.spellId;
        writeSpellCastVisualCreate(data, fields.spellVisual);
        data << fields.startTimeMs << fields.duration;
    }
}
