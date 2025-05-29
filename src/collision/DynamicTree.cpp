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

#include "DynamicTree.h"
#include "BoundingIntervalHierarchyWrapper.h"
#include "Logging/Logger.hpp"
#include "RegularGrid.h"
#include "Utilities/Util.hpp"
#include "GameObjectModel.h"
#include "MapTree.h"
#include "ModelInstance.h"
#include "VMapFactory.h"
#include "VMapManager2.h"
#include "WorldModel.h"
#include <G3D/AABox.h>
#include <G3D/Ray.h>
#include <G3D/Vector3.h>

#include "Utilities/TimeTracker.hpp"
#include <memory>

using VMAP::ModelInstance;

namespace {

int CHECK_TREE_PERIOD = 200;

} // namespace

template<> struct HashTrait< GameObjectModel>{
    static size_t hashCode(const GameObjectModel& g) { return (size_t)(void*)&g; }
};

template<> struct PositionTrait< GameObjectModel> {
    static void getPosition(const GameObjectModel& g, G3D::Vector3& p) { p = g.getPosition(); }
};

template<> struct BoundsTrait< GameObjectModel> {
    static void getBounds(const GameObjectModel& g, G3D::AABox& out) { out = g.getBounds();}
    static void getBounds2(const GameObjectModel* g, G3D::AABox& out) { out = g->getBounds();}
};

/*
static bool operator == (const GameObjectModel& mdl, const GameObjectModel& mdl2){
    return &mdl == &mdl2;
}
*/

typedef RegularGrid2D<GameObjectModel, BIHWrap<GameObjectModel> > ParentTree;

struct DynTreeImpl : public ParentTree/*, public Intersectable*/
{
    typedef GameObjectModel Model;
    typedef ParentTree base;

    DynTreeImpl() :
        rebalance_timer(std::make_unique<Util::SmallTimeTracker>(CHECK_TREE_PERIOD)),
        unbalanced_times(0)
    {
    }

    void insert(const Model& mdl)
    {
        base::insert(mdl);
        ++unbalanced_times;
    }

    void remove(const Model& mdl)
    {
        base::remove(mdl);
        ++unbalanced_times;
    }

    void balance()
    {
        base::balance();
        unbalanced_times = 0;
    }

    void update(uint32_t difftime)
    {
        if (!size())
            return;

        rebalance_timer->updateTimer(difftime);
        if (rebalance_timer->isTimePassed())
        {
            rebalance_timer->resetInterval(CHECK_TREE_PERIOD);
            if (unbalanced_times > 0)
                balance();
        }
    }

    std::unique_ptr<Util::SmallTimeTracker> rebalance_timer;
    int unbalanced_times;
};

DynamicMapTree::DynamicMapTree() : impl(std::make_unique<DynTreeImpl>()) { }

DynamicMapTree::~DynamicMapTree() = default;

void DynamicMapTree::insert(const GameObjectModel& mdl)
{
    impl->insert(mdl);
}

void DynamicMapTree::remove(const GameObjectModel& mdl)
{
    impl->remove(mdl);
}

bool DynamicMapTree::contains(const GameObjectModel& mdl) const
{
    return impl->contains(mdl);
}

void DynamicMapTree::balance()
{
    impl->balance();
}

int DynamicMapTree::size() const
{
    return impl->size();
}

void DynamicMapTree::update(uint32_t t_diff)
{
    impl->update(t_diff);
}

struct DynamicTreeIntersectionCallback
{
    bool did_hit;
    uint32_t phase_mask;
    DynamicTreeIntersectionCallback(uint32_t phasemask) : did_hit(false), phase_mask(phasemask) { }
    bool operator()(const G3D::Ray& r, const GameObjectModel& obj, float& distance)
    {
        did_hit = obj.intersectRay(r, distance, true, phase_mask);
        return did_hit;
    }
    bool didHit() const { return did_hit;}
};

struct DynamicTreeIntersectionCallback_WithLogger
{
    bool did_hit;
    uint32_t phase_mask;
    DynamicTreeIntersectionCallback_WithLogger(uint32_t phasemask) : did_hit(false), phase_mask(phasemask)
    {
        sLogger.debug("DynamicTreeIntersectionCallback_WithLogger : Dynamic Intersection log");
    }
    bool operator()(const G3D::Ray& r, const GameObjectModel& obj, float& distance)
    {
        sLogger.debug("DynamicTreeIntersectionCallback_WithLogger : testing intersection with {}", obj.name);
        bool hit = obj.intersectRay(r, distance, true, phase_mask);
        if (hit)
        {
            did_hit = true;
            sLogger.debug("DynamicTreeIntersectionCallback_WithLogger : result: intersects");
        }
        return hit;
    }
    bool didHit() const { return did_hit;}
};

struct DynamicTreeAreaInfoCallback
{
    DynamicTreeAreaInfoCallback(uint32_t phaseMask) : _phaseMask(phaseMask) {}

    void operator()(G3D::Vector3 const& p, GameObjectModel const& obj)
    {
        obj.intersectPoint(p, _areaInfo, _phaseMask);
    }

    VMAP::AreaInfo const& GetAreaInfo() const { return _areaInfo; }

private:
    uint32_t _phaseMask;
    VMAP::AreaInfo _areaInfo;
};

struct DynamicTreeLocationInfoCallback
{
    DynamicTreeLocationInfoCallback(uint32_t phaseMask) : _phaseMask(phaseMask), _hitModel(nullptr) {}

