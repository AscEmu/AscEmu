/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgEnchantmentLog : public ManagedPacket
    {
    public:
        uint64_t targetGuid;
        uint64_t casterGuid;
        uint32_t itemId;
        uint32_t enchantmentId;

        // 7.x and later: the enchanted item and its enchantment slot
        uint64_t itemGuid = 0;
        uint32_t enchantSlot = 0;

        SmsgEnchantmentLog() : SmsgEnchantmentLog(0, 0, 0, 0)
        {
        }

        SmsgEnchantmentLog(uint64_t targetGuid, uint64_t casterGuid, uint32_t itemId, uint32_t enchantmentId) :
            ManagedPacket(SMSG_ENCHANTMENTLOG, 24),
            targetGuid(targetGuid),
            casterGuid(casterGuid),
            itemId(itemId),
            enchantmentId(enchantmentId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 7.x: caster before owner, 9.x and later: owner before caster; item, its entry, enchantment, slot
                if (m_protocol.isLegion())
                {
                    packet << WoWGuid(casterGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                    packet << WoWGuid(targetGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                }
                else
                {
                    packet << WoWGuid(targetGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                    packet << WoWGuid(casterGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                }
                packet << WoWGuid(itemGuid).toGuid128(m_protocol.realmId, 0);
                packet << int32_t(itemId);
                packet << int32_t(enchantmentId);
                packet << int32_t(enchantSlot);
                return true;
            }

            // 6.x and 8.x: no known layout
            if (m_protocol.isWoD() || m_protocol.isBfA())
                return false;

            packet << targetGuid << casterGuid << itemId << enchantmentId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
