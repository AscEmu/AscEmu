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

#include "VMapManager2.h"
#include "MapTree.h"
#include "ModelInstance.h"
#include "WorldModel.h"
#include <G3D/Vector3.h>
#include "Logging/Logger.hpp"
#include "Debugging/Errors.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

using G3D::Vector3;

namespace VMAP
{
    ManagedModel::ManagedModel() : iModel(nullptr), iRefCount(0)
    {}

    void ManagedModel::setModel(std::unique_ptr<WorldModel> model)
    {
        iModel = std::move(model);
    }

    VMapManager2::VMapManager2()
    {
        GetLiquidFlagsPtr = &GetLiquidFlagsDummy;
        IsVMAPDisabledForPtr = &IsVMAPDisabledForDummy;
        thread_safe_environment = true;
    }

    VMapManager2::~VMapManager2(void) = default;

    void VMapManager2::InitializeThreadUnsafe(const std::vector<uint32_t>& mapIds)
    {
        // the caller must pass the list of all mapIds that will be used in the VMapManager2 lifetime
        for (const uint32_t& mapId : mapIds)
            iInstanceMapTrees.insert(InstanceTreeMap::value_type(mapId, nullptr));

        thread_safe_environment = false;
    }

    Vector3 VMapManager2::convertPositionToInternalRep(float x, float y, float z) const
    {
        Vector3 pos;
        const float mid = 0.5f * 64.0f * 533.33333333f;
        pos.x = mid - x;
        pos.y = mid - y;
        pos.z = z;

        return pos;
    }

    // move to MapTree too?
    std::string VMapManager2::getMapFileName(unsigned int mapId)
    {
        std::stringstream fname;
        fname.width(4);
        fname << std::setfill('0') << mapId << std::string(MAP_FILENAME_EXTENSION2);

        return fname.str();
    }

    int VMapManager2::loadMap(const char* basePath, unsigned int mapId, int x, int y)
    {
        int result = VMAP_LOAD_RESULT_IGNORED;
        if (isMapLoadingEnabled())
        {
            if (_loadMap(mapId, basePath, x, y))
                result = VMAP_LOAD_RESULT_OK;
            else
                result = VMAP_LOAD_RESULT_ERROR;
        }

        return result;
    }

    InstanceTreeMap::const_iterator VMapManager2::GetMapTree(uint32_t mapId) const
    {
        // return the iterator if found or end() if not found/NULL
        InstanceTreeMap::const_iterator itr = iInstanceMapTrees.find(mapId);
        if (itr != iInstanceMapTrees.cend() && !itr->second)
            itr = iInstanceMapTrees.cend();

        return itr;
    }

    // load one tile (internal use only)
    bool VMapManager2::_loadMap(uint32_t mapId, const std::string& basePath, uint32_t tileX, uint32_t tileY)
    {
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree == iInstanceMapTrees.end())
        {
            if (thread_safe_environment)
                instanceTree = iInstanceMapTrees.insert(InstanceTreeMap::value_type(mapId, nullptr)).first;
            else
            {
                sLogger.failure("Invalid mapId {} tile [{}, {}] passed to VMapManager2 after startup in thread unsafe environment", mapId, tileX, tileY);
                ASSERT(false);
            }
        }

        if (!instanceTree->second)
        {
            std::string mapFileName = getMapFileName(mapId);
            auto newTree = std::make_unique<StaticMapTree>(mapId, basePath);
            if (!newTree->InitMap(mapFileName, this))
            {
                return false;
            }
            instanceTree->second = std::move(newTree);
        }

