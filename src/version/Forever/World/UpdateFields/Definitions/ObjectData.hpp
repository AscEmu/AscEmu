/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    using ObjectDataCreate = CreateDefinition<
        ScalarField<&Fields::ObjectData::entryId, Fields::ObjectData::EntryIdBit, NoParentBit, FieldVerification::Verified, "entryId">,
        ScalarField<&Fields::ObjectData::dynamicFlags, Fields::ObjectData::DynamicFlagsBit, NoParentBit, FieldVerification::Verified, "dynamicFlags">,
        ScalarField<&Fields::ObjectData::scale, Fields::ObjectData::ScaleBit, NoParentBit, FieldVerification::Verified, "scale">>;

    using ObjectDataUpdate = UpdateDefinition<Fields::ObjectData::ChangeMaskSize,
        ScalarField<&Fields::ObjectData::entryId, Fields::ObjectData::EntryIdBit, Fields::ObjectData::GroupBit, FieldVerification::Verified, "entryId">,
        ScalarField<&Fields::ObjectData::dynamicFlags, Fields::ObjectData::DynamicFlagsBit, Fields::ObjectData::GroupBit, FieldVerification::Verified, "dynamicFlags">,
        ScalarField<&Fields::ObjectData::scale, Fields::ObjectData::ScaleBit, Fields::ObjectData::GroupBit, FieldVerification::Verified, "scale">>;
}
