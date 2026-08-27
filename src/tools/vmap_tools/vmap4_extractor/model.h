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

#ifndef MODEL_H
#define MODEL_H

#include "mpqlib/FileLoader.hpp"
#include "vec3d.h"
#include "modelheaders.h"
#include <vector>

class MPQFile;

Vec3D fixCoordSystem(Vec3D v);

class Model
{
private:
    void unload()
    {
        delete[] m_vertices;
        delete[] m_indices;
        m_vertices = NULL;
        m_indices = NULL;
    }
    std::string m_filename;
public:
    ModelHeader m_header;
    Vec3D* m_vertices;
    uint16_t* m_indices;

    bool open();
    bool convertToVMapModel(char const* outFileName);

    Model(std::string& filename);
    ~Model() { unload(); }
};

class ModelInstance
{
public:
    uint32_t m_id;
    Vec3D m_pos, m_rot;
    uint16_t m_scale, m_flags;
    float m_sc;

    ModelInstance() : m_id(0), m_scale(0), m_flags(0), m_sc(0.0f) {}
    ModelInstance(MPQFile& f, char const* modelInstName, uint32_t mapID, uint32_t tileX, uint32_t tileY, FILE* pDirfile);

};

#endif  //MODEL_H
