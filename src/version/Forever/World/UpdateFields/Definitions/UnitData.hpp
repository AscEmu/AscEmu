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
    // CREATE metadata mirrors writeUnitDataCreate order. REFERENCE entries keep the modern reference-schema
    // label only as referenceName; their canonical Forever name remains neutral until a Forever
    // capture/runtime differential proves the semantic identity. STRUCTURE entries keep an established runtime name while
    // explicitly stopping short of semantic capture verification. condition marks owner-visible conditional slots.
    inline constexpr std::array<CreateFieldMetadata, 138> UnitDataCreateFields{{
        CreateFieldMetadata{0, FieldVerification::StructureOnly, "displayId", "DisplayID", "int32", ""},
        CreateFieldMetadata{1, FieldVerification::Verified, "npcFlags", "", "uint32", ""},
        CreateFieldMetadata{2, FieldVerification::Verified, "npcFlags2", "", "uint32", ""},
        CreateFieldMetadata{3, FieldVerification::ReferenceOnly, "unknownU32Create3", "StateSpellVisualID", "uint32", ""},
        CreateFieldMetadata{4, FieldVerification::ReferenceOnly, "unknownU32Create4", "StateAnimID", "uint32", ""},
        CreateFieldMetadata{5, FieldVerification::ReferenceOnly, "unknownU32Create5", "StateAnimKitID", "uint32", ""},
        CreateFieldMetadata{6, FieldVerification::StructureOnly, "unknownU32Create6", "StateWorldEffectIDsCount", "uint32", ""},
        CreateFieldMetadata{7, FieldVerification::ReferenceOnly, "unknownU32Create7", "StateWorldEffectsQuestObjectiveID", "uint32", ""},
        CreateFieldMetadata{8, FieldVerification::ReferenceOnly, "unknownI32Create8", "SpellOverrideNameID", "int32", ""},
        CreateFieldMetadata{9, FieldVerification::ReferenceOnly, "unknownU32Vector0", "StateWorldEffectIDs", "uint32-vector", ""},
        CreateFieldMetadata{10, FieldVerification::StructureOnly, "unknownGuidCreate10", "Charm", "packed-guid", ""},
        CreateFieldMetadata{11, FieldVerification::StructureOnly, "unknownGuidCreate11", "Summon", "packed-guid", ""},
        CreateFieldMetadata{12, FieldVerification::StructureOnly, "unknownGuidCreate12", "Critter", "packed-guid", "ownerVisible"},
        CreateFieldMetadata{13, FieldVerification::StructureOnly, "unknownGuidCreate13", "CharmedBy", "packed-guid", ""},
        CreateFieldMetadata{14, FieldVerification::StructureOnly, "unknownGuidCreate14", "SummonedBy", "packed-guid", ""},
        CreateFieldMetadata{15, FieldVerification::StructureOnly, "unknownGuidCreate15", "CreatedBy", "packed-guid", ""},
        CreateFieldMetadata{16, FieldVerification::StructureOnly, "unknownGuidCreate16", "DemonCreator", "packed-guid", ""},
        CreateFieldMetadata{17, FieldVerification::StructureOnly, "unknownGuidCreate17", "LookAtControllerTarget", "packed-guid", ""},
        CreateFieldMetadata{18, FieldVerification::Verified, "target", "", "packed-guid", ""},
        CreateFieldMetadata{19, FieldVerification::StructureOnly, "unknownGuidCreate19", "BattlePetCompanionGUID", "packed-guid", ""},
        CreateFieldMetadata{20, FieldVerification::ReferenceOnly, "unknownU64Create20", "BattlePetDBID", "uint64", ""},
        CreateFieldMetadata{21, FieldVerification::StructureOnly, "unknownGuidCreate21", "BattlePetAttachedToDecorGUID", "packed-guid", ""},
        CreateFieldMetadata{22, FieldVerification::StructureOnly, "unknownGuidCreate22", "BattlePetDecorHouseGUID", "packed-guid", ""},
        CreateFieldMetadata{23, FieldVerification::StructureOnly, "unknownRecordCreate23", "ChannelData", "nested-record", ""},
        CreateFieldMetadata{24, FieldVerification::StructureOnly, "unknownI8Create24", "SpellEmpowerStage", "int8", ""},
        CreateFieldMetadata{25, FieldVerification::StructureOnly, "unknownU32Create25", "SummonedByHomeRealm", "uint32", ""},
        CreateFieldMetadata{26, FieldVerification::StructureOnly, "race", "Race", "uint8", ""},
        CreateFieldMetadata{27, FieldVerification::StructureOnly, "classId", "ClassID", "uint8", ""},
        CreateFieldMetadata{28, FieldVerification::StructureOnly, "playerClassId", "PlayerClassID", "uint8", ""},
        CreateFieldMetadata{29, FieldVerification::StructureOnly, "sex", "Sex", "uint8", ""},
        CreateFieldMetadata{30, FieldVerification::StructureOnly, "creatureType", "CreatureType", "uint8", ""},
        CreateFieldMetadata{31, FieldVerification::Verified, "displayPower", "", "uint8", ""},
        CreateFieldMetadata{32, FieldVerification::StructureOnly, "unknownU32Create32", "OverrideDisplayPowerID", "uint32", ""},
        CreateFieldMetadata{33, FieldVerification::Verified, "health", "", "uint64", ""},
        CreateFieldMetadata{34, FieldVerification::Verified, "power", "", "int32-array", ""},
        CreateFieldMetadata{35, FieldVerification::Verified, "maxPower", "", "int32-array", ""},
        CreateFieldMetadata{36, FieldVerification::StructureOnly, "unknownFloatArrayCreate36", "PowerRegenFlatModifier", "float-array", ""},
        CreateFieldMetadata{37, FieldVerification::StructureOnly, "unknownFloatArrayCreate37", "PowerRegenInterruptedFlatModifier", "float-array", ""},
        CreateFieldMetadata{38, FieldVerification::Verified, "maxHealth", "", "uint64", ""},
        CreateFieldMetadata{39, FieldVerification::Verified, "level", "", "uint32", ""},
        CreateFieldMetadata{40, FieldVerification::Verified, "effectiveLevel", "", "uint32", ""},
        CreateFieldMetadata{41, FieldVerification::StructureOnly, "unknownI32Create41", "ContentTuningID", "int32", ""},
        CreateFieldMetadata{42, FieldVerification::StructureOnly, "unknownI32Create42", "ScalingLevelMin", "int32", ""},
        CreateFieldMetadata{43, FieldVerification::StructureOnly, "unknownI32Create43", "ScalingLevelMax", "int32", ""},
        CreateFieldMetadata{44, FieldVerification::StructureOnly, "unknownI32Create44", "ScalingLevelDelta", "int32", ""},
        CreateFieldMetadata{45, FieldVerification::StructureOnly, "unknownI32Create45", "ScalingFactionGroup", "int32", ""},
        CreateFieldMetadata{46, FieldVerification::StructureOnly, "factionTemplate", "FactionTemplate", "int32", ""},
        CreateFieldMetadata{47, FieldVerification::StructureOnly, "virtualItems", "VirtualItems", "nested-array", ""},
        CreateFieldMetadata{48, FieldVerification::Verified, "unitFlags", "", "uint32", ""},
        CreateFieldMetadata{49, FieldVerification::Verified, "unitFlags2", "", "uint32", ""},
        CreateFieldMetadata{50, FieldVerification::Verified, "unitFlags3", "", "uint32", ""},
        CreateFieldMetadata{51, FieldVerification::Verified, "unitFlags4", "", "uint32", ""},
        CreateFieldMetadata{52, FieldVerification::Verified, "auraState", "", "uint32", ""},
        CreateFieldMetadata{53, FieldVerification::StructureOnly, "attackRoundBaseTime", "AttackRoundBaseTime", "uint32-array", ""},
        CreateFieldMetadata{54, FieldVerification::StructureOnly, "rangedAttackRoundBaseTime", "RangedAttackRoundBaseTime", "uint32", "ownerVisible"},
        CreateFieldMetadata{55, FieldVerification::StructureOnly, "boundingRadius", "BoundingRadius", "float", ""},
        CreateFieldMetadata{56, FieldVerification::StructureOnly, "combatReach", "CombatReach", "float", ""},
        CreateFieldMetadata{57, FieldVerification::StructureOnly, "displayScale", "DisplayScale", "float", ""},
        CreateFieldMetadata{58, FieldVerification::StructureOnly, "unknownI32Create58", "CreatureFamily", "int32", ""},
        CreateFieldMetadata{59, FieldVerification::StructureOnly, "unknownI32Create59", "OverrideCreatureType", "int32", ""},
        CreateFieldMetadata{60, FieldVerification::StructureOnly, "nativeDisplayId", "NativeDisplayID", "int32", ""},
        CreateFieldMetadata{61, FieldVerification::StructureOnly, "unknownFloatCreate61", "NativeXDisplayScale", "float", ""},
        CreateFieldMetadata{62, FieldVerification::StructureOnly, "mountDisplayId", "MountDisplayID", "int32", ""},
        CreateFieldMetadata{63, FieldVerification::StructureOnly, "unknownI32Create63", "CosmeticMountDisplayID", "int32", ""},
        CreateFieldMetadata{64, FieldVerification::Verified, "minDamage", "", "float", "ownerVisible"},
        CreateFieldMetadata{65, FieldVerification::Verified, "maxDamage", "", "float", "ownerVisible"},
        CreateFieldMetadata{66, FieldVerification::Verified, "minOffHandDamage", "", "float", "ownerVisible"},
        CreateFieldMetadata{67, FieldVerification::Verified, "maxOffHandDamage", "", "float", "ownerVisible"},
        CreateFieldMetadata{68, FieldVerification::Verified, "standState", "", "uint8", ""},
        CreateFieldMetadata{69, FieldVerification::Verified, "petTalentPoints", "", "uint8", ""},
        CreateFieldMetadata{70, FieldVerification::Verified, "visFlags", "", "uint8", ""},
        CreateFieldMetadata{71, FieldVerification::Verified, "animTier", "", "uint8", ""},
        CreateFieldMetadata{72, FieldVerification::StructureOnly, "unknownU32Create72", "PetNumber", "uint32", ""},
        CreateFieldMetadata{73, FieldVerification::StructureOnly, "unknownU32Create73", "PetNameTimestamp", "uint32", ""},
        CreateFieldMetadata{74, FieldVerification::StructureOnly, "unknownU32Create74", "PetExperience", "uint32", ""},
        CreateFieldMetadata{75, FieldVerification::StructureOnly, "unknownU32Create75", "", "uint32", ""},
        CreateFieldMetadata{76, FieldVerification::StructureOnly, "unknownU32Create76", "PetNextLevelExperience", "uint32", ""},
        CreateFieldMetadata{77, FieldVerification::StructureOnly, "unknownFloatCreate77", "ModCastingSpeed", "float", ""},
        CreateFieldMetadata{78, FieldVerification::StructureOnly, "unknownFloatCreate78", "ModCastingSpeedNeg", "float", ""},
        CreateFieldMetadata{79, FieldVerification::StructureOnly, "unknownFloatCreate79", "ModSpellHaste", "float", ""},
        CreateFieldMetadata{80, FieldVerification::StructureOnly, "unknownFloatCreate80", "ModHaste", "float", ""},
        CreateFieldMetadata{81, FieldVerification::StructureOnly, "unknownFloatCreate81", "ModRangedHaste", "float", ""},
        CreateFieldMetadata{82, FieldVerification::StructureOnly, "unknownFloatCreate82", "ModHasteRegen", "float", ""},
        CreateFieldMetadata{83, FieldVerification::StructureOnly, "unknownFloatCreate83", "ModTimeRate", "float", ""},
        CreateFieldMetadata{84, FieldVerification::StructureOnly, "createdBySpell", "CreatedBySpell", "int32", ""},
        CreateFieldMetadata{85, FieldVerification::StructureOnly, "emoteState", "EmoteState", "int32", ""},
        CreateFieldMetadata{86, FieldVerification::StructureOnly, "unknownOwnerI32Create86", "", "int32", "ownerVisible"},
        CreateFieldMetadata{87, FieldVerification::Verified, "stats", "", "stat-record-array", "ownerVisible"},
        CreateFieldMetadata{88, FieldVerification::Verified, "resistances", "", "int32-array", "ownerVisible"},
        CreateFieldMetadata{89, FieldVerification::StructureOnly, "unknownResistanceModifierCluster", "BonusResistanceMods/ManaCostModifier", "paired-array", "ownerVisible"},
        CreateFieldMetadata{90, FieldVerification::StructureOnly, "baseMana", "BaseMana", "int32", ""},
        CreateFieldMetadata{91, FieldVerification::StructureOnly, "baseHealth", "BaseHealth", "int32", "ownerVisible"},
        CreateFieldMetadata{92, FieldVerification::Verified, "sheatheState", "", "uint8", ""},
        CreateFieldMetadata{93, FieldVerification::Verified, "pvpFlags", "", "uint8", ""},
        CreateFieldMetadata{94, FieldVerification::Verified, "petFlags", "", "uint8", ""},
        CreateFieldMetadata{95, FieldVerification::Verified, "shapeshiftForm", "", "uint8", ""},
        CreateFieldMetadata{96, FieldVerification::StructureOnly, "unknownAttackPowerCluster", "AttackPower", "scalar-cluster", "ownerVisible"},
        CreateFieldMetadata{97, FieldVerification::StructureOnly, "unknownOwnerScalarCluster97", "", "scalar-cluster", "ownerVisible"},
        CreateFieldMetadata{98, FieldVerification::StructureOnly, "unknownRangedAttackPowerCluster", "RangedAttackPower", "scalar-cluster", "ownerVisible"},
        CreateFieldMetadata{99, FieldVerification::StructureOnly, "unknownWeaponPowerCluster", "WeaponAttackPower/Damage", "scalar-cluster", "ownerVisible"},
        CreateFieldMetadata{100, FieldVerification::StructureOnly, "maxHealthModifier", "MaxHealthModifier", "float", ""},
        CreateFieldMetadata{101, FieldVerification::StructureOnly, "hoverHeight", "HoverHeight", "float", ""},
        CreateFieldMetadata{102, FieldVerification::StructureOnly, "unknownI32Create102", "MinItemLevelCutoff", "int32", ""},
        CreateFieldMetadata{103, FieldVerification::StructureOnly, "unknownI32Create103", "MinItemLevel", "int32", ""},
        CreateFieldMetadata{104, FieldVerification::StructureOnly, "unknownI32Create104", "MaxItemLevel", "int32", ""},
        CreateFieldMetadata{105, FieldVerification::StructureOnly, "unknownI32Create105", "AzeriteItemLevel", "int32", ""},
        CreateFieldMetadata{106, FieldVerification::StructureOnly, "unknownU8Create106", "WildBattlePetLevel", "uint8", ""},
        CreateFieldMetadata{107, FieldVerification::StructureOnly, "unknownU32Create107", "BattlePetCompanionExperience", "uint32", ""},
        CreateFieldMetadata{108, FieldVerification::StructureOnly, "unknownU32Create108", "BattlePetCompanionNameTimestamp", "uint32", ""},
        CreateFieldMetadata{109, FieldVerification::StructureOnly, "unknownI32Create109", "InteractSpellID", "int32", ""},
        CreateFieldMetadata{110, FieldVerification::StructureOnly, "unknownI32Create110", "ScaleDuration", "int32", ""},
        CreateFieldMetadata{111, FieldVerification::StructureOnly, "unknownI32Create111", "LooksLikeMountID", "int32", ""},
        CreateFieldMetadata{112, FieldVerification::StructureOnly, "unknownI32Create112", "LooksLikeCreatureID", "int32", ""},
        CreateFieldMetadata{113, FieldVerification::StructureOnly, "unknownI32Create113", "LookAtControllerID", "int32", ""},
        CreateFieldMetadata{114, FieldVerification::StructureOnly, "unknownI32Create114", "PerksVendorItemID", "int32", ""},
        CreateFieldMetadata{115, FieldVerification::StructureOnly, "unknownI32Create115", "TaxiNodesID", "int32", ""},
        CreateFieldMetadata{116, FieldVerification::StructureOnly, "unknownGuidCreate116", "", "packed-guid", ""},
        CreateFieldMetadata{117, FieldVerification::StructureOnly, "unknownU32Create117", "PassiveSpellsCount", "uint32", ""},
        CreateFieldMetadata{118, FieldVerification::StructureOnly, "unknownU32Create118", "WorldEffectsCount", "uint32", ""},
        CreateFieldMetadata{119, FieldVerification::StructureOnly, "unknownU32Create119", "ChannelObjectsCount", "uint32", ""},
        CreateFieldMetadata{120, FieldVerification::StructureOnly, "unknownI32Create120", "FlightCapabilityID", "int32", ""},
        CreateFieldMetadata{121, FieldVerification::StructureOnly, "unknownFloatCreate121", "GlideEventSpeedDivisor", "float", ""},
        CreateFieldMetadata{122, FieldVerification::StructureOnly, "unknownI32Create122", "DriveCapabilityID", "int32", ""},
        CreateFieldMetadata{123, FieldVerification::StructureOnly, "unknownFloatCreate123", "MaxHealthModifierFlatNeg", "float", ""},
        CreateFieldMetadata{124, FieldVerification::StructureOnly, "unknownFloatCreate124", "MaxHealthModifierFlatPos", "float", ""},
        CreateFieldMetadata{125, FieldVerification::StructureOnly, "unknownU32Create125", "SilencedSchoolMask", "uint32", ""},
        CreateFieldMetadata{126, FieldVerification::StructureOnly, "unknownOwnerI32Create126", "", "int32", "ownerVisible"},
        CreateFieldMetadata{127, FieldVerification::StructureOnly, "unknownI32Create127", "CurrentAreaID", "int32", ""},
        CreateFieldMetadata{128, FieldVerification::StructureOnly, "unknownFloatCreate128", "NameplateDistanceMod", "float", ""},
        CreateFieldMetadata{129, FieldVerification::StructureOnly, "unknownFloatCreate129", "AutoAttackRangeMod", "float", ""},
        CreateFieldMetadata{130, FieldVerification::StructureOnly, "unknownOwnerExtension", "", "opaque-record", "ownerVisible"},
        CreateFieldMetadata{131, FieldVerification::StructureOnly, "unknownGuidCreate131", "NameplateAttachToGUID", "packed-guid", ""},
        CreateFieldMetadata{132, FieldVerification::StructureOnly, "unknownRecordVector132", "PassiveSpells", "record-vector", ""},
        CreateFieldMetadata{133, FieldVerification::StructureOnly, "unknownI32Vector133", "WorldEffects", "int32-vector", ""},
        CreateFieldMetadata{134, FieldVerification::StructureOnly, "unknownGuidVector134", "ChannelObjects", "packed-guid-vector", ""},
        CreateFieldMetadata{135, FieldVerification::StructureOnly, "unknownBitCreate135", "Field314", "bit", ""},
        CreateFieldMetadata{136, FieldVerification::StructureOnly, "unknownOptionalRecordPresent", "", "bit", ""},
        CreateFieldMetadata{137, FieldVerification::StructureOnly, "unknownOptionalRecord0", "UnitAssistActionData", "optional-record", ""}
    }};

    static_assert(hasContiguousCreateFieldOrder(UnitDataCreateFields));
    static_assert(referenceCreateFieldsHaveReferenceNames(UnitDataCreateFields));
    static_assert(referenceCreateFieldsUseNeutralNames(UnitDataCreateFields));
    static_assert(verifiedCreateFieldsHaveNoReferenceNames(UnitDataCreateFields));
    static_assert(countCreateFieldsByVerification(UnitDataCreateFields, FieldVerification::Verified) == 29);
    static_assert(countCreateFieldsByVerification(UnitDataCreateFields, FieldVerification::StructureOnly) == 102);
    static_assert(countCreateFieldsByVerification(UnitDataCreateFields, FieldVerification::ReferenceOnly) == 7);
    static_assert(countCreateFieldsByVerification(UnitDataCreateFields, FieldVerification::Unknown) == 0);

    // Only Forever-verified differential fields are permitted here. Verification metadata is
    // intentionally part of the descriptor so debug traces cannot accidentally present a
    // modern reference-schema label as confirmed Forever semantics.
    using UnitDataUpdate = UpdateDefinition<Fields::UnitData::ChangeMaskSize,
        ScalarField<&Fields::UnitData::displayPower, Fields::UnitData::DisplayPowerBit, 32, FieldVerification::Verified, "displayPower">,
        ScalarField<&Fields::UnitData::health, Fields::UnitData::HealthBit, 32, FieldVerification::Verified, "health">,
        ScalarField<&Fields::UnitData::maxHealth, Fields::UnitData::MaxHealthBit, 32, FieldVerification::Verified, "maxHealth">,
        ScalarField<&Fields::UnitData::level, Fields::UnitData::LevelBit, 32, FieldVerification::Verified, "level">,
        ScalarField<&Fields::UnitData::effectiveLevel, Fields::UnitData::EffectiveLevelBit, 32, FieldVerification::Verified, "effectiveLevel">,
        ScalarField<&Fields::UnitData::unitFlags, Fields::UnitData::FlagsBit, 32, FieldVerification::Verified, "unitFlags">,
        ScalarField<&Fields::UnitData::unitFlags2, Fields::UnitData::Flags2Bit, 32, FieldVerification::Verified, "unitFlags2">,
        ScalarField<&Fields::UnitData::auraState, Fields::UnitData::AuraStateBit, 32, FieldVerification::Verified, "auraState">,
        ScalarArrayField<&Fields::UnitData::power, Fields::UnitData::PowerGroupBit, Fields::UnitData::PowerFirstBit, FieldVerification::Verified, "power">,
        ScalarArrayField<&Fields::UnitData::maxPower, Fields::UnitData::PowerGroupBit, Fields::UnitData::MaxPowerFirstBit, FieldVerification::Verified, "maxPower">,
        ScalarArrayField<&Fields::UnitData::resistances, Fields::UnitData::ResistancesGroupBit, Fields::UnitData::ResistancesFirstBit, FieldVerification::Verified, "resistances">>;

    static_assert(updateFieldsMatchCreateMetadata(UnitDataCreateFields, UnitDataUpdate::Metadata));
}
