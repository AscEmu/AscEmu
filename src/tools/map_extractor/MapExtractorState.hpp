/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// The pieces of System.cpp the CASC map extraction shares with the MPQ one:
// the liquid lookups both fill from their client's tables, the ADT to .map
// conversion itself and the chunk tree loaders for files that already sit in
// memory.

#pragma once

#include "mpqlib/ChunkTree.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#define MAX_PATH_LENGTH 128

struct LiquidMaterialEntry
{
    int8_t LVF = -1;
    uint32_t Flags = 0;         // 0x2: visual only, no collision liquid
};

struct LiquidObjectEntry
{
    int16_t LiquidTypeID = 0;
};

struct LiquidTypeEntry
{
    uint8_t SoundBank = 0;
    uint8_t MaterialID = 0;
};

extern std::unordered_map<uint32_t, LiquidMaterialEntry> LiquidMaterials;
extern std::unordered_map<uint32_t, LiquidObjectEntry> LiquidObjects;
extern std::unordered_map<uint32_t, LiquidTypeEntry> LiquidTypes;

extern char output_path[MAX_PATH_LENGTH];

void CreateDir(std::string const& path);
bool IsDeepWaterIgnored(uint32_t mapId, uint32_t x, uint32_t y);

// Converts one loaded ADT into a .map file; inputName only names the tile in messages.
bool ConvertADT(mpqlib::ChunkTree const& adtTree, std::string const& inputName, std::string const& outputPath, uint32_t build, bool ignoreDeepWater);

// Chunk trees of ADT and WDT files handed over as bytes (CASC reads whole files).
std::optional<mpqlib::ChunkTree> loadAdtChunkTreeFromBytes(std::vector<std::byte> bytes, std::string_view description);
std::optional<mpqlib::ChunkTree> loadCascWdtChunkTreeFromBytes(std::vector<std::byte> bytes, std::string_view description);
