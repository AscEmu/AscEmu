/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "ForeverSpellPacketUtils.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgItemCooldown : public ManagedPacket
    {
    public:
        WoWGuid itemGuid;
        uint32_t spellId = 0;
        uint32_t cooldown = 0;
        uint16_t mapId = 0;

        SmsgItemCooldown() : SmsgItemCooldown(WoWGuid(), 0, 0, 0)
        {
        }

        SmsgItemCooldown(WoWGuid itemGuid, uint32_t spellId, uint32_t cooldown, uint16_t mapId) :
            ManagedPacket(SMSG_ITEM_COOLDOWN, 24),
            itemGuid(itemGuid),
            spellId(spellId),
            cooldown(cooldown),
            mapId(mapId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.isForever() ? size_t(24) : m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            ForeverSpellPacket::writePackedGuid(packet, ForeverSpellPacket::toModernGuid(itemGuid, m_protocol.realmId, mapId));
            packet << uint32_t(spellId);
            packet << uint32_t(cooldown);
            return true;
        }

        bool internalDeserialise(WorldPacket&) override
        {
            return false;
        }
    };
}
