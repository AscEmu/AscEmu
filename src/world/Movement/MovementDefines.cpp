/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MovementDefines.h"
#include "LocationVector.h"
#include "Objects/MovementInfo.hpp"

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
ChaseAngle::ChaseAngle(float angle, float _tolerance/* = M_PI_4*/) : RelativeAngle(normalizeOrientation(angle)), Tolerance(_tolerance) { }

float ChaseAngle::upperBound() const
{
    return normalizeOrientation(RelativeAngle + Tolerance);
}

float ChaseAngle::lowerBound() const
{
    return normalizeOrientation(RelativeAngle - Tolerance);
}

bool ChaseAngle::isAngleOkay(float relativeAngle) const
{
    float const diff = std::abs(relativeAngle - RelativeAngle);

    return (std::min(diff, float(2 * M_PI) - diff) <= Tolerance);
}

#if VERSION_STRING >= Cata
void ExtraMovementStatusElement::readNextElement(ByteBuffer& packet)
{
    MovementStatusElements const element = _elements[_index++];

    switch (element)
    {
        case MSEGuidBit0:
        case MSEGuidBit1:
        case MSEGuidBit2:
        case MSEGuidBit3:
        case MSEGuidBit4:
        case MSEGuidBit5:
        case MSEGuidBit6:
        case MSEGuidBit7:
            Data.guid[element - MSEGuidBit0] = packet.readBit();
            break;
        case MSEGuidByte0:
        case MSEGuidByte1:
        case MSEGuidByte2:
        case MSEGuidByte3:
        case MSEGuidByte4:
        case MSEGuidByte5:
        case MSEGuidByte6:
        case MSEGuidByte7:
            packet.ReadByteSeq(Data.guid[element - MSEGuidByte0]);
            break;
        case MSEExtraFloat:
            packet >> Data.floatData;
            break;
        case MSEExtraInt8:
            packet >> Data.byteData;
            break;
        default:
            sLogger.failure("Incorrect extraMovementStatusElement sequence {} detected", element);
            break;
    }
}

void ExtraMovementStatusElement::writeNextElement(ByteBuffer& packet)
{
    MovementStatusElements const element = _elements[_index++];

    switch (element)
    {
        case MSEGuidBit0:
        case MSEGuidBit1:
        case MSEGuidBit2:
        case MSEGuidBit3:
        case MSEGuidBit4:
        case MSEGuidBit5:
        case MSEGuidBit6:
        case MSEGuidBit7:
            packet.writeBit(Data.guid[element - MSEGuidBit0]);
            break;
        case MSEGuidByte0:
        case MSEGuidByte1:
        case MSEGuidByte2:
        case MSEGuidByte3:
        case MSEGuidByte4:
        case MSEGuidByte5:
        case MSEGuidByte6:
        case MSEGuidByte7:
            packet.WriteByteSeq(Data.guid[element - MSEGuidByte0]);
            break;
        case MSEExtraFloat:
            packet << Data.floatData;
            break;
        case MSEExtraInt8:
            packet << Data.byteData;
            break;
        default:
            sLogger.failure("Incorrect extraMovementStatusElement sequence {} detected", element);
            break;
    }
}
#endif
