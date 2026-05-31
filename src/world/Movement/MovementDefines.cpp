/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MovementDefines.h"
#include "LocationVector.hpp"
#include "Objects/MovementInfo.hpp"
#include "Utilities/MathConstants.hpp"

#if VERSION_STRING >= Cata
#include "Logging/Logger.hpp"
#endif

#define CONTACT_DISTANCE 0.5f

//////////////////////////////////////////////////////////////////////////////////////////
// ChaseRange
ChaseRange::ChaseRange(float range) : MinRange(range > CONTACT_DISTANCE ? 0 : range - CONTACT_DISTANCE), MinTolerance(range), MaxRange(range + CONTACT_DISTANCE), MaxTolerance(range) { }
ChaseRange::ChaseRange(float _minRange, float _maxRange) : MinRange(_minRange), MinTolerance(std::min(_minRange + CONTACT_DISTANCE, (_minRange + _maxRange) / 2)), MaxRange(_maxRange), MaxTolerance(std::max(_maxRange - CONTACT_DISTANCE, MinTolerance)) { }
ChaseRange::ChaseRange(float _minRange, float _minTolerance, float _maxTolerance, float _maxRange) : MinRange(_minRange), MinTolerance(_minTolerance), MaxRange(_maxRange), MaxTolerance(_maxTolerance) { }

//////////////////////////////////////////////////////////////////////////////////////////
// ChaseAngle
ChaseAngle::ChaseAngle(float angle, float _tolerance/* = M_PI_4*/) : RelativeAngle(LocationVector::normalizeOrientation(angle)), Tolerance(_tolerance) { }

float ChaseAngle::upperBound() const
{
    return LocationVector::normalizeOrientation(RelativeAngle + Tolerance);
}

float ChaseAngle::lowerBound() const
{
    return LocationVector::normalizeOrientation(RelativeAngle - Tolerance);
}

bool ChaseAngle::isAngleOkay(float relativeAngle) const
{
    float const diff = std::abs(relativeAngle - RelativeAngle);

    return (std::min(diff, float(2 * AscEmu::Math::PiF) - diff) <= Tolerance);
}
