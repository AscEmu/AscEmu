/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#include <cstddef>
#include <cstdint>

// Central inventory layout definition.
//
// The top-level values describe AscEmu's logical player inventory slots for
// the selected client build. Build-specific descriptor sizes live in
// InventoryLayout::Fields. Forever maps the core-facing slots it supports to
// ActivePlayerData::InvSlots through InventoryLayout::Forever.
namespace InventoryLayout
{
    using Slot = uint16_t;

    static inline constexpr int16_t NoSlotAvailable = -1;
    static inline constexpr int16_t SlotNotSet = -1;

    // AscEmu logical slot numbering for the selected client build. Keep the
    // ordering here as the single source of truth for all slot ranges.
    static inline constexpr Slot EquipmentStart = 0;
    static inline constexpr Slot EquipmentCount = 19;
    static inline constexpr Slot EquipmentEnd = EquipmentStart + EquipmentCount;

    static inline constexpr Slot BagStart = EquipmentEnd;
    static inline constexpr Slot BagCount = 4;
    static inline constexpr Slot BagEnd = BagStart + BagCount;

    static inline constexpr Slot InventorySlotCount = BagEnd - EquipmentStart;
    static inline constexpr Slot CharacterItemCount = InventorySlotCount;
    static inline constexpr Slot VisibleItemCount = EquipmentCount;

    static inline constexpr Slot PackStart = BagEnd;
    static inline constexpr Slot PackCount = 16;
    static inline constexpr Slot PackEnd = PackStart + PackCount;

    static inline constexpr Slot BankStart = PackEnd;

#if VERSION_STRING == Classic
    static inline constexpr Slot BankCount = 24;
    static inline constexpr Slot BankBagCount = 6;
    static inline constexpr Slot KeyringCount = 20;
    static inline constexpr Slot CurrencyTokenCount = 0;
#elif VERSION_STRING == TBC
    static inline constexpr Slot BankCount = 28;
    static inline constexpr Slot BankBagCount = 7;
    static inline constexpr Slot KeyringCount = 32;
    static inline constexpr Slot CurrencyTokenCount = 0;
#else
    static inline constexpr Slot BankCount = 28;
    static inline constexpr Slot BankBagCount = 7;
    static inline constexpr Slot KeyringCount = 32;
    static inline constexpr Slot CurrencyTokenCount = 32;
#endif

    static inline constexpr Slot BankEnd = BankStart + BankCount;

    static inline constexpr Slot BankBagStart = BankEnd;
    static inline constexpr Slot BankBagEnd = BankBagStart + BankBagCount;

    static inline constexpr Slot BuybackStart = BankBagEnd;
    static inline constexpr Slot BuybackCount = 12;
    static inline constexpr Slot BuybackEnd = BuybackStart + BuybackCount;

    static inline constexpr Slot KeyringStart = BuybackEnd;
    static inline constexpr Slot KeyringEnd = KeyringStart + KeyringCount;

    static inline constexpr Slot CurrencyTokenStart = KeyringEnd;
    static inline constexpr Slot CurrencyTokenEnd = CurrencyTokenStart + CurrencyTokenCount;

    static inline constexpr Slot MaxSlot = CurrencyTokenEnd;

    static_assert(EquipmentEnd == BagStart);
    static_assert(BagEnd == PackStart);
    static_assert(PackEnd == BankStart);
    static_assert(BankEnd == BankBagStart);
    static_assert(BankBagEnd == BuybackStart);
    static_assert(BuybackEnd == KeyringStart);
    static_assert(KeyringEnd == CurrencyTokenStart);
    static_assert(CurrencyTokenEnd == MaxSlot);

    // Build-specific sizes of the legacy WoWPlayer descriptor arrays. These are
    // wire/storage field counts, not AscEmu logical slot IDs.
    namespace Fields
    {
        static inline constexpr Slot VisibleItemCount = EquipmentCount;
        static inline constexpr Slot InventorySlotCount = InventoryLayout::InventorySlotCount;
        static inline constexpr Slot PackSlotCount = PackCount;
        static inline constexpr Slot BankSlotCount = BankCount;
        static inline constexpr Slot BankBagSlotCount = BankBagCount;
        static inline constexpr Slot BuybackCount = InventoryLayout::BuybackCount;
        static inline constexpr Slot KeyringSlotCount = KeyringCount;
        static inline constexpr Slot CurrencyTokenSlotCount = CurrencyTokenCount;
    }

