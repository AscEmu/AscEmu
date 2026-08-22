/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ItemInterface.h"
#include "Management/Guild/Guild.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Players/Player.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgMirrorimageComponentedData : public ManagedPacket
    {
    public:
        WoWGuid guid;
        Player* caster {nullptr};

        SmsgMirrorimageComponentedData() : SmsgMirrorimageComponentedData(0, nullptr)
        {
        }

        SmsgMirrorimageComponentedData(uint64_t guid, Player* caster) :
            ManagedPacket(SMSG_MIRRORIMAGE_COMPONENTED_DATA, 8 + 4 + 8 + 8 + (11 * 4)),
            guid(guid),
            caster(caster)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (caster == nullptr)
                return false;

            const WoWGuid guildGuid = caster->isInGuild() ? WoWGuid(caster->getGuild()->getGUID()) : WoWGuid(uint64_t(0));

            packet.writeBit(guid[4]);
            packet.writeBit(guildGuid[3]);
            packet.writeBit(guildGuid[6]);
            packet.writeBit(guid[0]);
            packet.writeBit(guildGuid[7]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[5]);
            packet.writeBit(guildGuid[2]);
            packet.writeBit(guildGuid[1]);
            packet.writeBit(guid[7]);
            packet.writeBit(guildGuid[4]);
            packet.writeBit(guildGuid[0]);
            packet.writeBit(guid[2]);
            packet.writeBit(guildGuid[5]);
            packet.writeBit(guid[3]);
            packet.writeBits(11, 22);    // item slot count, always 11
            packet.writeBit(guid[6]);
            packet.flushBits();

            packet << uint8_t(caster->getHairColor());
            packet << uint32_t(caster->getDisplayId());
            packet << uint8_t(caster->getFacialFeatures());

            packet.writeByteSeq(guildGuid[6]);
            packet.writeByteSeq(guildGuid[4]);
            packet.writeByteSeq(guid[7]);
            packet.writeByteSeq(guildGuid[1]);
            packet.writeByteSeq(guid[3]);
            packet << uint8_t(caster->getHairStyle());
            packet.writeByteSeq(guid[2]);
            packet.writeByteSeq(guid[0]);
            packet << uint8_t(caster->getRace());
            packet << uint8_t(caster->getSkinColor());
            packet.writeByteSeq(guildGuid[7]);

            static const uint32_t imageItemSlots[] =
            {
                EQUIPMENT_SLOT_HEAD,
                EQUIPMENT_SLOT_SHOULDERS,
                EQUIPMENT_SLOT_BODY,
                EQUIPMENT_SLOT_CHEST,
                EQUIPMENT_SLOT_WAIST,
                EQUIPMENT_SLOT_LEGS,
                EQUIPMENT_SLOT_FEET,
                EQUIPMENT_SLOT_WRISTS,
                EQUIPMENT_SLOT_HANDS,
                EQUIPMENT_SLOT_TABARD,
                EQUIPMENT_SLOT_BACK
            };

            for (uint8_t i = 0; i < 11; ++i)
            {
                Item* item = caster->getItemInterface()->GetInventoryItem(static_cast<int16_t>(imageItemSlots[i]));
                if (item != nullptr)
                    packet << uint32_t(item->getItemProperties()->DisplayInfoID);
                else
                    packet << uint32_t(0);
            }

            packet.writeByteSeq(guid[4]);
            packet << uint8_t(caster->getClass());
            packet << uint8_t(caster->getGender());
            packet << uint8_t(caster->getFace());
            packet.writeByteSeq(guid[5]);
            packet.writeByteSeq(guildGuid[3]);
            packet.writeByteSeq(guildGuid[2]);
            packet.writeByteSeq(guid[1]);
            packet.writeByteSeq(guildGuid[0]);
            packet.writeByteSeq(guildGuid[5]);
            packet.writeByteSeq(guid[6]);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
