/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/Units/Players/Player.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUpdateTalentData : public ManagedPacket
    {
    public:
        Player* player;

        SmsgUpdateTalentData() : SmsgUpdateTalentData(nullptr)
        {
        }

        SmsgUpdateTalentData(Player* player) :
            ManagedPacket(SMSG_UPDATE_TALENT_DATA, 500),
            player(player)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (player == nullptr)
                return false;

            if (m_protocol.expansion == WoW::Expansion::_WotLK ||
                m_protocol.expansion == WoW::Expansion::_Cata)
            {
                packet << uint8_t(0); // sendPetTalents
                packet << uint32_t(player->getActiveSpec().getTalentPoints()); // Free talent points
                packet << uint8_t(player->m_talentSpecsCount); // How many specs player has
                packet << uint8_t(player->m_talentActiveSpec); // Which spec is active right now

                if (player->m_talentSpecsCount > MAX_SPEC_COUNT)
                    player->m_talentSpecsCount = MAX_SPEC_COUNT;

                // Loop through specs
                for (uint8_t specId = 0; specId < player->m_talentSpecsCount; ++specId)
                {
                    PlayerSpec spec = player->m_specs[specId];

                    if (m_protocol.expansion == WoW::Expansion::_Cata)
                    {
                        // Send primary talent tree
                        packet << uint32_t(player->m_FirstTalentTreeLock);
                    }

                    // How many talents player has learnt
                    packet << uint8_t(spec.getTalents().size());
                    for (const auto& [talentId, rank] : spec.getTalents())
                    {
                        packet << uint32_t(talentId);
                        packet << uint8_t(rank);
                    }

                    // What kind of glyphs player has
                    packet << uint8_t(GLYPHS_COUNT);
                    for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
                    {
                        packet << uint16_t(player->getGlyph(specId, i));
                    }
                }

                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet << uint8_t(player->m_talentActiveSpec); // Which spec is active right now
                packet.writeBits(player->m_talentSpecsCount, 19);

                auto wpos = std::make_unique<size_t[]>(player->m_talentSpecsCount);
                for (uint8_t i = 0; i < player->m_talentSpecsCount; ++i)
                {
                    wpos[i] = packet.bitwpos();
                    packet.writeBits(0, 23);
                }

                packet.flushBits();

                for (uint8_t specId = 0; specId < player->m_talentSpecsCount; ++specId)
                {
                    PlayerSpec spec = player->m_specs[specId];

                    for (uint8_t i = 0; i < 6; ++i)
                    {
                        packet << uint16_t(player->getGlyph(specId, i));
                    }

                    int32_t talentCount = 0;
                    for (const auto& [talentId, rank] : spec.getTalents())
                    {
                        packet << uint16_t(talentId);
                        talentCount++;
                    }
                    packet.putBits(wpos[specId], talentCount, 23);
                    packet << uint32_t(spec.getTalentPoints());
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
