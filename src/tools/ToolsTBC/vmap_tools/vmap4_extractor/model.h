/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#ifndef MODEL_H
#define MODEL_H

#include "loadlib.h"
#include "vec3d.h"
#include "modelheaders.h"
#include <vector>
#include "vmapexport.h"

class MPQFile;

Vec3D fixCoordSystem(Vec3D v);

class Model
{
private:
    void _unload()
    {
        delete[] vertices;
        delete[] indices;
        vertices = NULL;
        indices = NULL;
    }
    std::string filename;
public:
    ModelHeader header;
    ModelBoundingVertex* boundingVertices;
    Vec3D* vertices;
    uint16* indices;
    size_t nIndices;

    bool open(StringSet& failedPaths);
    bool ConvertToVMAPModel(char const* outfilename);

    Model(std::string& filename);
    ~Model() { _unload(); }
};

class ModelInstance
{
public:
    uint32 id;
    Vec3D pos, rot;
    uint16 scale, flags;
    float sc;

    ModelInstance() : id(0), scale(0), flags(0), sc(0.0f) {}
    ModelInstance(MPQFile& f, char const* ModelInstName, uint32 mapID, uint32 tileX, uint32 tileY, FILE* pDirfile);

};

#endif  //MODEL_H
