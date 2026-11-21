/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string_view>

namespace AscEmu::Forever
{
    // the realm the 1.60 client is told as its last played one; the logon and the world server answer with the same entry
    inline constexpr std::string_view LastPlayedRealmEntry =
        "JamJSONRealmEntry:{"
        "\"wowRealmAddress\":1174470658,"
        "\"useBleepChance\":0.0,"
        "\"cfgTimezonesID\":1,"
        "\"populationState\":2,"
        "\"cfgCategoriesID\":26,"
        "\"version\":{"
            "\"versionMajor\":1,"
            "\"versionBuild\":69800,"
            "\"versionMinor\":60,"
            "\"versionRevision\":1"
        "},"
        "\"cfgRealmsID\":4618,"
        "\"gameServiceRegionId\":98,"
        "\"flags\":0,"
        "\"name\":\"Classic Beta PvE\","
        "\"cfgConfigsID\":1,"
        "\"cfgContentSetID\":137,"
        "\"cfgLanguagesID\":1,"
        "\"superDistrictID\":2"
        "}";
}
