/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Spell/SpellAuraDefines.hpp"

namespace AscEmu::Packets
{
    class SmsgAuraUpdate : public ManagedPacket
    {
    public:
        WoWGuid guid;
        bool remove;

        struct AuraUpdate
        {
            uint8_t visualSlot = 0;
            uint32_t spellId = 0;
            uint16_t flags = 0;
            uint8_t level = 0;
            uint8_t stackCount = 0;
            WoWGuid casterGuid{};
            uint32_t duration = 0;
            uint32_t timeLeft = 0;
            int32_t effAmount[5] = {0}; // 3 spell effects up till cata, 5 in mop
        };

        AuraUpdate aura_updates;

        SmsgAuraUpdate() : SmsgAuraUpdate(WoWGuid(), {}, false)
        {
        }

        SmsgAuraUpdate(WoWGuid guid, AuraUpdate aura_updates, bool remove = false) :
            ManagedPacket(SMSG_AURA_UPDATE, 30),
            guid(guid),
            remove(remove),
            aura_updates(aura_updates)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_TBC)
                return false;

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << guid;
                packet << aura_updates.visualSlot;

                if (remove)
                {
                    packet << uint32_t(0);
                }
                else
                {
                    packet << aura_updates.spellId;

                    if (m_protocol.expansion < WoW::Expansion::_Cata)
                        packet << static_cast<uint8_t>(aura_updates.flags);
                    else
                        packet << aura_updates.flags;

                    packet << aura_updates.level;
                    packet << aura_updates.stackCount;

                    if (!(aura_updates.flags & AFLAG_IS_CASTER))
                        packet << aura_updates.casterGuid;

                    if (aura_updates.flags & AFLAG_DURATION)
                    {
                        packet << aura_updates.duration;
                        packet << aura_updates.timeLeft;
                    }

                    if (aura_updates.flags & AFLAG_SEND_EFFECT_AMOUNT)
                    {
                        if (aura_updates.flags & AFLAG_EFFECT_1)
                            packet << aura_updates.effAmount[0];

                        if (aura_updates.flags & AFLAG_EFFECT_2)
                            packet << aura_updates.effAmount[1];

                        if (aura_updates.flags & AFLAG_EFFECT_3)
                            packet << aura_updates.effAmount[2];
                    }
                }
            }
            else // Mop
            {
                WoWGuid targetGuid = guid.getRawGuid();

                packet.writeBit(targetGuid[7]);
                packet.writeBit(0);
                packet.writeBits(1, 24);
                packet.writeBit(targetGuid[6]);
                packet.writeBit(targetGuid[1]);
                packet.writeBit(targetGuid[3]);
                packet.writeBit(targetGuid[0]);
                packet.writeBit(targetGuid[4]);
                packet.writeBit(targetGuid[2]);
                packet.writeBit(targetGuid[5]);
                packet.writeBit(!remove);

                uint8_t effCount = 0;
                if (!remove)
                {
                    if (aura_updates.flags & AFLAG_SEND_EFFECT_AMOUNT)
                    {
                        // Client indexes effects by slot: count must be (highest active effect index + 1),
                        // zero-padding gaps, not the number of active effect flags.
                        if (aura_updates.flags & AFLAG_EFFECT_1)
                            effCount = 1;

                        if (aura_updates.flags & AFLAG_EFFECT_2)
                            effCount = 2;

                        if (aura_updates.flags & AFLAG_EFFECT_3)
                            effCount = 3;

                        packet.writeBits(effCount, 22);
                    }
                    else
                    {
                        packet.writeBits(0, 22);
                    }

                    packet.writeBit(!(aura_updates.flags & AFLAG_IS_CASTER));

                    if (!(aura_updates.flags & AFLAG_IS_CASTER))
                    {
                        WoWGuid casterGuid = aura_updates.casterGuid.getRawGuid();
                        packet.writeBit(casterGuid[3]);
                        packet.writeBit(casterGuid[4]);
                        packet.writeBit(casterGuid[6]);
                        packet.writeBit(casterGuid[1]);
                        packet.writeBit(casterGuid[5]);
                        packet.writeBit(casterGuid[2]);
                        packet.writeBit(casterGuid[0]);
                        packet.writeBit(casterGuid[7]);
                    }

                    packet.writeBits(0, 22);
                    packet.writeBit(aura_updates.flags & AFLAG_DURATION);
                    packet.writeBit(aura_updates.flags & AFLAG_DURATION);
                }

                packet.flushBits();

                if (!remove)
                {
                    if (!(aura_updates.flags & AFLAG_IS_CASTER))
                    {
                        WoWGuid casterGuid = aura_updates.casterGuid.getRawGuid();
                        packet.writeByteSeq(casterGuid[3]);
                        packet.writeByteSeq(casterGuid[2]);
                        packet.writeByteSeq(casterGuid[1]);
                        packet.writeByteSeq(casterGuid[6]);
                        packet.writeByteSeq(casterGuid[4]);
                        packet.writeByteSeq(casterGuid[0]);
                        packet.writeByteSeq(casterGuid[5]);
                        packet.writeByteSeq(casterGuid[7]);
                    }

                    // Mop completely renumbered AURA_FLAGS on the wire (dropped the WotLK/Cata
                    // per-effect-index bits, added a dedicated "positive" bit)
                    uint8_t mopFlags = 0;
                    if (aura_updates.flags & AFLAG_IS_CASTER)
                        mopFlags |= 0x01; // AFLAG_CASTER
                    if (!(aura_updates.flags & AFLAG_NEGATIVE))
                        mopFlags |= 0x02; // AFLAG_POSITIVE
                    if (aura_updates.flags & AFLAG_DURATION)
                        mopFlags |= 0x04; // AFLAG_DURATION
                    if (aura_updates.flags & AFLAG_SEND_EFFECT_AMOUNT)
                        mopFlags |= 0x08; // AFLAG_ANY_EFFECT_AMOUNT_SENT
                    if (aura_updates.flags & AFLAG_NEGATIVE)
                        mopFlags |= 0x10; // AFLAG_NEGATIVE

                    packet << uint8_t(mopFlags);
                    packet << uint16_t(aura_updates.level);
                    packet << uint32_t(aura_updates.spellId);

                    if (aura_updates.flags & AFLAG_DURATION)
                    {
                        packet << uint32_t(aura_updates.duration);
                        packet << uint32_t(aura_updates.timeLeft);
                    }

                    packet << uint8_t(aura_updates.stackCount);
                    packet << uint32_t(aura_updates.flags & (AFLAG_EFFECT_1 | AFLAG_EFFECT_2 | AFLAG_EFFECT_3));

                    if (aura_updates.flags & AFLAG_SEND_EFFECT_AMOUNT)
                    {
                        // Must write exactly effCount floats to match the bit-header count written above
                        if (effCount >= 1)
                            packet << (aura_updates.flags & AFLAG_EFFECT_1 ? float(aura_updates.effAmount[0]) : float(0.f));

                        if (effCount >= 2)
                            packet << (aura_updates.flags & AFLAG_EFFECT_2 ? float(aura_updates.effAmount[1]) : float(0.f));

                        if (effCount >= 3)
                            packet << (aura_updates.flags & AFLAG_EFFECT_3 ? float(aura_updates.effAmount[2]) : float(0.f));
                    }
                }

                packet << uint8_t(aura_updates.visualSlot);

                packet.writeByteSeq(targetGuid[2]);
                packet.writeByteSeq(targetGuid[6]);
                packet.writeByteSeq(targetGuid[7]);
                packet.writeByteSeq(targetGuid[1]);
                packet.writeByteSeq(targetGuid[3]);
                packet.writeByteSeq(targetGuid[4]);
                packet.writeByteSeq(targetGuid[0]);
                packet.writeByteSeq(targetGuid[5]);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
