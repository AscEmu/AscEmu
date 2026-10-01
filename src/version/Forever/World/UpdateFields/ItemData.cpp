/*
Copyright (c) 2014-2026 AscEmu Team <http://ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ItemData.hpp"
#include "Definitions/ItemData.hpp"
#include "WireHelpers.hpp"

namespace AscEmu::Version::Forever::UpdateFields
{
    namespace
    {
        void writeItemEnchantmentCreateForever(ByteBuffer& data, Fields::ItemEnchantment const& enchantment)
        {
            data << enchantment.id
                 << enchantment.duration
                 << enchantment.charges
                 << enchantment.inactive;
        }
    }

    void writeItemDataCreate(ByteBuffer& data, Fields::ItemData const& fields, int32_t itemEntry)
    {
        // [FOREVER-VERIFIED] Captured Forever owner-visible ItemData create layout.
        //
        // Eight starter-item CREATE_OBJECT samples from the supplied retail
        // sniff have the same 279-byte ItemData body when their packed
        // owner GUIDs are 9 bytes. The typed field order below reproduces
        // that structure while keeping the one still-unidentified scalar
        // explicit instead of hiding it in padding.
        writeModernGuid(data, fields.owner);
        writeModernGuid(data, fields.containedIn);
        writeModernGuid(data, fields.creator);
        writeModernGuid(data, fields.giftCreator);

        data << fields.stackCount
             << fields.expiration;

        for (int32_t charge : fields.spellCharges)
            data << charge;

        data << fields.dynamicFlags;

        for (Fields::ItemEnchantment const& enchantment : fields.enchantment)
            writeItemEnchantmentCreateForever(data, enchantment);

        // Three zero uint32 values are present between Enchantment[13] and
        // Durability in every captured 70009 starter item. They correspond
        // to empty modern variable item-data collections.
        data << uint32_t(fields.modifiers.size())
             << uint32_t(fields.artifactPowers.size())
             << uint32_t(fields.gems.size());

        data << fields.durability
             << fields.maxDurability
             << fields.createPlayedTime
             << fields.createTime
             << fields.artifactXp
             << fields.itemAppearanceModId
             << fields.zoneFlags
             << fields.debugItemLevel;

        // ItemBonusKey is empty for the captured starter items. Keep the
        // create-layout scalar explicit until a non-empty retail sample
        // proves its concrete structure.
        data << uint32_t(0);

        // [FOREVER-VERIFIED] Forever serializes ItemContext as a 32-bit value in ItemData create.
        data << uint32_t(fields.context);

        // The captured create body carries the ItemID again in the trailing
        // item-instance record.
        data << itemEntry;

        // Empty trailing item-instance metadata in all captured starter items.
        data << uint32_t(0) << uint32_t(0) << uint16_t(0);
    }

    void writeItemDataUpdate(ByteBuffer& data, Fields::ItemData const& fields)
    {
        Definitions::ItemDataUpdate::writeUpdate(data, fields);
    }
}
