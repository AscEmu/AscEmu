/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include <array>
#include <cstdint>

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
