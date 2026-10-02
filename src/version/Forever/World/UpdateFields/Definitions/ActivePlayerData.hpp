/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

#include <array>

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    // CREATE metadata is descriptive only: it documents the proven wire order/type without
    // promoting imported modern reference-schema names to Forever semantics. order is serializer order,
    // not a byte offset; packed GUIDs and variable-length records make absolute offsets dynamic.
    inline constexpr std::array<CreateFieldMetadata, 49> ActivePlayerDataCreateFields{{
        CreateFieldMetadata{0, FieldVerification::Verified, "inventorySlots", "", "packed-guid[105]", ""},
        CreateFieldMetadata{1, FieldVerification::ReferenceOnly, "unknownGuidCreate0", "FarsightObject", "packed-guid", ""},
        CreateFieldMetadata{2, FieldVerification::ReferenceOnly, "unknownGuidCreate1", "SummonedBattlePetGUID", "packed-guid", ""},
        CreateFieldMetadata{3, FieldVerification::ReferenceOnly, "unknownU32Create2", "KnownTitlesCount", "uint32", ""},
        CreateFieldMetadata{4, FieldVerification::StructureOnly, "unknownByteSpanCreate3", "", "byte-span", ""},
        CreateFieldMetadata{5, FieldVerification::Verified, "coinage", "", "uint64", ""},
        CreateFieldMetadata{6, FieldVerification::ReferenceOnly, "unknownU64Create5", "AccountBankCoinage", "uint64", ""},
        CreateFieldMetadata{7, FieldVerification::Verified, "xp", "", "int32", ""},
        CreateFieldMetadata{8, FieldVerification::Verified, "nextLevelXp", "", "int32", ""},
        CreateFieldMetadata{9, FieldVerification::StructureOnly, "unknownI32Create9", "", "int32", ""},
        CreateFieldMetadata{10, FieldVerification::Verified, "skillInfo", "", "nested-record", ""},
        CreateFieldMetadata{11, FieldVerification::ReferenceOnly, "unknownI32AfterSkill0", "CharacterPoints", "int32", ""},
        CreateFieldMetadata{12, FieldVerification::ReferenceOnly, "unknownI32AfterSkill1", "MaxTalentTiers", "int32", ""},
        CreateFieldMetadata{13, FieldVerification::ReferenceOnly, "unknownU32AfterSkill2", "TrackCreatureMask", "uint32", ""},
        CreateFieldMetadata{14, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill3", "MainhandExpertise", "float", ""},
        CreateFieldMetadata{15, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill4", "OffhandExpertise", "float", ""},
        CreateFieldMetadata{16, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill5", "RangedExpertise", "float", ""},
        CreateFieldMetadata{17, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill6", "CombatRatingExpertise", "float", ""},
        CreateFieldMetadata{18, FieldVerification::Verified, "blockPercentage", "", "float", ""},
        CreateFieldMetadata{19, FieldVerification::Verified, "dodgePercentage", "", "float", ""},
        CreateFieldMetadata{20, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill9", "DodgePercentageFromAttribute", "float", ""},
        CreateFieldMetadata{21, FieldVerification::Verified, "parryPercentage", "", "float", ""},
        CreateFieldMetadata{22, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill11", "ParryPercentageFromAttribute", "float", ""},
        CreateFieldMetadata{23, FieldVerification::Verified, "critPercentage", "", "float", ""},
        CreateFieldMetadata{24, FieldVerification::Verified, "rangedCritPercentage", "", "float", ""},
        CreateFieldMetadata{25, FieldVerification::Verified, "offhandCritPercentage", "", "float", ""},
        CreateFieldMetadata{26, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill15", "SpellCritPercentage", "float", ""},
        CreateFieldMetadata{27, FieldVerification::Verified, "shieldBlock", "", "int32", ""},
        CreateFieldMetadata{28, FieldVerification::Verified, "shieldBlockCritPercentage", "", "float", ""},
        CreateFieldMetadata{29, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill18", "Mastery", "float", ""},
        CreateFieldMetadata{30, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill19", "Speed", "float", ""},
        CreateFieldMetadata{31, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill20", "Avoidance", "float", ""},
        CreateFieldMetadata{32, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill21", "Sturdiness", "float", ""},
        CreateFieldMetadata{33, FieldVerification::ReferenceOnly, "unknownI32AfterSkill22", "Versatility", "int32", ""},
        CreateFieldMetadata{34, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill23", "VersatilityBonus", "float", ""},
        CreateFieldMetadata{35, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill24", "PvpPowerDamage", "float", ""},
        CreateFieldMetadata{36, FieldVerification::ReferenceOnly, "unknownFloatAfterSkill25", "PvpPowerHealing", "float", ""},
        CreateFieldMetadata{37, FieldVerification::StructureOnly, "unknownPostSkillHeader", "", "byte-span", ""},
        CreateFieldMetadata{38, FieldVerification::StructureOnly, "unknownPostSkillRecords", "", "record-array", ""},
        CreateFieldMetadata{39, FieldVerification::StructureOnly, "unknownPostSkillTail", "", "byte-span", ""},
        CreateFieldMetadata{40, FieldVerification::StructureOnly, "unknownBeforeOutfit", "", "byte-span", ""},
        CreateFieldMetadata{41, FieldVerification::StructureOnly, "unknownU32BeforeOutfit0", "", "uint32", ""},
        CreateFieldMetadata{42, FieldVerification::StructureOnly, "unknownU32BeforeOutfit1", "", "uint32", ""},
        CreateFieldMetadata{43, FieldVerification::ReferenceOnly, "unknownOutfitRecord0", "ViewedOutfit", "nested-record", ""},
        CreateFieldMetadata{44, FieldVerification::StructureOnly, "unknownOutfitRecordCount", "", "uint32", ""},
        CreateFieldMetadata{45, FieldVerification::ReferenceOnly, "unknownOutfitRecords", "AdditionalOutfits", "nested-vector", ""},
        CreateFieldMetadata{46, FieldVerification::ReferenceOnly, "unknownOutfitMetadata", "TransmogOutfitMetadata", "nested-record", ""},
        CreateFieldMetadata{47, FieldVerification::ReferenceOnly, "unknownU64Vector0", "KnownTitles", "uint64-vector", ""},
        CreateFieldMetadata{48, FieldVerification::StructureOnly, "unknownAfterOutfit", "", "byte-span", ""}
    }};

    static_assert(hasContiguousCreateFieldOrder(ActivePlayerDataCreateFields));
    static_assert(referenceCreateFieldsHaveReferenceNames(ActivePlayerDataCreateFields));
    static_assert(referenceCreateFieldsUseNeutralNames(ActivePlayerDataCreateFields));
    static_assert(verifiedCreateFieldsHaveNoReferenceNames(ActivePlayerDataCreateFields));
    static_assert(countCreateFieldsByVerification(ActivePlayerDataCreateFields, FieldVerification::Verified) == 13);
    static_assert(countCreateFieldsByVerification(ActivePlayerDataCreateFields, FieldVerification::StructureOnly) == 10);
    static_assert(countCreateFieldsByVerification(ActivePlayerDataCreateFields, FieldVerification::ReferenceOnly) == 26);
    static_assert(countCreateFieldsByVerification(ActivePlayerDataCreateFields, FieldVerification::Unknown) == 0);

    // These live VALUES fields have Forever capture support. Large CREATE-only opaque regions
    // intentionally remain outside this definition until identified from Forever captures.
    using ActivePlayerDataCreateMappedUpdate = UpdateDefinition<Fields::ActivePlayerData::ChangeMaskSize,
        ScalarField<&Fields::ActivePlayerData::coinage, Fields::ActivePlayerData::CoinageBit, 32, FieldVerification::Verified, "coinage">,
        ScalarField<&Fields::ActivePlayerData::xp, Fields::ActivePlayerData::XpBit, 32, FieldVerification::Verified, "xp">,
        ScalarField<&Fields::ActivePlayerData::nextLevelXp, Fields::ActivePlayerData::NextLevelXpBit, 32, FieldVerification::Verified, "nextLevelXp">,
        GuidArrayField<&Fields::ActivePlayerData::invSlots, Fields::ActivePlayerData::InventorySlotsGroupBit, Fields::ActivePlayerData::InventorySlotsFirstBit, FieldVerification::Verified, "inventorySlots">>;

    static_assert(updateFieldsMatchCreateMetadata(ActivePlayerDataCreateFields, ActivePlayerDataCreateMappedUpdate::Metadata));

    // [FOREVER-VERIFIED] Retail 1.60.1.70124 sell differentials use one
    // Buyback group bit (353), price element bits 354..365 and timestamp
    // element bits 366..377. Values are serialized per slot: price, timestamp.
    struct BuybackDataField
    {
        static constexpr UpdateFieldMetadata metadata() { return {FieldVerification::ReferenceOnly, "buybackData", "BuybackPrice/BuybackTimestamp"}; }

        template <typename Owner, std::size_t N>
        static void copyKnownBits(Owner const& owner, std::bitset<N> const& source, std::bitset<N>& target)
        {
            if (source.test(Fields::ActivePlayerData::BuybackDataGroupBit))
                target.set(Fields::ActivePlayerData::BuybackDataGroupBit);

            for (std::size_t i = 0; i < owner.buybackPrice.size(); ++i)
            {
                if (source.test(Fields::ActivePlayerData::BuybackPriceFirstBit + i))
                    target.set(Fields::ActivePlayerData::BuybackPriceFirstBit + i);
                if (source.test(Fields::ActivePlayerData::BuybackTimestampFirstBit + i))
                    target.set(Fields::ActivePlayerData::BuybackTimestampFirstBit + i);
            }
        }

        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if (!changed(Fields::ActivePlayerData::BuybackDataGroupBit))
                return;

            for (std::size_t i = 0; i < owner.buybackPrice.size(); ++i)
            {
                if (changed(Fields::ActivePlayerData::BuybackPriceFirstBit + i))
                    data << owner.buybackPrice[i];
                if (changed(Fields::ActivePlayerData::BuybackTimestampFirstBit + i))
                    data << owner.buybackTimestamp[i];
            }
        }
    };

    // Its CREATE location is deliberately not claimed while that create span remains opaque.
    using ActivePlayerDataUpdate = UpdateDefinition<Fields::ActivePlayerData::ChangeMaskSize,
        ScalarField<&Fields::ActivePlayerData::coinage, Fields::ActivePlayerData::CoinageBit, 32, FieldVerification::Verified, "coinage">,
        ScalarField<&Fields::ActivePlayerData::xp, Fields::ActivePlayerData::XpBit, 32, FieldVerification::Verified, "xp">,
        ScalarField<&Fields::ActivePlayerData::nextLevelXp, Fields::ActivePlayerData::NextLevelXpBit, 32, FieldVerification::Verified, "nextLevelXp">,
        GuidArrayField<&Fields::ActivePlayerData::invSlots, Fields::ActivePlayerData::InventorySlotsGroupBit, Fields::ActivePlayerData::InventorySlotsFirstBit, FieldVerification::Verified, "inventorySlots">,
        BuybackDataField>;
}
