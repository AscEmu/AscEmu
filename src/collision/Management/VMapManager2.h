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

#ifndef _VMAPMANAGER2_H
#define _VMAPMANAGER2_H

#include "IVMapManager.h"
#include <mutex>
#include <unordered_map>
#include <vector>

//===========================================================

#define MAP_FILENAME_EXTENSION2 ".vmtree"

#define FILENAMEBUFFER_SIZE 500

/**
This is the main Class to manage loading and unloading of maps, line of sight, height calculation and so on.
For each map or map tile to load it reads a directory file that contains the ModelContainer files used by this map or map tile.
Each global map or instance has its own dynamic BSP-Tree.
The loaded ModelContainers are included in one of these BSP-Trees.
Additionally a table to match map ids and map names is used.
*/

//===========================================================

namespace G3D
{
    class Vector3;
}

namespace VMAP
{
    class StaticMapTree;
    class WorldModel;

    class ManagedModel
    {
        public:
            ManagedModel();
            void setModel(std::unique_ptr<WorldModel> model);
            WorldModel* getModel() { return iModel.get(); }
            void incRefCount() { ++iRefCount; }
            int decRefCount() { return --iRefCount; }
        protected:
            std::unique_ptr<WorldModel> iModel;
            int iRefCount;
    };

    typedef std::unordered_map<uint32_t, std::unique_ptr<StaticMapTree>> InstanceTreeMap;
    typedef std::unordered_map<std::string, ManagedModel> ModelFileMap;

    enum DisableTypes
    {
        VMAP_DISABLE_AREAFLAG       = 0x1,
        VMAP_DISABLE_HEIGHT         = 0x2,
        VMAP_DISABLE_LOS            = 0x4,
        VMAP_DISABLE_LIQUIDSTATUS   = 0x8
    };

    class SERVER_DECL VMapManager2 : public IVMapManager
    {
        protected:
            // Tree to check collision
            ModelFileMap iLoadedModelFiles;
            InstanceTreeMap iInstanceMapTrees;
            bool thread_safe_environment;
            // Mutex for iLoadedModelFiles
            std::mutex LoadedModelFilesLock;

            bool _loadMap(uint32_t mapId, const std::string& basePath, uint32_t tileX, uint32_t tileY);
            /* void _unloadMap(uint32_t pMapId, uint32_t x, uint32_t y); */

            static uint32_t GetLiquidFlagsDummy(uint32_t) { return 0; }
            static bool IsVMAPDisabledForDummy(uint32_t /*entry*/, uint8_t /*flags*/) { return false; }

            InstanceTreeMap::const_iterator GetMapTree(uint32_t mapId) const;

        public:
            // public for debug
            G3D::Vector3 convertPositionToInternalRep(float x, float y, float z) const;
            static std::string getMapFileName(unsigned int mapId);

            VMapManager2();
            ~VMapManager2(void);

            void InitializeThreadUnsafe(const std::vector<uint32_t>& mapIds);
            int loadMap(const char* pBasePath, unsigned int mapId, int x, int y) override;

            void unloadMap(unsigned int mapId, int x, int y) override;
            void unloadMap(unsigned int mapId) override;

            bool isInLineOfSight(unsigned int mapId, float x1, float y1, float z1, float x2, float y2, float z2) override;
            /**
            fill the hit pos and return true, if an object was hit
            */
            bool getObjectHitPos(unsigned int mapId, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float modifyDist) override;
            float getHeight(unsigned int mapId, float x, float y, float z, float maxSearchDist) override;

            bool processCommand(char* /*command*/) override { return false; } // for debug and extensions

            bool getAreaInfo(unsigned int pMapId, float x, float y, float& z, uint32_t& flags, int32_t& adtId, int32_t& rootId, int32_t& groupId) const override;
            bool getLiquidLevel(uint32_t pMapId, float x, float y, float z, uint8_t reqLiquidType, float& level, float& floor, uint32_t& type, uint32_t& mogpFlags) const override;
            void getAreaAndLiquidData(uint32_t mapId, float x, float y, float z, uint8_t reqLiquidType, AreaAndLiquidData& data) const override;

            WorldModel* acquireModelInstance(const std::string& basepath, const std::string& filename);
            void releaseModelInstance(const std::string& filename);

            // what's the use of this? o.O
            virtual std::string getDirFileName(unsigned int mapId, int /*x*/, int /*y*/) const override
            {
                return getMapFileName(mapId);
            }
            virtual bool existsMap(const char* basePath, unsigned int mapId, int x, int y) override;

            InstanceTreeMap const& getInstanceMapTree() const;

            typedef uint32_t(*GetLiquidFlagsFn)(uint32_t liquidType);
            GetLiquidFlagsFn GetLiquidFlagsPtr;

            typedef bool(*IsVMAPDisabledForFn)(uint32_t entry, uint8_t flags);
            IsVMAPDisabledForFn IsVMAPDisabledForPtr;
    };
}

#endif
