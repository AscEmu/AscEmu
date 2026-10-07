/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "ForeverSpellPacketUtils.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    struct SmsgSpellCooldownMap
    {
        uint32_t spellId;
        uint32_t duration;
    };

    class SmsgSpellCooldown : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t isGlobalCooldown;
        std::vector<SmsgSpellCooldownMap> spellMap;
        uint16_t mapId;

        SmsgSpellCooldown() : SmsgSpellCooldown(WoWGuid(), 0, {}, 0)
        {
        }

        SmsgSpellCooldown(WoWGuid guid, uint8_t isGlobalCooldown, std::vector<SmsgSpellCooldownMap> spellMap, uint16_t mapId = 0) :
            ManagedPacket(SMSG_SPELL_COOLDOWN, 16 + 1 + 4 + spellMap.size() * 12),
            guid(guid),
            isGlobalCooldown(isGlobalCooldown),
            spellMap(spellMap),
            mapId(mapId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                const WoWGuid modernCaster = ForeverSpellPacket::toModernGuid(guid, m_protocol.realmId, mapId);
                ForeverSpellPacket::writePackedGuid(packet, modernCaster);
                packet << uint8_t(isGlobalCooldown);
                packet << uint32_t(spellMap.size());

                for (auto const& cooldown : spellMap)
                    packet << uint32_t(cooldown.spellId) << uint32_t(cooldown.duration) << float(1.0f);

                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << guid;

                if (m_protocol.expansion > WoW::Expansion::_Classic)
                    packet << isGlobalCooldown;

                for (auto const& cooldowns : spellMap)
                    packet << cooldowns.spellId << cooldowns.duration;
            }
            else
            {
                packet.writeBit(guid[0]);
                packet.writeBit(guid[6]);
                packet.writeBit(isGlobalCooldown);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBits(spellMap.size(), 21);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[4]);
                packet.flushBits();

                for (auto const& cooldowns : spellMap)
                    packet << cooldowns.spellId << cooldowns.duration;

                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[6]);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
