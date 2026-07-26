/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

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

        SmsgSpellCooldown() : SmsgSpellCooldown(WoWGuid(), 0, {})
        {
        }

        SmsgSpellCooldown(WoWGuid guid, uint8_t isGlobalCooldown, std::vector<SmsgSpellCooldownMap> spellMap) :
            ManagedPacket(SMSG_SPELL_COOLDOWN, 8 + 1 + spellMap.size() * 8),
            guid(guid),
            isGlobalCooldown(isGlobalCooldown),
            spellMap(spellMap)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
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
