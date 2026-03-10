/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum class MovementElement : uint8_t
{
    PositionX, PositionY, PositionZ, Orientation, Pitch,

    HasMovementFlags, HasMovementFlags2,
    HasTimestamp, HasFallData, HasPitch, HasOrientation,
    HasSpline, HasSplineElevation, HasTransportData,
    HasFallDirection, HasCounter, ZeroBit,

    HasGuidByte0, HasGuidByte1, HasGuidByte2, HasGuidByte3,
    HasGuidByte4, HasGuidByte5, HasGuidByte6, HasGuidByte7,

    GuidByte0, GuidByte1, GuidByte2, GuidByte3,
    GuidByte4, GuidByte5, GuidByte6, GuidByte7,

    HasTransportGuidByte0, HasTransportGuidByte1, HasTransportGuidByte2, HasTransportGuidByte3,
    HasTransportGuidByte4, HasTransportGuidByte5, HasTransportGuidByte6, HasTransportGuidByte7,
    HasTransportTime2, HasTransportTime3,

    TransportGuidByte0, TransportGuidByte1, TransportGuidByte2, TransportGuidByte3,
    TransportGuidByte4, TransportGuidByte5, TransportGuidByte6, TransportGuidByte7,
    TransportPositionX, TransportPositionY, TransportPositionZ, TransportOrientation,
    TransportTime, TransportTime2, TransportTime3, TransportSeat,

    MovementFlags, MovementFlags2,
    Timestamp, Counter,
    FallTime, FallVerticalSpeed, FallCosAngle, FallSinAngle, FallHorizontalSpeed,
    SplineElevation,
    ForcesCount, Forces
};