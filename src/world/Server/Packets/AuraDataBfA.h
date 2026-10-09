/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/WorldPacket.hpp"
#include "Spell/SpellAuraDefines.hpp"
#include "WoWGuid.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // The aura of one slot in SMSG_AURA_UPDATE of 8.3.7 clients, shared by the single and the full update.
    // AuraUpdate: visualSlot, spellId, flags, level, stackCount, casterGuid, duration, timeLeft, effAmount[]
    template <typename AuraUpdate>
    void writeAuraSlotBfA(WorldPacket& packet, AuraUpdate const& aura, bool remove, uint32_t realmId, uint32_t mapId, uint32_t unitCounter)
    {
        packet << uint8_t(aura.visualSlot);
        packet.writeBit(!remove);
        packet.flushBits();

        if (remove)
            return;

        const bool hasCaster = !(aura.flags & AFLAG_IS_CASTER);
        const bool hasDuration = (aura.flags & AFLAG_DURATION) != 0;

        // the flags of 8.3.7: no caster, positive, duration, scalable, negative
        uint8_t auraFlags = 0;
        if (aura.flags & AFLAG_IS_CASTER)
            auraFlags |= 0x01;
        if (!(aura.flags & AFLAG_NEGATIVE))
            auraFlags |= 0x02;
        if (hasDuration)
            auraFlags |= 0x04;
        if (aura.flags & AFLAG_SEND_EFFECT_AMOUNT)
            auraFlags |= 0x08;
        if (aura.flags & AFLAG_NEGATIVE)
            auraFlags |= 0x10;

        // the effect amounts are indexed by effect, gaps are sent as zero
        uint8_t effectCount = 0;
        if (aura.flags & AFLAG_SEND_EFFECT_AMOUNT)
        {
            if (aura.flags & AFLAG_EFFECT_1)
                effectCount = 1;
            if (aura.flags & AFLAG_EFFECT_2)
                effectCount = 2;
            if (aura.flags & AFLAG_EFFECT_3)
                effectCount = 3;
        }

        // the cast the aura comes from: one per spell and slot of the unit
        packet << WoWGuid128::cast(realmId, mapId, aura.spellId, (uint64_t(unitCounter) << 8) | aura.visualSlot);
        packet << uint32_t(aura.spellId);
        packet << uint32_t(0);                                      // spell visual
        packet << uint8_t(auraFlags);
        packet << uint32_t(aura.flags & (AFLAG_EFFECT_1 | AFLAG_EFFECT_2 | AFLAG_EFFECT_3));
        packet << uint16_t(aura.level);
        packet << uint8_t(aura.stackCount != 0 ? aura.stackCount : 1);
        packet << int32_t(0);                                       // content tuning

        packet.writeBit(hasCaster);
        packet.writeBit(hasDuration);
        packet.writeBit(hasDuration);
        packet.writeBit(false);                                     // time modifier
        packet.writeBits(effectCount, 6);
        packet.writeBits(0, 6);                                     // estimated points
        packet.writeBit(false);                                     // content tuning data
        packet.flushBits();

        if (hasCaster)
            packet << aura.casterGuid.toGuid128(realmId, mapId);

        if (hasDuration)
        {
            packet << uint32_t(aura.duration);
            packet << uint32_t(aura.timeLeft);
        }

        for (uint8_t effect = 0; effect < effectCount; ++effect)
            packet << ((aura.flags & (AFLAG_EFFECT_1 << effect)) ? float(aura.effAmount[effect]) : 0.0f);
    }
}
