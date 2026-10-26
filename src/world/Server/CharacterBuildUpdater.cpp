/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CharacterBuildUpdater.hpp"

#include "Logging/Logger.hpp"
#include "Management/Achievements/AchievementConverter.hpp"
#include "Server/ClientProtocol.hpp"
#include "Server/DatabaseDefinition.hpp"

#include <iterator>
#include <sstream>

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
    auto result = CharacterDatabase.query("SELECT guid, acct, last_login_build, level FROM characters WHERE last_login_build < %u", serverBuild);
    if (result == nullptr)
        return;

    std::vector<OldCharacter> characters;
    do
    {
        Field* fields = result->fetch();
        characters.push_back({ fields[0].asUint32(), fields[1].asUint32(), fields[2].asUint32(), fields[3].asUint32() });
    }
    while (result->nextRow());

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

void CharacterBuildUpdater::squishLevels(std::vector<OldCharacter> const& characters)
{
    for (auto const& character : characters)
    {
        uint32_t const level = squishedLevel(character.level);
        CharacterDatabase.waitExecute("UPDATE characters SET level = %u, xp = 0 WHERE guid = %u", level, character.guid);
        sLogger.info("CharacterBuildUpdater : character {} was last played with build {}, level {} became {}", character.guid, character.lastLoginBuild, character.level, level);
    }
}
