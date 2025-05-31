/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DYNTREE_H
#define _DYNTREE_H

#include <cstdint>
#include <memory>

namespace G3D
{
    class Ray;
    class Vector3;
}

class GameObjectModel;
struct DynTreeImpl;

namespace VMAP
{
    struct AreaAndLiquidData;
}

class DynamicMapTree
{
    std::unique_ptr<DynTreeImpl> impl;

public:

    DynamicMapTree();
    ~DynamicMapTree();

    bool isInLineOfSight(float x1, float y1, float z1, float x2, float y2,
                         float z2, uint32_t phasemask) const;

    bool getIntersectionTime(uint32_t phasemask, const G3D::Ray& ray,
                             const G3D::Vector3& endPos, float& maxDist) const;
    bool getAreaInfo(float x, float y, float& z, uint32_t phasemask, uint32_t& flags, int32_t& adtId, int32_t& rootId, int32_t& groupId) const;
    void getAreaAndLiquidData(float x, float y, float z, uint32_t phasemask, uint8_t reqLiquidType, VMAP::AreaAndLiquidData& data) const;

    bool getObjectHitPos(uint32_t phasemask, const G3D::Vector3& pPos1,
                         const G3D::Vector3& pPos2, G3D::Vector3& pResultHitPos,
                         float pModifyDist) const;

    float getHeight(float x, float y, float z, float maxSearchDist, uint32_t phasemask) const;

    void insert(const GameObjectModel&);
    void remove(const GameObjectModel&);
    bool contains(const GameObjectModel&) const;
    int size() const;

    void balance();
    void update(uint32_t diff);
};

#endif // _DYNTREE_H
