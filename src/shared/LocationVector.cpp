/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LocationVector.hpp"
#include "Utilities/MathConstants.hpp"

#include <cmath>

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

float LocationVector::getExactDist2dSq(const float _x, const float _y) const
{
    float dx = _x - getPositionX();
    float dy = _y - getPositionY();
    return dx * dx + dy * dy;
}
float LocationVector::getExactDist2dSq(LocationVector const& pos) const { return getExactDist2dSq(pos.x, pos.y); }
float LocationVector::getExactDist2dSq(LocationVector const* pos) const { return getExactDist2dSq(*pos); }

float LocationVector::getExactDist2d(const float _x, const float _y) const { return std::sqrt(getExactDist2dSq(_x, _y)); }
float LocationVector::getExactDist2d(LocationVector const& pos) const { return getExactDist2d(pos.x, pos.y); }
float LocationVector::getExactDist2d(LocationVector const* pos) const { return getExactDist2d(*pos); }

float LocationVector::getExactDistSq(float _x, float _y, float _z) const
{
    float dz = _z - getPositionZ();
    return getExactDist2dSq(_x, _y) + dz * dz;
}
float LocationVector::getExactDistSq(LocationVector const& pos) const { return getExactDistSq(pos.x, pos.y, pos.z); }
float LocationVector::getExactDistSq(LocationVector const* pos) const { return getExactDistSq(*pos); }

float LocationVector::getExactDist(float _x, float _y, float _z) const { return std::sqrt(getExactDistSq(_x, _y, _z)); }
float LocationVector::getExactDist(LocationVector const& pos) const { return getExactDist(pos.x, pos.y, pos.z); }
float LocationVector::getExactDist(LocationVector const* pos) const { return getExactDist(*pos); }

float LocationVector::getAbsoluteAngle(float _x, float _y) const
{
    float dx = _x - getPositionX();
    float dy = _y - getPositionY();
    return normalizeOrientation(std::atan2(dy, dx));
}

float LocationVector::getAbsoluteAngle(LocationVector const& pos) { return getAbsoluteAngle(pos.x, pos.y); }
float LocationVector::getAbsoluteAngle(LocationVector const* pos) const { return getAbsoluteAngle(pos->x, pos->y); }
float LocationVector::toAbsoluteAngle(float relAngle) const { return normalizeOrientation(relAngle + o); }

float LocationVector::toRelativeAngle(float absAngle) const { return normalizeOrientation(absAngle - o); }
float LocationVector::getRelativeAngle(float _x, float _y) const { return toRelativeAngle(getAbsoluteAngle(_x, _y)); }
float LocationVector::getRelativeAngle(LocationVector const* pos) const { return toRelativeAngle(getAbsoluteAngle(pos)); }

bool LocationVector::isInDist2d(float _x, float _y, float dist) const { return getExactDist2dSq(_x, _y) < dist * dist; }
bool LocationVector::isInDist2d(LocationVector const* pos, float dist) const { return getExactDist2dSq(pos) < dist * dist; }

bool LocationVector::isInDist(float _x, float _y, float _z, float dist) const { return getExactDistSq(_x, _y, _z) < dist * dist; }
bool LocationVector::isInDist(LocationVector const& pos, float dist) const { return getExactDistSq(pos) < dist * dist; }
bool LocationVector::isInDist(LocationVector const* pos, float dist) const { return getExactDistSq(pos) < dist * dist; }

bool LocationVector::isWithinBox(LocationVector const& center, float xradius, float yradius, float zradius) const
{
    // rotate the Object position instead of rotating the whole cube, that way we can make a simplified
    // is-in-cube check and we have to calculate only one point instead of 4

    // 2PI = 360*, keep in mind that ingame orientation is counter-clockwise
    double rotation = 2 * AscEmu::Math::Pi - center.getOrientation();
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
        mod = fmod(mod, 2.0f * AscEmu::Math::PiF);
        mod = -mod + 2.0f * AscEmu::Math::PiF;
        return mod;
    }

    return fmod(orientation, 2.0f * AscEmu::Math::PiF);
}

// std::sqrt(dx * dx + dy * dy + dz * dz)
float LocationVector::distance(const LocationVector& comp)
{
    float delta_x = comp.x - x;
    float delta_y = comp.y - y;
    float delta_z = comp.z - z;

    return sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
}

float LocationVector::distance2DSq(const LocationVector& comp)
{
    float delta_x = comp.x - x;
    float delta_y = comp.y - y;

    return delta_x * delta_x + delta_y * delta_y;
}

float LocationVector::distance2D(LocationVector& comp)
{
    float delta_x = comp.x - x;
    float delta_y = comp.y - y;

    return sqrtf(delta_x * delta_x + delta_y * delta_y);
}

// atan2(dx / dy)
float LocationVector::calcAngTo(const LocationVector& dest)
{
    float dx = dest.x - x;
    float dy = dest.y - y;

    if (dy != 0.0f)
        return static_cast<float>(atan2(dy, dx));

    return 0.0f;
}

float LocationVector::calcAngFrom(const LocationVector& src)
{
    float dx = x - src.x;
    float dy = y - src.y;

    if (dy != 0.0f)
        return static_cast<float>(atan2(dy, dx));

    return 0.0f;
}

void LocationVector::changeCoords(const LocationVector& src)
{
    x = src.x;
    y = src.y;
    z = src.z;
    o = src.o;
}

void LocationVector::changeCoordsOffset(const LocationVector& offset)
{
    x = getPositionX() + (offset.getPositionX() * std::cos(getOrientation()) + offset.getPositionY() * std::sin(getOrientation() + AscEmu::Math::PiF));
    y = getPositionY() + (offset.getPositionY() * std::cos(getOrientation()) + offset.getPositionX() * std::sin(getOrientation()));
    z = getPositionZ() + offset.getPositionZ();
    o = getOrientation() + offset.o;
}

// add/subtract/equality vectors
LocationVector& LocationVector::operator += (const LocationVector& add)
{
    x += add.x;
    y += add.y;
    z += add.z;
    o += add.o;
    return *this;
}

LocationVector& LocationVector::operator -= (const LocationVector& sub)
{
    x -= sub.x;
    y -= sub.y;
    z -= sub.z;
    o -= sub.o;

    return *this;
}

LocationVector& LocationVector::operator = (const LocationVector& eq)
{
    x = eq.x;
    y = eq.y;
    z = eq.z;
    o = eq.o;

    return *this;
}

bool LocationVector::operator!=(LocationVector const& a)
{
    return !(operator==(a));
}

bool LocationVector::operator == (const LocationVector& eq)
{
    if (eq.x == x && eq.y == y && eq.z == z)
        return true;

    return false;
}
