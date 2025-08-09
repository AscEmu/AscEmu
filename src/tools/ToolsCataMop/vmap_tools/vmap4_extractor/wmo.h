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

#ifndef WMO_H
#define WMO_H
#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)

#include <string>
#include <set>
#include "vec3d.h"
#include "mpqfile.h"

 // MOPY flags
#define WMO_MATERIAL_NOCAMCOLLIDE    0x01
#define WMO_MATERIAL_DETAIL          0x02
#define WMO_MATERIAL_NO_COLLISION    0x04
#define WMO_MATERIAL_HINT            0x08
#define WMO_MATERIAL_RENDER          0x10
#define WMO_MATERIAL_COLLIDE_HIT     0x20
#define WMO_MATERIAL_WALL_SURFACE    0x40

class WMOInstance;
class WMOManager;
class MPQFile;

/* for whatever reason a certain company just can't stick to one coordinate system... */
static inline Vec3D fixCoords(const Vec3D &v) {
    return Vec3D(v.z, v.x, v.y);
}

class WMORoot
{
private:
    std::string filename;
public:
    unsigned int col;
    uint32_t nTextures, nGroups, nP, nLights, nModels, nDoodads, nDoodadSets, RootWMOID, liquidType;
    float bbcorn1[3];
    float bbcorn2[3];

    WMORoot(std::string& filename);

    bool open();
    bool ConvertToVMAPRootWmo(FILE* output);
};

struct WMOLiquidHeader
{
    int xverts, yverts, xtiles, ytiles;
    float pos_x;
    float pos_y;
    float pos_z;
    short type;
};

#pragma pack(push, 1)

struct WMOLiquidVert
{
    uint16_t unk1;
    uint16_t unk2;
    float height;
};

#pragma pack(pop)

class WMOGroup
{
private:
    std::string filename;
public:
    // MOGP

    char* MOPY;
    uint16_t* MOVI;
    uint16_t* MoviEx;
    float* MOVT;
    uint16_t* MOBA;
    int* MobaEx;
    WMOLiquidHeader* hlq;
    WMOLiquidVert* LiquEx;
    char* LiquBytes;
    int groupName, descGroupName;
    int mogpFlags;
    float bbcorn1[3];
    float bbcorn2[3];
    uint16_t moprIdx;
    uint16_t moprNItems;
    uint16_t nBatchA;
    uint16_t nBatchB;
    uint32_t nBatchC, fogIdx, liquidType, groupWMOID;

    int mopy_size, moba_size;
    int LiquEx_size;
    unsigned int nVertices; // number when loaded
    int nTriangles; // number when loaded
    uint32_t liquflags;

    WMOGroup(std::string const& filename);
    ~WMOGroup();

    bool open();
    int ConvertToVMAPGroupWmo(FILE* output, WMORoot* rootWMO, bool preciseVectorData);
};

class WMOInstance
{
    static std::set<int> ids;
public:
    std::string MapName;
    int currx;
    int curry;
    WMOGroup* wmo;
    int doodadset;
    Vec3D pos;
    Vec3D pos2, pos3, rot;
    uint32_t indx, id, d2, d3;

    WMOInstance(MPQFile&f, char const* WmoInstName, uint32_t mapID, uint32_t tileX, uint32_t tileY, FILE* pDirfile);

    static void reset();
};

#endif  //WMO_H
