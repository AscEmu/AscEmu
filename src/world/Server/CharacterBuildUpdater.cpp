/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CharacterBuildUpdater.hpp"

#include "Logging/Logger.hpp"
#include "Macros/PlayerMacros.hpp"
#include "Management/Achievements/AchievementConverter.hpp"
#include "Management/ItemProperties.hpp"
#include "Management/MailMgr.h"
#include "Map/Management/TerrainMgr.hpp"
#include "Map/Visibility/VisibilityTypes.hpp"
#include "Movement/MovementDefines.h"
#include "Objects/ItemDefines.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Server/ClientProtocol.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "VMapManager2.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace
{
    // the highest old level of every new level of the squish of 9.0, old levels above 120 become the new maximum
    constexpr uint8_t LevelSquishOldMax[] =
    {
        1, 4, 7, 9, 11, 13, 15, 17, 19, 22, 25, 28, 31, 34, 36, 38, 40, 42, 44, 47, 50, 53, 56, 59, 63,
        67, 71, 75, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 120
    };

    uint32_t squishedLevel(uint32_t level)
    {
        for (size_t newLevel = 0; newLevel < std::size(LevelSquishOldMax); ++newLevel)
            if (level <= LevelSquishOldMax[newLevel])
                return static_cast<uint32_t>(newLevel + 1);

        return static_cast<uint32_t>(std::size(LevelSquishOldMax));
    }

    // the feet of a placed character above the found floor
    constexpr float StandOffset = 0.1f;
    // smaller differences keep the stored height
    constexpr float HeightTolerance = 0.5f;
    // a floor is searched from this far above the stored height
    constexpr float FloorProbeHeight = 2.0f;
    constexpr float FloorSearchDistance = 100.0f;
    // deeper liquid lifts the character to its surface
    constexpr float SwimDepth = 2.0f;

    // the ranges behind the equipment as the server of a build stores them
    struct InventoryRanges
    {
        uint32_t fromBuild;
        int32_t professionCount;    // the slots after the equipment from 10.x on
        int32_t bagStart;
        int32_t reagentBagSlot;     // -1 without
        int32_t itemStart;
        int32_t itemCount;
        int32_t bankItemStart;
        int32_t bankItemCount;
        int32_t bankBagStart;
        int32_t bankBagCount;
        int32_t buybackStart;
    };

    constexpr InventoryRanges InventoryRangesOfBuilds[] =
    {
        { 0,                                 0, 19, -1, 23, 16, 39, 28, 67, 7, 74 },
        { WoW::Build::SHADOWLANDS_BUILD,     0, 19, -1, 23, 20, 51, 28, 79, 7, 86 },
        { WoW::Build::DRAGONFLIGHT_BUILD,   11, 30, 34, 35, 20, 63, 28, 91, 7, 98 },
        { WoW::Build::MIDNIGHT_BUILD,       11, 30, 34, 35, 20, 63,  0, 63, 6, 69 }
    };

    constexpr int32_t EquipmentSlotCount = 19;
    constexpr int32_t BagSlotCount = 4;
    constexpr int32_t BuybackSlotCount = 12;
    constexpr int32_t NoPlace = -1;
    constexpr int32_t SoldItem = -2;

    InventoryRanges const& inventoryRangesOf(uint32_t build)
    {
        InventoryRanges const* result = &InventoryRangesOfBuilds[0];
        for (auto const& layout : InventoryRangesOfBuilds)
            if (build >= layout.fromBuild)
                result = &layout;
        return *result;
    }

    // the slot of the same place in the new ranges, NoPlace when the new build has none, SoldItem for the buyback
    int32_t movedSlot(InventoryRanges const& from, InventoryRanges const& to, int32_t slot)
    {
        auto const within = [slot](int32_t start, int32_t count) { return slot >= start && slot < start + count; };
        auto const sameIndex = [](int32_t index, int32_t start, int32_t count) { return index < count ? start + index : NoPlace; };

        if (slot < EquipmentSlotCount)
            return slot;
        if (within(EquipmentSlotCount, from.professionCount))
            return sameIndex(slot - EquipmentSlotCount, EquipmentSlotCount, to.professionCount);
        if (within(from.bagStart, BagSlotCount))
            return to.bagStart + slot - from.bagStart;
        if (from.reagentBagSlot >= 0 && slot == from.reagentBagSlot)
            return to.reagentBagSlot >= 0 ? to.reagentBagSlot : NoPlace;
        if (within(from.itemStart, from.itemCount))
            return sameIndex(slot - from.itemStart, to.itemStart, to.itemCount);
        if (within(from.bankItemStart, from.bankItemCount))
            return sameIndex(slot - from.bankItemStart, to.bankItemStart, to.bankItemCount);
        if (within(from.bankBagStart, from.bankBagCount))
            return sameIndex(slot - from.bankBagStart, to.bankBagStart, to.bankBagCount);
        if (within(from.buybackStart, BuybackSlotCount))
            return SoldItem;

        // the keyring and the currency tokens of 3.x and older
        return NoPlace;
    }

    constexpr uint32_t ActionTypeSpell = 0;
    // a start spell without its own free button takes the first free one of the first bar
    constexpr size_t MainBarButtons = 12;

    struct ActionSlot
    {
        uint32_t action = 0;
        uint32_t type = 0;
        uint32_t misc = 0;
    };

    std::vector<ActionSlot> parseActionBar(char const* text)
    {
        std::vector<ActionSlot> buttons;
        std::istringstream stream(text != nullptr ? text : "");
        std::string action;
        std::string type;
        std::string misc;
        while (std::getline(stream, action, ',') && std::getline(stream, type, ',') && std::getline(stream, misc, ','))
        {
            buttons.push_back({ static_cast<uint32_t>(std::strtoul(action.c_str(), nullptr, 10)), static_cast<uint32_t>(std::strtoul(type.c_str(), nullptr, 10)),
                static_cast<uint32_t>(std::strtoul(misc.c_str(), nullptr, 10)) });
        }
        return buttons;
    }

    std::string joinActionBar(std::vector<ActionSlot> const& buttons)
    {
        std::ostringstream text;
        for (auto const& button : buttons)
            text << button.action << "," << button.type << "," << button.misc << ",";
        return text.str();
    }

    // the characters of a step: last played before the build the step brings, the server runs that build or a later one
    std::vector<CharacterBuildUpdater::OldCharacter> crossing(std::vector<CharacterBuildUpdater::OldCharacter> const& characters, uint32_t stepBuild, uint32_t serverBuild)
    {
        std::vector<CharacterBuildUpdater::OldCharacter> result;
        if (serverBuild < stepBuild)
            return result;

        for (auto const& character : characters)
            if (character.lastLoginBuild < stepBuild)
                result.push_back(character);
        return result;
    }
}

