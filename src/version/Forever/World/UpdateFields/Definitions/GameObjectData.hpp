/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    // [FOREVER-STRUCTURE] Captured structure; individual semantic labels remain reference unless explicitly proven.
    using GameObjectDataUpdate = UnfilteredUpdateDefinition<Fields::GameObjectData::ChangeMaskSize,
        ScalarVectorField<&Fields::GameObjectData::unknownU32Vector0, Fields::GameObjectData::StateWorldEffectIdsBit>,
        ScalarVectorField<&Fields::GameObjectData::enableDoodadSets, Fields::GameObjectData::EnableDoodadSetsBit>,
        ScalarVectorField<&Fields::GameObjectData::worldEffects, Fields::GameObjectData::WorldEffectsBit>,
        ScalarField<&Fields::GameObjectData::displayId, Fields::GameObjectData::DisplayIdBit>,
        ScalarField<&Fields::GameObjectData::spellVisualId, Fields::GameObjectData::SpellVisualIdBit>,
        ScalarField<&Fields::GameObjectData::unknownU32Create3, Fields::GameObjectData::StateSpellVisualIdBit>,
        ScalarField<&Fields::GameObjectData::spawnTrackingStateAnimId, Fields::GameObjectData::SpawnTrackingStateAnimIdBit>,
        ScalarField<&Fields::GameObjectData::spawnTrackingStateAnimKitId, Fields::GameObjectData::SpawnTrackingStateAnimKitIdBit>,
        ScalarField<&Fields::GameObjectData::unknownU32Create7, Fields::GameObjectData::StateWorldEffectsQuestObjectiveIdBit>,
        GuidField<&Fields::GameObjectData::createdBy, Fields::GameObjectData::CreatedByBit>,
        GuidField<&Fields::GameObjectData::guildGuid, Fields::GameObjectData::GuildGuidBit>,
        ScalarField<&Fields::GameObjectData::flags, Fields::GameObjectData::FlagsBit>,
        ScalarField<&Fields::GameObjectData::flagsB, Fields::GameObjectData::FlagsBBit>,
        WholeScalarArrayField<&Fields::GameObjectData::parentRotation, Fields::GameObjectData::ParentRotationBit>,
        ScalarField<&Fields::GameObjectData::factionTemplate, Fields::GameObjectData::FactionTemplateBit>,
        ScalarField<&Fields::GameObjectData::state, Fields::GameObjectData::StateBit>,
        ScalarField<&Fields::GameObjectData::typeId, Fields::GameObjectData::TypeIdBit>,
        ScalarField<&Fields::GameObjectData::percentHealth, Fields::GameObjectData::PercentHealthBit>,
        ScalarField<&Fields::GameObjectData::artKit, Fields::GameObjectData::ArtKitBit>,
        ScalarField<&Fields::GameObjectData::customParam, Fields::GameObjectData::CustomParamBit>,
        ScalarField<&Fields::GameObjectData::level, Fields::GameObjectData::LevelBit>,
        ScalarField<&Fields::GameObjectData::animGroupInstance, Fields::GameObjectData::AnimGroupInstanceBit>,
        ScalarField<&Fields::GameObjectData::uiWidgetItemId, Fields::GameObjectData::UiWidgetItemIdBit>,
        ScalarField<&Fields::GameObjectData::uiWidgetItemQuality, Fields::GameObjectData::UiWidgetItemQualityBit>,
        ScalarField<&Fields::GameObjectData::uiWidgetItemCount, Fields::GameObjectData::UiWidgetItemCountBit>,
        ScalarField<&Fields::GameObjectData::unknownU32_26, Fields::GameObjectData::UnknownU32Bit26>,
        ScalarField<&Fields::GameObjectData::unknownU32_27, Fields::GameObjectData::UnknownU32Bit27>>;
}
