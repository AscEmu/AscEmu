/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    // The wire positions are reused from the modern reference. The canonical descriptor names
    // stay neutral until Forever captures prove the semantic labels.
    using ContainerDataUpdate = UnfilteredUpdateDefinition<Fields::ContainerData::ChangeMaskSize,
        ScalarField<&Fields::ContainerData::numSlots, Fields::ContainerData::NumSlotsBit, NoParentBit, FieldVerification::ReferenceOnly, "unknownBit1", "NumSlots">,
        GuidArrayField<&Fields::ContainerData::slots, Fields::ContainerData::SlotsGroupBit, Fields::ContainerData::SlotsFirstBit, FieldVerification::ReferenceOnly, "unknownGuidArray", "Slots">>;
}
