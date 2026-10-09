/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

#include <array>
#include <bitset>
#include <cstddef>
#include <string_view>

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    // CREATE metadata follows writePlayerDataCreate. condition is empty for unconditional slots
    // and "partyMemberVisible" for the party-only branch. STRUCTURE means the concrete wire
    // shape/order is known even when the canonical field name stays neutral; semantic modern
    // labels are retained only in referenceName until Forever proves them.
    inline constexpr std::array<CreateFieldMetadata, 65> PlayerDataCreateFields{{
        CreateFieldMetadata{0, FieldVerification::ReferenceOnly, "unknownGuid0", "DuelArbiter", "packed-guid", ""},
        CreateFieldMetadata{1, FieldVerification::StructureOnly, "unknownGuid1", "", "packed-guid", ""},
        CreateFieldMetadata{2, FieldVerification::StructureOnly, "unknownGuid2", "", "packed-guid", ""},
        CreateFieldMetadata{3, FieldVerification::StructureOnly, "unknownU64_0", "", "uint64", ""},
        CreateFieldMetadata{4, FieldVerification::StructureOnly, "unknownGuid3", "", "packed-guid", ""},
        CreateFieldMetadata{5, FieldVerification::ReferenceOnly, "unknownU32_0", "PlayerFlags", "uint32", ""},
        CreateFieldMetadata{6, FieldVerification::StructureOnly, "unknownU32_1", "", "uint32", ""},
        CreateFieldMetadata{7, FieldVerification::StructureOnly, "unknownU32_2", "", "uint32", ""},
        CreateFieldMetadata{8, FieldVerification::StructureOnly, "unknownU32_3", "", "uint32", ""},
        CreateFieldMetadata{9, FieldVerification::StructureOnly, "unknownI32_0", "", "int32", ""},
        CreateFieldMetadata{10, FieldVerification::Verified, "cfgSuperDistrictIdPrimary", "", "uint32", ""},
        CreateFieldMetadata{11, FieldVerification::StructureOnly, "unknownAfterCfgSuperDistrictBeforeCustomizationCounts", "", "uint8", ""},
        CreateFieldMetadata{12, FieldVerification::StructureOnly, "unknownCustomizationCount0", "CustomizationsCount", "uint32", ""},
        CreateFieldMetadata{13, FieldVerification::StructureOnly, "unknownCustomizationCount1", "SecondCustomizationsCount", "uint32", ""},
        CreateFieldMetadata{14, FieldVerification::StructureOnly, "unknownBytes0", "", "uint8-array", ""},
        CreateFieldMetadata{15, FieldVerification::StructureOnly, "unknownU8_0", "", "uint8", ""},
        CreateFieldMetadata{16, FieldVerification::StructureOnly, "unknownU8_1", "", "uint8", ""},
        CreateFieldMetadata{17, FieldVerification::StructureOnly, "unknownU8_2", "", "uint8", ""},
        CreateFieldMetadata{18, FieldVerification::StructureOnly, "unknownU8_3", "", "uint8", ""},
        CreateFieldMetadata{19, FieldVerification::StructureOnly, "unknownU32_4", "", "uint32", ""},
        CreateFieldMetadata{20, FieldVerification::StructureOnly, "unknownI32_1", "", "int32", ""},
        CreateFieldMetadata{21, FieldVerification::Verified, "questLog", "", "nested-array", "partyMemberVisible"},
        CreateFieldMetadata{22, FieldVerification::Verified, "questLogQuestIdToIndex", "", "nested-map", "partyMemberVisible"},
        CreateFieldMetadata{23, FieldVerification::StructureOnly, "unknownPartyDynamicRecordCount", "", "uint32", "partyMemberVisible"},
        CreateFieldMetadata{24, FieldVerification::StructureOnly, "unknownVisibleItems", "VisibleItems", "nested-array", ""},
        CreateFieldMetadata{25, FieldVerification::StructureOnly, "unknownI32_2", "", "int32", ""},
        CreateFieldMetadata{26, FieldVerification::StructureOnly, "unknownI32_3", "", "int32", ""},
        CreateFieldMetadata{27, FieldVerification::StructureOnly, "unknownU32_5", "", "uint32", ""},
        CreateFieldMetadata{28, FieldVerification::ReferenceOnly, "unknownU32_6", "CurrentSpec", "uint32", ""},
        CreateFieldMetadata{29, FieldVerification::StructureOnly, "unknownI32_4", "", "int32", ""},
        CreateFieldMetadata{30, FieldVerification::StructureOnly, "unknownI32_5", "", "int32", ""},
        CreateFieldMetadata{31, FieldVerification::StructureOnly, "unknownFloatArray0", "", "float-array", ""},
        CreateFieldMetadata{32, FieldVerification::StructureOnly, "unknownU8_4", "", "uint8", ""},
        CreateFieldMetadata{33, FieldVerification::StructureOnly, "unknownI32_6", "", "int32", ""},
        CreateFieldMetadata{34, FieldVerification::StructureOnly, "unknownI64_0", "", "int64", ""},
        CreateFieldMetadata{35, FieldVerification::StructureOnly, "unknownDynamicRecords0Count", "", "uint32", ""},
        CreateFieldMetadata{36, FieldVerification::ReferenceOnly, "unknownFixedRecords0", "ZonePlayerForcedReaction", "record-array", ""},
        CreateFieldMetadata{37, FieldVerification::StructureOnly, "unknownI32_7", "", "int32", ""},
        CreateFieldMetadata{38, FieldVerification::StructureOnly, "unknownI32_8", "", "int32", ""},
        CreateFieldMetadata{39, FieldVerification::StructureOnly, "unknownI32_9", "", "int32", ""},
        CreateFieldMetadata{40, FieldVerification::StructureOnly, "unknownDynamicRecords1Count", "", "uint32", ""},
        CreateFieldMetadata{41, FieldVerification::ReferenceOnly, "unknownCtrOptions0", "CtrOptions", "nested-record", ""},
        CreateFieldMetadata{42, FieldVerification::StructureOnly, "unknownI32_10", "", "int32", ""},
        CreateFieldMetadata{43, FieldVerification::StructureOnly, "unknownI32_11", "", "int32", ""},
        CreateFieldMetadata{44, FieldVerification::ReferenceOnly, "unknownDungeonScore0", "DungeonScoreSummary", "nested-record", ""},
        CreateFieldMetadata{45, FieldVerification::ReferenceOnly, "unknownLeaverInfo0", "LeaverInfo", "nested-record", ""},
        CreateFieldMetadata{46, FieldVerification::StructureOnly, "unknownGuid4", "", "packed-guid", ""},
        CreateFieldMetadata{47, FieldVerification::StructureOnly, "unknownI32_12", "", "int32", ""},
        CreateFieldMetadata{48, FieldVerification::ReferenceOnly, "unknownItemInstances0", "ItemInstance", "record-array", ""},
        CreateFieldMetadata{49, FieldVerification::StructureOnly, "unknownI32Vector0Count", "", "uint32", ""},
        CreateFieldMetadata{50, FieldVerification::ReferenceOnly, "unknownU32Array0", "AttackRoundBaseTime", "uint32-array", ""},
        CreateFieldMetadata{51, FieldVerification::ReferenceOnly, "unknownCustomTabard0", "CustomTabardInfo", "nested-record", ""},
        CreateFieldMetadata{52, FieldVerification::ReferenceOnly, "unknownNpcAsPlayer0", "NpcAsPlayerInfo", "nested-record", ""},
        CreateFieldMetadata{53, FieldVerification::Verified, "cfgSuperDistrictIdSecondary", "", "uint32", ""},
        CreateFieldMetadata{54, FieldVerification::StructureOnly, "unknownAfterCfgSuperDistrictBeforeCustomizationPayload", "", "byte-span", ""},
        CreateFieldMetadata{55, FieldVerification::ReferenceOnly, "unknownCustomizationRecords0", "Customizations", "record-vector", ""},
        CreateFieldMetadata{56, FieldVerification::ReferenceOnly, "unknownCustomizationRecords1", "SecondCustomizations", "record-vector", ""},
        CreateFieldMetadata{57, FieldVerification::StructureOnly, "unknownPartyDynamicRecords0", "", "nested-vector", "partyMemberVisible"},
        CreateFieldMetadata{58, FieldVerification::StructureOnly, "unknownDynamicRecords0", "", "opaque-vector", ""},
        CreateFieldMetadata{59, FieldVerification::StructureOnly, "unknownDynamicRecords1", "", "opaque-vector", ""},
        CreateFieldMetadata{60, FieldVerification::StructureOnly, "unknownI32Vector0", "", "int32-vector", ""},
        CreateFieldMetadata{61, FieldVerification::Verified, "firstName", "", "string", ""},
        CreateFieldMetadata{62, FieldVerification::Verified, "lastName", "", "string", ""},
        CreateFieldMetadata{63, FieldVerification::StructureOnly, "unknownNameFlags", "", "uint8", ""},
        CreateFieldMetadata{64, FieldVerification::StructureOnly, "unknownOptionalNamePayload0", "", "optional-record", ""}
    }};

    static_assert(hasContiguousCreateFieldOrder(PlayerDataCreateFields));
    static_assert(referenceCreateFieldsHaveReferenceNames(PlayerDataCreateFields));
    static_assert(referenceCreateFieldsUseNeutralNames(PlayerDataCreateFields));
    static_assert(verifiedCreateFieldsHaveNoReferenceNames(PlayerDataCreateFields));
    static_assert(countCreateFieldsByVerification(PlayerDataCreateFields, FieldVerification::Verified) == 6);
    static_assert(countCreateFieldsByVerification(PlayerDataCreateFields, FieldVerification::StructureOnly) == 46);
    static_assert(countCreateFieldsByVerification(PlayerDataCreateFields, FieldVerification::ReferenceOnly) == 13);
    static_assert(countCreateFieldsByVerification(PlayerDataCreateFields, FieldVerification::Unknown) == 0);

    struct PlayerFieldMetadata
    {
        std::size_t bit;
        FieldVerification verification;
        std::string_view name;
        std::string_view referenceName;
        std::string_view wireType;
    };

    // Only fields currently emitted by PlayerData.cpp are listed here. Imported
    // external schema names remain reference unless Forever capture/runtime evidence proves them.
    inline constexpr PlayerFieldMetadata PlayerDuelArbiter{Fields::PlayerData::DuelArbiterBit, FieldVerification::ReferenceOnly, "unknownGuidBit9", "DuelArbiter", "guid"};
    inline constexpr PlayerFieldMetadata PlayerFlags{Fields::PlayerData::PlayerFlagsBit, FieldVerification::ReferenceOnly, "unknownU32Bit14", "PlayerFlags", "scalar"};
    inline constexpr PlayerFieldMetadata PlayerQuestLogQuestIdToIndex{Fields::PlayerData::QuestLogQuestIdToIndexBit, FieldVerification::Verified, "questLogQuestIdToIndex", "", "nested-map"};
    inline constexpr PlayerFieldMetadata PlayerCurrentSpec{Fields::PlayerData::CurrentSpecBit, FieldVerification::ReferenceOnly, "unknownU32Bit29", "CurrentSpec", "scalar"};
    inline constexpr PlayerFieldMetadata PlayerName{Fields::PlayerData::NameBit, FieldVerification::StructureOnly, "unknownNameBit36", "Name", "string-pair"};
    inline constexpr PlayerFieldMetadata PlayerQuestLogGroup{Fields::PlayerData::QuestLogGroupBit, FieldVerification::Verified, "questLog", "", "nested-array"};
    inline constexpr PlayerFieldMetadata PlayerVisibleItemsGroup{Fields::PlayerData::VisibleItemsGroupBit, FieldVerification::StructureOnly, "unknownVisibleItems", "VisibleItems", "nested-array"};

    inline constexpr std::array<PlayerFieldMetadata, 6> PlayerComparableValuesMetadata{{
        PlayerDuelArbiter,
        PlayerFlags,
        PlayerQuestLogQuestIdToIndex,
        PlayerCurrentSpec,
        PlayerQuestLogGroup,
        PlayerVisibleItemsGroup
    }};

    constexpr bool playerComparableValuesMatchCreateMetadata()
    {
        for (auto const& valueField : PlayerComparableValuesMetadata)
        {
            bool found = false;
            for (auto const& createField : PlayerDataCreateFields)
            {
                const bool sameCanonicalName = createField.name == valueField.name;
                const bool sameReferenceName = !valueField.referenceName.empty() && createField.referenceName == valueField.referenceName;
                if (!sameCanonicalName && !sameReferenceName)
                    continue;

                found = true;
                if (createField.verification != valueField.verification)
                    return false;
                break;
            }
            if (!found)
                return false;
        }
        return true;
    }

    // NameBit is intentionally excluded from the 1:1 check: VALUES emits one combined
    // name update while CREATE serializes verified firstName and lastName slots separately.
    static_assert(playerComparableValuesMatchCreateMetadata());

    inline std::bitset<Fields::PlayerData::ChangeMaskSize> getPlayerDataAllowedChanges(Fields::PlayerData const& fields)
    {
        // Keep the live Forever differential mask intact. Filtering individual PlayerData
        // bits here can remove parent/group bits required by the wire update.
        return fields.changes;
    }
}
