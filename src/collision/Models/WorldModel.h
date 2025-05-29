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

#ifndef _WORLDMODEL_H
#define _WORLDMODEL_H

#include <G3D/Vector3.h>
#include <G3D/AABox.h>
#include <G3D/Ray.h>
#include "BoundingIntervalHierarchy.h"

namespace VMAP
{
    class TreeNode;
    struct AreaInfo;
    struct LocationInfo;

    class MeshTriangle
    {
        public:
            MeshTriangle() : idx0(0), idx1(0), idx2(0) { }
            MeshTriangle(uint32_t na, uint32_t nb, uint32_t nc): idx0(na), idx1(nb), idx2(nc) { }

            uint32_t idx0;
            uint32_t idx1;
            uint32_t idx2;
    };

    class WmoLiquid
    {
        public:
            WmoLiquid(uint32_t width, uint32_t height, const G3D::Vector3 &corner, uint32_t type);
            WmoLiquid(const WmoLiquid &other);
            ~WmoLiquid();
            WmoLiquid& operator=(const WmoLiquid &other);
            bool GetLiquidHeight(const G3D::Vector3 &pos, float &liqHeight) const;
            uint32_t GetType() const { return iType; }
            float *GetHeightStorage() { return iHeight.get(); }
            uint8_t *GetFlagsStorage() { return iFlags.get(); }
            uint32_t GetFileSize();
            bool writeToFile(FILE* wf);
            static bool readFromFile(FILE* rf, std::unique_ptr<WmoLiquid> &liquid);
            void getPosInfo(uint32_t &tilesX, uint32_t &tilesY, G3D::Vector3 &corner) const;
        private:
            WmoLiquid() : iTilesX(0), iTilesY(0), iCorner(), iType(0), iHeight(NULL), iFlags(NULL) { }
            uint32_t iTilesX;       //!< number of tiles in x direction, each
            uint32_t iTilesY;
            G3D::Vector3 iCorner; //!< the lower corner
            uint32_t iType;         //!< liquid type
            std::unique_ptr<float[]> iHeight;       //!< (tilesX + 1)*(tilesY + 1) height values
            std::unique_ptr<uint8_t[]> iFlags;        //!< info if liquid tile is used
    };

    /*! holding additional info for WMO group files */
    class GroupModel
    {
        public:
            GroupModel() : iBound(), iMogpFlags(0), iGroupWMOID(0), iLiquid(nullptr) { }
            GroupModel(const GroupModel &other);
            GroupModel(uint32_t mogpFlags, uint32_t groupWMOID, const G3D::AABox &bound):
                        iBound(bound), iMogpFlags(mogpFlags), iGroupWMOID(groupWMOID), iLiquid(nullptr) { }
            ~GroupModel() = default;

            //! pass mesh data to object and create BIH. Passed vectors get get swapped with old geometry!
            void setMeshData(std::vector<G3D::Vector3> &vert, std::vector<MeshTriangle> &tri);
            void setLiquidData(std::unique_ptr<WmoLiquid> liquid) { iLiquid = std::move(liquid); }
            bool IntersectRay(const G3D::Ray &ray, float &distance, bool stopAtFirstHit) const;
            bool IsInsideObject(const G3D::Vector3 &pos, const G3D::Vector3 &down, float &z_dist) const;
            bool GetLiquidLevel(const G3D::Vector3 &pos, float &liqHeight) const;
            uint32_t GetLiquidType() const;
            bool writeToFile(FILE* wf);
            bool readFromFile(FILE* rf);
            const G3D::AABox& GetBound() const { return iBound; }
            uint32_t GetMogpFlags() const { return iMogpFlags; }
            uint32_t GetWmoID() const { return iGroupWMOID; }
            void getMeshData(std::vector<G3D::Vector3>& outVertices, std::vector<MeshTriangle>& outTriangles, WmoLiquid*& liquid) const;
        protected:
            G3D::AABox iBound;
            uint32_t iMogpFlags;// 0x8 outdor; 0x2000 indoor
            uint32_t iGroupWMOID;
            std::vector<G3D::Vector3> vertices;
            std::vector<MeshTriangle> triangles;
            BIH meshTree;
            std::unique_ptr<WmoLiquid> iLiquid;
    };

    /*! Holds a model (converted M2 or WMO) in its original coordinate space */
    class WorldModel
    {
        public:
            WorldModel(): RootWMOID(0) { }

            //! pass group models to WorldModel and create BIH. Passed vector is swapped with old geometry!
            void setGroupModels(std::vector<GroupModel> &models);
            void setRootWmoID(uint32_t id) { RootWMOID = id; }
            bool IntersectRay(const G3D::Ray &ray, float &distance, bool stopAtFirstHit) const;
            bool IntersectPoint(const G3D::Vector3 &p, const G3D::Vector3 &down, float &dist, AreaInfo &info) const;
            bool GetLocationInfo(const G3D::Vector3 &p, const G3D::Vector3 &down, float &dist, LocationInfo &info) const;
            bool writeFile(const std::string &filename);
            bool readFile(const std::string &filename);
            std::vector<GroupModel> const& getGroupModels() const;
        protected:
            uint32_t RootWMOID;
            std::vector<GroupModel> groupModels;
            BIH groupTree;
    };
} // namespace VMAP

#endif // _WORLDMODEL_H
