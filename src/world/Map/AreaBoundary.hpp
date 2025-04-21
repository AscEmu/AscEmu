/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LocationVector.h"

#include <memory>

class SERVER_DECL AreaBoundary
{
public:
    bool isWithinBoundary(LocationVector const* pos) const { return pos && (isWithinBoundaryArea(pos) != _isInvertedBoundary); }
    bool isWithinBoundary(LocationVector const& pos) const { return isWithinBoundary(&pos); }

    virtual ~AreaBoundary() { }

protected:
    explicit AreaBoundary(bool isInverted) : _isInvertedBoundary(isInverted) { }

    struct DoublePosition : LocationVector
    {
        DoublePosition(double x = 0.0, double y = 0.0, double z = 0.0, float o = 0.0f)
            : LocationVector(float(x), float(y), float(z), o), DoublePosX(x), DoublePosY(y), DoublePosZ(z) { }

        DoublePosition(float x, float y = 0.0f, float z = 0.0f, float o = 0.0f)
            : LocationVector(x, y, z, o), DoublePosX(x), DoublePosY(y), DoublePosZ(z) { }

        DoublePosition(LocationVector const& pos)
            : LocationVector(pos), DoublePosX(pos.x), DoublePosY(pos.y), DoublePosZ(pos.z) { }

        double getDoublePositionX() const { return DoublePosX; }
        double getDoublePositionY() const { return DoublePosY; }
        double getDoublePositionZ() const { return DoublePosZ; }

        double getDoubleExactDist2dSq(DoublePosition const& pos) const
        {
            double const offX = getDoublePositionX() - pos.getDoublePositionX();
            double const offY = getDoublePositionY() - pos.getDoublePositionY();
            return (offX * offX) + (offY * offY);
        }

        LocationVector* sync()
        {
            x = float(DoublePosX);
            y = float(DoublePosY);
            z = float(DoublePosZ);
            return this;
        }

        double DoublePosX;
        double DoublePosY;
        double DoublePosZ;
    };

    virtual bool isWithinBoundaryArea(LocationVector const* pos) const = 0;

private:
    bool _isInvertedBoundary;
};

class SERVER_DECL RectangleBoundary : public AreaBoundary
{
public:
    // X axis is north/south, Y axis is east/west, larger values are northwest
    RectangleBoundary(float southX, float northX, float eastY, float westY, bool isInverted = false);

protected:
    bool isWithinBoundaryArea(LocationVector const* pos) const override;

private:
    float const minX, maxX, minY, maxY;
};

class SERVER_DECL CircleBoundary : public AreaBoundary
{
public:
    CircleBoundary(LocationVector const& center, double radius, bool isInverted = false);
    CircleBoundary(LocationVector const& center, LocationVector const& pointOnCircle, bool isInverted = false);

protected:
    bool isWithinBoundaryArea(LocationVector const* pos) const override;

private:
    DoublePosition const _center;
    double const _radiusSq;
};

class SERVER_DECL EllipseBoundary : public AreaBoundary
{
public:
    EllipseBoundary(LocationVector const& center, double radiusX, double radiusY, bool isInverted = false);

protected:
    bool isWithinBoundaryArea(LocationVector const* pos) const override;

private:
    DoublePosition const _center;
    double const _radiusYSq, _scaleXSq;
};

class SERVER_DECL TriangleBoundary : public AreaBoundary
{
public:
    TriangleBoundary(LocationVector const& pointA, LocationVector const& pointB, LocationVector const& pointC, bool isInverted = false);

protected:
    bool isWithinBoundaryArea(LocationVector const* pos) const override;

private:
    DoublePosition const _a, _b, _c;
    double const _abx, _bcx, _cax, _aby, _bcy, _cay;
};

class SERVER_DECL ParallelogramBoundary : public AreaBoundary
{
public:
    // Note: AB must be orthogonal to AD
    ParallelogramBoundary(LocationVector const& cornerA, LocationVector const& cornerB, LocationVector const& cornerD, bool isInverted = false);

protected:
    bool isWithinBoundaryArea(LocationVector const* pos) const override;

private:
    DoublePosition const _a, _b, _d, _c;
    double const _abx, _dax, _aby, _day;
};

class SERVER_DECL ZRangeBoundary : public AreaBoundary
{
public:
    ZRangeBoundary(float minZ, float maxZ, bool isInverted = false);

protected:
    bool isWithinBoundaryArea(LocationVector const* pos) const override;

private:
    float const _minZ, _maxZ;
};

class SERVER_DECL BoundaryUnionBoundary : public AreaBoundary
{
public:
    BoundaryUnionBoundary(std::unique_ptr<AreaBoundary const> b1, std::unique_ptr<AreaBoundary const> b2, bool isInverted = false);

protected:
    virtual ~BoundaryUnionBoundary();
    bool isWithinBoundaryArea(LocationVector const* pos) const override;

private:
    std::unique_ptr<AreaBoundary const> const _b1;
    std::unique_ptr<AreaBoundary const> const _b2;
};
