/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#include "DBC\DBCStructures.hpp"


namespace VMAP
{
    class WorldModel;
}

class GameObject;

class GameObjectModel /*, public Intersectable*/
{
    G3D::uint32 phasemask;
    G3D::AABox iBound;
    G3D::Matrix3 iInvRot;
    G3D::Vector3 iPos;
    //G3D::Vector3 iRot;
    float iInvScale;
    float iScale;
    VMAP::WorldModel* iModel;

    GameObjectModel() : phasemask(0), iModel(NULL) {}
    bool initialize(GameObject& go, const DBC::Structures::GameObjectDisplayInfoEntry& info);

public:
    std::string name;

    const G3D::AABox& getBounds() const { return iBound; }

    ~GameObjectModel();

    const G3D::Vector3& getPosition() const { return iPos;}

    /**	Enables\disables collision. */
    void disable() { phasemask = 0;}
    void enable(G3D::uint32 ph_mask) { phasemask = ph_mask;}

    bool intersectRay(const G3D::Ray& Ray, float& MaxDist, bool StopAtFirstHit, G3D::uint32 ph_mask) const;

    static GameObjectModel* Create(GameObject& go);
};

#endif // _GAMEOBJECT_MODEL_H
