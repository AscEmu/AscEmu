/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CascMapExtractor.hpp"
#include "MapExtractorState.hpp"
#include "wdt.h"

#include "mpqlib/ClientDb2Layouts.hpp"
#include "mpqlib/Db2File.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <exception>
#include <execution>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    using mpqlib::Db2File;

    constexpr uint32_t kWdtHasMaid = 0x200;         // MPHD flag: the MAID chunk names the tile files

    struct MapEntry
    {
        uint32_t id = 0;
        std::string directory;
        std::string name;
        uint32_t wdtFileDataId = 0;
    };

    struct TileWorkItem
    {
        uint32_t mapId;
        std::string mapName;
        uint32_t adtFileDataId;         // 0: load by name
        std::string adtName;
        std::string outputFile;
        uint32_t y;
        uint32_t x;
        bool ignoreDeepWater;
    };

    std::optional<Db2File> loadTable(mpqlib::CascStorage const& storage, mpqlib::Db2TableLocation const& table, char const* what)
    {
        std::optional<std::vector<std::byte>> bytes;
        if (table.fileDataId != 0)
            bytes = storage.readFile(table.fileDataId);
        else
            bytes = storage.readFile(table.fileName);

        if (!bytes)
        {
            printf("Fatal error: cannot find %s in the client (%.*s)\n", what,
                static_cast<int>(mpqlib::CascStorage::errorText(mpqlib::CascStorage::lastError()).size()), mpqlib::CascStorage::errorText(mpqlib::CascStorage::lastError()).data());
            return std::nullopt;
        }

        std::string error;
        auto file = Db2File::load(std::move(*bytes), error);
        if (!file)
        {
            printf("Fatal error: cannot read %s: %s\n", what, error.c_str());
            return std::nullopt;
        }

        if (table.layoutHash != 0 && file->layoutHash() != table.layoutHash)
            printf("Warning: %s has layout 0x%08X, the column indices were taken from layout 0x%08X - check the extracted data\n", what, file->layoutHash(), table.layoutHash);

        return file;
    }

    bool readMapTable(mpqlib::CascStorage const& storage, mpqlib::MapTableColumns const& layout, std::vector<MapEntry>& maps)
    {
        printf("Read Map table...\n");
        auto file = loadTable(storage, layout.table, "Map");
        if (!file)
            return false;

        std::unordered_map<uint32_t, size_t> indexById;
        for (uint32_t i = 0; i < file->recordCount(); ++i)
        {
            if (!file->isReadable(i))
                continue;

            auto const record = file->record(i);
            MapEntry map;
            map.id = record.id();
            map.directory = std::string(record.getString(layout.directory));
            map.name = std::string(record.getString(layout.mapName));
            if (layout.wdtFileDataId >= 0)
                map.wdtFileDataId = record.getUInt32(static_cast<uint32_t>(layout.wdtFileDataId));
            if (map.directory.empty())
                continue;

            indexById[map.id] = maps.size();
            maps.push_back(std::move(map));
        }

        // rows that only rename another row's id share its files
        for (Db2File::Copy const& copy : file->copies())
        {
            auto const it = indexById.find(copy.sourceId);
            if (it == indexById.end())
                continue;

            MapEntry map = maps[it->second];
            map.id = copy.newId;
            maps.push_back(std::move(map));
        }

        printf("Done! (%zu maps loaded)\n", maps.size());
        return true;
    }

    bool readLiquidTables(mpqlib::CascStorage const& storage, mpqlib::ClientDb2Layouts const& layouts)
    {
        printf("Read LiquidMaterial table...\n");
        if (auto file = loadTable(storage, layouts.liquidMaterial.table, "LiquidMaterial"))
        {
            for (uint32_t i = 0; i < file->recordCount(); ++i)
            {
                if (!file->isReadable(i))
                    continue;

                auto const record = file->record(i);
                LiquidMaterialEntry& entry = LiquidMaterials[record.id()];
                entry.LVF = static_cast<int8_t>(record.getUInt32(layouts.liquidMaterial.lvf));
                if (layouts.liquidMaterial.flags >= 0)
                    entry.Flags = record.getUInt32(static_cast<uint32_t>(layouts.liquidMaterial.flags));
            }
            for (Db2File::Copy const& copy : file->copies())
                LiquidMaterials[copy.newId] = LiquidMaterials[copy.sourceId];
        }
        else
            return false;
        printf("Done! (%zu LiquidMaterials loaded)\n", LiquidMaterials.size());

        printf("Read LiquidObject table...\n");
        if (auto file = loadTable(storage, layouts.liquidObject.table, "LiquidObject"))
        {
            for (uint32_t i = 0; i < file->recordCount(); ++i)
            {
                if (!file->isReadable(i))
                    continue;

                auto const record = file->record(i);
                LiquidObjects[record.id()].LiquidTypeID = static_cast<int16_t>(record.getUInt32(layouts.liquidObject.liquidTypeId));
            }
            for (Db2File::Copy const& copy : file->copies())
                LiquidObjects[copy.newId] = LiquidObjects[copy.sourceId];
        }
        else
            return false;
        printf("Done! (%zu LiquidObjects loaded)\n", LiquidObjects.size());

        printf("Read LiquidType table...\n");
        if (auto file = loadTable(storage, layouts.liquidType.table, "LiquidType"))
        {
            for (uint32_t i = 0; i < file->recordCount(); ++i)
            {
                if (!file->isReadable(i))
                    continue;

                auto const record = file->record(i);
                LiquidTypeEntry& entry = LiquidTypes[record.id()];
                entry.SoundBank = static_cast<uint8_t>(record.getUInt32(layouts.liquidType.soundBank));
                entry.MaterialID = static_cast<uint8_t>(record.getUInt32(layouts.liquidType.materialId));
            }
            for (Db2File::Copy const& copy : file->copies())
                LiquidTypes[copy.newId] = LiquidTypes[copy.sourceId];
        }
        else
            return false;
        printf("Done! (%zu LiquidTypes loaded)\n", LiquidTypes.size());
        return true;
    }

    std::optional<std::vector<std::byte>> readWdt(mpqlib::CascStorage const& storage, MapEntry const& map)
    {
        if (map.wdtFileDataId != 0)
            return storage.readFile(map.wdtFileDataId);

        return storage.readFile("World\\Maps\\" + map.directory + "\\" + map.directory + ".wdt");
    }

    // One small WDT read per map tells which tiles exist and, from 8.1 on, which files hold them.
    void discoverTiles(mpqlib::CascStorage const& storage, MapEntry const& map, std::vector<TileWorkItem>& workItems)
    {
        auto bytes = readWdt(storage, map);
        if (!bytes)
            return;

        auto wdt = loadCascWdtChunkTreeFromBytes(std::move(*bytes), "WDT of map " + std::to_string(map.id));
        if (!wdt)
            return;

        mpqlib::ChunkNode const* main = wdt->find("MAIN");
        if (main == nullptr)
            return;

        wdt_MAIN const* mainData = nullptr;
        wdt_MAID const* maidData = nullptr;
        try
        {
            mainData = &main->as<wdt_MAIN>();
            if (mpqlib::ChunkNode const* mphd = wdt->find("MPHD"); mphd != nullptr && (mphd->as<wdt_MPHD>().flags & kWdtHasMaid))
            {
                if (mpqlib::ChunkNode const* maid = wdt->find("MAID"))
                    maidData = &maid->as<wdt_MAID>();
            }
        }
        catch (std::exception const& e)
        {
            printf("Error reading the WDT of map %u: %s\n", map.id, e.what());
            return;
        }

        char outputFile[1024];
        for (uint32_t y = 0; y < WDT_MAP_SIZE; ++y)
        {
            for (uint32_t x = 0; x < WDT_MAP_SIZE; ++x)
            {
                if (!(mainData->adt_list[y][x].flag & 0x1))
                    continue;

                TileWorkItem item;
                item.mapId = map.id;
                item.mapName = map.name;
                item.y = y;
                item.x = x;
                item.ignoreDeepWater = IsDeepWaterIgnored(map.id, y, x);
                if (maidData != nullptr)
                {
                    item.adtFileDataId = maidData->adt_files[y][x].rootADT;
                    if (item.adtFileDataId == 0)
                        continue;
                }
                else
                {
                    item.adtFileDataId = 0;
                    item.adtName = "World\\Maps\\" + map.directory + "\\" + map.directory + "_" + std::to_string(x) + "_" + std::to_string(y) + ".adt";
                }

                snprintf(outputFile, sizeof(outputFile), "%s/maps/%04u_%02u_%02u.map", output_path, map.id, y, x);
                item.outputFile = outputFile;
                workItems.push_back(std::move(item));
            }
        }
    }

    bool convertTile(mpqlib::CascStorage const& storage, TileWorkItem const& item, uint32_t build)
    {
        std::string const description = item.mapName + " [" + std::to_string(item.y) + "," + std::to_string(item.x) + "]";
        auto bytes = item.adtFileDataId != 0 ? storage.readFile(item.adtFileDataId) : storage.readFile(item.adtName);
        if (!bytes)
            return false;

        auto adt = loadAdtChunkTreeFromBytes(std::move(*bytes), description);
        if (!adt)
            return false;

        return ConvertADT(*adt, description, item.outputFile, build, item.ignoreDeepWater);
    }
}

