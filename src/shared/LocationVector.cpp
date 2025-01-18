/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

void LocationVector::changeCoords(float newX, float newY, float newZ)
{
    x = newX;
    y = newY;
    z = newZ;
}

bool LocationVector::isWithinBox(LocationVector const& center, float xradius, float yradius, float zradius) const
{
    // rotate the Object position instead of rotating the whole cube, that way we can make a simplified
    // is-in-cube check and we have to calculate only one point instead of 4

    // 2PI = 360*, keep in mind that ingame orientation is counter-clockwise
    double rotation = 2 * M_PI - center.getOrientation();
    double sinVal = std::sin(rotation);
    double cosVal = std::cos(rotation);

    float BoxDistX = getPositionX() - center.getPositionX();
    float BoxDistY = getPositionY() - center.getPositionY();

    float rotX = float(center.getPositionX() + BoxDistX * cosVal - BoxDistY * sinVal);
    float rotY = float(center.getPositionY() + BoxDistY * cosVal + BoxDistX * sinVal);

    // box edges are parallel to coordiante axis, so we can treat every dimension independently :D
    float dz = getPositionZ() - center.getPositionZ();
    float dx = rotX - center.getPositionX();
    float dy = rotY - center.getPositionY();
    if ((std::fabs(dx) > xradius) ||
        (std::fabs(dy) > yradius) ||
        (std::fabs(dz) > zradius))
        return false;

    return true;
}

float LocationVector::normalizeOrientation(float orientation)
{
    if (orientation < 0)
    {
        float mod = orientation * -1;
        mod = fmod(mod, 2.0f * static_cast<float>(M_PI));
        mod = -mod + 2.0f * static_cast<float>(M_PI);
        return mod;
    }

    return fmod(orientation, 2.0f * static_cast<float>(M_PI));
}
