/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LocationVector.h"

float LocationVector::distanceSquare(const LocationVector& target) const
{
    auto delta_x = target.x - x;
    auto delta_y = target.y - y;
    auto delta_z = target.z - z;

    return delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;
}

float LocationVector::distanceSquare(const float& X, const float& Y, const float& Z) const
{
    auto delta_x = X - x;
    auto delta_y = Y - y;
    auto delta_z = Z - z;

    return (delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
}

bool LocationVector::isSet() const
{
    return x || y || z || o;
}
