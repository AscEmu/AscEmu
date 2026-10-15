/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgItemEnchantmentTimeUpdate : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint32_t slot;
        uint32_t duration;
        uint64_t playerGuid;

        SmsgItemEnchantmentTimeUpdate() : SmsgItemEnchantmentTimeUpdate(0, 0, 0, 0)
        {
        }

        SmsgItemEnchantmentTimeUpdate(uint64_t itemGuid, uint32_t slot, uint32_t duration, uint64_t playerGuid) :
            ManagedPacket(SMSG_ITEM_ENCHANT_TIME_UPDATE, 24),
            itemGuid(itemGuid),
            slot(slot),
            duration(duration),
            playerGuid(playerGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // item, remaining duration, slot, owner
                packet << WoWGuid(itemGuid).toGuid128(m_protocol.realmId, 0);
                packet << uint32_t(duration);
                packet << uint32_t(slot);
                packet << WoWGuid(playerGuid).toGuid128(m_protocol.realmId, 0);
                return true;
            }

            packet << itemGuid << slot << duration << playerGuid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
