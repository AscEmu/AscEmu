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
        CreateFieldMetadata{1, FieldVerification::Unverified, "unknownGuidCreate0", "FarsightObject", "packed-guid", ""},
        CreateFieldMetadata{2, FieldVerification::Unverified, "unknownGuidCreate1", "SummonedBattlePetGUID", "packed-guid", ""},
        CreateFieldMetadata{3, FieldVerification::Unverified, "unknownU32Create2", "KnownTitlesCount", "uint32", ""},
        CreateFieldMetadata{4, FieldVerification::StructureOnly, "unknownByteSpanCreate3", "", "byte-span", ""},
        CreateFieldMetadata{5, FieldVerification::Verified, "coinage", "", "uint64", ""},
        CreateFieldMetadata{6, FieldVerification::Unverified, "unknownU64Create5", "AccountBankCoinage", "uint64", ""},
        CreateFieldMetadata{7, FieldVerification::Verified, "xp", "", "int32", ""},
        CreateFieldMetadata{8, FieldVerification::Verified, "nextLevelXp", "", "int32", ""},
        CreateFieldMetadata{9, FieldVerification::StructureOnly, "unknownI32Create9", "", "int32", ""},
        CreateFieldMetadata{10, FieldVerification::Verified, "skillInfo", "", "nested-record", ""},
        CreateFieldMetadata{11, FieldVerification::Unverified, "unknownI32AfterSkill0", "CharacterPoints", "int32", ""},
        CreateFieldMetadata{12, FieldVerification::Unverified, "unknownI32AfterSkill1", "MaxTalentTiers", "int32", ""},
        CreateFieldMetadata{13, FieldVerification::Unverified, "unknownU32AfterSkill2", "TrackCreatureMask", "uint32", ""},
        CreateFieldMetadata{14, FieldVerification::Unverified, "unknownFloatAfterSkill3", "MainhandExpertise", "float", ""},
        CreateFieldMetadata{15, FieldVerification::Unverified, "unknownFloatAfterSkill4", "OffhandExpertise", "float", ""},
        CreateFieldMetadata{16, FieldVerification::Unverified, "unknownFloatAfterSkill5", "RangedExpertise", "float", ""},
        CreateFieldMetadata{17, FieldVerification::Unverified, "unknownFloatAfterSkill6", "CombatRatingExpertise", "float", ""},
        CreateFieldMetadata{18, FieldVerification::Verified, "blockPercentage", "", "float", ""},
        CreateFieldMetadata{19, FieldVerification::Verified, "dodgePercentage", "", "float", ""},
        CreateFieldMetadata{20, FieldVerification::Unverified, "unknownFloatAfterSkill9", "DodgePercentageFromAttribute", "float", ""},
        CreateFieldMetadata{21, FieldVerification::Verified, "parryPercentage", "", "float", ""},
        CreateFieldMetadata{22, FieldVerification::Unverified, "unknownFloatAfterSkill11", "ParryPercentageFromAttribute", "float", ""},
        CreateFieldMetadata{23, FieldVerification::Verified, "critPercentage", "", "float", ""},
        CreateFieldMetadata{24, FieldVerification::Verified, "rangedCritPercentage", "", "float", ""},
        CreateFieldMetadata{25, FieldVerification::Verified, "offhandCritPercentage", "", "float", ""},
        CreateFieldMetadata{26, FieldVerification::Unverified, "unknownFloatAfterSkill15", "SpellCritPercentage", "float", ""},
        CreateFieldMetadata{27, FieldVerification::Verified, "shieldBlock", "", "int32", ""},
        CreateFieldMetadata{28, FieldVerification::Verified, "shieldBlockCritPercentage", "", "float", ""},
        CreateFieldMetadata{29, FieldVerification::Unverified, "unknownFloatAfterSkill18", "Mastery", "float", ""},
        CreateFieldMetadata{30, FieldVerification::Unverified, "unknownFloatAfterSkill19", "Speed", "float", ""},
        CreateFieldMetadata{31, FieldVerification::Unverified, "unknownFloatAfterSkill20", "Avoidance", "float", ""},
        CreateFieldMetadata{32, FieldVerification::Unverified, "unknownFloatAfterSkill21", "Sturdiness", "float", ""},
        CreateFieldMetadata{33, FieldVerification::Unverified, "unknownI32AfterSkill22", "Versatility", "int32", ""},
        CreateFieldMetadata{34, FieldVerification::Unverified, "unknownFloatAfterSkill23", "VersatilityBonus", "float", ""},
        CreateFieldMetadata{35, FieldVerification::Unverified, "unknownFloatAfterSkill24", "PvpPowerDamage", "float", ""},
        CreateFieldMetadata{36, FieldVerification::Unverified, "unknownFloatAfterSkill25", "PvpPowerHealing", "float", ""},
        CreateFieldMetadata{37, FieldVerification::StructureOnly, "unknownPostSkillHeader", "", "byte-span", ""},
        CreateFieldMetadata{38, FieldVerification::StructureOnly, "unknownPostSkillRecords", "", "record-array", ""},
        CreateFieldMetadata{39, FieldVerification::StructureOnly, "unknownPostSkillTail", "", "byte-span", ""},
        CreateFieldMetadata{40, FieldVerification::StructureOnly, "unknownBeforeOutfit", "", "byte-span", ""},
        CreateFieldMetadata{41, FieldVerification::StructureOnly, "unknownU32BeforeOutfit0", "", "uint32", ""},
        CreateFieldMetadata{42, FieldVerification::StructureOnly, "unknownU32BeforeOutfit1", "", "uint32", ""},
        CreateFieldMetadata{43, FieldVerification::Unverified, "unknownOutfitRecord0", "ViewedOutfit", "nested-record", ""},
        CreateFieldMetadata{44, FieldVerification::StructureOnly, "unknownOutfitRecordCount", "", "uint32", ""},
        CreateFieldMetadata{45, FieldVerification::Unverified, "unknownOutfitRecords", "AdditionalOutfits", "nested-vector", ""},
        CreateFieldMetadata{46, FieldVerification::Unverified, "unknownOutfitMetadata", "TransmogOutfitMetadata", "nested-record", ""},
        CreateFieldMetadata{47, FieldVerification::Unverified, "unknownU64Vector0", "KnownTitles", "uint64-vector", ""},
        CreateFieldMetadata{48, FieldVerification::StructureOnly, "unknownAfterOutfit", "", "byte-span", ""}
    }};

    static_assert(hasContiguousCreateFieldOrder(ActivePlayerDataCreateFields));
    static_assert(referenceCreateFieldsHaveReferenceNames(ActivePlayerDataCreateFields));
    static_assert(referenceCreateFieldsUseNeutralNames(ActivePlayerDataCreateFields));
    static_assert(verifiedCreateFieldsHaveNoReferenceNames(ActivePlayerDataCreateFields));
    static_assert(countCreateFieldsByVerification(ActivePlayerDataCreateFields, FieldVerification::Verified) == 13);
    static_assert(countCreateFieldsByVerification(ActivePlayerDataCreateFields, FieldVerification::StructureOnly) == 10);
    static_assert(countCreateFieldsByVerification(ActivePlayerDataCreateFields, FieldVerification::Unverified) == 26);
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
    // The timestamp value itself is 64-bit on the wire.
    struct BuybackDataField
    {
        static constexpr UpdateFieldMetadata metadata() { return {FieldVerification::Unverified, "buybackData", "BuybackPrice/BuybackTimestamp"}; }

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


    // [FOREVER-VERIFIED] Classic 1.60 wire positions for the trait-config map and
    // active combat config. AscEmu stores wire positions directly.
    struct TraitConfigsField
    {
        static constexpr UpdateFieldMetadata metadata() { return {FieldVerification::Verified, "traitConfigs", "map"}; }

        static void writeTraitEntry(ByteBuffer& data, Fields::TraitEntry const& entry)
        {
            data << entry.traitNodeId << entry.traitNodeEntryId << entry.rank << entry.grantedRanks << entry.bonusRanks;
        }

        static void writeTraitSubTree(ByteBuffer& data, Fields::TraitSubTreeCache const& subTree)
        {
            data << subTree.traitSubTreeId << uint32_t(subTree.entries.size());
            for (auto const& entry : subTree.entries)
                writeTraitEntry(data, entry);
            data.writeBit(subTree.active != 0);
            data.flushBits();
        }

        static void writeCompleteDynamicFieldUpdateMask(ByteBuffer& data, std::size_t size)
        {
            // Complete dynamic-field mask: 32-bit element count followed by one changed
            // bit per element. Keep this bit-oriented so byte alignment does not matter.
            data.writeBits(size, 32);
            for (std::size_t i = 0; i < size; ++i)
                data.writeBit(true);
        }

        static void writeTraitConfigUpdateAll(ByteBuffer& data, Fields::TraitConfig const& config)
        {
            // TraitConfig uses a 15-bit nested change mask. Changed map entries currently
            // send every nested field as changed while the outer map remains a diff update.
            data.writeBits(0x7FFFu, 15);
            writeCompleteDynamicFieldUpdateMask(data, config.entries.size());
            writeCompleteDynamicFieldUpdateMask(data, config.subTrees.size());
            data.flushBits();

            for (auto const& entry : config.entries)
                writeTraitEntry(data, entry);
            for (auto const& subTree : config.subTrees)
                writeTraitSubTree(data, subTree);

            data << config.id;

            data << int32_t(config.type == 1 ? 4 : config.type); // Forever CamelotCombat wire type
            if (config.type == 2)
                data << config.skillLineId;

            if (config.type == 1)
                data << config.chrSpecializationId << config.combatConfigFlags << config.localIdentifier;

            if (config.type == 3)
                data << config.traitSystemId << config.variationId;

            data.writeBits(config.name.size(), 9);
            data.flushBits();
            if (!config.name.empty())
                data.append(reinterpret_cast<uint8_t const*>(config.name.data()), config.name.size());
        }

        template <typename Owner, std::size_t N>
        static void copyKnownBits(Owner const&, std::bitset<N> const& source, std::bitset<N>& target)
        {
            if (source.test(Fields::ActivePlayerData::TraitDataParentBit))
                target.set(Fields::ActivePlayerData::TraitDataParentBit);
            if (source.test(Fields::ActivePlayerData::TraitConfigsBit))
                target.set(Fields::ActivePlayerData::TraitConfigsBit);
        }

        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if (!changed(Fields::ActivePlayerData::TraitDataParentBit) || !changed(Fields::ActivePlayerData::TraitConfigsBit))
                return;

            // Diff-map update: uint8 completeMap=0, uint16 changesCount, followed by
            // key/state/value-update records.
            data << uint8_t(0);

            uint16_t changesCount = 0;
            for (auto const& [configId, state] : owner.traitConfigUpdateStates)
                if (state != Fields::TraitConfigMapState::Unchanged)
                    ++changesCount;
            data << changesCount;

            for (auto const& [configId, state] : owner.traitConfigUpdateStates)
            {
                if (state == Fields::TraitConfigMapState::Unchanged)
                    continue;

                data << configId;
                data << static_cast<uint8_t>(state);

                if (state == Fields::TraitConfigMapState::Deleted)
                    continue;

                auto const itr = owner.traitConfigs.find(configId);
                if (itr == owner.traitConfigs.end())
                    continue;

                writeTraitConfigUpdateAll(data, itr->second);
            }
        }
    };

    // Its CREATE location is deliberately not claimed while that create span remains opaque.
    using ActivePlayerDataUpdate = UpdateDefinition<Fields::ActivePlayerData::ChangeMaskSize,
        ScalarField<&Fields::ActivePlayerData::coinage, Fields::ActivePlayerData::CoinageBit, 32, FieldVerification::Verified, "coinage">,
        ScalarField<&Fields::ActivePlayerData::xp, Fields::ActivePlayerData::XpBit, 32, FieldVerification::Verified, "xp">,
        ScalarField<&Fields::ActivePlayerData::nextLevelXp, Fields::ActivePlayerData::NextLevelXpBit, 32, FieldVerification::Verified, "nextLevelXp">,
        ScalarField<&Fields::ActivePlayerData::watchedFactionIndex, Fields::ActivePlayerData::WatchedFactionIndexBit, Fields::ActivePlayerData::WatchedFactionParentBit, FieldVerification::Verified, "watchedFactionIndex">,
        TraitConfigsField,
        ScalarField<&Fields::ActivePlayerData::activeCombatTraitConfigId, Fields::ActivePlayerData::ActiveCombatTraitConfigIdBit, Fields::ActivePlayerData::TraitDataParentBit, FieldVerification::Verified, "activeCombatTraitConfigId">,
        GuidArrayField<&Fields::ActivePlayerData::invSlots, Fields::ActivePlayerData::InventorySlotsGroupBit, Fields::ActivePlayerData::InventorySlotsFirstBit, FieldVerification::Verified, "inventorySlots">,
        BuybackDataField>;
}
