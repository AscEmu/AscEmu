/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once
#include "ManagedPacket.h"
#include <cstdint>
#include <vector>
namespace AscEmu::Packets
{
    enum class ItemCollectionType : uint8_t
    {
        None = 0, Toy = 1, Heirloom = 2, Transmog = 3, TransmogSetFavorite = 4, RuneforgeLegendaryAbility = 5, TransmogIllusion = 6, WarbandScene = 7, Room = 8, ExteriorFixture = 9, RoomTheme = 10, RoomMaterial = 11
    };
    struct ItemCollectionItemData
    {
        int32_t id = 0;
        ItemCollectionType type = ItemCollectionType::None;
        int64_t unknown1110 = 0;
        int32_t flags = 0;
    };
    class SmsgAccountItemCollectionData : public ManagedPacket
    {
    public:
        uint32_t unknown1110_1 = 0;
        ItemCollectionType type = ItemCollectionType::None;
        bool unknown1110_2 = false;
        std::vector<ItemCollectionItemData> items;
        explicit SmsgAccountItemCollectionData(ItemCollectionType type = ItemCollectionType::None) : ManagedPacket(SMSG_ACCOUNT_ITEM_COLLECTION_DATA, 10), unknown1110_1(type == ItemCollectionType::WarbandScene ? 0x682CBBA0U : 0U), type(type) {}
    protected:
        size_t expectedSize() const override { return 10 + items.size() * 17; }
        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;
            packet << unknown1110_1 << static_cast<uint8_t>(type) << static_cast<uint32_t>(items.size());
            for (const ItemCollectionItemData& item : items)
                packet << item.id << static_cast<uint8_t>(item.type) << item.unknown1110 << item.flags;
            packet.writeBit(unknown1110_2);
            packet.flushBits();
            return true;
        }
        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
