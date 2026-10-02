/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include <array>
#include <cstdint>
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

        SmsgInitializeFactions(std::array<FactionReputation*, PLAYER_REPUTATION_COUNT> reputationByListId, ReputationMap const& reputation) :
            ManagedPacket(SMSG_INITIALIZE_FACTIONS, PLAYER_REPUTATION_COUNT * (1 + 4) + 32),
            reputationByListId(reputationByListId)
        {
            for (const auto* const factionReputation : reputationByListId)
            {
                if (factionReputation == nullptr)
                    continue;

                for (const auto& [factionId, storedReputation] : reputation)
                {
                    if (storedReputation.get() != factionReputation)
                        continue;

                    foreverFactions.push_back({ static_cast<int32_t>(factionId), static_cast<uint16_t>(factionReputation->flag), factionReputation->calcStanding() });
                    break;
                }
            }
        }

    protected:
        struct ForeverFactionData
        {
            int32_t factionId = 0;
            uint16_t flags = 0;
            int32_t standing = 0;
        };

        std::vector<ForeverFactionData> foreverFactions;

        size_t expectedSize() const override
        {
            if (m_protocol.isForever())
                return 8 + foreverFactions.size() * 15;

            return PLAYER_REPUTATION_COUNT * (1 + 4) + 32;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                // Forever 1.60.1.70124: two uint32 counts, then FactionData
                // (int32 FactionID, uint16 Flags, int32 Standing) and a matching
                // FactionBonusData list (int32 FactionID, one flushed bonus bit).
                packet << static_cast<uint32_t>(foreverFactions.size());
                packet << static_cast<uint32_t>(foreverFactions.size());

                for (const auto& faction : foreverFactions)
                {
                    packet << faction.factionId;
                    packet << faction.flags;
                    packet << faction.standing;
                }

                for (const auto& faction : foreverFactions)
                {
                    packet << faction.factionId;
                    packet.writeBit(false);
                    packet.flushBits();
                }

                return true;
            }
            else if (m_protocol.isMop())
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
