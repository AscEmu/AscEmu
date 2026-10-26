/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CharacterBuildUpdater.hpp"

#include "Logging/Logger.hpp"
#include "Management/Achievements/AchievementConverter.hpp"
#include "Map/Management/TerrainMgr.hpp"
#include "Map/Visibility/VisibilityTypes.hpp"
#include "Movement/MovementDefines.h"
#include "Server/ClientProtocol.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "VMapManager2.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>

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
    auto result = CharacterDatabase.query("SELECT guid, acct, last_login_build, level, mapId, positionX, positionY, positionZ, transporter FROM characters WHERE last_login_build < %u", serverBuild);
    if (result == nullptr)
        return;

    std::vector<OldCharacter> characters;
    do
    {
        Field* fields = result->fetch();
        characters.push_back({ fields[0].asUint32(), fields[1].asUint32(), fields[2].asUint32(), fields[3].asUint32(),
            fields[4].asUint32(), fields[5].asFloat(), fields[6].asFloat(), fields[7].asFloat(), fields[8].asUint32() != 0 });
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
