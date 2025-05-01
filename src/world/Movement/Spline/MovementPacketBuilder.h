/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

class ByteBuffer;

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
    static void WriteSplineSync(MoveSpline const& mov, ByteBuffer& data);
};
} // namespace MovementMgr
