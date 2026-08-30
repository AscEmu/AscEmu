/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

#include "mpqlib/MPQFile.hpp"
#include "wmo.h"
#include "model.h"

#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)
#define UNITSIZE (CHUNKSIZE / 8.0f)

class Liquid;

typedef struct
{
    float x;
    float y;
    float z;
} svec;

struct vec
{
    double x;
    double y;
    double z;
};

struct triangle
{
    vec v[3];
};

typedef struct
{
    float v9[16 * 8 + 1][16 * 8 + 1];
    float v8[16 * 8][16 * 8];
} Cell;

typedef struct
{
    double v9[9][9];
    double v8[8][8];
    uint16_t area_id;
    float waterlevel[9][9];
    uint8_t flag;
} chunk;

typedef struct
{
    chunk ch[16][16];
} mcell;

struct MapChunkHeader
{
    uint32_t flags;
    uint32_t ix;
    uint32_t iy;
    uint32_t nLayers;
    uint32_t nDoodadRefs;
    uint32_t ofsHeight;
    uint32_t ofsNormal;
    uint32_t ofsLayer;
    uint32_t ofsRefs;
    uint32_t ofsAlpha;
    uint32_t sizeAlpha;
    uint32_t ofsShadow;
    uint32_t sizeShadow;
    uint32_t areaid;
    uint32_t nMapObjRefs;
    uint32_t holes;
    uint16_t s1;
    uint16_t s2;
    uint32_t d1;
    uint32_t d2;
    uint32_t d3;
    uint32_t predTex;
    uint32_t nEffectDoodad;
    uint32_t ofsSndEmitters;
    uint32_t nSndEmitters;
    uint32_t ofsLiquid;
    uint32_t sizeLiquid;
    float zpos;
    float xpos;
    float ypos;
    uint32_t textureId;
    uint32_t props;
    uint32_t effectId;
};

class ADTFile
{
public:
    ADTFile(std::string const& filename);
    ~ADTFile();

    bool init(uint32_t mapNum, uint32_t tileX, uint32_t tileY);

    int m_wmoCount;
    int m_modelCount;
    std::string* m_wmoInstanceNames;
    std::string* m_modelInstanceNames;

private:
    MPQFile m_adt;
    std::string m_adtFilename;
};

const char* getPlainName(const char* fileName);
char* getPlainName(char* fileName);
char* getExtension(char* fileName);
void fixNameCase(char* name, size_t len);
void fixNameSpaces(char* name, size_t len);
