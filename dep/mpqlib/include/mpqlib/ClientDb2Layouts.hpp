/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Which column of the client tables the extractors read holds what, per
// CASC client version. The tables carry no column names, only their packing,
// so the indices below are the one piece of per-version knowledge the tools
// need - a property of the client like the archive lists in
// ClientArchiveData. 6.x still reads the WDBC .dbc files by name, 7.x and
// later the .db2 files by their (stable) file data ids. The layout hashes are
// the ones of the builds these indices were taken from - a different hash
// only warns, a patch of the same expansion usually keeps the columns in
// place.

#pragma once

#include "mpqlib/ClientVersion.hpp"

#include <cstdint>
#include <optional>

namespace mpqlib
{
    struct Db2TableLocation
    {
        uint32_t layoutHash;            // 0 when the format has none (WDBC)
        uint32_t fileDataId;            // 0: open by name instead
        char const* fileName;           // for the .dbc clients
    };

    struct MapTableColumns
    {
        Db2TableLocation table;
        uint32_t directory;             // the folder of the map under World/Maps
        uint32_t mapName;
        int32_t wdtFileDataId;          // -1: the WDT is opened by name
    };

    struct LiquidTypeColumns
    {
        Db2TableLocation table;
        uint32_t soundBank;
        uint32_t materialId;
    };

    struct LiquidMaterialColumns
    {
        Db2TableLocation table;
        uint32_t lvf;
        int32_t flags;                  // -1: none in this build
    };

    struct LiquidObjectColumns
    {
        Db2TableLocation table;
        uint32_t liquidTypeId;
    };

    struct GameObjectDisplayInfoColumns
    {
        Db2TableLocation table;
        int32_t fileDataId;             // -1: the model is named by modelName instead
        uint32_t modelName;
    };

    struct ClientDb2Layouts
    {
        MapTableColumns map;
        LiquidTypeColumns liquidType;
        LiquidMaterialColumns liquidMaterial;
        LiquidObjectColumns liquidObject;
        GameObjectDisplayInfoColumns gameObjectDisplayInfo;
    };

    inline constexpr uint32_t kMapFileDataId = 1349477;
    inline constexpr uint32_t kLiquidTypeFileDataId = 1371380;
    inline constexpr uint32_t kLiquidMaterialFileDataId = 1132538;
    inline constexpr uint32_t kLiquidObjectFileDataId = 1308058;
    inline constexpr uint32_t kGameObjectDisplayInfoFileDataId = 1266277;

    // 6.2.4: WDBC files, the id is column 0 of every table
    inline constexpr ClientDb2Layouts kWodDb2Layouts =
    {
        { { 0, 0, "DBFilesClient\\Map.dbc" }, 1, 1, -1 },
        { { 0, 0, "DBFilesClient\\LiquidType.dbc" }, 3, 14 },
        { { 0, 0, "DBFilesClient\\LiquidMaterial.dbc" }, 1, 2 },
        { { 0, 0, "DBFilesClient\\LiquidObject.dbc" }, 3 },
        { { 0, 0, "DBFilesClient\\GameObjectDisplayInfo.dbc" }, -1, 1 },
    };

    // 7.3.5: WDC1, the id comes from the id list and is not a column
    inline constexpr ClientDb2Layouts kLegionDb2Layouts =
    {
        { { 0xF568DF12, kMapFileDataId, nullptr }, 0, 1, -1 },
        { { 0x3313BBF3, kLiquidTypeFileDataId, nullptr }, 13, 16 },
        { { 0x62BE0340, kLiquidMaterialFileDataId, nullptr }, 0, 1 },
        { { 0xACC168A6, kLiquidObjectFileDataId, nullptr }, 2 },
        { { 0x9F2098D1, kGameObjectDisplayInfoFileDataId, nullptr }, 0, 0 },
    };

