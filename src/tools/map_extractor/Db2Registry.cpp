/*
Copyright(c) 2014 - 2026 AscEmu Team < http://www.ascemu.org>
This file is released under the MIT license.See README - MIT for more information.
*/

#include "Db2Registry.hpp"
#include "Db2LegionDefinition.hpp"
#include "Db2ForeverDefinition.hpp"
// Add future expansion definitions here as needed Db2WodDefinition.hpp, Db2BfaDefinition.hpp, etc.

namespace MapExtractor::DB2
{
    namespace
    {
        // Central table of known build definitions, each with its own range and associated DB2 files.
        // This allows for easy extension as new expansions are released.
        constexpr BuildDefinition KNOWN_DEFINITIONS[] =
        {
            {
                "Legion",
                25961, // Legion 7.3.5 Start
                26972, // Legion 7.3.5 Release
                LegionDefinition::DB2_FILES
            },
            {
                "Forever",
                69876, // 1.60.1 first build 2026-09-16
                70170, // 1.60.1 current build 2026-10-01
                ForeverDefinition::DB2_FILES
            },
            // Placeholder for Warlords of Draenor build definitions.
            // Uncomment and fill in the actual DB2 files when they are available.
            /*
                {
                    "Warlords of Draenor",
                    21742, // Final 6.2.4 Build
                    21742,
                    WodDefinition::DB2_FILES
                },
                */
            // Add future expansion definitions here as needed:
            // {
            //     "Midnight",
            //     Midnight::MinSupportedBuild,
            //     Midnight::MaxSupportedBuild,
            //     MidnightDefinition::DB2_FILES
            // }
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
