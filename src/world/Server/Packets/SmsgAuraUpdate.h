/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAuraUpdate : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;
        bool remove;

        struct AuraUpdate
        {
            uint8_t visualSlot = 0;
            uint32_t spellId = 0;
#if VERSION_STRING < Cata
            uint8_t flags = 0;
#else
            uint16_t flags = 0;
#endif
            uint8_t level = 0;
            uint8_t stackCount = 0;
            WoWGuid casterGuid;
            uint32_t duration = 0;
            uint32_t timeLeft = 0;
            int32_t effAmount[5] = {0}; // 3 spell effects up till cata, 5 in mop
        };

        AuraUpdate aura_updates;

        SmsgAuraUpdate() : SmsgAuraUpdate(WoWGuid(), {0}, false)
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
#if VERSION_STRING < Mop
            packet << guid;
            packet << aura_updates.visualSlot;

            if (remove)
            {
                packet << uint32_t(0);
            }
            else
            {
                packet << aura_updates.spellId;

                packet << aura_updates.flags;

                packet << aura_updates.level;
                packet << aura_updates.stackCount;

                if (!(aura_updates.flags & 0x08))  // AFLAG_NOT_CASTER
                    packet << aura_updates.casterGuid;

                if (aura_updates.flags & 0x20)         // AFLAG_DURATION
                {
                    packet << aura_updates.duration;
                    packet << aura_updates.timeLeft;
                }

                if (aura_updates.flags & 0x40) // AFLAG_SEND_EFFECT_AMOUNT
                {
                    if (aura_updates.flags & 0x01) // AFLAG_EFFECT_1
                        packet << aura_updates.effAmount[0];

                    if (aura_updates.flags & 0x02) // AFLAG_EFFECT_2
                        packet << aura_updates.effAmount[1];

                    if (aura_updates.flags & 0x04) // AFLAG_EFFECT_3
                        packet << aura_updates.effAmount[2];
                }
            }
#else // Mop
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
            packet.writeBit(1);

            if (aura_updates.flags & AFLAG_SEND_EFFECT_AMOUNT)
            {
                uint8_t effCount = 0;
                if (aura_updates.flags & AFLAG_EFFECT_1)
                    effCount++;

                if (aura_updates.flags & AFLAG_EFFECT_2)
                    effCount++;

                if (aura_updates.flags & AFLAG_EFFECT_3)
                    effCount++;

                packet.writeBits(effCount, 22);
            }
            else
                packet.writeBits(0, 22);

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

            packet.flushBits();

            if (!(aura_updates.flags & AFLAG_IS_CASTER))
            {
                WoWGuid casterGuid = aura_updates.casterGuid.getRawGuid();
                packet.WriteByteSeq(casterGuid[3]);
                packet.WriteByteSeq(casterGuid[2]);
                packet.WriteByteSeq(casterGuid[1]);
                packet.WriteByteSeq(casterGuid[6]);
                packet.WriteByteSeq(casterGuid[4]);
                packet.WriteByteSeq(casterGuid[0]);
                packet.WriteByteSeq(casterGuid[5]);
                packet.WriteByteSeq(casterGuid[7]);
            }

            packet << uint8_t(aura_updates.flags);
            packet << uint16_t(aura_updates.level);
            packet << uint32_t(aura_updates.spellId);

            if (aura_updates.flags & AFLAG_DURATION)
            {
                packet << uint32_t(aura_updates.duration);
                packet << uint32_t(aura_updates.duration);
            }

            packet << uint8_t(aura_updates.stackCount);
            packet << uint32_t(0);

            if (aura_updates.flags & AFLAG_SEND_EFFECT_AMOUNT)
            {
                if (aura_updates.flags & AFLAG_EFFECT_1)
                    packet << float(aura_updates.effAmount[0]);
                else
                    packet << float(0.f);

                if (aura_updates.flags & AFLAG_EFFECT_2)
                    packet << float(aura_updates.effAmount[1]);
                else
                    packet << float(0.f);

                if (aura_updates.flags & AFLAG_EFFECT_3)
                    packet << float(aura_updates.effAmount[2]);
                else
                    packet << float(0.f);
            }

            packet << uint8_t(aura_updates.visualSlot);

            packet.WriteByteSeq(targetGuid[2]);
            packet.WriteByteSeq(targetGuid[6]);
            packet.WriteByteSeq(targetGuid[7]);
            packet.WriteByteSeq(targetGuid[1]);
            packet.WriteByteSeq(targetGuid[3]);
            packet.WriteByteSeq(targetGuid[4]);
            packet.WriteByteSeq(targetGuid[0]);
            packet.WriteByteSeq(targetGuid[5]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