    namespace Forever
    {
        // Modern Forever ActivePlayerData::InvSlots wire layout. These offsets
        // are deliberately independent from the legacy logical slot counts.
        static inline constexpr std::size_t InvSlotCount = 105;

        static inline constexpr std::size_t EquipmentOffset = 0;
        static inline constexpr std::size_t EquipmentCount = 19;

        static inline constexpr std::size_t ProfessionEquipmentOffset = 19;
        static inline constexpr std::size_t ProfessionEquipmentCount = 11;

        static inline constexpr std::size_t BagOffset = 30;
        static inline constexpr std::size_t BagCount = 4;

        static inline constexpr std::size_t ReagentBagOffset = 34;
        static inline constexpr std::size_t ReagentBagCount = 1;

        static inline constexpr std::size_t PackOffset = 35;
        static inline constexpr std::size_t PackCount = 28;

        static inline constexpr std::size_t BankBagOffset = 63;
        static inline constexpr std::size_t BankBagCount = 6;

        // [FOREVER-VERIFIED] Retail sell differentials move sold item GUIDs through
        // ActivePlayerData::InvSlots indices 72..83.
        static inline constexpr std::size_t BuybackOffset = 72;
        static inline constexpr std::size_t BuybackCount = 12;

        static inline constexpr std::size_t ChildEquipmentOffset = 81;
        static inline constexpr std::size_t ChildEquipmentCount = 3;

        static inline constexpr std::size_t EquipableSpellOffset = 84;
        static inline constexpr std::size_t EquipableSpellCount = 16;

        static inline constexpr std::size_t AccountBankBagOffset = 100;
        static inline constexpr std::size_t AccountBankBagCount = 5;

        static inline constexpr std::size_t InvalidIndex = InvSlotCount;

        static_assert(EquipmentOffset + EquipmentCount == ProfessionEquipmentOffset);
        static_assert(ProfessionEquipmentOffset + ProfessionEquipmentCount == BagOffset);
        static_assert(BagOffset + BagCount == ReagentBagOffset);
        static_assert(ReagentBagOffset + ReagentBagCount == PackOffset);
        static_assert(PackOffset + PackCount == BankBagOffset);
        // 69..71 and the post-buyback modern ranges are not inferred from the
        // retail buyback differential; do not assert false contiguity here.
        static_assert(BankBagOffset + BankBagCount <= BuybackOffset);
        static_assert(ChildEquipmentOffset + ChildEquipmentCount == EquipableSpellOffset);
        static_assert(EquipableSpellOffset + EquipableSpellCount == AccountBankBagOffset);
        static_assert(AccountBankBagOffset + AccountBankBagCount == InvSlotCount);

        constexpr std::size_t inventoryIndex(Slot logicalSlot)
        {
            if (logicalSlot >= EquipmentStart && logicalSlot < EquipmentEnd)
                return EquipmentOffset + (logicalSlot - EquipmentStart);

            if (logicalSlot >= BagStart && logicalSlot < BagEnd)
                return BagOffset + (logicalSlot - BagStart);

            return InvalidIndex;
        }

        constexpr std::size_t packIndex(Slot packSlot)
        {
            // The current core API exposes the legacy 16-slot backpack subset.
            return packSlot < InventoryLayout::PackCount ? PackOffset + packSlot : InvalidIndex;
        }

        constexpr int16_t logicalSlot(std::size_t wireSlot)
        {
            if (wireSlot >= EquipmentOffset && wireSlot < EquipmentOffset + EquipmentCount) return static_cast<int16_t>(InventoryLayout::EquipmentStart + wireSlot - EquipmentOffset);
            if (wireSlot >= BagOffset && wireSlot < BagOffset + BagCount) return static_cast<int16_t>(InventoryLayout::BagStart + wireSlot - BagOffset);
            if (wireSlot >= PackOffset && wireSlot < PackOffset + InventoryLayout::PackCount) return static_cast<int16_t>(InventoryLayout::PackStart + wireSlot - PackOffset);
            return InventoryLayout::NoSlotAvailable;
        }

        constexpr std::size_t bankBagIndex(Slot bankBagSlot)
        {
            // Forever currently exposes six modern bank-bag GUID slots here.
            return bankBagSlot < BankBagCount ? BankBagOffset + bankBagSlot : InvalidIndex;
        }

        constexpr std::size_t buybackIndex(Slot buybackSlot)
        {
            return buybackSlot < BuybackCount ? BuybackOffset + buybackSlot : InvalidIndex;
        }
    }
}
