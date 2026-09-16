/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    struct TalentSpecEntry
    {
        uint32_t primaryTalentTree = 0;                         // Cata: locked primary talent tree
        std::vector<std::pair<uint32_t, uint8_t>> talents;      // talent id, rank
        std::vector<uint16_t> glyphs;                           // one entry per glyph slot
        uint32_t specializationId = 0;                          // Mop: chosen specialization, 0 = none
    };

    struct PetTalentData
    {
        uint32_t unspentPoints = 0;
        std::vector<std::pair<uint32_t, uint8_t>> talents;      // talent id, rank
    };

    class SmsgUpdateTalentData : public ManagedPacket
    {
    public:
        bool isPet = false;
        uint32_t freeTalentPoints = 0;
        uint8_t activeSpec = 0;
        std::vector<TalentSpecEntry> specs;
        PetTalentData petTalents;

        SmsgUpdateTalentData() : SmsgUpdateTalentData(0, 0, {})
        {
        }

        SmsgUpdateTalentData(uint32_t freeTalentPoints, uint8_t activeSpec, std::vector<TalentSpecEntry> specs) :
            ManagedPacket(SMSG_UPDATE_TALENT_DATA, 500),
            freeTalentPoints(freeTalentPoints),
            activeSpec(activeSpec),
            specs(std::move(specs))
        {
        }

        explicit SmsgUpdateTalentData(PetTalentData petTalents) :
            ManagedPacket(SMSG_UPDATE_TALENT_DATA, 50),
            isPet(true),
            petTalents(std::move(petTalents))
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (isPet)
            {
                // pet talents exist for WotLK and Cata clients only
                if (m_protocol.expansion != WoW::Expansion::_WotLK && m_protocol.expansion != WoW::Expansion::_Cata)
                    return false;

                packet << uint8_t(1);                                   // pet talent packet identificator
                packet << uint32_t(petTalents.unspentPoints);
                packet << uint8_t(petTalents.talents.size());
                for (const auto& [talentId, rank] : petTalents.talents)
                {
                    packet << uint32_t(talentId);
                    packet << uint8_t(rank);
                }

                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_WotLK ||
                m_protocol.expansion == WoW::Expansion::_Cata)
            {
                packet << uint8_t(0);                                   // sendPetTalents
                packet << uint32_t(freeTalentPoints);
                packet << uint8_t(specs.size());
                packet << uint8_t(activeSpec);

                for (const auto& spec : specs)
                {
                    if (m_protocol.expansion == WoW::Expansion::_Cata)
                        packet << uint32_t(spec.primaryTalentTree);

                    packet << uint8_t(spec.talents.size());
                    for (const auto& [talentId, rank] : spec.talents)
                    {
                        packet << uint32_t(talentId);
                        packet << uint8_t(rank);
                    }

                    packet << uint8_t(spec.glyphs.size());
                    for (const auto glyph : spec.glyphs)
                        packet << uint16_t(glyph);
                }

                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet << uint8_t(activeSpec);
                packet.writeBits(specs.size(), 19);

                auto wpos = std::make_unique<size_t[]>(specs.size());
                for (size_t i = 0; i < specs.size(); ++i)
                {
                    wpos[i] = packet.bitwpos();
                    packet.writeBits(0, 23);
                }

                packet.flushBits();

                for (size_t specId = 0; specId < specs.size(); ++specId)
                {
                    const auto& spec = specs[specId];

                    for (const auto glyph : spec.glyphs)
                        packet << uint16_t(glyph);

                    for (const auto& [talentId, rank] : spec.talents)
                        packet << uint16_t(talentId);

                    packet.putBits(wpos[specId], static_cast<int32_t>(spec.talents.size()), 23);

                    packet << uint32_t(spec.specializationId);
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
