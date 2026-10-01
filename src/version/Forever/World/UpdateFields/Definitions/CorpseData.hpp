/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    // REFERENCE semantic names are kept only in ReferenceName metadata.
    using CorpseDataUpdate = UnfilteredUpdateDefinition<Fields::CorpseData::ChangeMaskSize,
        ScalarField<&Fields::CorpseData::dynamicFlags, Fields::CorpseData::DynamicFlagsBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU32Bit1", "DynamicFlags">,
        GuidField<&Fields::CorpseData::owner, Fields::CorpseData::OwnerBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownGuidBit2", "Owner">,
        GuidField<&Fields::CorpseData::partyGuid, Fields::CorpseData::PartyGuidBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownGuidBit3", "PartyGUID">,
        GuidField<&Fields::CorpseData::guildGuid, Fields::CorpseData::GuildGuidBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownGuidBit4", "GuildGUID">,
        ScalarField<&Fields::CorpseData::displayId, Fields::CorpseData::DisplayIdBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownI32Bit5", "DisplayID">,
        ScalarField<&Fields::CorpseData::raceId, Fields::CorpseData::RaceIdBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU8Bit6", "RaceID">,
        ScalarField<&Fields::CorpseData::sex, Fields::CorpseData::SexBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU8Bit7", "Sex">,
        ScalarField<&Fields::CorpseData::classId, Fields::CorpseData::ClassBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU8Bit8", "Class">,
        ScalarField<&Fields::CorpseData::flags, Fields::CorpseData::FlagsBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU32Bit9", "Flags">,
        ScalarField<&Fields::CorpseData::factionTemplate, Fields::CorpseData::FactionTemplateBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownI32Bit10", "FactionTemplate">,
        ScalarField<&Fields::CorpseData::stateSpellVisualKitId, Fields::CorpseData::StateSpellVisualKitIdBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownI32Bit11", "StateSpellVisualKitID">,
        ScalarArrayField<&Fields::CorpseData::items, Fields::CorpseData::ItemsGroupBit, Fields::CorpseData::ItemsFirstBit, FieldVerification::ReferenceOnly, "unknownU32Array", "Items">>;
}
