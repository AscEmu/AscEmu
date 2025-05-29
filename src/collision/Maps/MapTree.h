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

#ifndef _MAPTREE_H
#define _MAPTREE_H

#include "BoundingIntervalHierarchy.h"
#include <unordered_map>

namespace VMAP
{
    class ModelInstance;
    class GroupModel;
    class VMapManager2;

    struct LocationInfo
    {
        LocationInfo(): rootId(-1), hitInstance(nullptr), hitModel(nullptr), ground_Z(-G3D::finf()) { }
        int32_t rootId;
        const ModelInstance* hitInstance;
        const GroupModel* hitModel;
        float ground_Z;
    };

    class StaticMapTree
    {
        typedef std::unordered_map<uint32_t, bool> loadedTileMap;
        typedef std::unordered_map<uint32_t, uint32_t> loadedSpawnMap;
        private:
            uint32_t iMapID;
            bool iIsTiled;
            BIH iTree;
            std::unique_ptr<ModelInstance[]> iTreeValues; // the tree entries
            uint32_t iNTreeValues;

            // Store all the map tile idents that are loaded for that map
            // some maps are not splitted into tiles and we have to make sure, not removing the map before all tiles are removed
            // empty tiles have no tile file, hence map with bool instead of just a set (consistency check)
            loadedTileMap iLoadedTiles;
            // stores <tree_index, reference_count> to invalidate tree values, unload map, and to be able to report errors
            loadedSpawnMap iLoadedSpawns;
            std::string iBasePath;

        private:
            bool getIntersectionTime(const G3D::Ray& pRay, float &pMaxDist, bool pStopAtFirstHit) const;
            //bool containsLoadedMapTile(unsigned int pTileIdent) const { return(iLoadedMapTiles.containsKey(pTileIdent)); }
        public:
            static std::string getTileFileName(uint32_t mapID, uint32_t tileX, uint32_t tileY);
            static uint32_t packTileID(uint32_t tileX, uint32_t tileY) { return tileX<<16 | tileY; }
            static void unpackTileID(uint32_t ID, uint32_t &tileX, uint32_t &tileY) { tileX = ID>>16; tileY = ID&0xFF; }
            static bool CanLoadMap(const std::string &basePath, uint32_t mapID, uint32_t tileX, uint32_t tileY);

            StaticMapTree(uint32_t mapID, const std::string &basePath);
            ~StaticMapTree();

            bool isInLineOfSight(const G3D::Vector3& pos1, const G3D::Vector3& pos2) const;
            bool getObjectHitPos(const G3D::Vector3& pos1, const G3D::Vector3& pos2, G3D::Vector3& pResultHitPos, float pModifyDist) const;
            float getHeight(const G3D::Vector3& pPos, float maxSearchDist) const;
            bool getAreaInfo(G3D::Vector3 &pos, uint32_t &flags, int32_t &adtId, int32_t &rootId, int32_t &groupId) const;
            bool GetLocationInfo(const G3D::Vector3 &pos, LocationInfo &info) const;

            bool InitMap(const std::string &fname, VMapManager2* vm);
            void UnloadMap(VMapManager2* vm);
            bool LoadMapTile(uint32_t tileX, uint32_t tileY, VMapManager2* vm);
            void UnloadMapTile(uint32_t tileX, uint32_t tileY, VMapManager2* vm);
            bool isTiled() const { return iIsTiled; }
            uint32_t numLoadedTiles() const { return static_cast<uint32_t>(iLoadedTiles.size()); }
            void getModelInstances(ModelInstance* &models, uint32_t &count);

        private:
            StaticMapTree(StaticMapTree const& right) = delete;
            StaticMapTree& operator=(StaticMapTree const& right) = delete;
    };

    struct AreaInfo
    {
        AreaInfo(): result(false), ground_Z(-G3D::finf()), flags(0), adtId(0),
            rootId(0), groupId(0) { }
        bool result;
        float ground_Z;
        uint32_t flags;
        int32_t adtId;
        int32_t rootId;
        int32_t groupId;
    };
}                                                           // VMAP

#endif // _MAPTREE_H
