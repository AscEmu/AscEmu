/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/Units/Players/TraitManager.hpp"

namespace AscEmu::Packets
{
    class CmsgTraitsCommitConfig final : public ManagedPacket
    {
    public:
        Traits::Config config;

        CmsgTraitsCommitConfig() : ManagedPacket(CMSG_TRAITS_COMMIT_CONFIG, 16) { }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            uint32_t entryCount = 0;
            uint32_t subtreeCount = 0;
            int32_t configType = 0;
            packet >> config.id >> configType >> entryCount >> subtreeCount;
            // [FOREVER-VERIFIED] Classic 1.60 uses wire type 4 for combat trait configs.
            // Keep the internal config type independent from the wire representation.
            if (configType == 4)
                config.type = Traits::ConfigType::Combat;
            else
                config.type = static_cast<Traits::ConfigType>(configType);

            if (packet.hadReadFailure() || entryCount > 100 || subtreeCount > 10)
                return false;

            switch (config.type)
            {
                case Traits::ConfigType::Combat: packet >> config.specializationId >> config.combatConfigFlags >> config.localIdentifier; break;
                case Traits::ConfigType::Profession: packet >> config.skillLineId; break;
                case Traits::ConfigType::Generic: packet >> config.traitSystemId >> config.variationId; break;
                default: return false;
            }

            auto readEntry = [&packet](Traits::Entry& entry)
            {
                packet >> entry.traitNodeId >> entry.traitNodeEntryId >> entry.rank >> entry.grantedRanks >> entry.bonusRanks;
                return !packet.hadReadFailure();
            };

            config.entries.resize(entryCount);
            for (auto& entry : config.entries)
                if (!readEntry(entry))
                    return false;

            config.subTrees.resize(subtreeCount);
            for (auto& subtree : config.subTrees)
            {
                uint32_t subtreeEntryCount = 0;
                packet >> subtree.traitSubTreeId >> subtreeEntryCount;
                if (packet.hadReadFailure() || subtreeEntryCount > 100)
                    return false;
                subtree.entries.resize(subtreeEntryCount);
                for (auto& entry : subtree.entries)
                    if (!readEntry(entry))
                        return false;
                subtree.active = packet.readBit();
            }

            const uint32_t nameLength = packet.readBits(9);
            if (nameLength > packet.remaining())
                return false;
            config.name = packet.readString(nameLength);
            packet >> config.savedConfigId >> config.savedLocalIdentifier;
            return !packet.hadReadFailure();
        }
    };
}