    // 8.3.7: WDC3 with the WDT file data id in the map table
    inline constexpr ClientDb2Layouts kBfaDb2Layouts =
    {
        { { 0xDA9493AF, kMapFileDataId, nullptr }, 0, 1, 21 },
        { { 0xD4966711, kLiquidTypeFileDataId, nullptr }, 3, 14 },
        { { 0xD384A6A0, kLiquidMaterialFileDataId, nullptr }, 1, 0 },
        { { 0xF09062A1, kLiquidObjectFileDataId, nullptr }, 2 },
        { { 0xE56389DF, kGameObjectDisplayInfoFileDataId, nullptr }, 1, 0 },
    };

    // 9.2.7
    inline constexpr ClientDb2Layouts kShadowlandsDb2Layouts =
    {
        { { 0xAAA1F94E, kMapFileDataId, nullptr }, 0, 1, 21 },
        { { 0xAFFFC9E0, kLiquidTypeFileDataId, nullptr }, 3, 14 },
        { { 0x2CFFEA40, kLiquidMaterialFileDataId, nullptr }, 1, 0 },
        { { 0x6CAEB8A1, kLiquidObjectFileDataId, nullptr }, 2 },
        { { 0x7E33A93F, kGameObjectDisplayInfoFileDataId, nullptr }, 1, 0 },
    };

    // 10.2.7
    inline constexpr ClientDb2Layouts kDragonflightDb2Layouts =
    {
        { { 0x15542FE8, kMapFileDataId, nullptr }, 0, 1, 21 },
        { { 0xAFFFC9E0, kLiquidTypeFileDataId, nullptr }, 3, 14 },
        { { 0x2CFFEA40, kLiquidMaterialFileDataId, nullptr }, 1, 0 },
        { { 0x6CAEB8A1, kLiquidObjectFileDataId, nullptr }, 2 },
        { { 0x7F774625, kGameObjectDisplayInfoFileDataId, nullptr }, 1, 0 },
    };

    // 11.2.7
    inline constexpr ClientDb2Layouts kTheWarWithinDb2Layouts =
    {
        { { 0x75863E23, kMapFileDataId, nullptr }, 0, 1, 21 },
        { { 0xAFFFC9E0, kLiquidTypeFileDataId, nullptr }, 3, 14 },
        { { 0x2CFFEA40, kLiquidMaterialFileDataId, nullptr }, 1, 0 },
        { { 0x6CAEB8A1, kLiquidObjectFileDataId, nullptr }, 2 },
        { { 0xD85414D1, kGameObjectDisplayInfoFileDataId, nullptr }, 1, 0 },
    };

    // 12.1.5
    inline constexpr ClientDb2Layouts kMidnightDb2Layouts =
    {
        { { 0x75863E23, kMapFileDataId, nullptr }, 0, 1, 21 },
        { { 0xD1ECEEC9, kLiquidTypeFileDataId, nullptr }, 3, 14 },
        { { 0x98E5D7AA, kLiquidMaterialFileDataId, nullptr }, 1, 0 },
        { { 0xCB0D39E8, kLiquidObjectFileDataId, nullptr }, 2 },
        { { 0x16D1DD61, kGameObjectDisplayInfoFileDataId, nullptr }, 1, 0 },
    };

    inline std::optional<ClientDb2Layouts> clientDb2LayoutsFor(ClientVersion version)
    {
        switch (version)
        {
            case ClientVersion::WarlordsOfDraenor:  return kWodDb2Layouts;
            case ClientVersion::Legion:             return kLegionDb2Layouts;
            case ClientVersion::BattleForAzeroth:   return kBfaDb2Layouts;
            case ClientVersion::Shadowlands:        return kShadowlandsDb2Layouts;
            case ClientVersion::Dragonflight:       return kDragonflightDb2Layouts;
            case ClientVersion::TheWarWithin:       return kTheWarWithinDb2Layouts;
            case ClientVersion::Midnight:           return kMidnightDb2Layouts;
            default:                                return std::nullopt;
        }
    }
}
