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

#include "vmapexport.h"
#include "wmo.h"
#include "vec3d.h"
#include "mpqlib/MPQFile.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <span>
#undef min
#undef max

extern uint16_t* LiqType;

WMORoot::WMORoot(std::string& filename) :
    m_filename(filename), m_col(0), m_numTextures(0), m_numGroups(0), m_numPortals(0), m_numLights(0),
    m_numModels(0), m_numDoodads(0), m_numDoodadSets(0), m_rootWmoId(0), m_liquidType(0)
{
    memset(m_boundingBoxMin, 0, sizeof(m_boundingBoxMin));
    memset(m_boundingBoxMax, 0, sizeof(m_boundingBoxMax));
}

bool WMORoot::open()
{
    std::unique_ptr<MPQFile> file = OpenWorldFile(m_filename);
    MPQFile& f = *file;
    if (f.isEof())
    {
        printf("No such file.\n");
        return false;
    }

    uint32_t size;
    char fourCC[5];

    while (!f.isEof())
    {
        f.read(fourCC, 4);
        f.read(&size, 4);

        flipFourCC(fourCC);
        fourCC[4] = 0;

        size_t nextPos = f.getPos() + size;

        if (!strcmp(fourCC, "MOHD")) // header
        {
            f.read(&m_numTextures, 4);
            f.read(&m_numGroups, 4);
            f.read(&m_numPortals, 4);
            f.read(&m_numLights, 4);
            f.read(&m_numModels, 4);
            f.read(&m_numDoodads, 4);
            f.read(&m_numDoodadSets, 4);
            f.read(&m_col, 4);
            f.read(&m_rootWmoId, 4);
            f.read(m_boundingBoxMin, 12);
            f.read(m_boundingBoxMax, 12);
            f.read(&m_liquidType, 4);
        }
        else if (!strcmp(fourCC, "MOGN"))
        {
            // the group names, null terminated, named by offset from the group header
            m_groupNames.resize(size);
            f.read(m_groupNames.data(), size);
        }
        else if (!strcmp(fourCC, "GFID"))
        {
            // 7.x and later: the group files by file data id. Roots with
            // several detail levels list every level; only the first, most
            // detailed one is of interest here.
            for (uint32_t group = 0; group < m_numGroups && f.getPos() + 4 <= nextPos; ++group)
            {
                uint32_t fileDataId = 0;
                f.read(&fileDataId, 4);
                if (fileDataId != 0)
                    m_groupFileDataIds.push_back(fileDataId);
            }
        }

        f.seek(static_cast<int>(nextPos));
    }
    f.close();
    return true;
}

bool WMORoot::convertToVMapRootWmo(FILE* pOutfile)
{
    fwrite(szRawVMAPMagic, 1, 8, pOutfile);
    unsigned int nVectors = 0;
    fwrite(&nVectors, sizeof(nVectors), 1, pOutfile); // will be filled later
    fwrite(&m_numGroups, 4, 1, pOutfile);
    fwrite(&m_rootWmoId, 4, 1, pOutfile);
    return true;
}

WMOGroup::WMOGroup(const std::string& filename) :
    m_filename(filename), m_movt(nullptr), m_moba(nullptr), m_mobaEx(nullptr),
    m_liquidHeader(nullptr), m_liquidVerts(nullptr), m_liquidBytes(nullptr), m_groupNameOffset(0), m_descGroupNameOffset(0), m_mogpFlags(0),
    m_portalRefIdx(0), m_portalRefCount(0), m_batchCountA(0), m_batchCountB(0), m_batchCountC(0), m_fogIndex(0),
    m_liquidType(0), m_groupWmoId(0), m_mobaSize(0), m_liquidVertsSize(0),
    m_vertexCount(0), m_triangleCount(0), m_liquidFlags(0)
{
    memset(m_boundingBoxMin, 0, sizeof(m_boundingBoxMin));
    memset(m_boundingBoxMax, 0, sizeof(m_boundingBoxMax));
}

