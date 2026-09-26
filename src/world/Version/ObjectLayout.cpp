/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectLayout.hpp"

#include <array>

namespace Version
{
    void ExpansionLayouts::build(const ExpansionLayoutSources& sources)
    {
        object.build(sources.object);
        unit.build(sources.unit);
        player.build(sources.player);
        item.build(sources.item);
        container.build(sources.container);
        gameObject.build(sources.gameObject);
        dynamicObject.build(sources.dynamicObject);
        corpse.build(sources.corpse);
        areaTrigger.build(sources.areaTrigger);
    }

    namespace
    {
        // dense tables of the supported expansions, built once from the version tables
        struct BuiltLayouts
        {
            std::array<ExpansionLayouts, 7> perExpansion{};
            ExpansionLayouts empty{};

            BuiltLayouts()
            {
                perExpansion[0].build(Tables::classicLayouts);
                perExpansion[1].build(Tables::tbcLayouts);
                perExpansion[2].build(Tables::wotlkLayouts);
                perExpansion[3].build(Tables::cataLayouts);
                perExpansion[4].build(Tables::mopLayouts);
                perExpansion[5].build(Tables::wodLayouts);
                perExpansion[6].build(Tables::legionLayouts);
            }
        };

        const BuiltLayouts& built()
        {
            static const BuiltLayouts instance;
            return instance;
        }
    }

    bool hasLayoutsForExpansion(WoW::Expansion expansion) noexcept
    {
        const auto index = WoW::getOpcodeTableIndex(expansion);
        return index >= 0 && static_cast<size_t>(index) < built().perExpansion.size();
    }

    const ExpansionLayouts& layoutsForExpansion(WoW::Expansion expansion) noexcept
    {
        const BuiltLayouts& tables = built();
        const auto index = WoW::getOpcodeTableIndex(expansion);
        if (index < 0 || static_cast<size_t>(index) >= tables.perExpansion.size())
            return tables.empty;

        return tables.perExpansion[static_cast<size_t>(index)];
    }
}
