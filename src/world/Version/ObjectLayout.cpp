/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectLayout.hpp"

namespace Version
{
    namespace
    {
        const ExpansionLayouts emptyLayouts
        {
            { nullptr, 0, 0 },
            { nullptr, 0, 0 },
            { nullptr, 0, 0 },
            { nullptr, 0, 0 },
            { nullptr, 0, 0 },
            { nullptr, 0, 0 },
            { nullptr, 0, 0 },
            { nullptr, 0, 0 },
            { nullptr, 0, 0 },
        };
    }

    bool hasLayoutsForExpansion(WoW::Expansion expansion) noexcept
    {
        return &layoutsForExpansion(expansion) != &emptyLayouts;
    }

    const ExpansionLayouts& layoutsForExpansion(WoW::Expansion expansion) noexcept
    {
        switch (expansion)
        {
            case WoW::Expansion::_Classic: return Tables::classicLayouts;
            case WoW::Expansion::_TBC:     return Tables::tbcLayouts;
            case WoW::Expansion::_WotLK:   return Tables::wotlkLayouts;
            case WoW::Expansion::_Cata:    return Tables::cataLayouts;
            case WoW::Expansion::_Mop:     return Tables::mopLayouts;
            default:                       return emptyLayouts;
        }
    }
}