    void operator()(G3D::Vector3 const& p, GameObjectModel const& obj)
    {
        if (obj.getLocationInfo(p, _locationInfo, _phaseMask))
            _hitModel = &obj;
    }

    VMAP::LocationInfo& GetLocationInfo() { return _locationInfo; }
    GameObjectModel const* GetHitModel() const { return _hitModel; }

private:
    uint32_t _phaseMask;
    VMAP::LocationInfo _locationInfo;
    GameObjectModel const* _hitModel;
};

bool DynamicMapTree::getIntersectionTime(const uint32_t phasemask, const G3D::Ray& ray,
                                         const G3D::Vector3& endPos, float& maxDist) const
{
    float distance = maxDist;
    DynamicTreeIntersectionCallback callback(phasemask);
    impl->intersectRay(ray, callback, distance, endPos);
    if (callback.didHit())
        maxDist = distance;
    return callback.didHit();
}

bool DynamicMapTree::getObjectHitPos(const uint32_t phasemask, const G3D::Vector3& startPos,
                                     const G3D::Vector3& endPos, G3D::Vector3& resultHit,
                                     float modifyDist) const
{
    bool result = false;
    float maxDist = (endPos - startPos).magnitude();
    // valid map coords should *never ever* produce float overflow, but this would produce NaNs too
    ASSERT(maxDist < std::numeric_limits<float>::max());
    // prevent NaN values which can cause BIH intersection to enter infinite loop
    if (maxDist < 1e-10f)
    {
        resultHit = endPos;
        return false;
    }
    G3D::Vector3 dir = (endPos - startPos)/maxDist;              // direction with length of 1
    G3D::Ray ray(startPos, dir);
    float dist = maxDist;
    if (getIntersectionTime(phasemask, ray, endPos, dist))
    {
        resultHit = startPos + dir * dist;
        if (modifyDist < 0)
        {
            if ((resultHit - startPos).magnitude() > -modifyDist)
                resultHit = resultHit + dir*modifyDist;
            else
                resultHit = startPos;
        }
        else
            resultHit = resultHit + dir*modifyDist;

        result = true;
    }
    else
    {
        resultHit = endPos;
        result = false;
    }
    return result;
}

bool DynamicMapTree::isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t phasemask) const
{
    G3D::Vector3 v1(x1, y1, z1), v2(x2, y2, z2);

    float maxDist = (v2 - v1).magnitude();

    if (!G3D::fuzzyGt(maxDist, 0) )
        return true;

    G3D::Ray r(v1, (v2-v1) / maxDist);
    DynamicTreeIntersectionCallback callback(phasemask);
    impl->intersectRay(r, callback, maxDist, v2);

    return !callback.did_hit;
}

float DynamicMapTree::getHeight(float x, float y, float z, float maxSearchDist, uint32_t phasemask) const
{
    G3D::Vector3 v(x, y, z);
    G3D::Ray r(v, G3D::Vector3(0, 0, -1));
    DynamicTreeIntersectionCallback callback(phasemask);
    impl->intersectZAllignedRay(r, callback, maxSearchDist);

    if (callback.didHit())
        return v.z - maxSearchDist;
    else
        return -G3D::finf();
}

bool DynamicMapTree::getAreaInfo(float x, float y, float& z, uint32_t phasemask, uint32_t& flags, int32_t& adtId, int32_t& rootId, int32_t& groupId) const
{
    G3D::Vector3 v(x, y, z + 0.5f);
    DynamicTreeAreaInfoCallback intersectionCallBack(phasemask);
    impl->intersectPoint(v, intersectionCallBack);
    if (intersectionCallBack.GetAreaInfo().result)
    {
        flags = intersectionCallBack.GetAreaInfo().flags;
        adtId = intersectionCallBack.GetAreaInfo().adtId;
        rootId = intersectionCallBack.GetAreaInfo().rootId;
        groupId = intersectionCallBack.GetAreaInfo().groupId;
        z = intersectionCallBack.GetAreaInfo().ground_Z;
        return true;
    }
    return false;
}

void DynamicMapTree::getAreaAndLiquidData(float x, float y, float z, uint32_t phasemask, uint8_t reqLiquidType, VMAP::AreaAndLiquidData& data) const
{
    G3D::Vector3 v(x, y, z + 0.5f);
    DynamicTreeLocationInfoCallback intersectionCallBack(phasemask);
    impl->intersectPoint(v, intersectionCallBack);
    if (intersectionCallBack.GetLocationInfo().hitModel)
    {
        data.floorZ = intersectionCallBack.GetLocationInfo().ground_Z;
        uint32_t liquidType = intersectionCallBack.GetLocationInfo().hitModel->GetLiquidType();
        float liquidLevel;
        if (!reqLiquidType || VMAP::VMapFactory::createOrGetVMapManager()->GetLiquidFlagsPtr(liquidType) & reqLiquidType)
            if (intersectionCallBack.GetHitModel()->getLiquidLevel(v, intersectionCallBack.GetLocationInfo(), liquidLevel))
                data.liquidInfo.emplace(liquidType, liquidLevel);

        data.areaInfo.emplace(0,
            intersectionCallBack.GetLocationInfo().rootId,
            intersectionCallBack.GetLocationInfo().hitModel->GetWmoID(),
            intersectionCallBack.GetLocationInfo().hitModel->GetMogpFlags());
    }
}
