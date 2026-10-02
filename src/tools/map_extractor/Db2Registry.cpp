/*
Copyright(c) 2014 - 2026 AscEmu Team < http://www.ascemu.org>
This file is released under the MIT license.See README - MIT for more information.
*/

#include "Db2Registry.hpp"
#include "Db2LegionDefinition.hpp"
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
            //     "Forever/Classic+",
            //     Forever::MinSupportedBuild,
            //     Forever::MaxSupportedBuild,
            //     Forever::Db2Files
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
