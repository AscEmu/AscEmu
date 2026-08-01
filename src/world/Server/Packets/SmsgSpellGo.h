/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Spell/SpellCastTargets.hpp"
#include "Spell/Spell.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellGo : public ManagedPacket
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

        std::vector<SpellUniqueTarget> hittedTargets {};
        std::vector<SpellTargetMod> missedTargets {};

        uint8_t powerType {0};
        uint32_t powerValue {0};

        Spell::ProjectileData projectile {0,0};

        uint8_t runeAvailableBefore {0};
        uint8_t currentRunes {0};

        float missilePitch {.0f};
        uint32_t missileTravelTime {0};

        SmsgSpellGo(WoWGuid casterGuid, WoWGuid casterUnitGuid, uint32_t spellId, uint32_t castFlags,
            uint8_t extraCastNumber, uint32_t timer, uint32_t castTime, SpellCastTargets targets) :
            ManagedPacket(SMSG_SPELL_GO, 0),
            casterGuid(casterGuid), casterUnitGuid(casterUnitGuid), spellId(spellId), castFlags(castFlags),
            extraCastNumber(extraCastNumber), timer(timer), castTime(castTime), targets(targets)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + sizeof(SpellCastTargets) + (hittedTargets.size() * 8) + (missedTargets.size() * 8);
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
                    packet << uint16_t(castFlags);
                }

                if (m_protocol.expansion == WoW::Expansion::_Cata)
                    packet << uint32_t(timer);

                if (m_protocol.expansion > WoW::Expansion::_Classic)
                    packet << uint32_t(castTime);

                packet << uint8_t(hittedTargets.size());
                for (const auto& uniqueTarget : hittedTargets)
                    packet << uint64_t(uniqueTarget.first);

                packet << uint8_t(missedTargets.size());
                if (!missedTargets.empty())
                {
                    for (const auto& missedTarget : missedTargets)
                    {
                        packet << uint64_t(missedTarget.targetGuid);
                        packet << uint8_t(missedTarget.hitResult);

                        if (missedTarget.hitResult == SPELL_DID_HIT_REFLECT)
                            packet << uint8_t(missedTarget.extendedHitResult);
                    }
                }

                targets.write(packet);

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    packet << uint32_t(powerValue);

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    packet << uint32_t(projectile.displayInfo);
                    packet << uint32_t(projectile.inventoryType);
                }

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    if (castFlags & 0x200000) // SPELL_PACKET_FLAGS_RUNE_UPDATE
                    {
                        packet << uint8_t(runeAvailableBefore);
                        packet << uint8_t(currentRunes);
                        for (uint8_t i = 0; i < 6; ++i)
                        {
                            const uint8_t runeMask = 1U << i;
                            if ((runeMask & runeAvailableBefore) != (runeMask & currentRunes))
                                packet << uint8_t(0); // Value of the rune converted into byte
                        }
                    }

                    if (castFlags & 0x20000) // SPELL_PACKET_FLAGS_UPDATE_MISSILE
                    {
                        packet << float(missilePitch);
                        packet << uint32_t(missileTravelTime);
                    }

                    if (targets.hasDestination())
                        packet << uint8_t(0);
                }
            }
            else // Mop
            {
                WoWGuid targetGuid = targets.getGameObjectTargetGuid();
                WoWGuid itemTargetGuid = targets.getItemTargetGuid();
                WoWGuid unkGuid = 0;
                bool hasDestLocation = (targets.getTargetMask() & TARGET_FLAG_DEST_LOCATION) && targets.getDestination().isSet();
                bool hasSourceLocation = (targets.getTargetMask() & TARGET_FLAG_SOURCE_LOCATION) && targets.getDestination().isSet();
                bool hasDestUnkByte = targets.getTargetMask() & TARGET_FLAG_DEST_LOCATION;
                bool hasTargetString = targets.getTargetMask() & TARGET_FLAG_STRING;
                [[maybe_unused]] bool hasPredictedHeal = false;
                bool hasPredictedType = false;
                bool hasTargetMask = targets.getTargetMask() != 0;
                [[maybe_unused]] bool hasCastImmunities = false;
                [[maybe_unused]] bool hasCastSchoolImmunities = false;
                bool hasElevation = false;
                bool hasDelayTime = false;
                bool hasVisualChain = false;
                [[maybe_unused]] bool hasAmmoInventoryType = false;
                [[maybe_unused]] bool hasAmmoDisplayId = false;
                bool hasRunesStateBefore = false;
                bool hasRunesStateAfter = false;
                uint8_t predictedPowerCount = false;
                uint8_t runeCooldownPassedCount = false;

                packet.writeBit(casterUnitGuid[2]);
                packet.writeBit(1); // hasAmmoDisplayType
                packet.writeBit(hasSourceLocation);
                packet.writeBit(casterGuid[2]);


                packet.writeBit(casterGuid[6]);
                packet.writeBit(!hasDestUnkByte);
                packet.writeBit(casterUnitGuid[7]);
                packet.writeBits(0, 20); // Extra Target Count

                packet.writeBits(missedTargets.size(), 25); // Miss Type Count
                packet.writeBits(missedTargets.size(), 24); // Miss Count

                packet.writeBit(casterUnitGuid[1]);
                packet.writeBit(casterGuid[0]);
                packet.writeBits(0, 13); // Unknown bits

                for (const auto missedTarget : missedTargets)
                {
                    WoWGuid missGuid = missedTarget.targetGuid;

                    packet.writeBit(missGuid[1]);
                    packet.writeBit(missGuid[3]);
                    packet.writeBit(missGuid[6]);
                    packet.writeBit(missGuid[4]);
                    packet.writeBit(missGuid[5]);
                    packet.writeBit(missGuid[2]);
                    packet.writeBit(missGuid[0]);
                    packet.writeBit(missGuid[7]);
                }

                packet.writeBit(casterUnitGuid[5]);
                packet.writeBit(0); // Fake bit
                packet.writeBit(0); // Fake bit
                packet.writeBit(!hasTargetString);

                packet.writeBit(itemTargetGuid[7]);
                packet.writeBit(itemTargetGuid[2]);
                packet.writeBit(itemTargetGuid[1]);
                packet.writeBit(itemTargetGuid[3]);
                packet.writeBit(itemTargetGuid[6]);
                packet.writeBit(itemTargetGuid[0]);
                packet.writeBit(itemTargetGuid[5]);
                packet.writeBit(itemTargetGuid[4]);

                packet.writeBit(casterGuid[7]);

                packet.writeBit(targetGuid[0]);
                packet.writeBit(targetGuid[6]);
                packet.writeBit(targetGuid[5]);
                packet.writeBit(targetGuid[7]);
                packet.writeBit(targetGuid[4]);
                packet.writeBit(targetGuid[2]);
                packet.writeBit(targetGuid[3]);
                packet.writeBit(targetGuid[1]);

                packet.writeBit(!hasRunesStateBefore);
                packet.writeBits(predictedPowerCount, 21); // predictedPowerCount
                packet.writeBit(casterGuid[1]);
                packet.writeBit(!hasPredictedType);
                packet.writeBit(!hasTargetMask);
                packet.writeBit(casterUnitGuid[3]);

                packet.writeBit(1); // Missing Predict heal
                packet.writeBit(0); // hasPowerData
                packet.writeBit(1); // has castImmunitiy
                packet.writeBit(casterUnitGuid[6]);
                packet.writeBit(0); // Fake bit
                packet.writeBit(hasVisualChain);

                packet.writeBit(unkGuid[7]);
                packet.writeBit(unkGuid[6]);
                packet.writeBit(unkGuid[1]);
                packet.writeBit(unkGuid[2]);
                packet.writeBit(unkGuid[0]);
                packet.writeBit(unkGuid[5]);
                packet.writeBit(unkGuid[3]);
                packet.writeBit(unkGuid[4]);

                packet.writeBit(!hasDelayTime);
                packet.writeBit(1); // has School Immunities
                packet.writeBits(runeCooldownPassedCount, 3); // runeCooldownPassedCount
                packet.writeBit(casterUnitGuid[0]);

                for (const auto missedTarget : missedTargets)
                {
                    packet.writeBits(missedTarget.hitResult, 4);
                    if (missedTarget.hitResult == SPELL_DID_HIT_REFLECT)
                        packet.writeBits(missedTarget.extendedHitResult, 4);
                }

                if (hasTargetMask)
                    packet.writeBits(targets.getTargetMask(), 20);

                packet.writeBit(!hasElevation);
                packet.writeBit(!hasRunesStateAfter);
                packet.writeBit(casterGuid[4]);
                packet.writeBit(1); // hasAmmodisplayID
                packet.writeBit(hasDestLocation);
                packet.writeBit(casterGuid[5]);

                packet.writeBits(hittedTargets.size(), 24);

                packet.writeBit(casterUnitGuid[4]);

                for (const auto hittedTarget : hittedTargets)
                {
                    WoWGuid hitGuid = hittedTarget.first;

                    packet.writeBit(hitGuid[2]);
                    packet.writeBit(hitGuid[7]);
                    packet.writeBit(hitGuid[1]);
                    packet.writeBit(hitGuid[6]);
                    packet.writeBit(hitGuid[4]);
                    packet.writeBit(hitGuid[5]);
                    packet.writeBit(hitGuid[0]);
                    packet.writeBit(hitGuid[3]);
                }

                packet.writeBit(casterGuid[3]);
                packet.flushBits();

                packet.writeByteSeq(targetGuid[5]);
                packet.writeByteSeq(targetGuid[2]);
                packet.writeByteSeq(targetGuid[1]);
                packet.writeByteSeq(targetGuid[6]);
                packet.writeByteSeq(targetGuid[0]);
                packet.writeByteSeq(targetGuid[3]);
                packet.writeByteSeq(targetGuid[4]);
                packet.writeByteSeq(targetGuid[7]);

                packet.writeByteSeq(itemTargetGuid[5]);
                packet.writeByteSeq(itemTargetGuid[2]);
                packet.writeByteSeq(itemTargetGuid[0]);
                packet.writeByteSeq(itemTargetGuid[6]);
                packet.writeByteSeq(itemTargetGuid[7]);
                packet.writeByteSeq(itemTargetGuid[3]);
                packet.writeByteSeq(itemTargetGuid[1]);
                packet.writeByteSeq(itemTargetGuid[4]);

                packet.writeByteSeq(casterGuid[2]);

                for (const auto hittedTarget : hittedTargets)
                {
                    WoWGuid hitGuid = hittedTarget.first;

                    packet.writeByteSeq(hitGuid[0]);
                    packet.writeByteSeq(hitGuid[6]);
                    packet.writeByteSeq(hitGuid[2]);
                    packet.writeByteSeq(hitGuid[7]);
                    packet.writeByteSeq(hitGuid[5]);
                    packet.writeByteSeq(hitGuid[4]);
                    packet.writeByteSeq(hitGuid[3]);
                    packet.writeByteSeq(hitGuid[1]);
                }

                packet.writeByteSeq(unkGuid[6]);
                packet.writeByteSeq(unkGuid[2]);
                packet.writeByteSeq(unkGuid[7]);
                packet.writeByteSeq(unkGuid[1]);
                packet.writeByteSeq(unkGuid[4]);
                packet.writeByteSeq(unkGuid[3]);
                packet.writeByteSeq(unkGuid[5]);
                packet.writeByteSeq(unkGuid[0]);

                packet << uint32_t(::Util::getMSTime());

                for (const auto missedTarget : missedTargets)
                {
                    WoWGuid missGuid = missedTarget.targetGuid;

                    packet.writeByteSeq(missGuid[4]);
                    packet.writeByteSeq(missGuid[2]);
                    packet.writeByteSeq(missGuid[0]);
                    packet.writeByteSeq(missGuid[6]);
                    packet.writeByteSeq(missGuid[7]);
                    packet.writeByteSeq(missGuid[5]);
                    packet.writeByteSeq(missGuid[1]);
                    packet.writeByteSeq(missGuid[3]);
                }

                packet.writeByteSeq(casterGuid[6]);
                packet.writeByteSeq(casterUnitGuid[7]);
                packet.writeByteSeq(casterGuid[1]);

                if (hasVisualChain)
                {
                    packet << uint32_t(0);
                    packet << uint32_t(0);
                }

                packet << uint32_t(castFlags);

                packet.writeByteSeq(casterUnitGuid[6]);

                if (hasPredictedType)
                    packet << uint8_t(0);

                packet.writeByteSeq(casterGuid[4]);
                packet.writeByteSeq(casterUnitGuid[1]);

                if (powerValue)
                {
                    packet << uint8_t(powerType);
                    packet << uint32_t(powerValue);
                }

                packet.writeByteSeq(casterGuid[0]);

                packet << uint8_t(extraCastNumber);

                packet.writeByteSeq(casterGuid[5]);
                packet.writeByteSeq(casterUnitGuid[2]);
                packet.writeByteSeq(casterGuid[3]);
                packet.writeByteSeq(casterUnitGuid[5]);

                if (hasTargetString)
                    packet.writeString(targets.getStringTarget());

                packet << uint32_t(spellId);

                packet.writeByteSeq(casterUnitGuid[0]);
                packet.writeByteSeq(casterUnitGuid[3]);
                packet.writeByteSeq(casterUnitGuid[4]);
                packet.writeByteSeq(casterGuid[7]);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
