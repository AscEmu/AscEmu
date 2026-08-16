/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Macros/PetMacros.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUpdateTalentData : public ManagedPacket
    {
    public:
        Player* player;
        Pet* pet;

        SmsgUpdateTalentData() : SmsgUpdateTalentData(static_cast<Player*>(nullptr))
        {
        }

        SmsgUpdateTalentData(Player* player) :
            ManagedPacket(SMSG_UPDATE_TALENT_DATA, 500),
            player(player),
            pet(nullptr)
        {
        }

        // Pet talent variant (Objects/Units/Creatures/Pet.cpp - Pet::SendTalentsToOwner,
        // WotLK/Cata only build).
        explicit SmsgUpdateTalentData(Pet* pet) :
            ManagedPacket(SMSG_UPDATE_TALENT_DATA, 50),
            player(nullptr),
            pet(pet)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (pet != nullptr)
            {
                // Pet::SendTalentsToOwner is only ever compiled for WotLK/Cata server builds, and
                // getPetTalentPoints()/TalentEntry::TalentTree/TalentEntry::RankID only exist for
                // VERSION_STRING < Mop, so this whole branch must stay compile-time gated.
#if VERSION_STRING < Mop
                // Pet::SendTalentsToOwner is only ever compiled for WotLK/Cata server builds.
                if (m_protocol.expansion != WoW::Expansion::_WotLK && m_protocol.expansion != WoW::Expansion::_Cata)
                    return false;

                packet << uint8_t(1);                             // Pet talent packet identificator
                packet << uint32_t(pet->getPetTalentPoints());    // Unspent talent points

                uint8_t count = 0;
                const size_t countPos = packet.wpos();
                packet << uint8_t(0);                             // Amount of known talents (will be filled later)

                WDB::Structures::CreatureFamilyEntry const* cfe = sCreatureFamilyStore.lookupEntry(pet->GetCreatureProperties()->Family);
                if (!cfe || static_cast<int32_t>(cfe->talenttree) < 0)
                    return false;

                // go through talent trees
                for (uint32_t tte_id = PET_TALENT_TREE_START; tte_id <= PET_TALENT_TREE_END; tte_id++)
                {
                    auto talent_tab = sTalentTabStore.lookupEntry(tte_id);
                    if (talent_tab == nullptr)
                        continue;

                    // check if we match talent tab
                    if (!(talent_tab->PetTalentMask & (1 << cfe->talenttree)))
                        continue;

                    for (uint32_t t_id = 1; t_id < sTalentStore.getNumRows(); t_id++)
                    {
                        // get talent entries for our talent tree
                        auto talent = sTalentStore.lookupEntry(t_id);
                        if (talent == nullptr)
                            continue;

                        if (talent->TalentTree != tte_id)
                            continue;

                        // check our spells
                        for (uint8_t j = 0; j < 5; j++)
                            if (talent->RankID[j] > 0 && pet->hasSpell(talent->RankID[j]))
                            {
                                // if we have the spell, include it in packet
                                packet << talent->TalentID;   // Talent ID
                                packet << j;                  // Rank
                                ++count;
                            }
                    }
                    // tab loaded, we can exit
                    break;
                }

                // fill count of talents
                packet.put<uint8_t>(countPos, count);

                return true;
#else
                return false;
#endif
            }

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
                if (player->m_talentSpecsCount > MAX_SPEC_COUNT)
                    player->m_talentSpecsCount = MAX_SPEC_COUNT;

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

                    for (uint8_t i = 0; i < GLYPHS_COUNT; ++i)
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

                    // We do not track Mop talent specializations anywhere yet,
                    // so send 0 ("no specialization chosen")
                    packet << uint32_t(0);
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
