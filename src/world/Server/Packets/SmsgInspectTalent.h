/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildMgr.hpp"
#include "Management/ItemInterface.h"
#include "Objects/Units/Players/Player.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    struct InspectSpecEntry
    {
        std::vector<std::pair<uint32_t, uint8_t>> talents;      // talent id, highest known rank
        std::vector<uint16_t> glyphs;                           // one entry per glyph slot
    };

    class SmsgInspectTalent : public ManagedPacket
    {
    public:
        Player* inspectedPlayer {nullptr};
        uint32_t freeTalentPoints = 0;
        uint8_t activeSpec = 0;
        std::vector<InspectSpecEntry> specs;

        SmsgInspectTalent() : SmsgInspectTalent(nullptr, 0, 0, {})
        {
        }

        SmsgInspectTalent(Player* inspectedPlayer, uint32_t freeTalentPoints, uint8_t activeSpec, std::vector<InspectSpecEntry> specs) :
            ManagedPacket(SMSG_INSPECT_TALENT, 1000),
            inspectedPlayer(inspectedPlayer),
            freeTalentPoints(freeTalentPoints),
            activeSpec(activeSpec),
            specs(std::move(specs))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (inspectedPlayer == nullptr)
                return 0;

            return 8 + 4 + 1 + 1                                                   // packed guid, talent points, spec count, active spec
                + specs.size() * 128                                               // per-spec talent/glyph block
                + 4 + (EQUIPMENT_SLOT_END - EQUIPMENT_SLOT_START) * 32             // slot mask + per-slot item block
                + 24;                                                              // optional guild block
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (inspectedPlayer == nullptr)
                return false;

            // replaced by SMSG_INSPECT_RESULTS_UPDATE in Mop
            if (m_protocol.isMop())
                return false;

            ByteBuffer packedGuid;
            packedGuid.appendPackGuid(inspectedPlayer->getGuid());
            packet.append(packedGuid);

            packet << uint32_t(freeTalentPoints);
            packet << uint8_t(specs.size());
            packet << uint8_t(activeSpec);
            for (const auto& spec : specs)
            {
                packet << uint8_t(spec.talents.size());
                for (const auto& [talentId, rank] : spec.talents)
                {
                    packet << uint32_t(talentId);
                    packet << uint8_t(rank);
                }

                // glyphs exist since WotLK
                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    packet << uint8_t(spec.glyphs.size());
                    for (const auto glyph : spec.glyphs)
                        packet << uint16_t(glyph);
                }
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

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