CharacterBuildUpdater& CharacterBuildUpdater::getInstance()
{
    static CharacterBuildUpdater mInstance;
    return mInstance;
}

void CharacterBuildUpdater::run()
{
    uint32_t const serverBuild = WoW::getConfigBuild();
    auto result = CharacterDatabase.query("SELECT guid, acct, last_login_build, level, mapId, positionX, positionY, positionZ, transporter, race, class FROM characters WHERE last_login_build < %u", serverBuild);
    if (result == nullptr)
        return;

    std::vector<OldCharacter> characters;
    do
    {
        Field* fields = result->fetch();
        characters.push_back({ fields[0].asUint32(), fields[1].asUint32(), fields[2].asUint32(), fields[3].asUint32(),
            fields[4].asUint32(), fields[5].asFloat(), fields[6].asFloat(), fields[7].asFloat(), fields[8].asUint32() != 0, fields[9].asUint8(), fields[10].asUint8() });
    }
    while (result->nextRow());

    placeOnCurrentTerrain(crossing(characters, WoW::Build::CATA_BUILD, serverBuild), serverBuild);

    // the criteria trees of 5.4: the progress of the criteria rows moves to the trees, the account wide parts to the logonserver
    auto const achievementCharacters = crossing(characters, WoW::Build::MOP_BUILD, serverBuild);
    if (!achievementCharacters.empty())
    {
        std::vector<std::pair<uint32_t, uint32_t>> owners;
        for (auto const& character : achievementCharacters)
            owners.emplace_back(character.guid, character.accountId);
        Achievements::convertCharacters(owners);
    }

    squishLevels(crossing(characters, WoW::Build::SHADOWLANDS_BUILD, serverBuild));

    moveInventory(characters, serverBuild);
    refreshActionBars(characters);

    // every step is done, the characters count as last played with this build
    std::ostringstream query;
    for (size_t i = 0; i < characters.size(); ++i)
    {
        if (i % 500 == 0)
        {
            if (i != 0)
                CharacterDatabase.waitExecuteNA((query.str() + ")").c_str());
            query.str("");
            query << "UPDATE characters SET last_login_build = " << serverBuild << " WHERE guid IN (";
        }
        query << (i % 500 == 0 ? "" : ", ") << characters[i].guid;
    }
    CharacterDatabase.waitExecuteNA((query.str() + ")").c_str());

    sLogger.info("CharacterBuildUpdater : {} characters brought to build {}", characters.size(), serverBuild);
}

