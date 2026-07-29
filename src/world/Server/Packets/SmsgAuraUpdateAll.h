/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Spell/SpellAuraDefines.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgAuraUpdateAll : public ManagedPacket
    {
    public:
        WoWGuid guid;

        struct AuraUpdate
        {
            uint8_t visualSlot = 0;
            uint32_t spellId = 0;
            uint16_t flags = 0;
            uint8_t level = 0;
            uint8_t stackCount = 0;
            WoWGuid casterGuid;
            uint32_t duration = 0;
            uint32_t timeLeft = 0;
            int32_t effAmount[5] = {0}; // 3 spell effects up till cata, 5 in mop
        };

        std::vector<AuraUpdate> aura_updates;

        SmsgAuraUpdateAll() : SmsgAuraUpdateAll(WoWGuid(), std::vector<AuraUpdate>())
        {
        }

        SmsgAuraUpdateAll(WoWGuid guid, std::vector<AuraUpdate> aura_updates) :
            ManagedPacket(SMSG_AURA_UPDATE_ALL, 200),
            guid(guid),
            aura_updates(std::move(aura_updates))
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
            if (m_protocol.expansion < WoW::Expansion::_TBC)
                return false;

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << guid;

                for (const auto& auras : aura_updates)
                {
                    packet << auras.visualSlot;
                    packet << auras.spellId;

                    if (m_protocol.expansion < WoW::Expansion::_Cata)
                        packet << static_cast<uint8_t>(auras.flags);
                    else
                        packet << auras.flags;

                    packet << auras.level;
                    packet << auras.stackCount;

                    if (!(auras.flags & AFLAG_IS_CASTER))
                        packet << auras.casterGuid;

                    if (auras.flags & AFLAG_DURATION)
                    {
                        packet << auras.duration;
                        packet << auras.timeLeft;
                    }

                    if (auras.flags & AFLAG_SEND_EFFECT_AMOUNT)
                    {
                        if (auras.flags & AFLAG_EFFECT_1)
                            packet << auras.effAmount[0];

                        if (auras.flags & AFLAG_EFFECT_2)
                            packet << auras.effAmount[1];

                        if (auras.flags & AFLAG_EFFECT_3)
                            packet << auras.effAmount[2];
                    }
                }
            }
            else // Mop
            {
                WoWGuid targetGuid = guid.getRawGuid();

                packet.writeBit(targetGuid[7]);
                packet.writeBit(1); // Is AURA_UPDATE_ALL
                packet.writeBits(aura_updates.size(), 24); // Aura Count
                packet.writeBit(targetGuid[6]);
                packet.writeBit(targetGuid[1]);
                packet.writeBit(targetGuid[3]);
                packet.writeBit(targetGuid[0]);
                packet.writeBit(targetGuid[4]);
                packet.writeBit(targetGuid[2]);
                packet.writeBit(targetGuid[5]);

                for (const auto& auras : aura_updates)
                {
                    packet.writeBit(1); // Not remove

                    if (auras.flags & AFLAG_SEND_EFFECT_AMOUNT)
                    {
                        uint8_t effCount = 0;
                        if (auras.flags & AFLAG_EFFECT_1)
                            effCount++;

                        if (auras.flags & AFLAG_EFFECT_2)
                            effCount++;

                        if (auras.flags & AFLAG_EFFECT_3)
                            effCount++;

                        packet.writeBits(effCount, 22); // Effect Count
                    }
                    else
                        packet.writeBits(0, 22); // Effect Count

                    packet.writeBit(!(auras.flags & AFLAG_IS_CASTER)); // HasCasterGuid

                    if (!(auras.flags & AFLAG_IS_CASTER))
                    {
                        WoWGuid casterGuid = auras.casterGuid.getRawGuid();
                        packet.writeBit(casterGuid[3]);
                        packet.writeBit(casterGuid[4]);
                        packet.writeBit(casterGuid[6]);
                        packet.writeBit(casterGuid[1]);
                        packet.writeBit(casterGuid[5]);
                        packet.writeBit(casterGuid[2]);
                        packet.writeBit(casterGuid[0]);
                        packet.writeBit(casterGuid[7]);
                    }

                    packet.writeBits(0, 22); // Unk effect count
                    packet.writeBit(auras.flags & AFLAG_DURATION); // HasDuration
                    packet.writeBit(auras.flags & AFLAG_DURATION); // HasMaxDuration
                }

                packet.flushBits();

                for (const auto& auras : aura_updates)
                {
                    if (!(auras.flags & AFLAG_IS_CASTER))
                    {
                        WoWGuid casterGuid = auras.casterGuid.getRawGuid();
                        packet.writeByteSeq(casterGuid[3]);
                        packet.writeByteSeq(casterGuid[2]);
                        packet.writeByteSeq(casterGuid[1]);
                        packet.writeByteSeq(casterGuid[6]);
                        packet.writeByteSeq(casterGuid[4]);
                        packet.writeByteSeq(casterGuid[0]);
                        packet.writeByteSeq(casterGuid[5]);
                        packet.writeByteSeq(casterGuid[7]);
                    }

                    packet << uint8_t(auras.flags);
                    packet << uint16_t(auras.level);
                    packet << uint32_t(auras.spellId);

                    if (auras.flags & AFLAG_DURATION)
                    {
                        packet << uint32_t(auras.duration); // maxduration
                        packet << uint32_t(auras.duration);
                    }

                    packet << uint8_t(auras.stackCount);
                    packet << uint32_t(0); // effect mask

                    if (auras.flags & AFLAG_SEND_EFFECT_AMOUNT)
                    {
                        if (auras.flags & AFLAG_EFFECT_1)
                            packet << float(auras.effAmount[0]);
                        else
                            packet << float(0.f);

                        if (auras.flags & AFLAG_EFFECT_2)
                            packet << float(auras.effAmount[1]);
                        else
                            packet << float(0.f);

                        if (auras.flags & AFLAG_EFFECT_3)
                            packet << float(auras.effAmount[2]);
                        else
                            packet << float(0.f);
                    }

                    packet << uint8_t(auras.visualSlot);
                }

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
