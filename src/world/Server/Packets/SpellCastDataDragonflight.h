/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "SpellCastDataLegion.h"

#include "Network/WorldPacket.hpp"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/Definitions/SpellDidHitResult.hpp"
#include "WoWGuid.hpp"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    // The cast data of SMSG_SPELL_START and SMSG_SPELL_GO of 10.2.7 clients: the 9.2.7 layout with 28 target flag
    // bits and the miss results as bytes behind the hit results
    inline void writeSpellCastDataDragonflight(WorldPacket& packet, SpellCastDataLegion const& cast)
    {
        const uint32_t targetMask = cast.targets != nullptr ? cast.targets->getTargetMask() : 0;
        const bool hasSource = cast.targets != nullptr && (targetMask & TARGET_FLAG_SOURCE_LOCATION) && cast.targets->getSource().isSet();
        const bool hasDestination = cast.targets != nullptr && (targetMask & TARGET_FLAG_DEST_LOCATION) && cast.targets->getDestination().isSet();
        const std::string targetName = cast.targets != nullptr && (targetMask & TARGET_FLAG_STRING) ? cast.targets->getStringTarget() : std::string();

        uint64_t unitTarget = 0;
        uint64_t itemTarget = 0;
        if (cast.targets != nullptr)
        {
            unitTarget = cast.targets->getUnitTargetGuid() != 0 ? cast.targets->getUnitTargetGuid() : cast.targets->getGameObjectTargetGuid();
            itemTarget = cast.targets->getItemTargetGuid();
        }

        packet << cast.casterGuid.toGuid128(cast.realmId, cast.mapId);
        packet << cast.casterUnitGuid.toGuid128(cast.realmId, cast.mapId);
        // the cast: one per caster and spell, the start and the go of a cast carry the same guid
        const WoWGuid128 castGuid = WoWGuid128::cast(cast.realmId, cast.mapId, cast.spellId, cast.casterUnitGuid.getLowGuid());
        packet << castGuid;
        packet << castGuid;                                     // original cast
        packet << uint32_t(cast.spellId);
        packet << int32_t(0);                                   // spell visual
        packet << int32_t(0);                                   // script visual
        packet << uint32_t(cast.castFlags);
        packet << uint32_t(0);                                  // extended cast flags
        packet << uint32_t(cast.castTime);

        packet << uint32_t(cast.missileTravelTime);
        packet << float(cast.missilePitch);
        packet << int32_t(cast.ammoDisplayId);
        packet << uint8_t(0);                                   // destination of the cast index
        packet << uint32_t(0);                                  // immune school
        packet << uint32_t(0);                                  // immune value
        packet << uint32_t(0);                                  // predicted heal
        packet << uint8_t(0);                                   // predicted heal type
        packet << WoWGuid128();                                 // beacon

        packet.writeBits(static_cast<uint32_t>(cast.hitTargets.size()), 16);
        packet.writeBits(static_cast<uint32_t>(cast.missedTargets.size()), 16);
        packet.writeBits(static_cast<uint32_t>(cast.hitTargets.size()), 16);     // hit status
        packet.writeBits(static_cast<uint32_t>(cast.missedTargets.size()), 16);
        packet.writeBits(cast.hasPower ? 1 : 0, 9);
        packet.writeBit(false);                                 // remaining runes
        packet.writeBits(0, 16);                                // target points
        packet.flushBits();

        // the target data
        packet.writeBits(targetMask, 28);
        packet.writeBit(hasSource);
        packet.writeBit(hasDestination);
        packet.writeBit(false);                                 // orientation
        packet.writeBit(false);                                 // map
        packet.writeBits(static_cast<uint32_t>(targetName.length()), 7);
        packet.flushBits();

        packet << WoWGuid(unitTarget).toGuid128(cast.realmId, cast.mapId);
        packet << WoWGuid(itemTarget).toGuid128(cast.realmId, cast.mapId);

        if (hasSource)
        {
            packet << WoWGuid(cast.targets->getTransportSourceGuid()).toGuid128(cast.realmId, cast.mapId);
            packet << float(cast.targets->getSource().x);
            packet << float(cast.targets->getSource().y);
            packet << float(cast.targets->getSource().z);
        }

        if (hasDestination)
        {
            packet << WoWGuid(cast.targets->getTransportDestinationGuid()).toGuid128(cast.realmId, cast.mapId);
            packet << float(cast.targets->getDestination().x);
            packet << float(cast.targets->getDestination().y);
            packet << float(cast.targets->getDestination().z);
        }

        packet.writeString(targetName);

        for (const uint64_t hitTarget : cast.hitTargets)
            packet << WoWGuid(hitTarget).toGuid128(cast.realmId, cast.mapId);

        for (const auto& missedTarget : cast.missedTargets)
            packet << WoWGuid(missedTarget.targetGuid).toGuid128(cast.realmId, cast.mapId);

        for (size_t i = 0; i < cast.hitTargets.size(); ++i)
            packet << uint8_t(0);                               // hit status: none

        for (const auto& missedTarget : cast.missedTargets)
        {
            packet << uint8_t(missedTarget.hitResult);
            if (missedTarget.hitResult == SPELL_DID_HIT_REFLECT)
                packet << uint8_t(missedTarget.extendedHitResult);
        }

        if (cast.hasPower)
        {
            packet << int32_t(cast.powerValue);
            packet << uint8_t(cast.powerType);
        }
    }
}