bool WMOGroup::open()
{
    std::unique_ptr<MPQFile> file = OpenWorldFile(m_filename);
    MPQFile& f = *file;
    if (f.isEof())
    {
        printf("No such file.\n");
        return false;
    }
    uint32_t size;
    char fourCC[5];
    while (!f.isEof())
    {
        f.read(fourCC, 4);
        f.read(&size, 4);
        flipFourCC(fourCC);
        if (!strcmp(fourCC, "MOGP"))//Fix sizeoff = Data size.
        {
            size = 68;
        }
        fourCC[4] = 0;
        size_t nextPos = f.getPos() + size;
        m_liquidVertsSize = 0;
        m_liquidFlags = 0;

        if (!strcmp(fourCC, "MOGP"))//header
        {
            f.read(&m_groupNameOffset, 4);
            f.read(&m_descGroupNameOffset, 4);
            f.read(&m_mogpFlags, 4);
            f.read(m_boundingBoxMin, 12);
            f.read(m_boundingBoxMax, 12);
            f.read(&m_portalRefIdx, 2);
            f.read(&m_portalRefCount, 2);
            f.read(&m_batchCountA, 2);
            f.read(&m_batchCountB, 2);
            f.read(&m_batchCountC, 4);
            f.read(&m_fogIndex, 4);
            f.read(&m_liquidType, 4);
            f.read(&m_groupWmoId, 4);
        }
        else if (!strcmp(fourCC, "MOPY"))
        {
            // one flags byte and one material byte per triangle
            std::vector<uint8_t> mopy(size);
            f.read(mopy.data(), size);
            m_materialFlags.assign(mopy.begin(), mopy.end());
            m_triangleCount = static_cast<int>(size) / 2;
        }
        else if (!strcmp(fourCC, "MPY2"))
        {
            // 10.x and later: the same pairs as 16 bit words
            m_materialFlags.resize(size / 2);
            f.read(m_materialFlags.data(), size);
            m_triangleCount = static_cast<int>(size) / 4;
        }
        else if (!strcmp(fourCC, "MOVI"))
        {
            std::vector<uint16_t> movi(size / 2);
            f.read(movi.data(), size);
            m_indices.assign(movi.begin(), movi.end());
        }
        else if (!strcmp(fourCC, "MOVX"))
        {
            // 10.x and later: 32 bit indices for the big groups
            m_indices.resize(size / 4);
            f.read(m_indices.data(), size);
        }
        else if (!strcmp(fourCC, "MOVT"))
        {
            m_movt = new float[size / 4];
            f.read(m_movt, size);
            m_vertexCount = static_cast<int>(size) / 12;
        }
        else if (!strcmp(fourCC, "MONR"))
        {
        }
        else if (!strcmp(fourCC, "MOTV"))
        {
        }
        else if (!strcmp(fourCC, "MOBA"))
        {
            m_moba = new uint16_t[size / 2];
            m_mobaSize = size / 2;
            f.read(m_moba, size);
        }
        else if (!strcmp(fourCC, "MLIQ"))
        {
            m_liquidFlags |= 1;
            m_liquidHeader = new WMOLiquidHeader();
            f.read(m_liquidHeader, 0x1E);
            m_liquidVertsSize = sizeof(WMOLiquidVert) * m_liquidHeader->xverts * m_liquidHeader->yverts;
            m_liquidVerts = new WMOLiquidVert[m_liquidHeader->xverts * m_liquidHeader->yverts];
            f.read(m_liquidVerts, m_liquidVertsSize);
            int liquidByteCount = m_liquidHeader->xtiles * m_liquidHeader->ytiles;
            m_liquidBytes = new char[liquidByteCount];
            f.read(m_liquidBytes, liquidByteCount);
        }
        f.seek(static_cast<int>(nextPos));
    }
    f.close();

    // a group without geometry still writes a header, the loops below need matching sizes
    if (m_indices.size() < static_cast<size_t>(m_triangleCount) * 3)
        m_triangleCount = static_cast<int>(m_indices.size() / 3);
    if (m_materialFlags.size() < static_cast<size_t>(m_triangleCount) * 2)
        m_triangleCount = static_cast<int>(m_materialFlags.size() / 2);

    return true;
}

bool WMOGroup::shouldSkip(WMORoot const& rootWMO) const
{
    if (m_mogpFlags & WMO_GROUP_UNREACHABLE)
        return true;

    if (m_mogpFlags & WMO_GROUP_ANTIPORTAL)
        return true;

    if (m_groupNameOffset >= 0 && static_cast<size_t>(m_groupNameOffset) < rootWMO.m_groupNames.size())
    {
        char const* name = rootWMO.m_groupNames.data() + m_groupNameOffset;
        size_t const available = rootWMO.m_groupNames.size() - static_cast<size_t>(m_groupNameOffset);
        if (strncmp(name, "antiportal", available) == 0 && available > strlen("antiportal") && name[strlen("antiportal")] == '\0')
            return true;
    }

    return false;
}

