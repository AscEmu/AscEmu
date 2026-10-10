/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../FieldDefinition.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    template <auto Member, std::size_t Bit>
    struct ReferenceSpellCastVisualField
    {
        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if (!changed(Bit)) return;
            auto const& visual = owner.*Member;
            data << visual.spellXSpellVisualId << visual.scriptVisualId;
        }

    };

    using DynamicObjectDataUpdate = UnfilteredUpdateDefinition<Fields::DynamicObjectData::ChangeMaskSize,
        GuidField<&Fields::DynamicObjectData::caster, Fields::DynamicObjectData::CasterBit, NoParentBit, FieldVerification::Unverified, "unknownGuidBit1", "Caster">,
        ScalarField<&Fields::DynamicObjectData::type, Fields::DynamicObjectData::TypeBit, NoParentBit, FieldVerification::Unverified, "unknownU8Bit2", "Type">,
        ReferenceSpellCastVisualField<&Fields::DynamicObjectData::spellVisual, Fields::DynamicObjectData::SpellVisualBit>,
        ScalarField<&Fields::DynamicObjectData::spellId, Fields::DynamicObjectData::SpellIdBit, NoParentBit, FieldVerification::Unverified, "unknownI32Bit4", "SpellID">,
        ScalarField<&Fields::DynamicObjectData::radius, Fields::DynamicObjectData::RadiusBit, NoParentBit, FieldVerification::Unverified, "unknownFloatBit5", "Radius">,
        ScalarField<&Fields::DynamicObjectData::castTime, Fields::DynamicObjectData::CastTimeBit, NoParentBit, FieldVerification::Unverified, "unknownU32Bit6", "CastTime">>;
}
