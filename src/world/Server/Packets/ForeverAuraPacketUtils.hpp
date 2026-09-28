/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ForeverSpellPacketUtils.hpp"
#include "Spell/SpellAuraDefines.hpp"

#include <algorithm>
#include <cstdint>

namespace AscEmu::Packets::ForeverAuraPacket
{
    inline uint16_t makeAuraFlags(uint16_t legacyFlags)
    {
        uint16_t flags = 0;

        if (legacyFlags & AFLAG_IS_CASTER)
            flags |= 0x0001; // SELF_CAST
        if (legacyFlags & AFLAG_NEGATIVE)
            flags |= 0x0010; // NEGATIVE
        else
            flags |= 0x0002; // POSITIVE
        if (legacyFlags & AFLAG_DURATION)
            flags |= 0x0004; // DURATION
        if (legacyFlags & AFLAG_SEND_EFFECT_AMOUNT)
            flags |= 0x0008; // SCALABLE

        return flags;
    }

    inline uint8_t getPointCount(uint16_t legacyFlags)
    {
        if (!(legacyFlags & AFLAG_SEND_EFFECT_AMOUNT))
            return 0;

        uint8_t count = 0;
        if (legacyFlags & AFLAG_EFFECT_1)
            count = 1;
        if (legacyFlags & AFLAG_EFFECT_2)
            count = 2;
        if (legacyFlags & AFLAG_EFFECT_3)
            count = 3;

        return count;
    }

    template <typename AuraUpdate>
    inline void writeAuraData(WorldPacket& packet, AuraUpdate const& aura, uint32_t protocolRealmId, uint16_t mapId)
    {
        // Modern AuraData (verified against 1.60.1.70009 SMSG_AURA_UPDATE).
        const WoWGuid castId = WoWGuid::createModernWorldObject(ModernHighGuid::Cast, 3,::World::getInstance().settings.battleNetComm.realmId, mapId, 0, aura.spellId, aura.visualSlot);

        ForeverSpellPacket::writePackedGuid(packet, castId);
        packet << static_cast<int32_t>(aura.spellId);
        packet << static_cast<int32_t>(aura.spellXSpellVisualId);
        packet << static_cast<int32_t>(aura.scriptVisualId);
        packet << makeAuraFlags(aura.flags);
        packet << static_cast<uint32_t>(aura.flags & (AFLAG_EFFECT_1 | AFLAG_EFFECT_2 | AFLAG_EFFECT_3));
        packet << static_cast<uint16_t>(aura.level);
        packet << static_cast<uint8_t>(aura.stackCount);
        packet << static_cast<int32_t>(0); // ContentTuningID

        // DstLocation. AscEmu does not currently keep an aura destination here.
        packet << 0.0f << 0.0f << 0.0f;

        const bool hasCaster = !(aura.flags & AFLAG_IS_CASTER) && static_cast<bool>(aura.casterGuid);
        const bool hasDuration = (aura.flags & AFLAG_DURATION) != 0;
        const uint8_t pointCount = getPointCount(aura.flags);

        packet.writeBit(hasCaster);       // CastUnit
        packet.writeBit(false);           // CastItem
        packet.writeBit(hasDuration);     // Duration
        packet.writeBit(hasDuration);     // Remaining
        packet.writeBit(false);           // TimeMod
        packet.writeBits(pointCount, 6);  // Points
        packet.writeBits(0, 6);           // EstimatedPoints
        packet.writeBit(false);           // ContentTuning
        packet.flushBits();

        if (hasCaster)
            ForeverSpellPacket::writePackedGuid(packet, ForeverSpellPacket::toModernGuid(aura.casterGuid, protocolRealmId, mapId));

        if (hasDuration)
        {
            packet << static_cast<int32_t>(aura.duration);
            packet << static_cast<int32_t>(aura.timeLeft);
        }

        for (uint8_t i = 0; i < pointCount; ++i)
        {
            const uint16_t effectFlag = static_cast<uint16_t>(AFLAG_EFFECT_1 << i);
            packet << ((aura.flags & effectFlag) ? static_cast<float>(aura.effAmount[i]) : 0.0f);
        }
    }

    template <typename AuraContainer>
    inline void writeAuraUpdate(WorldPacket& packet, WoWGuid const& targetGuid, AuraContainer const& auras, bool updateAll, uint32_t protocolRealmId, uint16_t mapId)
    {
        packet.writeBit(updateAll);
        packet.writeBits(static_cast<uint32_t>(auras.size()), 9);
        packet.flushBits();

        ForeverSpellPacket::writePackedGuid(packet, ForeverSpellPacket::toModernGuid(targetGuid, protocolRealmId, mapId));

        for (auto const& aura : auras)
        {
            packet << static_cast<uint16_t>(aura.visualSlot);
            packet.writeBit(!aura.remove);
            packet.flushBits();

            if (!aura.remove)
                writeAuraData(packet, aura, protocolRealmId, mapId);
        }
    }
}
