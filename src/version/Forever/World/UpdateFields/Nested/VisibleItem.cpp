/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "VisibleItem.hpp"

#include "Network/ByteBuffer.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Nested
{
    void writeVisibleItemCreate(ByteBuffer& data, Fields::VisibleItem const& fields)
    {
        data << fields.itemId << fields.secondaryItemModifiedAppearanceId << fields.conditionalItemAppearanceId << fields.itemAppearanceModId << fields.itemVisual << fields.itemModifiedAppearanceId << fields.unknownVisibleItemField << fields.transmogSlotOption << fields.sheatheCategory;
        data.writeBit(fields.hasTransmog);
        data.writeBit(fields.hasIllusion);
        data.flushBits();
    }

    void writeVisibleItemUpdate(ByteBuffer& data, Fields::VisibleItem const& fields)
    {
        // Forever 1.60.1.70124 VisibleItem differential uses a 12-bit nested mask.
        // For AscEmu's current live equip path only ItemID is changed; send root + ItemID.
        data.writeBits(0x009U, 12);
        data.flushBits();
        data << fields.itemId;
    }
}
