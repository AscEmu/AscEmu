/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "CommonTypes.hpp"
#include "CommonDefines.hpp"

#include <math.h>
#include <string>
#include <cmath>

//////////////////////////////////////////////////////////////////////////////////////////
// Location vector class (X, Y, Z, O)
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL LocationVector
{
    public:
        // Constructors
        LocationVector(float X, float Y, float Z = 0, float O = 0) : x(X), y(Y), z(Z), o(O) {}
        LocationVector() = default;

        // MIT Start
        float getPositionX() const;
        float getPositionY() const;
        float getPositionZ() const;
        float getOrientation() const;

        void getPosition(float& rx, float& ry) const;
        void getPosition(float& rx, float& ry, float& rz) const;
        void getPosition(float& rx, float& ry, float& rz, float& ro) const;

        float distanceSquare(const LocationVector& comp) const;
        bool isSet() const;

        void changeCoords(float newX, float newY, float newZ, float newO);
        void changeCoords(float newX, float newY, float newZ);

        float getExactDist2dSq(const float _x, const float _y) const
        {
            float dx = _x - getPositionX();
            float dy = _y - getPositionY();
            return dx * dx + dy * dy;
        }
        float getExactDist2dSq(LocationVector const& pos) const { return getExactDist2dSq(pos.x, pos.y); }
        float getExactDist2dSq(LocationVector const* pos) const { return getExactDist2dSq(*pos); }

        float getExactDist2d(const float _x, const float _y) const { return std::sqrt(getExactDist2dSq(_x, _y)); }
        float getExactDist2d(LocationVector const& pos) const { return getExactDist2d(pos.x, pos.y); }
        float getExactDist2d(LocationVector const* pos) const { return getExactDist2d(*pos); }

        float getExactDistSq(float _x, float _y, float _z) const
        {
            float dz = _z - getPositionZ();
            return getExactDist2dSq(_x, _y) + dz * dz;
        }
        float getExactDistSq(LocationVector const& pos) const { return getExactDistSq(pos.x, pos.y, pos.z); }
        float getExactDistSq(LocationVector const* pos) const { return getExactDistSq(*pos); }

        float getExactDist(float _x, float _y, float _z) const { return std::sqrt(getExactDistSq(_x, _y, _z)); }
        float getExactDist(LocationVector const& pos) const { return getExactDist(pos.x, pos.y, pos.z); }
        float getExactDist(LocationVector const* pos) const { return getExactDist(*pos); }

        float getAbsoluteAngle(float _x, float _y) const
        {
            float dx = _x - getPositionX();
            float dy = _y - getPositionY();
            return normalizeOrientation(std::atan2(dy, dx));
        }

        float getAbsoluteAngle(LocationVector const& pos) { return getAbsoluteAngle(pos.x, pos.y); }
        float getAbsoluteAngle(LocationVector const* pos) const { return getAbsoluteAngle(pos->x, pos->y); }
        float toAbsoluteAngle(float relAngle) const { return normalizeOrientation(relAngle + o); }

        float toRelativeAngle(float absAngle) const { return normalizeOrientation(absAngle - o); }
        float getRelativeAngle(float _x, float _y) const { return toRelativeAngle(getAbsoluteAngle(_x, _y)); }
        float getRelativeAngle(LocationVector const* pos) const { return toRelativeAngle(getAbsoluteAngle(pos)); }

        bool isInDist2d(float _x, float _y, float dist) const { return getExactDist2dSq(_x, _y) < dist * dist; }
        bool isInDist2d(LocationVector const* pos, float dist) const { return getExactDist2dSq(pos) < dist * dist; }

        bool isInDist(float _x, float _y, float _z, float dist) const { return getExactDistSq(_x, _y, _z) < dist * dist; }
        bool isInDist(LocationVector const& pos, float dist) const { return getExactDistSq(pos) < dist * dist; }
        bool isInDist(LocationVector const* pos, float dist) const { return getExactDistSq(pos) < dist * dist; }

        bool isWithinBox(LocationVector const& center, float xradius, float yradius, float zradius) const;

        // constrain arbitrary radian orientation to interval [0,2*PI)
        static float normalizeOrientation(float o);
        // MIT End

        // std::sqrt(dx * dx + dy * dy + dz * dz)
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

        void ChangeCoordsOffset(const LocationVector& offset)
        {
            x = getPositionX() + (offset.getPositionX() * std::cos(getOrientation()) + offset.getPositionY() * std::sin(getOrientation() + float(M_PI)));
            y = getPositionY() + (offset.getPositionY() * std::cos(getOrientation()) + offset.getPositionX() * std::sin(getOrientation()));
            z = getPositionZ() + offset.getPositionZ();
            o = getOrientation() + offset.o;
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

        bool operator!=(LocationVector const& a)
        {
            return !(operator==(a));
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
