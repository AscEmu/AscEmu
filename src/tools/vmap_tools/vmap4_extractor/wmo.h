/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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
#include "mpqlib/FileLoader.hpp"

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

// for whatever reason a certain company just can't stick to one coordinate system
static inline Vec3D fixCoords(const Vec3D& v) { return Vec3D(v.z, v.x, v.y); }

class WMORoot
{
private:
    std::string m_filename;
public:
    unsigned int m_col;
    uint32_t m_numTextures, m_numGroups, m_numPortals, m_numLights, m_numModels, m_numDoodads, m_numDoodadSets, m_rootWmoId, m_liquidType;
    float m_boundingBoxMin[3];
    float m_boundingBoxMax[3];

    WMORoot(std::string& filename);

    bool open();
    bool convertToVMapRootWmo(FILE* output);
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
    std::string m_filename;
public:
    // MOGP

    char* m_mopy;
    uint16_t* m_movi;
    uint16_t* m_moviEx;
    float* m_movt;
    uint16_t* m_moba;
    int* m_mobaEx;
    WMOLiquidHeader* m_liquidHeader;
    WMOLiquidVert* m_liquidVerts;
    char* m_liquidBytes;
    int m_groupNameOffset, m_descGroupNameOffset;
    int m_mogpFlags;
    float m_boundingBoxMin[3];
    float m_boundingBoxMax[3];
    uint16_t m_portalRefIdx;
    uint16_t m_portalRefCount;
    uint16_t m_batchCountA;
    uint16_t m_batchCountB;
    uint32_t m_batchCountC, m_fogIndex, m_liquidType, m_groupWmoId;

    int m_mopySize, m_mobaSize;
    int m_liquidVertsSize;
    unsigned int m_vertexCount; // number when loaded
    int m_triangleCount;        // number when loaded
    uint32_t m_liquidFlags;

    WMOGroup(std::string const& filename);
    ~WMOGroup();

    bool open();
    int convertToVMapGroupWmo(FILE* output, WMORoot* rootWMO, bool preciseVectorData);
};

class WMOInstance
{
    static std::set<int> m_ids;
public:
    std::string m_mapName;
    int m_currX;
    int m_currY;
    WMOGroup* m_wmoGroup;
    int m_doodadSet;
    Vec3D m_pos;
    Vec3D m_pos2, m_pos3, m_rot;
    uint32_t m_index, m_id, m_d2, m_d3;

    WMOInstance(MPQFile& f, char const* wmoInstName, uint32_t mapID, uint32_t tileX, uint32_t tileY, FILE* pDirfile);

    static void reset();
};

#endif  //WMO_H
