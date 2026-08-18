/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Spell/SpellCastTargets.hpp"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/SpellPacketFlags.hpp"
#include "Spell/Spell.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellStart : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        WoWGuid casterUnitGuid;

        uint32_t spellId;
        uint32_t castFlags;
        uint8_t extraCastNumber;

        uint32_t timer;
        uint32_t castTime;

        SpellCastTargets targets;

        uint8_t powerType {0};
        uint32_t powerValue {0};

        Spell::ProjectileData projectile {0,0};

        SmsgSpellStart(WoWGuid casterGuid, WoWGuid casterUnitGuid, uint32_t spellId, uint32_t castFlags,
            uint8_t extraCastNumber, uint32_t timer, uint32_t castTime, SpellCastTargets targets) :
            ManagedPacket(SMSG_SPELL_START, 0),
            casterGuid(casterGuid), casterUnitGuid(casterUnitGuid), spellId(spellId), castFlags(castFlags),
            extraCastNumber(extraCastNumber), timer(timer), castTime(castTime), targets(targets)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + sizeof(SpellCastTargets);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << casterGuid;
                packet << casterUnitGuid;

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    packet << uint8_t(extraCastNumber);
                    packet << uint32_t(spellId);
                    packet << uint32_t(castFlags);
                }
                else //< WotLK
                {
                    packet << uint32_t(spellId);

                    if (m_protocol.expansion > WoW::Expansion::_Classic)
                        packet << uint8_t(extraCastNumber);

                    packet << uint16_t(castFlags);
                }

                packet << uint32_t(timer);

                if (m_protocol.expansion == WoW::Expansion::_Cata)
                    packet << uint32_t(castTime);

                targets.write(packet);

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    if (castFlags & SPELL_PACKET_FLAGS_POWER_UPDATE)
                        packet << uint32_t(powerValue);

                    if (castFlags & SPELL_PACKET_FLAGS_RANGED)
                    {
                        packet << uint32_t(projectile.displayInfo);
                        packet << uint32_t(projectile.inventoryType);
                    }
                }
            }
            else // Mop
            {
                WoWGuid targetGuid = targets.getUnitTargetGuid();
                WoWGuid itemTargetGuid = targets.getItemTargetGuid();
                WoWGuid unkGuid = 0;
                bool hasDestLocation = (targets.getTargetMask() & TARGET_FLAG_DEST_LOCATION) && targets.getDestination().isSet();
                bool hasSourceLocation = (targets.getTargetMask() & TARGET_FLAG_SOURCE_LOCATION) && targets.getSource().isSet();
                bool hasTargetString = targets.getTargetMask() & TARGET_FLAG_STRING;
                bool hasPredictedHeal = false;
                bool hasPredictedType = false;
                bool hasTargetMask = targets.getTargetMask() != 0;
                bool hasCastImmunities = false;
                bool hasCastSchoolImmunities = false;
                bool hasElevation = false;
                bool hasVisualChain = false;
                bool hasAmmoInventoryType = false;
                bool hasAmmoDisplayId = false;
                uint8_t runeCooldownPassedCount = 0;
                uint8_t predictedPowerCount = castFlags & 0x800 ? 1 : 0;

                packet.writeBits(0, 24);
                packet.writeBit(casterGuid[5]);

                packet.writeBit(1); // Unk read
                packet.writeBit(0); // Fake Bit
                packet.writeBit(casterUnitGuid[4]);
                packet.writeBit(casterGuid[2]);
                packet.writeBits(runeCooldownPassedCount, 3);
                packet.writeBit(casterUnitGuid[2]);
                packet.writeBit(casterUnitGuid[6]);
                packet.writeBits(0, 25);
                packet.writeBits(0, 13); // Unknown Bits
                packet.writeBit(casterGuid[4]);
                packet.writeBits(0, 24); // Hit Count
                packet.writeBit(casterUnitGuid[7]);

                packet.writeBit(hasSourceLocation);
                packet.writeBits(predictedPowerCount, 21);

                packet.writeBit(itemTargetGuid[3]);
                packet.writeBit(itemTargetGuid[0]);
                packet.writeBit(itemTargetGuid[1]);
                packet.writeBit(itemTargetGuid[7]);
                packet.writeBit(itemTargetGuid[2]);
                packet.writeBit(itemTargetGuid[6]);
                packet.writeBit(itemTargetGuid[4]);
                packet.writeBit(itemTargetGuid[5]);

                packet.writeBit(!hasElevation);
                packet.writeBit(!hasTargetString);
                packet.writeBit(!hasAmmoInventoryType);
                packet.writeBit(hasDestLocation);
                packet.writeBit(1); // Unk Read32
                packet.writeBit(casterGuid[3]);

                if (hasDestLocation)
                {
                    WoWGuid destTransportGuid = targets.getTransportDestinationGuid();
                    packet.writeBit(destTransportGuid[1]);
                    packet.writeBit(destTransportGuid[6]);
                    packet.writeBit(destTransportGuid[2]);
                    packet.writeBit(destTransportGuid[7]);
                    packet.writeBit(destTransportGuid[0]);
                    packet.writeBit(destTransportGuid[3]);
                    packet.writeBit(destTransportGuid[5]);
                    packet.writeBit(destTransportGuid[4]);
                }

                packet.writeBit(!hasAmmoDisplayId);

                if (hasSourceLocation)
                {
                    WoWGuid srcTransportGuid = targets.getTransportSourceGuid();
                    packet.writeBit(srcTransportGuid[4]);
                    packet.writeBit(srcTransportGuid[3]);
                    packet.writeBit(srcTransportGuid[5]);
                    packet.writeBit(srcTransportGuid[1]);
                    packet.writeBit(srcTransportGuid[7]);
                    packet.writeBit(srcTransportGuid[0]);
                    packet.writeBit(srcTransportGuid[6]);
                    packet.writeBit(srcTransportGuid[2]);
                }

                packet.writeBit(0); // Fake Bit
                packet.writeBit(casterGuid[6]);

                packet.writeBit(unkGuid[2]);
                packet.writeBit(unkGuid[1]);
                packet.writeBit(unkGuid[7]);
                packet.writeBit(unkGuid[6]);
                packet.writeBit(unkGuid[0]);
                packet.writeBit(unkGuid[5]);
                packet.writeBit(unkGuid[3]);
                packet.writeBit(unkGuid[4]);

                packet.writeBit(!hasTargetMask);

                if (hasTargetMask)
                    packet.writeBits(targets.getTargetMask(), 20);

                packet.writeBit(casterGuid[1]);
                packet.writeBit(!hasPredictedHeal);
                packet.writeBit(1); // Unk read int8_t
                packet.writeBit(!hasCastSchoolImmunities);
                packet.writeBit(casterUnitGuid[5]);
                packet.writeBit(0); // Fake Bit
                packet.writeBits(0, 20); // Not used

                packet.writeBit(targetGuid[1]);
                packet.writeBit(targetGuid[4]);
                packet.writeBit(targetGuid[6]);
                packet.writeBit(targetGuid[7]);
                packet.writeBit(targetGuid[5]);
                packet.writeBit(targetGuid[3]);
                packet.writeBit(targetGuid[0]);
                packet.writeBit(targetGuid[2]);

                packet.writeBit(casterGuid[0]);
                packet.writeBit(casterUnitGuid[3]);
                packet.writeBit(1); // Unk uint8_t

                if (hasTargetString)
                    packet.writeBits(uint32_t(targets.getStringTarget().length()), 7);

                packet.writeBit(!hasCastImmunities);
                packet.writeBit(casterUnitGuid[1]);
                packet.writeBit(hasVisualChain);
                packet.writeBit(casterGuid[7]);
                packet.writeBit(!hasPredictedType);
                packet.writeBit(casterUnitGuid[0]);

                packet.flushBits();

                packet.writeByteSeq(itemTargetGuid[1]);
                packet.writeByteSeq(itemTargetGuid[7]);
                packet.writeByteSeq(itemTargetGuid[6]);
                packet.writeByteSeq(itemTargetGuid[0]);
                packet.writeByteSeq(itemTargetGuid[4]);
                packet.writeByteSeq(itemTargetGuid[2]);
                packet.writeByteSeq(itemTargetGuid[3]);
                packet.writeByteSeq(itemTargetGuid[5]);

                packet.writeByteSeq(targetGuid[4]);
                packet.writeByteSeq(targetGuid[5]);
                packet.writeByteSeq(targetGuid[1]);
                packet.writeByteSeq(targetGuid[7]);
                packet.writeByteSeq(targetGuid[6]);
                packet.writeByteSeq(targetGuid[3]);
                packet.writeByteSeq(targetGuid[2]);
                packet.writeByteSeq(targetGuid[0]);

                packet << uint32_t(castTime);

                packet.writeByteSeq(unkGuid[4]);
                packet.writeByteSeq(unkGuid[5]);
                packet.writeByteSeq(unkGuid[3]);
                packet.writeByteSeq(unkGuid[2]);
                packet.writeByteSeq(unkGuid[1]);
                packet.writeByteSeq(unkGuid[6]);
                packet.writeByteSeq(unkGuid[7]);
                packet.writeByteSeq(unkGuid[0]);

                if (hasDestLocation)
                {
                    const LocationVector destPos = targets.getDestination();
                    WoWGuid destTransportGuid = targets.getTransportDestinationGuid();
                    packet.writeByteSeq(destTransportGuid[4]);
                    packet.writeByteSeq(destTransportGuid[0]);
                    packet.writeByteSeq(destTransportGuid[5]);
                    packet.writeByteSeq(destTransportGuid[7]);
                    packet.writeByteSeq(destTransportGuid[1]);
                    packet.writeByteSeq(destTransportGuid[2]);
                    packet.writeByteSeq(destTransportGuid[3]);
                    packet << float(destPos.y);
                    packet << float(destPos.z);
                    packet.writeByteSeq(destTransportGuid[6]);
                    packet << float(destPos.x);
                }

                if (hasSourceLocation)
                {
                    const LocationVector srcPos = targets.getSource();
                    WoWGuid srcTransportGuid = targets.getTransportSourceGuid();
                    packet.writeByteSeq(srcTransportGuid[0]);
                    packet.writeByteSeq(srcTransportGuid[5]);
                    packet.writeByteSeq(srcTransportGuid[4]);
                    packet.writeByteSeq(srcTransportGuid[7]);
                    packet.writeByteSeq(srcTransportGuid[3]);
                    packet.writeByteSeq(srcTransportGuid[6]);
                    packet << float(srcPos.x);
                    packet.writeByteSeq(srcTransportGuid[2]);
                    packet << float(srcPos.z);
                    packet.writeByteSeq(srcTransportGuid[1]);
                    packet << float(srcPos.y);
                }

                packet.writeByteSeq(casterGuid[4]);

                if (hasCastSchoolImmunities)
                    packet << uint32_t(0);

                packet.writeByteSeq(casterGuid[2]);

                if (hasCastImmunities)
                    packet << uint32_t(0);

                if (hasVisualChain)
                {
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                }

                if (predictedPowerCount > 0)
                {
                    // Mop wire format has the opposite field order of SMSG_SPELL_GO
                    // (power value, then power type) for this same "predicted power" block.
                    packet << int32_t(powerValue);
                    packet << uint8_t(powerType);
                }

                packet << uint32_t(castFlags);

                packet.writeByteSeq(casterGuid[5]);
                packet.writeByteSeq(casterGuid[7]);
                packet.writeByteSeq(casterGuid[1]);

                packet << uint8_t(extraCastNumber);

                packet.writeByteSeq(casterUnitGuid[7]);
                packet.writeByteSeq(casterUnitGuid[0]);
                packet.writeByteSeq(casterGuid[6]);
                packet.writeByteSeq(casterGuid[0]);
                packet.writeByteSeq(casterUnitGuid[1]);

                if (hasAmmoInventoryType)
                    packet << uint8_t(0);

                if (hasPredictedHeal)
                    packet << uint32_t(0);

                packet.writeByteSeq(casterUnitGuid[6]);
                packet.writeByteSeq(casterUnitGuid[3]);

                packet << uint32_t(spellId);

                if (hasAmmoDisplayId)
                    packet << uint32_t(0);

                packet.writeByteSeq(casterUnitGuid[4]);
                packet.writeByteSeq(casterUnitGuid[5]);
                packet.writeByteSeq(casterUnitGuid[2]);

                if (hasTargetString)
                    packet.writeString(targets.getStringTarget());

                if (hasPredictedType)
                    packet << uint8_t(0);

                packet.writeByteSeq(casterGuid[3]);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
