/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    // REFERENCE: canonical metadata names are deliberately neutral. The final string is
    // the external schema reference label and must never be presented as Forever-proven.
    using ItemDataUpdate = UnfilteredUpdateDefinition<Fields::ItemData::ChangeMaskSize,
        GuidField<&Fields::ItemData::owner, Fields::ItemData::OwnerBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownGuidBit3", "Owner">,
        GuidField<&Fields::ItemData::containedIn, Fields::ItemData::ContainedInBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownGuidBit4", "ContainedIn">,
        GuidField<&Fields::ItemData::creator, Fields::ItemData::CreatorBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownGuidBit5", "Creator">,
        GuidField<&Fields::ItemData::giftCreator, Fields::ItemData::GiftCreatorBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownGuidBit6", "GiftCreator">,
        ScalarField<&Fields::ItemData::stackCount, Fields::ItemData::StackCountBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU32Bit7", "StackCount">,
        ScalarField<&Fields::ItemData::expiration, Fields::ItemData::ExpirationBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU32Bit8", "Expiration">,
        ScalarField<&Fields::ItemData::dynamicFlags, Fields::ItemData::DynamicFlagsBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU32Bit9", "DynamicFlags">,
        ScalarField<&Fields::ItemData::durability, Fields::ItemData::DurabilityBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU32Bit10", "Durability">,
        ScalarField<&Fields::ItemData::maxDurability, Fields::ItemData::MaxDurabilityBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU32Bit11", "MaxDurability">,
        ScalarField<&Fields::ItemData::createPlayedTime, Fields::ItemData::CreatePlayedTimeBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownU32Bit12", "CreatePlayedTime">,
        ScalarArrayField<&Fields::ItemData::spellCharges, Fields::ItemData::SpellChargesGroupBit, Fields::ItemData::SpellChargesFirstBit, FieldVerification::ReferenceOnly, "unknownI32Array", "SpellCharges">>;
}
