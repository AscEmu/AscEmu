/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <math.h>
#include "CommonTypes.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Location vector class (X, Y, Z, O)
class SERVER_DECL LocationVector
{
public:
    // Constructors
    LocationVector(float X, float Y, float Z = 0, float O = 0) : x(X), y(Y), z(Z), o(O) {}
    LocationVector() {}

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
