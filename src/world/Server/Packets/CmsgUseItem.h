/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
        uint64_t itemGuid;
#if VERSION_STRING == TBC
        uint8_t spellIndex;
#else
        uint32_t spellId;
        uint32_t glyphIndex;
        uint8_t castFlags;
#endif

        CmsgUseItem() : CmsgUseItem
#if VERSION_STRING == TBC
        (0, 0, 0, 0, 0)
#else
        (0, 0, 0, 0, 0, 0, 0)
#endif
        {
        }

        CmsgUseItem(uint8_t containerIndex, uint8_t inventorySlot,
#if VERSION_STRING == TBC
            uint8_t spellIndex, uint8_t castCount, uint64_t itemGuid) :
            ManagedPacket(CMSG_USE_ITEM, 12),
            containerIndex(containerIndex),
            inventorySlot(inventorySlot),
            castCount(castCount),
            itemGuid(itemGuid),
            spellIndex(spellIndex)
#else
            uint8_t castCount, uint32_t spellId, uint64_t itemGuid, uint32_t glyphIndex, uint8_t castFlags) :
            ManagedPacket(CMSG_USE_ITEM, 20),
            containerIndex(containerIndex),
            inventorySlot(inventorySlot),
            castCount(castCount),
            itemGuid(itemGuid),
            spellId(spellId),
            glyphIndex(glyphIndex),
            castFlags(castFlags)
#endif
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
            packet >> containerIndex >> inventorySlot >> spellIndex >> castCount >> itemGuid;
#else
            packet >> containerIndex >> inventorySlot >> castCount >> spellId >> itemGuid >> glyphIndex >> castFlags;
#endif
            return true;
        }
    };
}
