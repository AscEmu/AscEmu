/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

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
#if VERSION_STRING < Mop

            packet << guid;
#if VERSION_STRING > Classic
            packet << isGlobalCooldown;
#endif
            for (auto const& cooldowns : spellMap)
                packet << cooldowns.spellId << cooldowns.duration;
#else

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

            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[6]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
