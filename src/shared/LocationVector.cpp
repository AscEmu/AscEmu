/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LocationVector.h"

float LocationVector::getPositionX() const { return x; }
float LocationVector::getPositionY() const { return y; }
float LocationVector::getPositionZ() const { return z; }
float LocationVector::getOrientation() const { return o; }

void LocationVector::getPosition(float& rx, float& ry) const
{
    rx = x;
    ry = y;
}

void LocationVector::getPosition(float& rx, float& ry, float& rz) const
{
    getPosition(rx, ry); rz = z;
}

void LocationVector::getPosition(float& rx, float& ry, float& rz, float& ro) const
{
    getPosition(rx, ry, rz); ro = o;
}

float LocationVector::distanceSquare(const LocationVector& target) const
{
    auto delta_x = target.x - x;
    auto delta_y = target.y - y;
    auto delta_z = target.z - z;

    return delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;
}

bool LocationVector::isSet() const
{
    return x || y || z || o;
}

void LocationVector::changeCoords(float newX, float newY, float newZ, float newO)
{
    x = newX;
    y = newY;
    z = newZ;
    o = newO;
}
