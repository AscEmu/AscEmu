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
#include "model.h"
#include "modelheaders.h"
#include "wmo.h"
#include "mpqlib/MPQFile.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>
#include <span>

extern std::unique_ptr<mpqlib::MpqPatchChain> WorldMpq;

Model::Model(std::string& filename) :
    m_filename(filename), m_nBoundingTriangles(0), m_ofsBoundingTriangles(0),
    m_nBoundingVertices(0), m_ofsBoundingVertices(0), m_vertices(nullptr), m_indices(nullptr)
{
}

bool Model::open()
{
    MPQFile f(*WorldMpq, m_filename.c_str());

    if (f.isEof())
    {
        f.close();
        // Do not show this error on console to avoid confusion, the extractor can continue working even if some models fail to load
        return false;
    }

    unload();

    if (IsPreWotLKModelFormat())
    {
        ModelHeaderLegacy header;
        memcpy(&header, f.getBuffer(), sizeof(header));
        m_nBoundingTriangles = header.nBoundingTriangles;
        m_ofsBoundingTriangles = header.ofsBoundingTriangles;
        m_nBoundingVertices = header.nBoundingVertices;
        m_ofsBoundingVertices = header.ofsBoundingVertices;
    }
    else
    {
        ModelHeaderModern header;
        memcpy(&header, f.getBuffer(), sizeof(header));
        m_nBoundingTriangles = header.nBoundingTriangles;
        m_ofsBoundingTriangles = header.ofsBoundingTriangles;
        m_nBoundingVertices = header.nBoundingVertices;
        m_ofsBoundingVertices = header.ofsBoundingVertices;
    }

    if (m_nBoundingTriangles > 0)
    {
        f.seek(0);
        f.seekRelative(m_ofsBoundingVertices);
        m_vertices = new Vec3D[m_nBoundingVertices];
        f.read(m_vertices, m_nBoundingVertices * 12);
        for (auto& vertex : std::span(m_vertices, m_nBoundingVertices))
            vertex = fixCoordSystem(vertex);
        f.seek(0);
        f.seekRelative(m_ofsBoundingTriangles);
        m_indices = new uint16_t[m_nBoundingTriangles];
        f.read(m_indices, m_nBoundingTriangles * 2);
        f.close();
    }
    else
    {
        f.close();
        return false;
    }
    return true;
}

bool Model::convertToVMapModel(const char* outFileName)
{
    int n[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
    FILE* output = fopen(outFileName, "wb");
    if (!output)
    {
        printf("Can't create the output file '%s'\n", outFileName);
        return false;
    }
    fwrite(szRawVMAPMagic, 8, 1, output);
    uint32_t vertexCount = m_nBoundingVertices;
    fwrite(&vertexCount, sizeof(int), 1, output);
    uint32_t groupCount = 1;
    fwrite(&groupCount, sizeof(uint32_t), 1, output);
    fwrite(n, 4 * 3, 1, output);// rootwmoid, flags, groupid
    fwrite(n, sizeof(float), 3 * 2, output);//bbox, only needed for WMO currently
    fwrite(n, 4, 1, output);// liquidflags
    fwrite("GRP ", 4, 1, output);
    uint32_t branches = 1;
    int wsize;
    wsize = sizeof(branches) + sizeof(uint32_t) * branches;
    fwrite(&wsize, sizeof(int), 1, output);
    fwrite(&branches, sizeof(branches), 1, output);
    uint32_t indexCount = m_nBoundingTriangles;
    fwrite(&indexCount, sizeof(uint32_t), 1, output);
    fwrite("INDX", 4, 1, output);
    wsize = sizeof(uint32_t) + sizeof(unsigned short) * indexCount;
    fwrite(&wsize, sizeof(int), 1, output);
    fwrite(&indexCount, sizeof(uint32_t), 1, output);
    if (indexCount > 0)
    {
        // Flip the winding order (2nd/3rd index) of each triangle
        for (uint32_t t = 0; t + 2 < indexCount; t += 3)
            std::swap(m_indices[t + 1], m_indices[t + 2]);

        fwrite(m_indices, sizeof(unsigned short), indexCount, output);
    }

    fwrite("VERT", 4, 1, output);
    wsize = sizeof(int) + sizeof(float) * 3 * vertexCount;
    fwrite(&wsize, sizeof(int), 1, output);
    fwrite(&vertexCount, sizeof(int), 1, output);
    if (vertexCount > 0)
    {
        for (auto& vertex : std::span(m_vertices, vertexCount))
        {
            float tmp = vertex.y;
            vertex.y = -vertex.z;
            vertex.z = tmp;
        }

        fwrite(m_vertices, sizeof(float) * 3, vertexCount, output);
    }

    fclose(output);

    return true;
}


Vec3D fixCoordSystem(Vec3D v)
{
    return Vec3D(v.x, v.z, -v.y);
}

Vec3D fixCoordSystem2(Vec3D v)
{
    return Vec3D(v.x, v.z, v.y);
}

ModelInstance::ModelInstance(MPQFile& f, char const* modelInstName, uint32_t mapID, uint32_t tileX, uint32_t tileY, FILE* pDirfile) :
    m_id(0), m_scale(0), m_flags(0)
{
    float ff[3];
    f.read(&m_id, 4);
    f.read(ff, 12);
    m_pos = fixCoords(Vec3D(ff[0], ff[1], ff[2]));
    f.read(ff, 12);
    m_rot = Vec3D(ff[0], ff[1], ff[2]);
    f.read(&m_scale, 2);
    f.read(&m_flags, 2);
    // scale factor - divide by 1024. blizzard devs must be on crack, why not just use a float?
    m_sc = m_scale / 1024.0f;

    char tempName[512];
    sprintf(tempName, "%s/%s", szWorkDirWmo, modelInstName);
    FILE* input = fopen(tempName, "r+b");

    if (!input)
        return;

    fseek(input, 8, SEEK_SET); // get the correct no of vertices
    int vertexCount;
    int count = static_cast<int>(fread(&vertexCount, sizeof(int), 1, input));
    fclose(input);

    if (count != 1 || vertexCount == 0)
        return;

    uint16_t adtId = 0;// not used for models
    uint32_t modFlags = MOD_M2;
    if (tileX == 65 && tileY == 65)
        modFlags |= MOD_WORLDSPAWN;

    uint32_t nameLength = static_cast<uint32_t>(strlen(modelInstName));

    //write mapID, tileX, tileY, Flags, ID, Pos, Rot, Scale, name
    {
        // Parallel ADT tiles each hold their own FILE* onto the same shared
        // dir_bin, so this whole record must go out (and be flushed to the
        // OS) as one unit before another tile's writer can touch the file.
        std::lock_guard<std::mutex> lock(g_dirFileMutex);
        fwrite(&mapID, sizeof(uint32_t), 1, pDirfile);
        fwrite(&tileX, sizeof(uint32_t), 1, pDirfile);
        fwrite(&tileY, sizeof(uint32_t), 1, pDirfile);
        fwrite(&modFlags, sizeof(uint32_t), 1, pDirfile);
        fwrite(&adtId, sizeof(uint16_t), 1, pDirfile);
        fwrite(&m_id, sizeof(uint32_t), 1, pDirfile);
        fwrite(&m_pos, sizeof(float), 3, pDirfile);
        fwrite(&m_rot, sizeof(float), 3, pDirfile);
        fwrite(&m_sc, sizeof(float), 1, pDirfile);
        fwrite(&nameLength, sizeof(uint32_t), 1, pDirfile);
        fwrite(modelInstName, sizeof(char), nameLength, pDirfile);
        fflush(pDirfile);
    }
}
