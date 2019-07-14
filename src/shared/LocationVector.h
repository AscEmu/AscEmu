/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LOCATIONVECTOR_H
#define _LOCATIONVECTOR_H

#include <math.h>
#include "CommonTypes.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Location vector class (X, Y, Z, O)
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL LocationVector
{
    public:

        // Constructors
        LocationVector(float X, float Y, float Z = 0, float O = 0) : x(X), y(Y), z(Z), o(O) {}
        LocationVector() {}

    // MIT Start
    float distanceSquare(const LocationVector& comp) const;
    bool isSet() const;
    // MIT End

        // sqrt(dx * dx + dy * dy + dz * dz)
        float Distance(const LocationVector & comp)
        {
            float delta_x = comp.x - x;
            float delta_y = comp.y - y;
            float delta_z = comp.z - z;

            return sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
        }

        float Distance2DSq(const LocationVector & comp)
        {
            float delta_x = comp.x - x;
            float delta_y = comp.y - y;

            return delta_x * delta_x + delta_y * delta_y;
        }

        float Distance2D(LocationVector & comp)
        {
            float delta_x = comp.x - x;
            float delta_y = comp.y - y;

            return sqrtf(delta_x * delta_x + delta_y * delta_y);
        }

        // atan2(dx / dy)
        float CalcAngTo(const LocationVector & dest)
        {
            float dx = dest.x - x;
            float dy = dest.y - y;

            if (dy != 0.0f)
                return static_cast<float>(atan2(dy, dx));

            return 0.0f;
        }

        float CalcAngFrom(const LocationVector & src)
        {
            float dx = x - src.x;
            float dy = y - src.y;

            if (dy != 0.0f)
                return static_cast<float>(atan2(dy, dx));

            return 0.0f;
        }

        void ChangeCoords(const LocationVector& src)
        {
            x = src.x;
            y = src.y;
            z = src.z;
            o = src.o;
        }

        // add/subtract/equality vectors
        LocationVector & operator += (const LocationVector & add)
        {
            x += add.x;
            y += add.y;
            z += add.z;
            o += add.o;
            return *this;
        }

        LocationVector & operator -= (const LocationVector & sub)
        {
            x -= sub.x;
            y -= sub.y;
            z -= sub.z;
            o -= sub.o;

            return *this;
        }

        LocationVector & operator = (const LocationVector & eq)
        {
            x = eq.x;
            y = eq.y;
            z = eq.z;
            o = eq.o;

            return *this;
        }

        bool operator == (const LocationVector & eq)
        {
            if (eq.x == x && eq.y == y && eq.z == z)
                return true;

            return false;
        }

        float x{};
        float y{};
        float z{};
        float o{};
};

#endif      //_LOCATIONVECTOR_H
