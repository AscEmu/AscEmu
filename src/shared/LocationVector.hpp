/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

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

        float getExactDist2dSq(const float _x, const float _y) const;
        float getExactDist2dSq(LocationVector const& pos) const;
        float getExactDist2dSq(LocationVector const* pos) const;

        float getExactDist2d(const float _x, const float _y) const;
        float getExactDist2d(LocationVector const& pos) const;
        float getExactDist2d(LocationVector const* pos) const;

        float getExactDistSq(float _x, float _y, float _z) const;
        float getExactDistSq(LocationVector const& pos) const;
        float getExactDistSq(LocationVector const* pos) const;

        float getExactDist(float _x, float _y, float _z) const;
        float getExactDist(LocationVector const& pos) const;
        float getExactDist(LocationVector const* pos) const;

        float getAbsoluteAngle(float _x, float _y) const;

        float getAbsoluteAngle(LocationVector const& pos);
        float getAbsoluteAngle(LocationVector const* pos) const;
        float toAbsoluteAngle(float relAngle) const;

        float toRelativeAngle(float absAngle) const;
        float getRelativeAngle(float _x, float _y) const;
        float getRelativeAngle(LocationVector const* pos) const;

        bool isInDist2d(float _x, float _y, float dist) const;
        bool isInDist2d(LocationVector const* pos, float dist) const;

        bool isInDist(float _x, float _y, float _z, float dist) const;
        bool isInDist(LocationVector const& pos, float dist) const;
        bool isInDist(LocationVector const* pos, float dist) const;

        bool isWithinBox(LocationVector const& center, float xradius, float yradius, float zradius) const;

        // constrain arbitrary radian orientation to interval [0,2*PI)
        static float normalizeOrientation(float o);

        float distance(const LocationVector & comp);

        float distance2DSq(const LocationVector & comp);

        float distance2D(LocationVector & comp);

        // atan2(dx / dy)
        float calcAngTo(const LocationVector & dest);

        float calcAngFrom(const LocationVector & src);

        void changeCoords(const LocationVector& src);

        void changeCoordsOffset(const LocationVector& offset);

        // add/subtract/equality vectors
        LocationVector & operator += (const LocationVector & add);

        LocationVector & operator -= (const LocationVector & sub);

        LocationVector & operator = (const LocationVector & eq);

        bool operator!=(LocationVector const& a);

        bool operator == (const LocationVector & eq);

        float x{};
        float y{};
        float z{};
        float o{};
};
