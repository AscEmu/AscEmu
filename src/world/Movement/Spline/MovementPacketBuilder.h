/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

class ByteBuffer;
class Unit;

namespace G3D
{
    class Vector3;
}

namespace MovementMgr {

class MoveSpline;
class PacketBuilder
{
    static void WriteCommonMonsterMovePart(MoveSpline const& mov, ByteBuffer& data);
public:
    static void WriteMonsterMove(MoveSpline const& mov, ByteBuffer& data);
    static void WriteStopMovement(G3D::Vector3 const& loc, uint32_t splineId, ByteBuffer& data);
    static void WriteCreate(MoveSpline const& mov, ByteBuffer& data);
#if VERSION_STRING >= Cata
    static void WriteCreateData(MoveSpline const& moveSpline, ByteBuffer& data);
    static void WriteCreateBits(MoveSpline const& moveSpline, ByteBuffer& data);
#endif
    // Mop 5.4.8 uses a completely different, bit-packed payload that also
    // folds the moving unit's transport guid/offsets/vehicle-seat directly into the body (Mop has
    // no separate SMSG_MONSTER_MOVE_TRANSPORT opcode at all). These overloads are additive-only:
    // they are only invoked at runtime when m_protocol.isMop() is true. The pre-Mop overloads
    // above are untouched and keep serving Classic/TBC/WotLK/Cata connections exactly as before.
    // Not compile-time gated (unlike WriteCreateData/WriteCreateBits above): the call site picks
    // between the pre-Mop and Mop overloads via a runtime m_protocol.isMop() check inside a single
    // function shared by all versions, so both overloads must exist in every build.
    static void WriteMonsterMove(MoveSpline const& mov, ByteBuffer& data, Unit* unit);
    static void WriteStopMovement(G3D::Vector3 const& loc, uint32_t splineId, ByteBuffer& data, Unit* unit);
    static void WriteSplineSync(MoveSpline const& mov, ByteBuffer& data);
};
} // namespace MovementMgr
