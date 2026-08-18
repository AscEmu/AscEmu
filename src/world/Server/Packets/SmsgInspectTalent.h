/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildMgr.hpp"
#include "Management/ItemInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgInspectTalent : public ManagedPacket
    {
    public:
        Player* inspectedPlayer {nullptr};

        SmsgInspectTalent() : SmsgInspectTalent(nullptr)
        {
        }

        explicit SmsgInspectTalent(Player* inspectedPlayer) :
            ManagedPacket(SMSG_INSPECT_TALENT, 1000),
            inspectedPlayer(inspectedPlayer)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (inspectedPlayer == nullptr)
                return 0;

            return 8 + 4 + 1 + 1                                                   // packed guid, talent points, spec count, active spec
                + inspectedPlayer->m_talentSpecsCount * 128                        // per-spec talent/glyph block
                + 4 + (EQUIPMENT_SLOT_END - EQUIPMENT_SLOT_START) * 32             // slot mask + per-slot item block
                + 24;                                                              // optional guild block
        }

        bool internalSerialise([[maybe_unused]] WorldPacket& packet) override
        {
            if (inspectedPlayer == nullptr)
                return false;

#if VERSION_STRING < Mop
            // TalentEntry only exposes TalentTree/RankID for pre-Mop clients (SMSG_INSPECT_TALENT
            // itself does not exist past Mop, where SMSG_INSPECT_RESULTS_UPDATE is used instead).
            if (!m_protocol.isMop())
            {
                ByteBuffer packedGuid;
                packedGuid.appendPackGuid(inspectedPlayer->getGuid());
                packet.append(packedGuid);

                packet << uint32_t(inspectedPlayer->getActiveSpec().getTalentPoints());
                packet << uint8_t(inspectedPlayer->m_talentSpecsCount);
                packet << uint8_t(inspectedPlayer->m_talentActiveSpec);
                for (uint8_t s = 0; s < inspectedPlayer->m_talentSpecsCount; ++s)
                {
                    const PlayerSpec playerSpec = inspectedPlayer->m_specs[s];

                    uint8_t talentCount = 0;
                    const auto talentCountPos = packet.wpos();
                    packet << uint8_t(talentCount);

                    const auto talentTabIds = getTalentTabPages(inspectedPlayer->getClass());
                    for (uint8_t i = 0; i < 3; ++i)
                    {
                        const uint32_t talentTabId = talentTabIds[i];
                        for (uint32_t j = 0; j < sTalentStore.getNumRows(); ++j)
                        {
                            const auto talentInfo = sTalentStore.lookupEntry(j);
                            if (talentInfo == nullptr)
                                continue;

                            if (talentInfo->TalentTree != talentTabId)
                                continue;

                            int32_t talentMaxRank = -1;
                            for (int32_t k = 4; k > -1; --k)
                            {
                                if (talentInfo->RankID[k] != 0 && inspectedPlayer->hasSpell(talentInfo->RankID[k]))
                                {
                                    talentMaxRank = k;
                                    break;
                                }
                            }

                            if (talentMaxRank < 0)
                                continue;

                            packet << uint32_t(talentInfo->TalentID);
                            packet << uint8_t(talentMaxRank);

                            ++talentCount;
                        }
                    }
                    packet.put<uint8_t>(talentCountPos, talentCount);

#ifdef FT_GLYPHS
                    packet << uint8_t(GLYPHS_COUNT);

                    for (const auto& glyph : playerSpec.getGlyphs())
                        packet << uint16_t(glyph);
#endif
                }

                uint32_t slotMask = 0;
                const auto slotMaskPos = packet.wpos();
                packet << uint32_t(slotMask);

                auto itemInterface = inspectedPlayer->getItemInterface();
                for (uint32_t i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
                {
                    const auto inventoryItem = itemInterface->GetInventoryItem(static_cast<uint16_t>(i));
                    if (!inventoryItem)
                        continue;

                    slotMask |= (1 << i);

                    packet << uint32_t(inventoryItem->getEntry());

                    uint16_t enchantMask = 0;
                    const auto enchantMaskPos = packet.wpos();

                    packet << uint16_t(enchantMask);

                    for (uint8_t slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
                    {
                        const uint32_t enchantId = inventoryItem->getEnchantmentId(slot);
                        if (!enchantId)
                            continue;

                        enchantMask |= (1 << slot);
                        packet << uint16_t(enchantId);
                    }
                    packet.put<uint16_t>(enchantMaskPos, enchantMask);

                    packet << uint16_t(0);
                    FastGUIDPack(packet, inventoryItem->getCreatorGuid());
                    packet << uint32_t(0);
                }
                packet.put<uint32_t>(slotMaskPos, slotMask);

                if (m_protocol.expansion >= WoW::Expansion::_Cata)
                {
                    if (Guild* guild = sGuildMgr.getGuildById(inspectedPlayer->getGuildId()))
                    {
                        packet << guild->getGUID();
                        packet << uint32_t(guild->getLevel());
                        packet << uint64_t(guild->getExperience());
                        packet << uint32_t(guild->getMembersCount());
                    }
                }
                return true;
            }
#endif
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
