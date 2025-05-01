/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADT_FILE_H
#define ADT_FILE_H

#include "mpq_libmpq04.h"
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
}svec;

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
    float v9[16*8+1][16*8+1];
    float v8[16*8][16*8];
}Cell;

typedef struct
{
    double v9[9][9];
    double v8[8][8];
    uint16_t area_id;
    //Liquid *lq;
    float waterlevel[9][9];
    uint8_t flag;
}chunk;

typedef struct
{
    chunk ch[16][16];
}mcell;

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
    float  zpos;
    float  xpos;
    float  ypos;
    uint32_t textureId;
    uint32_t props;
    uint32_t effectId;
};


class ADTFile
{
private:
    MPQFile ADT;
    std::string Adtfilename;
public:
    ADTFile(char* filename);
    ~ADTFile();
    int nWMO;
    int nMDX;
    std::string* WmoInstansName;
    std::string* ModelInstansName;
    bool init(uint32_t map_num, uint32_t tileX, uint32_t tileY);
};

const char* GetPlainName(const char* FileName);
char* GetPlainName(char* FileName);
char* GetExtension(char* FileName);
void fixnamen(char *name, size_t len);
void fixname2(char *name, size_t len);

#endif  //ADT_FILE_H