void CharacterBuildUpdater::placeOnCurrentTerrain(std::vector<OldCharacter> const& characters, uint32_t serverBuild)
{
    if (characters.empty())
        return;

    using visibility::Terrain;
    std::string const dataDir = worldConfig.server.dataDir;
    std::string const vmapPath = dataDir + "vmaps";

    // own terrain and model data, the map threads keep theirs untouched
    std::unordered_map<uint32_t, std::unique_ptr<TerrainHolder>> terrains;
    std::unique_ptr<VMAP::VMapManager2> models;
    if (worldConfig.terrainCollision.isCollisionEnabled)
        models = std::make_unique<VMAP::VMapManager2>();
    std::set<std::tuple<uint32_t, int32_t, int32_t>> loadedTiles;

    uint32_t placed = 0;
    for (auto const& character : characters)
    {
        // a passenger moves with its transport
        if (character.onTransport)
            continue;

        int32_t const tileX = static_cast<int32_t>(Terrain::MapCenter - character.x / Terrain::TileSize);
        int32_t const tileY = static_cast<int32_t>(Terrain::MapCenter - character.y / Terrain::TileSize);
        if (tileX < 0 || tileY < 0 || tileX >= Terrain::TilesCount || tileY >= Terrain::TilesCount)
            continue;

        auto& terrain = terrains[character.mapId];
        if (!terrain)
            terrain = std::make_unique<TerrainHolder>(character.mapId);

        if (loadedTiles.emplace(character.mapId, tileX, tileY).second)
        {
            char tileFile[32];
            std::snprintf(tileFile, sizeof(tileFile), "maps/%04u_%02i_%02i.map", character.mapId, tileX, tileY);
            if (std::filesystem::exists(dataDir + tileFile))
                terrain->loadTile(tileX, tileY);
            if (models)
                models->loadMap(vmapPath.c_str(), character.mapId, tileX, tileY);
        }

        float terrainZ = INVALID_HEIGHT;
        float liquidZ = INVALID_HEIGHT;
        if (TerrainTile* tile = terrain->getTile(tileX, tileY))
        {
            terrainZ = tile->m_map.getHeight(character.x, character.y);
            liquidZ = tile->m_map.getLiquidLevel(character.x, character.y);
        }

        float const probeZ = character.z + FloorProbeHeight;
        float const modelZ = models ? models->getHeight(character.mapId, character.x, character.y, probeZ, FloorSearchDistance) : VMAP_INVALID_HEIGHT_VALUE;

        // the highest floor under the character, a model floor keeps a character under the terrain (caves, cities below ground)
        bool const terrainBelow = terrainZ > INVALID_HEIGHT && terrainZ <= probeZ;
        bool const modelBelow = modelZ > INVALID_HEIGHT;
        float floorZ = INVALID_HEIGHT;
        bool onTerrain = true;
        if (terrainBelow && (!modelBelow || terrainZ > modelZ))
            floorZ = terrainZ;
        else if (modelBelow)
        {
            floorZ = modelZ;
            onTerrain = false;
        }
        else if (terrainZ > INVALID_HEIGHT)
            floorZ = terrainZ; // the terrain covers the character now
        else
            continue;

        if (onTerrain && liquidZ > floorZ + SwimDepth)
            floorZ = liquidZ;

        float const newZ = floorZ + StandOffset;
        if (std::fabs(newZ - character.z) <= HeightTolerance)
            continue;

        CharacterDatabase.waitExecute("UPDATE characters SET positionZ = %f WHERE guid = %u", newZ, character.guid);
        sLogger.info("CharacterBuildUpdater : character {} on map {} was last played with build {}, height {} became {}", character.guid, character.mapId, character.lastLoginBuild, character.z, newZ);
        ++placed;
    }

    sLogger.info("CharacterBuildUpdater : {} of {} characters placed on the terrain of build {}", placed, characters.size(), serverBuild);
}

