/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AreaBoundary.hpp"
#include "Objects/Units/Unit.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Rectangle
RectangleBoundary::RectangleBoundary(float southX, float northX, float eastY, float westY, bool isInverted) :
    AreaBoundary(isInverted), minX(southX), maxX(northX), minY(eastY), maxY(westY) { }
bool RectangleBoundary::isWithinBoundaryArea(LocationVector const* pos) const
{
    return !(
        pos->getPositionX() < minX ||
        pos->getPositionX() > maxX ||
        pos->getPositionY() < minY ||
        pos->getPositionY() > maxY
        );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Circle
CircleBoundary::CircleBoundary(LocationVector const& center, double radius, bool isInverted) :
    AreaBoundary(isInverted), _center(center), _radiusSq(radius*radius) { }
CircleBoundary::CircleBoundary(LocationVector const& center, LocationVector const& pointOnCircle, bool isInverted) :
    AreaBoundary(isInverted), _center(center), _radiusSq(_center.getDoubleExactDist2dSq(pointOnCircle)) { }
bool CircleBoundary::isWithinBoundaryArea(LocationVector const* pos) const
{
    double offX = _center.getDoublePositionX() - pos->getPositionX();
    double offY = _center.getDoublePositionY() - pos->getPositionY();
    return offX*offX+offY*offY <= _radiusSq;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Ellipse
EllipseBoundary::EllipseBoundary(LocationVector const& center, double radiusX, double radiusY, bool isInverted) :
    AreaBoundary(isInverted), _center(center), _radiusYSq(radiusY*radiusY), _scaleXSq(_radiusYSq / (radiusX*radiusX)) { }
bool EllipseBoundary::isWithinBoundaryArea(LocationVector const* pos) const
{
    double offX = _center.getDoublePositionX() - pos->getPositionX();
    double offY = _center.getDoublePositionY() - pos->getPositionY();
    return (offX*offX)*_scaleXSq + (offY*offY)  <=  _radiusYSq;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Triangle
TriangleBoundary::TriangleBoundary(LocationVector const& pointA, LocationVector const& pointB, LocationVector const& pointC, bool isInverted) :
    AreaBoundary(isInverted), _a(pointA), _b(pointB), _c(pointC), _abx(_b.getDoublePositionX()-_a.getDoublePositionX()), _bcx(_c.getDoublePositionX()-_b.getDoublePositionX()), _cax(_a.getDoublePositionX() - _c.getDoublePositionX()), _aby(_b.getDoublePositionY()-_a.getDoublePositionY()), _bcy(_c.getDoublePositionY()-_b.getDoublePositionY()), _cay(_a.getDoublePositionY() - _c.getDoublePositionY()) { }
bool TriangleBoundary::isWithinBoundaryArea(LocationVector const* pos) const
{
    // half-plane signs
    bool sign1 = ((-_b.getDoublePositionX() + pos->getPositionX()) * _aby - (-_b.getDoublePositionY() + pos->getPositionY()) * _abx) < 0;
    bool sign2 = ((-_c.getDoublePositionX() + pos->getPositionX()) * _bcy - (-_c.getDoublePositionY() + pos->getPositionY()) * _bcx) < 0;
    bool sign3 = ((-_a.getDoublePositionX() + pos->getPositionX()) * _cay - (-_a.getDoublePositionY() + pos->getPositionY()) * _cax) < 0;

    // if all signs are the same, the point is inside the triangle
    return ((sign1 == sign2) && (sign2 == sign3));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Parallelogram
ParallelogramBoundary::ParallelogramBoundary(LocationVector const& cornerA, LocationVector const& cornerB, LocationVector const& cornerD, bool isInverted) :
    AreaBoundary(isInverted), _a(cornerA), _b(cornerB), _d(cornerD), _c(DoublePosition(_d.getDoublePositionX() + (_b.getDoublePositionX() - _a.getDoublePositionX()), _d.getDoublePositionY() + (_b.getDoublePositionY() - _a.getDoublePositionY()))), _abx(_b.getDoublePositionX() - _a.getDoublePositionX()), _dax(_a.getDoublePositionX() - _d.getDoublePositionX()), _aby(_b.getDoublePositionY() - _a.getDoublePositionY()), _day(_a.getDoublePositionY() - _d.getDoublePositionY()) { }
bool ParallelogramBoundary::isWithinBoundaryArea(LocationVector const* pos) const
{
    // half-plane signs
    bool sign1 = ((-_b.getDoublePositionX() + pos->getPositionX()) * _aby - (-_b.getDoublePositionY() + pos->getPositionY()) * _abx) < 0;
    bool sign2 = ((-_a.getDoublePositionX() + pos->getPositionX()) * _day - (-_a.getDoublePositionY() + pos->getPositionY()) * _dax) < 0;
    bool sign3 = ((-_d.getDoublePositionY() + pos->getPositionY()) * _abx - (-_d.getDoublePositionX() + pos->getPositionX()) * _aby) < 0; // AB = -CD
    bool sign4 = ((-_c.getDoublePositionY() + pos->getPositionY()) * _dax - (-_c.getDoublePositionX() + pos->getPositionX()) * _day) < 0; // DA = -BC

    // if all signs are equal, the point is inside
    return ((sign1 == sign2) && (sign2 == sign3) && (sign3 == sign4));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Z Range
ZRangeBoundary::ZRangeBoundary(float minZ, float maxZ, bool isInverted) :
    AreaBoundary(isInverted), _minZ(minZ), _maxZ(maxZ) { }
bool ZRangeBoundary::isWithinBoundaryArea(LocationVector const* pos) const
{
    return (_minZ <= pos->getPositionZ() && pos->getPositionZ() <= _maxZ);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Union of 2 boundaries
BoundaryUnionBoundary::BoundaryUnionBoundary(std::unique_ptr<AreaBoundary const> b1, std::unique_ptr<AreaBoundary const> b2, bool isInverted) :
    AreaBoundary(isInverted), _b1(std::move(b1)), _b2(std::move(b2))
{
    ASSERT(b1 && b2);
}

BoundaryUnionBoundary::~BoundaryUnionBoundary() = default;

bool BoundaryUnionBoundary::isWithinBoundaryArea(LocationVector const* pos) const
{
    return (_b1->isWithinBoundary(pos) || _b2->isWithinBoundary(pos));
}
