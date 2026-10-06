/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"

#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

namespace WDB::Structures { struct ItemBonusEntry; }

class SERVER_DECL ItemBonusMgr
{
private:
    ItemBonusMgr() = default;
    ~ItemBonusMgr() = default;

public:
    static ItemBonusMgr& getInstance();
    void initialize();
    void finalize();

    ItemBonusMgr(ItemBonusMgr&&) = delete;
    ItemBonusMgr(ItemBonusMgr const&) = delete;
    ItemBonusMgr& operator=(ItemBonusMgr&&) = delete;
    ItemBonusMgr& operator=(ItemBonusMgr const&) = delete;

    [[nodiscard]] std::span<WDB::Structures::ItemBonusEntry const* const> getItemBonuses(uint32_t bonusListId) const;
    [[nodiscard]] bool hasItemBonusList(uint32_t bonusListId) const;
    [[nodiscard]] std::size_t getItemBonusListCount() const;

private:
    std::unordered_map<uint32_t, std::vector<WDB::Structures::ItemBonusEntry const*>> m_itemBonusLists;
};

#define sItemBonusMgr ItemBonusMgr::getInstance()
