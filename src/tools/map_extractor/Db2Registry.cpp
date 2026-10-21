/*
Copyright(c) 2014 - 2026 AscEmu Team < http://www.ascemu.org>
This file is released under the MIT license.See README - MIT for more information.
*/

#include "Db2Registry.hpp"
#include "Db2WodDefinition.hpp"
#include "Db2LegionDefinition.hpp"
#include "Db2BfaDefinition.hpp"
#include "Db2ShadowlandsDefinition.hpp"
#include "Db2DragonflightDefinition.hpp"
#include "Db2TheWarWithinDefinition.hpp"
#include "Db2MidnightDefinition.hpp"
#include "Db2ForeverDefinition.hpp"

namespace MapExtractor::DB2
{
    namespace
    {
        // Central table of known build definitions, each with its own range and associated DB2 files.
        // This allows for easy extension as new expansions are released.
        // The file data ids are stable within an expansion, the ranges start a few patches
        // before the supported build; a layout that moved is caught by the server's loader.
        constexpr BuildDefinition KNOWN_DEFINITIONS[] =
        {
            {
                "Warlords of Draenor",
                20000, // 6.2.0 and later
                21742, // 6.2.4
                WodDefinition::DB2_FILES
            },
            {
                "Legion",
                25961, // Legion 7.3.5 Start
                26972, // Legion 7.3.5 Release
                LegionDefinition::DB2_FILES
            },
            {
                "Battle for Azeroth",
                33000, // 8.3.0 and later
                35662, // 8.3.7
                BfaDefinition::DB2_FILES
            },
            {
                "Shadowlands",
                43000, // 9.2.0 and later
                45745, // 9.2.7
                ShadowlandsDefinition::DB2_FILES
            },
            {
                "Dragonflight",
                53000, // 10.2.0 and later
                55664, // 10.2.7
                DragonflightDefinition::DB2_FILES
            },
            {
                "The War Within",
                63000, // 11.2.0 and later
                65299, // 11.2.7
                TheWarWithinDefinition::DB2_FILES
            },
            {
                "Midnight",
                70300, // 12.1.5 and later
                75000,
                MidnightDefinition::DB2_FILES
            },
            {
                "Forever",
                69876, // 1.60.1 first build 2026-09-16
                70170, // 1.60.1 current build 2026-10-01
                ForeverDefinition::DB2_FILES
            },
        };
    }

    const BuildDefinition* findBuildDefinition(uint32_t buildNumber)
    {
        for (const auto& def : KNOWN_DEFINITIONS)
        {
            if (buildNumber >= def.minBuild && buildNumber <= def.maxBuild)
            {
                return &def;
            }
        }
        return nullptr;
    }
}
