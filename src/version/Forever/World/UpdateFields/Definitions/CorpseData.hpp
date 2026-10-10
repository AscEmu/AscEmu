/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    // UNVERIFIED semantic names are kept only in ReferenceName metadata.
    using CorpseDataUpdate = UnfilteredUpdateDefinition<Fields::CorpseData::ChangeMaskSize,
        ScalarField<&Fields::CorpseData::dynamicFlags, Fields::CorpseData::DynamicFlagsBit, NoParentBit, FieldVerification::Unverified, "unknownU32Bit1", "DynamicFlags">,
        GuidField<&Fields::CorpseData::owner, Fields::CorpseData::OwnerBit, NoParentBit, FieldVerification::Unverified, "unknownGuidBit2", "Owner">,
        GuidField<&Fields::CorpseData::partyGuid, Fields::CorpseData::PartyGuidBit, NoParentBit, FieldVerification::Unverified, "unknownGuidBit3", "PartyGUID">,
        GuidField<&Fields::CorpseData::guildGuid, Fields::CorpseData::GuildGuidBit, NoParentBit, FieldVerification::Unverified, "unknownGuidBit4", "GuildGUID">,
        ScalarField<&Fields::CorpseData::displayId, Fields::CorpseData::DisplayIdBit, NoParentBit, FieldVerification::Unverified, "unknownI32Bit5", "DisplayID">,
        ScalarField<&Fields::CorpseData::raceId, Fields::CorpseData::RaceIdBit, NoParentBit, FieldVerification::Unverified, "unknownU8Bit6", "RaceID">,
        ScalarField<&Fields::CorpseData::sex, Fields::CorpseData::SexBit, NoParentBit, FieldVerification::Unverified, "unknownU8Bit7", "Sex">,
        ScalarField<&Fields::CorpseData::classId, Fields::CorpseData::ClassBit, NoParentBit, FieldVerification::Unverified, "unknownU8Bit8", "Class">,
        ScalarField<&Fields::CorpseData::flags, Fields::CorpseData::FlagsBit, NoParentBit, FieldVerification::Unverified, "unknownU32Bit9", "Flags">,
        ScalarField<&Fields::CorpseData::factionTemplate, Fields::CorpseData::FactionTemplateBit, NoParentBit, FieldVerification::Unverified, "unknownI32Bit10", "FactionTemplate">,
        ScalarField<&Fields::CorpseData::stateSpellVisualKitId, Fields::CorpseData::StateSpellVisualKitIdBit, NoParentBit, FieldVerification::Unverified, "unknownI32Bit11", "StateSpellVisualKitID">,
        ScalarArrayField<&Fields::CorpseData::items, Fields::CorpseData::ItemsGroupBit, Fields::CorpseData::ItemsFirstBit, FieldVerification::Unverified, "unknownU32Array", "Items">>;
}
