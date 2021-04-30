/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAuraUpdateAll : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        WoWGuid guid;

        struct AuraUpdate
        {
            uint8_t visualSlot;
            uint32_t spellId;
#if VERSION_STRING < Cata
            uint8_t flags;
#else
            uint16_t flags;
#endif
            uint8_t level;
            uint8_t stackCount;
            WoWGuid casterGuid;
            uint32_t duration;
            uint32_t timeLeft;
            int32_t effAmount[5]; // 3 spell effects up till cata, 5 in mop
        };

        std::vector<AuraUpdate> aura_updates;

        SmsgAuraUpdateAll() : SmsgAuraUpdateAll(WoWGuid(), std::vector<AuraUpdate>())
        {
        }

        SmsgAuraUpdateAll(WoWGuid guid, std::vector<AuraUpdate> aura_updates) :
            ManagedPacket(SMSG_AURA_UPDATE_ALL, 200),
            guid(guid),
            aura_updates(move(aura_updates))
        {
        }

        void addAuraUpdate(AuraUpdate aura_update)
        {
            aura_updates.push_back(aura_update);
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            packet << guid;

            for (const auto& auras : aura_updates)
            {
                packet << auras.visualSlot;
                packet << auras.spellId;

                packet << auras.flags;

                packet << auras.level;
                packet << auras.stackCount;

                if (!(auras.flags & 0x08))  // AFLAG_NOT_CASTER
                    packet << auras.casterGuid;

                if (auras.flags & 0x20)         // AFLAG_DURATION
                {
                    packet << auras.duration;
                    packet << auras.timeLeft;
                }

                if (auras.flags & 0x40) // AFLAG_SEND_EFFECT_AMOUNT
                {
                    if (auras.flags & 0x01) // AFLAG_EFFECT_1
                        packet << auras.effAmount[0];

                    if (auras.flags & 0x02) // AFLAG_EFFECT_2
                        packet << auras.effAmount[1];

                    if (auras.flags & 0x04) // AFLAG_EFFECT_3
                        packet << auras.effAmount[2];
                }
            }
#else
            ObjectGuid targetGuid = guid.getRawGuid();

            for (const auto& auras : aura_updates)

            packet.writeBit(targetGuid[7]);
            packet.writeBit(1);                                   // Is AURA_UPDATE_ALL
            packet.writeBits(aura_updates.size(), 24);           // Aura Count
            packet.writeBit(targetGuid[6]);
            packet.writeBit(targetGuid[1]);
            packet.writeBit(targetGuid[3]);
            packet.writeBit(targetGuid[0]);
            packet.writeBit(targetGuid[4]);
            packet.writeBit(targetGuid[2]);
            packet.writeBit(targetGuid[5]);

            for (const auto& auras : aura_updates)
            {
                packet.writeBit(1);                               // Not remove

                if (auras.flags & AFLAG_SEND_EFFECT_AMOUNT)
                {
                    uint8 effCount = 0;
                    if (auras.flags & 0x01) // AFLAG_EFFECT_1
                        effCount++;

                    if (auras.flags & 0x02) // AFLAG_EFFECT_2
                        effCount++;

                    if (auras.flags & 0x04) // AFLAG_EFFECT_3
                            effCount++;

                    packet.writeBits(effCount, 22);               // Effect Count
                }
                else
                    packet.writeBits(0, 22);                      // Effect Count

                packet.writeBit(!(auras.flags & AFLAG_IS_CASTER));         // HasCasterGuid

                if (!(auras.flags & AFLAG_IS_CASTER))
                {
                    ObjectGuid casterGuid = auras.casterGuid.getRawGuid();
                    packet.writeBit(casterGuid[3]);
                    packet.writeBit(casterGuid[4]);
                    packet.writeBit(casterGuid[6]);
                    packet.writeBit(casterGuid[1]);
                    packet.writeBit(casterGuid[5]);
                    packet.writeBit(casterGuid[2]);
                    packet.writeBit(casterGuid[0]);
                    packet.writeBit(casterGuid[7]);
                }

                packet.writeBits(0, 22);                          // Unk effect count
                packet.writeBit(auras.flags & AFLAG_DURATION);          // HasDuration
                packet.writeBit(auras.flags & AFLAG_DURATION);          // HasMaxDuration
            }

            packet.flushBits();

            for (const auto& auras : aura_updates)
            {
                if (!(auras.flags & AFLAG_IS_CASTER))
                {
                    ObjectGuid casterGuid = auras.casterGuid.getRawGuid();
                    packet.WriteByteSeq(casterGuid[3]);
                    packet.WriteByteSeq(casterGuid[2]);
                    packet.WriteByteSeq(casterGuid[1]);
                    packet.WriteByteSeq(casterGuid[6]);
                    packet.WriteByteSeq(casterGuid[4]);
                    packet.WriteByteSeq(casterGuid[0]);
                    packet.WriteByteSeq(casterGuid[5]);
                    packet.WriteByteSeq(casterGuid[7]);
                }

                packet << uint8(auras.flags);
                packet << uint16(auras.level);
                packet << uint32(auras.spellId);

                if (auras.flags & AFLAG_DURATION)
                {
                    packet << uint32(auras.duration);   //maxduration
                    packet << uint32(auras.duration);
                }

                // send stack amount for aura which could be stacked (never 0 - causes incorrect display) or charges
                // stack amount has priority over charges (checked on retail with spell 50262)
                packet << uint8(auras.stackCount);
                packet << uint32(0);    //effekt mask

                if (auras.flags & AFLAG_SEND_EFFECT_AMOUNT)
                {
                    if (auras.flags & 0x01) // AFLAG_EFFECT_1
                        packet << float(auras.effAmount[0]);
                    else
                        packet << float(0.f);

                    if (auras.flags & 0x02) // AFLAG_EFFECT_2
                        packet << float(auras.effAmount[1]);
                    else
                        packet << float(0.f);

                    if (auras.flags & 0x04) // AFLAG_EFFECT_3
                        packet << float(auras.effAmount[2]);
                    else
                        packet << float(0.f);

                }

                packet << uint8(auras.visualSlot);
            }

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