bool extractMapsFromCasc(mpqlib::CascStorage const& storage, mpqlib::ClientVersion version, uint32_t build)
{
    auto const layouts = mpqlib::clientDb2LayoutsFor(version);
    if (!layouts)
    {
        printf("No map table layout is known for %.*s, maps are not extracted.\n",
            static_cast<int>(mpqlib::getClientVersionName(version).size()), mpqlib::getClientVersionName(version).data());
        return false;
    }

    printf("Extracting maps...\n");

    std::vector<MapEntry> maps;
    if (!readMapTable(storage, layouts->map, maps))
        return false;
    if (!readLiquidTables(storage, *layouts))
        return false;

    CreateDir(std::string(output_path) + "/maps/");

    // Tile discovery is one small read per map and stays sequential; the
    // conversion of the tiles is independent per tile and runs on every core,
    // CascStorage serialises the reads itself and ConvertADT's scratch
    // buffers are thread_local.
    std::vector<TileWorkItem> workItems;
    printf("Discovering tiles\n");
    for (size_t i = 0; i < maps.size(); ++i)
    {
        printf("Discover %s (%zu/%zu)\n", maps[i].name.c_str(), i + 1, maps.size());
        discoverTiles(storage, maps[i], workItems);
    }
    printf("Converting %zu map tiles...\n", workItems.size());

    std::atomic<size_t> tilesCompleted{ 0 };
    std::mutex progressMutex;
    size_t const total = workItems.size();
    size_t const reportInterval = std::max<size_t>(1, total / 100);

    std::for_each(std::execution::par, workItems.begin(), workItems.end(), [&](TileWorkItem const& item)
    {
        try
        {
            convertTile(storage, item, build);
        }
        catch (std::exception const& e)
        {
            printf("Error converting %s [%u,%u]: %s\n", item.mapName.c_str(), item.y, item.x, e.what());
        }

        size_t const completed = tilesCompleted.fetch_add(1, std::memory_order_relaxed) + 1;
        if (completed % reportInterval == 0 || completed == total)
        {
            std::lock_guard<std::mutex> lock(progressMutex);
            printf("\rConverting tiles: %zu/%zu (%u%%)          ", completed, total, static_cast<unsigned>(completed * 100 / total));
            fflush(stdout);
        }
    });
    printf("\n");
    return true;
}