void CharacterBuildUpdater::squishLevels(std::vector<OldCharacter> const& characters)
{
    for (auto const& character : characters)
    {
        uint32_t const level = squishedLevel(character.level);
        CharacterDatabase.waitExecute("UPDATE characters SET level = %u, xp = 0 WHERE guid = %u", level, character.guid);
        sLogger.info("CharacterBuildUpdater : character {} was last played with build {}, level {} became {}", character.guid, character.lastLoginBuild, character.level, level);
    }
}

void CharacterBuildUpdater::moveInventory(std::vector<OldCharacter> const& characters, uint32_t serverBuild)
{
    InventoryRanges const& to = inventoryRangesOf(serverBuild);
    for (auto const& character : characters)
    {
        InventoryRanges const& from = inventoryRangesOf(character.lastLoginBuild);
        if (&from == &to)
            continue;

        auto result = CharacterDatabase.query("SELECT guid, entry, containerslot, slot FROM playeritems WHERE ownerguid = %u", character.guid);
        if (result == nullptr)
            continue;

        struct StoredItem
        {
            uint32_t guid;
            uint32_t entry;
            int32_t container;
            int32_t slot;
            int32_t newContainer;
            int32_t newSlot;
        };

        std::vector<StoredItem> items;
        do
        {
            Field* fields = result->fetch();
            int32_t const container = fields[2].asInt32();
            int32_t const slot = fields[3].asInt32();
            items.push_back({ fields[0].asUint32(), fields[1].asUint32(), container, slot, container, slot });
        }
        while (result->nextRow());

        std::set<std::pair<int32_t, int32_t>> taken;
        std::vector<StoredItem*> homeless;
        std::vector<uint32_t> sold;

        // an item in a bag follows the slot of its bag
        for (auto& item : items)
        {
            int32_t const target = movedSlot(from, to, item.container < 0 ? item.slot : item.container);
            if (target == SoldItem)
            {
                sold.push_back(item.guid);
                continue;
            }

            if (target == NoPlace)
            {
                homeless.push_back(&item);
                continue;
            }

            if (item.container < 0)
                item.newSlot = target;
            else
                item.newContainer = target;
            taken.emplace(item.newContainer, item.newSlot);
        }

        // the free backpack slots, then the free slots of the bags that take every item
        std::vector<std::pair<int32_t, int32_t>> freePlaces;
        for (int32_t slot = to.itemStart; slot < to.itemStart + to.itemCount; ++slot)
            if (!taken.contains({ -1, slot }))
                freePlaces.emplace_back(-1, slot);

        for (int32_t bagSlot = to.bagStart; bagSlot < to.bagStart + BagSlotCount; ++bagSlot)
        {
            auto const bag = std::find_if(items.begin(), items.end(), [bagSlot](StoredItem const& item)
            {
                return item.newContainer < 0 && item.newSlot == bagSlot && item.container < 0;
            });
            if (bag == items.end() || std::find(homeless.begin(), homeless.end(), &*bag) != homeless.end())
                continue;

            ItemProperties const* properties = sMySQLStore.getItemProperties(bag->entry);
            if (properties == nullptr || properties->InventoryType != INVTYPE_BAG || properties->BagFamily != 0)
                continue;

            for (int32_t slot = 0; slot < static_cast<int32_t>(properties->ContainerSlots); ++slot)
                if (!taken.contains({ bagSlot, slot }))
                    freePlaces.emplace_back(bagSlot, slot);
        }

        std::vector<uint64_t> mailed;
        size_t nextPlace = 0;
        for (StoredItem* item : homeless)
        {
            if (nextPlace < freePlaces.size())
            {
                item->newContainer = freePlaces[nextPlace].first;
                item->newSlot = freePlaces[nextPlace].second;
                ++nextPlace;
            }
            else
            {
                item->newContainer = item->container;
                item->newSlot = item->slot;
                mailed.push_back(item->guid);
            }
        }

        uint32_t moved = 0;
        for (auto const& item : items)
        {
            if (item.newContainer == item.container && item.newSlot == item.slot)
                continue;

            CharacterDatabase.waitExecute("UPDATE playeritems SET containerslot = %d, slot = %d WHERE guid = %u", item.newContainer, item.newSlot, item.guid);
            ++moved;
        }

        for (uint32_t const guid : sold)
            CharacterDatabase.waitExecute("DELETE FROM playeritems WHERE guid = %u", guid);

        // a mail item belongs to nobody until it is taken
        constexpr size_t MailAttachments = MAIL_MAX_ITEM_SLOT;
        for (size_t first = 0; first < mailed.size(); first += MailAttachments)
        {
            std::vector<uint64_t> attachments(mailed.begin() + first, mailed.begin() + std::min(mailed.size(), first + MailAttachments));
            for (uint64_t const guid : attachments)
                CharacterDatabase.waitExecute("UPDATE playeritems SET ownerguid = 0, containerslot = -1, slot = 0 WHERE guid = %u", static_cast<uint32_t>(guid));

            sMailSystem.SendAutomatedMessage(MAIL_TYPE_NORMAL, character.guid, character.guid, "Items of your inventory",
                "These items had no place in the inventory of the new game version.", 0, 0, attachments, MAIL_STATIONERY_GM);
        }

        sLogger.info("CharacterBuildUpdater : character {} was last played with build {}, {} items moved, {} sent by mail, {} sold items removed",
            character.guid, character.lastLoginBuild, moved, mailed.size(), sold.size());
    }
}

