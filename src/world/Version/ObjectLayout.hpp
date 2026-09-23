/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    Version specific layouts of the WoW data structs. Code addresses a field by its version
//          neutral id (ObjectFields.hpp), the layout table of the active server expansion resolves
//          it to the byte offset and size inside the object values. The tables live in
//          Version/Layouts/ObjectLayout_<Version>.cpp and are generated from the Data/WoW*.hpp
//          structs until those are retired.

#pragma once

#include "ObjectFields.hpp"
#include "Server/ClientProtocol.hpp"

#include <cstddef>
#include <cstdint>

namespace Version
{
    /// byte offset and byte size of one field inside the object values
    struct FieldDesc
    {
        uint16_t offset;
        uint16_t size;
    };

    /// offset value of a field that does not exist in the client version
    inline constexpr uint16_t kNoField = 0xFFFF;

    /// all fields of one struct for one client version, indexed by the field id enum
    struct LayoutTable
    {
        const FieldDesc* fields;
        uint16_t fieldCount;
        uint16_t structSize;

        template <typename FieldId>
        [[nodiscard]] const FieldDesc& get(FieldId id) const noexcept
        {
            static constexpr FieldDesc noField{ kNoField, 0 };
            const auto index = static_cast<size_t>(id);
            return index < fieldCount ? fields[index] : noField;
        }

        template <typename FieldId>
        [[nodiscard]] bool has(FieldId id) const noexcept { return get(id).offset != kNoField; }

        /// byte offset of the field, kNoField when it is not part of this client version
        template <typename FieldId>
        [[nodiscard]] uint16_t offset(FieldId id) const noexcept { return get(id).offset; }

        /// index of the field in uint32 units, as used by the update masks
        template <typename FieldId>
        [[nodiscard]] uint16_t index(FieldId id) const noexcept
        {
            const auto desc = get(id);
            return desc.offset == kNoField ? kNoField : static_cast<uint16_t>(desc.offset / sizeof(uint32_t));
        }

        /// number of uint32 values the struct occupies
        [[nodiscard]] uint16_t valueCount() const noexcept { return static_cast<uint16_t>(structSize / sizeof(uint32_t)); }
    };

    /// the struct layouts of one client version, each table holds the own fields of its struct
    /// (a player object uses object, unit and player, a container uses object, item and container)
    struct ExpansionLayouts
    {
        LayoutTable object;
        LayoutTable unit;
        LayoutTable player;
        LayoutTable item;
        LayoutTable container;
        LayoutTable gameObject;
        LayoutTable dynamicObject;
        LayoutTable corpse;
        LayoutTable areaTrigger;
    };

    namespace Tables
    {
        extern const ExpansionLayouts classicLayouts;
        extern const ExpansionLayouts tbcLayouts;
        extern const ExpansionLayouts wotlkLayouts;
        extern const ExpansionLayouts cataLayouts;
        extern const ExpansionLayouts mopLayouts;
    }

    [[nodiscard]] bool hasLayoutsForExpansion(WoW::Expansion expansion) noexcept;

    /// The layouts of an expansion, an empty layout set (every field absent, size 0) for expansions without tables
    [[nodiscard]] const ExpansionLayouts& layoutsForExpansion(WoW::Expansion expansion) noexcept;
}
