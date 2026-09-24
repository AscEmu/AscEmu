/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    Version specific layouts of the WoW data structs. Code addresses a field by its version
//          neutral id (ObjectFields.hpp), the layout table of the active server expansion resolves
//          it to the byte offset and size inside the object values. The per version tables live in
//          Version/Layouts/ObjectLayout_<Version>.cpp as a list of the fields that exist in that
//          version, in struct order. The dense tables used at runtime are built from them once.

#pragma once

#include "ObjectFields.hpp"
#include "Server/ClientProtocol.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Version
{
    /// one field inside the object values: byte offset and size of the field (of one element for
    /// arrays), the element count and the byte stride between two elements
    struct FieldDesc
    {
        uint16_t offset;
        uint16_t size;
        uint16_t count;
        uint16_t stride;
    };

    /// offset value of a field that does not exist in the client version
    inline constexpr uint16_t kNoField = 0xFFFF;

    /// one line of a version table: which field sits where
    template <typename FieldId>
    struct FieldEntry
    {
        FieldId id;
        FieldDesc desc;
    };

    /// the fields of one struct in one client version, as written in the version table file
    template <typename FieldId>
    struct LayoutSource
    {
        const FieldEntry<FieldId>* entries;
        uint16_t entryCount;
        uint16_t structSize;
    };

    /// all fields of one struct for one client version, indexed by the field id enum
    struct LayoutTable
    {
        std::vector<FieldDesc> fields;
        uint16_t structSize = 0;

        /// fills the dense table from a version table, every id without an entry stays absent
        template <typename FieldId>
        void build(const LayoutSource<FieldId>& source)
        {
            fields.assign(static_cast<size_t>(FieldId::Count), FieldDesc{ kNoField, 0, 0, 0 });
            structSize = source.structSize;
            for (uint16_t i = 0; i < source.entryCount; ++i)
            {
                const auto index = static_cast<size_t>(source.entries[i].id);
                if (index < fields.size())
                    fields[index] = source.entries[i].desc;
            }
        }

        template <typename FieldId>
        [[nodiscard]] const FieldDesc& get(FieldId id) const noexcept
        {
            static constexpr FieldDesc noField{ kNoField, 0, 0, 0 };
            const auto index = static_cast<size_t>(id);
            return index < fields.size() ? fields[index] : noField;
        }

        template <typename FieldId>
        [[nodiscard]] bool has(FieldId id) const noexcept { return get(id).offset != kNoField; }

        /// byte offset of the field or of one of its array elements, kNoField when it is not part of this client version
        template <typename FieldId>
        [[nodiscard]] uint32_t offset(FieldId id, uint32_t arrayIndex = 0) const noexcept
        {
            const FieldDesc& desc = get(id);
            return desc.offset == kNoField ? kNoField : desc.offset + arrayIndex * desc.stride;
        }

        /// element count of an array field, 1 for a plain field, 0 when it is not part of this client version
        template <typename FieldId>
        [[nodiscard]] uint16_t count(FieldId id) const noexcept { return get(id).count; }

        /// index of the field (or array element) in uint32 units, as used by the update masks
        template <typename FieldId>
        [[nodiscard]] uint32_t index(FieldId id, uint32_t arrayIndex = 0) const noexcept
        {
            const uint32_t byteOffset = offset(id, arrayIndex);
            return byteOffset == kNoField ? kNoField : byteOffset / sizeof(uint32_t);
        }

        /// number of uint32 values the struct occupies
        [[nodiscard]] uint16_t valueCount() const noexcept { return static_cast<uint16_t>(structSize / sizeof(uint32_t)); }
    };

    /// the version tables of one client version, each holds the own fields of its struct
    struct ExpansionLayoutSources
    {
        LayoutSource<ObjectField> object;
        LayoutSource<UnitField> unit;
        LayoutSource<PlayerField> player;
        LayoutSource<ItemField> item;
        LayoutSource<ContainerField> container;
        LayoutSource<GameObjectField> gameObject;
        LayoutSource<DynamicObjectField> dynamicObject;
        LayoutSource<CorpseField> corpse;
        LayoutSource<AreaTriggerField> areaTrigger;
    };

    /// the dense struct layouts of one client version, each table holds the own fields of its struct
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

        void build(const ExpansionLayoutSources& sources);
    };

    namespace Tables
    {
        extern const ExpansionLayoutSources classicLayouts;
        extern const ExpansionLayoutSources tbcLayouts;
        extern const ExpansionLayoutSources wotlkLayouts;
        extern const ExpansionLayoutSources cataLayouts;
        extern const ExpansionLayoutSources mopLayouts;
    }

    [[nodiscard]] bool hasLayoutsForExpansion(WoW::Expansion expansion) noexcept;

    /// The layouts of an expansion, an empty layout set (every field absent, size 0) for expansions without tables
    [[nodiscard]] const ExpansionLayouts& layoutsForExpansion(WoW::Expansion expansion) noexcept;

    /// The layouts bound to the server expansion (world.conf), the ones every object in memory uses
    [[nodiscard]] const ExpansionLayouts& layouts() noexcept;

    /// The bound layout table that belongs to a field id enum
    template <typename FieldId> [[nodiscard]] const LayoutTable& layoutFor() noexcept;
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<ObjectField>() noexcept { return layouts().object; }
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<UnitField>() noexcept { return layouts().unit; }
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<PlayerField>() noexcept { return layouts().player; }
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<ItemField>() noexcept { return layouts().item; }
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<ContainerField>() noexcept { return layouts().container; }
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<GameObjectField>() noexcept { return layouts().gameObject; }
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<DynamicObjectField>() noexcept { return layouts().dynamicObject; }
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<CorpseField>() noexcept { return layouts().corpse; }
    template <> [[nodiscard]] inline const LayoutTable& layoutFor<AreaTriggerField>() noexcept { return layouts().areaTrigger; }

    // Value index of the dynamic flags: an own unit or game object field before Mop, the object data field since
    [[nodiscard]] inline uint32_t unitDynamicFlagsIndex() noexcept
    {
        return layouts().unit.has(UnitField::DynamicFlags) ? layouts().unit.index(UnitField::DynamicFlags) : layouts().object.index(ObjectField::DynamicField);
    }

    [[nodiscard]] inline uint32_t gameObjectDynamicFlagsIndex() noexcept
    {
        return layouts().gameObject.has(GameObjectField::Dynamic) ? layouts().gameObject.index(GameObjectField::Dynamic) : layouts().object.index(ObjectField::DynamicField);
    }
}
