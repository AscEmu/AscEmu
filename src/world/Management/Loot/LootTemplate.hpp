/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

struct Loot;
struct LootStoreItem;
class LootTemplate;
class Player;

typedef std::vector<LootStoreItem> LootStoreItemList;
typedef std::unordered_map<uint32_t, std::unique_ptr<LootTemplate>> LootTemplateMap;

class LootTemplate
{
public:
    LootTemplate();
    ~LootTemplate();

    // Adds an entry to our LootTempelate
    void addEntry(LootStoreItem& item);

    // Generats the Loot for our current LootTempelate
    void generateLoot(Loot& loot, uint8_t lootDifficulty) const;

    // Returns True when in our LootTempelate is atleast 1 QuestItem
    // Can also check for given player
    bool hasQuestDrop(LootTemplateMap const& store, Player const* forPlayer = nullptr) const;

private:
    // Loot entrys on our current LootTempelate
    LootStoreItemList Entries;
};
