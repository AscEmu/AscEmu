/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LootItem.hpp"

#include <array>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

struct Loot;
class LootTemplate;
class Player;

typedef std::unordered_map<uint32_t, std::unique_ptr<LootTemplate>> LootTemplateMap;

// a row of a loot set: an item or currency with its chance, or a reference into the loot groups
struct LootEntry
{
    LootStoreItem item;
    uint32_t groupId = 0;       // the referenced loot group, 0 for an item
    uint32_t rolls = 1;         // how often a reference rolls its group
    uint8_t referencedGroup = 0;    // the exclusive group of the referenced set a reference rolls, 0 for the whole set
};

// the rows of one set: independent rolls and exclusive groups that drop one of their rows per roll
class LootRowSet
{
public:
    void addIndependent(LootEntry const& entry);
    void addExclusive(uint8_t groupIndex, LootEntry const& entry);
    bool empty() const { return m_independent.empty() && m_exclusive.empty(); }
    size_t size() const;

    // rolls the set (or only one of its exclusive groups) into the loot, a reference brings the rows of its group
    void generate(Loot& loot, uint8_t depth, uint8_t onlyGroup = 0) const;
    bool hasQuestDrop(Player const* forPlayer, uint8_t depth, uint8_t onlyGroup = 0) const;

    std::vector<LootEntry> const& getIndependent() const { return m_independent; }
    std::map<uint8_t, std::vector<LootEntry>> const& getExclusive() const { return m_exclusive; }

private:
    void generateEntry(Loot& loot, LootEntry const& entry, uint8_t depth) const;
    static LootEntry const* rollExclusive(std::vector<LootEntry> const& entries);

    std::vector<LootEntry> m_independent;
    std::map<uint8_t, std::vector<LootEntry>> m_exclusive;
};

class LootTemplate
{
public:
    // the set a chance index (normal 10, normal 25, heroic 10, heroic 25) rolls, sets may be shared
    void setRowSet(uint8_t index, std::shared_ptr<LootRowSet const> const& set);
    LootRowSet const* getRowSet(uint8_t index) const;
    // the rows of the template, a shared set counted once
    size_t size() const;

    // Generats the Loot for our current LootTempelate
    void generateLoot(Loot& loot, uint8_t lootDifficulty) const;

    // Returns True when in our LootTempelate is atleast 1 QuestItem
    // Can also check for given player
    bool hasQuestDrop(Player const* forPlayer = nullptr) const;

private:
    std::array<std::shared_ptr<LootRowSet const>, 4> m_sets;
};
