/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgUseItem : public ManagedPacket
    {
    public:
        uint8_t containerIndex;
        uint8_t inventorySlot;
        uint8_t castCount;
        uint32_t spellId;
        uint64_t itemGuid;
        uint32_t glyphIndex;
        uint8_t castFlags;

        CmsgUseItem() : CmsgUseItem(0, 0, 0, 0, 0, 0, 0)
        {
        }

        CmsgUseItem(uint8_t containerIndex, uint8_t inventorySlot, uint8_t castCount, uint32_t spellId,
            uint64_t itemGuid, uint32_t glyphIndex, uint8_t castFlags) :
            ManagedPacket(CMSG_USE_ITEM, 12),
            containerIndex(containerIndex),
            inventorySlot(inventorySlot),
            castCount(castCount),
            spellId(spellId),
            itemGuid(itemGuid),
            glyphIndex(glyphIndex),
            castFlags(castFlags)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING == TBC
            uint8_t temp;
            packet >> containerIndex >> inventorySlot >> temp >> castCount >> itemGuid;
            return true;
#else
            packet >> containerIndex >> inventorySlot >> castCount >> spellId >> itemGuid >> glyphIndex >> castFlags;
            return true;
#endif
        }
    };
}