void CharacterBuildUpdater::refreshActionBars(std::vector<OldCharacter> const& characters)
{
    for (auto const& character : characters)
    {
        auto result = CharacterDatabase.query("SELECT actions1, actions2 FROM characters WHERE guid = %u", character.guid);
        if (result == nullptr)
            continue;

        Field* fields = result->fetch();
        PlayerCreateInfo const* createInfo = sMySQLStore.getPlayerCreateInfo(character.race, character.playerClass);

        uint32_t removed = 0;
        uint32_t added = 0;
        std::string bars[2];
        for (uint8_t spec = 0; spec < 2; ++spec)
        {
            auto buttons = parseActionBar(fields[spec].asCString());
            bool const used = std::any_of(buttons.begin(), buttons.end(), [](ActionSlot const& button) { return button.action != 0; });

            for (auto& button : buttons)
            {
                if (button.type != ActionTypeSpell || button.action == 0 || sSpellMgr.getSpellInfo(button.action) != nullptr)
                    continue;

                button = {};
                ++removed;
            }

            // a second bar nobody used stays empty
            if (createInfo != nullptr && (spec == 0 || used))
            {
                if (buttons.size() < static_cast<size_t>(PLAYER_ACTION_BUTTON_COUNT))
                    buttons.resize(PLAYER_ACTION_BUTTON_COUNT);

                for (auto const& start : createInfo->actionbars)
                {
                    if (start.type != ActionTypeSpell || start.action == 0 || sSpellMgr.getSpellInfo(start.action) == nullptr)
                        continue;

                    bool const onBar = std::any_of(buttons.begin(), buttons.end(), [&start](ActionSlot const& button)
                    {
                        return button.type == ActionTypeSpell && button.action == start.action;
                    });
                    if (onBar)
                        continue;

                    size_t target = start.button;
                    if (target >= buttons.size() || buttons[target].action != 0)
                    {
                        target = buttons.size();
                        for (size_t index = 0; index < MainBarButtons && index < buttons.size(); ++index)
                        {
                            if (buttons[index].action == 0)
                            {
                                target = index;
                                break;
                            }
                        }
                    }

                    if (target >= buttons.size())
                        continue;

                    buttons[target] = { start.action, start.type, start.misc };
                    ++added;
                }
            }

            bars[spec] = joinActionBar(buttons);
        }

        if (removed == 0 && added == 0)
            continue;

        CharacterDatabase.waitExecute("UPDATE characters SET actions1 = '%s', actions2 = '%s' WHERE guid = %u", bars[0].c_str(), bars[1].c_str(), character.guid);
        sLogger.info("CharacterBuildUpdater : character {} was last played with build {}, {} unknown spells left the action bars, {} start spells joined them",
            character.guid, character.lastLoginBuild, removed, added);
    }
}
