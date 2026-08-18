/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellnonmeleedamagelog : public ManagedPacket
    {
    public:
        WoWGuid targetGuid;
        WoWGuid casterGuid;
        uint32_t spellId;
        uint32_t damage;
        uint32_t overKill;
        uint8_t school;
        uint32_t absorbedDamage;
        uint32_t resistedDamage;
        bool isPeriodicDamage;
        uint32_t blockedDamage;
        bool isCriticalHit;

        SmsgSpellnonmeleedamagelog() : SmsgSpellnonmeleedamagelog(WoWGuid(), WoWGuid(), 0, 0, 0, 0, 0, 0, false, 0, false)
        {
        }

        SmsgSpellnonmeleedamagelog(WoWGuid targetGuid, WoWGuid casterGuid, uint32_t spellId, uint32_t damage, uint32_t overKill,
            uint8_t school, uint32_t absorbedDamage, uint32_t resistedDamage, bool isPeriodicDamage, uint32_t blockedDamage, bool isCriticalHit) :
            ManagedPacket(SMSG_SPELLNONMELEEDAMAGELOG, 48),
            targetGuid(targetGuid), casterGuid(casterGuid), spellId(spellId), damage(damage), overKill(overKill),
            school(school), absorbedDamage(absorbedDamage), resistedDamage(resistedDamage),
            isPeriodicDamage(isPeriodicDamage), blockedDamage(blockedDamage), isCriticalHit(isCriticalHit)
        {
        }

    protected:
        size_t expectedSize() const override { return 48; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << targetGuid;
                packet << casterGuid;
                packet << spellId;
                packet << damage;

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    packet << overKill;

                packet << school;
                packet << absorbedDamage;
                packet << resistedDamage;
                packet << uint8_t(isPeriodicDamage);
                packet << uint8_t(0); // unk

                packet << blockedDamage;

                // Some sort of hit info, other values need more research
                if (isCriticalHit)
                    packet << uint32_t(0x2);
                else
                    packet << uint32_t(0);

                packet << uint8_t(0); // debug mode boolean

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(targetGuid[2]);
                packet.writeBit(casterGuid[7]);
                packet.writeBit(casterGuid[6]);
                packet.writeBit(casterGuid[1]);
                packet.writeBit(casterGuid[5]);

                packet.writeBit(0);                        // unk

                packet.writeBit(casterGuid[0]);
                packet.writeBit(targetGuid[0]);
                packet.writeBit(targetGuid[7]);
                packet.writeBit(casterGuid[3]);
                packet.writeBit(targetGuid[6]);

                packet.writeBit(0);                        // unk
                packet.writeBit(0);                        // hasPowerData

                packet.writeBit(targetGuid[1]);

                packet.writeBit(0);                        // no floats

                packet.writeBit(targetGuid[5]);
                packet.writeBit(casterGuid[2]);
                packet.writeBit(casterGuid[4]);
                packet.writeBit(targetGuid[3]);
                packet.writeBit(targetGuid[4]);

                packet.flushBits();

                // Base SPELL_HIT_TYPE flags (UNK1 | UNK3 | UNK6) plus CRIT when applicable.
                const uint32_t hitInfo = 0x25 | (isCriticalHit ? 0x2 : 0);

                packet << uint32_t(blockedDamage);

                packet.writeByteSeq(casterGuid[1]);

                packet << uint32_t(overKill);

                packet.writeByteSeq(targetGuid[3]);
                packet.writeByteSeq(casterGuid[0]);
                packet.writeByteSeq(targetGuid[6]);
                packet.writeByteSeq(targetGuid[4]);
                packet.writeByteSeq(casterGuid[7]);

                packet << uint32_t(resistedDamage);
                packet << uint32_t(absorbedDamage);

                packet.writeByteSeq(casterGuid[5]);
                packet.writeByteSeq(targetGuid[5]);
                packet.writeByteSeq(casterGuid[3]);
                packet.writeByteSeq(casterGuid[2]);
                packet.writeByteSeq(targetGuid[2]);
                packet.writeByteSeq(casterGuid[6]);
                packet.writeByteSeq(targetGuid[0]);
                packet.writeByteSeq(casterGuid[4]);

                packet << uint32_t(damage);
                packet << uint8_t(school);

                packet.writeByteSeq(targetGuid[7]);

                packet << uint32_t(hitInfo);

                packet.writeByteSeq(targetGuid[1]);

                packet << uint32_t(spellId);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
