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

#include "VMapFactory.h"
#include "VMapManager2.h"
#include "VMapDefinitions.h"
#include "WorldModel.h"
#include "GameObjectModel.h"
#include "Logging/Logger.hpp"
#include "Utilities/Util.hpp"
#include "MapTree.h"

using G3D::Vector3;
using G3D::Ray;
using G3D::AABox;

struct GameobjectModelData
{
    GameobjectModelData(const std::string& name_, const AABox& box, bool isWmo_) :
        bound(box), name(name_), isWmo(isWmo_) { }

    AABox bound;
    std::string name;
    bool isWmo;
};

typedef std::unordered_map<uint32_t, GameobjectModelData> ModelList;
ModelList model_list;

void LoadGameObjectModelList(std::string const& dataPath)
{
#ifndef NO_CORE_FUNCS
    auto startTime = Util::TimeNow();
#endif

    FILE* model_list_file = fopen((dataPath + "/" + VMAP::GAMEOBJECT_MODELS).c_str(), "rb");
    if (!model_list_file)
    {
        sLogger.failure("Unable to open '{}' file.", VMAP::GAMEOBJECT_MODELS);
        return;
    }

    uint32_t name_length, displayId;
    uint8_t isWmo;
    char buff[500];
    while (true)
    {
        Vector3 v1, v2;
        if (fread(&displayId, sizeof(uint32_t), 1, model_list_file) != 1)
            if (feof(model_list_file))  // EOF flag is only set after failed reading attempt
                break;

        if (fread(&isWmo, sizeof(uint8_t), 1, model_list_file) != 1
            || fread(&name_length, sizeof(uint32_t), 1, model_list_file) != 1
            || name_length >= sizeof(buff)
            || fread(&buff, sizeof(char), name_length, model_list_file) != name_length
            || fread(&v1, sizeof(Vector3), 1, model_list_file) != 1
            || fread(&v2, sizeof(Vector3), 1, model_list_file) != 1)
        {
            sLogger.failure("File '{}' seems to be corrupted!", VMAP::GAMEOBJECT_MODELS);
            break;
        }

        if (v1.isNaN() || v2.isNaN())
        {
            sLogger.failure("File '{}' Model '{}' has invalid v1{} v2{} values!", VMAP::GAMEOBJECT_MODELS, std::string(buff, name_length), v1.toString(), v2.toString());
            continue;
        }

        model_list.insert
        (
            ModelList::value_type(displayId, GameobjectModelData(std::string(buff, name_length), AABox(v1, v2), isWmo != 0))
        );
    }

    fclose(model_list_file);
    sLogger.info("LoadGameObjectModelList : Loaded {} GameObject models in {} ms", uint32_t(model_list.size()), static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

GameObjectModel::~GameObjectModel()
{
    if (iModel)
        ((VMAP::VMapManager2*)VMAP::VMapFactory::createOrGetVMapManager())->releaseModelInstance(name);
}

bool GameObjectModel::initialize(std::unique_ptr<GameObjectModelOwnerBase> modelOwner, std::string const& dataPath)
{
    ModelList::const_iterator it = model_list.find(modelOwner->GetDisplayId());
    if (it == model_list.end())
        return false;

    G3D::AABox mdl_box(it->second.bound);
    // ignore models with no bounds
    if (mdl_box == G3D::AABox::zero())
    {
        sLogger.failure("GameObject model {} has zero bounds, loading skipped", it->second.name);
        return false;
    }

    iModel = ((VMAP::VMapManager2*)VMAP::VMapFactory::createOrGetVMapManager())->acquireModelInstance(dataPath + "vmaps/", it->second.name);

    if (!iModel)
        return false;

    name = it->second.name;
    iPos = modelOwner->GetPosition();
    phasemask = modelOwner->GetPhaseMask();
    iScale = modelOwner->GetScale();
    iInvScale = 1.f / iScale;

    G3D::Matrix3 iRotation = G3D::Matrix3::fromEulerAnglesZYX(modelOwner->GetOrientation(), 0, 0);
    iInvRot = iRotation.inverse();
    // transform bounding box:
    mdl_box = AABox(mdl_box.low() * iScale, mdl_box.high() * iScale);
    AABox rotated_bounds;
    for (int i = 0; i < 8; ++i)
        rotated_bounds.merge(iRotation * mdl_box.corner(i));

    iBound = rotated_bounds + iPos;
#ifdef SPAWN_CORNERS
    // test:
    for (int i = 0; i < 8; ++i)
    {
        Vector3 pos(iBound.corner(i));
        modelOwner->DebugVisualizeCorner(pos);
    }
#endif

    owner = std::move(modelOwner);
    isWmo = it->second.isWmo;
    return true;
}

std::unique_ptr<GameObjectModel> GameObjectModel::Create(std::unique_ptr<GameObjectModelOwnerBase> modelOwner, std::string const& dataPath)
{
    auto mdl = std::unique_ptr<GameObjectModel>(new GameObjectModel());
    if (!mdl->initialize(std::move(modelOwner), dataPath))
    {
        return nullptr;
    }

    return mdl;
}

bool GameObjectModel::intersectRay(const G3D::Ray& ray, float& MaxDist, bool StopAtFirstHit, uint32_t ph_mask) const
{
    if (!(phasemask & ph_mask) || !owner->IsSpawned())
        return false;

    float time = ray.intersectionTime(iBound);
    if (time == G3D::finf())
        return false;

    // child bounds are defined in object space:
    Vector3 p = iInvRot * (ray.origin() - iPos) * iInvScale;
    Ray modRay(p, iInvRot * ray.direction());
    float distance = MaxDist * iInvScale;
    bool hit = iModel->IntersectRay(modRay, distance, StopAtFirstHit);
    if (hit)
    {
        distance *= iScale;
        MaxDist = distance;
    }
    return hit;
}

void GameObjectModel::intersectPoint(G3D::Vector3 const& point, VMAP::AreaInfo& info, uint32_t ph_mask) const
{
    if (!(phasemask & ph_mask) || !owner->IsSpawned() || !isMapObject())
        return;

    if (!iBound.contains(point))
        return;

    // child bounds are defined in object space:
    Vector3 pModel = iInvRot * (point - iPos) * iInvScale;
    Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
    float zDist;
    if (iModel->IntersectPoint(pModel, zDirModel, zDist, info))
    {
        Vector3 modelGround = pModel + zDist * zDirModel;
        float world_Z = ((modelGround * iInvRot) * iScale + iPos).z;
        if (info.ground_Z < world_Z)
            info.ground_Z = world_Z;
    }
}

bool GameObjectModel::getLocationInfo(G3D::Vector3 const& point, VMAP::LocationInfo& info, uint32_t ph_mask) const
{
    if (!(phasemask & ph_mask) || !owner->IsSpawned() || !isMapObject())
        return false;

    if (!iBound.contains(point))
        return false;

    // child bounds are defined in object space:
    Vector3 pModel = iInvRot * (point - iPos) * iInvScale;
    Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
    float zDist;
    if (iModel->GetLocationInfo(pModel, zDirModel, zDist, info))
    {
        Vector3 modelGround = pModel + zDist * zDirModel;
        float world_Z = ((modelGround * iInvRot) * iScale + iPos).z;
        if (info.ground_Z < world_Z)
        {
            info.ground_Z = world_Z;
            return true;
        }
    }

    return false;
}

bool GameObjectModel::getLiquidLevel(G3D::Vector3 const& point, VMAP::LocationInfo& info, float& liqHeight) const
{
    // child bounds are defined in object space:
    Vector3 pModel = iInvRot * (point - iPos) * iInvScale;
    //Vector3 zDirModel = iInvRot * Vector3(0.f, 0.f, -1.f);
    float zDist;
    if (info.hitModel->GetLiquidLevel(pModel, zDist))
    {
        // calculate world height (zDist in model coords):
        // assume WMO not tilted (wouldn't make much sense anyway)
        liqHeight = zDist * iScale + iPos.z;
        return true;
    }
    return false;
}

bool GameObjectModel::UpdatePosition()
{
    if (!iModel)
        return false;

    ModelList::const_iterator it = model_list.find(owner->GetDisplayId());
    if (it == model_list.end())
        return false;

    G3D::AABox mdl_box(it->second.bound);
    // ignore models with no bounds
    if (mdl_box == G3D::AABox::zero())
    {
        sLogger.failure("GameObject model {} has zero bounds, loading skipped", it->second.name);
        return false;
    }

    iPos = owner->GetPosition();

    G3D::Matrix3 iRotation = G3D::Matrix3::fromEulerAnglesZYX(owner->GetOrientation(), 0, 0);
    iInvRot = iRotation.inverse();
    // transform bounding box:
    mdl_box = AABox(mdl_box.low() * iScale, mdl_box.high() * iScale);
    AABox rotated_bounds;
    for (int i = 0; i < 8; ++i)
        rotated_bounds.merge(iRotation * mdl_box.corner(i));

    iBound = rotated_bounds + iPos;
#ifdef SPAWN_CORNERS
    // test:
    for (int i = 0; i < 8; ++i)
    {
        Vector3 pos(iBound.corner(i));
        owner->DebugVisualizeCorner(pos);
    }
#endif

    return true;
}
