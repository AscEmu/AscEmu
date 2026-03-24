/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"
#include "Spell/SpellCastTargets.hpp"

namespace AscEmu::Packets
{
    class CmsgUseItem : public ManagedPacket
    {
    public:
        uint8_t containerIndex;     // since 5875
        uint8_t inventorySlot;      // since 5875
#if VERSION_STRING <= TBC
        uint8_t spellIndex = 0;     // only for 5875 and 8606
#endif
        uint32_t spellId = 0;       // since 8606
        uint8_t castCount = 0;      // since 8606

        uint64_t itemGuidRaw = 0;   // just a helper
        WoWGuid itemGuid;           // since 12340

        uint32_t glyphIndex = 0;    // since 12340
        uint8_t castFlags = 0;      // since 12340

        SpellCastTargets targets;

        bool hasAdditionalData = false;

        float projectilePitch = 0.0f;
        float projectileSpeed = 0.0f;

        bool hasMovementData = false;

        CmsgUseItem() : CmsgUseItem(0, 0)
        {
        }

        CmsgUseItem(uint8_t containerIndex, uint8_t inventorySlot) :
            ManagedPacket(CMSG_USE_ITEM, 12),
            containerIndex(containerIndex),
            inventorySlot(inventorySlot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
#if VERSION_STRING == Classic
            packet >> containerIndex >> inventorySlot >> spellIndex;
#endif
#if VERSION_STRING == TBC
            packet >> containerIndex >> inventorySlot >> spellIndex >> castCount >> itemGuidRaw;
            itemGuid.init(itemGuidRaw);
#endif
#if VERSION_STRING == WotLK
            packet >> containerIndex >> inventorySlot >> castCount >> spellId >> itemGuidRaw >> glyphIndex >> castFlags;
            itemGuid.init(itemGuidRaw);
#endif
#if VERSION_STRING == Cata
            packet >> containerIndex >> inventorySlot >> castCount >> spellId >> itemGuidRaw >> glyphIndex >> castFlags;
            itemGuid.init(itemGuidRaw);
#endif
            targets.read(packet);

            if (castFlags & 0x02)
            {
                hasAdditionalData = true;
                packet >> projectilePitch >> projectileSpeed >> hasMovementData;
            }

#else // Mop
            packet >> containerIndex >> inventorySlot >> castCount >> spellId >> itemGuidRaw >> glyphIndex >> castFlags;
            itemGuid.init(itemGuidRaw);
#endif
            return true;
        }
    };
}