int WMOGroup::convertToVMapGroupWmo(FILE* output, WMORoot* rootWMO, bool preciseVectorData)
{
    fwrite(&m_mogpFlags, sizeof(uint32_t), 1, output);
    fwrite(&m_groupWmoId, sizeof(uint32_t), 1, output);
    // group bound
    fwrite(m_boundingBoxMin, sizeof(float), 3, output);
    fwrite(m_boundingBoxMax, sizeof(float), 3, output);
    fwrite(&m_liquidFlags, sizeof(uint32_t), 1, output);
    int collisionTriangleCount = 0;

    // The vmap format stores 16 bit indices; a group with more vertices than
    // that can only be written in the compacted, collision only form.
    bool const fitsPrecise = m_vertexCount <= 0xFFFF;
    if (preciseVectorData && !fitsPrecise)
        printf("WMO group %s has %u vertices, writing its collision triangles only\n", m_filename.c_str(), m_vertexCount);

    if (preciseVectorData && fitsPrecise)
    {
        char grp[] = "GRP ";
        fwrite(grp, 1, 4, output);

        int batchIndex = 0;
        int batchCount = m_mobaSize / 12;
        m_mobaEx = new int[batchCount * 4];
        for (int i = 8; i < m_mobaSize; i += 12)
            m_mobaEx[batchIndex++] = m_moba[i];

        int batchGroupSize = batchCount * 4 + 4;
        fwrite(&batchGroupSize, 4, 1, output);
        fwrite(&batchCount, 4, 1, output);
        fwrite(m_mobaEx, 4, batchIndex, output);
        delete[] m_mobaEx;

        uint32_t indexCount = m_triangleCount * 3;
        std::vector<uint16_t> indices(indexCount);
        for (uint32_t i = 0; i < indexCount; ++i)
            indices[i] = static_cast<uint16_t>(m_indices[i]);

        if (fwrite("INDX", 4, 1, output) != 1)
        {
            printf("Error while writing file nbraches ID");
            exit(0);
        }
        int wsize = sizeof(uint32_t) + sizeof(unsigned short) * indexCount;
        if (fwrite(&wsize, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file wsize");
            // no need to exit?
        }
        if (fwrite(&indexCount, sizeof(uint32_t), 1, output) != 1)
        {
            printf("Error while writing file nIndexes");
            exit(0);
        }
        if (indexCount > 0)
        {
            if (fwrite(indices.data(), sizeof(unsigned short), indexCount, output) != indexCount)
            {
                printf("Error while writing file indexarray");
                exit(0);
            }
        }

        if (fwrite("VERT", 4, 1, output) != 1)
        {
            printf("Error while writing file nbraches ID");
            exit(0);
        }
        wsize = sizeof(int) + sizeof(float) * 3 * m_vertexCount;
        if (fwrite(&wsize, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file wsize");
            // no need to exit?
        }
        if (fwrite(&m_vertexCount, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file nVertices");
            exit(0);
        }
        if (m_vertexCount > 0)
        {
            if (fwrite(m_movt, sizeof(float) * 3, m_vertexCount, output) != m_vertexCount)
            {
                printf("Error while writing file vectors");
                exit(0);
            }
        }

        collisionTriangleCount = m_triangleCount;
    }
    else
    {
        char grp[] = "GRP ";
        fwrite(grp, 1, 4, output);
        int batchIndex = 0;
        int batchCount = m_mobaSize / 12;
        m_mobaEx = new int[batchCount * 4];
        for (int i = 8; i < m_mobaSize; i += 12)
            m_mobaEx[batchIndex++] = m_moba[i];

        int batchGroupSize = batchCount * 4 + 4;
        fwrite(&batchGroupSize, 4, 1, output);
        fwrite(&batchCount, 4, 1, output);
        fwrite(m_mobaEx, 4, batchIndex, output);
        delete[] m_mobaEx;

        //-------INDX------------------------------------
        //-------MOPY--------
        std::vector<uint32_t> collisionIndices(static_cast<size_t>(m_triangleCount) * 3); // "worst case" size...
        auto indexRenumber = std::make_unique<int[]>(m_vertexCount);
        memset(indexRenumber.get(), 0xFF, m_vertexCount * sizeof(int));
        for (int i = 0; i < m_triangleCount; ++i)
        {
            // Skip triangles with no collision. A plain render face (RENDER
            // set, DETAIL not set) is treated as collidable by default.
            // F_COLLISION's own real meaning is unusual/uncertain even in
            // wowdev.wiki's own documentation ("turns off water ripple rendering...
            // should be used for ghost material triangles"), so it's an additional
            // way for a triangle to opt in, not the primary signal.
            bool isRenderFace = (m_materialFlags[2 * i] & WMO_MATERIAL_RENDER) && !(m_materialFlags[2 * i] & WMO_MATERIAL_DETAIL);
            bool isCollision = (m_materialFlags[2 * i] & WMO_MATERIAL_COLLISION) || isRenderFace;
            if (!isCollision)
                continue;
            // Use this triangle
            bool validIndices = true;
            for (int j = 0; j < 3; ++j)
                if (m_indices[3 * i + j] >= m_vertexCount)
                    validIndices = false;
            if (!validIndices)
                continue;
            for (int j = 0; j < 3; ++j)
            {
                indexRenumber[m_indices[3 * i + j]] = 1;
                collisionIndices[3 * collisionTriangleCount + j] = m_indices[3 * i + j];
            }
            ++collisionTriangleCount;
        }

        // assign new vertex index numbers
        int collisionVertexCount = 0;
        for (uint32_t i = 0; i < m_vertexCount; ++i)
        {
            if (indexRenumber[i] == 1)
            {
                indexRenumber[i] = collisionVertexCount;
                ++collisionVertexCount;
            }
        }

        if (collisionVertexCount > 0xFFFF)
        {
            // cannot be addressed with 16 bit indices, the group goes out without geometry
            printf("WMO group %s has %d collision vertices, more than the vmap format can hold - skipped\n", m_filename.c_str(), collisionVertexCount);
            collisionTriangleCount = 0;
            collisionVertexCount = 0;
            memset(indexRenumber.get(), 0xFF, m_vertexCount * sizeof(int));
        }

        // translate triangle indices to new numbers
        std::vector<uint16_t> indices(static_cast<size_t>(collisionTriangleCount) * 3);
        for (int i = 0; i < 3 * collisionTriangleCount; ++i)
        {
            assert(collisionIndices[i] < m_vertexCount);
            indices[i] = static_cast<uint16_t>(indexRenumber[collisionIndices[i]]);
        }

        // write triangle indices
        int indx[] = { 0x58444E49, collisionTriangleCount * 6 + 4, collisionTriangleCount * 3 };
        fwrite(indx, 4, 3, output);
        fwrite(indices.data(), 2, collisionTriangleCount * 3, output);

        // write vertices
        int vert[] = { 0x54524556, collisionVertexCount * 3 * static_cast<int>(sizeof(float)) + 4, collisionVertexCount };// "VERT"
        int check = 3 * collisionVertexCount;
        fwrite(vert, 4, 3, output);
        for (uint32_t i = 0; i < m_vertexCount; ++i)
            if (indexRenumber[i] >= 0)
                check -= static_cast<int>(fwrite(m_movt + 3 * i, sizeof(float), 3, output));

        assert(check == 0);
    }

    //------LIQU------------------------
    if (m_liquidVertsSize != 0)
    {
        int liquHeader[] = { 0x5551494C, static_cast<int>(sizeof(WMOLiquidHeader) + m_liquidVertsSize) + m_liquidHeader->xtiles * m_liquidHeader->ytiles };// "LIQU"
        fwrite(liquHeader, 4, 2, output);

        // according to WoW.Dev Wiki:
        uint32_t liquidEntry;
        if (rootWMO->m_liquidType & 4)
            liquidEntry = m_liquidType;
        else if (m_liquidType == 15)
            liquidEntry = 0;
        else
            liquidEntry = m_liquidType + 1;

        if (!liquidEntry)
        {
            int tileCount = m_liquidHeader->xtiles * m_liquidHeader->ytiles;
            int tileIndex = 0;
            if (tileCount > 0)
            {
                while ((m_liquidBytes[tileIndex] & 0xF) == 15)
                {
                    ++tileIndex;
                    if (tileIndex >= tileCount)
                        break;
                }

                if (tileIndex < tileCount && (m_liquidBytes[tileIndex] & 0xF) != 15)
                    liquidEntry = (m_liquidBytes[tileIndex] & 0xF) + 1;
            }
        }

        if (liquidEntry && liquidEntry < 21)
        {
            switch ((liquidEntry - 1) & 3)
            {
                case 0:
                    liquidEntry = ((m_mogpFlags & 0x80000) != 0) + 13;
                    break;
                case 1:
                    liquidEntry = 14;
                    break;
                case 2:
                    liquidEntry = 19;
                    break;
                case 3:
                    liquidEntry = 20;
                    break;
            }
        }

        m_liquidHeader->type = static_cast<short>(liquidEntry);

        fwrite(m_liquidHeader, sizeof(WMOLiquidHeader), 1, output);
        // only need height values, the other values are unknown anyway
        for (uint32_t i = 0; i < m_liquidVertsSize / sizeof(WMOLiquidVert); ++i)
            fwrite(&m_liquidVerts[i].height, sizeof(float), 1, output);
        // todo: compress to bit field
        fwrite(m_liquidBytes, 1, m_liquidHeader->xtiles * m_liquidHeader->ytiles, output);
    }

    return collisionTriangleCount;
}

WMOGroup::~WMOGroup()
{
    delete[] m_movt;
    delete[] m_moba;
    delete m_liquidHeader;
    delete[] m_liquidVerts;
    delete[] m_liquidBytes;
}

WMOInstance::WMOInstance(AdtMapObjDef const& def, char const* wmoInstName, uint32_t mapID, uint32_t tileX, uint32_t tileY, FILE* pDirfile) :
    m_currX(0), m_currY(0), m_wmoGroup(nullptr), m_doodadSet(def.doodadSet), m_pos(), m_index(0), m_id(def.uniqueId), m_d2(0), m_d3(0)
{
    m_pos = Vec3D(def.position[0], def.position[1], def.position[2]);
    m_rot = Vec3D(def.rotation[0], def.rotation[1], def.rotation[2]);
    m_pos2 = Vec3D(def.boundsMin[0], def.boundsMin[1], def.boundsMin[2]);
    m_pos3 = Vec3D(def.boundsMax[0], def.boundsMax[1], def.boundsMax[2]);
    m_d2 = static_cast<uint32_t>(def.flags) | (static_cast<uint32_t>(def.doodadSet) << 16);
    uint16_t adtId = def.nameSet;

    //-----------add_in _dir_file----------------

    char tempName[512];
    sprintf(tempName, "%s/%s", szWorkDirWmo, wmoInstName);
    FILE* input = fopen(tempName, "r+b");

    if (!input)
    {
        printf("WMOInstance::WMOInstance: couldn't open %s\n", tempName);
        return;
    }

    fseek(input, 8, SEEK_SET); // get the correct no of vertices
    int vertexCount;
    int count = static_cast<int>(fread(&vertexCount, sizeof(int), 1, input));
    fclose(input);

    if (count != 1 || vertexCount == 0)
        return;

    if (m_pos.x == 0 && m_pos.z == 0)
    {
        m_pos.x = 533.33333f * 32;
        m_pos.z = 533.33333f * 32;
    }
    m_pos = fixCoords(m_pos);
    m_pos2 = fixCoords(m_pos2);
    m_pos3 = fixCoords(m_pos3);

    // the CASC clients may scale a map object; nothing before 6.x sets the flag
    float scale = 1.0f;
    if (def.flags & kMapObjDefHasScale)
        scale = def.scale / 1024.0f;
    uint32_t flags = MOD_HAS_BOUND;
    if (tileX == 65 && tileY == 65)
        flags |= MOD_WORLDSPAWN;
    uint32_t nameLength = static_cast<uint32_t>(strlen(wmoInstName));

    //write mapID, tileX, tileY, Flags, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
    {
        // See ModelInstance's constructor for why this needs to be locked:
        // every ADT tile writes to the same dir_bin through its own FILE*.
        std::lock_guard<std::mutex> lock(g_dirFileMutex);
        fwrite(&mapID, sizeof(uint32_t), 1, pDirfile);
        fwrite(&tileX, sizeof(uint32_t), 1, pDirfile);
        fwrite(&tileY, sizeof(uint32_t), 1, pDirfile);
        fwrite(&flags, sizeof(uint32_t), 1, pDirfile);
        fwrite(&adtId, sizeof(uint16_t), 1, pDirfile);
        fwrite(&m_id, sizeof(uint32_t), 1, pDirfile);
        fwrite(&m_pos, sizeof(float), 3, pDirfile);
        fwrite(&m_rot, sizeof(float), 3, pDirfile);
        fwrite(&scale, sizeof(float), 1, pDirfile);
        fwrite(&m_pos2, sizeof(float), 3, pDirfile);
        fwrite(&m_pos3, sizeof(float), 3, pDirfile);
        fwrite(&nameLength, sizeof(uint32_t), 1, pDirfile);
        fwrite(wmoInstName, sizeof(char), nameLength, pDirfile);
        fflush(pDirfile);
    }
}
