/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ChangeMask.hpp"
#include "WireHelpers.hpp"

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace AscEmu::Version::Forever::UpdateFields
{
    inline constexpr std::size_t NoParentBit = static_cast<std::size_t>(-1);

    // VERIFIED: Forever captures/runtime differentials prove the semantic identity.
    // STRUCTURE: serializer order and wire shape/type are known, but the semantic identity is not proven.
    // REFERENCE: serializer order and wire shape/type are known and a modern reference label exists, but Forever has not proven it.
    // UNKNOWN: the remaining payload/bit is still opaque enough that even its structure should not be treated as settled.
    enum class FieldVerification : uint8_t
    {
        Verified,
        StructureOnly,
        ReferenceOnly,
        Unknown
    };

    struct CreateFieldMetadata
    {
        std::size_t order;
        FieldVerification verification;
        std::string_view name;
        std::string_view referenceName;
        std::string_view wireType;
        std::string_view condition;
    };

    struct UpdateFieldMetadata
    {
        FieldVerification verification;
        std::string_view name;
        std::string_view wireKind;
    };

    constexpr bool createWireTypeMatchesUpdateKind(std::string_view createWireType, std::string_view updateWireKind)
    {
        if (updateWireKind == "guid")
            return createWireType == "packed-guid";
        if (updateWireKind == "guid-array")
            return createWireType.starts_with("packed-guid[");
        if (updateWireKind == "scalar-array")
            return createWireType.ends_with("-array");
        if (updateWireKind == "scalar")
            return !createWireType.empty() && createWireType.find("array") == std::string_view::npos && createWireType.find("vector") == std::string_view::npos && createWireType.find("record") == std::string_view::npos && createWireType.find("guid") == std::string_view::npos && createWireType != "byte-span";
        return true;
    }

    template <std::size_t CreateCount, std::size_t UpdateCount>
    constexpr bool updateFieldsMatchCreateMetadata(std::array<CreateFieldMetadata, CreateCount> const& createFields, std::array<UpdateFieldMetadata, UpdateCount> const& updateFields)
    {
        for (auto const& updateField : updateFields)
        {
            bool found = false;
            for (auto const& createField : createFields)
            {
                if (createField.name != updateField.name)
                    continue;
                found = true;
                if (createField.verification != updateField.verification)
                    return false;
                if (!createWireTypeMatchesUpdateKind(createField.wireType, updateField.wireKind))
                    return false;
                break;
            }
            if (!found)
                return false;
        }
        return true;
    }

    template <std::size_t N>
    constexpr std::size_t countCreateFieldsByVerification(std::array<CreateFieldMetadata, N> const& fields, FieldVerification verification)
    {
        std::size_t count = 0;
        for (auto const& field : fields)
            if (field.verification == verification)
                ++count;
        return count;
    }

    template <std::size_t N>
    constexpr bool hasContiguousCreateFieldOrder(std::array<CreateFieldMetadata, N> const& fields)
    {
        for (std::size_t i = 0; i < N; ++i)
            if (fields[i].order != i)
                return false;
        return true;
    }

    template <std::size_t N>
    constexpr bool referenceCreateFieldsHaveReferenceNames(std::array<CreateFieldMetadata, N> const& fields)
    {
        for (auto const& field : fields)
            if (field.verification == FieldVerification::ReferenceOnly && field.referenceName.empty())
                return false;
        return true;
    }

    template <std::size_t N>
    constexpr bool referenceCreateFieldsUseNeutralNames(std::array<CreateFieldMetadata, N> const& fields)
    {
        for (auto const& field : fields)
            if (field.verification == FieldVerification::ReferenceOnly && !field.name.starts_with("unknown"))
                return false;
        return true;
    }

    template <std::size_t N>
    constexpr bool verifiedCreateFieldsHaveNoReferenceNames(std::array<CreateFieldMetadata, N> const& fields)
    {
        for (auto const& field : fields)
            if (field.verification == FieldVerification::Verified && !field.referenceName.empty())
                return false;
        return true;
    }

    template <std::size_t N>
    struct FixedString
    {
        char value[N]{};

        constexpr FixedString(char const (&text)[N])
        {
            for (std::size_t i = 0; i < N; ++i)
                value[i] = text[i];
        }

        constexpr std::string_view view() const { return std::string_view(value, N - 1); }
    };

    template <auto Member, std::size_t Bit, std::size_t ParentBit = NoParentBit, FieldVerification Verification = FieldVerification::Unknown, FixedString Name = "unknown", FixedString ReferenceName = "">
    struct ScalarField
    {
        static constexpr UpdateFieldMetadata metadata() { return {Verification, Name.view(), "scalar"}; }
        template <typename Owner, std::size_t N>
        static void copyKnownBits(Owner const&, std::bitset<N> const& source, std::bitset<N>& target)
        {
            if constexpr (ParentBit != NoParentBit)
                if (source.test(ParentBit)) target.set(ParentBit);
            if (source.test(Bit)) target.set(Bit);
        }

        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if constexpr (ParentBit != NoParentBit)
                if (!changed(ParentBit)) return;
            if (changed(Bit)) data << owner.*Member;
        }

        template <typename Owner>
        static void writeCreate(ByteBuffer& data, Owner const& owner)
        {
            data << owner.*Member;
        }

    };

    template <auto Member, std::size_t Bit, std::size_t ParentBit = NoParentBit, FieldVerification Verification = FieldVerification::Unknown, FixedString Name = "unknown", FixedString ReferenceName = "">
    struct GuidField
    {
        static constexpr UpdateFieldMetadata metadata() { return {Verification, Name.view(), "guid"}; }
        template <typename Owner, std::size_t N>
        static void copyKnownBits(Owner const&, std::bitset<N> const& source, std::bitset<N>& target)
        {
            if constexpr (ParentBit != NoParentBit)
                if (source.test(ParentBit)) target.set(ParentBit);
            if (source.test(Bit)) target.set(Bit);
        }

        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if constexpr (ParentBit != NoParentBit)
                if (!changed(ParentBit)) return;
            if (changed(Bit)) writeModernGuid(data, owner.*Member);
        }

    };

    template <auto Member, std::size_t GroupBit, std::size_t FirstBit, FieldVerification Verification = FieldVerification::Unknown, FixedString Name = "unknown", FixedString ReferenceName = "">
    struct ScalarArrayField
    {
        static constexpr UpdateFieldMetadata metadata() { return {Verification, Name.view(), "scalar-array"}; }
        template <typename Owner, std::size_t N>
        static void copyKnownBits(Owner const& owner, std::bitset<N> const& source, std::bitset<N>& target)
        {
            if (source.test(GroupBit)) target.set(GroupBit);
            auto const& values = owner.*Member;
            for (std::size_t i = 0; i < values.size(); ++i)
                if (source.test(FirstBit + i)) target.set(FirstBit + i);
        }

        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if (!changed(GroupBit)) return;
            auto const& values = owner.*Member;
            for (std::size_t i = 0; i < values.size(); ++i)
                if (changed(FirstBit + i)) data << values[i];
        }

    };

    template <auto Member, std::size_t Bit, std::size_t ParentBit = NoParentBit, FieldVerification Verification = FieldVerification::Unknown, FixedString Name = "unknown", FixedString ReferenceName = "">
    struct ScalarVectorField
    {
        template <typename Owner, std::size_t N>
        static void copyKnownBits(Owner const&, std::bitset<N> const& source, std::bitset<N>& target)
        {
            if constexpr (ParentBit != NoParentBit)
                if (source.test(ParentBit)) target.set(ParentBit);
            if (source.test(Bit)) target.set(Bit);
        }

        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if constexpr (ParentBit != NoParentBit)
                if (!changed(ParentBit)) return;
            if (!changed(Bit)) return;
            auto const& values = owner.*Member;
            data << static_cast<uint32_t>(values.size());
            for (auto const& value : values)
                data << value;
        }

    };

    template <auto Member, std::size_t Bit, std::size_t ParentBit = NoParentBit, FieldVerification Verification = FieldVerification::Unknown, FixedString Name = "unknown", FixedString ReferenceName = "">
    struct WholeScalarArrayField
    {
        template <typename Owner, std::size_t N>
        static void copyKnownBits(Owner const&, std::bitset<N> const& source, std::bitset<N>& target)
        {
            if constexpr (ParentBit != NoParentBit)
                if (source.test(ParentBit)) target.set(ParentBit);
            if (source.test(Bit)) target.set(Bit);
        }

        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if constexpr (ParentBit != NoParentBit)
                if (!changed(ParentBit)) return;
            if (!changed(Bit)) return;
            for (auto const& value : owner.*Member)
                data << value;
        }

    };

    template <auto Member, std::size_t GroupBit, std::size_t FirstBit, FieldVerification Verification = FieldVerification::Unknown, FixedString Name = "unknown", FixedString ReferenceName = "">
    struct GuidArrayField
    {
        static constexpr UpdateFieldMetadata metadata() { return {Verification, Name.view(), "guid-array"}; }
        template <typename Owner, std::size_t N>
        static void copyKnownBits(Owner const& owner, std::bitset<N> const& source, std::bitset<N>& target)
        {
            if (source.test(GroupBit)) target.set(GroupBit);
            auto const& values = owner.*Member;
            for (std::size_t i = 0; i < values.size(); ++i)
                if (source.test(FirstBit + i)) target.set(FirstBit + i);
        }

        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner, auto const& changed)
        {
            if (!changed(GroupBit)) return;
            auto const& values = owner.*Member;
            for (std::size_t i = 0; i < values.size(); ++i)
                if (changed(FirstBit + i)) writeModernGuid(data, values[i]);
        }

    };

    template <std::size_t MaskSize, typename... FieldDescriptors>
    struct UpdateDefinition
    {
        inline static constexpr std::array<UpdateFieldMetadata, sizeof...(FieldDescriptors)> Metadata{{FieldDescriptors::metadata()...}};
        template <typename Owner>
        static std::bitset<MaskSize> filterChanges(Owner const& owner)
        {
            std::bitset<MaskSize> filtered{};
            (FieldDescriptors::copyKnownBits(owner, owner.changes, filtered), ...);
            return filtered;
        }

        template <typename Owner>
        static void writeUpdate(ByteBuffer& data, Owner const& owner)
        {
            const auto changes = filterChanges(owner);
            writeChangeMask(data, changes);
            const auto changed = [&changes](std::size_t bit) { return changes.test(bit); };
            (FieldDescriptors::write(data, owner, changed), ...);
        }

    };

    template <std::size_t MaskSize, typename... FieldDescriptors>
    struct UnfilteredUpdateDefinition
    {
        template <typename Owner>
        static void writeUpdate(ByteBuffer& data, Owner const& owner)
        {
            writeChangeMask(data, owner.changes);
            const auto changed = [&owner](std::size_t bit) { return owner.changes.test(bit); };
            (FieldDescriptors::write(data, owner, changed), ...);
        }

    };

    template <typename... FieldDescriptors>
    struct CreateDefinition
    {
        template <typename Owner>
        static void write(ByteBuffer& data, Owner const& owner)
        {
            (FieldDescriptors::writeCreate(data, owner), ...);
        }
    };
}
