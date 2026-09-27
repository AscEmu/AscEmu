/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/WorldPacket.hpp"
#include "Server/World.h"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/SpellCastTargets.hpp"
#include "WoWGuid.hpp"

#include <cstddef>
#include <cstdint>

namespace AscEmu::Packets::ForeverSpellPacket
{
    inline bool readPackedGuid(WorldPacket& packet, WoWGuid& guid)
    {
        std::size_t consumed = 0;
        if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), guid, consumed))
            return false;

        packet.rpos(packet.rpos() + consumed);
        return true;
    }

    inline void writePackedGuid(WorldPacket& packet, WoWGuid const& guid)
    {
        const auto packedGuid = guid.packModern();
        packet.append(packedGuid.data(), packedGuid.size());
    }

    inline bool isModernUnitTarget(ModernHighGuid type)
    {
        return type == ModernHighGuid::Player ||
               type == ModernHighGuid::Creature ||
               type == ModernHighGuid::Vehicle ||
               type == ModernHighGuid::Pet ||
               type == ModernHighGuid::Corpse;
    }

    inline bool isLegacyWorldObject(HighGuid type)
    {
        switch (type)
        {
            case HighGuid::Corpse:
            case HighGuid::DynamicObject:
            case HighGuid::GameObject:
            case HighGuid::Unit:
            case HighGuid::Pet:
            case HighGuid::Vehicle:
            case HighGuid::AreaTrigger:
                return true;
            default:
                return false;
        }
    }

    inline WoWGuid toModernGuid(WoWGuid const& guid, uint32_t protocolRealmId, uint16_t mapId)
    {
        if (!guid)
            return WoWGuid::createModernEmpty();

        const uint32_t realmId = isLegacyWorldObject(guid.getHighType())
            ? ::World::getInstance().settings.battleNetComm.realmId
            : protocolRealmId;

        return WoWGuid::createModernFromLegacy(guid.getRawGuid(), realmId, mapId, 0);
    }

    inline bool readTargetData(WorldPacket& packet, SpellCastTargets& targets)
    {
        uint32_t targetFlags = 0;
        packet >> targetFlags;
        targets.setTargetMask(targetFlags);

        WoWGuid unitGuid;
        WoWGuid itemGuid;
        WoWGuid housingGuid;
        if (!readPackedGuid(packet, unitGuid) || !readPackedGuid(packet, itemGuid) || !readPackedGuid(packet, housingGuid))
            return false;

        const bool housingIsResident = packet.readBit();
        const bool hasSource = packet.readBit();
        const bool hasDestination = packet.readBit();
        const bool hasOrientation = packet.readBit();
        const bool hasMapId = packet.readBit();
        const uint32_t targetNameLength = packet.readBits(7);
        (void)housingIsResident;
        (void)housingGuid;

        if (hasSource)
        {
            WoWGuid transportGuid;
            if (!readPackedGuid(packet, transportGuid))
                return false;

            LocationVector source;
            packet >> source.x >> source.y >> source.z;
            targets.setTransportSourceGuid(transportGuid.toLegacyRaw());
            targets.setSource(source);
        }

        if (hasDestination)
        {
            WoWGuid transportGuid;
            if (!readPackedGuid(packet, transportGuid))
                return false;

            LocationVector destination;
            packet >> destination.x >> destination.y >> destination.z;
            targets.setTransportDestinationGuid(transportGuid.toLegacyRaw());
            targets.setDestination(destination);
        }

        if (hasOrientation)
            packet.readSkip<float>();
        if (hasMapId)
            packet.readSkip<int32_t>();

        if (targetNameLength != 0)
            targets.setStringTarget(packet.readString(targetNameLength));

        if (unitGuid)
        {
            if (unitGuid.getModernHighType() == ModernHighGuid::GameObject)
                targets.setGameObjectTarget(unitGuid.toLegacyRaw());
            else if (isModernUnitTarget(unitGuid.getModernHighType()))
                targets.setUnitTarget(unitGuid.toLegacyRaw());
        }

        if (itemGuid)
            targets.setItemTarget(itemGuid.toLegacyRaw());

        return !packet.hadReadFailure();
    }

    inline void writeTargetData(WorldPacket& packet, SpellCastTargets const& targets, uint32_t protocolRealmId, uint16_t mapId)
    {
        packet << static_cast<uint32_t>(targets.getTargetMask());

        uint64_t targetRawGuid = targets.getUnitTargetGuid();
        if (targetRawGuid == 0)
            targetRawGuid = targets.getGameObjectTargetGuid();

        writePackedGuid(packet, toModernGuid(WoWGuid(targetRawGuid), protocolRealmId, mapId));
        writePackedGuid(packet, toModernGuid(WoWGuid(targets.getItemTargetGuid()), protocolRealmId, mapId));
        writePackedGuid(packet, WoWGuid::createModernEmpty()); // HousingGUID

        const bool hasSource = (targets.getTargetMask() & TARGET_FLAG_SOURCE_LOCATION) != 0 && targets.getSource().isSet();
        const bool hasDestination = (targets.getTargetMask() & TARGET_FLAG_DEST_LOCATION) != 0 && targets.getDestination().isSet();
        const bool hasTargetString = (targets.getTargetMask() & TARGET_FLAG_STRING) != 0 && !targets.getStringTarget().empty();

        packet.writeBit(false); // HousingIsResident
        packet.writeBit(hasSource);
        packet.writeBit(hasDestination);
        packet.writeBit(false); // Orientation
        packet.writeBit(false); // MapID
        packet.writeBits(static_cast<uint32_t>(hasTargetString ? targets.getStringTarget().size() : 0), 7);
        packet.flushBits();

        if (hasSource)
        {
            writePackedGuid(packet, toModernGuid(WoWGuid(targets.getTransportSourceGuid()), protocolRealmId, mapId));
            const auto source = targets.getSource();
            packet << source.x << source.y << source.z;
        }

        if (hasDestination)
        {
            writePackedGuid(packet, toModernGuid(WoWGuid(targets.getTransportDestinationGuid()), protocolRealmId, mapId));
            const auto destination = targets.getDestination();
            packet << destination.x << destination.y << destination.z;
        }

        if (hasTargetString)
            packet.writeString(targets.getStringTarget());
    }
}
