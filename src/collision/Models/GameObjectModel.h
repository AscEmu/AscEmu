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

#ifndef _GAMEOBJECT_MODEL_H
#define _GAMEOBJECT_MODEL_H

#include <G3D/Matrix3.h>
#include <G3D/Vector3.h>
#include <G3D/AABox.h>
#include <G3D/Ray.h>

#include "MapTree.h"
#include <memory>

namespace VMAP
{
    class WorldModel;
}

class GameObject;
struct GameObjectDisplayInfoEntry;

class GameObjectModelOwnerBase
{
public:
    virtual bool IsSpawned() const { return false; }
    virtual uint32_t GetDisplayId() const { return 0; }
    virtual uint32_t GetPhaseMask() const { return 0; }
    virtual G3D::Vector3 GetPosition() const { return G3D::Vector3::zero(); }
    virtual float GetOrientation() const { return 0.0f; }
    virtual float GetScale() const { return 1.0f; }
    virtual void DebugVisualizeCorner(G3D::Vector3 const& /*corner*/) const { }
};

class GameObjectModel /*, public Intersectable*/
{
    GameObjectModel() : phasemask(0), iInvRot(), iInvScale(0), iScale(0), iModel(nullptr), isWmo(false)
    {
    }

public:
    std::string name;

    const G3D::AABox& getBounds() const { return iBound; }

    ~GameObjectModel();

    const G3D::Vector3& getPosition() const { return iPos;}

    /**    Enables\disables collision. */
    void disable() { phasemask = 0;}
    void enable(uint32_t ph_mask) { phasemask = ph_mask;}

    bool isEnabled() const {return phasemask != 0;}
    bool isMapObject() const { return isWmo; }

    bool intersectRay(const G3D::Ray& Ray, float& MaxDist, bool StopAtFirstHit, uint32_t ph_mask) const;
    void intersectPoint(G3D::Vector3 const& point, VMAP::AreaInfo& info, uint32_t ph_mask) const;
    bool getLocationInfo(G3D::Vector3 const& point, VMAP::LocationInfo& info, uint32_t ph_mask) const;
    bool getLiquidLevel(G3D::Vector3 const& point, VMAP::LocationInfo& info, float& liqHeight) const;

    static std::unique_ptr<GameObjectModel> Create(std::unique_ptr<GameObjectModelOwnerBase> modelOwner, std::string const& dataPath);

    bool UpdatePosition();

private:
    bool initialize(std::unique_ptr<GameObjectModelOwnerBase> modelOwner, std::string const& dataPath);

    uint32_t phasemask;
    G3D::AABox iBound;
    G3D::Matrix3 iInvRot;
    G3D::Vector3 iPos;
    float iInvScale;
    float iScale;
    VMAP::WorldModel* iModel;
    std::unique_ptr<GameObjectModelOwnerBase> owner;
    bool isWmo;
};

#endif // _GAMEOBJECT_MODEL_H