        return instanceTree->second->LoadMapTile(tileX, tileY, this);
    }

    void VMapManager2::unloadMap(unsigned int mapId)
    {
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree != iInstanceMapTrees.end() && instanceTree->second)
        {
            instanceTree->second->UnloadMap(this);
            if (instanceTree->second->numLoadedTiles() == 0)
            {
                instanceTree->second = nullptr;
            }
        }
    }

    void VMapManager2::unloadMap(unsigned int mapId, int x, int y)
    {
        InstanceTreeMap::iterator instanceTree = iInstanceMapTrees.find(mapId);
        if (instanceTree != iInstanceMapTrees.end() && instanceTree->second)
        {
            instanceTree->second->UnloadMapTile(x, y, this);
            if (instanceTree->second->numLoadedTiles() == 0)
            {
                instanceTree->second = nullptr;
            }
        }
    }

    bool VMapManager2::isInLineOfSight(unsigned int mapId, float x1, float y1, float z1, float x2, float y2, float z2)
    {
        if (!isLineOfSightCalcEnabled() || IsVMAPDisabledForPtr(mapId, VMAP_DISABLE_LOS))
            return true;

        InstanceTreeMap::const_iterator instanceTree = GetMapTree(mapId);
        if (instanceTree != iInstanceMapTrees.end())
        {
            Vector3 pos1 = convertPositionToInternalRep(x1, y1, z1);
            Vector3 pos2 = convertPositionToInternalRep(x2, y2, z2);
            if (pos1 != pos2)
            {
                return instanceTree->second->isInLineOfSight(pos1, pos2);
            }
        }

        return true;
    }

    /**
    get the hit position and return true if we hit something
    otherwise the result pos will be the dest pos
    */
    bool VMapManager2::getObjectHitPos(unsigned int mapId, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float &ry, float& rz, float modifyDist)
    {
        if (isLineOfSightCalcEnabled() && !IsVMAPDisabledForPtr(mapId, VMAP_DISABLE_LOS))
        {
            InstanceTreeMap::const_iterator instanceTree = GetMapTree(mapId);
            if (instanceTree != iInstanceMapTrees.end())
            {
                Vector3 pos1 = convertPositionToInternalRep(x1, y1, z1);
                Vector3 pos2 = convertPositionToInternalRep(x2, y2, z2);
                Vector3 resultPos;
                bool result = instanceTree->second->getObjectHitPos(pos1, pos2, resultPos, modifyDist);
                resultPos = convertPositionToInternalRep(resultPos.x, resultPos.y, resultPos.z);
                rx = resultPos.x;
                ry = resultPos.y;
                rz = resultPos.z;
                return result;
            }
        }

        rx = x2;
        ry = y2;
        rz = z2;

        return false;
    }

    /**
    get height or INVALID_HEIGHT if no height available
    */

    float VMapManager2::getHeight(unsigned int mapId, float x, float y, float z, float maxSearchDist)
    {
        if (isHeightCalcEnabled() && !IsVMAPDisabledForPtr(mapId, VMAP_DISABLE_HEIGHT))
        {
            InstanceTreeMap::const_iterator instanceTree = GetMapTree(mapId);
            if (instanceTree != iInstanceMapTrees.end())
            {
                Vector3 pos = convertPositionToInternalRep(x, y, z);
                float height = instanceTree->second->getHeight(pos, maxSearchDist);
                if (!(height < G3D::finf()))
                    return height = VMAP_INVALID_HEIGHT_VALUE; // No height

                return height;
            }
        }

        return VMAP_INVALID_HEIGHT_VALUE;
    }

    bool VMapManager2::getAreaInfo(unsigned int mapId, float x, float y, float& z, uint32_t& flags, int32_t& adtId, int32_t& rootId, int32_t& groupId) const
    {
        if (!IsVMAPDisabledForPtr(mapId, VMAP_DISABLE_AREAFLAG))
        {
            InstanceTreeMap::const_iterator instanceTree = GetMapTree(mapId);
            if (instanceTree != iInstanceMapTrees.end())
            {
                Vector3 pos = convertPositionToInternalRep(x, y, z);
                bool result = instanceTree->second->getAreaInfo(pos, flags, adtId, rootId, groupId);
                // z is not touched by convertPositionToInternalRep(), so just copy
                z = pos.z;
                return result;
            }
        }

        return false;
    }

    bool VMapManager2::getLiquidLevel(uint32_t mapId, float x, float y, float z, uint8_t reqLiquidType, float& level, float& floor, uint32_t& type, uint32_t& mogpFlags) const
    {
        if (!IsVMAPDisabledForPtr(mapId, VMAP_DISABLE_LIQUIDSTATUS))
        {
            InstanceTreeMap::const_iterator instanceTree = GetMapTree(mapId);
            if (instanceTree != iInstanceMapTrees.end())
            {
                LocationInfo info;
                Vector3 pos = convertPositionToInternalRep(x, y, z);
                if (instanceTree->second->GetLocationInfo(pos, info))
                {
                    floor = info.ground_Z;
                    ASSERT(floor < std::numeric_limits<float>::max());
                    ASSERT(info.hitModel);
                    if (info.hitModel)
                    {
                        type = info.hitModel->GetLiquidType();  // entry from LiquidType.dbc
                        mogpFlags = info.hitModel->GetMogpFlags();
                    }
                    if (reqLiquidType && !(GetLiquidFlagsPtr(type) & reqLiquidType))
                        return false;
                    ASSERT(info.hitInstance);
                    if (info.hitInstance && info.hitInstance->GetLiquidLevel(pos, info, level))
                        return true;
                }
            }
        }

        return false;
    }

    void VMapManager2::getAreaAndLiquidData(unsigned int mapId, float x, float y, float z, uint8_t reqLiquidType, AreaAndLiquidData& data) const
    {
        if (IsVMAPDisabledForPtr(mapId, VMAP_DISABLE_LIQUIDSTATUS))
        {
            data.floorZ = z;
            int32_t adtId, rootId, groupId;
            uint32_t flags;
            if (getAreaInfo(mapId, x, y, data.floorZ, flags, adtId, rootId, groupId))
                data.areaInfo.emplace(adtId, rootId, groupId, flags);
            return;
        }
        InstanceTreeMap::const_iterator instanceTree = GetMapTree(mapId);
        if (instanceTree != iInstanceMapTrees.end())
        {
            LocationInfo info;
            Vector3 pos = convertPositionToInternalRep(x, y, z);
            if (instanceTree->second->GetLocationInfo(pos, info))
            {
                ASSERT(info.hitModel);
                ASSERT(info.hitInstance);
                data.floorZ = info.ground_Z;
                uint32_t liquidType = info.hitModel->GetLiquidType();
                float liquidLevel;
                if (!reqLiquidType || (GetLiquidFlagsPtr(liquidType) & reqLiquidType))
                    if (info.hitInstance->GetLiquidLevel(pos, info, liquidLevel))
                        data.liquidInfo.emplace(liquidType, liquidLevel);

                if (!IsVMAPDisabledForPtr(mapId, VMAP_DISABLE_AREAFLAG))
                    data.areaInfo.emplace(info.hitInstance->adtId, info.rootId, info.hitModel->GetWmoID(), info.hitModel->GetMogpFlags());
            }
        }
    }

    WorldModel* VMapManager2::acquireModelInstance(const std::string& basepath, const std::string& filename)
    {
        //! Critical section, thread safe access to iLoadedModelFiles
        std::lock_guard<std::mutex> lock(LoadedModelFilesLock);

        ModelFileMap::iterator model = iLoadedModelFiles.find(filename);
        if (model == iLoadedModelFiles.end())
        {
            auto worldmodel = std::make_unique<WorldModel>();
            if (!worldmodel->readFile(basepath + filename + ".vmo"))
            {
                sLogger.failure("could not load '{}{}.vmo'", basepath, filename);
                return nullptr;
            }
            sLogger.debug("VMapManager2 loading file '{}{}'", basepath, filename);
            model = iLoadedModelFiles.insert(std::pair<std::string, ManagedModel>(filename, ManagedModel())).first;
            model->second.setModel(std::move(worldmodel));
        }
        model->second.incRefCount();
        return model->second.getModel();
    }

    void VMapManager2::releaseModelInstance(const std::string &filename)
    {
        //! Critical section, thread safe access to iLoadedModelFiles
        std::lock_guard<std::mutex> lock(LoadedModelFilesLock);

        ModelFileMap::iterator model = iLoadedModelFiles.find(filename);
        if (model == iLoadedModelFiles.end())
        {
            sLogger.failure("trying to unload non-loaded file '{}'", filename);
            return;
        }
        if (model->second.decRefCount() == 0)
        {
            sLogger.debug("VMapManager2 unloading file '{}'", filename);
            iLoadedModelFiles.erase(model);
        }
    }

    bool VMapManager2::existsMap(const char* basePath, unsigned int mapId, int x, int y)
    {
        return StaticMapTree::CanLoadMap(std::string(basePath), mapId, x, y);
    }

    InstanceTreeMap const& VMapManager2::getInstanceMapTree() const
    {
        return iInstanceMapTrees;
    }

} // namespace VMAP
