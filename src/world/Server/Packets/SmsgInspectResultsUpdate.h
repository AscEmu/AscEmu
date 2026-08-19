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
    class SmsgInspectResultsUpdate : public ManagedPacket
    {
    public:
        Player* inspectedPlayer {nullptr};

        SmsgInspectResultsUpdate() : SmsgInspectResultsUpdate(nullptr)
        {
        }

        explicit SmsgInspectResultsUpdate(Player* inspectedPlayer) :
            ManagedPacket(SMSG_INSPECT_RESULTS_UPDATE, 1000),
            inspectedPlayer(inspectedPlayer)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (inspectedPlayer == nullptr)
                return 0;

            return 8 + 8                                                           // player guid, guild guid
                + (EQUIPMENT_SLOT_END - EQUIPMENT_SLOT_START) * 32                // per-slot item/enchant block
                + 32                                                               // glyphs
                + 4 + 512;                                                        // talent points + talent block
        }

        bool internalSerialise([[maybe_unused]] WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            if (inspectedPlayer == nullptr)
                return false;

            // TalentEntry only exposes playerClass/SpellId for Mop clients (SMSG_INSPECT_RESULTS_UPDATE
            // itself does not exist before Mop, where SMSG_INSPECT_TALENT is used instead).

            if (m_protocol.isMop())
            {
                [[maybe_unused]] uint32_t talentPoints = 41;
                uint32_t slotCount = 0;
                uint32_t glyphCount = 0;
                uint32_t talentCount = 0;

                WoWGuid guid = inspectedPlayer->getGuid();
                Guild* guild = sGuildMgr.getGuildById(inspectedPlayer->getGuildId());
                WoWGuid guildGuid = guild ? guild->getGUID() : 0;

                ByteBuffer enchantData;

                packet.writeBit(guild ? 1 : 0);
                packet.writeBit(guid[2]);

                if (guild)
                {
                    packet.writeBit(guildGuid[7]);
                    packet.writeBit(guildGuid[0]);
                    packet.writeBit(guildGuid[5]);
                    packet.writeBit(guildGuid[3]);
                    packet.writeBit(guildGuid[2]);
                    packet.writeBit(guildGuid[4]);
                    packet.writeBit(guildGuid[6]);
                    packet.writeBit(guildGuid[1]);
                }

                packet.writeBit(guid[4]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[7]);

                size_t slotCountBitPos = packet.bitwpos();
                packet.writeBits(0, 20);

                packet.writeBit(guid[0]);

                auto itemInterface = inspectedPlayer->getItemInterface();
                for (uint32_t itemSlot = EQUIPMENT_SLOT_START; itemSlot < EQUIPMENT_SLOT_END; ++itemSlot)
                {
                    const auto inventoryItem = itemInterface->GetInventoryItem(static_cast<uint16_t>(itemSlot));
                    if (!inventoryItem)
                        continue;

                    ++slotCount;

                    WoWGuid creatorGuid = inventoryItem->getCreatorGuid();
                    uint32_t enchantCount = 0;

                    packet.writeBit(creatorGuid[1]);
                    packet.writeBit(0); // unk1
                    packet.writeBit(0);
                    packet.writeBit(creatorGuid[3]);

                    size_t enchCountBitPos = packet.bitwpos();
                    packet.writeBits(0, 21);

                    packet.writeBit(creatorGuid[2]);
                    packet.writeBit(creatorGuid[6]);
                    packet.writeBit(creatorGuid[4]);

                    packet.writeBit(0); // unk2

                    packet.writeBit(creatorGuid[0]);
                    packet.writeBit(creatorGuid[5]);
                    packet.writeBit(creatorGuid[7]);

                    enchantData.writeByteSeq(creatorGuid[3]);
                    enchantData << uint32_t(0);

                    for (uint8_t enchantSlot = 0; enchantSlot < MAX_ENCHANTMENT_SLOT; ++enchantSlot)
                    {
                        uint32_t enchantId = inventoryItem->getEnchantmentId(enchantSlot);

                        if (!enchantId)
                            continue;

                        ++enchantCount;
                        enchantData << enchantId;
                        enchantData << uint8_t(enchantSlot);
                    }

                    packet.putBits(enchCountBitPos, enchantCount, 21);

                    enchantData << inventoryItem->getEntry();

                    enchantData.writeByteSeq(creatorGuid[6]);
                    enchantData.writeByteSeq(creatorGuid[4]);
                    enchantData.writeByteSeq(creatorGuid[7]);
                    enchantData.writeByteSeq(creatorGuid[2]);

                    enchantData.writeByteSeq(creatorGuid[5]);

                    enchantData << uint8_t(itemSlot);

                    enchantData.writeByteSeq(creatorGuid[0]);
                    enchantData.writeByteSeq(creatorGuid[1]);
                }

                packet.putBits(slotCountBitPos, slotCount, 20);

                size_t glyphCountBitPos = packet.bitwpos();
                packet.writeBits(0, 23);

                size_t talentCountBitPos = packet.bitwpos();
                packet.writeBits(0, 23);

                packet.writeBit(guid[6]);
                packet.writeBit(guid[1]);
                packet.flushBits();

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[2]);

                packet.append(enchantData);

                if (guild)
                {
                    packet.writeByteSeq(guildGuid[6]);
                    packet.writeByteSeq(guildGuid[2]);
                    packet.writeByteSeq(guildGuid[5]);
                    packet.writeByteSeq(guildGuid[0]);

                    packet << uint32_t(guild->getMembersCount());

                    packet.writeByteSeq(guildGuid[4]);
                    packet.writeByteSeq(guildGuid[7]);

                    packet << uint64_t(guild->getExperience());

                    packet.writeByteSeq(guildGuid[1]);

                    packet << uint32_t(guild->getLevel());

                    packet.writeByteSeq(guildGuid[3]);
                }

                packet.writeByteSeq(guid[5]);

                const PlayerSpec playerSpec = inspectedPlayer->m_specs[inspectedPlayer->m_talentActiveSpec];

                for (const auto& glyph : playerSpec.getGlyphs())
                {
                    if (glyph)
                    {
                        ++glyphCount;
                        packet << glyph;
                    }
                }

                packet.putBits(glyphCountBitPos, glyphCount, 23);

                packet.writeByteSeq(guid[0]);

                // No Mop talent anywhere yet, so send 0 ("no specialization chosen")
                packet << uint32_t(0);

                for (uint32_t j = 0; j < sTalentStore.getNumRows(); ++j)
                {
                    const auto talentInfo = sTalentStore.lookupEntry(j);
                    if (talentInfo == nullptr)
                        continue;

                    if (talentInfo->playerClass != inspectedPlayer->getClass())
                        continue;

                    if (!inspectedPlayer->hasSpell(talentInfo->SpellId))
                        continue;

                    packet << uint16_t(talentInfo->TalentID);
                    ++talentCount;
                }

                packet.putBits(talentCountBitPos, talentCount, 23);

                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[6]);

                return true;
            }
#endif
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
