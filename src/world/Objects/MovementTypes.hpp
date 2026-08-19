/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "MovementInfo.hpp"
#include "Server/ClientProtocol.hpp"
#include <array>
#include <cstdint>
#include <span>

enum class Cond : uint8_t
{
    Always,
    NoReadOptWrite,
    ReadOptWrite,
    HasTransport,
    HasFallData,
    HasFallDirection,
    HasOrientation,
    HasPitch,
    HasSplineElevation,
    HasTimestamp,
    HasMovementFlags,
    HasMovementFlags2,
    HasTransportTime2,
    HasTransportTime3,
    HasCount,
    IsTransportPresent
};

enum class MovementOp : uint16_t
{
    End,

    Guid,
    GuidBit0, GuidBit1, GuidBit2, GuidBit3, GuidBit4, GuidBit5, GuidBit6, GuidBit7,
    GuidByte0, GuidByte1, GuidByte2, GuidByte3, GuidByte4, GuidByte5, GuidByte6, GuidByte7,

    Guid2,
    Guid2Bit0, Guid2Bit1, Guid2Bit2, Guid2Bit3, Guid2Bit4, Guid2Bit5, Guid2Bit6, Guid2Bit7,
    Guid2Byte0, Guid2Byte1, Guid2Byte2, Guid2Byte3, Guid2Byte4, Guid2Byte5, Guid2Byte6, Guid2Byte7,

    Flags, Flags2,

    PosX, PosY, PosZ, Orientation,
    Timestamp,
    Pitch,
    FallTime,
    SplineElevation,

    JumpVelocity,
    JumpSin,
    JumpCos,
    JumpXYSpeed,

    TGuid,
    TGuidBit0, TGuidBit1, TGuidBit2, TGuidBit3, TGuidBit4, TGuidBit5, TGuidBit6, TGuidBit7,
    TGuidByte0, TGuidByte1, TGuidByte2, TGuidByte3, TGuidByte4, TGuidByte5, TGuidByte6, TGuidByte7,

    TPosX, TPosY, TPosZ, TOrientation,
    TTime, TTime2, TTime3, TSeat,

    HasTransport,
    HasFallData,
    HasFallDirection,
    HasOrientation,
    HasPitch,
    HasSpline,
    HasSplineElevation,
    HasTimestamp,
    HasMovementFlags,
    HasMovementFlags2,
    HasTransportTime2,
    HasTransportTime3,
    HasCount,

    ForcesCount,
    Count,
    NewSpeed,

    // collision height/scale change (SMSG_MOVE_UPDATE_COLLISION_HEIGHT, Cata/Mop only)
    CollisionHeight,

    SkipBit,
    SkipUInt32,
    SkipForcesCountUInt32,

    WriteBit0,
    WriteBit1,
    WriteUInt32_0,
    WriteUInt8_1,
    WriteFloat1,

    FlushBits
};

struct MovementStep
{
    MovementOp op;
    Cond cond = Cond::Always;
};

template <WoW::Expansion Version>
struct MovementVersionTraits;

template <>
struct MovementVersionTraits<WoW::Expansion::_Classic>
{
    static constexpr bool hasFlags2 = false;
    static constexpr int flags2BitWidth = 0;
    static constexpr bool flags2IsBitPacked = false;
};

template <>
struct MovementVersionTraits<WoW::Expansion::_TBC>
{
    static constexpr bool hasFlags2 = true;
    static constexpr int flags2BitWidth = 8;
    static constexpr bool flags2IsBitPacked = false;
};

template <>
struct MovementVersionTraits<WoW::Expansion::_WotLK>
{
    static constexpr bool hasFlags2 = true;
    static constexpr int flags2BitWidth = 16;
    static constexpr bool flags2IsBitPacked = false;
};

template <>
struct MovementVersionTraits<WoW::Expansion::_Cata>
{
    static constexpr bool hasFlags2 = true;
    static constexpr int flags2BitWidth = 12;
    static constexpr bool flags2IsBitPacked = true;
};

template <>
struct MovementVersionTraits<WoW::Expansion::_Mop>
{
    static constexpr bool hasFlags2 = true;
    static constexpr int flags2BitWidth = 13;
    static constexpr bool flags2IsBitPacked = true;
};
