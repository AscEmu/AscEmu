/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgInitializeFactions : public ManagedPacket
    {
    public:
        std::array<FactionReputation*, PLAYER_REPUTATION_COUNT> reputationByListId{};

        SmsgInitializeFactions() : SmsgInitializeFactions(std::array<FactionReputation*, PLAYER_REPUTATION_COUNT>{})
        {
        }

        explicit SmsgInitializeFactions(std::array<FactionReputation*, PLAYER_REPUTATION_COUNT> reputationByListId) :
            ManagedPacket(SMSG_INITIALIZE_FACTIONS, PLAYER_REPUTATION_COUNT * (1 + 4) + 32),
            reputationByListId(reputationByListId)
        {
        }

    protected:
        size_t expectedSize() const override { return PLAYER_REPUTATION_COUNT * (1 + 4) + 32; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isTheWarWithinOrLater())
            {
                // 11.x: the factions are listed with their id instead of being indexed by the reputation list: faction
                // count, bonus count, every faction with id, flags (16 bit) and standing, then every bonus with id and bit
                std::vector<std::pair<int32_t, uint32_t>> factionsByListId;   // reputation index, faction id
                for (const auto& [factionId, factionEntry] : sFactionStore)
                {
                    if (factionEntry.reputationIndex >= 0 && static_cast<size_t>(factionEntry.reputationIndex) < reputationByListId.size())
                        factionsByListId.emplace_back(factionEntry.reputationIndex, factionId);
                }
                std::ranges::sort(factionsByListId);

                packet << uint32_t(factionsByListId.size());
                packet << uint32_t(factionsByListId.size());

                for (const auto& [listId, factionId] : factionsByListId)
                {
                    const auto* const factionReputation = reputationByListId[static_cast<size_t>(listId)];
                    packet << int32_t(factionId);
                    packet << uint16_t(factionReputation != nullptr ? factionReputation->flag : 0);
                    packet << int32_t(factionReputation != nullptr ? factionReputation->calcStanding() : 0);
                }

                for (const auto& [listId, factionId] : factionsByListId)
                {
                    packet << int32_t(factionId);
                    packet.writeBit(false);             // faction has a bonus
                    packet.flushBits();
                }

                return true;
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight())
            {
                // 256 (6.x), 300 (7.x), 350 (8.x), 400 (9.x) or 1000 (10.x) factions: flags (16 bit in 10.x) and
                // standing, then one bonus bit per faction
                const uint16_t factionCount = m_protocol.isWoD() ? 256 : m_protocol.isLegion() ? 300 : m_protocol.isDragonflight() ? 1000 : m_protocol.isShadowlands() ? 400 : 350;

                for (uint16_t i = 0; i < factionCount; ++i)
                {
                    const auto* const factionReputation = i < reputationByListId.size() ? reputationByListId[i] : nullptr;
                    const uint8_t flags = factionReputation != nullptr ? factionReputation->flag : 0;
                    if (m_protocol.isDragonflight())
                        packet << uint16_t(flags);
                    else
                        packet << uint8_t(flags);
                    packet << int32_t(factionReputation != nullptr ? factionReputation->calcStanding() : 0);
                }

                for (uint16_t i = 0; i < factionCount; ++i)
                    packet.writeBit(false);
                packet.flushBits();
                return true;
            }

            if (m_protocol.isMop())
            {
                ByteBuffer buffer;

                for (const auto* const factionReputation : reputationByListId)
                {
                    if (factionReputation == nullptr)
                    {
                        packet << static_cast<uint8_t>(0);
                        packet << static_cast<uint32_t>(0);
                    }
                    else
                    {
                        packet << factionReputation->flag;
                        packet << static_cast<uint32_t>(factionReputation->calcStanding());
                    }
                    buffer.writeBit(0);
                }

                buffer.flushBits();

                packet.append(buffer);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << uint32_t(PLAYER_REPUTATION_COUNT);

                for (const auto* const factionReputation : reputationByListId)
                {
                    if (factionReputation == nullptr)
                    {
                        packet << uint8_t(0);
                        packet << uint32_t(0);
                    }
                    else
                    {
                        packet << uint8_t(factionReputation->flag);
                        packet << uint32_t(factionReputation->calcStanding());
                    }
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
