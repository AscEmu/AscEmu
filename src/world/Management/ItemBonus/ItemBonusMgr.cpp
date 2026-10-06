/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ItemBonusMgr.hpp"

#include "Logging/Logger.hpp"
#include "Storage/WDB/WDBStores.hpp"

#include <algorithm>

ItemBonusMgr& ItemBonusMgr::getInstance()
{
    static ItemBonusMgr instance;
    return instance;
}

void ItemBonusMgr::initialize()
{
    m_itemBonusLists.clear();

#if defined(AE_FOREVER)
    for (auto const& [id, bonus] : sItemBonusStore)
    {
        (void)id;
        m_itemBonusLists[bonus.ParentItemBonusListID].push_back(&bonus);
    }

    for (auto& [bonusListId, bonuses] : m_itemBonusLists)
    {
        (void)bonusListId;
        std::ranges::sort(bonuses, [](auto const* left, auto const* right)
        {
            if (left->OrderIndex != right->OrderIndex)
                return left->OrderIndex < right->OrderIndex;
            return left->ID < right->ID;
        });
    }

    sLogger.info("ItemBonusMgr : indexed {} ItemBonus rows into {} bonus lists.", sItemBonusStore.getNumRows(), m_itemBonusLists.size());
#endif
}

void ItemBonusMgr::finalize()
{
    m_itemBonusLists.clear();
}

std::span<WDB::Structures::ItemBonusEntry const* const> ItemBonusMgr::getItemBonuses(uint32_t bonusListId) const
{
    const auto itr = m_itemBonusLists.find(bonusListId);
    if (itr == m_itemBonusLists.end())
        return {};

    return {itr->second.data(), itr->second.size()};
}

bool ItemBonusMgr::hasItemBonusList(uint32_t bonusListId) const
{
    return m_itemBonusLists.contains(bonusListId);
}

std::size_t ItemBonusMgr::getItemBonusListCount() const
{
    return m_itemBonusLists.size();
}
