/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Loot.hpp"
#include "LootItem.hpp"
#include "LootMgr.hpp"
#include "LootTemplate.hpp"
#include "Management/ItemProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

#include <algorithm>

namespace
{
    constexpr uint8_t MaxReferenceDepth = 5;

    // equippable items are limited to 2 drops, the others to 3
    bool canAddItem(Loot const& loot, ItemProperties const* proto)
    {
        if (proto == nullptr)
            return true;

        uint8_t equipable = 0;
        uint8_t notEquipable = 0;
        for (const auto& item : loot.items)
        {
            if (item.itemproto == nullptr)
                continue;

            if (item.itemproto->InventoryType == INVTYPE_NON_EQUIP)
                ++notEquipable;
            else
                ++equipable;
        }

        if (proto->InventoryType == INVTYPE_NON_EQUIP)
            return notEquipable < MAX_NR_LOOT_NONEQUIPABLE;

        return equipable < MAX_NR_LOOT_EQUIPABLE;
    }
}

void LootRowSet::addIndependent(LootEntry const& entry)
{
    m_independent.push_back(entry);
}

void LootRowSet::addExclusive(uint8_t groupIndex, LootEntry const& entry)
{
    m_exclusive[groupIndex].push_back(entry);
}

size_t LootRowSet::size() const
{
    size_t count = m_independent.size();
    for (const auto& [groupIndex, entries] : m_exclusive)
        count += entries.size();

    return count;
}

LootEntry const* LootRowSet::rollExclusive(std::vector<LootEntry> const& entries)
{
    // the rows with a chance take their share of one roll, the rest falls to the rows without one
    float roll = Util::getRandomFloat(100.0f);
    std::vector<LootEntry const*> equal;
    for (const auto& entry : entries)
    {
        if (entry.item.chance <= 0.0f)
        {
            equal.push_back(&entry);
            continue;
        }

        if (entry.item.chance >= 100.0f)
            return &entry;

        roll -= entry.item.chance;
        if (roll < 0.0f)
            return &entry;
    }

    if (equal.empty())
        return nullptr;

    return equal[Util::getRandomUInt(0, static_cast<uint32_t>(equal.size() - 1))];
}

void LootRowSet::generateEntry(Loot& loot, LootEntry const& entry, uint8_t depth) const
{
    if (entry.groupId != 0)
    {
        if (depth >= MaxReferenceDepth)
            return;

        const auto group = sLootMgr.getLootGroup(entry.groupId);
        if (group == nullptr)
            return;

        for (uint32_t i = 0; i < std::max<uint32_t>(1, entry.rolls); ++i)
            group->generate(loot, depth + 1, entry.referencedGroup);

        return;
    }

    if (entry.item.isCurrency)
    {
        loot.addLootCurrency(entry.item);
        return;
    }

    if (!canAddItem(loot, entry.item.itemproto))
        return;

    loot.addLootItem(entry.item);
}

void LootRowSet::generate(Loot& loot, uint8_t depth, uint8_t onlyGroup/* = 0*/) const
{
    if (onlyGroup != 0)
    {
        const auto group = m_exclusive.find(onlyGroup);
        if (group != m_exclusive.cend())
            if (const auto entry = rollExclusive(group->second))
                generateEntry(loot, *entry, depth);

        return;
    }

    // a random order, so the drop limits do not always favour the first rows
    std::vector<LootEntry const*> order;
    order.reserve(m_independent.size());
    for (const auto& entry : m_independent)
        order.push_back(&entry);
    Util::randomShuffleVector(order);

    for (const auto entry : order)
        if (entry->item.roll())
            generateEntry(loot, *entry, depth);

    for (const auto& [groupIndex, entries] : m_exclusive)
        if (const auto entry = rollExclusive(entries))
            generateEntry(loot, *entry, depth);
}

bool LootRowSet::hasQuestDrop(Player const* forPlayer, uint8_t depth, uint8_t onlyGroup/* = 0*/) const
{
    const auto isQuestDrop = [&](LootEntry const& entry)
    {
        if (entry.groupId != 0)
        {
            if (depth >= MaxReferenceDepth)
                return false;

            const auto group = sLootMgr.getLootGroup(entry.groupId);
            return group != nullptr && group->hasQuestDrop(forPlayer, depth + 1, entry.referencedGroup);
        }

        if (entry.item.isCurrency)
            return false;

        return forPlayer != nullptr ? forPlayer->hasQuestForItem(entry.item.itemId) : entry.item.needs_quest;
    };

    if (onlyGroup == 0)
        for (const auto& entry : m_independent)
            if (isQuestDrop(entry))
                return true;

    for (const auto& [groupIndex, entries] : m_exclusive)
        if (onlyGroup == 0 || onlyGroup == groupIndex)
            for (const auto& entry : entries)
                if (isQuestDrop(entry))
                    return true;

    return false;
}

void LootTemplate::setRowSet(uint8_t index, std::shared_ptr<LootRowSet const> const& set)
{
    if (index < m_sets.size())
        m_sets[index] = set;
}

LootRowSet const* LootTemplate::getRowSet(uint8_t index) const
{
    return index < m_sets.size() ? m_sets[index].get() : nullptr;
}

size_t LootTemplate::size() const
{
    size_t count = 0;
    for (size_t i = 0; i < m_sets.size(); ++i)
    {
        if (m_sets[i] == nullptr)
            continue;

        bool counted = false;
        for (size_t j = 0; j < i; ++j)
            if (m_sets[j] == m_sets[i])
                counted = true;

        if (!counted)
            count += m_sets[i]->size();
    }

    return count;
}

void LootTemplate::generateLoot(Loot& loot, uint8_t lootDifficulty) const
{
    // the difficulties beyond the four chance indexes roll as normal
    const auto set = getRowSet(lootDifficulty < m_sets.size() ? lootDifficulty : 0);
    if (set != nullptr)
        set->generate(loot, 0);
}

bool LootTemplate::hasQuestDrop(Player const* forPlayer/* = nullptr*/) const
{
    for (size_t i = 0; i < m_sets.size(); ++i)
    {
        if (m_sets[i] == nullptr)
            continue;

        bool checked = false;
        for (size_t j = 0; j < i; ++j)
            if (m_sets[j] == m_sets[i])
                checked = true;

        if (!checked && m_sets[i]->hasQuestDrop(forPlayer, 0))
            return true;
    }

    return false;
}
