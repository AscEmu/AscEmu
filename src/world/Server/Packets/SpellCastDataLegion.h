/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/WorldPacket.hpp"
#include "Spell/SpellCastTargets.hpp"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/SpellDidHitResult.hpp"
#include "Spell/Definitions/SpellTargetMod.hpp"
#include "WoWGuid.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    // The cast data shared by SMSG_SPELL_START and SMSG_SPELL_GO of 6.2.4 and 7.3.5 clients
    struct SpellCastDataLegion
    {
        WoWGuid casterGuid;
        WoWGuid casterUnitGuid;
        uint32_t spellId = 0;
        uint32_t castFlags = 0;
        uint32_t castTime = 0;
        uint8_t castNumber = 0;
        SpellCastTargets const* targets = nullptr;

        std::vector<uint64_t> hitTargets;
        std::vector<SpellTargetMod> missedTargets;

        bool hasPower = false;
        uint8_t powerType = 0;
        uint32_t powerValue = 0;

        uint32_t ammoDisplayId = 0;
        float missilePitch = 0.0f;
        uint32_t missileTravelTime = 0;

        uint32_t realmId = 0;
        uint32_t mapId = 0;

        void write(WorldPacket& packet) const
        {
            const uint32_t targetMask = targets != nullptr ? targets->getTargetMask() : 0;
            const bool hasSource = targets != nullptr && (targetMask & TARGET_FLAG_SOURCE_LOCATION) && targets->getSource().isSet();
            const bool hasDestination = targets != nullptr && (targetMask & TARGET_FLAG_DEST_LOCATION) && targets->getDestination().isSet();
            const std::string targetName = targets != nullptr && (targetMask & TARGET_FLAG_STRING) ? targets->getStringTarget() : std::string();

            uint64_t unitTarget = 0;
            uint64_t itemTarget = 0;
            if (targets != nullptr)
            {
                unitTarget = targets->getUnitTargetGuid() != 0 ? targets->getUnitTargetGuid() : targets->getGameObjectTargetGuid();
                itemTarget = targets->getItemTargetGuid();
            }

            packet << casterGuid.toGuid128(realmId, mapId);
            packet << casterUnitGuid.toGuid128(realmId, mapId);
            // the cast: one per caster and spell, the start and the go of a cast carry the same guid
            const WoWGuid128 castGuid = WoWGuid128::cast(realmId, mapId, spellId, casterUnitGuid.getLowGuid());
            packet << castGuid;
            packet << castGuid;                                     // original cast
            packet << uint32_t(spellId);
            packet << uint32_t(0);                                  // spell visual
            packet << uint32_t(castFlags);
            packet << uint32_t(castTime);

            packet << uint32_t(missileTravelTime);
            packet << float(missilePitch);
            packet << int32_t(ammoDisplayId);
            packet << uint8_t(0);                                   // destination of the cast index
            packet << uint32_t(0);                                  // immune school
            packet << uint32_t(0);                                  // immune value
            packet << uint32_t(0);                                  // predicted heal
            packet << uint8_t(0);                                   // predicted heal type
            packet << WoWGuid128();                                 // beacon

            packet.writeBits(0, 23);                                // extended cast flags
            packet.writeBits(static_cast<uint32_t>(hitTargets.size()), 16);
            packet.writeBits(static_cast<uint32_t>(missedTargets.size()), 16);
            packet.writeBits(static_cast<uint32_t>(missedTargets.size()), 16);
            packet.writeBits(hasPower ? 1 : 0, 9);
            packet.writeBit(false);                                 // remaining runes
            packet.writeBits(0, 16);                                // target points
            packet.flushBits();

            for (const auto& missedTarget : missedTargets)
            {
                packet.writeBits(static_cast<uint32_t>(missedTarget.hitResult), 4);
                if (missedTarget.hitResult == SPELL_DID_HIT_REFLECT)
                    packet.writeBits(static_cast<uint32_t>(missedTarget.extendedHitResult), 4);
                packet.flushBits();
            }

            // the target data
            packet.writeBits(targetMask, 25);
            packet.writeBit(hasSource);
            packet.writeBit(hasDestination);
            packet.writeBit(false);                                 // orientation
            packet.writeBit(false);                                 // map
            packet.writeBits(static_cast<uint32_t>(targetName.length()), 7);
            packet.flushBits();

            packet << WoWGuid(unitTarget).toGuid128(realmId, mapId);
            packet << WoWGuid(itemTarget).toGuid128(realmId, mapId);

            if (hasSource)
            {
                packet << WoWGuid(targets->getTransportSourceGuid()).toGuid128(realmId, mapId);
                packet << float(targets->getSource().x);
                packet << float(targets->getSource().y);
                packet << float(targets->getSource().z);
            }

            if (hasDestination)
            {
                packet << WoWGuid(targets->getTransportDestinationGuid()).toGuid128(realmId, mapId);
                packet << float(targets->getDestination().x);
                packet << float(targets->getDestination().y);
                packet << float(targets->getDestination().z);
            }

            packet.writeString(targetName);

            for (const uint64_t hitTarget : hitTargets)
                packet << WoWGuid(hitTarget).toGuid128(realmId, mapId);

            for (const auto& missedTarget : missedTargets)
                packet << WoWGuid(missedTarget.targetGuid).toGuid128(realmId, mapId);

            if (hasPower)
            {
                packet << int32_t(powerValue);
                packet << uint8_t(powerType);
            }
        }

        // 6.2.4: the cast is a counter, the counts are part of the header and the target data follows them
        void writeWoD(WorldPacket& packet) const
        {
            const uint32_t targetMask = targets != nullptr ? targets->getTargetMask() : 0;
            const bool hasSource = targets != nullptr && (targetMask & TARGET_FLAG_SOURCE_LOCATION) && targets->getSource().isSet();
            const bool hasDestination = targets != nullptr && (targetMask & TARGET_FLAG_DEST_LOCATION) && targets->getDestination().isSet();
            const std::string targetName = targets != nullptr && (targetMask & TARGET_FLAG_STRING) ? targets->getStringTarget() : std::string();

            uint64_t unitTarget = 0;
            uint64_t itemTarget = 0;
            if (targets != nullptr)
            {
                unitTarget = targets->getUnitTargetGuid() != 0 ? targets->getUnitTargetGuid() : targets->getGameObjectTargetGuid();
                itemTarget = targets->getItemTargetGuid();
            }

            packet << casterGuid.toGuid128(realmId, mapId);
            packet << casterUnitGuid.toGuid128(realmId, mapId);
            packet << uint8_t(castNumber);
            packet << int32_t(spellId);
            packet << uint32_t(0);                                  // spell visual
            packet << uint32_t(castFlags);
            packet << uint32_t(castTime);
            packet << uint32_t(hitTargets.size());
            packet << uint32_t(missedTargets.size());
            packet << uint32_t(missedTargets.size());

            // the target data
            packet.writeBits(targetMask, 23);
            packet.writeBit(hasSource);
            packet.writeBit(hasDestination);
            packet.writeBit(false);                                 // orientation
            packet.writeBits(static_cast<uint32_t>(targetName.length()), 7);
            packet.flushBits();

            packet << WoWGuid(unitTarget).toGuid128(realmId, mapId);
            packet << WoWGuid(itemTarget).toGuid128(realmId, mapId);

            if (hasSource)
            {
                packet << WoWGuid(targets->getTransportSourceGuid()).toGuid128(realmId, mapId);
                packet << float(targets->getSource().x);
                packet << float(targets->getSource().y);
                packet << float(targets->getSource().z);
            }

            if (hasDestination)
            {
                packet << WoWGuid(targets->getTransportDestinationGuid()).toGuid128(realmId, mapId);
                packet << float(targets->getDestination().x);
                packet << float(targets->getDestination().y);
                packet << float(targets->getDestination().z);
            }

            packet.writeString(targetName);

            packet << uint32_t(hasPower ? 1 : 0);
            packet << uint32_t(missileTravelTime);
            packet << float(missilePitch);
            packet << int32_t(ammoDisplayId);
            packet << int8_t(0);                                    // ammo inventory type
            packet << uint8_t(0);                                   // destination of the cast index
            packet << uint32_t(0);                                  // target points
            packet << int32_t(0);                                   // immune school
            packet << int32_t(0);                                   // immune value
            packet << int32_t(0);                                   // predicted heal
            packet << uint8_t(0);                                   // predicted heal type
            packet << WoWGuid128();                                 // beacon

            for (const uint64_t hitTarget : hitTargets)
                packet << WoWGuid(hitTarget).toGuid128(realmId, mapId);

            for (const auto& missedTarget : missedTargets)
                packet << WoWGuid(missedTarget.targetGuid).toGuid128(realmId, mapId);

            for (const auto& missedTarget : missedTargets)
            {
                const bool reflected = missedTarget.hitResult == SPELL_DID_HIT_REFLECT;

                packet.writeBits(static_cast<uint32_t>(missedTarget.hitResult), 4);
                packet.writeBits(reflected ? static_cast<uint32_t>(missedTarget.extendedHitResult) : 0, 4);
            }
            packet.flushBits();

            if (hasPower)
            {
                packet << int32_t(powerValue);
                packet << int8_t(powerType);
            }

            packet.writeBits(0, 20);                                // extended cast flags
            packet.writeBit(false);                                 // remaining runes
            packet.flushBits();
        }
    };
}
