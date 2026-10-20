/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// The model and map object placement records of ADT and WDT files, as they
// lie on disk (wowdev.wiki/ADT: MDDF and MODF). Unchanged since Classic
// except for the flags the CASC clients added: from 8.1 on an entry may name
// its model by file data id instead of by an index into the name list, and
// map objects may carry a scale.

#pragma once

#include <cstdint>

#pragma pack(push, 1)

// MDDF: one doodad (M2) placement
struct AdtDoodadDef
{
    uint32_t id;                // index into MMDX, or a file data id with kDoodadDefFileDataId
    uint32_t uniqueId;
    float position[3];
    float rotation[3];
    uint16_t scale;             // 1024 = 1.0
    uint16_t flags;
};

// MODF: one map object (WMO) placement
struct AdtMapObjDef
{
    uint32_t id;                // index into MWMO, or a file data id with kMapObjDefFileDataId
    uint32_t uniqueId;
    float position[3];
    float rotation[3];
    float boundsMin[3];
    float boundsMax[3];
    uint16_t flags;
    uint16_t doodadSet;
    uint16_t nameSet;
    uint16_t scale;             // 1024 = 1.0, only with kMapObjDefHasScale
};

#pragma pack(pop)

static_assert(sizeof(AdtDoodadDef) == 36);
static_assert(sizeof(AdtMapObjDef) == 64);

inline constexpr uint16_t kDoodadDefFileDataId = 0x40;
inline constexpr uint16_t kMapObjDefHasScale = 0x4;
inline constexpr uint16_t kMapObjDefFileDataId = 0x8;
